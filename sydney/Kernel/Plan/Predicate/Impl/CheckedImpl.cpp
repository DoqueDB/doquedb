// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CheckedImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Predicate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/CheckedImpl.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckUnknown.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Predicate/Fetch.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/PrepareFailed.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	// CONSTANT local
	//	$$$::_cEmptyFileSet --
	//
	// NOTES

	Utility::FileSet _cEmptyFileSet;

	// CLASS local
	//	_SortByCost -- sorter for chosen operands
	//
	// NOTES
	class _SortByCost
	{
	public:
		_SortByCost(Opt::Environment& cEnvironment_,
					Candidate::Table* pTable_,
					bool bIsTop_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(pTable_, bIsTop_),
			  m_cTableCount()
		{
			if (pTable_) m_cTableCount = pTable_->getEstimateCount(cEnvironment_);
			else m_cTableCount = 1;
		}

		bool operator()(Interface::IPredicate* pPredicate0_,
						Interface::IPredicate* pPredicate1_)
		{
			bool bNeedScan0 = pPredicate0_->getChosen()->isNeedScan(m_cArgument);
			bool bNeedScan1 = pPredicate1_->getChosen()->isNeedScan(m_cArgument);
			if (bNeedScan0 == bNeedScan1) {
				if ((pPredicate0_->getType() == Tree::Node::Not)
					!= (pPredicate1_->getType() == Tree::Node::Not)) {
					// if either is NOT, it should be placed tail
					return pPredicate1_->getType() == Tree::Node::Not;
				}
				if ((pPredicate0_->isNeedIndex() || pPredicate0_->isFetch())
					== (pPredicate1_->isNeedIndex() || pPredicate1_->isFetch())) {
					// compare tuple count
					AccessPlan::Cost cCost0;
					AccessPlan::Cost cCost1;
					cCost0.setIsSetCount(false);
					cCost1.setIsSetCount(false);
					cCost0.setTableCount(m_cTableCount);
					cCost1.setTableCount(m_cTableCount);
					pPredicate0_->getChosen()->getCost(m_cEnvironment, cCost0);
					pPredicate1_->getChosen()->getCost(m_cEnvironment, cCost1);

					if (cCost0.isFetch() != cCost1.isFetch()) {
						return cCost0.isFetch();
					}
					return cCost0.getRate() < cCost1.getRate();
				}
				// otherwise need index predicate should become first
				return pPredicate0_->isNeedIndex();
			}
			// need scan predicate should become last
			return !bNeedScan0;
		}
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		Predicate::CheckNeedScanArgument m_cArgument;
		AccessPlan::Cost::Value m_cTableCount;
	};

} // namespace

///////////////////////////////////////////////
// Plan::Predicate::CheckedImpl::Base

// FUNCTION public
//	Predicate::CheckedImpl::Base::getFile -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Utility::FileSet&
//
// EXCEPTIONS

//virtual
const Utility::FileSet&
CheckedImpl::Base::
getFile()
{
	return _cEmptyFileSet;
}

// FUNCTION public
//	Predicate::CheckedImpl::Base::choose -- choose one possible usage of index files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::Base::
choose(Opt::Environment& cEnvironment_,
	   ChooseArgument& cArgument_)
{
	if (cArgument_.m_bForceOrder) {
		return 0;
	} else {
		// add refered columns as retrieved
		getPredicate()->require(cEnvironment_,
								cArgument_.m_pCandidate);
		// create choseninterface
		return Predicate::ChosenInterface::create(cEnvironment_,
												  getPredicate(),
												  getNotChecked(),
												  static_cast<Candidate::File*>(0));
	}
}

////////////////////////////////////////////////////
// Plan::Predicate::CheckedImpl::SingleIndex

// FUNCTION public
//	Predicate::CheckedImpl::SingleIndex::choose -- choose one possible usage of index files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::SingleIndex::
choose(Opt::Environment& cEnvironment_,
	   ChooseArgument& cArgument_)
{
	if (getFile().isEmpty() == false) {
		; _SYDNEY_ASSERT(getTable());

		ChosenInterface* pResult = chooseIndex(cEnvironment_,
											   getPredicate(),
											   cArgument_);

		if (pResult) {
			if (isFetch() == false
				&& cArgument_.m_pFetch) {
				// rechoose to check cost
				pResult = pResult->rechoose(cEnvironment_, cArgument_)->getChosen();
			}
			return pResult;
		}
	}
	// no index files can be used
	if (isNeedIndex()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return Super::choose(cEnvironment_,
						 cArgument_);
}

// FUNCTION public
//	Predicate::CheckedImpl::SingleIndex::createFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::SingleIndex::
createFetch(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	Interface::IPredicate* pFetchForm = getPredicate()->createFetch(cEnvironment_,
																	cFetchKey_);
	if (pFetchForm) {
		return CheckedInterface::create(cEnvironment_,
										pFetchForm,
										getTable(),
										getFile());
	}
	return 0;
}

// FUNCTION protected
//	Predicate::CheckedImpl::SingleIndex::chooseIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	ChooseArgument& cArgument_
//	
// RETURN
//	ChosenInterface*
//
// EXCEPTIONS

ChosenInterface*
CheckedImpl::SingleIndex::
chooseIndex(Opt::Environment& cEnvironment_,
			Interface::IPredicate* pPredicate_,
			ChooseArgument& cArgument_)
{
	// in preparation, predicates with placeholder can't be adopted here
	if (cEnvironment_.isPrepare()
		&& getPredicate()->hasParameter()) {
		_SYDNEY_THROW0(Exception::PrepareFailed);
	}

	File::CheckArgument cCheckArgument(getTable());
	if (chooseFile(cEnvironment_, pPredicate_, cArgument_, cCheckArgument)) {

		if (pPredicate_->isFetch()) {
			if (cArgument_.m_pFetch) {
				// already fetched -> don't use index
				return 0;
			}
		}

		Candidate::File* pFile =
			cCheckArgument.m_pFile->createCandidate(cEnvironment_,
													getTable(),
													cCheckArgument.m_pParameter);
		Interface::IPredicate* pCheckUnknown = 0;
		if (cArgument_.m_bCheckUnknown) {
			CheckUnknownArgument cCheckUnknownArgument;
			if (getPredicate()->getCheckUnknown(cEnvironment_,
												cCheckUnknownArgument)) {
				if (cCheckUnknownArgument.m_cKey.isEmpty() == false) {
					pCheckUnknown = CheckUnknown::create(cEnvironment_,
														 cCheckUnknownArgument.m_cKey.getVector(),
														 cCheckUnknownArgument.m_bArray);
				}
			} else {
				// can't create check unknown predicate -> can't use index
				return 0;
			}
		}

		// remember schema file when table is updated
		pFile->setForUpdate(cEnvironment_);

		ChosenInterface* pResult =
			Predicate::ChosenInterface::create(
								  cEnvironment_,
								  getPredicate(),
								  getNotChecked(),
								  pFile,
								  pCheckUnknown);
		if (pPredicate_->isFetch()) {
			cArgument_.m_pFetch = pResult;
		}
		return pResult;
	}
	// no index file can be used
	return 0;
}

// FUNCTION protected
//	Predicate::CheckedImpl::SingleIndex::chooseFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	ChooseArgument& cArgument_
//	File::CheckArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckedImpl::SingleIndex::
chooseFile(Opt::Environment& cEnvironment_,
		   Interface::IPredicate* pPredicate_,
		   ChooseArgument& cArgument_,
		   File::CheckArgument& cCheckArgument_)
{
	// check availability using logical file interface
	Utility::FileSet cFile;
	Utility::FileSet cRestFile;

	Utility::FileSet::ConstIterator cFirst;
	Utility::FileSet::ConstIterator cLast;

	ChooseArgument cMyArgument(cArgument_);

	if (cArgument_.m_pCheckedOrder
		&& cArgument_.m_pCheckedOrder->getFile().isContainingAny(getFile())) {
		// check ordering files first
		getFile().intersect(cArgument_.m_pCheckedOrder->getFile(),
							cFile);
		if (cArgument_.m_bForceOrder == false) {
			// if ordering is forced, rest files are not checked
			// if order is not forced, rest file is whole available files
			cRestFile = getFile();
		}
		cFirst = cFile.begin();
		cLast = cFile.end();

	} else {
		if (cArgument_.m_bForceOrder
			|| (cArgument_.m_pCheckedOrder
				&& isFetch())) {
			return 0;
		}

		cFirst = getFile().begin();
		cLast = getFile().end();
		cMyArgument.m_pCheckedOrder = 0; // no order check
	}
	; _SYDNEY_ASSERT(cFirst != cLast);

	if (isNeedIndex()) {
		cCheckArgument_.noEstimate();
	}

	Relation::Table::AutoReset cReset = getTable()->getTable()->setEstimatePredicate(pPredicate_);

	bool bResult = checkFiles(cEnvironment_,
							  pPredicate_,
							  getTable(),
							  cFirst,
							  cLast,
							  cCheckArgument_,
							  cMyArgument);

	if (bResult
		&& cArgument_.m_pCheckedOrder != 0
		&& cArgument_.m_pCheckedOrder->hasAlternativeValue(cEnvironment_) == false
		&& cCheckArgument_.m_pParameter
		&& cCheckArgument_.m_pParameter->getOrder() != 0) {
		// file to process order has been found
		// -> no more check for ordering
		cArgument_.m_pCheckedOrder = 0;
	}

	if (!bResult && !cRestFile.isEmpty()) { // check rest files if exists
		cMyArgument.m_pCheckedOrder = 0;
		bResult = checkFiles(cEnvironment_,
							 pPredicate_,
							 getTable(),
							 cRestFile.begin(),
							 cRestFile.end(),
							 cCheckArgument_,
							 cMyArgument);
	}

	return bResult;
}

// FUNCTION private
//	Predicate::CheckedImpl::SingleIndex::checkFiles -- check availability for a set of file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::Table* pTable_
//	Utility::FileSet::ConstIterator first_
//	Utility::FileSet::ConstIterator last_
//	File::CheckArgument& cCheckArgument_
//	ChooseArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckedImpl::SingleIndex::
checkFiles(Opt::Environment& cEnvironment_,
		   Interface::IPredicate* pPredicate_,
		   Candidate::Table* pTable_,
		   Utility::FileSet::ConstIterator first_,
		   Utility::FileSet::ConstIterator last_,
		   File::CheckArgument& cCheckArgument_,
		   ChooseArgument& cArgument_)
{
	if (last_ - first_ <= 1) {
		cCheckArgument_.skipEstimate();
	}
	Utility::FileSet::ConstIterator min =
		Opt::FindLast(first_,
					  last_,
					  boost::bind(
							  &This::checkFile,
							  this,
							  boost::ref(cEnvironment_),
							  pPredicate_,
							  pTable_,
							  _1,
							  static_cast<File::Parameter*>(0),
							  boost::ref(cCheckArgument_),
							  boost::ref(cArgument_)));
	return min != last_;
}

// FUNCTION protected
//	Predicate::CheckedImpl::SingleIndex::checkFile -- check availability for each file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	File::Parameter* pParameter_
//	File::CheckArgument& cCheckArgument_
//	ChooseArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckedImpl::SingleIndex::
checkFile(Opt::Environment& cEnvironment_,
		  Interface::IPredicate* pPredicate_,
		  Candidate::Table* pTable_,
		  Interface::IFile* pFile_,
		  File::Parameter* pParameter_,
		  File::CheckArgument& cCheckArgument_,
		  ChooseArgument& cArgument_)
{
	File::CheckArgument cMyCheckArgument(cCheckArgument_);
	bool bResult =
		pFile_->isSearchable(cEnvironment_,
							 pPredicate_,
							 pTable_,
							 pParameter_,
							 cMyCheckArgument,
							 cArgument_.m_cScanCost);

	if (bResult) {
		; _SYDNEY_ASSERT(cMyCheckArgument.m_pFile);

		if (cMyCheckArgument.m_pParameter) {
			// set predicate processed by the parameter
			cMyCheckArgument.m_pParameter->setPredicate(pPredicate_);
		}

		// check order if available
		if (cMyCheckArgument.m_pParameter
			&& cArgument_.m_pCheckedOrder
			&& checkOrder(cEnvironment_,
						  pTable_,
						  pFile_,
						  cMyCheckArgument,
						  cArgument_.m_pCheckedOrder,
						  cArgument_.m_cScanCost) == false) {
			// can't process order
			bResult = false;

		} else {
			// set result parameter
			cCheckArgument_ = cMyCheckArgument;
		}
	}
	return bResult;
}

// FUNCTION protected
//	Predicate::CheckedImpl::SingleIndex::checkOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Table* pTable_
//	Interface::IFile* pFile_
//	File::CheckArgument& cCheckArgument_
//	Order::CheckedSpecification* pCheckedOrder_
//	const AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckedImpl::SingleIndex::
checkOrder(Opt::Environment& cEnvironment_,
		   Candidate::Table* pTable_,
		   Interface::IFile* pFile_,
		   File::CheckArgument& cCheckArgument_,
		   Order::CheckedSpecification* pCheckedOrder_,
		   const AccessPlan::Cost& cCost_)
{
	// if order is specified, this file can be chosen only when order can be processed
	File::CheckOrderArgument cOrderArgument(pTable_);
	cCheckArgument_.skipEstimate();

	if (pCheckedOrder_->createCheckOrderArgument(cEnvironment_,
												 pFile_,
												 cOrderArgument)
		&& pTable_->checkFile(cEnvironment_,
							  pFile_,
							  cCheckArgument_.m_pParameter,
							  cCheckArgument_,
							  cCost_,
							  boost::bind(&LogicalFile::AutoLogicalFile::getSortParameter,
										  _1,
										  cOrderArgument.m_pOrder,
										  _2))) {
		; _SYDNEY_ASSERT(cCheckArgument_.m_pParameter);

		if (pCheckedOrder_->isGroupBy()
			&& pFile_->getSchemaFile()->isAbleToBitSetSort()) {
			pCheckedOrder_->setBitSetSort();
		}

		cCheckArgument_.m_pParameter->setOrder(pCheckedOrder_);

		return true;
	}
	return false;
}

////////////////////////////////////////////////////////
// Plan::Predicate::CheckedImpl::MultipleIndex

// FUNCTION public
//	Predicate::CheckedImpl::MultipleIndex::choose -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::MultipleIndex::
choose(Opt::Environment& cEnvironment_,
	   ChooseArgument& cArgument_)
{
	ChooseArgument cMyChooseArgument(cArgument_);
	VECTOR<Interface::IPredicate*> vecChosen;
	ChosenInterface* pResult = 0;

	if (getFile().isEmpty() == false) {

		; _SYDNEY_ASSERT(getTable());

		pResult = chooseIndex(cEnvironment_,
							  getPredicate(),
							  cArgument_);
		if (pResult) {
			// index file can process whole predicate
			if (cMyChooseArgument.m_bForceOrder == false
				&& cArgument_.m_pCheckedOrder
				&& (!m_bNoTop &&  getType() == Tree::Node::And)
				&& pResult->getFile(cEnvironment_)
				&& pResult->getFile(cEnvironment_)->hasOrder() == false) {
				// but can't process order
				// -> find index file for ordering using operand one by one
				vecChosen.PUSHBACK(pResult);

				cMyChooseArgument.m_bForceOrder = true;

				// continue to getFile().isEmpty() cases

			} else {
				// order is not specified or it can process order
				return pResult;
			}
		}
	}
	// no common index file which can evaluate whole
	// -> choose operands one by one

	if (cMyChooseArgument.m_bForceOrder) {
		// just search for order processable file
		VECTOR<Interface::IPredicate*>::ITERATOR iterator = m_vecChecked.begin();
		const VECTOR<Interface::IPredicate*>::ITERATOR last = m_vecChecked.end();
		for (; iterator != last; ++iterator) {
			Interface::IPredicate* pChosen =
				(*iterator)->getChecked()->choose(cEnvironment_,
												  cMyChooseArgument);
			if (pChosen) {
				// order can be processed
				if (vecChosen.ISEMPTY()) {
					// just return this predicate
					return pChosen;
				} else {
					vecChosen.PUSHBACK(pChosen);
					return Predicate::ChosenInterface::create(cEnvironment_,
															  getPredicate(),
															  getNotChecked(),
															  vecChosen);
				}
			}
		}
		// no index file for operand can process order
		return pResult;
	}

	; _SYDNEY_ASSERT(cMyChooseArgument.m_bForceOrder == false);

	if ((m_bNoTop || getType() != Tree::Node::And)
		&& cMyChooseArgument.m_pCheckedOrder
		&& cMyChooseArgument.m_pCheckedOrder->hasAlternativeValue(cEnvironment_) == false) {
		// don't process order in operands
		cMyChooseArgument.m_pCheckedOrder = 0;
	}

	Opt::MapContainer(m_vecChecked,
					  vecChosen,
					  boost::bind(&CheckedInterface::choose,
								  boost::bind(&Interface::IPredicate::getChecked,
											  _1),
								  boost::ref(cEnvironment_),
								  boost::ref(cMyChooseArgument)));
	if (m_bNoTop == false
		&& getType() == Tree::Node::And) {
		checkCost(cEnvironment_,
				  cMyChooseArgument,
				  vecChosen);
	}

	return Predicate::ChosenInterface::create(cEnvironment_,
											  getPredicate(),
											  getNotChecked(),
											  vecChosen);
}

// FUNCTION public
//	Predicate::CheckedImpl::MultipleIndex::isFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CheckedImpl::MultipleIndex::
isFetch()
{
	return Opt::IsAny(m_vecChecked,
					  boost::bind(&Interface::IPredicate::isFetch,
								  _1));
}

// FUNCTION public
//	Predicate::CheckedImpl::MultipleIndex::getFetchKey -- get fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CheckedImpl::MultipleIndex::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	Utility::ScalarSet cTmpFetchKey;
	bool bResult = Opt::IsAll(m_vecChecked,
							  boost::bind(&Interface::IPredicate::getFetchKey,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cTmpFetchKey)));
	bResult = bResult && (cFetchKey_.isContaining(cTmpFetchKey) == false);
	if (bResult) {
		cFetchKey_.merge(cTmpFetchKey);
	}
	return bResult;
}

// FUNCTION private
//	Predicate::CheckedImpl::MultipleIndex::checkCost -- check cost of operand candidates
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	VECTOR<Interface::IPredicate*>& vecChosen_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
CheckedImpl::MultipleIndex::
checkCost(Opt::Environment& cEnvironment_,
		  ChooseArgument& cArgument_,
		  VECTOR<Interface::IPredicate*>& vecChosen_)
{
	; _SYDNEY_ASSERT(Opt::IsAll(vecChosen_, boost::bind(&Interface::IPredicate::isChosen, _1)));

	if (vecChosen_.GETSIZE() > 1) {
		// sort operands by cost
		SORT(vecChosen_.begin(), vecChosen_.end(),
			 _SortByCost(cEnvironment_, getTable(), !m_bNoTop));

		// search for operand at which scanning become less cost
		AccessPlan::Cost::Value cLimit;

		if (cArgument_.m_pCheckedOrder != 0
			&& Opt::IsAny(vecChosen_, boost::bind(&ChosenInterface::hasOrder,
												  boost::bind(&Interface::IPredicate::getChosen, _1),
												  cArgument_.m_pCheckedOrder))) {
			cLimit = cArgument_.m_cEstimateLimit;
		}

		Predicate::ChosenInterface* pTop = vecChosen_[0]->getChosen();

		// use minimum tuple count for scanning count
		AccessPlan::Cost cCost;
		if (getTable()) {
			cCost.setTableCount(getTable()->getEstimateCount(cEnvironment_));
		} else if (cArgument_.m_cScanCost.getTableCount().isInfinity() == false) {
			cCost.setTableCount(cArgument_.m_cScanCost.getTableCount());
		} else {
			cCost.setTableCount(1);
		}
		pTop->getCost(cEnvironment_, cCost);

		AccessPlan::Cost::Value cCount =
			cCost.getTupleCount() * cArgument_.m_cRepeatCount;

		VECTOR<Interface::IPredicate*>::ITERATOR start = vecChosen_.begin();
		VECTOR<Interface::IPredicate*>::ITERATOR last = vecChosen_.end();
		if ((cArgument_.m_pFetch == 0
			 || (*start)->isFetch())
			&& cArgument_.m_pCheckedOrder == 0) {
			// first operand always use index unless sorting or fetch is processed
			++start;
		}
		VECTOR<Interface::IPredicate*>::ITERATOR found =
			Opt::Find(start,
					  last,
					  boost::bind(&This::compareCost,
								  this,
								  boost::ref(cEnvironment_),
								  boost::cref(cArgument_),
								  boost::cref(cCount),
								  boost::cref(cLimit),
								  _1));

		for (; found != last; ++found) {
			if ((*found)->isNeedIndex()) {
				// use it
				continue;
			} else {
				// change to noindex
				// add refered columns as retrieved
				(*found)->require(cEnvironment_,
								  cArgument_.m_pCandidate);
				(*found)->getChosen()->setScanBetter();
			}
		}
	}
}

// FUNCTION private
//	Predicate::CheckedImpl::MultipleIndex::compareCost -- compare cost of one operand
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const ChooseArgument& cArgument_
//	const AccessPlan::Cost::Value& cCount_
//	const AccessPlan::Cost::Value& cLimit_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
CheckedImpl::MultipleIndex::
compareCost(Opt::Environment& cEnvironment_,
			const ChooseArgument& cArgument_,
			const AccessPlan::Cost::Value& cCount_,
			const AccessPlan::Cost::Value& cLimit_,
			Interface::IPredicate* pPredicate_)
{
	if (pPredicate_->isNeedIndex()) {
		return false;
	}
	; _SYDNEY_ASSERT(pPredicate_->isChosen());

	if (cArgument_.m_pCheckedOrder
		&& pPredicate_->getChosen()->getFile(cEnvironment_)
		&& cArgument_.m_pCheckedOrder->getFile().isContaining(
			   pPredicate_->getChosen()->getFile(cEnvironment_)->getFile())) {
		// if chosen predicate uses required file, use it anyway
		return false;
	}

	if (cArgument_.m_pFetch
		&& pPredicate_->isFetch()) {
		// if fetch is already processed, other fetch can't use index
		// -> disable here
		pPredicate_->require(cEnvironment_,
							 cArgument_.m_pCandidate);
		pPredicate_->getChosen()->setScanBetter();
		// continue process for other predicates
		return false;
	}

	// scanning count = cCount (if no limit)
	//                  cLimit * (cTotalCount / cMyCount) (if with limit)
	// index count = cMyCount
	; _SYDNEY_ASSERT(cArgument_.m_pFetch == 0
					 || cArgument_.m_pFetch->isChosen());

	AccessPlan::Cost cMyCost;
	cMyCost.setTableCount(cCount_);
	pPredicate_->getChosen()->getCost(cEnvironment_, cMyCost);

	AccessPlan::Cost cScanCost;
	if (cArgument_.m_pFetch) {
		cArgument_.m_pFetch->getChosen()->getCost(cEnvironment_, cScanCost);
	} else {
		cScanCost = cArgument_.m_cScanCost;
	}

	AccessPlan::Cost::Value cScanCount;
	if (cLimit_.isInfinity()) {
		// no limit
		cScanCount = cCount_;
	} else {
		// with limit
		if (cMyCost.getTupleCount() >= 1) {
			cScanCount = cLimit_ * (cScanCost.getTupleCount() / cMyCost.getTupleCount());
			if (cScanCount > cCount_) {
				cScanCount = cCount_;
			}
		} else {
			cScanCount = cCount_;
		}
	}
	cScanCost.setLimitCount(cScanCount);

	pPredicate_->getChosen()->addLockPenalty(cEnvironment_,
											 cMyCost,
											 cArgument_.m_cScanCost);

	// index scans all tuples -> no limit set
	// compare costs
	return cScanCost < cMyCost;
}

////////////////////////////////////////////////////////
// Plan::Predicate::CheckedImpl::CombinedFetch

// FUNCTION public
//	Predicate::CheckedImpl::CombinedFetch::choose -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::CombinedFetch::
choose(Opt::Environment& cEnvironment_,
	   ChooseArgument& cArgument_)
{
	if (cArgument_.m_bForceOrder) return 0;
	if (cArgument_.m_bCheckUnknown) {
		// if unknown case should be checked, obviously index cannot be used for fetch
		return Base::choose(cEnvironment_,
							cArgument_);
	}

	if (getFile().isEmpty() == false) {

		// in preparation, predicates with placeholder can't be adopted here
		if (cEnvironment_.isPrepare()
			&& getPredicate()->hasParameter()) {
			_SYDNEY_THROW0(Exception::PrepareFailed);
		}

		; _SYDNEY_ASSERT(getTable());
		; _SYDNEY_ASSERT(m_cChecked.first->isChecked());
		; _SYDNEY_ASSERT(m_cChecked.second->isChecked());

		// first, check non-fetch predicate
		File::CheckArgument cCheckArgument(getTable());
		ChooseArgument cMyArgument(cArgument_);
		cMyArgument.m_pCheckedOrder = 0;

		if (chooseFile(cEnvironment_,
					   m_cChecked.second->getChecked()->getPredicate(),
					   cMyArgument,
					   cCheckArgument)) {
			// index file can process whole predicate
			// -> check fetch predicate using chosen file
			if (checkFile(cEnvironment_,
						  m_cChecked.first->getChecked()->getPredicate(),
						  getTable(),
						  cCheckArgument.m_pFile,
						  cCheckArgument.m_pParameter,
						  cCheckArgument,
						  cMyArgument)) {
				// both can be processed -> choose the file

				; _SYDNEY_ASSERT(cCheckArgument.m_pParameter);
				cCheckArgument.m_pParameter->setPredicate(getPredicate());
				Candidate::File* pFile =
					cCheckArgument.m_pFile->createCandidate(cEnvironment_,
															getTable(),
															cCheckArgument.m_pParameter);

				// remember schema file when table is updated
				pFile->setForUpdate(cEnvironment_);

				; _SYDNEY_ASSERT(getPredicate()->isFetch());

				return Predicate::ChosenInterface::create(
									  cEnvironment_,
									  m_cChecked.first,
									  getNotChecked(),
									  pFile);
			}
		}
	}
	// choose each
	VECTOR<Interface::IPredicate*> vecChosen;
	vecChosen.PUSHBACK(m_cChecked.first->getChecked()->choose(cEnvironment_,
															  cArgument_));
	vecChosen.PUSHBACK(m_cChecked.second->getChecked()->choose(cEnvironment_,
															   cArgument_));

	return Predicate::ChosenInterface::create(cEnvironment_,
											  getPredicate(),
											  getNotChecked(),
											  vecChosen);
}

// FUNCTION public
//	Predicate::CheckedImpl::CombinedFetch::isFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CheckedImpl::CombinedFetch::
isFetch()
{
	return true;
}

// FUNCTION public
//	Predicate::CheckedImpl::CombinedFetch::getFetchKey -- get fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CheckedImpl::CombinedFetch::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	return m_cChecked.second->getFetchKey(cEnvironment_, cFetchKey_);
}

////////////////////////////////////////////////////////
// Plan::Predicate::CheckedImpl::OperandIndex

// FUNCTION public
//	Predicate::CheckedImpl::OperandIndex::choose -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
CheckedImpl::OperandIndex::
choose(Opt::Environment& cEnvironment_,
	   ChooseArgument& cArgument_)
{
	; _SYDNEY_ASSERT(m_pOperand->isChecked());
	; _SYDNEY_ASSERT(m_pOperand->getNotChecked() == 0);

	ChooseArgument cMyArgument(cArgument_);
	// no order
	cMyArgument.m_pCheckedOrder = 0;
	cMyArgument.m_bForceOrder = false;
	if (Opt::Configuration::getNoUnknown().get() == false) {
		cMyArgument.m_bCheckUnknown = true;
	}

	Interface::IPredicate* pOperand =
		m_pOperand->getChecked()->choose(cEnvironment_, cMyArgument);

	return Predicate::ChosenInterface::create(cEnvironment_,
											  getPredicate(),
											  pOperand);
}

// FUNCTION public
//	Predicate::CheckedImpl::OperandIndex::isFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
CheckedImpl::OperandIndex::
isFetch()
{
	return false;
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
