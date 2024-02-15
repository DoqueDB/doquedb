// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ChosenImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
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
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/ChosenImpl.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/File/Parameter.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/Trace.h"
#include "Plan/Utility/Transaction.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/BitSet.h"
#include "Execution/Iterator/File.h"
#include "Execution/Iterator/Loop.h"
#include "Execution/Iterator/UnionDistinct.h"
#include "Execution/Operator/BitSet.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Predicate/Combinator.h"
#include "Execution/Predicate/IsEmpty.h"
#include "Execution/Predicate/RowIDCheck.h"
#include "Execution/Utility/Transaction.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/Estimate.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	AccessPlan::Cost::Value _minCost(const AccessPlan::Cost::Value& cCost0_,
									 const AccessPlan::Cost::Value& cCost1_)
	{
		return MIN(cCost0_, cCost1_);
	}
	AccessPlan::Cost::Value _maxCost(const AccessPlan::Cost::Value& cCost0_,
									 const AccessPlan::Cost::Value& cCost1_)
	{
		if (cCost0_.isInfinity()) return cCost1_;
		if (cCost1_.isInfinity()) return cCost0_;
		return MAX(cCost0_, cCost1_);
	}
}

//////////////////////////////////////
// Predicate::ChosenImpl::Base

// FUNCTION public
//	Predicate::ChosenImpl::Base::getCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::Base::
getCost(Opt::Environment& cEnvironment_,
		AccessPlan::Cost& cCost_)
{
	if (cCost_.isSetRate() && cCost_.isSetCount()) {
		// use cache
		if (m_cCost.isInfinity()) {
			m_cCost = cCost_;
			if (calculateCost(cEnvironment_, m_cCost) == false) {
				return false;
			}
		}
		cCost_ = m_cCost;

	} else {
		// calculate here
		if (calculateCost(cEnvironment_, cCost_) == false) {
			return false;
		}
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		if (cCost_.isSetCount()) {
			stream << "Predicate cost:("
				   << Utility::Trace::toString(cEnvironment_, getPredicate())
				   << ")\n  " << cCost_;
		} else {
			stream << "Predicate rate:("
				   << Utility::Trace::toString(cEnvironment_, getPredicate())
				   << ")\n  " << cCost_.getRate();
		}
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::getEstimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::Base::
getEstimateCost(Opt::Environment& cEnvironment_,
				AccessPlan::Cost& cCost_)
{
	// calculate here
	if (estimateCost(cEnvironment_, cCost_) == false) {
		return false;
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		if (cCost_.isSetCount()) {
			stream << "Estimate cost:("
				   << Utility::Trace::toString(cEnvironment_, getPredicate())
				   << ")\n  " << cCost_;
		} else {
			stream << "Estimate rate:("
				   << Utility::Trace::toString(cEnvironment_, getPredicate())
				   << ")\n  " << cCost_.getRate();
		}
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::adoptScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::Base::
adoptScan(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Candidate::AdoptArgument& cArgument_,
		  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	return createScan(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::Base::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// no index
	return getPredicate()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::Base::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (cArgument_.m_pScanFile) {
		return createScanByScan(cEnvironment_,
								cProgram_,
								cArgument_,
								cIndexArgument_);
	}
	// no index -> never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::createGetBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, int>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, int>
ChosenImpl::Base::
createGetBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (cIndexArgument_.m_bForceGetBitSet
		&& cArgument_.m_pScanFile) {
		return createGetBitSetByScan(cEnvironment_,
									 cProgram_,
									 cArgument_,
									 cIndexArgument_,
									 cCheckArgument_);
	}
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::createBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::Base::
createBitSet(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 Candidate::AdoptIndexArgument& cIndexArgument_,
			 Candidate::CheckIndexArgument& cCheckArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::Base::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	// no index -> never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::addUnionBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::Base::
addUnionBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	// no index -> never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::addExceptBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::Base::
addExceptBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	// no index -> never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::Base::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	// no index
	cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
	require(cEnvironment_, cArgument_.m_pTable);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::checkLimit -- 
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
ChosenImpl::Base::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	// can't process limit
	return false;
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::requireIfNeeded -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::Base::
requireIfNeeded(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::ChosenImpl::Base::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::Base::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cCost_)
{
	AccessPlan::Cost::Value cRate = getEstimateRate();
	// if predicate itself can estimate cost, use it
	bool bResult;
	if (bResult = getPredicate()->estimateCost(cEnvironment_,
											   cCost_)) {
		if (cRate.isInfinity() == false) {
			if (cCost_.isSetRate()) {
				cCost_.setRate(cRate);
			}
			if (cCost_.isSetCount()) {
				AccessPlan::Cost::Value cCount = cCost_.getTableCount();
				if (cCount.isInfinity()) {
					// use default
					cCount = 100000;
				}
				cCost_.setTupleCount(cCount * cRate);
			}
		}
	}
	return bResult;
}

// FUNCTION protected
//	Predicate::ChosenImpl::Base::createScanByScan -- create scan iterator by scan file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::Base::
createScanByScan(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Candidate::AdoptArgument& cArgument_,
				 Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// create scan iterator using scanfile
	cArgument_.m_pScanFile->addField(cEnvironment_,
									 cArgument_.m_pTable->getRowID());
	Execution::Action::FileAccess* pFileAccess =
		cArgument_.m_pScanFile->createFileAccess(cProgram_);

	Execution::Interface::IIterator* pResult =
		cArgument_.m_pScanFile->createScan(cEnvironment_,
										   cProgram_,
										   pFileAccess,
										   cArgument_,
										   cIndexArgument_);
	return MAKEPAIR(pResult, reinterpret_cast<Interface::IPredicate*>(this));
}

// FUNCTION protected
//	Predicate::ChosenImpl::Base::createGetBitSetByScan -- create bitset by scan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, int>
//
// EXCEPTIONS

PAIR<Execution::Interface::IIterator*, int>
ChosenImpl::Base::
createGetBitSetByScan(Opt::Environment& cEnvironment_,
					  Execution::Interface::IProgram& cProgram_,
					  Candidate::AdoptArgument& cArgument_,
					  Candidate::AdoptIndexArgument& cIndexArgument_,
					  Candidate::CheckIndexArgument& cCheckArgument_)
{
	// create bitset by scanning
	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);
	cElement.m_bForceBitSet = true;
	int iBitSetID = cElement.prepareBitSetData(cProgram_);

	bool bForceRowIDSave = cArgument_.m_bForceRowID;
	cArgument_.m_bForceRowID = false;

	Execution::Interface::IIterator* pIterator = 0;
	Interface::IPredicate* pPredicate = 0;

	if (cElement.m_iSearchBitSetID >= 0) {
		pIterator = createBitSetIterator(cEnvironment_,
										 cProgram_,
										 cArgument_,
										 cIndexArgument_,
										 cElement.m_iSearchBitSetID);
		pPredicate = this;
	} else {
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			createScanByScan(cEnvironment_,
							 cProgram_,
							 cArgument_,
							 cIndexArgument_);
		pIterator = cScan.first;
		pPredicate = cScan.second;
	}

	cArgument_.m_bForceRowID = bForceRowIDSave;

	Execution::Interface::IIterator* pResult = pIterator;
	if (pPredicate) {
		addCheckPredicate(cEnvironment_,
						  cProgram_,
						  pResult,
						  cArgument_,
						  cIndexArgument_,
						  pPredicate);
	}

	// put rowid to bitset
	int iRowIDID = cArgument_.m_pTable->getRowID()->generate(
								  cEnvironment_,
								  cProgram_,
								  pResult,
								  cArgument_);
	VECTOR<int> vecData(1, iRowIDID);
	pResult->addCalculation(cProgram_,
							Execution::Operator::BitSet::Collection::create(
									cProgram_,
									pResult,
									cProgram_.addVariable(vecData),
									iBitSetID));
	return MAKEPAIR(pResult, iBitSetID);
}

// FUNCTION protected
//	Predicate::ChosenImpl::Base::createBitSetIterator -- create bitset scan iterator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	int iBitSetID_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
ChosenImpl::Base::
createBitSetIterator(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Candidate::AdoptArgument& cArgument_,
					 Candidate::AdoptIndexArgument& cIndexArgument_,
					 int iBitSetID_)
{
	if (cArgument_.m_pTable->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Execution::Interface::IIterator* pResult =
		Execution::Iterator::BitSet::create(cProgram_,
											iBitSetID_);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0(CheckCancel));

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	// set output
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 cArgument_.m_pTable->getRowID()->generate(
											  cEnvironment_,
											  cProgram_,
											  pResult,
											  cArgument_)));

	Execution::Action::LockerArgument cLockerArgument;
	if (Utility::Transaction::Locker::createArgument(
							 cEnvironment_,
							 cArgument_,
							 cArgument_.m_pTable->getTable()->getSchemaTable(),
							 true /* collecting */,
							 cLockerArgument)) {
		// add locker
		pResult->addLocker(cProgram_,
						   cLockerArgument);
	}
	return pResult;
}

// FUNCTION private
//	Predicate::ChosenImpl::Base::addCheckPredicate -- add predicate to a iterator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::Base::
addCheckPredicate(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_,
				  Interface::IPredicate* pPredicate_)
{
	pIterator_->addPredicate(cProgram_,
							 pPredicate_->getChosen()->createCheck(cEnvironment_,
																   cProgram_,
																   pIterator_,
																   cArgument_,
																   cIndexArgument_),
							 cArgument_.m_eTarget);
}

// FUNCTION private
//	Predicate::ChosenImpl::Base::calculateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::Base::
calculateCost(Opt::Environment& cEnvironment_,
			  AccessPlan::Cost& cCost_)
{
	return estimateCost(cEnvironment_,
						cCost_);
}

//////////////////////////////////////
// Predicate::ChosenImpl::SingleImpl

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::setFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
setFile(Opt::Environment& cEnvironment_,
		Candidate::File* pFile_)
{
	if (pFile_->getFile() == m_pFile->getFile()) {
		m_pFile = pFile_;
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::chooseOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	Candidate::File*
//
// EXCEPTIONS

//virtual
Candidate::File*
ChosenImpl::SingleImpl::
chooseOrder(Opt::Environment& cEnvironment_,
			Order::Specification* pSpecification_,
			File::CheckOrderArgument& cCheckArgument_)
{
	if (m_pFile->hasOrder(pSpecification_)) {
		// create checkargument again
		Order::CheckedSpecification* pCheckedSpecification = pSpecification_->getChecked();
		if (pCheckedSpecification->createCheckOrderArgument(cEnvironment_,
															m_pFile->getFile(),
															cCheckArgument_)) {
			cCheckArgument_.m_pPredicate = this;
			cCheckArgument_.m_pFile = m_pFile->getFile();
			cCheckArgument_.m_pParameter = m_pFile->getParameter();
			return m_pFile;
		}
	}
	return 0;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::rechoose -- 
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
ChosenImpl::SingleImpl::
rechoose(Opt::Environment& cEnvironment_,
		 ChooseArgument& cArgument_)
{
	if (isNeedIndex()) {
		return this;
	}

	AccessPlan::Cost cMyCost;
	AccessPlan::Cost::Value cTableCount(m_pFile->getTable()->getEstimateCount(cEnvironment_));
	cMyCost.setIsSetCount(false); // treat as evaluated predicate
	cMyCost.setTableCount(cTableCount);
	(void)getCost(cEnvironment_, cMyCost);

	AccessPlan::Cost cScanCost;
	if (cArgument_.m_pFetch) {
		cScanCost.setIsSetCount(false); // treat as evaluated predicate
		cScanCost.setTableCount(cTableCount);
		(void)cArgument_.m_pFetch->getChosen()->getCost(cEnvironment_, cScanCost);
	} else {
		cScanCost = cArgument_.m_cScanCost;
	}

	cScanCost.setLimitCount(cArgument_.m_cEstimateLimit);

	addLockPenalty(cEnvironment_,
				   cMyCost,
				   cScanCost);

	if (cScanCost < cMyCost) {
		// don't use index
		require(cEnvironment_,
				cArgument_.m_pCandidate);
		setScanBetter();
	}
	return this;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::addLockPenalty -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	const AccessPlan::Cost& cScanCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
addLockPenalty(Opt::Environment& cEnvironment_,
			   AccessPlan::Cost& cCost_,
			   const AccessPlan::Cost& cScanCost_)
{
	if (getFile(cEnvironment_)
		&& getFile(cEnvironment_)->getTable()->isNeedLock(cEnvironment_)) {
		// add locking penalty totalcost * (count/10)^3
		AccessPlan::Cost::Value cMyTotalCost = cCost_.getTotalCost();
		AccessPlan::Cost::Value cMyCount = cScanCost_.getTupleCount() * cCost_.getRate();
		cMyCount /= 10;
		cMyTotalCost += cScanCost_.getProcessCost() * cMyCount * cMyCount * cMyCount;

		cCost_.setTotalCost(cMyTotalCost);
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isNeedIndex -- 
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
ChosenImpl::SingleImpl::
isNeedIndex()
{
	return m_pFile->isAlwaysUsed() || getPredicate()->isNeedIndex();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isNeedScan -- 
//
// NOTES
//
// ARGUMENTS
//	CheckNeedScanArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::SingleImpl::
isNeedScan(CheckNeedScanArgument& cArgument_)
{
	// when indexs table is not target table, scan is needed
	return isScanBetter()
		|| (cArgument_.m_pTable != 0
			&& cArgument_.m_pTable != m_pFile->getTable());
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isNeedRetrieve -- 
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
ChosenImpl::SingleImpl::
isNeedRetrieve()
{
	return m_pFile->isNeedRetrieve();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isSearchByBitSet -- 
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
ChosenImpl::SingleImpl::
isSearchByBitSet()
{
	return m_pFile->isSearchByBitSet();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isAbleToSearchByBitSet -- 
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
ChosenImpl::SingleImpl::
isAbleToSearchByBitSet()
{
	return m_pFile->isSearchByBitSetAvailable();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isAbleToGetByBitSet -- 
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
ChosenImpl::SingleImpl::
isAbleToGetByBitSet()
{
	return m_pFile->isGetByBitSetAvailable();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isAbleToProcessByBitSet -- 
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
ChosenImpl::SingleImpl::
isAbleToProcessByBitSet()
{
	return isAbleToGetByBitSet();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::hasOrder -- 
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
ChosenImpl::SingleImpl::
hasOrder(Order::Specification* pSpecification_)
{
	return m_pFile->hasOrder(pSpecification_);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isLimitAvailable -- 
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
ChosenImpl::SingleImpl::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	return true;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::isRetrievable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::SingleImpl::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  CheckRetrievableArgument& cCheckArgument_)
{
	if (cCheckArgument_.m_pField->isRowID()) {
		return m_pFile->getFile() == pFile_;
	}

	if (cCheckArgument_.m_pPredicate) {
		return cCheckArgument_.m_pPredicate == this;
	}

	if (m_pFile->isRetrievable(cEnvironment_,
							   pFile_,
							   cCheckArgument_.m_pField)) {
		if (cCheckArgument_.m_pField->isFunction()) {
			// function field needs to check availability with the predicate
			if (false ==
				cCheckArgument_.m_pField->checkAvailability(this)) {
				return false;
			}
		}
		if ((cCheckArgument_.m_bNeedScan == false
			 && !isScanBetter())
			|| m_pFile->hasOrder()) {
			// if needscan mode, this file can be used only when file is used for ordering

			// if field has alternative value,
			// it should be obtained from all the possible files
			if (cCheckArgument_.m_pField->hasAlternativeValue(cEnvironment_) == false) {
				cCheckArgument_.m_pPredicate = this;
				if (m_pFile->hasOrder()) {
					cCheckArgument_.m_pFile = m_pFile;
				}
			}
			return true;
		}
	}
	return false;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::addRetrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
addRetrieve(Opt::Environment& cEnvironment_,
			Interface::IFile* pFile_,
			CheckRetrievableArgument& cCheckArgument_)
{
	Relation::Table::AutoReset cReset = m_pFile->getTable()->getTable()->setEstimatePredicate(this);
	if (Scalar::Field* pField = cCheckArgument_.m_pField->getField(m_pFile->getFile())) {
		m_pFile->getTable()->addRetrieveField(cEnvironment_,
											  m_pFile,
											  m_pFile->getFile(),
											  cCheckArgument_.m_pField);
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::adoptScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::SingleImpl::
adoptScan(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Candidate::AdoptArgument& cArgument_,
		  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// check index availability
	if (isFetch() == false) {
		Candidate::CheckIndexArgument cCheckArgument;
		m_pFile->checkIndex(cEnvironment_, cCheckArgument);
	}

	return createScan(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::SingleImpl::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Order::Specification* pOrder = m_pFile->getTable()->getOrder();
	if (pOrder && pOrder->isChosen()
		&& pOrder->getChosen()->getPredicate() == this) {
		// this file is used for ordering
		return -1;

	} else if (isFetch()) {
		// fetch predicate can't be checked using file
		return Super::createCheck(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cArgument_,
								  cIndexArgument_);
	} else if (!isNeedIndex()
			   && cArgument_.m_pTable
			   && cArgument_.m_pTable->isNeedLock(cEnvironment_)
			   && getFile(cEnvironment_)
			   && getFile(cEnvironment_)->getTable() == cArgument_.m_pTable) {
		// if lock is needed, checking predicate don't use index
		return Super::createCheck(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cArgument_,
								  cIndexArgument_);
	} else if (isScanBetter() && !cIndexArgument_.m_bForceGetBitSet) {
		// if scan is better and bitset is not required, don't use index
		return Super::createCheck(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cArgument_,
								  cIndexArgument_);
	}

	// check index availability
	Candidate::CheckIndexArgument cCheckArgument;
	cCheckArgument.m_bTop = false;
	m_pFile->checkIndex(cEnvironment_, cCheckArgument);

	// create fileaccess action
	Execution::Action::FileAccess* pFileAccess = m_pFile->createFileAccess(cProgram_);
	// create actions specific to predicate
	adoptIndex(cEnvironment_,
			   cProgram_,
			   pFileAccess,
			   m_pFile,
			   cArgument_);

	// create check operator action
	return m_pFile->createCheckAction(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  pFileAccess,
									  cArgument_,
									  cIndexArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::SingleImpl::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (cArgument_.m_cLimit.isSpecified()) {
		// Limit is specified
		if (m_pFile->getParameter() == 0
			|| m_pFile->getParameter()->isLimited() == false) {
			// set limit to file
			AccessPlan::Limit cLimit(cArgument_.m_cLimit);
			cLimit.setIntermediate();
			m_pFile->checkLimit(cEnvironment_,
								cLimit);
		}
	}

	// create fileaccess action
	Execution::Action::FileAccess* pFileAccess = m_pFile->createFileAccess(cProgram_);

	// create actions specific to predicate
	adoptIndex(cEnvironment_,
			   cProgram_,
			   pFileAccess,
			   m_pFile,
			   cArgument_);

	// create filescan iterator using index file
	Execution::Interface::IIterator* pResult =
		m_pFile->createScan(cEnvironment_,
							cProgram_,
							pFileAccess,
							cArgument_,
							cIndexArgument_);

	if (isFetch()) {
		// add input data for fetch key
		if (cArgument_.m_pInput == 0) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		int iKeyID = generateKey(cEnvironment_, cProgram_, cArgument_.m_pInput, cArgument_);
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT1(Input,
											 iKeyID));
	}

	if (m_pFile->isGetByBitSet()) {
		Candidate::AdoptIndexArgument::Element& cElement =
			cIndexArgument_.getElement(m_pFile->getTable());
		if (cElement.m_bForceBitSet == false) {
			// scanning bitset is needed
			Execution::Interface::IIterator* pGetBitSet = pResult;
			; _SYDNEY_ASSERT(cElement.m_iBitSetID >= 0);

			// result is bitset scanner
			pResult = createBitSetIterator(cEnvironment_,
										   cProgram_,
										   cArgument_,
										   cIndexArgument_,
										   cElement.m_iBitSetID);

			// add file iterator as start up to get bitset first
			pResult->addCalculation(cProgram_,
									Execution::Operator::Iterate::Once::create(
											   cProgram_,
											   pResult,
											   pGetBitSet->getID()),
									Execution::Action::Argument::Target::StartUp);
		}
	}

	return MAKEPAIR(pResult,
					static_cast<Interface::IPredicate*>(0));
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::createGetBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, int>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, int>
ChosenImpl::SingleImpl::
createGetBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(m_pFile->getTable());
	cElement.m_bForceBitSet = true;

	AccessPlan::Limit cLimitSave = cArgument_.m_cLimit;
	cArgument_.m_cLimit = AccessPlan::Limit();

	bool bForceRowIDSave = cArgument_.m_bForceRowID;
	cArgument_.m_bForceRowID = false;

	// create action to get bitset
	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
		createScan(cEnvironment_,
				   cProgram_,
				   cArgument_,
				   cIndexArgument_);
	; _SYDNEY_ASSERT(cScan.second == 0);
	Execution::Interface::IIterator* pGetBitSet = cScan.first;

	cArgument_.m_cLimit = cLimitSave;
	cArgument_.m_bForceRowID = bForceRowIDSave;

	; _SYDNEY_ASSERT(cElement.m_iBitSetID >= 0);

	return MAKEPAIR(pGetBitSet, cElement.m_iBitSetID);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::createBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::SingleImpl::
createBitSet(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 Candidate::AdoptIndexArgument& cIndexArgument_,
			 Candidate::CheckIndexArgument& cCheckArgument_)
{
	; _SYDNEY_ASSERT(m_pFile->isAbleToGetByBitSet());

	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(m_pFile->getTable());
	cElement.m_bForceBitSet = true;

	if (cElement.m_iPrevBitSetID >= 0) {
		// add merge bitset instead of creating bitset
		addMergeBitSet(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   cArgument_,
					   cIndexArgument_,
					   cCheckArgument_);

		return cElement.m_iPrevBitSetID;
	} else {
		// create action to get bitset
		PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
			createGetBitSet(cEnvironment_,
							cProgram_,
							cArgument_,
							cIndexArgument_,
							cCheckArgument_);

		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Iterate::Once::create(
												  cProgram_,
												  pIterator_,
												  cGetBitSet.first->getID()),
								   cArgument_.m_eTarget);
		return cGetBitSet.second;
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	Candidate::Table* pTable = m_pFile->getTable();
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iPrevBitSetID >= 0);

	// check bitset is empty
	Execution::Interface::IAction* pIsEmpty =
		Execution::Predicate::IsEmpty::BitSet::create(cProgram_,
													  pIterator_,
													  cElement.m_iPrevBitSetID);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsEmpty->getID(),
											  cArgument_.m_eTarget));

	// create filescan to get bitset for this file
	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);

	// use current bitset ID as search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iPrevBitSetID;

	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	// create merge action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Intersect::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iPrevBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::addUnionBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
addUnionBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	Candidate::Table* pTable = m_pFile->getTable();
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iBitSetID >= 0);

	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	// create filescan to get bitset for this file
	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	// create union action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Union::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::addExceptBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
addExceptBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	Candidate::Table* pTable = m_pFile->getTable();
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iPrevBitSetID >= 0);

	// check bitset is empty
	Execution::Interface::IAction* pIsEmpty =
		Execution::Predicate::IsEmpty::BitSet::create(cProgram_,
													  pIterator_,
													  cElement.m_iPrevBitSetID);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsEmpty->getID(),
											  cArgument_.m_eTarget));

	// create filescan to get bitset for this file
	Candidate::AdoptIndexArgument cMyIndexArgument;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);

	// use current bitset ID as search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iPrevBitSetID;

	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	// create merge action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Difference::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iPrevBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	Order::Specification* pOrder = m_pFile->getTable()->getOrder();
	if (pOrder && pOrder->isChosen()
		&& pOrder->getChosen()->getPredicate() == this) {
		// this file is used for ordering
		m_pFile->checkIndex(cEnvironment_,
							cCheckArgument_);
		m_pFile->disableGetByBitSet();
		cCheckArgument_.m_pOrderScan = this;

	} else if (isFetch()) {
		cCheckArgument_.m_vecFetch.PUSHBACK(this);

	} else if ((cCheckArgument_.m_pTable == 0
				|| m_pFile->getTable() == cCheckArgument_.m_pTable)
			   &&
			   (cCheckArgument_.m_bIgnoreField || !isScanBetter())) {
		m_pFile->checkIndex(cEnvironment_,
							cCheckArgument_);
		if (isScanBetter()) {
			m_pFile->disableGetByBitSet();
		}
		m_pFile->addCheckIndexResult(cEnvironment_,
									 cCheckArgument_,
									 this);
		// require if needed
		requireIfNeeded(cEnvironment_, cArgument_.m_pTable, cCheckArgument_);
	} else {
		// different table's index can't be scanned
		cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
		getPredicate()->require(cEnvironment_, cArgument_.m_pTable);
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::checkLimit -- 
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
ChosenImpl::SingleImpl::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	return m_pFile->checkLimit(cEnvironment_, cLimit_);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::requireIfNeeded -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
requireIfNeeded(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (isFetch()) {
		getPredicate()->require(cEnvironment_, pCandidate_);

	} else if (!isNeedIndex()
			   && getFile(cEnvironment_)
			   && getFile(cEnvironment_)->getTable()->isNeedLock(cEnvironment_)
			   && getFile(cEnvironment_)->getTable() == pCandidate_
			   // if getbybitsetrowid is set and this predicate is placed top,
			   // donot require
			   && !(cCheckArgument_.m_bTop
					&& getFile(cEnvironment_)->getTable()->isGetByBitSetRowID())) {
		// if locking is needed, require means no index can be used
		getPredicate()->require(cEnvironment_, pCandidate_);
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::SingleImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cCost_)
{
	// This method is called only when file can't estimate count by itself
	// -> assume it doesn't reduce record number

	bool bResult;
	if (bResult = Super::estimateCost(cEnvironment_,
									  cCost_)) {
		AccessPlan::Cost::Value cRate(_minCost(1.0, cCost_.getRate() * 2.0));
		if (cCost_.isSetRate()) {
			cCost_.setRate(cRate);
		}
		if (cCost_.isSetCount()) {
			cCost_.setTupleCount(cCost_.getTableCount() * cRate);
		}
	}
	return bResult;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	if (isFetch()) {
		// when fetch predicate is required, it means no index can be used
		getPredicate()->require(cEnvironment_, pCandidate_);
	} else if (!isNeedIndex()
			   && getFile(cEnvironment_)
			   && getFile(cEnvironment_)->getTable()->isNeedLock(cEnvironment_)
			   && getFile(cEnvironment_)->getTable() == pCandidate_) {
		// if locking is needed, require means no index can be used
		getPredicate()->require(cEnvironment_, pCandidate_);
	}

	// otherwise, do nothing
}

// FUNCTION private
//	Predicate::ChosenImpl::SingleImpl::calculateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::SingleImpl::
calculateCost(Opt::Environment& cEnvironment_,
			  AccessPlan::Cost& cCost_)
{
	// cached in predicate node
	AccessPlan::Cost cFileCost = getPredicate()->getCost();
	if (cFileCost.isInfinity()) {
		cFileCost = m_pFile->getCost(cEnvironment_, this);
		getPredicate()->setCost(cFileCost);
	}
	cCost_ = cFileCost;
	if (cCost_.isSetRate() == false) {
		cCost_.setRate(1);
	}
	if (cCost_.isSetCount() == false) {
		cCost_.setTupleCount(AccessPlan::Cost::Value());
	}
	return true;
}

//////////////////////////////////////
// Predicate::ChosenImpl::SingleCheckUnknownImpl

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::SingleCheckUnknownImpl::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// use local index argument
	Candidate::AdoptIndexArgument cMyIndexArgument;

	// generate operand
	VECTOR<int> vecOperandID;
	vecOperandID.reserve(2);
	vecOperandID.PUSHBACK(Super::createCheck(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument_,
											 cMyIndexArgument));
	vecOperandID.PUSHBACK(m_pCheckUnknown->generate(cEnvironment_,
													cProgram_,
													pIterator_,
													cArgument_));

	Execution::Interface::IAction* pResult =
		Execution::Predicate::Combinator::Or::create(cProgram_,
													 pIterator_,
													 vecOperandID);
	return pResult->getID();
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::SingleCheckUnknownImpl::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::addUnionBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
addUnionBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::addExceptBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
addExceptBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
	require(cEnvironment_, cArgument_.m_pTable);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::checkLimit -- 
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
ChosenImpl::SingleCheckUnknownImpl::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	return false;
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::requireIfNeeded -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
requireIfNeeded(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	require(cEnvironment_, pCandidate_);
}

// FUNCTION public
//	Predicate::ChosenImpl::SingleCheckUnknownImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::SingleCheckUnknownImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	Super::require(cEnvironment_,
				   pCandidate_);
	m_pCheckUnknown->require(cEnvironment_,
							 pCandidate_);
}


////////////////////////////////////////
// Predicate::ChosenImpl::MultipleImpl

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::setFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::MultipleImpl::
setFile(Opt::Environment& cEnvironment_,
		Candidate::File* pFile_)
{
	m_pFile = pFile_;
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::rechoose -- 
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
ChosenImpl::MultipleImpl::
rechoose(Opt::Environment& cEnvironment_,
		 ChooseArgument& cArgument_)
{
	VECTOR<Interface::IPredicate*> vecOperand;
	vecOperand.reserve(getOperand().GETSIZE());

	VECTOR<Interface::IPredicate*>::ITERATOR found =
		Opt::FindLast(getOperand().begin(),
					  getOperand().end(),
					  boost::bind(&This::rechooseOperand,
								  boost::ref(cEnvironment_),
								  boost::ref(cArgument_),
								  _1,
								  boost::ref(vecOperand)));
	if (found != getOperand().end()) {
		return ChosenInterface::create(cEnvironment_,
									   getPredicate(),
									   getNotChecked(),
									   vecOperand);
	}
	return this;
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::addLockPenalty -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	const AccessPlan::Cost& cScanCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::MultipleImpl::
addLockPenalty(Opt::Environment& cEnvironment_,
			   AccessPlan::Cost& cCost_,
			   const AccessPlan::Cost& cScanCost_)
{
	FOREACH(getOperand(),
			boost::bind(&ChosenInterface::addLockPenalty,
						boost::bind(&Interface::IPredicate::getChosen,
									 _1),
						boost::ref(cEnvironment_),
						boost::ref(cCost_),
						boost::cref(cScanCost_)));
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::isNeedRetrieve -- 
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
ChosenImpl::MultipleImpl::
isNeedRetrieve()
{
	// retrieval is needed when any operand need
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isNeedRetrieve,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::isIndexAvailable -- 
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
ChosenImpl::MultipleImpl::
isIndexAvailable(Opt::Environment& cEnvironment_)
{
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isIndexAvailable,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::requireIfNeeded -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::MultipleImpl::
requireIfNeeded(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	Opt::ForEach(getOperand(),
				 boost::bind(&Predicate::ChosenInterface::requireIfNeeded,
							 boost::bind(&Interface::IPredicate::getChosen,
										 _1),
							 boost::ref(cEnvironment_),
							 pCandidate_,
							 boost::ref(cCheckArgument_)));
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::MultipleImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cCost_)
{
	boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)>
		cFunction = boost::bind(&Predicate::ChosenInterface::getEstimateCost,
								boost::bind(&Interface::IPredicate::getChosen,
											_1),
								boost::ref(cEnvironment_),
								_2);
	FOREACH(getOperand(),
			boost::bind(&This::accumulateCost,
						this,
						boost::ref(cEnvironment_),
						_1,
						cFunction,
						boost::ref(cCost_)));
	return true;
}

// FUNCTION public
//	Predicate::ChosenImpl::MultipleImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::MultipleImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	Opt::ForEach(getOperand(),
				 boost::bind(&Interface::IPredicate::require,
							 _1,
							 boost::ref(cEnvironment_),
							 pCandidate_));
}

// FUNCTION protected
//	Predicate::ChosenImpl::MultipleImpl::getCheckIndexArgument -- 
//
// NOTES
//
// ARGUMENTS
//	bool bIgnoreField_
//	
// RETURN
//	Candidate::CheckIndexArgument&
//
// EXCEPTIONS

//virtual
Candidate::CheckIndexArgument&
ChosenImpl::MultipleImpl::
getCheckIndexArgument(bool bIgnoreField_)
{
	if (m_cCheckIndexArgument.m_bIgnoreField != bIgnoreField_) {
		m_cCheckIndexArgument.clear();
		m_cCheckIndexArgument.m_bIgnoreField = bIgnoreField_;
	}
	return m_cCheckIndexArgument;
}

// FUNCTION protected
//	Predicate::ChosenImpl::MultipleImpl::createCheckOperand -- create action list checking each operand
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	VECTOR<int>& vecAction_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::MultipleImpl::
createCheckOperand(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Execution::Interface::IIterator* pIterator_,
				   VECTOR<int>& vecAction_,
				   Candidate::AdoptArgument& cArgument_,
				   Candidate::AdoptIndexArgument& cIndexArgument_,
				   Candidate::CheckIndexArgument& cCheckArgument_)
{
	vecAction_.reserve(getOperand().GETSIZE());

	Opt::MapContainer(cCheckArgument_.m_vecSearchBitSet,
					  vecAction_,
					  boost::bind(&ChosenInterface::createCheck,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_),
								  boost::ref(cIndexArgument_)));
	Opt::MapContainer(cCheckArgument_.m_vecIndexScan,
					  vecAction_,
					  boost::bind(&ChosenInterface::createCheck,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_),
								  boost::ref(cIndexArgument_)));
	if (cCheckArgument_.m_vecBitSet.ISEMPTY() == false) {
		// check by bitset should be placed here so that
		// necessary result can be output when same predicate is used
		// (for example, score in contains predicate)
		if (cArgument_.m_pTable
			&& cArgument_.m_pTable->isNeedLock(cEnvironment_)) {
			Opt::MapContainer(cCheckArgument_.m_vecBitSet,
							  vecAction_,
							  boost::bind(&ChosenInterface::createCheck,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_),
										  boost::ref(cIndexArgument_)));
		} else {
			Candidate::AdoptIndexArgument cMyIndexArgument;
			createCheckBitSet(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  vecAction_,
							  cArgument_,
							  cMyIndexArgument,
							  cCheckArgument_);
		}
	}
	Opt::MapContainer(cCheckArgument_.m_vecFetch,
					  vecAction_,
					  boost::bind(&ChosenInterface::createCheck,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_),
								  boost::ref(cIndexArgument_)));

	Opt::MapContainer(cCheckArgument_.m_vecNeedScan,
					  vecAction_,
					  boost::bind(&ChosenInterface::createCheck,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_),
								  boost::ref(cIndexArgument_)));
}

// FUNCTION protected
//	Predicate::ChosenImpl::MultipleImpl::checkOperandIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::MultipleImpl::
checkOperandIndex(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (cCheckArgument_.m_bChecked) return;

	cCheckArgument_.m_pTable = cArgument_.m_pTable;

	VECTOR<Interface::IPredicate*>::ITERATOR iterator = getOperand().begin();
	const VECTOR<Interface::IPredicate*>::ITERATOR last = getOperand().end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)->isChosen()) {
			(*iterator)->getChosen()->checkIndex(cEnvironment_,
												 cProgram_,
												 cArgument_,
												 cCheckArgument_);
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	cCheckArgument_.m_bChecked = true;
}

// FUNCTION private
//	Predicate::ChosenImpl::MultipleImpl::rechooseOperand -- rechoose an operand
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	ChooseArgument& cArgument_
//	Interface::IPredicate* pOperand_
//	VECTOR<Interface::IPredicate*>& vecRechosen_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
ChosenImpl::MultipleImpl::
rechooseOperand(Opt::Environment& cEnvironment_,
				ChooseArgument& cArgument_,
				Interface::IPredicate* pOperand_,
				VECTOR<Interface::IPredicate*>& vecRechosen_)
{
	; _SYDNEY_ASSERT(pOperand_->isChosen());
	Interface::IPredicate* pRechosen = pOperand_->getChosen()->rechoose(cEnvironment_,
																		cArgument_);
	vecRechosen_.PUSHBACK(pRechosen);
	return (pRechosen != pOperand_);
}

// FUNCTION private
//	Predicate::ChosenImpl::MultipleImpl::createCheckBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	VECTOR<int>& vecAction_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::MultipleImpl::
createCheckBitSet(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  VECTOR<int>& vecAction_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_,
				  Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (cArgument_.m_pTable->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Candidate::AdoptIndexArgument::Element& cMyElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	// create iterator to getbitset
	PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
		createGetBitSet(cEnvironment_,
						cProgram_,
						cArgument_,
						cIndexArgument_,
						cCheckArgument_);

	// add checking bitset action using the bitset
	Execution::Predicate::RowIDCheck* pCheck =
		Execution::Predicate::RowIDCheck::ByBitSet::create(
								   cProgram_,
								   pIterator_,
								   cGetBitSet.first->getID(),
								   cArgument_.m_pTable->getRowID()->generate(cEnvironment_,
																			 cProgram_,
																			 pIterator_,
																			 cArgument_),
								   cGetBitSet.second,
								   cMyElement.m_iPrevBitSetID);
	vecAction_.PUSHBACK(pCheck->getID());
}

// FUNCTION private
//	Predicate::ChosenImpl::MultipleImpl::calculateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::MultipleImpl::
calculateCost(Opt::Environment& cEnvironment_,
			  AccessPlan::Cost& cCost_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		_OPT_OPTIMIZATION_MESSAGE << "Calculate multiple cost("
								  << (cCost_.isSetCount() ? "set count" : "set rate")
								  << "):("
								  << Utility::Trace::toString(cEnvironment_, getPredicate())
								  << ") -------->" << ModEndl;
	}
#endif
	boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)>
		cFunction = boost::bind(&Predicate::ChosenInterface::getCost,
								boost::bind(&Interface::IPredicate::getChosen,
											_1),
								boost::ref(cEnvironment_),
								_2);
	VECTOR<Interface::IPredicate*>::ITERATOR iterator = getOperand().begin();
	const VECTOR<Interface::IPredicate*>::ITERATOR last = getOperand().end();

	bool bIsFetch = isFetch();

	// if isFetch, first operand is fetch with lowest rate
	// -> get count, donot get rate for the first operand
	bool bIsSetCount = bIsFetch || cCost_.isSetCount();
	bool bIsSetRate = !bIsFetch && cCost_.isSetRate();
	bool bDoNotSetRate = !cCost_.isSetRate() && !bIsFetch;

	AccessPlan::Cost cFirstCost;
	cFirstCost.setIsSetCount(bIsSetCount);
	cFirstCost.setIsSetRate(bIsSetRate);
	cFirstCost.setTableCount(cCost_.getTableCount());

	// calculate first cost
	accumulateCost(cEnvironment_,
				   *iterator,
				   cFunction,
				   cFirstCost);
	; _SYDNEY_ASSERT(!cFirstCost.isSetCount() || !cFirstCost.isInfinity());
	++iterator;

	// for rest operands, donot get count in fetching
	bIsSetCount = !bIsFetch && cCost_.isSetCount();
	bIsSetRate = bIsFetch || cCost_.isSetRate();

	AccessPlan::Cost cRestCost;
	if (bIsFetch || !bIsSetRate) {
		cRestCost = cFirstCost;
	} else {
		cRestCost.reset();
		cRestCost.setRate(cFirstCost.getRate());
		cRestCost.setTupleCount(cFirstCost.getTupleCount());
	}
	cRestCost.setIsSetCount(bIsSetCount);
	cRestCost.setIsSetRate(bIsSetRate);
	cRestCost.setTableCount(cCost_.getTableCount());

	FOREACH(iterator,
			last,
			boost::bind(&This::accumulateCost,
						this,
						boost::ref(cEnvironment_),
						_1,
						cFunction,
						boost::ref(cRestCost)));
	; _SYDNEY_ASSERT(!cRestCost.isSetCount() || !cRestCost.isInfinity());

	// set result
	cCost_ = cRestCost;
	if (cCost_.isSetCount() && !cRestCost.isSetCount() && cRestCost.getRate() > 0) {
		cCost_.setTotalCost(cCost_.getTotalCost() / cRestCost.getRate());
	}

	if (bIsFetch) {
		cCost_.setIsFetch();
		// tuple count is same as first cost
		cCost_.setTupleCount(cFirstCost.getTupleCount());
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		if (cCost_.isSetCount()) {
			_OPT_OPTIMIZATION_MESSAGE << "<-------- Calculate multiple cost:("
									  << Utility::Trace::toString(cEnvironment_, getPredicate())
									  << ")\n  " << cCost_
									  << ModEndl;
		} else {
			_OPT_OPTIMIZATION_MESSAGE << "<-------- Calculate multiple rate:("
									  << Utility::Trace::toString(cEnvironment_, getPredicate())
									  << ")\n  " << cCost_.getRate()
									  << ModEndl;
		}
	}
#endif
	; _SYDNEY_ASSERT(!cCost_.isSetCount() || !cCost_.isInfinity());

	return true;
}

////////////////////////////////////////
// Predicate::ChosenImpl::AndImpl

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::chooseOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	Interface::IFile*
//
// EXCEPTIONS

//virtual
Candidate::File*
ChosenImpl::AndImpl::
chooseOrder(Opt::Environment& cEnvironment_,
			Order::Specification* pSpecification_,
			File::CheckOrderArgument& cCheckArgument_)
{
	// search for index file which can process orderinng with minimum cost
	Candidate::File* pResult = 0;
	if (getOperand().GETSIZE() <= 1) {
		cCheckArgument_.skipEstimate();
	}
	bool bHasFunction = pSpecification_->hasFunctionKey();
	bool bHasAlternate = pSpecification_->hasAlternativeValue(cEnvironment_);

	// for intersect file
	VECTOR<Candidate::File*> vecOrderFile;
	VECTOR<Interface::IPredicate*> vecOperand;
	VECTOR<Interface::IPredicate*> vecOther;
	Candidate::Table* pTable = 0;

	VECTOR<Interface::IPredicate*>::ITERATOR iterator = getOperand().begin();
	const VECTOR<Interface::IPredicate*>::ITERATOR last = getOperand().end();
	for (; iterator != last; ++iterator) {
		; _SYDNEY_ASSERT((*iterator)->isChosen());
		ChosenInterface* pChosen = (*iterator)->getChosen();
		Candidate::File* pTmp = 0;
		if (pTmp = pChosen->chooseOrder(cEnvironment_,
										pSpecification_,
										cCheckArgument_)) {
			// the operand can process the specified order
			pResult = pTmp;
		}
		if (bHasFunction && bHasAlternate == false) {
			// if function key is included,
			// when any key can be retrieved from this operand,
			// the result of that operand should be chosen
			; _SYDNEY_ASSERT(pSpecification_->isChecked());
			Order::CheckedSpecification* pCheckedSpecification =
				pSpecification_->getChecked();
			if (pCheckedSpecification->isKeyRetrievable(cEnvironment_,
														pChosen)) {
				// [NOTES]
				// if pChosen can't process order, this can't process order
				break;
			}
		}
		if (bHasAlternate) {
			// if order by include alterative value key,
			// it should be considered to use intersection
			if (pTmp) {
				if (pTable && pTable != pTmp->getTable()) {
					// can't intersect different tables
					pTable = 0;
					pResult = 0;
					break;
				}
				pTable = pTmp->getTable();
				vecOrderFile.PUSHBACK(pTmp);
				vecOperand.PUSHBACK(pChosen);
			} else {
				if (pChosen->hasOrder(pSpecification_)) {
					// operand has index file prividing key but it can't process order
					// -> whole can't process order
					pTable = 0;
					pResult = 0;
					break;
				}
				vecOther.PUSHBACK(pChosen);
			}
		}
	}

	if (pTable && vecOperand.GETSIZE() > (cCheckArgument_.m_bIsTop ? 1 : 0)) {
		; _SYDNEY_ASSERT(vecOrderFile.GETSIZE() == vecOperand.GETSIZE());

		int n = vecOrderFile.GETSIZE();
		for (int i = 0; i < n; ++i) {
			if (vecOrderFile[i]) {
				vecOperand[i]->getChosen()->setFile(cEnvironment_,
													vecOrderFile[i]);
			}
		}

		// add non-order files to operand
		vecOperand.insert(vecOperand.end(), vecOther.begin(), vecOther.end());

		cCheckArgument_.m_pPredicate = this;
		// require sort key
		cCheckArgument_.m_pOrder->require(cEnvironment_, pTable);

		// create intersection file
		pResult = Candidate::File::create(cEnvironment_,
										  pTable,
										  vecOrderFile,
										  getPredicate(),
										  vecOperand,
										  cCheckArgument_.m_pOrder);
	}
	return pResult;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isNeedScan -- 
//
// NOTES
//
// ARGUMENTS
//	CheckNeedScanArgument& cArgument_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::AndImpl::
isNeedScan(CheckNeedScanArgument& cArgument_)
{
	if (getCombinedFile()) {
		return false;
	}

	// for and-combinator, scan is needed only when all need
	CheckNeedScanArgument cMyArgument(cArgument_);
	cMyArgument.m_bInAnd = true;

	// check whether all arguments needs scanning
	bool bResult =
		Opt::IsAll(getOperand(),
				   boost::bind(&ChosenInterface::isNeedScan,
							   boost::bind(&Interface::IPredicate::getChosen,
										   _1),
							   boost::ref(cMyArgument)));

	if (bResult == false
		&& cArgument_.m_bNeedScan == true
		&& cArgument_.m_bIsTop == false) {
		// check operands again with setting flag
		cMyArgument.m_bNeedScan = false;
		// if not-top, scan is needed when any operand need
		bResult =
			Opt::IsAny(getOperand(),
					   boost::bind(&ChosenInterface::isNeedScan,
								   boost::bind(&Interface::IPredicate::getChosen,
											   _1),
								   boost::ref(cMyArgument)));
	}
	return bResult;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isSearchByBitSet -- 
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
ChosenImpl::AndImpl::
isSearchByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isSearchByBitSet();
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isSearchByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isAbleToSearchByBitSet -- 
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
ChosenImpl::AndImpl::
isAbleToSearchByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToSearchByBitSet();
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isAbleToSearchByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isAbleToGetByBitSet -- 
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
ChosenImpl::AndImpl::
isAbleToGetByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToGetByBitSet();
	}
	VECTOR<Interface::IPredicate*>::ITERATOR first = getOperand().begin();
	const VECTOR<Interface::IPredicate*>::ITERATOR last = getOperand().end();

	return (first != last)
		&& (*first)->getChosen()->isAbleToGetByBitSet()
		&& Opt::IsAll(first + 1, last,
					  boost::bind(&ChosenInterface::isAbleToProcessByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isAbleToProcessByBitSet -- 
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
ChosenImpl::AndImpl::
isAbleToProcessByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToProcessByBitSet();
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isAbleToProcessByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::hasOrder -- 
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
ChosenImpl::AndImpl::
hasOrder(Order::Specification* pSpecification_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->hasOrder(pSpecification_);
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::hasOrder,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  pSpecification_));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isLimitAvailable -- 
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
ChosenImpl::AndImpl::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isLimitAvailable(cEnvironment_);
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isLimitAvailable,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isRetrievable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::AndImpl::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  CheckRetrievableArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isRetrievable(cEnvironment_,
																		 pFile_,
																		 cCheckArgument_);
	}
	CheckRetrievableArgument cCheckArgument(cCheckArgument_);

	if (cCheckArgument.m_pPredicate == 0) {
		; _SYDNEY_ASSERT(cCheckArgument_.m_pField);
		; _SYDNEY_ASSERT(cCheckArgument_.m_pField->isChecked()
						 || cCheckArgument_.m_pField->isFunction());
		if (cCheckArgument.m_pField->isRowID() == false
			&& cCheckArgument.m_pField->isFunction() == false
			&& cCheckArgument.m_pField->getChecked()->getFileSet().getSize() > 1) {
			cCheckArgument.m_bNeedScan = true;
		}
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isRetrievable,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_),
								  pFile_,
								  boost::ref(cCheckArgument)));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::addRetrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
addRetrieve(Opt::Environment& cEnvironment_,
			Interface::IFile* pFile_,
			CheckRetrievableArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->addFieldForPredicate(cEnvironment_,
												pFile_,
												cCheckArgument_);
		return;
	}
	if (cCheckArgument_.m_pPredicate) {
		cCheckArgument_.m_pPredicate->getChosen()->addRetrieve(cEnvironment_,
															   pFile_,
															   cCheckArgument_);
	} else {
		CheckRetrievableArgument cCheckArgument(cCheckArgument_);
		; _SYDNEY_ASSERT(cCheckArgument_.m_pField);
		; _SYDNEY_ASSERT(cCheckArgument_.m_pField->isChecked());
		if (cCheckArgument.m_pField->isRowID() == false
			&& cCheckArgument.m_pField->isFunction() == false
			&& cCheckArgument.m_pField->getChecked()->getFileSet().getSize() > 1) {
			cCheckArgument.m_bNeedScan = true;
		}
		FOREACH_if(getOperand(),
				   // function
				   boost::bind(&ChosenInterface::addRetrieve,
							   boost::bind(&Interface::IPredicate::getChosen,
										   _1),
							   boost::ref(cEnvironment_),
							   pFile_,
							   boost::ref(cCheckArgument)),
				   // filter
				   boost::bind(&ChosenInterface::isRetrievable,
							   boost::bind(&Interface::IPredicate::getChosen,
										   _1),
							   boost::ref(cEnvironment_),
							   pFile_,
							   boost::ref(cCheckArgument)));
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::adoptScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::AndImpl::
adoptScan(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Candidate::AdoptArgument& cArgument_,
		  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	cCheckArgument.clear();

	// divide operands according to index traits
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	if (cArgument_.m_cLimit.isSpecified()) {
		// limit can be used only when operands consist of
		// only one search-by-bitset and other get-by-bitsets

		if (cCheckArgument.isLimitAvailable() == false) {
			cArgument_.m_cLimit = AccessPlan::Limit();
		}
	}

	return createScan(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::AndImpl::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	VECTOR<int> vecAction;
	createCheckOperand(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   vecAction,
					   cArgument_,
					   cIndexArgument_,
					   cCheckArgument);

	if (vecAction.ISEMPTY() == false) {
		return createCheckPredicate(cProgram_,
									pIterator_,
									vecAction);
	}
	return -1;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::AndImpl::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getCombinedFile()) {
		if (cIndexArgument_.m_bForceGetBitSet == false) {
			// order is processed by operand
			Execution::Interface::IIterator* pResult =
				getCombinedFile()->createScan(cEnvironment_,
											  cProgram_,
											  0,
											  cArgument_,
											  cIndexArgument_);
			return MAKEPAIR(pResult, static_cast<Interface::IPredicate*>(0));
		} else {
			return getCombinedFile()->getPredicateInterface()->createScan(cEnvironment_,
																		  cProgram_,
																		  cArgument_,
																		  cIndexArgument_);
		}
	}

	// it's assumed checkOperandIndex has been called
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	Execution::Interface::IIterator* pResult = 0;

	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	// create scan iterator
	if (cCheckArgument.m_vecFetch.ISEMPTY() == false) {
		pResult = createScanFetch(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
	} else if (cElement.m_bForceBitSet
			   || (cCheckArgument.isScanBitSet(cEnvironment_,
											   cArgument_.m_pTable)
				   && !(cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false
						&& (cArgument_.m_pTable
							&& cArgument_.m_pTable->isNeedLock(cEnvironment_))))) {
		// use bitset
		pResult = createScanBitSet(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
		; _SYDNEY_ASSERT(cElement.m_iBitSetID >= 0);

	} else {
		// ordinal scan
		pResult = createScanNormal(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
	}

	; _SYDNEY_ASSERT(pResult);

	return MAKEPAIR(pResult,
					static_cast<Interface::IPredicate*>(this));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::createGetBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, int>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, int>
ChosenImpl::AndImpl::
createGetBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->createGetBitSet(cEnvironment_,
																		   cProgram_,
																		   cArgument_,
																		   cIndexArgument_,
																		   cCheckArgument_);
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	if (cCheckArgument_.m_bIgnoreField
		&& m_iBitSetIDCache >= 0) {

		Execution::Interface::IIterator* pIterator = 0;

		if (cElement.m_iSearchBitSetID >= 0
			&& cElement.m_iSearchBitSetID != m_iBitSetIDCache) {
			// add action to intersect bitset
			pIterator = Execution::Iterator::Loop::Once::create(cProgram_);

			// create intersect action
			pIterator->addCalculation(cProgram_,
									  Execution::Operator::BitSet::Intersect::create(
											  cProgram_,
											  pIterator,
											  m_iBitSetIDCache,
											  cElement.m_iSearchBitSetID,
											  pIterator->getLocker(cProgram_)));
		}
		return MAKEPAIR(pIterator, m_iBitSetIDCache);
	}

	if (cIndexArgument_.m_bForceGetBitSet
		&& cCheckArgument.isOnlyBitSet() == false) {
		PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
			createGetBitSetByScan(cEnvironment_,
								  cProgram_,
								  cArgument_,
								  cIndexArgument_,
								  cCheckArgument_);
		if (cCheckArgument_.m_bIgnoreField) {
			m_iBitSetIDCache = cGetBitSet.second;
		}
		return cGetBitSet;
	}

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);
	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cMyElement =
		cMyIndexArgument.getElement(cArgument_.m_pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	// get iterator to getbitset by first operand
	PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
		cCheckArgument.m_vecBitSet[0]->createGetBitSet(cEnvironment_,
													   cProgram_,
													   cArgument_,
													   cMyIndexArgument,
													   cCheckArgument_);

	// set search-by-bitset ID using result of first operand
	cMyElement.m_iSearchBitSetID = cMyElement.m_iPrevBitSetID = cGetBitSet.second;

	// add merge action to iterator
	FOREACH(cCheckArgument.m_vecBitSet.begin() + 1,
			cCheckArgument.m_vecBitSet.end(),
			boost::bind(&ChosenInterface::addMergeBitSet,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						cGetBitSet.first,
						boost::ref(cArgument_),
						boost::ref(cMyIndexArgument),
						boost::ref(cCheckArgument_)));
	if (cIndexArgument_.m_bForceGetBitSet == false) {
		cCheckArgument.m_vecBitSet.clear();
	}
	cElement.m_iBitSetID = cGetBitSet.second;

	if (cCheckArgument_.m_bIgnoreField) {
		m_iBitSetIDCache = cElement.m_iBitSetID;
	}
	return cGetBitSet;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::createBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::AndImpl::
createBitSet(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 Candidate::AdoptIndexArgument& cIndexArgument_,
			 Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->createBitSet(
								cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								cIndexArgument_,
								cCheckArgument_);
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);

	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	if (cCheckArgument_.m_bIgnoreField
		&& m_iBitSetIDCache >= 0) {
		return cElement.m_iBitSetID = m_iBitSetIDCache;
	}

	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cMyElement =
		cMyIndexArgument.getElement(cArgument_.m_pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;
	// force bitset
	cMyElement.m_bForceBitSet = true;

	// create action to create initial bitset
	cCheckArgument.m_vecBitSet[0]->createBitSet(cEnvironment_,
												cProgram_,
												pIterator_,
												cArgument_,
												cMyIndexArgument,
												cCheckArgument_);

	cMyElement.m_iSearchBitSetID = cMyElement.m_iPrevBitSetID = cMyElement.m_iBitSetID;
	// add merge action
	FOREACH(cCheckArgument.m_vecBitSet.begin() + 1,
			cCheckArgument.m_vecBitSet.end(),
			boost::bind(&ChosenInterface::addMergeBitSet,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						boost::ref(cMyIndexArgument),
						boost::ref(cCheckArgument_)));
	if (cIndexArgument_.m_bForceGetBitSet == false) {
		cCheckArgument.m_vecBitSet.clear();
	}
	cElement.m_iBitSetID = cMyElement.m_iBitSetID;

	if (cCheckArgument_.m_bIgnoreField) {
		m_iBitSetIDCache = cElement.m_iBitSetID;
	}

	return cElement.m_iBitSetID;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addMergeBitSet(cEnvironment_,
																   cProgram_,
																   pIterator_,
																   cArgument_,
																   cIndexArgument_,
																   cCheckArgument_);
	}

	if (cCheckArgument_.m_bIgnoreField) {
		Candidate::Table* pTable = cArgument_.m_pTable;
		Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
		; _SYDNEY_ASSERT(cElement.m_iPrevBitSetID >= 0);

		Candidate::AdoptIndexArgument cMyIndexArgument;
		cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
		Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);
		createBitSet(cEnvironment_,
					 cProgram_,
					 pIterator_,
					 cArgument_,
					 cMyIndexArgument,
					 cCheckArgument_);
		; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::BitSet::Intersect::create(
											  cProgram_,
											  pIterator_,
											  cElement.m_iPrevBitSetID,
											  cMyElement.m_iBitSetID,
											  pIterator_->getLocker(cProgram_)),
								   cArgument_.m_eTarget);
		return;
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);

	// add merge action to this iterator
	FOREACH(cCheckArgument.m_vecBitSet.begin(),
			cCheckArgument.m_vecBitSet.end(),
			boost::bind(&ChosenInterface::addMergeBitSet,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						boost::ref(cIndexArgument_),
						boost::ref(cCheckArgument_)));
	if (cIndexArgument_.m_bForceGetBitSet == false) {
		cCheckArgument.m_vecBitSet.clear();
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::addUnionBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
addUnionBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addUnionBitSet(cEnvironment_,
																   cProgram_,
																   pIterator_,
																   cArgument_,
																   cIndexArgument_,
																   cCheckArgument_);
		return;
	}

	Candidate::Table* pTable = cArgument_.m_pTable;
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iBitSetID >= 0);

	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	// add union action
	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Union::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::addExceptBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
addExceptBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addExceptBitSet(cEnvironment_,
																	cProgram_,
																	pIterator_,
																	cArgument_,
																	cIndexArgument_,
																	cCheckArgument_);
		return;
	}

	Candidate::Table* pTable = cArgument_.m_pTable;
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iPrevBitSetID >= 0);

	// check bitset is empty
	Execution::Interface::IAction* pIsEmpty =
		Execution::Predicate::IsEmpty::BitSet::create(cProgram_,
													  pIterator_,
													  cElement.m_iPrevBitSetID);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsEmpty->getID(),
											  cArgument_.m_eTarget));

	// create bitset for this file
	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);

	// use current bitset ID as search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iPrevBitSetID;

	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	// add difference action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Difference::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iPrevBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (cCheckArgument_.m_bIgnoreField == false
		&& getCombinedFile()
		&& getCombinedFile()->getTable()->getOrder()
		&& getCombinedFile()->getTable()->getOrder()->isChosen()) {
		// No.2115
		// this file is used for ordering
		getCombinedFile()->checkIndex(cEnvironment_,
									  cCheckArgument_);
		; _SYDNEY_ASSERT(getCombinedFile()->hasOrder());

		if (getCombinedFile()->getTable()->getOrder()->getChosen()->getPredicate() == this) {
			cCheckArgument_.m_pOrderScan = this;
		} else {
			getCombinedFile()->addCheckIndexResult(cEnvironment_,
												   cCheckArgument_,
												   this);
		}

	} else if (cCheckArgument_.m_bIgnoreField == false
			   && cCheckArgument_.m_bTop) {
		// set operand result directly
		checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument_);
		// not set checked flag yet
		cCheckArgument_.m_bChecked = false;

	} else {
		Candidate::CheckIndexArgument& cTmpArgument =
			getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
		cTmpArgument.m_bTop = cCheckArgument_.m_bTop;
		checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cTmpArgument);

		if (cTmpArgument.m_pOrderScan) {
			cCheckArgument_.m_pOrderScan = cTmpArgument.m_pOrderScan;
			cTmpArgument.m_pOrderScan = 0;
		}

		if (cTmpArgument.m_vecNeedScan.ISEMPTY() == false) {
			if (cCheckArgument_.m_bInOr
				&& (cTmpArgument.m_vecSearchBitSet.ISEMPTY() == false
					|| cTmpArgument.m_vecIndexScan.ISEMPTY() == false
					|| cTmpArgument.m_vecBitSet.ISEMPTY() == false)) {
				// insert this to index scan
				Candidate::File::insertCheckIndexResult(cEnvironment_,
														this,
														&cCheckArgument_.m_vecIndexScan,
														0);
			} else {
				cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
			}
		} else {
			if (cTmpArgument.m_vecSearchBitSet.ISEMPTY() == false
				&& cTmpArgument.m_vecBitSet.ISEMPTY()
				&& cCheckArgument_.m_bInAnd == false) {
				// can't search by bitset -> treat as index scan
				cTmpArgument.m_vecIndexScan.insert(cTmpArgument.m_vecIndexScan.begin(),
												   cTmpArgument.m_vecSearchBitSet.begin(),
												   cTmpArgument.m_vecSearchBitSet.end());
				cTmpArgument.m_vecSearchBitSet.clear();
			}
			if (cTmpArgument.m_vecIndexScan.ISEMPTY() == false) {
				// insert this to index scan
				Candidate::File::insertCheckIndexResult(cEnvironment_,
														this,
														&cCheckArgument_.m_vecIndexScan,
														0);
			} else if (cTmpArgument.m_vecSearchBitSet.ISEMPTY() == false) {
				Candidate::File::insertCheckIndexResult(cEnvironment_,
														this,
														&cCheckArgument_.m_vecSearchBitSet,
														0);
			} else if (cTmpArgument.m_vecBitSet.ISEMPTY() == false) {
				Candidate::File::insertCheckIndexResult(cEnvironment_,
														this,
														&cCheckArgument_.m_vecBitSet,
														0);
			} else {
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
	}
	// require if needed
	requireIfNeeded(cEnvironment_, cArgument_.m_pTable, cCheckArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::checkLimit -- 
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
ChosenImpl::AndImpl::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	if (getFile(cEnvironment_)) {
		(void)getFile(cEnvironment_)->checkLimit(cEnvironment_,
												 cLimit_);
	}
	// anyway, this method returns false
	// because limit is not entirely processed by the file
	return false;
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::isFetch -- 
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
ChosenImpl::AndImpl::
isFetch()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isFetch();
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&Interface::IPredicate::isFetch,
								  _1));
}

// FUNCTION public
//	Predicate::ChosenImpl::AndImpl::getFetchKey -- get fetch key
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
ChosenImpl::AndImpl::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->getFetchKey(cEnvironment_,
																	   cFetchKey_);
	}
	Utility::ScalarSet cTmpFetchKey;
	bool bResult = Opt::IsAll(getOperand(),
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

// FUNCTION protected
//	Predicate::ChosenImpl::AndImpl::getCheckIndexArgument -- 
//
// NOTES
//
// ARGUMENTS
//	bool bIgnoreField_
//
// RETURN
//	Candidate::CheckIndexArgument&
//
// EXCEPTIONS

//virtual
Candidate::CheckIndexArgument&
ChosenImpl::AndImpl::
getCheckIndexArgument(bool bIgnoreField_)
{
	Candidate::CheckIndexArgument& cResult =
		Super::getCheckIndexArgument(bIgnoreField_);
	cResult.setAnd();
	return cResult;
}

// FUNCTION private
//	Predicate::ChosenImpl::AndImpl::createScanFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
ChosenImpl::AndImpl::
createScanFetch(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	; _SYDNEY_ASSERT(cCheckArgument.m_vecFetch.ISEMPTY() == false);

	// choose first predicate
	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
		cCheckArgument.m_vecFetch[0]->getChosen()->createScan(cEnvironment_,
															  cProgram_,
															  cArgument_,
															  cIndexArgument_);
	if (cScan.second) {
		; _SYDNEY_ASSERT(cScan.second->isChosen());
		cCheckArgument.m_vecNeedScan.PUSHBACK(cScan.second->getChosen());
		cScan.second->require(cEnvironment_,
							  cArgument_.m_pTable);
	}
	// rest fetch predicates are treated as needscan
	cCheckArgument.m_vecNeedScan.insert(cCheckArgument.m_vecNeedScan.end(),
										cCheckArgument.m_vecFetch.begin() + 1,
										cCheckArgument.m_vecFetch.end());
	Opt::ForEach(cCheckArgument.m_vecFetch.begin() + 1,
				 cCheckArgument.m_vecFetch.end(),
				 boost::bind(&Interface::IPredicate::require,
							 _1,
							 boost::ref(cEnvironment_),
							 cArgument_.m_pTable));
	cCheckArgument.m_vecFetch.clear();

	return cScan.first;
}

// FUNCTION private
//	Predicate::ChosenImpl::AndImpl::createScanBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
ChosenImpl::AndImpl::
createScanBitSet(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Candidate::AdoptArgument& cArgument_,
				 Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);

	Execution::Interface::IIterator* pResult = 0;
	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	// get iterator to get bitset
	PAIR<Execution::Interface::IIterator*, int> cGetBitSet(0, cElement.m_iSearchBitSetID);

	if (cCheckArgument.m_vecBitSet.ISEMPTY() == false) {
		cGetBitSet = createGetBitSet(cEnvironment_,
									 cProgram_,
									 cArgument_,
									 cIndexArgument_,
									 cCheckArgument);

		// use result bitset ID as search-by-bitset ID
		cElement.m_iSearchBitSetID = cGetBitSet.second;

		if (cGetBitSet.first) {
			cProgram_.addExecuteIterator(cGetBitSet.first);
		}
	}

	// create main iterator
	if (cCheckArgument.m_pOrderScan) {
		// if ordering file is used, the file is scanned
		; _SYDNEY_ASSERT(cCheckArgument.m_pOrderScan->isChosen());
		; _SYDNEY_ASSERT(cCheckArgument.m_pOrderScan->getFile(cEnvironment_)->isSearchByBitSet());

		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_pOrderScan->getChosen()->createScan(cEnvironment_,
																 cProgram_,
																 cArgument_,
																 cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
	} else if (cCheckArgument.m_vecSearchBitSet.ISEMPTY()) {
		// if no index file can search by bitset, bitset iterator is used
		pResult = createBitSetIterator(cEnvironment_,
									   cProgram_,
									   cArgument_,
									   cIndexArgument_,
									   cGetBitSet.second);
	} else {
		// first search-by-bitset file scanned
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_vecSearchBitSet[0]->createScan(cEnvironment_,
															cProgram_,
															cArgument_,
															cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
		cCheckArgument.m_vecSearchBitSet.POPFRONT();
	}

	return pResult;
}

// FUNCTION private
//	Predicate::ChosenImpl::AndImpl::createScanNormal -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
ChosenImpl::AndImpl::
createScanNormal(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Candidate::AdoptArgument& cArgument_,
				 Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);

	Execution::Interface::IIterator* pResult = 0;

	if (cCheckArgument.m_pOrderScan) {
		// if ordering file is used, the file is scanned
		; _SYDNEY_ASSERT(cCheckArgument.m_pOrderScan->isChosen());

		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_pOrderScan->getChosen()->createScan(cEnvironment_,
																 cProgram_,
																 cArgument_,
																 cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
	} else if (cCheckArgument.m_vecIndexScan.ISEMPTY() == false) {
		// vecIndexScan is stored in order of tuple count
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_vecIndexScan[0]->createScan(cEnvironment_,
														 cProgram_,
														 cArgument_,
														 cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
		cCheckArgument.m_vecIndexScan.POPFRONT();

	} else if (cCheckArgument.m_vecSearchBitSet.ISEMPTY() == false) {
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_vecSearchBitSet[0]->createScan(cEnvironment_,
															cProgram_,
															cArgument_,
															cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
		cCheckArgument.m_vecSearchBitSet.POPFRONT();

	} else if (cCheckArgument.m_vecBitSet.ISEMPTY() == false) {
		if (cCheckArgument.m_vecBitSet[0]->getFile(cEnvironment_)) {
			cCheckArgument.m_vecBitSet[0]->getFile(cEnvironment_)->clearBitSetFlag();
		}
		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			cCheckArgument.m_vecBitSet[0]->createScan(cEnvironment_,
													  cProgram_,
													  cArgument_,
													  cIndexArgument_);
		pResult = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pResult,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
		cCheckArgument.m_vecBitSet.POPFRONT();

	} else {
		// no file can be scaned
		_SYDNEY_THROW0(Exception::NotSupported);			
	}

	return pResult;
}
 
// FUNCTION private
//	Predicate::ChosenImpl::AndImpl::createCheckPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	VECTOR<int>& vecAction_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
ChosenImpl::AndImpl::
createCheckPredicate(Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 VECTOR<int>& vecAction_)
{
	Execution::Interface::IPredicate* pResult =
		Execution::Predicate::Combinator::And::create(cProgram_,
													  pIterator_,
													  vecAction_);
	return pResult->getID();
}

// FUNCTION private
//	Predicate::ChosenImpl::AndImpl::accumulateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::AndImpl::
accumulateCost(Opt::Environment& cEnvironment_,
			   Interface::IPredicate* pPredicate_,
			   boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_,
			   AccessPlan::Cost& cResult_)
{
	AccessPlan::Cost cMyCost;
	cMyCost.setIsFetch(cResult_.isFetch());
	cMyCost.setIsSetCount(cResult_.isSetCount());
	cMyCost.setIsSetRate(cResult_.isSetRate());
	cMyCost.setTupleCount(cResult_.getTupleCount());
	cMyCost.setTableCount(cResult_.getTableCount());

	if (cFunction_(pPredicate_, boost::ref(cMyCost)) == false) {
		// use estimate instead
		if (pPredicate_->estimateCost(cEnvironment_,
									  cMyCost) == false) {
			return;
		}
	}
	if (cResult_.isInfinity()) {
		cResult_ = cMyCost;
	} else {
		AccessPlan::Cost::Value cOverhead = cResult_.getOverhead() + cMyCost.getOverhead();
		AccessPlan::Cost::Value cRetrieveCost = cResult_.getRetrieveCost() + cMyCost.getRetrieveCost();

		cResult_.setOverhead(cOverhead);
		if (cResult_.isSetRate()) {
			cResult_.setRate(_minCost(cResult_.getRate(), cMyCost.getRate()));
		} else {
			cResult_.setRate(1);
		}
		if (cResult_.isSetCount()) {
			if (cResult_.getTupleCount() > cMyCost.getTupleCount()) {
				cResult_.setTupleCount(cMyCost.getTupleCount());
				cResult_.setTotalCost(cMyCost.getTotalCost());
			}
		} else {
			cResult_.setTupleCount(AccessPlan::Cost::Value());
		}
		cResult_.setRetrieveCost(cRetrieveCost);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		if (cResult_.isSetCount()) {
			stream << "Accumulate cost(and):("
				   << Utility::Trace::toString(cEnvironment_, pPredicate_)
				   << ")\n  " << cResult_;
		} else {
			stream << "Accumulate rate(and):("
				   << Utility::Trace::toString(cEnvironment_, pPredicate_)
				   << ")\n  " << cResult_.getRate();
		}
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
}

////////////////////////////////////////
// Predicate::ChosenImpl::OrImpl

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::chooseOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	File::CheckOrderArgument& cCheckArgument_
//	
// RETURN
//	Interface::IFile*
//
// EXCEPTIONS

//virtual
Candidate::File*
ChosenImpl::OrImpl::
chooseOrder(Opt::Environment& cEnvironment_,
			Order::Specification* pSpecification_,
			File::CheckOrderArgument& cCheckArgument_)
{
	if (pSpecification_->hasAlternativeValue(cEnvironment_)) {
		// No.2115
		// If order key has alternative value, ordering is passed to operands

		bool bAscending = pSpecification_->isAscending();

		VECTOR<Candidate::File*> vecOrderFile;
		VECTOR<Interface::IPredicate*> vecOperand;
		Candidate::Table* pTable = 0;

		File::CheckOrderArgument cMyCheckArgument(cCheckArgument_);
		bool bIsTopSave = cMyCheckArgument.m_bIsTop;
		cMyCheckArgument.m_bIsTop = false;

		VECTOR<Interface::IPredicate*>::ITERATOR iterator = getOperand().begin();
		const VECTOR<Interface::IPredicate*>::ITERATOR last = getOperand().end();
		for (; iterator != last; ++iterator) {
			; _SYDNEY_ASSERT((*iterator)->isChosen());
			ChosenInterface* pChosen = (*iterator)->getChosen();
			Candidate::File* pFile = pChosen->chooseOrder(cEnvironment_,
														  pSpecification_,
														  cMyCheckArgument);
			if ((pFile == 0) == bAscending) {
				// when ascending order, no order file is placed ahead.
				// when descending order, ordered file is placed ahead.
				vecOrderFile.PUSHFRONT(pFile);
				vecOperand.PUSHFRONT(*iterator);
			} else {
				vecOrderFile.PUSHBACK(pFile);
				vecOperand.PUSHBACK(*iterator);
			}
			if (pFile) {
				if (pTable && pTable != pFile->getTable()) {
					// can't merge different table
					pTable = 0;
					break;
				}
				pTable = pFile->getTable();
			}
		}
		cMyCheckArgument.m_bIsTop = bIsTopSave;

		if (pTable) {
			; _SYDNEY_ASSERT(vecOrderFile.GETSIZE() == vecOperand.GETSIZE());
			int n = vecOrderFile.GETSIZE();
			for (int i = 0; i < n; ++i) {
				if (vecOrderFile[i]) {
					vecOperand[i]->getChosen()->setFile(cEnvironment_,
														vecOrderFile[i]);
				}
			}
			// create result
			cCheckArgument_ = cMyCheckArgument;
			cCheckArgument_.m_pPredicate = this;

			// require sort key
			cCheckArgument_.m_pOrder->require(cEnvironment_, pTable);

			// create union file
			return Candidate::File::create(cEnvironment_,
										   pTable,
										   vecOrderFile,
										   getPredicate(),
										   vecOperand,
										   cCheckArgument_.m_pOrder);
		}
	}
	// no order
	return 0;
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isNeedScan -- 
//
// NOTES
//
// ARGUMENTS
//	CheckNeedScanArgument& cArgument_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::OrImpl::
isNeedScan(CheckNeedScanArgument& cArgument_)
{
	if (getCombinedFile()) {
		return false;
	}

	CheckNeedScanArgument cMyArgument(cArgument_, false /* no top */);

	// for or-combinator, scan is needed when any one needs
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::isNeedScan,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cMyArgument)))
		|| Opt::IsAny(getOperand(),
					  boost::bind(&Interface::IPredicate::isFetch,
								  _1));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isSearchByBitSet -- 
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
ChosenImpl::OrImpl::
isSearchByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isSearchByBitSet();
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isSearchByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isAbleToSearchByBitSet -- 
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
ChosenImpl::OrImpl::
isAbleToSearchByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToSearchByBitSet();
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isAbleToSearchByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isAbleToGetByBitSet -- 
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
ChosenImpl::OrImpl::
isAbleToGetByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToGetByBitSet();
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isAbleToGetByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isAbleToProcessByBitSet -- 
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
ChosenImpl::OrImpl::
isAbleToProcessByBitSet()
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isAbleToProcessByBitSet();
	}

	// OR is processed by bitset only when all operands can be gotten by bitset
	// [NOTES]
	// This implementation can be replaced to ProcessByBitSet if NotImpl::addUnionBitSet is implemented
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isAbleToGetByBitSet,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1)));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::hasOrder -- 
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
ChosenImpl::OrImpl::
hasOrder(Order::Specification* pSpecification_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->hasOrder(pSpecification_);
	}
	return Opt::IsAny(getOperand(),
					  boost::bind(&ChosenInterface::hasOrder,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  pSpecification_));

}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isLimitAvailable -- 
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
ChosenImpl::OrImpl::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isLimitAvailable(cEnvironment_);
	}
	return Opt::IsAll(getOperand(),
					  boost::bind(&ChosenInterface::isLimitAvailable,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_)));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::isRetrievable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::OrImpl::
isRetrievable(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  CheckRetrievableArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->isRetrievable(cEnvironment_,
																		 pFile_,
																		 cCheckArgument_);
	}

	// if field is virtual field (ex. score),
	// it can be retrieved from index file
	if (cCheckArgument_.m_pField->isFunction()) {
		// check for operands
		return Opt::IsAny(getOperand(),
						  boost::bind(&ChosenInterface::isRetrievable,
									  boost::bind(&Interface::IPredicate::getChosen,
												  _1),
									  boost::ref(cEnvironment_),
									  pFile_,
									  boost::ref(cCheckArgument_)));
	}
	return cCheckArgument_.m_pField->isRowID();
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::addRetrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	CheckRetrievableArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
addRetrieve(Opt::Environment& cEnvironment_,
			Interface::IFile* pFile_,
			CheckRetrievableArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->addFieldForPredicate(cEnvironment_,
												pFile_,
												cCheckArgument_);
		return;
	}
	if (cCheckArgument_.m_pField->isRowID()
		|| (cCheckArgument_.m_pField->hasAlternativeValue(cEnvironment_)
			&& isCombinedFile())) {
		// retrieve from all operands
		FOREACH_if(getOperand(),
				   // function
				   boost::bind(&ChosenInterface::addRetrieve,
							   boost::bind(&Interface::IPredicate::getChosen,
										   _1),
							   boost::ref(cEnvironment_),
							   pFile_,
							   boost::ref(cCheckArgument_)),
				   // filter
				   boost::bind(&ChosenInterface::isRetrievable,
							   boost::bind(&Interface::IPredicate::getChosen,
										   _1),
							   boost::ref(cEnvironment_),
							   pFile_,
							   boost::ref(cCheckArgument_)));
	} else {
		// find first operand which is retrievalbe
		VECTOR<Interface::IPredicate*>::ITERATOR found =
			Opt::Find(getOperand().begin(),
					  getOperand().end(),
					  boost::bind(&ChosenInterface::isRetrievable,
								  boost::bind(&Interface::IPredicate::getChosen,
											  _1),
								  boost::ref(cEnvironment_),
								  pFile_,
								  boost::ref(cCheckArgument_)));
		if (found != getOperand().end()) {
			// found
			; _SYDNEY_ASSERT((*found)->isChosen());
			(*found)->getChosen()->addRetrieve(cEnvironment_,
											   pFile_,
											   cCheckArgument_);
			// set to operand<->field map
			m_mapRetrieveField[*found].add(cCheckArgument_.m_pField);
		}
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::adoptScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::OrImpl::
adoptScan(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Candidate::AdoptArgument& cArgument_,
		  Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (cArgument_.m_cLimit.isSpecified()
		&& getCombinedFile() == 0) {
		// divide operands according to index traits
		Candidate::CheckIndexArgument cMyArgument;
		cMyArgument.setOr();
		checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cMyArgument);

		if (cMyArgument.isLimitAvailable() == false) {
			cArgument_.m_cLimit = AccessPlan::Limit();
		}
	}
	return createScan(cEnvironment_, cProgram_, cArgument_, cIndexArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::OrImpl::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;

	VECTOR<int> vecAction;
	createCheckOperand(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   vecAction,
					   cArgument_,
					   cMyIndexArgument,
					   cCheckArgument);

	if (vecAction.ISEMPTY() == false) {
		return createCheckPredicate(cProgram_,
									pIterator_,
									vecAction);
	}
	return -1;
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::createScan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, Interface::IPredicate*>
ChosenImpl::OrImpl::
createScan(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::AdoptIndexArgument& cIndexArgument_)
{
	if (getCombinedFile()) {
		if (cIndexArgument_.m_bForceGetBitSet == false) {
			// order is processed by operand
			Execution::Interface::IIterator* pResult =
				getCombinedFile()->createScan(cEnvironment_,
											  cProgram_,
											  0,
											  cArgument_,
											  cIndexArgument_);
			return MAKEPAIR(pResult, static_cast<Interface::IPredicate*>(0));
		} else {
			return getCombinedFile()->getPredicateInterface()->createScan(cEnvironment_,
																		  cProgram_,
																		  cArgument_,
																		  cIndexArgument_);
		}
	}

	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cIndexArgument_.m_bForceGetBitSet);

	// divide operands according to index traits
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	if (cCheckArgument.isSearchBitSet(cEnvironment_)) {
		// getbybitset is used as searchbybitset
		; _SYDNEY_ASSERT(Opt::IsAll(cCheckArgument.m_vecBitSet,
									boost::bind(&ChosenInterface::isSearchByBitSet,
												_1)));
		cCheckArgument.m_vecSearchBitSet.insert(cCheckArgument.m_vecSearchBitSet.begin(),
												cCheckArgument.m_vecBitSet.begin(),
												cCheckArgument.m_vecBitSet.end());
		cCheckArgument.m_vecBitSet.clear();
	} else {
		// searchbybitset cannot be processed
		cCheckArgument.m_vecIndexScan.insert(cCheckArgument.m_vecIndexScan.begin(),
											 cCheckArgument.m_vecSearchBitSet.begin(),
											 cCheckArgument.m_vecSearchBitSet.end());
		cCheckArgument.m_vecSearchBitSet.clear();
	}

	if (cCheckArgument.m_vecFetch.ISEMPTY() == false) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Execution::Interface::IIterator* pResult = 0;

	if (cCheckArgument.m_vecNeedScan.ISEMPTY() == false) {
		Candidate::AdoptIndexArgument::Element& cElement =
			cIndexArgument_.getElement(cArgument_.m_pTable);
		if (cElement.m_iSearchBitSetID >= 0) {
			// use bitset
			pResult = createBitSetIterator(cEnvironment_,
										   cProgram_,
										   cArgument_,
										   cIndexArgument_,
										   cElement.m_iSearchBitSetID);

			return MAKEPAIR(pResult,
							static_cast<Interface::IPredicate*>(this));
		} else if (cArgument_.m_pScanFile) {
			return createScanByScan(cEnvironment_,
									cProgram_,
									cArgument_,
									cIndexArgument_);
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

	} else if (cCheckArgument.m_vecBitSet.ISEMPTY() == false) {
		// create iterator to get bitset
		PAIR<Execution::Interface::IIterator*, int> cGetBitSet =
			createGetBitSet(cEnvironment_,
							cProgram_,
							cArgument_,
							cIndexArgument_,
							cCheckArgument);

		// iterate on the bitset
		pResult = createBitSetIterator(cEnvironment_,
									   cProgram_,
									   cArgument_,
									   cIndexArgument_,
									   cGetBitSet.second);

		if (cGetBitSet.first) {
			// add start up action to create bitset
			pResult->addCalculation(cProgram_,
									Execution::Operator::Iterate::Once::create(
												   cProgram_,
												   pResult,
												   cGetBitSet.first->getID()),
									Execution::Action::Argument::Target::StartUp);
		}
	}

	if (pResult == 0
		&& cCheckArgument.m_vecIndexScan.GETSIZE() == 1) {
		// only one operand
		return cCheckArgument.m_vecIndexScan[0]->createScan(cEnvironment_,
															cProgram_,
															cArgument_,
															cIndexArgument_);
	}

	if (cCheckArgument.m_vecIndexScan.ISEMPTY() == false) {

		// create union distinct plan
		pResult = createScanByUnion(cEnvironment_,
									cProgram_,
									cArgument_,
									cIndexArgument_,
									pResult,
									cCheckArgument.m_vecIndexScan);

	} else if (cCheckArgument.isSearchBitSet(cEnvironment_)) {

		// create union distinct plan
		pResult = createScanByUnion(cEnvironment_,
									cProgram_,
									cArgument_,
									cIndexArgument_,
									pResult,
									cCheckArgument.m_vecSearchBitSet);
	}

	return MAKEPAIR(pResult,
					static_cast<Interface::IPredicate*>(0));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::createGetBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	PAIR<Execution::Interface::IIterator*, int>
//
// EXCEPTIONS

//virtual
PAIR<Execution::Interface::IIterator*, int>
ChosenImpl::OrImpl::
createGetBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->createGetBitSet(cEnvironment_,
																		   cProgram_,
																		   cArgument_,
																		   cIndexArgument_,
																		   cCheckArgument_);
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	if (cIndexArgument_.m_bForceGetBitSet
		&& cCheckArgument.isOnlyBitSet() == false) {
		return createGetBitSetByScan(cEnvironment_,
									 cProgram_,
									 cArgument_,
									 cIndexArgument_,
									 cCheckArgument_);
	}

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);

	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);
	Candidate::AdoptIndexArgument::Element& cMyElement =
		cMyIndexArgument.getElement(cArgument_.m_pTable);
	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

	if (cCheckArgument.m_vecBitSet.GETSIZE() == 1
		&& cCheckArgument.m_vecBitSet[0] != this) {
		return cCheckArgument.m_vecBitSet[0]->createGetBitSet(cEnvironment_,
															  cProgram_,
															  cArgument_,
															  cMyIndexArgument,
															  cCheckArgument_);
	} else {
		// create loop iterator
		Execution::Interface::IIterator* pIterator =
			Execution::Iterator::Loop::Once::create(cProgram_);

		// add action to union bitset
		createBitSet(cEnvironment_,
					 cProgram_,
					 pIterator,
					 cArgument_,
					 cMyIndexArgument,
					 cCheckArgument_);

		int iBitSetID =  cMyElement.m_iBitSetID;

		; _SYDNEY_ASSERT(iBitSetID >= 0);

		if (cCheckArgument.isOnlyBitSet()) {
			// propagate bitsetid to caller
			cElement.m_iBitSetID = iBitSetID;
		}

		return MAKEPAIR(pIterator, iBitSetID);
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::createBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::OrImpl::
createBitSet(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 Candidate::AdoptIndexArgument& cIndexArgument_,
			 Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		return getCombinedFile()->getPredicateInterface()->createBitSet(
								cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								cIndexArgument_,
								cCheckArgument_);
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(cArgument_.m_pTable);
	cElement.m_bForceBitSet = true;

	if (cElement.m_iPrevBitSetID >= 0) {
		// add merge bitset instead of creating bitset
		addUnionBitSet(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   cArgument_,
					   cIndexArgument_,
					   cCheckArgument_);

		return cElement.m_iPrevBitSetID;

	} else if (cArgument_.m_pTable->isNeedLock(cEnvironment_) == false
			   && cArgument_.m_eTarget != Candidate::AdoptArgument::Target::Parallel) {
		// if no lock needed, use parallel plan
		return createBitSetParallel(cEnvironment_,
									cProgram_,
									pIterator_,
									cArgument_,
									cIndexArgument_,
									cCheckArgument);

	} else {
		Candidate::AdoptIndexArgument cMyIndexArgument;
		cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;

		Candidate::AdoptIndexArgument::Element& cMyElement =
			cMyIndexArgument.getElement(cArgument_.m_pTable);
		// propagate current search-by-bitset ID
		cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;
		cMyElement.m_bForceBitSet = true;

		// create action to create initial bitset
		cCheckArgument.m_vecBitSet[0]->createBitSet(cEnvironment_,
													cProgram_,
													pIterator_,
													cArgument_,
													cMyIndexArgument,
													cCheckArgument_);
		; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

		// add merge action to this iterator
		FOREACH(cCheckArgument.m_vecBitSet.begin() + 1,
				cCheckArgument.m_vecBitSet.end(),
				boost::bind(&ChosenInterface::addUnionBitSet,
							_1,
							boost::ref(cEnvironment_),
							boost::ref(cProgram_),
							pIterator_,
							boost::ref(cArgument_),
							boost::ref(cMyIndexArgument),
							boost::ref(cCheckArgument_)));
		if (cIndexArgument_.m_bForceGetBitSet == false) {
			cCheckArgument.m_vecBitSet.clear();
		}
		// set result
		cElement.m_iBitSetID = cMyElement.m_iBitSetID;

		return cElement.m_iBitSetID;
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addMergeBitSet(cEnvironment_,
																   cProgram_,
																   pIterator_,
																   cArgument_,
																   cIndexArgument_,
																   cCheckArgument_);
		return;
	}

	Candidate::Table* pTable = cArgument_.m_pTable;
	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument_.getElement(pTable);
	; _SYDNEY_ASSERT(cElement.m_iPrevBitSetID >= 0);

	// check bitset is empty
	Execution::Interface::IAction* pIsEmpty =
		Execution::Predicate::IsEmpty::BitSet::create(cProgram_,
													  pIterator_,
													  cElement.m_iPrevBitSetID);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsEmpty->getID(),
											  cArgument_.m_eTarget));

	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;
	Candidate::AdoptIndexArgument::Element& cMyElement = cMyIndexArgument.getElement(pTable);

	// set current bitset ID as search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iPrevBitSetID;

	// add union action
	createBitSet(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 cMyIndexArgument,
				 cCheckArgument_);

	; _SYDNEY_ASSERT(cMyElement.m_iBitSetID >= 0);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::BitSet::Intersect::create(
										  cProgram_,
										  pIterator_,
										  cElement.m_iPrevBitSetID,
										  cMyElement.m_iBitSetID,
										  pIterator_->getLocker(cProgram_)),
							   cArgument_.m_eTarget);
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::addUnionBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
addUnionBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addUnionBitSet(cEnvironment_,
																   cProgram_,
																   pIterator_,
																   cArgument_,
																   cIndexArgument_,
																   cCheckArgument_);
		return;
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);

	// add union action to this iterator
	FOREACH(cCheckArgument.m_vecBitSet.begin(),
			cCheckArgument.m_vecBitSet.end(),
			boost::bind(&ChosenInterface::addUnionBitSet,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						boost::ref(cIndexArgument_),
						boost::ref(cCheckArgument_)));
	if (cIndexArgument_.m_bForceGetBitSet == false) {
		cCheckArgument.m_vecBitSet.clear();
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::addExceptBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
addExceptBitSet(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Candidate::AdoptIndexArgument& cIndexArgument_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	if (getCombinedFile()) {
		getCombinedFile()->getPredicateInterface()->addExceptBitSet(cEnvironment_,
																	cProgram_,
																	pIterator_,
																	cArgument_,
																	cIndexArgument_,
																	cCheckArgument_);
		return;
	}

	// divide operands according to index traits
	Candidate::CheckIndexArgument& cCheckArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cCheckArgument);

	; _SYDNEY_ASSERT(cCheckArgument.m_vecBitSet.ISEMPTY() == false);

	// add merge action to this iterator
	FOREACH(cCheckArgument.m_vecBitSet.begin(),
			cCheckArgument.m_vecBitSet.end(),
			boost::bind(&ChosenInterface::addExceptBitSet,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pIterator_,
						boost::ref(cArgument_),
						boost::ref(cIndexArgument_),
						boost::ref(cCheckArgument_)));
	if (cIndexArgument_.m_bForceGetBitSet == false) {
		cCheckArgument.m_vecBitSet.clear();
	}
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	// check operands
	Candidate::CheckIndexArgument& cTmpArgument =
		getCheckIndexArgument(cCheckArgument_.m_bIgnoreField);
	checkOperandIndex(cEnvironment_, cProgram_, cArgument_, cTmpArgument);

	if (cCheckArgument_.m_bIgnoreField == false
		&& getCombinedFile()
		&& getCombinedFile()->getTable()->getOrder()
		&& getCombinedFile()->getTable()->getOrder()->isChosen()) {
		// No.2115
		// this file is used for ordering
		getCombinedFile()->checkIndex(cEnvironment_,
									  cCheckArgument_);
		; _SYDNEY_ASSERT(getCombinedFile()->hasOrder());

		if (getCombinedFile()->getTable()->getOrder()->getChosen()->getPredicate() == this) {
			cCheckArgument_.m_pOrderScan = this;
		} else {
			getCombinedFile()->addCheckIndexResult(cEnvironment_,
												   cCheckArgument_,
												   this);
		}
	} else if (cTmpArgument.isSearchBitSet(cEnvironment_)) {
		// All the operands can search by bitset
		cCheckArgument_.m_vecSearchBitSet.PUSHBACK(this);

	} else {
		// searchbybitset can't be processed in or
		cTmpArgument.m_vecIndexScan.insert(cTmpArgument.m_vecIndexScan.begin(),
										   cTmpArgument.m_vecSearchBitSet.begin(),
										   cTmpArgument.m_vecSearchBitSet.end());
		cTmpArgument.m_vecSearchBitSet.clear();

		if (cTmpArgument.m_vecNeedScan.ISEMPTY() == false
			|| cTmpArgument.m_vecFetch.ISEMPTY() == false) {
			cCheckArgument_.m_vecNeedScan.PUSHBACK(this);

		} else if (cTmpArgument.m_vecBitSet.ISEMPTY() == false) {

			if (cTmpArgument.m_vecIndexScan.ISEMPTY()) {
				// vecBitSetNot can be not-empty

				// insert this to bitset predicates
				Candidate::File::insertCheckIndexResult(cEnvironment_,
														this,
														&cCheckArgument_.m_vecBitSet,
														0);
			} else {
				cCheckArgument_.m_vecIndexScan.PUSHBACK(this);
			}
		} else {
			cCheckArgument_.m_vecIndexScan.PUSHBACK(this);
		}
	}
	// require if needed
	requireIfNeeded(cEnvironment_, cArgument_.m_pTable, cCheckArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::OrImpl::checkLimit -- 
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
ChosenImpl::OrImpl::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	if (getFile(cEnvironment_)) {
		(void)getFile(cEnvironment_)->checkLimit(cEnvironment_,
												 cLimit_);
	} else {
		// each element of OR can check limit
		AccessPlan::Limit cLimit(cLimit_);
		cLimit.setIntermediate();
		Opt::ForEach(getOperand(),
					 boost::bind(&Predicate::ChosenInterface::checkLimit,
								 boost::bind(&Interface::IPredicate::getChosen,
											 _1),
								 boost::ref(cEnvironment_),
								 boost::cref(cLimit)));
	}
	// anyway, this method returns false
	// because limit is not entirely processed by the file
	return false;
}

// FUNCTION protected
//	Predicate::ChosenImpl::OrImpl::getCheckIndexArgument -- 
//
// NOTES
//
// ARGUMENTS
//	bool bIgnoreField_
//
// RETURN
//	Candidate::CheckIndexArgument&
//
// EXCEPTIONS

//virtual
Candidate::CheckIndexArgument&
ChosenImpl::OrImpl::
getCheckIndexArgument(bool bIgnoreField_)
{
	Candidate::CheckIndexArgument& cResult =
		Super::getCheckIndexArgument(bIgnoreField_);
	cResult.setOr();
	return cResult;
}

// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::createScanByUnion -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Execution::Interface::IIterator* pGetBitSet_
//	const VECTOR<ChosenInterface*>& vecChosen_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
ChosenImpl::OrImpl::
createScanByUnion(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_,
				  Execution::Interface::IIterator* pGetBitSet_,
				  const VECTOR<ChosenInterface*>& vecChosen_)
{
	if (cArgument_.m_pTable->getRowID() == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// create union iterator
	Execution::Interface::IIterator* pResult = Execution::Iterator::UnionDistinct::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	if (pGetBitSet_) {
		// add as operand
		addUnionOperand(cEnvironment_,
						cProgram_,
						pGetBitSet_,
						cArgument_,
						pResult,
						-1,
						0,
						0);
	}

	// set key data at the first element of union result tuple
	VECTOR<int> vecOutID;
	VECTOR<int> vecKey;
	vecKey.PUSHBACK(cArgument_.m_pTable->getRowID()->generateFromType(cEnvironment_,
																	  cProgram_,
																	  pResult,
																	  cArgument_));
	vecOutID.PUSHBACK(cProgram_.addVariable(vecKey));

	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);

	// add union operand and output data for all index scan operands
	VECTOR<Predicate::ChosenInterface*>::CONSTITERATOR iterator = vecChosen_.begin();
	const VECTOR<Predicate::ChosenInterface*>::CONSTITERATOR last = vecChosen_.end();
	for (; iterator != last; ++iterator) {
		Candidate::AdoptIndexArgument cMyIndexArgument;
		cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;

		Candidate::AdoptIndexArgument::Element& cMyElement =
			cMyIndexArgument.getElement(cArgument_.m_pTable);
		cMyElement.m_eOrder = Candidate::AdoptIndexArgument::Order::ByRowID;
		cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;

		PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cScan =
			(*iterator)->createScan(cEnvironment_,
									cProgram_,
									cArgument_,
									cMyIndexArgument);
		Execution::Interface::IIterator* pIterator = cScan.first;
		if (cScan.second) {
			addCheckPredicate(cEnvironment_,
							  cProgram_,
							  pIterator,
							  cArgument_,
							  cIndexArgument_,
							  cScan.second);
		}
		addUnionOperand(cEnvironment_,
						cProgram_,
						pIterator,
						cArgument_,
						pResult,
						-1,
						&vecOutID,
						*iterator);
	}

	; _SYDNEY_ASSERT(vecOutID.ISEMPTY() == false);

	// set outdata
	int iResult = cProgram_.addVariable(vecOutID);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResult));

	return pResult;
}

// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::addUnionOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pOperand_
//	Candidate::AdoptArgument& cArgument_
//	Execution::Interface::IIterator* pUnion_
//	int iKeyID_
//	VECTOR<int>* pvecOutID_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::OrImpl::
addUnionOperand(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pOperand_,
				Candidate::AdoptArgument& cArgument_,
				Execution::Interface::IIterator* pUnion_,
				int iKeyID_,
				VECTOR<int>* pvecOutID_,
				Interface::IPredicate* pPredicate_)
{
	if (iKeyID_ < 0) {
		// rowid data in operand
		int iRowID = cArgument_.m_pTable->getRowID()->generate(cEnvironment_,
															   cProgram_,
															   pOperand_,
															   cArgument_);
		// add operand using rowid as input data
		VECTOR<int> vecTmp(1, iRowID); // data used in input should be array
		iKeyID_ = cProgram_.addVariable(vecTmp);
	}

	pUnion_->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pOperand_->getID(),
										iKeyID_));
	if (pvecOutID_ == 0) {
		// no input data other than rowid
		pUnion_->addAction(cProgram_,
						   _ACTION_ARGUMENT0(InData));
	} else {
		; _SYDNEY_ASSERT(pPredicate_);

		// get retrieved field
		Utility::FieldSet& cFieldSet = m_mapRetrieveField[pPredicate_];
		if (cFieldSet.isEmpty()) {
			// no input data other than rowid
			pUnion_->addAction(cProgram_,
							   _ACTION_ARGUMENT0(InData));
		} else {
			// generate retrieved field in operand
			VECTOR<int> vecOperandData;
			cFieldSet.mapElement(vecOperandData,
								 boost::bind(&Scalar::Field::generate,
											 _1,
											 boost::ref(cEnvironment_),
											 boost::ref(cProgram_),
											 pOperand_,
											 boost::ref(cArgument_)));
			// generate retrieved field by type in union
			VECTOR<int> vecData;
			cFieldSet.mapElement(vecData,
								 boost::bind(&Scalar::Field::generateFromType,
											 _1,
											 boost::ref(cEnvironment_),
											 boost::ref(cProgram_),
											 pUnion_,
											 boost::ref(cArgument_)));
			VECTOR<int> vecAlternative;
			cFieldSet.mapElement(vecAlternative,
								 boost::bind(&Scalar::Field::generateAlternative,
											 _1,
											 boost::ref(cEnvironment_),
											 boost::ref(cProgram_),
											 pOperand_,
											 boost::ref(cArgument_)));

			// set input data
			pUnion_->addAction(cProgram_,
							   _ACTION_ARGUMENT3(InData,
												 cProgram_.addVariable(vecOperandData),
												 cProgram_.addVariable(vecData),
												 cProgram_.addVariable(vecAlternative)));

			// add each element to out data
			pvecOutID_->insert(pvecOutID_->end(), vecData.begin(), vecData.end());
		}
	}
}
 
// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::createCheckPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	VECTOR<int>& vecAction_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
ChosenImpl::OrImpl::
createCheckPredicate(Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 VECTOR<int>& vecAction_)
{
	Execution::Interface::IPredicate* pResult =
		Execution::Predicate::Combinator::Or::create(cProgram_,
													 pIterator_,
													 vecAction_);
	return pResult->getID();
}

// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::createBitSetParallel -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
ChosenImpl::OrImpl::
createBitSetParallel(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Candidate::AdoptArgument& cArgument_,
					 Candidate::AdoptIndexArgument& cIndexArgument_,
					 Candidate::CheckIndexArgument& cCheckArgument_)
{
	; _SYDNEY_ASSERT(!cCheckArgument_.m_vecBitSet.ISEMPTY());

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(BeginParallel,
											  cArgument_.m_eTarget));

	VECTOR<int> vecBitSetID;
	// add parallel action to this iterator
	Opt::MapContainer(cCheckArgument_.m_vecBitSet,
					  vecBitSetID,
					  boost::bind(&This::addParallelBitSet,
								  this,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_),
								  boost::ref(cIndexArgument_),
								  boost::ref(cCheckArgument_),
								  _1));
	; _SYDNEY_ASSERT(cCheckArgument_.m_vecBitSet.GETSIZE() == vecBitSetID.GETSIZE());

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndParallel,
											  cArgument_.m_eTarget));

	// use first bitset as the result
	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);
	cElement.m_iBitSetID = vecBitSetID[0];

	// create union action for each bitset
	FOREACH(vecBitSetID.begin() + 1,
			vecBitSetID.end(),
			boost::bind(&Execution::Interface::IIterator::addCalculation,
						pIterator_,
						boost::ref(cProgram_),
						boost::bind(&Execution::Operator::BitSet::Union::create,
									boost::ref(cProgram_),
									pIterator_,
									cElement.m_iBitSetID,
									_1,
									-1),
						cArgument_.m_eTarget));

	return cElement.m_iBitSetID;
}

// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::addParallelBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	ChosenInterface* pChosen_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
ChosenImpl::OrImpl::
addParallelBitSet(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_,
				  Candidate::AdoptIndexArgument& cIndexArgument_,
				  Candidate::CheckIndexArgument& cCheckArgument_,
				  ChosenInterface* pChosen_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Candidate::AdoptArgument::Target::Parallel;

	Candidate::AdoptIndexArgument cMyIndexArgument;

	Candidate::AdoptIndexArgument::Element& cElement =
		cIndexArgument_.getElement(cArgument_.m_pTable);
	Candidate::AdoptIndexArgument::Element& cMyElement =
		cMyIndexArgument.getElement(cArgument_.m_pTable);

	// propagate current search-by-bitset ID
	cMyElement.m_iSearchBitSetID = cElement.m_iSearchBitSetID;
	cMyElement.m_bForceBitSet = true;

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(ParallelList,
											  cArgument_.m_eTarget));

	// get bitset
	int iBitSetID = pChosen_->createBitSet(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cMyArgument,
										   cMyIndexArgument,
										   cCheckArgument_);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(ReturnParallelData,
											  iBitSetID,
											  cArgument_.m_eTarget));
	return iBitSetID;
}

// FUNCTION private
//	Predicate::ChosenImpl::OrImpl::accumulateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::OrImpl::
accumulateCost(Opt::Environment& cEnvironment_,
			   Interface::IPredicate* pPredicate_,
			   boost::function<bool(Interface::IPredicate*, AccessPlan::Cost&)> cFunction_,
			   AccessPlan::Cost& cResult_)
{
	AccessPlan::Cost cMyCost;
	cMyCost.setIsSetCount(cResult_.isSetCount());
	cMyCost.setIsSetRate(cResult_.isSetRate());
	cMyCost.setTupleCount(cResult_.getTupleCount());
	cMyCost.setTableCount(cResult_.getTableCount());

	if (cFunction_(pPredicate_, boost::ref(cMyCost)) == false) {
		// use estimate instead
		if (pPredicate_->estimateCost(cEnvironment_,
									  cMyCost) == false) {
			return;
		}
	}
	if (cResult_.isInfinity()) {
		cResult_ = cMyCost;
	} else {
		AccessPlan::Cost::Value cOverhead = cResult_.getOverhead() + cMyCost.getOverhead();
		AccessPlan::Cost::Value cRetrieveCost = cResult_.getRetrieveCost() + cMyCost.getRetrieveCost();

		cResult_.setOverhead(cOverhead);
		if (cResult_.isSetRate()) {
			cResult_.setRate(_maxCost(cResult_.getRate(), cMyCost.getRate()));
		} else {
			cResult_.setRate(1);
		}
		if (cResult_.isSetCount()) {
			if (cResult_.getTupleCount() < cMyCost.getTupleCount()) {
				cResult_.setTupleCount(cMyCost.getTupleCount());
				cResult_.setTotalCost(cMyCost.getTotalCost());
			}
		} else {
			cResult_.setTupleCount(AccessPlan::Cost::Value());
		}
		cResult_.setRetrieveCost(cRetrieveCost);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		if (cResult_.isSetCount()) {
			stream << "Accumulate cost(or):("
				   << Utility::Trace::toString(cEnvironment_, pPredicate_)
				   << ")\n  " << cResult_;
		} else {
			stream << "Accumulate rate(or):("
				   << Utility::Trace::toString(cEnvironment_, pPredicate_)
				   << ")\n  " << cResult_.getRate();
		}
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
}

////////////////////////////////////////
// Predicate::ChosenImpl::NotImpl

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::getFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::File*
//
// EXCEPTIONS

//virtual
Candidate::File*
ChosenImpl::NotImpl::
getFile(Opt::Environment& cEnvironment_)
{
	return m_pOperand->getChosen()->getFile(cEnvironment_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::setFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
setFile(Opt::Environment& cEnvironment_,
		Candidate::File* pFile_)
{
	m_pOperand->getChosen()->setFile(cEnvironment_, pFile_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::rechoose -- 
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
ChosenImpl::NotImpl::
rechoose(Opt::Environment& cEnvironment_,
		 ChooseArgument& cArgument_)
{
	Interface::IPredicate* pRechosen =
		m_pOperand->getChosen()->rechoose(cEnvironment_,
										  cArgument_);
	if (pRechosen != m_pOperand) {
		return ChosenInterface::create(cEnvironment_,
									   getPredicate(),
									   getNotChecked(),
									   pRechosen);
	}
	return this;
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::addLockPenalty -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	const AccessPlan::Cost& cScanCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
addLockPenalty(Opt::Environment& cEnvironment_,
			   AccessPlan::Cost& cCost_,
			   const AccessPlan::Cost& cScanCost_)
{
	AccessPlan::Cost cCostSave(cCost_);
	cCostSave.setTupleCount(cCost_.getTableCount());

	// NotのRateとTupleCountをオペランドのものに変換
	convertRate(cCostSave, cCost_);

	// オペランドのLockPenaltyを計算
	m_pOperand->getChosen()->addLockPenalty(cEnvironment_,
											cCost_,
											cScanCost_);

	// RateとTupleCountをNotのものに再変換
	convertRate(cCostSave, cCost_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::isNeedScan -- 
//
// NOTES
//
// ARGUMENTS
//	CheckNeedScanArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::NotImpl::
isNeedScan(CheckNeedScanArgument& cArgument_)
{
	if (cArgument_.m_bInAnd
		&& cArgument_.m_bNeedScan == false) {
		CheckNeedScanArgument cArgument(cArgument_, false /* no top */);
		return m_pOperand->getChosen()->isNeedScan(cArgument);
	}
	return true;
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::isNeedRetrieve -- 
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
ChosenImpl::NotImpl::
isNeedRetrieve()
{
	return m_pOperand->getChosen()->isNeedRetrieve();
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::isAbleToProcessByBitSet -- 
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
ChosenImpl::NotImpl::
isAbleToProcessByBitSet()
{
	return m_pOperand->getChosen()->isAbleToGetByBitSet();
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::createCheck -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenImpl::NotImpl::
createCheck(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Candidate::AdoptIndexArgument& cIndexArgument_)
{
	Candidate::AdoptIndexArgument cMyIndexArgument;
	cMyIndexArgument.m_bForceGetBitSet = cIndexArgument_.m_bForceGetBitSet;

	// generate operand
	int iOperandID = getOperand()->getChosen()->createCheck(cEnvironment_,
															cProgram_,
															pIterator_,
															cArgument_,
															cMyIndexArgument);
	Execution::Interface::IAction* pResult =
		Execution::Predicate::Combinator::Not::create(cProgram_,
													  pIterator_,
													  iOperandID);
	return pResult->getID();
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::addMergeBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::AdoptIndexArgument& cIndexArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
addMergeBitSet(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   Candidate::AdoptIndexArgument& cIndexArgument_,
			   Candidate::CheckIndexArgument& cCheckArgument_)
{
	getOperand()->getChosen()->addExceptBitSet(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_,
											   cIndexArgument_,
											   cCheckArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::checkIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
checkIndex(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Candidate::CheckIndexArgument& cCheckArgument_)
{
	Candidate::CheckIndexArgument cOperandCheckResult;
	cOperandCheckResult.m_bTop = false;
	getOperand()->getChosen()->checkIndex(cEnvironment_,
										  cProgram_,
										  cArgument_,
										  cOperandCheckResult);
	if (cOperandCheckResult.m_vecNeedScan.ISEMPTY() == false) {
		// whole operand needs scan
		cCheckArgument_.m_vecNeedScan.PUSHBACK(this);

	} else if (cOperandCheckResult.m_vecBitSet.ISEMPTY() == false) {

		if (cCheckArgument_.m_bInAnd
			&& cCheckArgument_.m_vecBitSet.ISEMPTY() == false
			&& cOperandCheckResult.m_vecSearchBitSet.ISEMPTY()
			&& cOperandCheckResult.m_vecIndexScan.ISEMPTY()
			&& cOperandCheckResult.m_vecFetch.ISEMPTY()) {

			// NOT is inserted to last element anyway
			cCheckArgument_.m_vecBitSet.PUSHBACK(this);
#if 0
		} else if (cCheckArgument_.m_bInOr) {
			cCheckArgument_.m_vecIndexScan.PUSHBACK(this);
#endif
		} else {
			cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
		}
	} else {
		cCheckArgument_.m_vecNeedScan.PUSHBACK(this);
	}
	// require if needed
	requireIfNeeded(cEnvironment_, cArgument_.m_pTable, cCheckArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::isIndexAvailable -- 
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
ChosenImpl::NotImpl::
isIndexAvailable(Opt::Environment& cEnvironment_)
{
	return m_pOperand->getChosen()->isIndexAvailable(cEnvironment_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::requireIfNeeded -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Candidate::CheckIndexArgument& cCheckArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
requireIfNeeded(Opt::Environment& cEnvironment_,
				Interface::ICandidate* pCandidate_,
				Candidate::CheckIndexArgument& cCheckArgument_)
{
	getOperand()->getChosen()->requireIfNeeded(cEnvironment_, pCandidate_, cCheckArgument_);
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::setScanBetter -- 
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
ChosenImpl::NotImpl::
setScanBetter()
{
	getOperand()->getChosen()->setScanBetter();
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::isScanBetter -- 
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
ChosenImpl::NotImpl::
isScanBetter()
{
	return getOperand()->getChosen()->isScanBetter();
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::NotImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cCost_)
{
	AccessPlan::Cost cOperandCost;
	cOperandCost.setIsSetCount(cCost_.isSetCount());
	cOperandCost.setIsSetRate(cCost_.isSetRate());
	cOperandCost.setTupleCount(cCost_.getTupleCount());
	cOperandCost.setTableCount(cCost_.getTableCount());

	if (m_pOperand->getChosen()->getEstimateCost(cEnvironment_, cOperandCost)) {

		; _SYDNEY_ASSERT(cCost_.getTableCount().isInfinity() == false);
		cCost_.setTupleCount(cCost_.getTableCount());

		convertRate(cCost_, cOperandCost);
		cCost_ = cOperandCost;
		return true;
	}
	return false;
}

// FUNCTION public
//	Predicate::ChosenImpl::NotImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ChosenImpl::NotImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	getOperand()->require(cEnvironment_, pCandidate_);
}

// FUNCTION private
//	Predicate::ChosenImpl::NotImpl::calculateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ChosenImpl::NotImpl::
calculateCost(Opt::Environment& cEnvironment_,
			  AccessPlan::Cost& cCost_)
{
	AccessPlan::Cost cOperandCost;
	cOperandCost.setIsSetCount(cCost_.isSetCount());
	cOperandCost.setIsSetRate(cCost_.isSetRate());
	cOperandCost.setTupleCount(cCost_.getTupleCount());
	cOperandCost.setTableCount(cCost_.getTableCount());
	if (m_pOperand->getChosen()->getCost(cEnvironment_, cOperandCost)) {
		convertRate(cCost_, cOperandCost);
		cCost_ = cOperandCost;
		return true;
	}
	return false;
}

// FUNCTION private
//	Predicate::ChosenImpl::NotImpl::convertRate -- 
//
// NOTES
//
// ARGUMENTS
//	const AccessPlan::Cost& cCost_
//	AccessPlan::Cost& cOperandCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ChosenImpl::NotImpl::
convertRate(const AccessPlan::Cost& cCost_,
			AccessPlan::Cost& cOperandCost_)
{
	if (cCost_.isSetRate()) {
		cOperandCost_.setRate(1.0 - cOperandCost_.getRate().get());
	} else {
		cOperandCost_.setRate(1);
	}
	if (cCost_.isSetCount()) {
		cOperandCost_.setTupleCount(_maxCost(1,
											 cCost_.getTupleCount() - cOperandCost_.getTupleCount()));
	} else {
		cOperandCost_.setTupleCount(AccessPlan::Cost::Value());
	}
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
