// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/FileImpl.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/FileImpl.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Candidate/Row.h"
#include "Plan/File/Parameter.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IRow.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Tree/Fetch.h"
#include "Plan/Utility/Transaction.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Exception/VariableNotFound.h"

#include "Execution/Action/Argument.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Action/Locker.h"
#include "Execution/Collection/BitsetDisintegration.h"
#include "Execution/Collection/Distinct.h"
#include "Execution/Collection/Sort.h"
#include "Execution/Collection/Store.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/BitSet.h"
#include "Execution/Iterator/CascadeInput.h"
#include "Execution/Iterator/File.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Loop.h"
#include "Execution/Iterator/MergeSort.h"
#include "Execution/Operator/FileFetch.h"
#include "Execution/Operator/FileOperation.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Locker.h"
#include "Execution/Operator/SetNull.h"
#include "Execution/Operator/UndoLog.h"
#include "Execution/Predicate/CollectionCheck.h"
#include "Execution/Predicate/Comparison.h"
#include "Execution/Predicate/RowIDCheck.h"
#include "Execution/Utility/Transaction.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/Estimate.h"
#include "LogicalFile/OpenOption.h"

#include "Opt/Algorithm.h"
#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/UndoLog.h"

#include "Schema/Field.h"
#include "Schema/File.h"

#include "Server/Session.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace
{
	// CLASS local
	//	$$$::_ByPosition -- function class for sorting
	//
	// NOTES

	class _ByPosition
	{
	public:
		bool operator()(Scalar::Field* pField1_,
						Scalar::Field* pField2_) const
		{
			return (pField1_->isFunction()
					&& (pField2_->isFunction()
						&& pField1_->getID() < pField2_->getID()))
				|| (!pField1_->isFunction()
					&& (pField2_->isFunction()
						|| (!pField2_->isFunction()
							&& (pField1_->getSchemaField()->getPosition()
								< pField2_->getSchemaField()->getPosition()))));
		}
	};

	// FUNCTION local
	//	$$$::_addUndoLogElement -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	Opt::Environment& cEnvironment_
	//	Execution::Interface::IProgram& cProgram_
	//	Execution::Interface::IIterator* pIterator_
	//	Candidate::AdoptArgument& cArgument_
	//	Scalar::Field* pField_
	//	ModVector<unsigned int>& vecFieldID_
	//	VECTOR<int>& vecData_
	//	VECTOR<int>& vecLog_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_addUndoLogElement(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   Scalar::Field* pField_,
					   ModVector<unsigned int>& vecFieldID_,
					   VECTOR<int>& vecData_,
					   VECTOR<int>& vecLog_)
	{
		; _SYDNEY_ASSERT(pField_->isFunction() == false);
		vecFieldID_.pushBack(pField_->getSchemaField()->getID());
		vecData_.PUSHBACK(pField_->generate(cEnvironment_,
											cProgram_,
											pIterator_,
											cArgument_));
		Common::Data::Pointer pData =
			(pField_->isOperation()) ? new Common::DataArrayData() : pField_->getDataType().createData();
		if (pData.get()) {
			vecLog_.PUSHBACK(cProgram_.addVariable(pData));
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

///////////////////////////////////////
// Candidate::FileImpl::Normal::

// FUNCTION public
//	Candidate::FileImpl::Normal::setIsSimple -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
setIsSimple()
{
	m_cExplainArgument.m_bIsSimple = true;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::clearBitSetFlag -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
clearBitSetFlag()
{
	m_bGetByBitSet = false;
	if (getParameter()) {
		getParameter()->setIsGetByBitSet(false);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::check -- check file function availability
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	File::CheckArgument& cArgument_
//	boost::function<bool(LogicalFile::AutoLogicalFile&
//	LogicalFile::OpenOption&
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Normal::
check(Opt::Environment& cEnvironment_,
	  File::CheckArgument& cArgument_,
	  boost::function<bool(LogicalFile::AutoLogicalFile&,
						   LogicalFile::OpenOption&)> function_)
{
	if (getTable()->checkFile(cEnvironment_,
							  getFile(),
							  getParameter(),
							  cArgument_,
							  AccessPlan::Cost(),
							  function_)) {
		setParameter(cArgument_.m_pParameter);
		return true;
	}
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isAlwaysUsed -- check whether index is always used
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
FileImpl::Normal::
isAlwaysUsed()
{
	return getFile()->getSchemaFile()->getCategory() == Schema::File::Category::FullText;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isAlwaysBitSet -- check whether bitset is always needed
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
FileImpl::Normal::
isAlwaysBitSet()
{
	return getFile()->getSchemaFile()->getCategory() == Schema::File::Category::Bitmap
		&& !getTable()->isGetByBitSetRowID();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isCheckByKey -- check whether original value is not used in updating
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Normal::
isCheckByKey(Opt::Environment& cEnvironment_)
{
	return isLocatorUsed()
		&& (cEnvironment_.isRollback() == false
			|| isUseUndoUpdate(cEnvironment_));
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isLocatorUsed -- check whether locator is used
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
FileImpl::Normal::
isLocatorUsed()
{
	return getFile()->getSchemaFile()->getCategory() == Schema::File::Category::Lob;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isOtherFieldUsed -- 
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
FileImpl::Normal::
isOtherFieldUsed()
{
	return getField().getSize() > 1;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addCheckIndexResult -- check index availability
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	CheckIndexArgument& cArgument_
//	Predicate::ChosenInterface* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addCheckIndexResult(Opt::Environment& cEnvironment_,
					CheckIndexArgument& cArgument_,
					Predicate::ChosenInterface* pPredicate_)
{
	// if searchbybitset is not needed, cancel the flag
	if (cArgument_.m_bUseSearchBitSet == false) {
		m_bSearchByBitSet = false;
	}

	VECTOR<Predicate::ChosenInterface*>* pTarget =
		isAbleToGetByBitSet() ? &cArgument_.m_vecBitSet
		: (isSearchByBitSet() ? &cArgument_.m_vecSearchBitSet
		   : &cArgument_.m_vecIndexScan);

	if (pPredicate_->getType() == Tree::Node::Not) {
		pTarget->PUSHBACK(pPredicate_);

	} else {
		Super::insertCheckIndexResult(cEnvironment_,
									  pPredicate_,
									  pTarget,
									  this);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::getCost -- get cost of file access
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_ = 0
//	
// RETURN
//	AccessPlan::Cost&
//
// EXCEPTIONS

//virtual
AccessPlan::Cost&
FileImpl::Normal::
getCost(Opt::Environment& cEnvironment_,
		Interface::IPredicate* pPredicate_ /* = 0 */)
{
	double dblOverhead;
	double dblProcessCost;
	double dblCount = 0.0;
	AccessPlan::Cost::Value cEstimateRate = getParameter()->getCost().getRate();
	AccessPlan::Cost::Value cTableCount;

	if (getParameter()->getCost().isInfinity()) {
		double* pdblCount = &dblCount;

		if (pPredicate_) {
			cEstimateRate = pPredicate_->getEstimateRate();
			cTableCount = getTable()->getEstimateCount(cEnvironment_);
			if (cEstimateRate.isInfinity() == false) {
				// calculate from rate cache
				dblCount = cTableCount.get() * cEstimateRate.get();
				pdblCount = 0;
			}
		}
		LogicalFile::AutoLogicalFile& cLogicalFile =
			getFile()->attach(cEnvironment_.getTransaction());
		cLogicalFile.getCost(cEnvironment_.getTransaction(),
							 getParameter()->getOpenOption(),
							 dblOverhead,
							 dblProcessCost,
							 pdblCount);

		getParameter()->getCost().setOverhead(dblOverhead);
		getParameter()->getCost().setTotalCost(dblProcessCost * MAX(dblCount, 1.0));
		getParameter()->getCost().setTupleCount(dblCount);

		if (pPredicate_
			&& pPredicate_->isChosen()
			&& cEstimateRate.isInfinity()) {
			if (cLogicalFile.isAbleTo(LogicalFile::File::Capability::EstimateCount) == false) {
				// file driver can't estimate count
				AccessPlan::Cost cEstimateCost;
				cEstimateCost.setIsSetCount();
				cEstimateCost.setTableCount(cTableCount);

				pPredicate_->getChosen()->getEstimateCost(cEnvironment_, cEstimateCost);
				dblCount = cEstimateCost.getTupleCount().get();
			}
			cEstimateRate = cTableCount > 0 ? dblCount / cTableCount.get() : 1.0;
		}
	}
	if (pPredicate_
		&& getParameter()->getCost().getRate().isInfinity()) {

		dblOverhead = getParameter()->getCost().getOverhead().get();
		dblProcessCost = getParameter()->getCost().getProcessCost().get();
		dblCount = getParameter()->getCost().getTupleCount().get();
		cTableCount = getTable()->getEstimateCount(cEnvironment_);

		if (cEstimateRate.isInfinity()) {
			cEstimateRate = cTableCount > 0 ? dblCount / cTableCount.get() : 1.0;
		}

		if (isGetByBitSetAvailable()
			&& pPredicate_->isFetch() == false
			&& getParameter()->getOrder() == 0
			&& getTable()->isNeedLock(cEnvironment_) == false) {
			dblOverhead +=
				(static_cast<double>(sizeof(unsigned int))
				 / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory))
				* dblCount;
			dblProcessCost = (static_cast<double>(sizeof(unsigned int))
							  / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory));
		}

		if (pPredicate_->isFetch()) {
			getParameter()->getCost().setIsFetch();
			getParameter()->getCost().setRate(1);
			// reduce count because index estimation has some ambiguities
			dblCount = MIN(dblCount,
						   MAX(10.0, dblCount / 100));
		} else {
			// set result to related objects as cache
			pPredicate_->setEstimateRate(cEstimateRate);
			// set rate
			getParameter()->getCost().setRate(cEstimateRate);
		}

		getParameter()->getCost().setOverhead(dblOverhead);
		getParameter()->getCost().setTotalCost(dblProcessCost * MAX(dblCount, 1.0));
		getParameter()->getCost().setTupleCount(dblCount);
		getParameter()->getCost().setTableCount(cTableCount);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "File cost(" << getFile()->getSchemaFile()->getName()
				   << "):" << getParameter()->getCost();
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif
	}
	return getParameter()->getCost();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addFieldForPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addFieldForPredicate(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Predicate::CheckRetrievableArgument& cCheckArgument_)
{
	addField(cEnvironment_,
			 cCheckArgument_.m_pField);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addField -- add retrieved fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_->getField(getFile());
	if (pField == 0
		|| (pField->isExpandElement()
			&& getFile()->getSchemaFile()->getCategory()
			!= Schema::File::Category::Bitmap)){
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	if (pField->isFunction() == false
		&& pField->getSchemaField()->isObjectID()
		&& m_cKey.isContaining(pField)) {
		// key objectid is not added
		;
	} else {
		pField->addField(getFile(),
						 m_cField);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addPutKey -- add put key fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addPutKey(Opt::Environment& cEnvironment_,
		  Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_->getField(getFile());
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_cKey.add(pField);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addInsertField -- add insert fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addInsertField(Opt::Environment& cEnvironment_,
			   Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_->getField(getFile());
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_cInsertField.add(pField);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addUndoField -- add undo fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
addUndoField(Opt::Environment& cEnvironment_,
			 Scalar::Field* pField_)
{
	Scalar::Field* pField = pField_->getField(getFile());
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_cUndoField.add(pField);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::checkIndex -- check index ability mainly about bitset
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::CheckIndexArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
checkIndex(Opt::Environment& cEnvironment_,
		   CheckIndexArgument& cArgument_)
{
	if (getTable()->getRowID()) {
		m_bSearchByBitSet = isSearchByBitSetAvailable();
		m_bGetByBitSet = (isGetByBitSetAvailable()
						  && (cArgument_.m_bIgnoreField
							  || m_cField.GETSIZE() <= 1));
	}
	cArgument_.m_cCheckedFile.add(getFile());
}

// FUNCTION public
//	Candidate::FileImpl::Normal::checkForUpdate -- check update situation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
setForUpdate(Opt::Environment& cEnvironment_)
{
	if (cEnvironment_.isUpdateTable(getTable()->getTable()->getSchemaTable())) {
		// remember index file for checking necessity of collection before updating 
		cEnvironment_.addIndexScan(getFile()->getSchemaFile());
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isGetByBitSetAvailable -- 
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
FileImpl::Normal::
isGetByBitSetAvailable()
{
	return getFile()->getSchemaFile()->isAbleToGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isSearchByBitSetAvailable -- 
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
FileImpl::Normal::
isSearchByBitSetAvailable()
{
	return getFile()->getSchemaFile()->isAbleToSearchByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isGetByBitSet -- check whether rowid is obtained by bitset
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

bool
FileImpl::Normal::
isGetByBitSet()
{
	return getParameter() && getParameter()->isGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::hasOrder -- check whether ordering is processed by file
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

bool
FileImpl::Normal::
hasOrder()
{
	return getParameter() && getParameter()->getOrder();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::hasOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
hasOrder(Order::Specification* pSpecification_)
{
	return hasOrder()
		&& Order::Specification::isCompatible(getParameter()->getOrder(),
											  pSpecification_);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isSearchable -- is searchable for a predicate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node* pNode_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isSearchable(Opt::Environment& cEnvironment_,
			 Tree::Node* pNode_)
{
	LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cEnvironment_.getTransaction());
	return cLogicalFile.getSearchParameter(pNode_,
										   getParameter()->getOpenOption());
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isNeedRetrieve -- 
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
FileImpl::Normal::
isNeedRetrieve()
{
	return !(getParameter() && getParameter()->getOrder()
			 && getParameter()->getOrder()->isBitSetSort());
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isRetrievable -- is retrievable for a field?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Scalar::Field* pField_)
{
	if (pFile_ == getFile()) {
		if (getFile()->getSchemaFile()->getCategory() != Schema::File::Category::Bitmap
			|| pField_->isRowID()
			|| hasOrder()) {
			// bitmap file can scan key only when it is used with group by
			return true;
		}
	}
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createFileAccess -- create fileaction object
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Execution::Action::FileAccess*
//
// EXCEPTIONS

Execution::Action::FileAccess*
FileImpl::Normal::
createFileAccess(Execution::Interface::IProgram& cProgram_)
{
	return
		Execution::Action::FileAccess::create(cProgram_,
											  getTable()->getTable()->getSchemaTable(),
											  getFile()->getSchemaFile(),
											  getParameter()->getOpenOption());
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createCheckAction -- create action checking predicate by index
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
createCheckAction(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Execution::Action::FileAccess* pFileAccess_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (isAbleToGetByBitSet()) {
		return createCheckByBitSet(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   pFileAccess_,
								   cArgument_,
								   cIndexArgument_);
	} else {
		return createCheckByCollection(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   pFileAccess_,
									   cArgument_,
									   cIndexArgument_);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createScan -- create filescan iterator using index
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
FileImpl::Normal::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// iBitSet is not negative value when any file has been defined as get bitset
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(getTable());

	// if bitset data is already prepared, use it as input bitset for searchbybitset
	int iSearchBitSetID = cElement.m_iSearchBitSetID;

	m_cExplainArgument.m_pTable = getTable()->getTable();
	m_cExplainArgument.m_pParameter = getParameter();

	if (m_bSearchByBitSet && iSearchBitSetID >= 0) {
		// set searchbybitset parameter
		setSearchByBitSet(cEnvironment_,
						  cProgram_,
						  iSearchBitSetID,
						  pFileAccess_);
	}

	if (getRankBitSetID() >= 0) {
		// set rankbybitset parameter
		setRankByBitSet(cEnvironment_,
						cProgram_,
						getRankBitSetID(),
						pFileAccess_);
	}

	int iDataID = -1;
	bool bNeedSort = false;
	if (isAbleToGetByBitSet()
		&& (isAlwaysBitSet() || cElement.m_bForceBitSet)) {
		// prepare bitset data if not yet
		int iBitSetID = cElement.prepareBitSetData(cProgram_);
		setGetByBitSet(cEnvironment_,
					   cProgram_,
					   pFileAccess_->getOpenOption());

		// set result data as a bitset
		VECTOR<int> vecData;
		vecData.PUSHBACK(iBitSetID);
		iDataID = cProgram_.addVariable(vecData);

		addLock(cEnvironment_,
				cProgram_,
				pFileAccess_,
				cArgument_,
				iBitSetID,
				true /* collection */);

	} else if (cElement.m_eOrder == Candidate::AdoptIndexArgument::Order::ByRowID) {
		// Some logical file can't sort by rowid (eg. FullText2)
		bNeedSort = !setOrderByRowID(cEnvironment_,
									 cProgram_,
									 pFileAccess_->getOpenOption());
	}

	m_cExplainArgument.setValues();

	Execution::Interface::IIterator* pFileIterator =
		Execution::Iterator::File::create(cProgram_,
										  pFileAccess_->getID(),
										  m_cExplainArgument);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pFileIterator->copyNodeVariable(cArgument_.m_pInput);
	}

	if (cEnvironment_.isRecovery() == false) {
		if (cArgument_.m_eTarget != Candidate::AdoptArgument::Target::Parallel
			&& isGetByBitSet() == false) {
			// add cancel poling
			pFileIterator->addAction(cProgram_,
									 _ACTION_ARGUMENT0_T(CheckCancel,
														 cArgument_.m_eTarget));
		}
	}

	Execution::Interface::IIterator* pResult = pFileIterator;

	if (iDataID < 0) {
		// set projection
		iDataID = setProjection(cEnvironment_,
								cProgram_,
								pFileIterator,
								cArgument_,
								pFileAccess_->getOpenOption());

		if (Scalar::Field* pRowID = getTable()->getRowID()) {
			if (getTable()->isGetByBitSetRowID()
				&& getTable()->getUsedField(cEnvironment_, pRowID)->isBitSetRowID()) {
				pRowID = getTable()->getUsedField(cEnvironment_, pRowID);
			}

			addLock(cEnvironment_,
					cProgram_,
					pFileAccess_,
					cArgument_,
					pRowID->generate(cEnvironment_,
									 cProgram_,
									 pFileIterator,
									 cArgument_),
					bNeedSort /* need collection if sorted */);
		}
		if (bNeedSort) {
			// insert sorting iterator
			VECTOR<Scalar::Field*> vecField;
			sortField(m_cField, vecField);
			VECTOR<Scalar::Field*>::ITERATOR found = Opt::Find(vecField.begin(),
															   vecField.end(),
															   boost::bind(&Scalar::Field::isRowID,
																		   _1));
			; _SYDNEY_ASSERT(found != vecField.end());
			VECTOR<int> vecKey(1, static_cast<int>(found - vecField.begin()));
			VECTOR<int> vecDirection(1, 0);

			Execution::Interface::ICollection* pSort =
				Execution::Collection::Sort::create(cProgram_,
													vecKey,
													vecDirection);
			pResult = Execution::Iterator::Filter::create(cProgram_,
														  pSort->getID());

			pFileIterator->addAction(cProgram_,
									 _ACTION_ARGUMENT1(OutData,
													   iDataID));
			pResult->addAction(cProgram_,
							   _ACTION_ARGUMENT(Input,
												pFileIterator->getID(),
												iDataID));
			pResult->copyNodeVariable(pFileIterator, true /* collection */);
		}
	}

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iDataID));

	if(cArgument_.m_bForceRowID) {
		pResult = addBitSetToRowIDFilter(cEnvironment_, cProgram_, pResult, cArgument_, iDataID);
	}
	return pResult;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createScanWithSearchByBitSetOption -- create filescan iterator using index
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
FileImpl::Normal::
createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getTable()->getRowID()) {
		m_bSearchByBitSet = getFile()->getSchemaFile()->isAbleToSearchByBitSet();
	
	}
	return createScan(cEnvironment_, cProgram_, pFileAccess_, cArgument_, cIndexArgument_);
}
// FUNCTION public
//	Candidate::FileImpl::Normal::createFetch -- create retrieve action by fetch
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Scalar::Field* pFetchKey_
//	int iFetchKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createFetch(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Scalar::Field* pFetchKey_,
			int iFetchKeyID_)
{
	if (isLocatorUsed()) {
		createGetLocator(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_,
						 iFetchKeyID_);
		return;
	}

	FileFetchMap::Iterator found = m_mapFetch.find(pIterator_);
	if (found == m_mapFetch.end()) {
		// create fetching operator
		Trans::Transaction& cTrans = cEnvironment_.getTransaction();
		LogicalFile::OpenOption cOpenOption;
		Tree::Fetch cTmpFetch(pFetchKey_);

		Relation::Table* pTable = getTable()->getTable();
		Relation::Table::AutoReset cAutoReset = pTable->setEstimateFile(getFile());
		LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cTrans);
		if (!cLogicalFile.getSearchParameter(&cTmpFetch, cOpenOption)) {
			// can't execute
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		Execution::Action::FileAccess* pFileAccess =
			Execution::Action::FileAccess::create(cProgram_,
												  pTable->getSchemaTable(),
												  getFile()->getSchemaFile(),
												  cOpenOption);
		Execution::Operator::FileFetch* pFileFetch =
			Execution::Operator::FileFetch::create(
								  cProgram_,
								  pIterator_,
								  pFileAccess->getID(),
								  iFetchKeyID_);

		// set projection parameter to file open option
		pFileFetch->setOutput(setProjection(cEnvironment_,
											cProgram_,
											pIterator_,
											cArgument_,
											pFileAccess->getOpenOption()));

		m_mapFetch.insert(pIterator_, pFileFetch);

		// add action
		pIterator_->addCalculation(cProgram_,
								   pFileFetch,
								   cArgument_.m_eTarget);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createGetLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
createGetLocator(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 int iKeyID_)
{
	FileFetchMap::Iterator found = m_mapFetch.find(pIterator_);
	if (found == m_mapFetch.end()) {

		// create fetching operator
		Trans::Transaction& cTrans = cEnvironment_.getTransaction();
		LogicalFile::OpenOption cOpenOption;
		Relation::Table* pTable = getTable()->getTable();

		cOpenOption.setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
							   (cArgument_.m_bForUpdate) ?
							   LogicalFile::OpenOption::OpenMode::Update
							   : LogicalFile::OpenOption::OpenMode::Read);

		Execution::Action::FileAccess* pFileAccess =
			Execution::Action::FileAccess::create(cProgram_,
												  pTable->getSchemaTable(),
												  getFile()->getSchemaFile(),
												  cOpenOption);
		// allocate locator data
		Execution::Action::Locator* pLocator =
			Execution::Action::Locator::create(cProgram_);

		Execution::Operator::FileFetch* pFileFetch =
			Execution::Operator::FileFetch::GetLocator::create(
								  cProgram_,
								  pIterator_,
								  pFileAccess->getID(),
								  iKeyID_,
								  pLocator->getID());

		// add ID->locator relationship to gotten fields
		m_cField.foreachElement(boost::bind(&Scalar::Field::addLocator,
											_1,
											boost::ref(cEnvironment_),
											pIterator_,
											pLocator->getID()));

		m_mapFetch.insert(pIterator_, pFileFetch);

		// add action
		pIterator_->addCalculation(cProgram_,
								   pFileFetch,
								   cArgument_.m_eTarget);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createInsert -- create insert action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createInsert(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	// create fileaccess object for insert
	Execution::Action::FileAccess* pFileAccess = createFileAccess(cProgram_);

	if (isBatchMode(cEnvironment_, cArgument_)) {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Batch);
	} else {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Update);
	}

	if (cEnvironment_.isRecovery() == false
		&& cArgument_.m_eTarget != Candidate::AdoptArgument::Target::Parallel) {
		// add cancel poling
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(CheckCancel,
												  cArgument_.m_eTarget));
	}

	// create data for checking skip condition if needed
	Candidate::AdoptArgument cPostArgument(cArgument_);
	cPostArgument.m_bSkipCheck = true;
	int iSkipCheckID = createSkipCheck(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cPostArgument);

	if (iSkipCheckID >= 0) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(Unless,
												  iSkipCheckID,
												  cArgument_.m_eTarget));
	}

	// add insert action
	addInsert(cEnvironment_,
			  cProgram_,
			  pIterator_,
			  cArgument_,
			  pFileAccess);

	if (iSkipCheckID >= 0) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createExpunge -- create expunge action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createExpunge(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = (cEnvironment_.isUndo() == false); // take original

	// create fileaccess object for insert
	Execution::Action::FileAccess* pFileAccess = createFileAccess(cProgram_);

	if (isBatchMode(cEnvironment_, cArgument_)) {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Batch);
	} else {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Update);
	}

	if (cEnvironment_.isRecovery() == false
		&& cArgument_.m_eTarget != Candidate::AdoptArgument::Target::Parallel) {
		// add cancel poling
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(CheckCancel,
												  cArgument_.m_eTarget));
	}

	// create data for checking skip condition if needed
	int iSkipCheckID = createSkipCheck(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cPrevArgument);

	if (iSkipCheckID >= 0) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(Unless,
												  iSkipCheckID,
												  cArgument_.m_eTarget));
	}

	addExpunge(cEnvironment_,
			   cProgram_,
			   pIterator_,
			   cArgument_,
			   pFileAccess);

	if (iSkipCheckID >= 0) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createUpdate -- create update action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createUpdate(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	// create fileaccess object for insert
	Execution::Action::FileAccess* pFileAccess = createFileAccess(cProgram_);

	if (isBatchMode(cEnvironment_, cArgument_)) {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Batch);
	} else {
		pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
												LogicalFile::OpenOption::OpenMode::Update);
	}

	if (cEnvironment_.isRecovery() == false
		&& cArgument_.m_eTarget != Candidate::AdoptArgument::Target::Parallel) {
		// add cancel poling
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(CheckCancel,
												  cArgument_.m_eTarget));
	}

	// create data for checking skip condition if needed
	Candidate::AdoptArgument cPostArgument(cArgument_);
	cPostArgument.m_bSkipCheck = true;
	int iSkipCheckID = createSkipCheck(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cPostArgument);

	if (iSkipCheckID < 0) {
		// no check
		addUpdate(cEnvironment_,
				  cProgram_,
				  pIterator_,
				  cArgument_,
				  pFileAccess);

	} else {
		// create data for checking skip condition by previous data
		Candidate::AdoptArgument cPrevArgument(cArgument_);
		cPrevArgument.m_bOriginal = true; // take original
		int iPrevSkipCheckID = createSkipCheck(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cPrevArgument);
		; _SYDNEY_ASSERT(iPrevSkipCheckID >= 0);

		// generate necessary fields here
		if (!isUseOperation()) {
			m_cInsertField.foreachElement(boost::bind(&Scalar::Field::generate,
													  _1,
													  boost::ref(cEnvironment_),	 
													  boost::ref(cProgram_),
													  pIterator_,
													  boost::ref(cArgument_)));
			m_cInsertField.foreachElement(boost::bind(&Scalar::Field::generate,
													  _1,
													  boost::ref(cEnvironment_),	 
													  boost::ref(cProgram_),
													  pIterator_,
													  boost::ref(cPrevArgument)));
		}
		m_cField.foreachElement(boost::bind(&Scalar::Field::generate,
											_1,
											boost::ref(cEnvironment_),	 
											boost::ref(cProgram_),
											pIterator_,
											boost::ref(cArgument_)));
		m_cField.foreachElement(boost::bind(&Scalar::Field::generate,
											_1,
											boost::ref(cEnvironment_),	 
											boost::ref(cProgram_),
											pIterator_,
											boost::ref(cPrevArgument)));
		m_cKey.foreachElement(boost::bind(&Scalar::Field::generate,
										  _1,
										  boost::ref(cEnvironment_),	 
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_)));
		m_cKey.foreachElement(boost::bind(&Scalar::Field::generate,
										  _1,
										  boost::ref(cEnvironment_),	 
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cPrevArgument)));

		////////////////////////////
		// if (prev is not inserted)
		////////////////////////////
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(If,
												  iPrevSkipCheckID,
												  cArgument_.m_eTarget));
		{
			/////////////////////////////////
			// if (now is inserted)
			/////////////////////////////////
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(Unless,
													  iSkipCheckID,
													  cArgument_.m_eTarget));
			{
				// prev is not inserted, now is inserted
				// -> insert to the file
				addInsert(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cArgument_,
						  pFileAccess);
			}
			/////////////////////////////////
			// else -> do nothing
			// end if (now is inserted)
			/////////////////////////////////
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
		}
		/////////////////////////////////
		// else if (prev is inserted)
		/////////////////////////////////
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(Unless,
												  iPrevSkipCheckID,
												  cArgument_.m_eTarget));
		{
			/////////////////////////////////
			// if (now is not inserted)
			/////////////////////////////////
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(If,
													  iSkipCheckID,
													  cArgument_.m_eTarget));
			{
				// prev is inserted, now is not inserted
				// -> expunge from the file
				addExpunge(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cArgument_,
						   pFileAccess);
			}
			/////////////////////////////////
			// else if (now is inserted)
			/////////////////////////////////
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(Unless,
													  iSkipCheckID,
													  cArgument_.m_eTarget));
			{
				// prev is inserted, now is also inserted
				// -> update the file
				addUpdate(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cArgument_,
						  pFileAccess);
			}
			////////////////////////////////////
			// end if (now is inserted)
			////////////////////////////////////
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
		}
		////////////////////////////////////
		// end if (prev is inserted)
		////////////////////////////////////
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createUndoExpunge -- create undo expunge action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createUndoExpunge(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	; _SYDNEY_ASSERT(cEnvironment_.isRollback());

	// create fileaccess object for insert
	Execution::Action::FileAccess* pFileAccess = createFileAccess(cProgram_);

	pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
											LogicalFile::OpenOption::OpenMode::Update);
	addUndoExpunge(cEnvironment_,
				   cProgram_,
				   pIterator_,
				   cArgument_,
				   pFileAccess);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::createUndoUpdate -- create undo update action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
createUndoUpdate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	; _SYDNEY_ASSERT(cEnvironment_.isRollback());

	// create fileaccess object for insert
	Execution::Action::FileAccess* pFileAccess = createFileAccess(cProgram_);

	pFileAccess->getOpenOption().setInteger(LogicalFile::OpenOption::KeyNumber::OpenMode,
											LogicalFile::OpenOption::OpenMode::Update);
	addUndoUpdate(cEnvironment_,
				  cProgram_,
				  pIterator_,
				  cArgument_,
				  pFileAccess);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addUnionFileScan -- add union action by filescan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addUnionFileScan(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 Execution::Action::FileAccess* pFileAccess_)
{
	if (getTable()->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	addField(cEnvironment_,
			 getTable()->getRowID());

	// create scan iterator
	Candidate::AdoptIndexArgument cIndexArgument;
	Execution::Interface::IIterator* pFileIterator =
		createScan(cEnvironment_, cProgram_, pFileAccess_, cArgument_, cIndexArgument);

	int iRowIDID = getTable()->getRowID()->generate(cEnvironment_,
													cProgram_,
													pFileIterator,
													cArgument_);

	addLock(cEnvironment_,
			cProgram_,
			pFileAccess_,
			cArgument_,
			iRowIDID,
			false /* not collection */);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT_T(Input,
											 pFileIterator->getID(),
											 iRowIDID,
											 cArgument_.m_eTarget));
}

// FUNCTION public
//	Candidate::FileImpl::Normal::checkPartitionBy -- check partition by
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
checkPartitionBy(Opt::Environment& cEnvironment,
				 Order::Specification* pSpecification_)
{
	bool bResult = false;
	const VECTOR<Interface::IScalar*>& vecPartitionKey = pSpecification_->getPartitionKey();

	if (vecPartitionKey.GETSIZE() == 1
		&& vecPartitionKey[0]->isField()) {
		Scalar::Field* pField = vecPartitionKey[0]->getField();
		if (pField
			&& pField->isFunction()
			&& pField->getFunction()->getType() == Tree::Node::ClusterID) {
			// if key is score and ordering is desc, it is supported
			if (pSpecification_->getKeySize() == 1
				&& pSpecification_->getKey(0)->getScalar()->isField()
				&& pSpecification_->getKey(0)->getDirection() == Order::Direction::Descending) {
				Scalar::Field* pKey = pSpecification_->getKey(0)->getScalar()->getField();
				if (pKey
					&& pKey->isFunction()
					&& pKey->getFunction()->getType() == Tree::Node::Score) {
					bResult = true;
				}
			}
		}
	}
	return bResult;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::checkLimit -- check limit
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	// check whether scan file can process limit
	Common::IntegerArrayData cValue;
	CheckArgument cArgument(getTable());
	cArgument.noEstimate();
	if (cLimit_.getValue(cEnvironment_, cValue)
		&& check(cEnvironment_,
				 cArgument,
				 boost::bind(&LogicalFile::AutoLogicalFile::getLimitParameter,
							 _1,
							 boost::cref(cValue),
							 _2))) {
		// limit specification is processed by file
		getParameter()->setIsLimited(true);
		return true;
	}
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isUseUndoExpunge -- use undoexpunge?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isUseUndoExpunge(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isRollback()
		&& useUndo()
		&& m_cKey.isEmpty() == false;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isUseUndoUpdate -- use undoupdate?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isUseUndoUpdate(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.isRollback()
		&& useUndo()
		&& m_cField.isEmpty();
}

// FUNCTION public
//	Candidate::FileImpl::Normal::isUseOperation -- 
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
FileImpl::Normal::
isUseOperation()
{
	return isLocatorUsed()
		&& m_cField.isAny(boost::bind(&Scalar::Field::isOperation, _1));
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addBitSetToRowIDFilter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
FileImpl::Normal::
addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   int iInDataID_)
{

	Execution::Collection::BitsetDisintegration* pBitsetDisintegration =
		Execution::Collection::BitsetDisintegration::create(cProgram_);
	
	Execution::Interface::IIterator* pBitsetFilter =
		Execution::Iterator::Filter::create(cProgram_,
											pBitsetDisintegration->getID());

	Scalar::Field* pField = 0;
	Candidate::Row* pRow = getTable()->getKey(cEnvironment_);
	if (pRow->getSize() == 1
		&& (*pRow->begin())->isField()) {
		pField = (*pRow->begin())->getField();
		if(!pField->isRowID()) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	
	VECTOR<Scalar::Field*> vecField;
	sortField(m_cField, vecField);
	VECTOR<Scalar::Field*>::ITERATOR found = Opt::Find(vecField.begin(),
													   vecField.end(),
													   boost::bind(&Scalar::Field::isRowID,
																   _1));
	if (found == vecField.end()) _SYDNEY_THROW0(Exception::Unexpected);
	
	// BitSetFieldのRowIDを削除し、unsigned shortのRowIDを追加する
	found = vecField.insert(found, pField);
	vecField.erase(++found);

	int iOutID = generateField(cEnvironment_,
							   cProgram_,
							   pBitsetFilter,
							   cArgument_,
							   vecField);

	pBitsetFilter->addAction(cProgram_,
							 _ACTION_ARGUMENT1(OutData,
											   iOutID));	

	pBitsetFilter->addAction(cProgram_,
							 _ACTION_ARGUMENT(Input,
											  pIterator_->getID(),
											  iInDataID_));
	return pBitsetFilter;
}

// FUNCTION public
//	Candidate::FileImpl::Normal::generatePutField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Normal::
generatePutField(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = true; // take original

	// generate (put data)
	if (!isUseOperation()) {
		m_cInsertField.foreachElement(boost::bind(&Scalar::Field::generate,
												  _1,
												  boost::ref(cEnvironment_),	 
												  boost::ref(cProgram_),
												  pIterator_,
												  boost::ref(cArgument_)));
	}
	m_cField.foreachElement(boost::bind(&Scalar::Field::generate,
										_1,
										boost::ref(cEnvironment_),	 
										boost::ref(cProgram_),
										pIterator_,
										boost::ref(cArgument_)));
	m_cKey.foreachElement(boost::bind(&Scalar::Field::generate,
									  _1,
									  boost::ref(cEnvironment_),	 
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument_)));
	// generate (original data)
	if (!isUseOperation()) {
		m_cInsertField.foreachElement(boost::bind(&Scalar::Field::generate,
												  _1,
												  boost::ref(cEnvironment_),	 
												  boost::ref(cProgram_),
												  pIterator_,
												  boost::ref(cPrevArgument)));
	}
	m_cField.foreachElement(boost::bind(&Scalar::Field::generate,
										_1,
										boost::ref(cEnvironment_),	 
										boost::ref(cProgram_),
										pIterator_,
										boost::ref(cPrevArgument)));
	m_cKey.foreachElement(boost::bind(&Scalar::Field::generate,
									  _1,
									  boost::ref(cEnvironment_),	 
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cPrevArgument)));
}

// FUNCTION public
//	Candidate::FileImpl::Normal::getPredicateInterface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Predicate::ChosenInterface*
//
// EXCEPTIONS

//virtual
Predicate::ChosenInterface*
FileImpl::Normal::
getPredicateInterface()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::createCheckByBitSet -- create check by index (getbybitset)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
createCheckByBitSet(Opt::Environment& cEnvironment_,
					Execution::Interface::IProgram& cProgram_,
					Execution::Interface::IIterator* pIterator_,
					Execution::Action::FileAccess* pFileAccess_,
					Candidate::AdoptArgument& cArgument_,
					Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getTable()->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(getTable());
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(getTable());
	cMyElement.m_bForceBitSet = true;
	cMyElement.m_iPrevBitSetID = cElement.m_iPrevBitSetID;
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	Execution::Interface::IIterator* pGetBitSet =
		createScan(cEnvironment_,
				   cProgram_,
				   pFileAccess_,
				   cArgument_,
				   cMyIndexArgument);
	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	int iRowIDID = getTable()->getRowID()->generate(cEnvironment_,
												  cProgram_,
												  pIterator_,
												  cArgument_);

	Execution::Predicate::RowIDCheck* pCheck =
		Execution::Predicate::RowIDCheck::ByBitSet::create(
								  cProgram_,
								  pIterator_,
								  pGetBitSet->getID(),
								  iRowIDID,
								  cMyElement.m_iBitSetID,
								  cMyElement.m_iPrevBitSetID);

	return pCheck->getID();
}

// FUNCTION private
//	Candidate::FileImpl::Normal::createCheckByCollection -- add restriction by index (not getbybitset)
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
createCheckByCollection(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						Execution::Action::FileAccess* pFileAccess_,
						Candidate::AdoptArgument& cArgument_,
						Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getTable()->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(getTable());

	addField(cEnvironment_,
			 getTable()->getRowID());

	Execution::Interface::IIterator* pIterator =
		createScan(cEnvironment_,
				   cProgram_,
				   pFileAccess_,
				   cArgument_,
				   cIndexArgument_);

	// rowid data to be checked

	// in scan iterator
	int iInRowIDID = getTable()->getRowID()->generate(cEnvironment_,
													cProgram_,
													pIterator,
													cArgument_);
	// in outer iterator
	int iRowIDID = getTable()->getRowID()->generate(cEnvironment_,
												  cProgram_,
												  pIterator_,
												  cArgument_);
	// create data to get result
	VECTOR<Scalar::Field*> vecField;
	sortField(m_cField, vecField);

	// in scan iterator
	int iInDataID = pIterator->getOutData(cProgram_);
	; _SYDNEY_ASSERT(iInDataID >= 0);

	// in outer iterator
	int iDataID = generateField(cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								vecField);

	// store collection
	Execution::Interface::ICollection* pStore =
		Execution::Collection::Store::create(cProgram_);

	Execution::Predicate::RowIDCheck* pCheck =
		Execution::Predicate::RowIDCheck::ByCollection::create(
									  cProgram_,
									  pIterator_,
									  pIterator->getID(),
									  MAKEPAIR(iRowIDID, iInRowIDID),
									  MAKEPAIR(iDataID, iInDataID),
									  cElement.m_iPrevBitSetID,
									  pStore->getID());

	return pCheck->getID();
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addLock -- create lock action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	bool bForceCollection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addLock(Opt::Environment& cEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Execution::Action::FileAccess* pFileAccess_,
		Candidate::AdoptArgument& cArgument_,
		int iDataID_,
		bool bForceCollection_)
{
	Execution::Action::Locker::Argument cLockerArgument;
	if (Utility::Transaction::Locker::createArgument(cEnvironment_,
													 cArgument_,
													 getTable()->getTable()->getSchemaTable(),
													 bForceCollection_,
													 cLockerArgument)) {
		pFileAccess_->addLock(cProgram_,
							  cLockerArgument,
							  iDataID_);
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::createSkipCheck -- create skip check
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
createSkipCheck(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	int iResult = -1;

	if (cEnvironment_.isUndo() == false) {
		VECTOR<Scalar::Field*> vecChecked;
		bool bCheckByAny = false;
		if (isCheckByKey(cEnvironment_) == false) {
			vecChecked = getFile()->getSkipCheckKey(cEnvironment_,
													getTable()->getTable());
		} else if (cEnvironment_.isRollback() == false
				   && cArgument_.m_bOriginal == false) {
			// target is post value
			if (isUseOperation() == false) {
				vecChecked = getFile()->getSkipCheckKey(cEnvironment_,
														getTable()->getTable());
			} else {
				sortField(m_cField, vecChecked);
				bCheckByAny = true;
			}
		} else {
			; _SYDNEY_ASSERT(m_cKey.isEmpty() == false);
			sortField(m_cKey, vecChecked);
		}

		if (vecChecked.ISEMPTY() == false) {
			VECTOR<int> vecData;
			Opt::MapContainer(vecChecked,
							  vecData,
							  boost::bind(&Scalar::Field::generate,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_)));
			Execution::Interface::IAction* pCheck =
				bCheckByAny ?
				Execution::Predicate::Comparison::AnyElement::create(
											 cProgram_,
											 pIterator_,
											 vecData,
											 Execution::Predicate::Comparison::Type::IsNull,
											 true /* cascade */,
											 true /* check is array */)
				: Execution::Predicate::Comparison::AllElement::create(
											 cProgram_,
											 pIterator_,
											 vecData,
											 Execution::Predicate::Comparison::Type::IsNull,
											 (getFile()->getSchemaFile()->getSkipInsertType()
											  != Schema::File::SkipInsertType::ValueIsNull));
			iResult = pCheck->getID();
		}
	}
	return iResult;
}

// FUNCTION private
//	Candidate::FileImpl::Normal::sortField -- sort fields by position
//
// NOTES
//
// ARGUMENTS
//	const Utility::FieldSet& cFieldSet_
//	VECTOR<Scalar::Field*>& vecResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
sortField(const Utility::FieldSet& cFieldSet_,
		  VECTOR<Scalar::Field*>& vecResult_)
{
	vecResult_.assign(cFieldSet_.begin(), cFieldSet_.end());
	SORT(vecResult_.begin(), vecResult_.end(), _ByPosition());
}

// FUNCTION private
//	Candidate::FileImpl::Normal::generateField -- create retrieving fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const VECTOR<Scalar::Field*>& vecField_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
generateField(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_,
			  const VECTOR<Scalar::Field*>& vecField_)
{
	VECTOR<int> vecData;
	Opt::MapContainer(vecField_,
					  vecData,
					  boost::bind(&Scalar::Field::generate,
								  _1,
								  boost::ref(cEnvironment_),	 
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_)));

	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::isBatchMode -- is batch mode?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isBatchMode(Opt::Environment& cEnvironment_,
			Candidate::AdoptArgument& cArgument_)
{
	return cEnvironment_.getTransaction().isBatchMode() || cArgument_.m_bBatch;
}

// FUNCTION private
//	Candidate::FileImpl::Normal::setGetByBitSet -- set getbybitset parameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IPrgoram& cProgram_
//	LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
setGetByBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   LogicalFile::OpenOption& cOpenOption_)
{
	; _SYDNEY_ASSERT(isAbleToGetByBitSet());
	; _SYDNEY_ASSERT(getTable()->getRowID());

	// set getbybitset flag to openoption
	cOpenOption_.setBoolean(LogicalFile::OpenOption::KeyNumber::GetByBitSet, true);

	// set projection
	Relation::Table::AutoReset cAutoReset = getTable()->getTable()->setEstimateFile(getFile());
	LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cEnvironment_.getTransaction());
	if (!cLogicalFile.getProjectionParameter(getTable()->getRowID(), cOpenOption_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	getParameter()->setIsGetByBitSet(true);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::setSearchByBitSet -- set searchbybitset parameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	int iPrevBitSet_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
setSearchByBitSet(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  int iPrevBitSet_,
				  Execution::Action::FileAccess* pFileAccess_)
{
	if (iPrevBitSet_ >= 0) {
		pFileAccess_->setSearchByBitSet(iPrevBitSet_);
		getParameter()->setIsSearchByBitSet(true);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Normal::setRankByBitSet -- set rankbybitset parameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	int iBitSet_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
setRankByBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				int iBitSet_,
				Execution::Action::FileAccess* pFileAccess_)
{
	if (iBitSet_ >= 0) {
		pFileAccess_->setRankByBitSet(iBitSet_);
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::setOrderByRowID -- set order
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
setOrderByRowID(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				LogicalFile::OpenOption& cOpenOption_)
{
	if (getTable()->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	if (getParameter()->getOrder()) {
		// already sorted
		return false;
	}
	Order::Key* pKey = Order::Key::create(cEnvironment_,
										  getTable()->getRowID(),
										  Order::Direction::Ascending);
	Order::Specification* pSpecification =
		Order::Specification::create(cEnvironment_,
									 pKey);

	// set sort parameter
	Relation::Table::AutoReset cAutoReset = getTable()->getTable()->setEstimateFile(getFile());
	LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cEnvironment_.getTransaction());
	return cLogicalFile.getSortParameter(pSpecification,
										 cOpenOption_);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::setProjection -- set projection parameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
FileImpl::Normal::
setProjection(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_,
			  LogicalFile::OpenOption& cOpenOption_)
{
	if (m_cField.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort field by position
	VECTOR<Scalar::Field*> vecField;
	sortField(m_cField, vecField);

	// create row
	VECTOR<Interface::IScalar*> vecOperand;
	Opt::MapContainer(vecField,
					  vecOperand,
					  boost::bind(&Scalar::Field::getOriginal,
								  _1));
	Interface::IRow* pRow = Interface::IRow::create(cEnvironment_,
													vecOperand);
	// set projection parameter
	Relation::Table::AutoReset cAutoReset = getTable()->getTable()->setEstimateFile(getFile());
	LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cEnvironment_.getTransaction());
	if (!cLogicalFile.getProjectionParameter(pRow, cOpenOption_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// create data to get result
	return generateField(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_,
						 vecField);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::setUpdate -- set update parameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const VECTOR<Scalar::Field*>& vecField_
//	LogicalFile::OpenOption& cOpenOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
setUpdate(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Execution::Interface::IIterator* pIterator_,
		  Candidate::AdoptArgument& cArgument_,
		  const VECTOR<Scalar::Field*>& vecField_,
		  LogicalFile::OpenOption& cOpenOption_)
{
	// create position array
	ModVector<int> vecProjection;
	Opt::MapContainer(vecField_,
					  vecProjection,
					  boost::bind(&Schema::Field::getPosition,
								  boost::bind(&Scalar::Field::getSchemaField,
											  _1)));
	// set update parameter
	LogicalFile::AutoLogicalFile& cLogicalFile = getFile()->attach(cEnvironment_.getTransaction());
	if (!cLogicalFile.getUpdateParameter(Common::IntegerArrayData(vecProjection), cOpenOption_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addInsert -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addInsert(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Execution::Interface::IIterator* pIterator_,
		  Candidate::AdoptArgument& cArgument_,
		  Execution::Action::FileAccess* pFileAccess_)
{
	if (isUseUndoExpunge(cEnvironment_)) {
		// use undoExpunge instead
		addUndoExpunge(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   cArgument_,
					   pFileAccess_);
		return;
	}
	if (isUseOperation()) {
		// operationが値にあったらinsertしない
		return;
	}

	if (m_cInsertField.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort field by position
	VECTOR<Scalar::Field*> vecField;
	sortField(m_cInsertField, vecField);

	// create inserted data
	int iDataID = generateField(cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								vecField);

	if (isNeedLog(cEnvironment_)) {
		// prepare undolog
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::UndoLog::Prepare::create(cProgram_, pIterator_),
								   cArgument_.m_eTarget);
	}

	// add insert action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::FileOperation::Insert::create(
									  cProgram_,
									  pIterator_,
									  pFileAccess_->getID(),
									  iDataID),
							   cArgument_.m_eTarget);

	if (isNeedLog(cEnvironment_)) {
		// pick up put-key
		VECTOR<Scalar::Field*> vecPutKey;
		sortField(m_cKey, vecPutKey);

		// add store undolog action
		Insert::addUndoLog(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   this,
						   vecPutKey,
						   cArgument_);
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addUpdate -- add update action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addUpdate(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Execution::Interface::IIterator* pIterator_,
		  Candidate::AdoptArgument& cArgument_,
		  Execution::Action::FileAccess* pFileAccess_)
{
	if (isUseUndoUpdate(cEnvironment_)) {
		// use undoUpdate instead
		addUndoUpdate(cEnvironment_,
					  cProgram_,
					  pIterator_,
					  cArgument_,
					  pFileAccess_);
		return;
	}

	if (m_cField.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (isUseOperation()) {
		// operationが値にあったらoperationを使う
		addOperationUpdate(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cArgument_,
						   pFileAccess_);
		return;
	}

	// sort key and field by position
	VECTOR<Scalar::Field*> vecField;
	VECTOR<Scalar::Field*> vecKey;
	sortField(m_cField, vecField);
	sortField(m_cKey, vecKey);

	// set update parameter
	setUpdate(cEnvironment_,
			  cProgram_,
			  pIterator_,
			  cArgument_,
			  vecField,
			  pFileAccess_->getOpenOption());

	// candidate argument to get original data
	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = (cEnvironment_.isUndo() == false);

	// check value change for files
	bool bCheckDistinct = (cEnvironment_.isUndo() == false
						   && isCheckByKey(cEnvironment_) == false);

	// create update data and key
	int iDataID = generateField(cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								vecField);
	if (bCheckDistinct) {
		// create original data
		int iPrevDataID = generateField(cEnvironment_,
										cProgram_,
										pIterator_,
										cPrevArgument,
										vecField);
		// add check distinct predicate
		Execution::Interface::IAction* pCheck =
			Execution::Predicate::Comparison::create(cProgram_,
													 pIterator_,
													 iDataID,
													 iPrevDataID,
													 Execution::Predicate::Comparison::Type::IsDistinct);
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(If,
												  pCheck->getID(),
												  cArgument_.m_eTarget));
	}

	if (isNeedLog(cEnvironment_)) {
		// prepare undolog
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::UndoLog::Prepare::create(cProgram_, pIterator_),
								   cArgument_.m_eTarget);
	}

	// for creating key, use original data flag
	int iKeyID = generateField(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cPrevArgument,
							   vecKey);

	// add update action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::FileOperation::Update::create(
									  cProgram_,
									  pIterator_,
									  pFileAccess_->getID(),
									  iKeyID,
									  iDataID),
							   cArgument_.m_eTarget);

	if (isNeedLog(cEnvironment_)) {
		// add store undolog action
		if (useUndo()) {
			UndoUpdate::addUndoLog(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   this,
								   vecKey,
								   cArgument_);
		} else {
			Update::addUndoLog(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   this,
							   vecKey,
							   vecField,
							   cArgument_);
		}
	}
	if (bCheckDistinct) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addExpunge -- add expunge action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addExpunge(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   Candidate::AdoptArgument& cArgument_,
		   Execution::Action::FileAccess* pFileAccess_)
{
	if (m_cKey.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort key and field by position
	VECTOR<Scalar::Field*> vecField;
	VECTOR<Scalar::Field*> vecKey;
	if (cEnvironment_.isRecovery() == false) {
		sortField(m_cUndoField, vecField);
	}
	sortField(m_cKey, vecKey);

	// candidate argument to get original data
	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = (cEnvironment_.isUndo() == false);

	// create original key
	int iKeyID = generateField(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cPrevArgument,
							   vecKey);

	if (isNeedLog(cEnvironment_)) {
		// prepare undolog
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::UndoLog::Prepare::create(cProgram_, pIterator_),
								   cArgument_.m_eTarget);
	}

	// add expunge action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::FileOperation::Expunge::create(
									  cProgram_,
									  pIterator_,
									  pFileAccess_->getID(),
									  iKeyID),
							   cArgument_.m_eTarget);

	if (isNeedLog(cEnvironment_)) {
		// add store undolog action
		if (useUndo()) {
			UndoExpunge::addUndoLog(cEnvironment_,
									cProgram_,
									pIterator_,
									this,
									vecKey,
									cArgument_);
		} else {
			Expunge::addUndoLog(cEnvironment_,
								cProgram_,
								pIterator_,
								this,
								vecField,
								cPrevArgument);
		}
	}
	if (vecKey.GETSIZE() == 1
		&& vecKey[0]->isObjectID()) {
		// set null to key
		int iPostKeyID = generateField(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cArgument_,
									   vecKey);
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::SetNull::create(
												cProgram_,
												pIterator_,
												iPostKeyID),
								   cArgument_.m_eTarget);
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addUndoUpdate -- add undo update action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addUndoUpdate(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_,
			  Execution::Action::FileAccess* pFileAccess_)
{
	if (m_cKey.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort key by position
	VECTOR<Scalar::Field*> vecKey;
	sortField(m_cKey, vecKey);

	// create key
	int iKeyID = generateField(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_,
							   vecKey);

	// add expunge action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::FileOperation::UndoUpdate::create(
									  cProgram_,
									  pIterator_,
									  pFileAccess_->getID(),
									  iKeyID),
							   cArgument_.m_eTarget);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::addUndoExpunge -- add undo expunge action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Normal::
addUndoExpunge(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Execution::Action::FileAccess* pFileAccess_)
{
	if (m_cKey.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort key by position
	VECTOR<Scalar::Field*> vecKey;
	sortField(m_cKey, vecKey);

	// create key
	int iKeyID = generateField(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_,
							   vecKey);

	// add expunge action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::FileOperation::UndoExpunge::create(
									  cProgram_,
									  pIterator_,
									  pFileAccess_->getID(),
									  iKeyID),
							   cArgument_.m_eTarget);
}

// FUNCTION public
//	Candidate::FileImpl::Normal::addOperationUpdate -- add operation update action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// add operation update action
void
FileImpl::Normal::
addOperationUpdate(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Execution::Interface::IIterator* pIterator_,
				   Candidate::AdoptArgument& cArgument_,
				   Execution::Action::FileAccess* pFileAccess_)
{
	if (m_cField.isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	if (!isUseOperation()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// sort field by position
	VECTOR<Scalar::Field*> vecKey;
	VECTOR<Scalar::Field*> vecField;
	sortField(m_cKey, vecKey);
	sortField(m_cField, vecField);

	Candidate::AdoptArgument cOperationArgument(cArgument_);
	cOperationArgument.m_bOperation = true;

	if (isNeedLog(cEnvironment_)) {
		// prepare undolog
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::UndoLog::Prepare::create(cProgram_, pIterator_),
								   cArgument_.m_eTarget);
	}

	if (cEnvironment_.isUndo()) {
		// locatorが作られていなかったら作る
		// for creating key, use original data flag
		Candidate::AdoptArgument cPrevArgument(cArgument_);
		cPrevArgument.m_bOriginal = false;
		; _SYDNEY_ASSERT(vecKey.GETSIZE() == 1);

		int iKeyID = (*vecKey.begin())->generate(cEnvironment_,
												 cProgram_,
												 pIterator_,
												 cPrevArgument);

		Candidate::AdoptArgument cPutArgument(cArgument_);
		cPutArgument.m_bForUpdate = true;
		createGetLocator(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cPutArgument,
						 iKeyID);
	}

	// create operation
	generateField(cEnvironment_,
				  cProgram_,
				  pIterator_,
				  cOperationArgument,
				  vecField);

	if (isNeedLog(cEnvironment_)) {
		// add store undolog action
		if (useUndo()) {
			UndoUpdate::addUndoLog(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   this,
								   vecKey,
								   cArgument_);
		} else {
			Update::addUndoLog(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   this,
							   vecKey,
							   vecField,
							   cArgument_);
		}
	}
}

// FUNCTION private
//	Candidate::FileImpl::Normal::isNeedLog -- undolog needed?
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
FileImpl::Normal::
isNeedLog(Opt::Environment& cEnvironment_)
{
	// UndoLog is necessary when transaction is set as necessary
	// even in recovery (for replication)
	return (cEnvironment_.getTransaction().isNecessaryLog() == true)
		&& (getTable()->getTable()->getSchemaTable()->isTemporary() == false);
}

// FUNCTION private
//	Candidate::FileImpl::Normal::useUndo -- use undoExpunge/undoUpdate?
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

bool
FileImpl::Normal::
useUndo()
{
	return getFile()->getSchemaFile()->isAbleToUndo()
		&& ( ! isUseOperation() );
}

////////////////////////////////////////
// FileImpl::Normal::Insert::
////////////////////////////////////////

// FUNCTION private
//	Candidate::FileImpl::Normal::Insert::addUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	File* pFile_
//	const VECTOR<Scalar::Field*>& vecPutKey_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FileImpl::Normal::Insert::
addUndoLog(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   File* pFile_,
		   const VECTOR<Scalar::Field*>& vecPutKey_,
		   Candidate::AdoptArgument& cArgument_)
{
	// generate put-key data
	ModVector<unsigned int> vecFieldID;
	VECTOR<int> vecData;
	VECTOR<int> vecLog;

	FOREACH(vecPutKey_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						_1,
						boost::ref(vecFieldID),
						boost::ref(vecData),
						boost::ref(vecLog)));

	////////////////////////////////////////////////////////////////////////////
	// Insert undolog: {EXPUNGE, <fileID>, {<key fieldIDs>}, {<key values>}}
	////////////////////////////////////////////////////////////////////////////

	// value storage is prepared apart
	int iDataID = cProgram_.addVariable(vecData);
	int iLogDataID = cProgram_.addVariable(vecLog);

	VECTOR<int> vecUndoLog;
	vecUndoLog.reserve(Opt::UndoLog::Format::SingleValueNum);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(Opt::UndoLog::Type::Expunge)));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  pFile_->getFile()->getSchemaFile()->getID())));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecFieldID)));
	vecUndoLog.PUSHBACK(iLogDataID);

	// add undolog action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::UndoLog::create(
									cProgram_,
									pIterator_,
									cProgram_.addVariable(vecUndoLog),
									iDataID,
									iLogDataID),
							   cArgument_.m_eTarget);
}

////////////////////////////////////////
// FileImpl::Normal::Expunge::
////////////////////////////////////////

// FUNCTION private
//	Candidate::FileImpl::Normal::Expunge::addUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	File* pFile_
//	const VECTOR<Scalar::Field*>& vecField_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FileImpl::Normal::Expunge::
addUndoLog(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   File* pFile_,
		   const VECTOR<Scalar::Field*>& vecField_,
		   Candidate::AdoptArgument& cArgument_)
{
	// generate put-key data
	ModVector<unsigned int> vecFieldID;
	VECTOR<int> vecData;
	VECTOR<int> vecLog;

	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = true;

	FOREACH(vecField_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cPrevArgument),
						_1,
						boost::ref(vecFieldID),
						boost::ref(vecData),
						boost::ref(vecLog)));

	////////////////////////////////////////////////////////////////////////////
	// Expunge undolog: {INSERT, <fileID>, {<fieldIDs>}, {<values>}}
	////////////////////////////////////////////////////////////////////////////

	// value storage is prepared apart
	int iDataID = cProgram_.addVariable(vecData);
	int iLogDataID = cProgram_.addVariable(vecLog);

	VECTOR<int> vecUndoLog;
	vecUndoLog.reserve(Opt::UndoLog::Format::SingleValueNum);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(Opt::UndoLog::Type::Insert)));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  pFile_->getFile()->getSchemaFile()->getID())));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecFieldID)));
	vecUndoLog.PUSHBACK(iLogDataID);

	// add undolog action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::UndoLog::create(
									 cProgram_,
									 pIterator_,
									 cProgram_.addVariable(vecUndoLog),
									 iDataID,
									 iLogDataID),
							   cArgument_.m_eTarget);
}

////////////////////////////////////////
// FileImpl::Normal::Update::
////////////////////////////////////////

// FUNCTION private
//	Candidate::FileImpl::Normal::Update::addUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	File* pFile_
//	const VECTOR<Scalar::Field*>& vecPutKey_
//	const VECTOR<Scalar::Field*>& vecField_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FileImpl::Normal::Update::
addUndoLog(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   File* pFile_,
		   const VECTOR<Scalar::Field*>& vecPutKey_,
		   const VECTOR<Scalar::Field*>& vecField_,
		   Candidate::AdoptArgument& cArgument_)
{
	// generate put-key data
	ModVector<unsigned int> vecKeyFieldID;
	ModVector<unsigned int> vecValueFieldID;
	VECTOR<int> vecKeyData;
	VECTOR<int> vecValueData;
	VECTOR<int> vecKeyLog;
	VECTOR<int> vecValueLog;

	// use original to undo data
	Candidate::AdoptArgument cPrevArgument(cArgument_);
	cPrevArgument.m_bOriginal = true;

	FOREACH(vecPutKey_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						_1,
						boost::ref(vecKeyFieldID),
						boost::ref(vecKeyData),
						boost::ref(vecKeyLog)));

	FOREACH(vecField_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cPrevArgument), // use original data
						_1,
						boost::ref(vecValueFieldID),
						boost::ref(vecValueData),
						boost::ref(vecValueLog)));

	////////////////////////////////////////////////////////////////////////////
	// Update undolog: {UPDATE, <fileID>, {<key fieldIDs>}, {<key values>},
	//												{<fieldIDs>}, {<values>}}
	////////////////////////////////////////////////////////////////////////////

	// value storage is prepared apart
	int iKeyDataID = cProgram_.addVariable(vecKeyData);
	int iKeyLogDataID = cProgram_.addVariable(vecKeyLog);
	int iValueDataID = cProgram_.addVariable(vecValueData);
	int iValueLogDataID = cProgram_.addVariable(vecValueLog);

	VECTOR<int> vecUndoLog;
	vecUndoLog.reserve(Opt::UndoLog::Format::DoubleValueNum);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(Opt::UndoLog::Type::Update)));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  pFile_->getFile()->getSchemaFile()->getID())));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecKeyFieldID)));
	vecUndoLog.PUSHBACK(iKeyLogDataID);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecValueFieldID)));
	vecUndoLog.PUSHBACK(iValueLogDataID);

	// add undolog action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::UndoLog::create(
									cProgram_,
									pIterator_,
									cProgram_.addVariable(vecUndoLog),
									iKeyDataID,
									iKeyLogDataID,
									iValueDataID,
									iValueLogDataID),
							   cArgument_.m_eTarget);
}

////////////////////////////////////////
// FileImpl::Normal::UndoExpunge::
////////////////////////////////////////

// FUNCTION private
//	Candidate::FileImpl::Normal::UndoExpunge::addUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	File* pFile_
//	const VECTOR<Scalar::Field*>& vecPutKey_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FileImpl::Normal::UndoExpunge::
addUndoLog(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   File* pFile_,
		   const VECTOR<Scalar::Field*>& vecPutKey_,
		   Candidate::AdoptArgument& cArgument_)
{
	// generate put-key data
	ModVector<unsigned int> vecFieldID;
	VECTOR<int> vecData;
	VECTOR<int> vecLog;

	FOREACH(vecPutKey_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						_1,
						boost::ref(vecFieldID),
						boost::ref(vecData),
						boost::ref(vecLog)));

	////////////////////////////////////////////////////////////////////////////
	// UndoExpunge undolog: {UNDOEXPUNGE, <fileID>, {<fieldIDs>}, {<values>}}
	////////////////////////////////////////////////////////////////////////////

	// value storage is prepared apart
	int iDataID = cProgram_.addVariable(vecData);
	int iLogDataID = cProgram_.addVariable(vecLog);

	VECTOR<int> vecUndoLog;
	vecUndoLog.reserve(Opt::UndoLog::Format::SingleValueNum);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(Opt::UndoLog::Type::UndoExpunge)));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  pFile_->getFile()->getSchemaFile()->getID())));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecFieldID)));
	vecUndoLog.PUSHBACK(iLogDataID);

	// add undolog action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::UndoLog::create(
									 cProgram_,
									 pIterator_,
									 cProgram_.addVariable(vecUndoLog),
									 iDataID,
									 iLogDataID),
							   cArgument_.m_eTarget);
}

////////////////////////////////////////
// FileImpl::Normal::UndoUpdate::
////////////////////////////////////////

// FUNCTION private
//	Candidate::FileImpl::Normal::UndoUpdate::addUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	File* pFile_
//	const VECTOR<Scalar::Field*>& vecPutKey_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
FileImpl::Normal::UndoUpdate::
addUndoLog(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   File* pFile_,
		   const VECTOR<Scalar::Field*>& vecPutKey_,
		   Candidate::AdoptArgument& cArgument_)
{
	// generate put-key data
	ModVector<unsigned int> vecFieldID;
	VECTOR<int> vecData;
	VECTOR<int> vecLog;

	FOREACH(vecPutKey_,
			boost::bind(&_addUndoLogElement,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						_1,
						boost::ref(vecFieldID),
						boost::ref(vecData),
						boost::ref(vecLog)));

	////////////////////////////////////////////////////////////////////////////
	// UndoUpdate undolog: {UNDOUPDATE, <fileID>, {<fieldIDs>}, {<values>}}
	////////////////////////////////////////////////////////////////////////////

	// value storage is prepared apart
	int iDataID = cProgram_.addVariable(vecData);
	int iLogDataID = cProgram_.addVariable(vecLog);

	VECTOR<int> vecUndoLog;
	vecUndoLog.reserve(Opt::UndoLog::Format::SingleValueNum);
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(Opt::UndoLog::Type::UndoUpdate)));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  pFile_->getFile()->getSchemaFile()->getID())));
	vecUndoLog.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecFieldID)));
	vecUndoLog.PUSHBACK(iLogDataID);

	// add undolog action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::UndoLog::create(
									cProgram_,
									pIterator_,
									cProgram_.addVariable(vecUndoLog),
									iDataID,
									iLogDataID),
							   cArgument_.m_eTarget);
}

//////////////////////////
// FileImpl::Variable::
//////////////////////////

// FUNCTION public
//	Candidate::FileImpl::Variable::setIsSimple -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
setIsSimple()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::clearBitSetFlag -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
clearBitSetFlag()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	File::CheckArgument& cArgument_
//	boost::function<bool(LogicalFile::AutoLogicalFile&
//	LogicalFile::OpenOption&
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
check(Opt::Environment& cEnvironment_,
	  File::CheckArgument& cArgument_,
	  boost::function<bool(LogicalFile::AutoLogicalFile&,
						   LogicalFile::OpenOption&)> function_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isAlwaysUsed -- 
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
FileImpl::Variable::
isAlwaysUsed()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isAlwaysBitSet -- 
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
FileImpl::Variable::
isAlwaysBitSet()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isCheckByKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
isCheckByKey(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isLocatorUsed -- 
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
FileImpl::Variable::
isLocatorUsed()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isOtherFieldUsed -- 
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
FileImpl::Variable::
isOtherFieldUsed()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addCheckIndexResult -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	CheckIndexArgument& cArgument_
//	Predicate::ChosenInterface* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addCheckIndexResult(Opt::Environment& cEnvironment_,
					CheckIndexArgument& cArgument_,
					Predicate::ChosenInterface* pPredicate_)
{

	VECTOR<Predicate::ChosenInterface*>* pTarget =
		isAbleToGetByBitSet() ? &cArgument_.m_vecBitSet
		: (isSearchByBitSet() ? &cArgument_.m_vecSearchBitSet
		   : &cArgument_.m_vecIndexScan);

	if (pPredicate_->getType() == Tree::Node::Not) {
		cArgument_.m_vecBitSet.PUSHBACK(pPredicate_);

	} else {
		Super::insertCheckIndexResult(cEnvironment_,
									  pPredicate_,
									  &cArgument_.m_vecBitSet,
									  this);
	}
}

// FUNCTION public
//	Candidate::FileImpl::Variable::getCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	AccessPlan::Cost&
//
// EXCEPTIONS

//virtual
AccessPlan::Cost&
FileImpl::Variable::
getCost(Opt::Environment& cEnvironment_,
		Interface::IPredicate* pPredicate_)
{
	if (m_cCost.isInfinity()) {
		m_cCost.setOverhead(0);
		m_cCost.setTupleSize(4);
		Server::Session::BitSetVariable* pSessionVariable =
			getSessionVariable(cEnvironment_);
		m_cCost.setTupleCount(pSessionVariable->getValue().count());
		m_cCost.setTotalCost((m_cCost.getTupleSize()
							  / AccessPlan::Cost::Value(
									LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory)))
							 * m_cCost.getTupleCount());
	}
	return m_cCost;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addFieldForPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addFieldForPredicate(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Predicate::CheckRetrievableArgument& cCheckArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addPutKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addPutKey(Opt::Environment& cEnvironment_,
		  Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addInsertField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addInsertField(Opt::Environment& cEnvironment_,
			   Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addUndoField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addUndoField(Opt::Environment& cEnvironment_,
			 Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	CheckIndexArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
checkIndex(Opt::Environment& cEnvironment_,
		   CheckIndexArgument& cArgument_)
{
	cArgument_.m_cCheckedFile.add(getFile());
}

// FUNCTION public
//	Candidate::FileImpl::Variable::setForUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
setForUpdate(Opt::Environment& cEnvironment_)
{
	; // do nothing
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isAbleToGetByBitSet -- 
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
FileImpl::Variable::
isAbleToGetByBitSet()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::disableGetByBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
disableGetByBitSet()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isSearchByBitSet -- 
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
FileImpl::Variable::
isSearchByBitSet()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isGetByBitSetAvailable -- 
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
FileImpl::Variable::
isGetByBitSetAvailable()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isSearchByBitSetAvailable -- 
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
FileImpl::Variable::
isSearchByBitSetAvailable()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::getRankBitSetID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FileImpl::Variable::
getRankBitSetID()
{
	return -1;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::setRankBitSetID -- 
//
// NOTES
//
// ARGUMENTS
//	int iRankBitSetID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
setRankBitSetID(int iRankBitSetID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isGetByBitSet -- 
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
FileImpl::Variable::
isGetByBitSet()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::hasOrder -- 
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
FileImpl::Variable::
hasOrder()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::hasOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
hasOrder(Order::Specification* pSpecification_)
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isSearchable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node* pNode_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
isSearchable(Opt::Environment& cEnvironment_,
			 Tree::Node* pNode_)
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isNeedRetrieve -- 
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

bool
FileImpl::Variable::
isNeedRetrieve()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isRetrievable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Scalar::Field* pField_)
{
	return (pFile_ == getFile()
			&& pField_->isRowID());
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createFileAccess -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Execution::Action::FileAccess*
//
// EXCEPTIONS

//virtual
Execution::Action::FileAccess*
FileImpl::Variable::
createFileAccess(Execution::Interface::IProgram& cProgram_)
{
	return 0;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createCheckAction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FileImpl::Variable::
createCheckAction(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Execution::Action::FileAccess* pFileAccess_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getTable()->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(getTable());

	Execution::Interface::IIterator* pGetBitSet =
		createScan(cEnvironment_,
				   cProgram_,
				   pFileAccess_,
				   cArgument_,
				   cMyIndexArgument);
	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	int iRowIDID = getTable()->getRowID()->generate(cEnvironment_,
												  cProgram_,
												  pIterator_,
												  cArgument_);

	Execution::Predicate::RowIDCheck* pCheck =
		Execution::Predicate::RowIDCheck::ByBitSet::create(
								  cProgram_,
								  pIterator_,
								  pGetBitSet->getID(),
								  iRowIDID,
								  cMyElement.m_iBitSetID,
								  cMyElement.m_iPrevBitSetID);

	return pCheck->getID();
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::Variable::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Server::Session::BitSetVariable* pSessionVariable = getSessionVariable(cEnvironment_);

	// iBitSet is not negative value when any file has been defined as get bitset
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(getTable());

	// prepare bitset data if not yet
	int iBitSetID = cElement.prepareBitSetData(cProgram_);

	// create iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Loop::Once::create(cProgram_);
	// add assign action
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Assign,
										cProgram_.addVariable(
											Common::Data::Pointer(
											  static_cast<const Common::Data*>(
												   &(pSessionVariable->getValue())))),
										iBitSetID));
	// add lock
	addLock(cEnvironment_,
			cProgram_,
			pResult,
			cArgument_,
			iBitSetID);

	return pResult;
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createScanWithSearchByBitSetOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::Variable::
createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Action::FileAccess* pFileAccess_,
								   Candidate::AdoptArgument& cArgument_,
								   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Scalar::Field* pFetchKey_
//	int iFetchKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createFetch(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Scalar::Field* pFetchKey_,
			int iFetchKeyID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createGetLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createGetLocator(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 int iKeyID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addUnionFileScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
addUnionFileScan(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 Execution::Action::FileAccess* pFileAccess_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createInsert -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createInsert(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createExpunge(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createUpdate(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createUndoExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createUndoExpunge(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::createUndoUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
createUndoUpdate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::checkPartitionBy -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
checkPartitionBy(Opt::Environment& cEnvironment,
				 Order::Specification* pSpecification_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::checkLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isUseUndoExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
isUseUndoExpunge(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isUseUndoUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Variable::
isUseUndoUpdate(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::isUseOperation -- 
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
FileImpl::Variable::
isUseOperation()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::addBitSetToRowIDFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::Variable::
addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   int iInDataID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::generatePutField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Variable::
generatePutField(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::Variable::getPredicateInterface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Predicate::ChosenInterface*
//
// EXCEPTIONS

//virtual
Predicate::ChosenInterface*
FileImpl::Variable::
getPredicateInterface()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Candidate::FileImpl::Variable::getSessionVariable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Server::Session::BitSetVariable*
//
// EXCEPTIONS

Server::Session::BitSetVariable*
FileImpl::Variable::
getSessionVariable(Opt::Environment& cEnvironment_)
{
	if (m_pSessionVariable == 0) {
		Server::Session* pSession =
			Server::Session::getSession(cEnvironment_.getTransaction().getSessionID());
		if (pSession == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pSessionVariable = pSession->getBitSetVariable(m_pVariable->getName());
		if (m_pSessionVariable == 0) {
			_SYDNEY_THROW1(Exception::VariableNotFound,
						   m_pVariable->getName());
		}
	}
	return m_pSessionVariable;
}

// FUNCTION private
//	Candidate::FileImpl::Variable::addLock -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Variable::
addLock(Opt::Environment& cEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Execution::Interface::IIterator* pIterator_,
		Candidate::AdoptArgument& cArgument_,
		int iDataID_)
{
	Execution::Action::Locker::Argument cLockerArgument;
	if (Utility::Transaction::Locker::createArgument(cEnvironment_,
													 cArgument_,
													 getTable()->getTable()->getSchemaTable(),
													 true, /* collection */
													 cLockerArgument)) {
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Locker::BitSet::create(cProgram_,
																			   pIterator_,
																			   cLockerArgument,
																			   iDataID_));
	}
}

//////////////////////////
// FileImpl::NadicBase::
//////////////////////////

// FUNCTION public
//	Candidate::FileImpl::NadicBase::setIsSimple -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
setIsSimple()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::clearBitSetFlag -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
clearBitSetFlag()
{
	Opt::ForEach_if(m_vecFiles,
					boost::bind(&Candidate::File::clearBitSetFlag,
								_1),
					Opt::ValidPointerFilter<Candidate::File>());
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	File::CheckArgument& cArgument_
//	boost::function<bool(LogicalFile::AutoLogicalFile&
//	LogicalFile::OpenOption&
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
check(Opt::Environment& cEnvironment_,
	  File::CheckArgument& cArgument_,
	  boost::function<bool(LogicalFile::AutoLogicalFile&,
						   LogicalFile::OpenOption&)> function_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isAlwaysUsed -- 
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
FileImpl::NadicBase::
isAlwaysUsed()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isAlwaysBitSet -- 
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
FileImpl::NadicBase::
isAlwaysBitSet()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isCheckByKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
isCheckByKey(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isLocatorUsed -- 
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
FileImpl::NadicBase::
isLocatorUsed()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isOtherFieldUsed -- 
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
FileImpl::NadicBase::
isOtherFieldUsed()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addCheckIndexResult -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	CheckIndexArgument& cArgument_
//	Predicate::ChosenInterface* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addCheckIndexResult(Opt::Environment& cEnvironment_,
					CheckIndexArgument& cArgument_,
					Predicate::ChosenInterface* pPredicate_)
{
	VECTOR<Predicate::ChosenInterface*>* pTarget =
		isAbleToGetByBitSet() ? &cArgument_.m_vecBitSet
		: (isSearchByBitSet() ? &cArgument_.m_vecSearchBitSet
		   : &cArgument_.m_vecIndexScan);

	if (pPredicate_->getType() == Tree::Node::Not) {
		pTarget->PUSHBACK(pPredicate_);

	} else {
		Super::insertCheckIndexResult(cEnvironment_,
									  pPredicate_,
									  pTarget,
									  this);
	}
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::getCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	AccessPlan::Cost&
//
// EXCEPTIONS

//virtual
AccessPlan::Cost&
FileImpl::NadicBase::
getCost(Opt::Environment& cEnvironment_,
		Interface::IPredicate* pPredicate_)
{
	if (m_cCost.isInfinity()) {
		AccessPlan::Cost::Value cOverhead(0);
		AccessPlan::Cost::Value cTotalCost(0);
		AccessPlan::Cost::Value cTupleCount(0);
		AccessPlan::Cost::Value cTupleSize(0);

		VECTOR<Candidate::File*>::ITERATOR iterator = m_vecFiles.begin();
		const VECTOR<Candidate::File*>::ITERATOR last = m_vecFiles.end();
		for (; iterator != last; ++iterator) {
			if (Candidate::File* pFile = *iterator) {
				AccessPlan::Cost cCost = pFile->getCost(cEnvironment_,
														pPredicate_);
				cOverhead += cCost.getOverhead();
				cTotalCost += cCost.getTotalCost();
				cTupleCount = MAX(cTupleCount, cCost.getTupleCount());
				cTupleSize += cCost.getTupleSize();
			}
		}
		m_cCost.setOverhead(cOverhead);
		m_cCost.setTupleSize(cTupleSize);
		m_cCost.setTupleCount(cTupleCount);
		m_cCost.setTotalCost(cTotalCost);
	}
	return m_cCost;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addField(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	m_cField.add(pField_);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addPutKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addPutKey(Opt::Environment& cEnvironment_,
		  Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addInsertField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addInsertField(Opt::Environment& cEnvironment_,
			   Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addUndoField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addUndoField(Opt::Environment& cEnvironment_,
			 Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	CheckIndexArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
checkIndex(Opt::Environment& cEnvironment_,
		   CheckIndexArgument& cArgument_)
{
	Opt::ForEach_if(m_vecFiles,
					boost::bind(&Candidate::File::checkIndex,
								_1,
								boost::ref(cEnvironment_),
								boost::ref(cArgument_)),
					Opt::ValidPointerFilter<Candidate::File>());
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::setForUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
setForUpdate(Opt::Environment& cEnvironment_)
{
	; // do nothing
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::getRankBitSetID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FileImpl::NadicBase::
getRankBitSetID()
{
	return -1;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::setRankBitSetID -- 
//
// NOTES
//
// ARGUMENTS
//	int iRankBitSetID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
setRankBitSetID(int iRankBitSetID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isGetByBitSet -- 
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
FileImpl::NadicBase::
isGetByBitSet()
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::hasOrder -- 
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
FileImpl::NadicBase::
hasOrder()
{
	return m_pOrder != 0;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::hasOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
hasOrder(Order::Specification* pSpecification_)
{
	return hasOrder()
		&& Order::Specification::isCompatible(m_pOrder,
											  pSpecification_);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isSearchable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node* pNode_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
isSearchable(Opt::Environment& cEnvironment_,
			 Tree::Node* pNode_)
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isNeedRetrieve -- 
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

bool
FileImpl::NadicBase::
isNeedRetrieve()
{
	return true;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isRetrievable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Scalar::Field* pField_)
{
	if (pField_->hasAlternativeValue(cEnvironment_)) {
		return Opt::IsAny(m_vecFiles,
						  Opt::LogicalAnd<Candidate::File>(
							   Opt::ValidPointerFilter<Candidate::File>(),
							   boost::bind(&Candidate::File::isRetrievable,
										   _1,
										   boost::ref(cEnvironment_),
										   pFile_,
										   pField_)));
	}
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createFileAccess -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Execution::Action::FileAccess*
//
// EXCEPTIONS

//virtual
Execution::Action::FileAccess*
FileImpl::NadicBase::
createFileAccess(Execution::Interface::IProgram& cProgram_)
{
	return 0;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createCheckAction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
FileImpl::NadicBase::
createCheckAction(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Execution::Action::FileAccess* pFileAccess_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createScanWithSearchByBitSetOption -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::NadicBase::
createScanWithSearchByBitSetOption(Opt::Environment& cEnvironment_,
								   Execution::Interface::IProgram& cProgram_,
								   Execution::Action::FileAccess* pFileAccess_,
								   Candidate::AdoptArgument& cArgument_,
								   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Scalar::Field* pFetchKey_
//	int iFetchKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createFetch(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Scalar::Field* pFetchKey_,
			int iFetchKeyID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createGetLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iKeyID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createGetLocator(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 int iKeyID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addUnionFileScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Action::FileAccess* pFileAccess_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
addUnionFileScan(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 Execution::Action::FileAccess* pFileAccess_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createInsert -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createInsert(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createExpunge(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createUpdate(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createUndoExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createUndoExpunge(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::createUndoUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
createUndoUpdate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::checkPartitionBy -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
checkPartitionBy(Opt::Environment& cEnvironment,
				 Order::Specification* pSpecification_)
{
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isUseUndoExpunge -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
isUseUndoExpunge(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isUseUndoUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::NadicBase::
isUseUndoUpdate(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::isUseOperation -- 
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
FileImpl::NadicBase::
isUseOperation()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::addBitSetToRowIDFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::NadicBase::
addBitSetToRowIDFilter(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   int iInDataID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Candidate::FileImpl::NadicBase::generatePutField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::NadicBase::
generatePutField(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION protected
//	Candidate::FileImpl::NadicBase::createOperandScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
FileImpl::NadicBase::
createOperandScan(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_,
				  Interface::IPredicate* pPredicate_)
{
	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
		pPredicate_->getChosen()->createScan(cEnvironment_,
											 cProgram_,
											 cArgument_,
											 cIndexArgument_);
	Execution::Interface::IIterator* pResult = cScan.first;
	if (cScan.second) {
		pResult->addPredicate(cProgram_,
							  cScan.second->getChosen()->createCheck(cEnvironment_,
																	 cProgram_,
																	 pResult,
																	 cArgument_,
																	 cIndexArgument_),
							  cArgument_.m_eTarget);
	}
	return pResult;
}

// FUNCTION protected
//	Candidate::FileImpl::NadicBase::addMergeOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pOperand_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Interface::IIterator* pIterator_
//	const VECTOR<Interface::IScalar*>& vecTuple_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::NadicBase::
addMergeOperand(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pOperand_,
				Candidate::AdoptArgument& cArgument_,
				Execution::Interface::IIterator* pIterator_,
				const VECTOR<Interface::IScalar*>& vecTuple_)
{
	// set input tuple
	VECTOR<int> vecOperandData;
	Opt::MapContainer(vecTuple_,
					  vecOperandData,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pOperand_,
								  boost::ref(cArgument_)));
	int iDataID = cProgram_.addVariable(vecOperandData);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pOperand_->getID(),
											iDataID));
}

//////////////////////////
// FileImpl::Intersect::
//////////////////////////

// FUNCTION public
//	Candidate::FileImpl::Intersect::addFieldForPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Intersect::
addFieldForPredicate(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Predicate::CheckRetrievableArgument& cCheckArgument_)
{
	Super2::addRetrieve(cEnvironment_,
						pFile_,
						cCheckArgument_);
	addField(cEnvironment_,
			 cCheckArgument_.m_pField);
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::isAbleToGetByBitSet -- 
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
FileImpl::Intersect::
isAbleToGetByBitSet()
{
	return Super2::isAbleToGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::disableGetByBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Intersect::
disableGetByBitSet()
{
	// do nothing
	;
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::isSearchByBitSet -- 
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
FileImpl::Intersect::
isSearchByBitSet()
{
	return Super2::isSearchByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::isGetByBitSetAvailable -- 
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
FileImpl::Intersect::
isGetByBitSetAvailable()
{
	return Super2::isAbleToGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::isSearchByBitSetAvailable -- 
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
FileImpl::Intersect::
isSearchByBitSetAvailable()
{
	return Opt::IsAny(getFiles(),
					  boost::bind(&Candidate::File::isSearchByBitSetAvailable,
								  _1));
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::Intersect::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	; _SYDNEY_ASSERT(getOrder());

	Candidate::CheckIndexArgument& cCheckArgument =
		Super2::getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	cCheckArgument.m_pTable = cArgument_.m_pTable;

	// create bitset to narrow result to AND result set
	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);
	Candidate::AdoptIndexArgument::Element& cMyElement =
		cMyIndexArgument.getElement(cArgument_.m_pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	// generate iterator to get bitset
	cMyIndexArgument.m_bForceGetBitSet = true;
	Candidate::CheckIndexArgument cMyCheckArgument;
	cMyCheckArgument.setAnd();
	cMyCheckArgument.m_bIgnoreField = true;

	PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
		Super2::createGetBitSet(cEnvironment_,
								cProgram_,
								cArgument_,
								cMyIndexArgument,
								cMyCheckArgument);
	; _SYDNEY_ASSERT(cGetBitSet.second >= 0);

	if (cGetBitSet.first) {
		cProgram_.addExecuteIterator(cGetBitSet.first);
	}

	// clear getbybitset flag
	cMyIndexArgument.m_bForceGetBitSet = false;
	clearBitSetFlag();

	// set limit if exists
	setOperandLimit(cEnvironment_);

	// use result bitset to searchbybitset
	cElement.m_iSearchBitSetID = cGetBitSet.second;

	// add RowID to retrieved field
	addField(cEnvironment_,
			 cArgument_.m_pTable->getRowID());

	// create row
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);
	getFields().foreachElement(boost::bind(&Candidate::Row::addScalar,
										   pRow,
										   _1));

	// prepare iterator
	int n = getFiles().GETSIZE();
	; _SYDNEY_ASSERT(n > 0);
	; _SYDNEY_ASSERT(n <= getOperand().GETSIZE());

	Execution::Interface::IIterator* pResult = 0;
	if (n == 1) {
		// single operand
		pResult = createOperandScan(cEnvironment_,
									cProgram_,
									cArgument_,
									cIndexArgument_,
									getOperand()[0]);
	} else {
		// create mergesort iterator to get result
		Execution::Iterator::MergeSort* pMergeSort =
			Execution::Iterator::MergeSort::create(cProgram_);

		// generate order
		Candidate::RowDelayArgument cDelayArgument;
		cDelayArgument.m_bGenerate = false; // prepare only data
		Order::GeneratedSpecification* pGenerated =
			getOrder()->generate(cEnvironment_,
							   cProgram_,
								 pMergeSort,
								 cArgument_,
								 pRow,
								 cDelayArgument);
		pMergeSort->setSortParameter(pGenerated->getPosition(),
									 pGenerated->getDirection());

		// result is mergesort
		pResult = pMergeSort;

		// add operands
		for (int i = 0; i < n; ++i) {
			Interface::IPredicate* pPredicate = getOperand()[i];

			Candidate::File* pFile = getFiles()[i];
			; _SYDNEY_ASSERT(pFile);
			// has order
			; _SYDNEY_ASSERT(pFile->hasOrder());

			Execution::Interface::IIterator* pIterator =
				createOperandScan(cEnvironment_,
								  cProgram_,
								  cArgument_,
								  cIndexArgument_,
								  pPredicate);

			addMergeOperand(cEnvironment_,
							cProgram_,
							pIterator,
							cArgument_,
							pResult,
							pGenerated->getTuple());
		}

		// other predicates is not needed to generate
		// because bitset includes the information

		// collection on memory with distinct
		Execution::Collection::Distinct* pDistinct =
			Execution::Collection::Distinct::ByRowID::create(cProgram_);

		// distinct by rowid
		int iKeyID = cArgument_.m_pTable->getRowID()->generate(cEnvironment_,
															   cProgram_,
															   pResult,
															   cArgument_);
		// add predicate checking duplication
		pResult->addPredicate(cProgram_,
							  Execution::Predicate::CollectionCheck::create(
									cProgram_,
									pResult,
									pDistinct->getID(),
									iKeyID),
							  cArgument_.m_eTarget);

		// set outdata
		VECTOR<int> vecOutID;
		Opt::MapContainer(pGenerated->getTuple(),
						  vecOutID,
						  boost::bind(&Interface::IScalar::generateFromType,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pResult,
									  boost::ref(cArgument_)));
		int iResult = cProgram_.addVariable(vecOutID);

		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT1(OutData,
											 iResult));
	}

	return pResult;
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::checkLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Intersect::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	m_cLimit = cLimit_;
	m_cLimit.setIntermediate();
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Intersect::getPredicateInterface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Predicate::ChosenInterface*
//
// EXCEPTIONS

//virtual
Predicate::ChosenInterface*
FileImpl::Intersect::
getPredicateInterface()
{
	return static_cast<Super2*>(this);
}

// FUNCTION private
//	Candidate::FileImpl::Intersect::setOperandLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Intersect::
setOperandLimit(Opt::Environment& cEnvironment_)
{
	if (m_cLimit.isSpecified()) {
		Opt::ForEach(getFiles(),
					 boost::bind(&Candidate::File::checkLimit,
								 _1,
								 boost::ref(cEnvironment_),
								 boost::cref(m_cLimit)));
	}
}

//////////////////////////
// FileImpl::Union::
//////////////////////////

// FUNCTION public
//	Candidate::FileImpl::Union::addFieldForPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Union::
addFieldForPredicate(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_,
					 Predicate::CheckRetrievableArgument& cCheckArgument_)
{
	Super2::addRetrieve(cEnvironment_,
						pFile_,
						cCheckArgument_);
	addField(cEnvironment_,
			 cCheckArgument_.m_pField);
}

// FUNCTION public
//	Candidate::FileImpl::Union::isAbleToGetByBitSet -- 
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
FileImpl::Union::
isAbleToGetByBitSet()
{
	return Super2::isAbleToGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Union::disableGetByBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
FileImpl::Union::
disableGetByBitSet()
{
	// do nothing
	;
}

// FUNCTION public
//	Candidate::FileImpl::Union::isSearchByBitSet -- 
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
FileImpl::Union::
isSearchByBitSet()
{
	return Super2::isSearchByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Union::isGetByBitSetAvailable -- 
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
FileImpl::Union::
isGetByBitSetAvailable()
{
	return Super2::isAbleToGetByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Union::isSearchByBitSetAvailable -- 
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
FileImpl::Union::
isSearchByBitSetAvailable()
{
	return Super2::isAbleToSearchByBitSet();
}

// FUNCTION public
//	Candidate::FileImpl::Union::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::FileAccess* pFileAccess_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
FileImpl::Union::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Action::FileAccess* pFileAccess_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	; _SYDNEY_ASSERT(getOrder());

	Candidate::CheckIndexArgument& cCheckArgument =
		Super2::getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	cCheckArgument.m_pTable = cArgument_.m_pTable;

	// set limit if exists
	setOperandLimit(cEnvironment_);

	int n = getFiles().GETSIZE();
	; _SYDNEY_ASSERT(n > 0);
	; _SYDNEY_ASSERT(n == getOperand().GETSIZE());

	Execution::Iterator::MergeSort* pResult =
		Execution::Iterator::MergeSort::create(cProgram_);

	// add RowID to retrieved field
	addField(cEnvironment_,
			 cArgument_.m_pTable->getRowID());

	// create row
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);
	getFields().foreachElement(boost::bind(&Candidate::Row::addScalar,
										pRow,
										_1));
	// generate order
	Candidate::RowDelayArgument cDelayArgument;
	cDelayArgument.m_bGenerate = false; // prepare only data
	Order::GeneratedSpecification* pGenerated =
		getOrder()->generate(cEnvironment_,
							 cProgram_,
							 pResult,
							 cArgument_,
							 pRow,
							 cDelayArgument);
	pResult->setSortParameter(pGenerated->getPosition(), pGenerated->getDirection());

	bool bHasOther = false;
	for (int i = 0; i < n; ++i) {
		Interface::IPredicate* pPredicate = getOperand()[i];

		if (Candidate::File* pFile = getFiles()[i]) {
			// has order
			; _SYDNEY_ASSERT(pFile->hasOrder());

			Candidate::AdoptIndexArgument cMyIndexArgument;

			Candidate::AdoptIndexArgument::Element& cElement =
				cIndexArgument_.getElement(cArgument_.m_pTable);
			Candidate::AdoptIndexArgument::Element& cMyElement =
				cMyIndexArgument.getElement(cArgument_.m_pTable);
			// propagate current search-by-bitset ID
			cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

			Execution::Interface::IIterator* pIterator =
				createOperandScan(cEnvironment_,
								  cProgram_,
								  cArgument_,
								  cMyIndexArgument,
								  pPredicate);

			addMergeOperand(cEnvironment_,
							cProgram_,
							pIterator,
							cArgument_,
							pResult,
							pGenerated->getTuple());
		} else {
			; _SYDNEY_ASSERT(pPredicate->isChosen());
			pPredicate->getChosen()->checkIndex(cEnvironment_,
												cProgram_,
												cArgument_,
												cCheckArgument);
			bHasOther = true;
		}
	}
	cCheckArgument.m_bChecked = true;

	if (bHasOther) {
		// create using normal algorithm
		Execution::Interface::IIterator* pIterator =
			createOperandScan(cEnvironment_,
							  cProgram_,
							  cArgument_,
							  cIndexArgument_,
							  this); // 'this' processes only others

		addMergeOperand(cEnvironment_,
						cProgram_,
						pIterator,
						cArgument_,
						pResult,
						pGenerated->getTuple());
	}

	// collection on memory with distinct
	Execution::Collection::Distinct* pDistinct =
		Execution::Collection::Distinct::ByRowID::create(cProgram_);

	// distinct by rowid
	int iKeyID = cArgument_.m_pTable->getRowID()->generate(cEnvironment_,
														   cProgram_,
														   pResult,
														   cArgument_);
	// add predicate checking duplication
	pResult->addPredicate(cProgram_,
						  Execution::Predicate::CollectionCheck::create(
								cProgram_,
								pResult,
								pDistinct->getID(),
								iKeyID),
						  cArgument_.m_eTarget);

	// set outdata
	VECTOR<int> vecOutID;
	Opt::MapContainer(pGenerated->getTuple(),
					  vecOutID,
					  boost::bind(&Interface::IScalar::generateFromType,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pResult,
								  boost::ref(cArgument_)));
	int iResult = cProgram_.addVariable(vecOutID);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResult));

	return pResult;
}

// FUNCTION public
//	Candidate::FileImpl::Union::checkLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
FileImpl::Union::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	m_cLimit = cLimit_;
	m_cLimit.setIntermediate();
	return false;
}

// FUNCTION public
//	Candidate::FileImpl::Union::getPredicateInterface -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Predicate::ChosenInterface*
//
// EXCEPTIONS

//virtual
Predicate::ChosenInterface*
FileImpl::Union::
getPredicateInterface()
{
	return static_cast<Super2*>(this);
}

// FUNCTION private
//	Candidate::FileImpl::Union::setOperandLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FileImpl::Union::
setOperandLimit(Opt::Environment& cEnvironment_)
{
	if (m_cLimit.isSpecified()) {
		Opt::ForEach_if(getFiles(),
						boost::bind(&Candidate::File::checkLimit,
									_1,
									boost::ref(cEnvironment_),
									boost::cref(m_cLimit)),
						Opt::ValidPointerFilter<Candidate::File>());

		VECTOR<Interface::IPredicate*>::ITERATOR start;
		VECTOR<Interface::IPredicate*>::ITERATOR last;
		if (getFiles()[0] == 0) {
			start = getOperand().begin();
			last = getOperand().end() - getFiles().GETSIZE();
		} else {
			start = getOperand().begin() + getFiles().GETSIZE();
			last = getOperand().end();
		}
		Opt::ForEach(start,
					 last,
					 boost::bind(&Predicate::ChosenInterface::checkLimit,
								 boost::bind(&Interface::IPredicate::getChosen,
											 _1),
								 boost::ref(cEnvironment_),
								 boost::cref(m_cLimit)));
	}
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
