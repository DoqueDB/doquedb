// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/TableImpl.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/TableImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/File/Argument.h"
#include "Plan/File/Parameter.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Predicate/Comparison.h"
#include "Plan/Predicate/Fetch.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Operation.h"
#include "Plan/Scalar/UpdateField.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Tree/Fetch.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/ForeignKeyViolation.h"
#include "Exception/InvalidFullTextUsage.h"
#include "Exception/NotSupported.h"
#include "Exception/NullabilityViolation.h"
#include "Exception/ReferedKeyViolation.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Collection/Connection.h"
#include "Execution/Collection/Queue.h"
#include "Execution/Collection/Store.h"
#include "Execution/Collection/VirtualTable.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/BitSet.h"
#include "Execution/Iterator/File.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Iterator/Loop.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Locker.h"
#include "Execution/Operator/Logger.h"
#include "Execution/Operator/Transaction.h"
#include "Execution/Operator/Throw.h"
#include "Execution/Operator/UndoLog.h"
#include "Execution/Predicate/Comparison.h"
#include "Execution/Predicate/IsEmpty.h"
#include "Execution/Utility/Transaction.h"

#include "Lock/Name.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/OpenOption.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/LogData.h"
#include "Opt/Sort.h"
#include "Opt/UndoLog.h"

#include "Schema/Constraint.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Table.h"

#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

namespace
{
	// CLASS local
	// _FieldElementMap --
	//
	// NOTES
	//	This class is used to avoid method ambiguity.
	class _FieldElementMap
		: public MAP<unsigned int, int, LESS<unsigned int> >
	{
	public:
		void addElement(unsigned int iKey_, int iValue_)
		{insert(iKey_, iValue_);}
	};

#ifndef NO_TRACE
	void _addString(OSTRSTREAM& stream_,
					const STRING& string_)
	{
		if (!stream_.isEmpty()) {
			stream_ << ',';
		}
		stream_ << string_;
	}
	STRING _getCorrelationName(Opt::Environment& cEnvironment_,
							   Interface::ICandidate* pCandidate_)
	{
		OSTRSTREAM stream;

		Utility::RelationSet cRelation;
		pCandidate_->createReferingRelation(cRelation);

		cRelation.foreachElement(boost::bind(&_addString,
											 boost::ref(stream),
											 boost::bind(&Interface::IRelation::getCorrelationName,
														 _1,
														 boost::ref(cEnvironment_))));
		return stream.getString();
	}
	STRING _getCorrelationNames(Opt::Environment& cEnvironment_,
								const VECTOR<Interface::ICandidate*>& vecCandidate_)
	{
		OSTRSTREAM stream;

		FOREACH(vecCandidate_, boost::bind(&_addString,
										   boost::ref(stream),
										   boost::bind(&_getCorrelationName,
													   boost::ref(cEnvironment_),
													   _1)));
		return stream.getString();
	}
#endif
}

//////////////////////////////////////////////////
//	Plan::Candidate::TableImpl::Retrieve

// FUNCTION public
//	Candidate::TableImpl::Retrieve::require -- add retrieved columns
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

void
TableImpl::Retrieve::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	if (pField_->getTable() == getTable()) {
		if (m_cRequiredColumn.isContaining(pField_) == false) {
			m_cRequiredColumn.add(pField_);
			if (m_bRowCreated == false) {
				// if pField has been set as delayable, it is canceled
				m_cDelayedColumn.remove(pField_);
			}
			retrieve(cEnvironment_,
					 pField_);
			m_cRetrieve.m_bChanged = true;
		}
	}
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::retrieve -- add retrieved columns
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

void
TableImpl::Retrieve::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	if (pField_->getTable() == getTable()
		&& m_cRetrievedColumn.isContaining(pField_) == false) {
		m_cRetrievedColumn.add(pField_);
		m_cRetrieve.m_bChanged = true;
		m_pScanFileCache = 0;
	}
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::use -- 
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
TableImpl::Retrieve::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	require(cEnvironment_,
			pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::delay -- add retrieved but delayable columns
//
// NOTES
//	If delayable columns are already required, it is not delayed.
//	When minimum is specified for delay-argument, it is delayed if it can.
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	
	if (pField_->getTable() == getTable()) {
		if (cArgument_.m_bMinimum == true
			|| pField_->isRowID()
			|| !m_cRequiredColumn.isContaining(pField_)) {
			m_cDelayedColumn.add(pField_);
			m_cRetrievedColumn.add(pField_);
			m_pScanFileCache = 0;

			cArgument_.m_cKey.add(pField_->getDelayedKey(cEnvironment_));
			return true;
		} else {
			// if field itself is not delayable, add as required
			m_cRequiredColumn.add(pField_);
			m_cRetrievedColumn.add(pField_);
			m_pScanFileCache = 0;
		}
	}
	// if field is not delayable or not related, false is returned
	return false;
}



// FUNCTION public
//	Candidate::TableImpl::Retrieve::isGetByBitSetRowID 
//
// NOTES
//	
//	
//
// ARGUMENTS
//	
//	
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
isGetByBitSetRowID()
{
	if(getOrder()  && getOrder()->isChosen()) {
		return getOrder()->isBitSetSort();
	}
	return false;
}




// FUNCTION public
//	Candidate::TableImpl::Retrieve::checkPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
checkPredicate(Opt::Environment& cEnvironment_,
			   AccessPlan::Source& cPlanSource_)
{
	if (Interface::IPredicate* pPredicate = cPlanSource_.getPredicate()) {
		Predicate::CheckArgument cCheckArgument(this,
												cPlanSource_.getPrecedingCandidate());
		setPredicate(pPredicate->check(cEnvironment_,
									   cCheckArgument));
		// don't require here
	}
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::isLimitAvailable -- 
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
TableImpl::Retrieve::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	if (getPredicate() && getPredicate()->isChosen()) {
		return getPredicate()->getChosen()->isLimitAvailable(cEnvironment_);
	}
	return false;
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Retrieve::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// choose retrieving file for each columns
	Utility::FieldSet& cTargetColumn = m_cDelayedColumn.isEmpty() ? m_cRetrievedColumn : m_cRequiredColumn;
 
	chooseRetrieve(cEnvironment_);
	chooseRetrieveFile(cEnvironment_,
					   cTargetColumn,
					   m_cRetrieve);

	cArgument_.m_pTable = this;
	cArgument_.m_pScanFile = chooseScan(cEnvironment_);
	cArgument_.setCandidate(this);

	// create iterator using chosen start file
	Execution::Interface::IIterator* pResult =
		adoptStartFile(cEnvironment_,
					   cProgram_,
					   cArgument_,
					   cTargetColumn,
					   m_cRetrieve);

	// generate required field
	cTargetColumn.foreachElement(boost::bind(&Interface::IScalar::generate,
											 _1,
											 boost::ref(cEnvironment_),
											 boost::ref(cProgram_),
											 pResult,
											 boost::ref(cArgument_)));

	// generate check predicate
	if (getPredicate()
		&& (getPredicate()->isChosen() || getPredicate()->isChecked())) {
		// if any predicate left, generate it
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	// add to refered table
	cEnvironment_.addReferedTable(getTable()->getSchemaTable());
 
	cArgument_.m_pTable = 0;
	cArgument_.m_pScanFile = 0;

	return pResult;
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::ICandidate::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::ICandidate::InquiryResult
TableImpl::Retrieve::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & InquiryArgument::Target::ReferTable) {
		if (getTable()->getSchemaTable() == cArgument_.m_pSchemaTable) {
			iResult |= InquiryArgument::Target::ReferTable;
		}
	}
	return iResult;
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::generateDelayed -- generate additional action to obtain delayed scalars
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Execution::Interface::IAction*
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	if (!m_cDelayedColumn.isEmpty()) {
		m_cRetrieve.clear();
		m_cRetrieve.m_bDelayed = true;
		chooseRetrieveFile(cEnvironment_,
						   m_cDelayedColumn,
						   m_cRetrieve);

		// set this as adopted candidate to relation object
		getTable()->setAdoptCandidate(this);

		Candidate::AdoptArgument cArgument;
		cArgument.m_pTable = this;

		m_cDelayedColumn.foreachElement(boost::bind(&Interface::IScalar::generate,
													_1,
													boost::ref(cEnvironment_),
													boost::ref(cProgram_),
													pIterator_,
													boost::ref(cArgument)));
	}
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// set retrieved columns as initial value
	setRetrievedColumn(cEnvironment_,
					   cFieldSet_);

	// enumerate possible combination of indices
	enumerate(cEnvironment_,
			  cPlanSource_);

	// select one candidate for the PlanSource by checking index validity
	choose(cEnvironment_,
		   cPlanSource_);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::isDelayed -- 
//
// NOTES
//
// ARGUMENTS
//	Scalar::Field* pField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Retrieve::
isDelayed(Scalar::Field* pField_)
{
	return m_cDelayedColumn.isContaining(pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::addFileFetch -- get file fetch operator for a fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Scalar::Field* pField_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
addFileFetch(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Scalar::Field* pField_,
			 Candidate::AdoptArgument& cArgument_)
{
	if (pField_->isRowID()) return false;

	// get file to obtain the field value
	Interface::IFile* pFile = m_cRetrieve.m_mapFieldRetrieve[pField_];
	if (pFile == 0) {
		if (m_cRetrieve.m_mapAlternativeValue.ISEMPTY() == false) {
			if (createAlternativeValue(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   pField_,
									   cArgument_,
									   m_cRetrieve)) {
				return true;
			}
		}
		// get from scan file
		return false;
	}

	return createFileFetch(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   pFile,
						   pField_,
						   cArgument_,
						   m_cRetrieve);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::addConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
addConstraint(Opt::Environment& cEnvironment_,
			  Schema::Constraint* pSchemaConstraint_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::getConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const VECTOR<Schema::Constraint*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Schema::Constraint*>&
TableImpl::Retrieve::
getConstraint()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::addRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidateFile_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
addRetrieveField(Opt::Environment& cEnvironment_,
				 Candidate::File* pCandidateFile_,
				 Interface::IFile* pFile_,
				 Scalar::Field* pField_)
{
	addRetrieveField(cEnvironment_,
					 pCandidateFile_,
					 pFile_,
					 pField_,
					 m_cRetrieve);
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::getEstimateCount -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
TableImpl::Retrieve::
getEstimateCount(Opt::Environment& cEnvironment_)
{
	AccessPlan::Cost::Value cResult = getTable()->getEstimateCount();
	if (cResult.isInfinity()) {
		cResult = calculateScanCost(cEnvironment_).getTupleCount();
		if (cResult.isInfinity()) {
			// this table is not normal -> treat as one
			cResult = 1;
		}
		getTable()->setEstimateCount(cResult);
	}
	return cResult;
}

// FUNCTION protected
//	Candidate::TableImpl::Retrieve::setRetrievedColumn -- set retrieved columns
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
setRetrievedColumn(Opt::Environment& cEnvironment_,
				   const Utility::FieldSet& cFieldSet_)
{
	m_cRetrievedColumn = cFieldSet_;
	m_pScanFileCache = 0;

	if (cFieldSet_.isAll(boost::bind(&Scalar::Field::isRowIDAvailable,
									 _1,
									 boost::ref(cEnvironment_)))) {
		// require rowid
		Interface::IScalar* pScalar = getTable()->getScalar(cEnvironment_, 0);
		; _SYDNEY_ASSERT(pScalar->isField());
		require(cEnvironment_,
				pScalar->getField());

	} else if (cFieldSet_.isAny(boost::bind(&Scalar::Field::isRowIDAvailable,
											_1,
											boost::ref(cEnvironment_)))) {
		// fields which require rowid and ones not require
		// cannot be specified together.
		_SYDNEY_THROW0(Exception::InvalidFullTextUsage);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::enumerate -- enumerate possible plans
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccussPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
enumerate(Opt::Environment& cEnvironment_,
		  AccessPlan::Source& cPlanSource_)
{
	// create checked predicate
	checkPredicate(cEnvironment_,
				   cPlanSource_);

	// create checked sorting order
	if (Order::Specification* pOrder = cPlanSource_.getOrder()) {
		Order::CheckArgument cCheckArgument(this,
											cPlanSource_.getPrecedingCandidate(),
											cPlanSource_.isCheckPartial(),
											cPlanSource_.isGrouping());

		if (checkOrder(cEnvironment_,
					   pOrder,
					   cCheckArgument) == false) {
			// can't process order
			// -> set key columns as required
			pOrder->require(cEnvironment_, this);

			// erase limit specification
			cPlanSource_.eraseLimit();
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::choose -- choose index to be used
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
choose(Opt::Environment& cEnvironment_,
	   AccessPlan::Source& cPlanSource_)
{
	// choose index files for predicate
	if (getPredicate()) {
		choosePredicate(cEnvironment_, cPlanSource_);
	}

	// choose index file for order
	if (getOrder() && getOrder()->isChecked()) {
		chooseOrder(cEnvironment_,
					cPlanSource_);
		if (getOrder() == 0
			|| (cPlanSource_.getOrder()->getPartitionKey().ISEMPTY() == false
				&& Order::Specification::hasSamePartitionKey(getOrder(),
															 cPlanSource_.getOrder()) == false)) {
			// order cannot processed by file
			// -> erase limit specification
			cPlanSource_.eraseLimit();
		} else {
			if (getPredicate() && getPredicate()->isChosen()
				&& m_pScanFile
				&& isGetByBitSetRowID() == false) {
				// order can processed by file
				// and no predicate are processed by the file
				// -> check index cost again
				Predicate::ChooseArgument cChooseArgument(this,
														  0,
														  calculateScanCost(cEnvironment_),
														  cPlanSource_.getEstimateLimit(),
														  calculateRepeatCount(cEnvironment_,
																			   cPlanSource_));
				setPredicate(getPredicate()->getChosen()->rechoose(cEnvironment_,
																   cChooseArgument));
			}
		}
	}
	// check index file for limit
	if (cPlanSource_.getLimit().isSpecified()) {
		checkLimit(cEnvironment_,
				   cPlanSource_.getLimit());
	}
	if (m_pScanFile == 0) {
		Predicate::CheckNeedScanArgument cArgument(this, true /* top */);
		if (getPredicate() == 0
			|| !getPredicate()->isChosen()
			|| getPredicate()->getChosen()->isNeedScan(cArgument)) {
			// choose scan file
			m_pScanFile = chooseScan(cEnvironment_);
			if (m_pScanFile == 0) {
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pOrder_
//	const Order::CheckArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkOrder(Opt::Environment& cEnvironment_,
		   Order::Specification* pOrder_,
		   const Order::CheckArgument& cArgument_)
{
	Order::Specification* pCheckedOrder = pOrder_->check(cEnvironment_,
														 cArgument_);
	setOrder(pCheckedOrder);
	return pCheckedOrder != 0;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkLimit -- check file for limit
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
checkLimit(Opt::Environment& cEnvironment_,
		   const AccessPlan::Limit& cLimit_)
{
	// Limit can be processed by file only when
	//	scaned file is single and no more predicate
	Candidate::File* pScanFile = 0;
	if (m_pScanFile && getPredicate() == 0) {
		if (m_pScanFile->checkLimit(cEnvironment_,
									cLimit_)) {
			setLimit(cLimit_);
		}
	} else if (m_pScanFile == 0
			   && getPredicate() != 0
			   && getPredicate()->isChosen()
/*     5	   && getPredicate()->getChosen()->getFile(cEnvironment_) */
			   && getPredicate()->getChosen()->getNotChecked() == 0) {
		if (getPredicate()->getChosen()->checkLimit(cEnvironment_,
													cLimit_)) {
			setLimit(cLimit_);
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::choosePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
choosePredicate(Opt::Environment& cEnvironment_,
				AccessPlan::Source& cPlanSource_)
{
	if (getPredicate()->isChecked()) {
		Predicate::ChooseArgument cChooseArgument(this,
												  (getOrder() && getOrder()->isChecked())
												  ? getOrder()->getChecked()
												  : 0,
												  calculateScanCost(cEnvironment_),
												  cPlanSource_.getEstimateLimit(),
												  calculateRepeatCount(cEnvironment_,
																	   cPlanSource_));
		setPredicate(getPredicate()->getChecked()->choose(
									 cEnvironment_,
									 cChooseArgument));
	}
	if (Interface::IPredicate* pNotChecked = getPredicate()->getNotChecked()) {
		pNotChecked->require(cEnvironment_, this);
	}
}


// FUNCTION private
//	Candidate::TableImpl::Retrieve::chooseOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
chooseOrder(Opt::Environment& cEnvironment_,
			AccessPlan::Source& cPlanSource_)
{
	Order::ChooseArgument cChooseArgument(this,
										  getPredicate(),
										  cPlanSource_.getLimit());
	setOrder(getOrder()->getChecked()->choose(
								 cEnvironment_,
								 cChooseArgument));
	if (getOrder() && getOrder()->isChosen()) {
		// set to scan file
		// [NOTES]
		//	if ordering file is also used to evaluate a predicate,
		//	m_pScanFile become 0.
		m_pScanFile = getOrder()->getChosen()->getFile();
	} else {
		setOrder(0);
	}
}

// FUNCTION private
//	Candidate::TableImpl::RetrieveImpl::Retrieve::chooseRetrieve -- 
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
TableImpl::Retrieve::
chooseRetrieve(Opt::Environment& cEnvironment_)
{
	// reset member variables used in generate
	clear();
	m_cRetrieve.clear();

	if (m_pScanFile) {
		// set file->candidate entry for scan file
		m_cRetrieve.addCandidate(m_pScanFile);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::chooseRetrieveFile -- choose retrieved files for each retrieved columns
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Utility::FieldSet& cTargetColumn_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
chooseRetrieveFile(Opt::Environment& cEnvironment_,
				   Utility::FieldSet& cTargetColumn_,
				   RetrieveInfo& cRetrieve_)
{
	// replace each field to checked field
	Utility::FieldSet cTargetColumn(cTargetColumn_);
	Opt::MapContainer(cTargetColumn,
					  boost::bind(&Scalar::Field::checkRetrieve,
								  _1,
								  boost::ref(cEnvironment_)));

	Utility::FieldSet::ConstIterator found =
		cTargetColumn.findElement(boost::bind(&Scalar::Field::isRowID,
											  _1));
	
	if (found != cTargetColumn.end()) {
		setRowID(*found);
	}

	// find required files
	if (!cTargetColumn.isAll(boost::bind(&This::checkRetrieveFile,
										 this,
										 boost::ref(cEnvironment_),
										 _1,
										 boost::ref(cRetrieve_)))) {
		// any target can't find file
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// add fetch key fields to required
	cTargetColumn.add(cRetrieve_.m_vecFetchKey.begin(), cRetrieve_.m_vecFetchKey.end());

	// create retrieving fields from files
	cTargetColumn.foreachElement(
					 boost::bind(&This::chooseFile,
								 this,
								 boost::ref(cEnvironment_),
								 _1,
								 boost::ref(cRetrieve_)));

	cRetrieve_.m_bChanged = false;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkRetrieveFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkRetrieveFile(Opt::Environment& cEnvironment_,
				  Scalar::Field* pField_,
				  RetrieveInfo& cRetrieve_)
{
	; _SYDNEY_ASSERT(pField_->isChecked());

	if (cRetrieve_.m_bDelayed && pField_->isRowID()) {
		// if this method is called for delayed-retrieval,
		// rowid is thought as already retrieved
		return true;
	}

	Scalar::CheckedField* pCheckedField = pField_->getChecked();
	Utility::FileSet cFile = pCheckedField->getFileSet();

	if (pField_->isColumn()
		&& !pField_->isRowID()
		&& !pField_->isPutKey()
		&& !pField_->isExpandElement()
		&& cRetrieve_.m_cRequiredFile.isContainingAny(cFile)) {
		// if retrieved file is already appeared, use it
		Utility::FileSet cIntersect;
		Utility::FileSet cDifference;
		cFile.divide(cRetrieve_.m_cRequiredFile,
					 cIntersect,
					 cDifference);
		cFile = cIntersect;
		// remove from checked field
		cDifference.foreachElement(boost::bind(&Scalar::CheckedField::removeFile,
											   pCheckedField,
											   _1));
	}
	if (cFile.getSize() == 1) {
		Interface::IFile* pFile = *cFile.begin();
		// add to required file
		cRetrieve_.m_cRequiredFile.add(pFile);
	}
	Predicate::CheckRetrievableArgument cCheckArgument(pField_);
	if (!cFile.isAny(boost::bind(&This::isRetrievableFile,
								 this,
								 boost::ref(cEnvironment_),
								 _1,
								 boost::ref(cCheckArgument),
								 boost::ref(cRetrieve_)))) {
		if (getRowID() == 0) {
			// if rowid is not available, get by fetch is not available
			return false;
		}
		// if no element in cFile are not included in start file
		// and can be fetched, add fetch key files
		Utility::FileSet::Iterator found =
			Opt::Find(cFile.begin(), cFile.end(),
					  boost::bind(&This::checkFetchFile,
								  this,
								  boost::ref(cEnvironment_),
								  _1,
								  pField_,
								  boost::ref(cRetrieve_)));
		if (found == cFile.end()) {
			// no file can be retrieved or fetched
			return checkAlternativeValue(cEnvironment_,
										 pField_,
										 cRetrieve_);
		}
		// add fetched file as required
		cRetrieve_.m_cRequiredFile.add(*found);
	}
	return true;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::createAlternativeValue -- create alternative value if exists
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Scalar::Field* pField_
//	Candidate::AdoptArgument& cArgument_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
createAlternativeValue(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Scalar::Field* pField_,
					   Candidate::AdoptArgument& cArgument_,
					   RetrieveInfo& cRetrieve_)
{
	FieldScalarMap::Iterator found = cRetrieve_.m_mapAlternativeValue.find(pField_);
	if (found != cRetrieve_.m_mapAlternativeValue.end()) {
		int iInData = (*found).second->generate(cEnvironment_,
												cProgram_,
												pIterator_,
												cArgument_);
		int iOutData = pIterator_->getNodeVariable(pField_->getID());
		; _SYDNEY_ASSERT(iInData >= 0);
		; _SYDNEY_ASSERT(iOutData >= 0);

		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT_T(Assign,
												 iInData,
												 iOutData,
												 cArgument_.m_eTarget));
		return true;
	}
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkAlternativeValue -- check alternative value in union
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkAlternativeValue(Opt::Environment& cEnvironment_,
					  Scalar::Field* pField_,
					  RetrieveInfo& cRetrieve_)
{
	Interface::IScalar* pValue = pField_->getAlternativeValue(cEnvironment_);
	if (pValue) {
		cRetrieve_.m_mapAlternativeValue[pField_] = pValue;
		return true;
	}
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkFetchFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkFetchFile(Opt::Environment& cEnvironment_,
			   Interface::IFile* pFile_,
			   Scalar::Field* pField_,
			   RetrieveInfo& cRetrieve_)
{
	; _SYDNEY_ASSERT(pField_->isChecked());

	if (pFile_->getSchemaFile()->isAbleToFetch()) {
		FileFieldMap::Iterator found = cRetrieve_.m_mapFetchKey.find(pFile_);
		if (found != cRetrieve_.m_mapFetchKey.end()) {
			return true;

		} else {
			// get Scalar::Field corresponds to Schema::Field on pFile_
			Scalar::Field* pField = pField_->getField(pFile_);

			// create Scalar::Field corresponds to fetch key for getting the field
			if (Scalar::Field* pFetchKey = pField->createFetchKey(cEnvironment_)) {
				// check retrievable file for the check key
				Scalar::Field* pChecked = pFetchKey->checkRetrieve(cEnvironment_);
				// remove pFile from retrieving file
				pChecked->getChecked()->removeFile(pFile_);
				if (pFetchKey->isRowID() == false) {
					cRetrieve_.m_vecFetchKey.PUSHBACK(pChecked);
					// check retrieve file for the fetch key
					if (checkRetrieveFile(cEnvironment_,
										  pChecked,
										  cRetrieve_)) {
						// add to fetch key set
						cRetrieve_.m_mapFetchKey[pFile_] = pChecked;
						return true;
					}
				} else {
					// rowid is checked
					// -> add to fetch key set
					cRetrieve_.m_mapFetchKey[pFile_] = pChecked;
					return true;
				}
			}
		}
	}
	// can't fetch
	// -> remove from retrieve file
	pField_->getChecked()->removeFile(pFile_);
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::chooseFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
chooseFile(Opt::Environment& cEnvironment_,
		   Scalar::Field* pField_,
		   RetrieveInfo& cRetrieve_)
{
	if (cRetrieve_.m_mapFieldRetrieve[pField_] != 0) {
		// already chosen
		return;
	}

	; _SYDNEY_ASSERT(pField_->isChecked());
	const Utility::FileSet& cRetrieveFile = pField_->getChecked()->getFileSet();
	if (cRetrieveFile.isEmpty()) {
		return;
	}

	if (pField_->isRowID()) {
		if (getPredicate() && getPredicate()->isChosen()) {
			Predicate::CheckRetrievableArgument cCheckArgument(pField_);
			FOREACH_if(cRetrieveFile,
					   boost::bind(&This::addField,
								   this,
								   boost::ref(cEnvironment_),
								   _1,
								   pField_,
								   cRetrieve_),
					   boost::bind(&Predicate::ChosenInterface::isRetrievable,
								   getPredicate()->getChosen(),
								   boost::ref(cEnvironment_),
								   _1,
								   boost::ref(cCheckArgument)));
		}
		if (m_pScanFile
			&& cRetrieveFile.isContaining(m_pScanFile->getFile())) {
			addField(cEnvironment_,
					 m_pScanFile->getFile(),
					 pField_,
					 cRetrieve_);
		}
	} else {
		if (pField_->isExpandElement()
			&& isGetByBitSetRowID()
			&& m_pScanFile
			&& cRetrieveFile.isContaining(m_pScanFile->getFile())) {
			addField(cEnvironment_,
					 m_pScanFile->getFile(),
					 pField_,
					 cRetrieve_);
		} else if (pField_->isChoice()) {
			// field include field choice -> use all
			cRetrieveFile.foreachElement(boost::bind(&This::addField,
													 this,
													 boost::ref(cEnvironment_),
													 _1,
													 pField_,
													 boost::ref(cRetrieve_)));

		} else if (cRetrieveFile.isContainingAny(cRetrieve_.m_cRequiredFile)) {
			// if requiredfile is contained, use it
			Utility::FileSet cFile;
			cRetrieveFile.intersect(cRetrieve_.m_cRequiredFile, cFile);
			; _SYDNEY_ASSERT(!cFile.isEmpty());

			addField(cEnvironment_,
					 *cFile.begin(),
					 pField_,
					 cRetrieve_);

		} else if (cRetrieve_.m_bDelayed)  {
			// if delayed retrieval, all files can be used
			addField(cEnvironment_,
					 *cRetrieveFile.begin(),
					 pField_,
					 cRetrieve_);

		} else if (m_pScanFile
				   && cRetrieveFile.isContaining(m_pScanFile->getFile())) {
			// if scan file is included, use it
			addField(cEnvironment_,
					 m_pScanFile->getFile(),
					 pField_,
					 cRetrieve_);

		} else if (getPredicate() && getPredicate()->isChosen()) {
			// if index file is included, use it
			Predicate::CheckRetrievableArgument cCheckArgument(pField_);

			Utility::FileSet::ConstIterator iterator =
				Opt::Find(cRetrieveFile.begin(),
						  cRetrieveFile.end(),
						  boost::bind(&Predicate::ChosenInterface::isRetrievable,
									  getPredicate()->getChosen(),
									  boost::ref(cEnvironment_),
									  _1,
									  boost::ref(cCheckArgument)));

			if (iterator != cRetrieveFile.end()) {
				addField(cEnvironment_,
						 *iterator,
						 pField_,
						 cRetrieve_);
			} else {
				// can't retrieve from index file
				// -> use first candidate
				addField(cEnvironment_,
						 *cRetrieveFile.begin(),
						 pField_,
						 cRetrieve_);
			}
		} else {
			// otherwive, all files can be used
			addField(cEnvironment_,
					 *cRetrieveFile.begin(),
					 pField_,
					 cRetrieve_);
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::checkScanFile -- check a file can be scanned
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Plan::File::CheckArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
checkScanFile(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Plan::File::CheckArgument& cArgument_)
{
	return checkFile(cEnvironment_,
					 pFile_,
					 0,
					 cArgument_,
					 AccessPlan::Cost(),
					 boost::bind(&LogicalFile::AutoLogicalFile::getSearchParameter,
								 _1,
								 static_cast<Interface::IPredicate*>(0),
								 _2));
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::addField -- add retrieved fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
addField(Opt::Environment& cEnvironment_,
		 Interface::IFile* pFile_,
		 Scalar::Field* pField_,
		 RetrieveInfo& cRetrieve_)
{
	; _SYDNEY_ASSERT(pField_->isChecked());

	Scalar::Field* pField = pField_->getField(pFile_);
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// get candidate::file
	Candidate::File* pCandidateFile = cRetrieve_.getCandidate(pFile_);
	if (pCandidateFile == 0) {
		Predicate::CheckRetrievableArgument cCheckArgument(pField_);
		if (!cRetrieve_.m_bDelayed
			&& getPredicate() && getPredicate()->isChosen()
			&& getPredicate()->getChosen()->isRetrievable(cEnvironment_,
														  pFile_,
														  cCheckArgument)) {
			if (cCheckArgument.m_pFile) {
				pCandidateFile = cCheckArgument.m_pFile;
			} else {
				// add through predicate object
				getPredicate()->getChosen()->addRetrieve(cEnvironment_,
														 pFile_,
														 cCheckArgument);
			}
		} else {
			// create new candidate::file
			pCandidateFile = pFile_->createCandidate(cEnvironment_,
													 this,
													 File::Parameter::create(cEnvironment_));
			cRetrieve_.addCandidate(pCandidateFile);
		}
	}
	if (pCandidateFile) {
		addRetrieveField(cEnvironment_,
						 pCandidateFile,
						 pFile_,
						 pField_,
						 cRetrieve_);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::addRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidateFile_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
addRetrieveField(Opt::Environment& cEnvironment_,
				 Candidate::File* pCandidateFile_,
				 Interface::IFile* pFile_,
				 Scalar::Field* pField_,
				 RetrieveInfo& cRetrieve_)
{
	Scalar::Field* pField = pField_->getField(pFile_);
	; _SYDNEY_ASSERT(pField);

	pCandidateFile_->addField(cEnvironment_, pField);
	cRetrieve_.m_mapFieldRetrieve[pField_] = pFile_;
//	if (pField_->isChecked())
//		cRetrieve_.m_mapFieldRetrieve[pField_->getChecked()->getWrappedField()] = pFile_;
}

// FUNCTION public
//	Candidate::TableImpl::Retrieve::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

//virtual
Scalar::Field*
TableImpl::Retrieve::
getUsedField(Opt::Environment& cEnvironment_,
			 Scalar::Field* pField_)
{
	if(!pField_) _SYDNEY_THROW0(Exception::BadArgument);
	
	Scalar::Field* result = pField_;
	FieldRetrieveMap::ConstIterator ite =
		m_cRetrieve.m_mapFieldRetrieve.find(pField_);
	if ( ite != m_cRetrieve.m_mapFieldRetrieve.end()) {
		result = pField_->getField((*ite).second);
	}

	return result;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::chooseScan -- choose scannable file
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

Candidate::File*
TableImpl::Retrieve::
chooseScan(Opt::Environment& cEnvironment_)
{
	if (m_pScanFileCache) return m_pScanFileCache;

	// Get scanning file for each retrieved column
	Utility::FileSet cFile;
	Utility::FieldSet::Iterator iterator = m_cRetrievedColumn.begin();
	const Utility::FieldSet::Iterator last = m_cRetrievedColumn.end();
	for (; iterator != last; ++iterator) {
		if (!(*iterator)->isDelayed(this)) {
			Utility::FileSet cTmp;
			Scalar::Field::getScanFile(cEnvironment_,
									   Scalar::GetFileArgument(*iterator,
															   0,
															   cTmp));
			// check fetchkey anyway
			Scalar::Field* pFetchKey = (*iterator)->createFetchKey(cEnvironment_);
			if (pFetchKey) {
				Scalar::Field::getScanFile(cEnvironment_,
										   Scalar::GetFileArgument(pFetchKey,
																   0,
																   cTmp));
			}
			if (cTmp.isEmpty()) {
				// cannot scanned
				// -> it may exist in index file
				continue;
			}

			if (cFile.isEmpty()) cFile = cTmp;
			else cFile.intersect(cTmp);

			if (cFile.isEmpty()) {
				// no file can scan
				return 0;
			}
		}
	}

	// create checkedinterface for scanning
	Plan::File::CheckArgument cArgument(this);
	if (cFile.getSize() <= 1) {
		cArgument.skipEstimate();
	}
	Utility::FileSet::ConstIterator min =
		Opt::FindLast(cFile.begin(),
					  cFile.end(),
					  boost::bind(&This::checkScanFile,
								  this,
								  boost::ref(cEnvironment_),
								  _1,
								  boost::ref(cArgument)));

	if (min == cFile.end()) {
		// no file can scan
		return 0;
	}

	Candidate::File* pResult =
		cArgument.m_pFile->createCandidate(cEnvironment_,
										   this,
										   cArgument.m_pParameter);
	// remember schema file when table is updated
	pResult->setForUpdate(cEnvironment_);

	return m_pScanFileCache = pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::calculateScanCost -- calculate scan cost
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	const AccessPlan::Cost&
//
// EXCEPTIONS

const AccessPlan::Cost&
TableImpl::Retrieve::
calculateScanCost(Opt::Environment& cEnvironment_)
{
	if (m_cScanCost.isInfinity()) {
		Candidate::File* pScanFile = chooseScan(cEnvironment_);
		if (pScanFile) {
			m_cScanCost = pScanFile->getCost(cEnvironment_);
		}
	}
	return m_cScanCost;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::calculateRepeatCount -- calculate repeat count
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

AccessPlan::Cost::Value
TableImpl::Retrieve::
calculateRepeatCount(Opt::Environment& cEnvironment_,
					 AccessPlan::Source& cPlanSource_)
{
	AccessPlan::Cost::Value cResult(1);
	if (int n = cPlanSource_.getPrecedingCandidate().GETSIZE()) {
		// multiply tuplecount
		const VECTOR<Interface::ICandidate*>& vecCandidate = cPlanSource_.getPrecedingCandidate();
		VECTOR<Interface::ICandidate*>::CONSTITERATOR iterator = vecCandidate.begin();
		const VECTOR<Interface::ICandidate*>::CONSTITERATOR last = vecCandidate.end();
		for (; iterator != last; ++iterator) {
			(*iterator)->createCost(cEnvironment_, cPlanSource_);
			cResult *= (*iterator)->getCost().getTupleCount();
		}
	}
	return cResult;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::addPredicateCost -- add predicate cost for scanning plan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Retrieve::
addPredicateCost(Opt::Environment& cEnvironment_,
				 AccessPlan::Cost& cCost_)
{
	if (getPredicate() && getPredicate()->isChosen()) {
		AccessPlan::Cost::Value cOverhead;
		AccessPlan::Cost::Value cTotalCost;
		AccessPlan::Cost::Value cTupleCount;
		AccessPlan::Cost::Value cRetrieveCost;
		AccessPlan::Cost::Value cRate;

		AccessPlan::Cost cMyCost;
		cMyCost.setIsSetCount(false); // donot set tuplecount by getCost
		cMyCost.setTupleCount(cCost_.getTupleCount());
		cMyCost.setTableCount(getEstimateCount(cEnvironment_));
		if (getPredicate()->getChosen()->getCost(cEnvironment_, cMyCost) == false
			|| cMyCost.isInfinity()) {
			// index cannot be used or cannot be estimated
			getPredicate()->getChosen()->getEstimateCost(cEnvironment_, cMyCost);
			cOverhead = cCost_.getOverhead() + cMyCost.getOverhead();
			cTotalCost = cCost_.getTotalCost() + (cMyCost.getTotalCost() * cCost_.getTupleCount());

		} else {
			// index is used to check tuples
			cOverhead = cCost_.getOverhead() + cMyCost.getOverhead() + cMyCost.getTotalCost(); // index should store tuples
			cTotalCost = cCost_.getTotalCost();
		}
		cRate = cMyCost.getRate();
		cRetrieveCost = cCost_.getRetrieveCost() + cMyCost.getRetrieveCost();
		cCost_.setOverhead(cOverhead);
		cCost_.setTotalCost(cTotalCost);
		cCost_.setRetrieveCost(cRetrieveCost);
		cCost_.setRate(cRate);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::createFileFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	Candidate::AdoptArgument& cArgument_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Retrieve::
createFileFetch(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Interface::IFile* pFile_,
				Scalar::Field* pField_,
				Candidate::AdoptArgument& cArgument_,
				RetrieveInfo& cRetrieve_)
{
	// check whether the obtaining file needs another file for fetch key
	FileFieldMap::Iterator found = cRetrieve_.m_mapFetchKey.find(pFile_);
	if (found != cRetrieve_.m_mapFetchKey.end()) {
		Scalar::Field* pFetchKey = (*found).second;
		Scalar::Field* pFetchKeyValue = pFetchKey;

		if (!(cRetrieve_.m_bDelayed && pFetchKey->isRowID())) {
			// get retrieving file for the fetchkey
			Interface::IFile* pFetchKeyFile = cRetrieve_.m_mapFieldRetrieve[pFetchKey];

			if (pFetchKeyFile) {
				// replace fetch key field by target file's field
				pFetchKeyValue = pFetchKey->getField(pFetchKeyFile);
			} else {
				; _SYDNEY_ASSERT(pFetchKey->isRowID());
			}
		}

		// generate field value
		; _SYDNEY_ASSERT(getTable()->getAdoptCandidate() == 0);

		int iFetchKeyID = pFetchKeyValue->generate(cEnvironment_,
												   cProgram_,
												   pIterator_,
												   cArgument_);
		if (iFetchKeyID < 0) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		// add fetch action if necessary
		addFileFetch(cEnvironment_,
					 cProgram_,
					 pIterator_,
					 pFetchKey,
					 cArgument_);

		// get candidate::file
		Candidate::File* pCandidate = cRetrieve_.getCandidate(pFetchKey->getFile());
		; _SYDNEY_ASSERT(pCandidate);

		// generate filefetch action
		pCandidate->createFetch(cEnvironment_,
								cProgram_,
								pIterator_,
								cArgument_,
								pFetchKey,
								iFetchKeyID);
		return true;
	}
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::isRetrievableFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Retrieve::
isRetrievableFile(Opt::Environment& cEnvironment_,
				  Interface::IFile* pFile_,
				  Predicate::CheckRetrievableArgument& cCheckArgument_,
				  RetrieveInfo& cRetrieve_)
{
	return !cRetrieve_.m_bDelayed
		&& ((m_pScanFile && m_pScanFile->isRetrievable(cEnvironment_,
													   pFile_,
													   cCheckArgument_.m_pField))
			|| (getPredicate() && getPredicate()->isChosen()
				&& getPredicate()->getChosen()->isRetrievable(cEnvironment_,
															  pFile_,
															  cCheckArgument_)));
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::adoptStartFile -- set projection to start file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Utility::FieldSet& cTargetColumn_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Retrieve::
adoptStartFile(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Candidate::AdoptArgument& cArgument_,
			   Utility::FieldSet& cTargetColumn_,
			   RetrieveInfo& cRetrieve_)
{
	// set this as adopted candidate to relation object
	getTable()->setAdoptCandidate(this);

	if (m_pScanFile) {
		m_pIterator = adoptByScan(cEnvironment_,
								  cProgram_,
								  cArgument_,
								  cTargetColumn_,
								  cRetrieve_);
	} else if (getPredicate() && getPredicate()->isChosen()) {
		m_pIterator = adoptByPredicate(cEnvironment_,
									   cProgram_,
									   cArgument_,
									   cTargetColumn_,
									   cRetrieve_);
	} else {
		// can't adopt start file
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	if(cArgument_.m_bForceRowID) {
		cArgument_.m_bForceRowID = false;
		cArgument_.m_bForceRowIDSet = true;
	}
	return m_pIterator;
}


// FUNCTION private
//	Candidate::TableImpl::Retrieve::addNarrowByBitSetAction
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS


Execution::Interface::IIterator*
TableImpl::Retrieve::
addNarrowByBitSetAction(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Candidate::AdoptArgument& cArgument_)
{

	Candidate::AdoptIndexArgument cIndexArgument;

	Candidate::AdoptIndexArgument::Element& cElement = cIndexArgument.getElement(this);

	cElement.m_bForceBitSet = true;
	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cResult =
		getPredicate()->getChosen()->adoptScan(cEnvironment_,
											   cProgram_,
											   cArgument_,
											   cIndexArgument);
	setPredicate(cResult.second);
	
	cElement = cIndexArgument.getElement(this);
	cElement.m_iSearchBitSetID = cElement.m_iBitSetID;
	cElement.m_iBitSetID = -1;
	Execution::Interface::IIterator* pIterator = m_pScanFile->createScanWithSearchByBitSetOption(cEnvironment_,
																								 cProgram_,
																								 m_pScanFile->createFileAccess(cProgram_),
																								 cArgument_,
																								 cIndexArgument);
	
	pIterator->addCalculation(cProgram_, 
								Execution::Operator::Iterate::Once::create(
									cProgram_,
									pIterator,
									cResult.first->getID()),
								Execution::Action::Argument::Target::StartUp);
	return pIterator;
}



// FUNCTION private
//	Candidate::TableImpl::Retrieve::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Retrieve::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		bool bAddRetrieveCost = true;
		if (m_pIterator) {
			// already adopted
			// -> this case should be called by subquery generator
			// -> use zero cost and 1 tuple
			//		(for subquery, this relation can be regarded as returning 1 tuple)
			cCost_.reset();
			cCost_.setTupleCount(1);
			bAddRetrieveCost = false;

		} else if (m_pScanFile) {
			cCost_ = m_pScanFile->getCost(cEnvironment_);
			cCost_.setTableCount(cCost_.getTupleCount());
			addPredicateCost(cEnvironment_, cCost_);

			bAddRetrieveCost = (m_pScanFile->getParameter()
								&& m_pScanFile->getParameter()->getOrder());

		} else if (getPredicate() && getPredicate()->isChosen()) {
			cCost_.setIsSetRate(false); // donot set rate for scanned predicate
			cCost_.setTableCount(getEstimateCount(cEnvironment_));
			getPredicate()->getChosen()->getCost(cEnvironment_, cCost_);

			bAddRetrieveCost = (chooseScan(cEnvironment_) != 0);

		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		if (bAddRetrieveCost) {
			// add scan cost after processing predicates
			const AccessPlan::Cost& cScanCost = calculateScanCost(cEnvironment_);
			AccessPlan::Cost::Value cRetrieveCost = cScanCost.getTotalCost();
			if (!cRetrieveCost.isInfinity()) {
				if (cScanCost.getTupleCount() >= 1) {
					cRetrieveCost /= cScanCost.getTupleCount();
				}
				cCost_.setRetrieveCost(cRetrieveCost);
			}
		}

		if (isGetByBitSetRowID()) {
			// tuple count is reduced
			if (cCost_.getTupleCount() > 100) {
				cCost_.setTupleCount(100);
			}
		}

		if (cPlanSource_.getOrder() == 0
			|| Order::Specification::isCompatible(getOrder(),
												  cPlanSource_.getOrder())) {
			cCost_.setLimitCount(cPlanSource_.getEstimateLimit());
		}
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "Result table(" << getTable()->getCorrelationName(cEnvironment_)
				   << ")["
				   << _getCorrelationNames(cEnvironment_,
										   cPlanSource_.getPrecedingCandidate())
				   << "] cost: " << cCost_;
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif
		; _SYDNEY_ASSERT(!cCost_.isInfinity());
	}
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::createRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
TableImpl::Retrieve::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);

	m_cRetrievedColumn.foreachElement(boost::bind(&Candidate::Row::addScalar,
												  pRow,
												  _1));
	m_bRowCreated = true;
	return pRow;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
TableImpl::Retrieve::
createKey(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);

	Utility::FieldSet::ConstIterator found =
		m_cRetrievedColumn.findElement(boost::bind(&Scalar::Field::isRowID,
												   _1));
	if (found != m_cRetrievedColumn.end()) {
		pRow->addScalar(*found);
	}
	return pRow;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::createCheckPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
TableImpl::Retrieve::
createCheckPredicate(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Candidate::AdoptArgument& cArgument_,
					 Interface::IPredicate* pPredicate_)
{
	; _SYDNEY_ASSERT(pPredicate_);

	if (pPredicate_->isChosen()) {
		Candidate::AdoptIndexArgument cIndexArgument;
		return pPredicate_->getChosen()->createCheck(cEnvironment_,
													 cProgram_,
													 pIterator_,
													 cArgument_,
													 cIndexArgument);
	}
	; _SYDNEY_ASSERT(pPredicate_->isChecked());
	return pPredicate_->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::generateContinue -- 
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
TableImpl::Retrieve::
generateContinue(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	// unlock tuple before continue
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(UnlockTuple,
											  cArgument_.m_eTarget));
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(Continue,
											  cArgument_.m_eTarget));
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::adoptByScan -- adopt start file using scanfile
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Utility::FieldSet& cTargetColumn_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Retrieve::
adoptByScan(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Candidate::AdoptArgument& cArgument_,
			Utility::FieldSet& cTargetColumn_,
			RetrieveInfo& cRetrieve_)
{
	Execution::Interface::IIterator* pResult = 0;
	bool bGroupByBitSet = false;
	if (getPredicate() && getPredicate()->isChosen()) {
		// check index here
		Candidate::CheckIndexArgument cCheckArgument;
			
		getPredicate()->getChosen()->checkIndex(cEnvironment_,
												cProgram_,
												cArgument_,
												cCheckArgument);
		if (m_cRetrieve.m_bChanged) {	
			// retrieved column has been changed at adopting
			// -> choose retrieve file again
			chooseRetrieveFile(cEnvironment_,
							   cTargetColumn_,
							   m_cRetrieve);
			m_cRetrieve.m_bChanged = false;
		}

		// group byの場合で、bitsetで取得できる述語がある場合は、
		// bitsetから検索範囲を絞るactionをstartupに追加
		if (isGetByBitSetRowID()) {
			if (!cCheckArgument.isOnlyBitSet()) {
						
				// bitsetで取得できない述語がある場合は、BitSetIDをRowIDに変換する
				cArgument_.m_bForceRowID = true;
						
			}
			if (cCheckArgument.hasBitSet()) {
				bGroupByBitSet = true;
				pResult = addNarrowByBitSetAction(cEnvironment_, cProgram_, cArgument_);
			}
		}
	}
	if(!bGroupByBitSet) {
		Candidate::AdoptIndexArgument cIndexArgument;
		// if scan file is already set, use it
		pResult = m_pScanFile->createScan(cEnvironment_, cProgram_,
										  m_pScanFile->createFileAccess(cProgram_),
										  cArgument_,
										  cIndexArgument);
	}
	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Retrieve::adoptByPredicate -- adopt start file using predicate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Utility::FieldSet& cTargetColumn_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Retrieve::
adoptByPredicate(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Candidate::AdoptArgument& cArgument_,
				 Utility::FieldSet& cTargetColumn_,
				 RetrieveInfo& cRetrieve_)
{
	Candidate::CheckIndexArgument cCheckArgument;
	getPredicate()->getChosen()->checkIndex(cEnvironment_,
											cProgram_,
											cArgument_,
											cCheckArgument);
	if (isGetByBitSetRowID()) {
		if (!cCheckArgument.isOnlyBitSet()) {
			// bitsetで取得できない述語がある場合は、BitSetIDをRowIDに変換する
			cArgument_.m_bForceRowID = true;
		}
	}

	if (m_cRetrieve.m_bChanged) {
		// retrieved column has been changed at checkindex
		// -> choose retrieve file again
		chooseRetrieveFile(cEnvironment_,
						   cTargetColumn_,
						   m_cRetrieve);
		m_cRetrieve.m_bChanged = false;
	}

	// create scan iterator from predicate
	Candidate::AdoptIndexArgument cIndexArgument;
	PAIR<Execution::Interface::IIterator*, Interface::IPredicate*> cResult;
	cResult = getPredicate()->getChosen()->adoptScan(cEnvironment_,
													 cProgram_,
													 cArgument_,
													 cIndexArgument);
	setPredicate(cResult.second);

	if (m_cRetrieve.m_bChanged) {
		// retrieved column has been changed at adopting
		// -> choose retrieve file again
		chooseRetrieveFile(cEnvironment_,
						   cTargetColumn_,
						   m_cRetrieve);
		m_cRetrieve.m_bChanged = false;
	}
	return cResult.first;
}



//////////////////////////////////////////////////
//	Plan::Candidate::TableImpl::Virtual

// FUNCTION public
//	Candidate::TableImpl::Virtual::require -- add retrieved columns
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

void
TableImpl::Virtual::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	retrieve(cEnvironment_,
			 pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Virtual::use -- 
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
TableImpl::Virtual::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	retrieve(cEnvironment_,
			 pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Virtual::delay -- add retrieved but delayable columns
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Virtual::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	// no columns can be delayed
	return false;
}

// FUNCTION public
//	Candidate::TableImpl::Virtual::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Virtual::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	if (getRetrievedColumn().isEmpty()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// result is input iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Input::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	cArgument_.m_pTable = this;
	cArgument_.setCandidate(this);

	// generate variable and position to get output
	VECTOR<int> vecDataID;
	VECTOR<int> vecPosition;
	getRetrievedColumn().mapElement(vecDataID,
									boost::bind(&Scalar::Field::generate,
												_1,
												boost::ref(cEnvironment_),
												boost::ref(cProgram_),
												pResult,
												boost::ref(cArgument_)));
	getRetrievedColumn().mapElement(vecPosition,
									boost::bind(&Schema::Column::getPosition,
												boost::bind(&Scalar::Field::getSchemaColumn,
															_1)));

	// create collection to generate virtual table data
	Execution::Interface::ICollection* pCollection =
		Execution::Collection::VirtualTable::create(cProgram_,
													*getTable()->getSchemaTable(),
													vecPosition);
	// create variable to get data
	int iDataID = cProgram_.addVariable(vecDataID);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pCollection->getID(),
										iDataID));

	if (getPredicate()
		&& (getPredicate()->isChosen() || getPredicate()->isChecked())) {
		// if any predicate left, generate it
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	// add to refered table
	cEnvironment_.addReferedTable(getTable()->getSchemaTable());
 
	cArgument_.m_pTable = 0;

	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::checkPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	const Predicate::CheckArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Virtual::
checkPredicate(Opt::Environment& cEnvironment_,
			   Interface::IPredicate* pPredicate_,
			   const Predicate::CheckArgument& cArgument_)
{
	setPredicate(pPredicate_->check(cEnvironment_,
									cArgument_));
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::choosePredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Virtual::
choosePredicate(Opt::Environment& cEnvironment_,
				AccessPlan::Source& cPlanSource_)
{
	if (getPredicate()->isChecked()) {
		Predicate::ChooseArgument cChooseArgument(this,
												  0,
												  AccessPlan::Cost(),
												  AccessPlan::Cost::Value(),
												  AccessPlan::Cost::Value(1));
		setPredicate(getPredicate()->getChecked()->choose(
									 cEnvironment_,
									 cChooseArgument));
	}
	if (Interface::IPredicate* pNotChecked = getPredicate()->getNotChecked()) {
		pNotChecked->require(cEnvironment_, this);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::enumerate -- enumerate possible plans
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccussPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Virtual::
enumerate(Opt::Environment& cEnvironment_,
		  AccessPlan::Source& cPlanSource_)
{
	// create checked predicate
	if (Interface::IPredicate* pPredicate = cPlanSource_.getPredicate()) {
		Predicate::CheckArgument cCheckArgument(this,
												cPlanSource_.getPrecedingCandidate());
		checkPredicate(cEnvironment_,
					   pPredicate,
					   cCheckArgument);
	}
	// create checked sorting order
	if (Order::Specification* pOrder = cPlanSource_.getOrder()) {
		// virtual table can't process order
		// -> erase limit specification
		cPlanSource_.eraseLimit();
	}
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::choose -- choose index to be used
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Virtual::
choose(Opt::Environment& cEnvironment_,
	   AccessPlan::Source& cPlanSource_)
{
	// choose index files for predicate
	if (getPredicate()) {
		choosePredicate(cEnvironment_, cPlanSource_);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Virtual::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	// reset by zero
	cCost_.reset();
	// set dummy tuple count
	cCost_.setTupleCount(1);
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::createCheckPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
TableImpl::Virtual::
createCheckPredicate(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Candidate::AdoptArgument& cArgument_,
					 Interface::IPredicate* pPredicate_)
{
	; _SYDNEY_ASSERT(pPredicate_);
	return pPredicate_->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Virtual::generateContinue -- 
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
TableImpl::Virtual::
generateContinue(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(Continue,
											  cArgument_.m_eTarget));
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Refer

// FUNCTION public
//	Candidate::TableImpl::Refer::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Refer::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// YET
	return 0;
}

// FUNCTION public
//	Candidate::TableImpl::Refer::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Refer::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	setRetrievedColumn(cEnvironment_,
					   cFieldSet_);
}

// FUNCTION public
//	Candidate::TableImpl::Refer::addConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Refer::
addConstraint(Opt::Environment& cEnvironment_,
			  Schema::Constraint* pSchemaConstraint_)
{
	m_vecSchemaConstraint.PUSHBACK(pSchemaConstraint_);
}

// FUNCTION public
//	Candidate::TableImpl::Refer::getConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const VECTOR<Schema::Constraint*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Schema::Constraint*>&
TableImpl::Refer::
getConstraint()
{
	return m_vecSchemaConstraint;
}

// FUNCTION public
//	Candidate::TableImpl::Refer::addRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidateFile_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Refer::
addRetrieveField(Opt::Environment& cEnvironment_,
				 Candidate::File* pCandidateFile_,
				 Interface::IFile* pFile_,
				 Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Simple

// FUNCTION public
//	Candidate::TableImpl::Simple::require -- 
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
TableImpl::Simple::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	if (pField_->isRowIDAvailable(cEnvironment_)) {
		// can't add as retrieved field
	} else {
		Super::require(cEnvironment_,
					   pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Simple::retrieve -- 
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
TableImpl::Simple::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	if (pField_->isRowIDAvailable(cEnvironment_)) {
		// can't add as retrieved field
	} else {
		Super::retrieve(cEnvironment_,
						pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Simple::use -- 
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
TableImpl::Simple::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	require(cEnvironment_,
			pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Simple::delay -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Simple::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Simple::isRetrievableFile -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Predicate::CheckRetrievableArgument& cCheckArgument_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Simple::
isRetrievableFile(Opt::Environment& cEnvironment_,
				  Interface::IFile* pFile_,
				  Predicate::CheckRetrievableArgument& cCheckArgument_,
				  RetrieveInfo& cRetrieve_)
{
	return true;
}

// FUNCTION private
//	Candidate::TableImpl::Simple::adoptStartFile -- create start iterator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Utility::FieldSet& cTargetColumn_
//	RetrieveInfo& cRetrieve_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Simple::
adoptStartFile(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Candidate::AdoptArgument& cArgument_,
			   Utility::FieldSet& cTargetColumn_,
			   RetrieveInfo& cRetrieve_)
{
	// set table is simple
	cEnvironment_.setSimpleTable(getTable()->getSchemaTable());

	// result is iterate-once iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Loop::Once::create(cProgram_);

	Opt::ForEachKey(cRetrieve_.m_mapFieldRetrieve,
					boost::bind(&Interface::IScalar::generate,
								_1,
								boost::ref(cEnvironment_),
								boost::ref(cProgram_),
								pResult,
								boost::ref(cArgument_)));

	Candidate::AdoptArgument cArgument(cArgument_);
	cArgument.m_pInput = pResult;
	Candidate::AdoptIndexArgument cIndexArgument;
	Opt::ForEachValue(cRetrieve_.m_mapFileCandidate,
					  boost::bind(&Candidate::File::setIsSimple,
								  _1));

	Opt::ForEachValue(cRetrieve_.m_mapFileCandidate,
					  boost::bind(
						  &Execution::Interface::IIterator::addCalculation,
						  pResult,
						  boost::ref(cProgram_),
						  boost::bind(
							  &Execution::Operator::Iterate::Once::create,
							  boost::ref(cProgram_),
							  pResult,
							  boost::bind(
								  &Execution::Interface::IAction::getID,
								  boost::bind(
									  &Candidate::File::createScan,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  boost::bind(&Candidate::File::createFileAccess,
												  _1,
												  boost::ref(cProgram_)),
									  boost::ref(cArgument),
									  boost::ref(cIndexArgument))),
							  true /* only once */),
						  cArgument_.m_eTarget));
	return pResult;
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Put

// FUNCTION public
//	Candidate::TableImpl::Put::require -- 
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
TableImpl::Put::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	if (getOperand()) {
		getOperand()->require(cEnvironment_,
							  pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Put::retrieve -- 
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
TableImpl::Put::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	if (getOperand()) {
		getOperand()->retrieve(cEnvironment_,
							   pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Put::use -- 
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
TableImpl::Put::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	if (getOperand()) {
		getOperand()->use(cEnvironment_,
						  pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Put::addFileFetch -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Scalar::Field* pField_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Put::
addFileFetch(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Scalar::Field* pField_,
			 Candidate::AdoptArgument& cArgument_)
{
	return true;
}

// FUNCTION public
//	Candidate::TableImpl::Put::addConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
addConstraint(Opt::Environment& cEnvironment_,
			  Schema::Constraint* pSchemaConstraint_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Put::getConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const VECTOR<Schema::Constraint*>&
//
// EXCEPTIONS

//virtual
const VECTOR<Schema::Constraint*>&
TableImpl::Put::
getConstraint()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Put::addRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidateFile_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
addRetrieveField(Opt::Environment& cEnvironment_,
				 Candidate::File* pCandidateFile_,
				 Interface::IFile* pFile_,
				 Scalar::Field* pField_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Put::getEstimateCount -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
TableImpl::Put::
getEstimateCount(Opt::Environment& cEnvironment_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Candidate::TableImpl::Put::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Put::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	Execution::Interface::IIterator* pIterator = 0;
	if (getOperand() == 0) {
		// use default values (for insert and undo)
		pIterator = Execution::Iterator::Loop::Once::create(cProgram_);

	} else {
		Candidate::AdoptArgument cOperandArgument(cArgument_);
		cOperandArgument.m_bForUpdate = true;

		if (isStoringNeeded(cEnvironment_) == false) {
			// use input relation
			pIterator = adoptInput(cEnvironment_,
								   cProgram_,
								   cOperandArgument);
		} else {
			// storing is needed
			pIterator = addStoring(cEnvironment_,
								   cProgram_,
								   cOperandArgument);
		}

		// create delayed retrieving program if needed
		getOperand()->generateDelayed(cEnvironment_,
									  cProgram_,
									  pIterator);
	}

	// set candidate to relation
	getTable()->setAdoptCandidate(this);

	// add operation action to the iterator
	// do not reflect candidate modification to parent candidate
	AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pTable = this;
	cMyArgument.setCandidate(this);

	// check constraint
	generateCheckConstraint(cEnvironment_, cProgram_, pIterator, cMyArgument,
							false /* after */);

	if (cEnvironment_.getTransaction().isBatchMode()) {
		// add start batch insert action to start up
		generateStartBatch(cEnvironment_, cProgram_, pIterator, cArgument_);
	}

	// generate put field 
	generatePutField(cEnvironment_, cProgram_, pIterator, cMyArgument);
	// generate put action
	pIterator = generate(cEnvironment_, cProgram_, pIterator, cMyArgument);

	// add to refered table
	cEnvironment_.addReferedTable(getTable()->getSchemaTable());

	return pIterator;
}

// FUNCTION protected
//	Candidate::TableImpl::Put::createTargetColumn -- create target column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
createTargetColumn(Opt::Environment& cEnvironment_,
				   const Utility::FieldSet& cFieldSet_)
{
	m_vecTarget.reserve(cFieldSet_.getSize());
	Opt::MapContainer(cFieldSet_,
					  m_vecTarget,
					  boost::bind(&Scalar::Field::checkPut,
								  _1,
								  boost::ref(cEnvironment_)));
}

// FUNCTION protected
//	Candidate::TableImpl::Put::checkUndo -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	const Utility::UIntSet& cLogRequiredPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
checkUndo(Opt::Environment& cEnvironment_,
		  Interface::IFile* pFile_,
		  Scalar::Field* pField_,
		  const Utility::UIntSet& cLogRequiredPosition_)
{
	; _SYDNEY_ASSERT(pFile_->getSchemaFile()->isAbleToUndo());
	if (pField_->isColumn()
		&& m_cPut.isUndoFile(pFile_) == true
		&& (cLogRequiredPosition_.isContaining(
						  pField_->getSchemaColumn()->getPosition()))) {
		m_cPut.removeUndoFile(pFile_);
	}
}

// FUNCTION protected
//	Candidate::TableImpl::Put::orderCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	FileCandidateMap& cMap_
//	VECTOR<Candidate::File*>& vecCandidate_
//	VECTOR<ParallelSplit>* pvecSplit_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
orderCandidate(Interface::IFile* pFile_,
			   FileCandidateMap& cMap_,
			   VECTOR<Candidate::File*>& vecCandidate_,
			   VECTOR<ParallelSplit>* pvecSplit_)
{
	FileCandidateMap::Iterator iterator = cMap_.find(pFile_);
	if (iterator != cMap_.end()) {
		Candidate::File* pCandidate = (*iterator).second;
		cMap_.erase(iterator);
		PutInfo& cPutInfo = getPutInfo();
		FileDependentMap::Iterator found = cPutInfo.m_mapDependent.find(pFile_);
		if (found != cPutInfo.m_mapDependent.end()) {
			Opt::ForEach((*found).second,
						 boost::bind(&This::orderCandidate,
									 this,
									 _1,
									 boost::ref(cMap_),
									 boost::ref(vecCandidate_),
									 static_cast<VECTOR<ParallelSplit>*>(0)));
		}
		vecCandidate_.PUSHBACK(pCandidate);
		if (pvecSplit_) {
			// record independent position
			pvecSplit_->PUSHBACK(vecCandidate_.GETSIZE());
		}
	}
}

// FUNCTION protected
//	Candidate::TableImpl::Put::generateParallel -- generate parallel plan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	VECTOR<Candidate::File*>& vecCandidate_
//	VECTOR<ParallelSplit>& vecSplit_
//	boost::function<void(Candidate::File*, Candidate::AdoptArgument&)> function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateParallel(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 VECTOR<Candidate::File*>& vecCandidate_,
				 VECTOR<ParallelSplit>& vecSplit_,
				 boost::function<void(Candidate::File*,
									  Candidate::AdoptArgument&)> function_)
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(CheckCancel,
											  cArgument_.m_eTarget));
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(BeginParallel,
											  cArgument_.m_eTarget));

	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Candidate::AdoptArgument::Target::Parallel;

	ParallelSplit iHead = 0;
	VECTOR<ParallelSplit>::ITERATOR iterator = vecSplit_.begin();
	const VECTOR<ParallelSplit>::ITERATOR last = vecSplit_.end();
	for (; iterator != last; ++iterator) {
		ParallelSplit iTail = *iterator;
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(ParallelList,
												  cArgument_.m_eTarget));

		Opt::ForEach(vecCandidate_.begin() + iHead,
					 vecCandidate_.begin() + iTail,
					 boost::bind(function_,
								 _1,
								 boost::ref(cMyArgument)));

		iHead = iTail;
	}
	; _SYDNEY_ASSERT(iHead == vecCandidate_.GETSIZE());

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndParallel,
											  cArgument_.m_eTarget));
}

// FUNCTION protected
//	Candidate::TableImpl::Put::generateObjectLock -- add object lock action
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
TableImpl::Put::
generateObjectLock(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Execution::Interface::IIterator* pIterator_,
				   Candidate::AdoptArgument& cArgument_)
{
	; _SYDNEY_ASSERT(getTable()->getSchemaTable());

	Trans::Transaction& cTrans = cEnvironment_.getTransaction();

	ModVector<Schema::Index*> vecSchemaIndex = getTable()->getSchemaTable()->getIndex(cTrans);
	Opt::FilterContainer(vecSchemaIndex,
						 boost::bind(&Schema::Index::isUnique,
									 _1));

	Opt::ForEach(vecSchemaIndex,
				 boost::bind(&This::generateObjectLockForIndex,
							 this,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 boost::ref(cArgument_),
							 _1));
}

// FUNCTION protected
//	Candidate::TableImpl::Put::getGeneratedObjectID -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
TableImpl::Put::
getGeneratedObjectID(Opt::Environment& cEnvironment_,
					 Interface::IFile* pFile_)
{
	if (pFile_->getSchemaFile()->isKeyGenerated()
		&& pFile_->getSchemaFile()->getObjectID(cEnvironment_.getTransaction())) {
		// add OID
		Scalar::Field* pOIDField = 
			Scalar::Field::create(cEnvironment_,
								  pFile_->getSchemaFile()->getObjectID(
												cEnvironment_.getTransaction()),
								  pFile_,
								  getTable());
		// OID's source value is itself
		return Scalar::UpdateField::create(cEnvironment_,
										   pOIDField,
										   pOIDField);
	}
	return 0;
}

// FUNCTION protected
//	Candidate::TableImpl::Put::generateThis -- common implementation for generate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	boost::function<void(Candidate::File*,Candidate::AdoptArgument&)> function_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Put::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 boost::function<void(Candidate::File*,
								  Candidate::AdoptArgument&)> function_)
{
	// list up file candidates according to the order of operation
	VECTOR<Candidate::File*> vecCandidate;
	VECTOR<ParallelSplit> vecSplit;
	FileCandidateMap cMap(getPutInfo().m_mapFileCandidate);

	vecCandidate.reserve(getPutInfo().m_mapFileCandidate.getSize());

	Opt::ForEachKey(getPutInfo().m_mapDependent,
					boost::bind(&This::orderCandidate,
								this,
								_1,
								boost::ref(cMap),
								boost::ref(vecCandidate),
								static_cast<VECTOR<ParallelSplit>*>(0)));
	if (vecCandidate.ISEMPTY() == false) {
		vecSplit.PUSHBACK(vecCandidate.GETSIZE());
	}

	Opt::ForEachKey(getPutInfo().m_mapFileCandidate,
					boost::bind(&This::orderCandidate,
								this,
								_1,
								boost::ref(cMap),
								boost::ref(vecCandidate),
								&vecSplit));

	// lock
	generateLock(cEnvironment_, cProgram_, pIterator_, cArgument_);

	if (isNeedLog(cEnvironment_)) {
		// log
		generateLog(cEnvironment_, cProgram_, pIterator_);
	} else if (isNeedUndoLog(cEnvironment_)) {
		// reset undoLog
		generateResetUndoLog(cEnvironment_, cProgram_, pIterator_);
	}

	// generate file
	if (vecSplit.GETSIZE() > 1
		&& vecSplit.GETSIZE() <= Opt::Configuration::getThreadMaxNumber()
		&& isCanParallel(cEnvironment_)) {
		// use parallel plan
		generateParallel(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_,
						 vecCandidate,
						 vecSplit,
						 function_);
	} else {
		// use sequential plan
		Opt::ForEach(vecCandidate,
					 boost::bind(function_,
								 _1,
								 boost::ref(cArgument_)));
	}

	return pIterator_;
}

// FUNCTION protected
//	Candidate::TableImpl::Put::adoptInput -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Put::
adoptInput(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_)
{
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);

	// check constraint
	generateCheckConstraint(cEnvironment_, cProgram_, pResult, cArgument_,
							true /* before */);

	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Put::addStoring -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Put::
addStoring(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_)
{
	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	Candidate::Row* pOperandRow = getOperand()->getRow(cEnvironment_);
	bool bDelayed = pOperandRow->delay(cEnvironment_,
									   getOperand(),
									   cDelayArgument);
	Utility::ScalarSet cTuple;
	cTuple.merge(cDelayArgument.m_cKey);
	cTuple.merge(cDelayArgument.m_cNotDelayed);
	if (bDelayed == false) {
		// add key here
		Candidate::Row* pOperandKey = getOperand()->getKey(cEnvironment_);
		cTuple.add(pOperandKey->begin(), pOperandKey->end());
	}

	Candidate::AdoptArgument cAdoptArgument(cArgument_);
	cAdoptArgument.m_pTable = this;
	cAdoptArgument.setCandidate(this);
	cAdoptArgument.m_bCollecting = true;

	// adopt input relation
	Execution::Interface::IIterator* pIterator = adoptInput(cEnvironment_,
															cProgram_,
															cAdoptArgument);

	// generate row data of operand
	VECTOR<int> vecDataID;
	cTuple.mapElement(vecDataID,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator,
								  cAdoptArgument));
	int iRowDataID = cProgram_.addVariable(vecDataID);

	// collection storing all tuples
	Execution::Interface::ICollection* pCollection =
		Execution::Collection::Store::create(cProgram_);

	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Filter::create(cProgram_,
											pCollection->getID());
	// filter input is operand result
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pIterator->getID(),
										iRowDataID));
	cTuple.foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 true /* collecting */));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iRowDataID));
	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateStartBatch -- insert start batch actions to startup
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
TableImpl::Put::
generateStartBatch(Opt::Environment& cEnvironment_,
				   Execution::Interface::IProgram& cProgram_,
				   Execution::Interface::IIterator* pIterator_,
				   Candidate::AdoptArgument& cArgument_)
{
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Transaction::StartBatch::create(
												cProgram_,
												pIterator_,
												getTable()->getSchemaTable()),
							   Execution::Action::Argument::Target::StartUp);
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateCheckConstraint -- add check constraint action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	bool bBefore_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateCheckConstraint(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						Candidate::AdoptArgument& cArgument_,
						bool bBefore_)
{
	if (bBefore_ == false
		&& isCheckColumnConstraint(cEnvironment_)) {
		// Check column constraint
		// for now, only not null constraint is checked
		const VECTOR<Scalar::Field*>& vecTargetColumn = getTargetColumn();
		VECTOR<Scalar::Field*> vecNotNullColumn;
		// select not-null columns
		Opt::FilterContainer(vecTargetColumn, vecNotNullColumn,
							 boost::bind(&Scalar::Field::isKnownNotNull,
										 _1,
										 boost::ref(cEnvironment_)));
		Opt::ForEach(vecNotNullColumn,
					 boost::bind(&This::generateCheckColumnConstraint,
								 this,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_),
								 _1));
	}
	if (bBefore_ == false
		&& m_vecReferenceAfter.ISEMPTY() == false) {
		// Check table constraint
		Opt::ForEach(m_vecReferenceAfter,
					 boost::bind(&This::generateCheckTableConstraints,
								 this,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_),
								 _1));
	}
	if (bBefore_ == true
		&& m_vecReferenceBefore.ISEMPTY() == false) {
		// Check table constraint
		Opt::ForEach(m_vecReferenceBefore,
					 boost::bind(&This::generateCheckTableConstraints,
								 this,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_),
								 _1));
	}
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateCheckColumnConstraint -- add check column constraint action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateCheckColumnConstraint(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_,
							  Scalar::Field* pColumn_)
{
	if (pColumn_->isRowID() == false) {
		// check not null constraint
		if (pColumn_->isUpdate()) {
			if (Interface::IScalar* pInput = pColumn_->getUpdate()->getInput()) {
				if (pInput->isKnownNotNull(cEnvironment_)) {
					// input data is known as not null -> do nothing
					return;
				}
			}

			int iDataID = pColumn_->generate(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument_);
			Execution::Interface::IPredicate* pIsNull =
				Execution::Predicate::Comparison::create(
									 cProgram_,
									 pIterator_,
									 iDataID,
									 Execution::Predicate::Comparison::Type::IsNull);
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(If,
													  pIsNull->getID(),
													  cArgument_.m_eTarget));
			pIterator_->addCalculation(cProgram_,
									   Execution::Operator::Throw::create(
												  cProgram_,
												  pIterator_,
												  Exception::NullabilityViolation(
													  moduleName,
													  srcFile,
													  __LINE__,
													  pColumn_->getName())),
									   cArgument_.m_eTarget);
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateCheckTableConstraints --
//					add check table constraint actions for one refered table
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::Table* pReference_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateCheckTableConstraints(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  Candidate::AdoptArgument& cArgument_,
							  Candidate::Table* pReference_)
{
	// add to refered table
	cEnvironment_.addReferedTable(pReference_->getTable()->getSchemaTable());

	if (pReference_->getConstraint().ISEMPTY() == false) {
		// add convert lock action
		Execution::Action::LockerArgument cPrevArgument;
		Execution::Action::LockerArgument cPostArgument;
		if (setLockMode(cEnvironment_,
						cProgram_,
						pReference_->getTable()->getSchemaTable(),
						Lock::Name::Category::Table,
						Lock::Name::Category::Tuple,
						cPrevArgument)
			&&
			setLockMode(cEnvironment_,
						cProgram_,
						pReference_->getTable()->getSchemaTable(),
						Lock::Name::Category::Table,
						Lock::Name::Category::Table,
						cPostArgument)) {
			// add convert lock action
			Execution::Operator::Locker* pLock =
				Execution::Operator::Locker::ConvertTable::create(cProgram_,
																  pIterator_,
																  cPostArgument,
																  cPrevArgument);
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(Calculation,
													  pLock->getID(),
													  cArgument_.m_eTarget));
		}

		Opt::ForEach(pReference_->getConstraint(),
					 boost::bind(&This::generateCheckTableConstraint,
								 this,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_),
								 pReference_,
								 _1));
	}
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateCheckTableConstraint -- add check table constraint action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Candidate::Table* pReference_
//	Schema::Constraint* pSchemaConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateCheckTableConstraint(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 Candidate::Table* pReference_,
							 Schema::Constraint* pSchemaConstraint_)
{
	; _SYDNEY_ASSERT(pSchemaConstraint_->getCategory() == Schema::Constraint::Category::ForeignKey
					 || pSchemaConstraint_->getCategory() == Schema::Constraint::Category::ReferedKey);

	Trans::Transaction& cTrans = cEnvironment_.getTransaction();

	// create key fields for checking
	const ModVector<Schema::ObjectID::Value>& vecKeyID = pSchemaConstraint_->getColumnID();
	VECTOR<Interface::IScalar*> vecCheckValue;
	vecCheckValue.reserve(vecKeyID.getSize());
	Opt::MapContainer(vecKeyID, vecCheckValue,
					  boost::bind(&This::getCheckValue,
								  this,
								  boost::ref(cEnvironment_),
								  pSchemaConstraint_,
								  boost::bind(&Schema::Table::getColumnByID,
											  getTable()->getSchemaTable(),
											  _1,
											  boost::ref(cTrans))));

	// get refered index
	Schema::Object::ID::Value iIndexID = pSchemaConstraint_->getReferedIndexID();
	; _SYDNEY_ASSERT(iIndexID != Schema::Object::ID::Invalid);

	Schema::Index* pReferedIndex =
		pReference_->getTable()->getSchemaTable()->getIndex(iIndexID, cTrans);
	; _SYDNEY_ASSERT(pReferedIndex);

	// generate file fetch iterator using constraint
	Execution::Interface::IIterator* pCheckIterator =
		generateFileFetch(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  pReferedIndex,
						  pReference_,
						  vecCheckValue);

	// if first key is null, skip following predicate
	; _SYDNEY_ASSERT(vecCheckValue.ISEMPTY() == false);
	int iFirstValueID = vecCheckValue[0]->generate(cEnvironment_,
												   cProgram_,
												   pIterator_,
												   cArgument_);
	Execution::Interface::IPredicate* pIsNull =
		Execution::Predicate::Comparison::create(cProgram_,
												 pIterator_,
												 iFirstValueID,
												 Execution::Predicate::Comparison::Type::IsNull);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsNull->getID(),
											  cArgument_.m_eTarget));

	// create empty check predicate
	Execution::Interface::IPredicate* pIsEmpty =
		Execution::Predicate::IsEmpty::Iterator::create(cProgram_,
														pIterator_,
														pCheckIterator->getID());

	switch (pSchemaConstraint_->getCategory()) {
	case Schema::Constraint::Category::ForeignKey:
		{
			// if empty, constraint is violated
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(If,
													  pIsEmpty->getID(),
													  cArgument_.m_eTarget));
			pIterator_->addCalculation(cProgram_,
									   Execution::Operator::Throw::create(
												cProgram_,
												pIterator_,
												Exception::ForeignKeyViolation(
													moduleName,
													srcFile,
													__LINE__)),
									   cArgument_.m_eTarget);
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
			break;
		}
	case Schema::Constraint::Category::ReferedKey:
		{
			// if not empty, constraint is violated
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT1_T(Unless,
													  pIsEmpty->getID(),
													  cArgument_.m_eTarget));
			pIterator_->addCalculation(cProgram_,
									   Execution::Operator::Throw::create(
												cProgram_,
												pIterator_,
												Exception::ReferedKeyViolation(
													moduleName,
													srcFile,
													__LINE__)),
									   cArgument_.m_eTarget);
			pIterator_->addAction(cProgram_,
								  _ACTION_ARGUMENT0_T(EndIf,
													  cArgument_.m_eTarget));
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}

	// endif for isnull
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateFileFetch -- generate file fetch iterator using constraint and key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Schema::Index* pSchemaIndex_
//	Candidate::Table* pReference_
//	const VECTOR<Interface::IScalar*>& vecCheckValue_
//	bool bForLock_ = false
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

Execution::Interface::IIterator*
TableImpl::Put::
generateFileFetch(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Schema::Index* pSchemaIndex_,
				  Candidate::Table* pReference_,
				  const VECTOR<Interface::IScalar*>& vecCheckValue_,
				  bool bForLock_ /* = false */)
{
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();

	Schema::File* pReferedFile = pSchemaIndex_->getFile(cTrans);
	; _SYDNEY_ASSERT(pReferedFile);

	// create candidate file object
	Interface::IFile* pFile = Interface::IFile::create(cEnvironment_,
													   pReferedFile);
	Candidate::File* pCandidateFile =
		pFile->createCandidate(cEnvironment_,
							   pReference_,
							   File::Parameter::create(cEnvironment_));

	// rowid field
	Schema::Column* pRowIDColumn = pReference_->getTable()->getSchemaTable()->getTupleID(cTrans);
	; _SYDNEY_ASSERT(pRowIDColumn);
	Schema::Field* pRowIDField = pRowIDColumn->getField(cTrans, pReferedFile);
	if (pRowIDField == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	Scalar::Field* pRowID = pReference_->getTable()->createField(cEnvironment_,
																 pRowIDField,
																 pFile);
	pReference_->setRowID(pRowID);

	// create key fields
	ModVector<Schema::Field*> vecKeyField =
		pReferedFile->getField(Schema::Field::Category::Key,
							   cTrans);
	VECTOR<Interface::IScalar*> vecKey;
	Opt::MapContainer(vecKeyField, vecKey,
					  boost::bind(&Relation::Table::createField,
								  pReference_->getTable(),
								  boost::ref(cEnvironment_),
								  _1,
								  pFile,
								  static_cast<Scalar::Field*>(0)));
	; _SYDNEY_ASSERT(vecKey.GETSIZE() == vecCheckValue_.GETSIZE());

	// create value field
	ModVector<Schema::Field*> vecRetrieveField =
		pReferedFile->getField(Schema::Field::Category::Data,
							   cTrans);
	VECTOR<Scalar::Field*> vecRetrieveValue;
	Opt::MapContainer(vecRetrieveField, vecRetrieveValue,
					  boost::bind(&Relation::Table::createField,
								  pReference_->getTable(),
								  boost::ref(cEnvironment_),
								  _1,
								  pFile,
								  static_cast<Scalar::Field*>(0)));
	// add value field as retrieved field
	Opt::ForEach(vecRetrieveValue,
				 boost::bind(&Candidate::File::addField,
							 pCandidateFile,
							 boost::ref(cEnvironment_),
							 _1));
	// create fetch predicate
	Interface::IPredicate* pFetch =
		Predicate::Fetch::create(cEnvironment_,
								 Predicate::Comparison::create(cEnvironment_,
															   Tree::Node::Equals,
															   MAKEPAIR(vecKey, vecCheckValue_),
															   false /* do not check comparability */),
								 vecKey,
								 vecCheckValue_);
	if (pCandidateFile->isSearchable(cEnvironment_,
									 pFetch)) {
		Execution::Action::FileAccess* pFileAccess = pCandidateFile->createFileAccess(cProgram_);

		if (bForLock_) {
			// set special parameter to openoption
			pFileAccess->getOpenOption().setBoolean(LogicalFile::OpenOption::KeyNumber::GetForConstraintLock,
													true);
		}

		Candidate::AdoptArgument cArgument;
		cArgument.m_pTable = this;
		// treat as collecting because checked tuple should exists until end transaction
		cArgument.m_bCollecting = true;

		cArgument.m_bForLock = bForLock_;

		Candidate::AdoptIndexArgument cIndexArgument;
		Execution::Interface::IIterator* pResult =
			pCandidateFile->createScan(cEnvironment_,
									   cProgram_,
									   pFileAccess,
									   cArgument,
									   cIndexArgument);
		int iKeyID = pFetch->generateKey(cEnvironment_,
										 cProgram_,
										 pIterator_, // use target iterator
										 cArgument);
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT1(Input,
											 iKeyID));

		return pResult;
	}
	// Can't fetch -> should not happen
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Candidate::TableImpl::Put::getCheckValue -- get checked value for constraint
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Constraint* pSchemaConstraint_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Put::
getCheckValue(Opt::Environment& cEnvironment_,
			  Schema::Constraint* pSchemaConstraint_,
			  Schema::Column* pSchemaColumn_)
{

	Interface::IScalar* pResult = 0;
	if (pSchemaConstraint_ == 0
		|| pSchemaConstraint_->getCategory() == Schema::Constraint::Category::ForeignKey) {
		// use post-update values
		pResult = getField(cEnvironment_,
						   pSchemaColumn_);
	}
	if (pResult == 0) {
		// column is not included in target field or constraint uses pre-update values
		// -> use original value
		Scalar::Field* pTargetField = Scalar::Field::create(cEnvironment_,
															pSchemaColumn_,
															getTable());
		pResult = getOriginalField(cEnvironment_,
								   pTargetField);
	}
	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Put::generatePutField -- generate put field
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
TableImpl::Put::
generatePutField(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	ForEachValue(getPutInfo().m_mapFileCandidate,
				 boost::bind(&Candidate::File::generatePutField,
							 _1,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 boost::ref(cArgument_)));
}

// FUNCTION private
//	Candidate::TableImpl::Put::isCanParallel -- check whether parallel execution can be considered
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
TableImpl::Put::
isCanParallel(Opt::Environment& cEnvironment_)
{
	if (cEnvironment_.getTransaction().isBatchMode()) {
		// in batch mode, parallel insert can't be applied
		return false;
	}

	// parallel execution can be considered only when
	// no index-scaned files keep latch
	VECTOR<Interface::IFile*> vecIndexScan;
	Opt::ForEachKey(getPutInfo().m_mapFileCandidate,
					boost::bind(&VECTOR<Interface::IFile*>::pushBack,
								&vecIndexScan,
								_1));
	Opt::FilterContainer(vecIndexScan,
						 boost::bind(&Opt::Environment::isIndexScan,
									 &cEnvironment_,
									 boost::bind(&Interface::IFile::getSchemaFile,
												 _1)));
	Opt::FilterContainer(vecIndexScan,
						 boost::bind(&Interface::IFile::isKeepLatch,
									 _1,
									 boost::ref(cEnvironment_)));

	return vecIndexScan.ISEMPTY();
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateObjectLockForIndex -- add object lock action for one index
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Schema::Index* pSchemaIndex_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Put::
generateObjectLockForIndex(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_,
						   Schema::Index* pSchemaIndex_)
{
	// create key fields for checking
	ModVector<Schema::Column*> vecKeyColumn = pSchemaIndex_->getColumn(cEnvironment_.getTransaction());
	VECTOR<Interface::IScalar*> vecCheckValue;
	vecCheckValue.reserve(vecKeyColumn.getSize());
	Opt::MapContainer(vecKeyColumn, vecCheckValue,
					  boost::bind(&This::getCheckValue,
								  this,
								  boost::ref(cEnvironment_),
								  static_cast<Schema::Constraint*>(0),
								  _1));

	// create refer candidate
	Candidate::Table* pRefer = Table::Refer::create(cEnvironment_,
													getTable(),
													getTable());

	// generate file fetch iterator using constraint
	Execution::Interface::IIterator* pCheckIterator =
		generateFileFetch(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  pSchemaIndex_,
						  pRefer,
						  vecCheckValue,
						  true /* for lock */);

	// if first key is null, skip following predicate
	; _SYDNEY_ASSERT(vecCheckValue.ISEMPTY() == false);
	int iFirstValueID = vecCheckValue[0]->generate(cEnvironment_,
												   cProgram_,
												   pIterator_,
												   cArgument_);
	Execution::Interface::IPredicate* pIsNull =
		Execution::Predicate::Comparison::create(cProgram_,
												 pIterator_,
												 iFirstValueID,
												 Execution::Predicate::Comparison::Type::IsNull);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  pIsNull->getID(),
											  cArgument_.m_eTarget));

	// iterate check iterator
	pIterator_->addCalculation(cProgram_, 
							   Execution::Operator::Iterate::Once::create(
												 cProgram_,
												 pIterator_,
												 pCheckIterator->getID()));

	// endif for isnull
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateLock -- add lock action
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
TableImpl::Put::
generateLock(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	; // default: do nothing
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateLog -- add log action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
generateLog(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_)
{
	; // default: do nothing
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateLogType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
TableImpl::Put::
generateLogType(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_)
{
	// default: do nothing
	return -1;
}

// FUNCTION private
//	Candidate::TableImpl::Put::generateResetUndoLog -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
generateResetUndoLog(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_)
{
	int iLogTypeID = generateLogType(cEnvironment_, cProgram_);
	if (iLogTypeID >= 0) {
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::UndoLog::Reset::create(
											cProgram_,
											pIterator_,
											iLogTypeID));
	}
}

// FUNCTION private
//	Candidate::TableImpl::Put::isNeedUndoLog -- 
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
TableImpl::Put::
isNeedUndoLog(Opt::Environment& cEnvironment_)
{
	return cEnvironment_.getTransaction().isNecessaryLog() == true
		&& cEnvironment_.isRecovery() == true;
}

// FUNCTION private
//	Candidate::TableImpl::Put::getField -- get checked field
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Put::
getField(Opt::Environment& cEnvironment_,
		 Schema::Column* pSchemaColumn_)
{
	return getTable()->getField(pSchemaColumn_);
}

// FUNCTION private
//	Candidate::TableImpl::Put::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Put::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	// reset by zero
	cCost_.reset();
	// set dummy tuple count
	cCost_.setTupleCount(1);
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Insert

// FUNCTION public
//	Candidate::TableImpl::Insert::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Insert::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// enumerate target files to insert
	enumerate(cEnvironment_,
			  cFieldSet_);
}

// FUNCTION private
//	Candidate::TableImpl::Insert::enumerate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
enumerate(Opt::Environment& cEnvironment_,
		  const Utility::FieldSet& cFieldSet_)
{
	// list up all the target files
	createTargetColumn(cEnvironment_,
					   cFieldSet_);

	// add each target files
	Opt::ForEach(getTargetColumn(),
				 boost::bind(&This::addPutFile,
							 this,
							 boost::ref(cEnvironment_),
							 _1));
	// add target fields
	Opt::ForEachValue(getPutInfo().m_mapFileCandidate,
					  boost::bind(&This::addPutField,
								  this,
								  boost::ref(cEnvironment_),
								  _1));
}

// FUNCTION private
//	Candidate::TableImpl::Insert::addPutFile -- check put files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
addPutFile(Opt::Environment& cEnvironment_,
		   Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_->isChecked());
	; _SYDNEY_ASSERT(pColumn_->isUpdate());

	if (pColumn_->isRowID()) {
		setRowID(pColumn_);
	}

	const Utility::FileSet& cFileSet = pColumn_->getChecked()->getFileSet();
	cFileSet.foreachElement(boost::bind(&This::addFile,
										this,
										boost::ref(cEnvironment_),
										_1));
}

// FUNCTION private
//	Candidate::TableImpl::Insert::addFile -- add inserted file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
addFile(Opt::Environment& cEnvironment_,
		Interface::IFile* pFile_)
{
	// get candidate::file from ifile
	Candidate::File* pCandidateFile = getPutInfo().getCandidate(pFile_);
	if (pCandidateFile == 0) {
		// create new candidate::file
		pCandidateFile = pFile_->createCandidate(cEnvironment_,
												 this,
												 File::Parameter::create(cEnvironment_));
		// add to putinfo
		getPutInfo().addCandidate(pCandidateFile);

		if (cEnvironment_.isRollback() == false
			|| pFile_->getSchemaFile()->isAbleToUndo() == false) {
			if (Scalar::Field* pOIDUpdate = getGeneratedObjectID(cEnvironment_,
																 pFile_)) {
				// add files for OID and destinations
				addPutFile(cEnvironment_, pOIDUpdate->checkPut(cEnvironment_));
			}
		}
	}
	; _SYDNEY_ASSERT(pCandidateFile);
}

// FUNCTION private
//	Candidate::TableImpl::Insert::addPutField -- add all inserted fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
addPutField(Opt::Environment& cEnvironment_,
			Candidate::File* pCandidate_)
{
	const ModVector<Schema::Field*>& vecSchemaField =
		pCandidate_->getFile()->getSchemaFile()->getField(cEnvironment_.getTransaction());

	Opt::ForEach(vecSchemaField,
				 boost::bind(&This::addField,
							 this,
							 boost::ref(cEnvironment_),
							 pCandidate_,
							 _1));
}

// FUNCTION private
//	Candidate::TableImpl::Insert::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Insert::
addField(Opt::Environment& cEnvironment_,
		 Candidate::File* pCandidate_,
		 Schema::Field* pSchemaField_)
{
	// add only for putable fields 
	if (pSchemaField_->isPutable() == false) return;

	Scalar::Field* pField = getTable()->getField(pSchemaField_);
	if (pField == 0) {
		pField = Scalar::Field::create(cEnvironment_,
									   pSchemaField_,
									   pCandidate_->getFile(),
									   getTable());
	}

	if (pField->isUpdate() == false) {
		Interface::IScalar* pInput = getSourceValue(cEnvironment_,
													pCandidate_,
													pSchemaField_);
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField,
											 pInput);
	}

	; _SYDNEY_ASSERT(pField->isUpdate());
	pCandidate_->addInsertField(cEnvironment_,
								pField);
	if (pField->isPutKey()) {
		pCandidate_->addPutKey(cEnvironment_,
							   pField);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Insert::getSourceValue -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Insert::
getSourceValue(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	Interface::IScalar* pResult = 0;

	Schema::Field* pSourceSchemaField =
		pSchemaField_->getSource(cEnvironment_.getTransaction());
	if (pSourceSchemaField) {
		Scalar::Field* pSourceField = getTable()->getField(pSourceSchemaField);
		if (pSourceField) {
			; _SYDNEY_ASSERT(pSourceField->isUpdate());
			pResult = pSourceField->getUpdate()->getInput();

			if ((pSourceSchemaField->isObjectID()
				 || pSourceSchemaField->isKey())
				&& pSourceField->getFile()->getSchemaFile()->isKeyGenerated()) {
				// register file->source relationship
				getPutInfo().m_mapDependent[pCandidate_->getFile()].PUSHBACK(pSourceField->getFile());
			}
		}
	}
	if (pResult == 0) {
		pResult = Scalar::Value::Null::create(cEnvironment_);
	}

	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Insert::generateLock -- add lock action
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
TableImpl::Insert::
generateLock(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		if (getTable()->getSchemaTable()->isTemporary() == false) {

			if (cEnvironment_.isRecovery() == false) {
				generateObjectLock(cEnvironment_, cProgram_, pIterator_, cArgument_);
			}

			Execution::Action::LockerArgument cLockerArgument;
			cLockerArgument.setTable(getTable()->getSchemaTable());
			cLockerArgument.m_bIsPrepare = cEnvironment_.isPrepare();
			cLockerArgument.m_bIsUpdate = true;
			if (Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
																 Lock::Name::Category::Tuple,
																 false, /* readwrite */
																 cEnvironment_.getTransaction().isBatchMode(),
																 cLockerArgument)) {
				Candidate::AdoptArgument cArgument;
				cArgument.m_pTable = this;
				int iDataID = pRowID->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument);
				pIterator_->addCalculation(cProgram_,
										   Execution::Operator::Locker::Tuple::create(
												  cProgram_,
												  pIterator_,
												  cLockerArgument,
												  iDataID),
										   cArgument.m_eTarget);
			}
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Insert::generateLog -- add log action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Insert::
generateLog(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		//////////////////////////////////////////////////
		// Insert logical log:
		// {INSERT, <tableID>, <rowID>, {<columnID>, ...}, {<data>, ...}}
		VECTOR<int> vecLogData;
		vecLogData.reserve(Opt::LogData::Format::SingleValueNum);

		// logtype is used apart
		int iLogType = generateLogType(cEnvironment_, cProgram_);
		Candidate::AdoptArgument cArgument;
		cArgument.m_pTable = this;
		cArgument.setCandidate(this);

		vecLogData.PUSHBACK(iLogType);
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  getTable()->getSchemaTable()->getID())));
		vecLogData.PUSHBACK(pRowID->generate(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument));

		ModVector<unsigned int> vecColumnID;
		VECTOR<int> vecData;
		Opt::MapContainer(getLogged(), vecColumnID,
						  boost::bind(&Schema::Column::getID,
									  boost::bind(&Scalar::Field::getSchemaColumn,
												  _1)));
		Opt::MapContainer(getLogged(), vecData,
						  boost::bind(&Scalar::Field::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument)));
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecColumnID)));
		vecLogData.PUSHBACK(cProgram_.addVariable(vecData));

		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Logger::create(
											cProgram_,
											pIterator_,
											cProgram_.addVariable(vecLogData),
											iLogType),
								   cArgument.m_eTarget);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Insert::generateLogType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
TableImpl::Insert::
generateLogType(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_)
{
	return cProgram_.addVariable(new Common::IntegerData(Opt::LogData::Type::Insert));
}

// FUNCTION private
//	Candidate::TableImpl::Insert::isStoringNeeded -- 
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
TableImpl::Insert::
isStoringNeeded(Opt::Environment& cEnvironment_)
{
	return Inquiry::isReferTable(cEnvironment_,
								 getOperand(),
								 getTable()->getSchemaTable());
}

// FUNCTION private
//	Candidate::TableImpl::Insert::isCheckColumnConstraint -- 
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
TableImpl::Insert::
isCheckColumnConstraint(Opt::Environment& cEnvironment_)
{
	// check column constraint only in normal mode
	return cEnvironment_.isRecovery() == false;
}

// FUNCTION private
//	Candidate::TableImpl::Insert::generate -- 
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
TableImpl::Insert::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	generateThis(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 boost::bind(&Candidate::File::createInsert,
							 _1,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 _2));
	return pIterator_;
}

// FUNCTION private
//	Candidate::TableImpl::Insert::getOriginalField -- get original value for a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Insert::
getOriginalField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_)
{
	return Scalar::Value::Null::create(cEnvironment_);
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Delete

// FUNCTION public
//	Candidate::TableImpl::Delete::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Delete::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// enumerate target files to delete
	enumerate(cEnvironment_,
			  cFieldSet_);
}

// FUNCTION protected
//	Candidate::TableImpl::Delete::adoptInput -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Delete::
adoptInput(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_bCollecting
		|| cEnvironment_.isRecovery()) {
		return Super::adoptInput(cEnvironment_,
								 cProgram_,
								 cArgument_);
	}

	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	Candidate::Row* pOperandRow = getOperand()->getRow(cEnvironment_);
	bool bDelayed = pOperandRow->delay(cEnvironment_,
									   getOperand(),
									   cDelayArgument);
	Utility::ScalarSet cTuple;
	cTuple.merge(cDelayArgument.m_cKey);
	cTuple.merge(cDelayArgument.m_cNotDelayed);
	if (bDelayed == false) {
		// add key here
		Candidate::Row* pOperandKey = getOperand()->getKey(cEnvironment_);
		cTuple.add(pOperandKey->begin(), pOperandKey->end());
	}

	Candidate::AdoptArgument cAdoptArgument(cArgument_);
	cAdoptArgument.m_pTable = this;
	cAdoptArgument.setCandidate(this);
	cAdoptArgument.m_bCollecting = true;

	// adopt input relation
	Execution::Interface::IIterator* pIterator = Super::adoptInput(cEnvironment_,
																   cProgram_,
																   cAdoptArgument);

	// generate row data of operand
	VECTOR<int> vecDataID;
	cTuple.mapElement(vecDataID,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator,
								  cAdoptArgument));
	int iRowDataID = cProgram_.addVariable(vecDataID);

	// queue collection
	Execution::Collection::Queue* pQueue =
		Execution::Collection::Queue::create(cProgram_,
											 2 /* doubling */);

	// create filtering iterator using queue collection
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Filter::create(cProgram_,
											pQueue->getID());
	// filter input is operand result
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pIterator->getID(),
										iRowDataID));
	VECTOR<int> vecGetDataID;
	cTuple.mapElement(vecGetDataID,
					  boost::bind(&Interface::IScalar::generateFromType,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pResult,
								  boost::ref(cArgument_)));
	int iGetDataID = cProgram_.addVariable(vecGetDataID);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iGetDataID));

	// create delayed retrieving program if needed
	getOperand()->generateDelayed(cEnvironment_,
								  cProgram_,
								  pResult);

	return pResult;
	
}

// FUNCTION private
//	Candidate::TableImpl::Delete::enumerate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
enumerate(Opt::Environment& cEnvironment_,
		  const Utility::FieldSet& cFieldSet_)
{
	// set rowid using retrieved rowid
	setRowID(m_pRetrieve->getScalar(cEnvironment_, 0)->getField());

	// list up all the files for each target columns
	createTargetColumn(cEnvironment_,
					   cFieldSet_);

	// add file candidate for each target files
	Opt::ForEach(getTargetColumn(),
				 boost::bind(&This::addPutFile,
							 this,
							 boost::ref(cEnvironment_),
							 _1));
	// add target fields
	Opt::ForEachValue(getPutInfo().m_mapFileCandidate,
					  boost::bind(&This::addPutField,
								  this,
								  boost::ref(cEnvironment_),
								  _1));

	// create pre-updated data for logical log
	createPrevLogged(cEnvironment_);
}

// FUNCTION private
//	Candidate::TableImpl::Delete::createPrevLogged -- create previous value for logged fields
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
TableImpl::Delete::
createPrevLogged(Opt::Environment& cEnvironment_)
{
	m_vecPrevLogged.reserve(getLogged().GETSIZE());
	Opt::MapContainer(getLogged(),
					  m_vecPrevLogged,
					  boost::bind(&This::getRetrieveField,
								  this,
								  boost::ref(cEnvironment_),
								  _1,
								  true /* consider undo */));
}

// FUNCTION private
//	Candidate::TableImpl::Delete::getRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	bool bConsiderUndo_ = false
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Delete::
getRetrieveField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_,
				 bool bConsiderUndo_ /* = false */)
{
	Interface::IScalar* pResult = 0;

	if (bConsiderUndo_
		&& pField_->isColumn()) {
		; _SYDNEY_ASSERT(pField_->isUpdate());

		// check put file
		Scalar::Field* pCheckedField = pField_->checkPut(cEnvironment_);
		; _SYDNEY_ASSERT(pCheckedField->isChecked());

		const Utility::FileSet& cPutFile = pCheckedField->getChecked()->getFileSet();
		Utility::FileSet::ConstIterator found =
			Opt::Find(cPutFile.begin(), cPutFile.end(),
					   boost::bind(&PutInfo::isUndoFile,
								   &getPutInfo(),
								   _1));
		if (found != cPutFile.end()) {
			// delete can be undoed
			// -> use OID field for previous value in log instead
			Schema::Field* pOIDField =
				(*found)->getSchemaFile()->getObjectID(cEnvironment_.getTransaction());
			; _SYDNEY_ASSERT(pOIDField);

			pResult = Scalar::Field::create(cEnvironment_,
											pOIDField,
											*found,
											m_pRetrieve);
			// set field as retrieved
			pResult->retrieve(cEnvironment_,
							  getOperand());
		}
	}
	if (pResult == 0) {
		pResult = getOriginalField(cEnvironment_,
								   pField_);
	}
	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Delete::addPutFile -- check put files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
addPutFile(Opt::Environment& cEnvironment_,
		   Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_->isChecked());
	; _SYDNEY_ASSERT(pColumn_->isUpdate());

	const Utility::FileSet& cFileSet = pColumn_->getChecked()->getFileSet();
	cFileSet.foreachElement(boost::bind(&This::addFile,
										this,
										boost::ref(cEnvironment_),
										_1,
										pColumn_));
}

// FUNCTION private
//	Candidate::TableImpl::Delete::addFile -- add file candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
addFile(Opt::Environment& cEnvironment_,
		Interface::IFile* pFile_,
		Scalar::Field* pField_)
{
	; _SYDNEY_ASSERT(pField_->isChecked() || pField_->getFile() == pFile_);
	; _SYDNEY_ASSERT(pField_->isUpdate());

	Scalar::Field* pField = pField_->getField(pFile_);
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	; _SYDNEY_ASSERT(pField->isUpdate());

	// get candidate::file from ifile
	Candidate::File* pCandidateFile = getPutInfo().getCandidate(pFile_);
	if (pCandidateFile == 0) {
		// create new candidate::file
		pCandidateFile = pFile_->createCandidate(cEnvironment_,
												 this,
												 File::Parameter::create(cEnvironment_));
		// add to putinfo
		getPutInfo().addCandidate(pCandidateFile);

		if (pField_->isColumn()
			&& pFile_->getSchemaFile()->isAbleToUndo()) {
			getPutInfo().addUndoFile(pFile_);
		}

		if (pFile_->getSchemaFile()->isKeyGenerated()
			&& pFile_->getSchemaFile()->getObjectID(cEnvironment_.getTransaction())) {
			// add OID
			Scalar::Field* pOIDField =
				getUpdateField(cEnvironment_,
							   pCandidateFile,
							   pFile_->getSchemaFile()->getObjectID(
										cEnvironment_.getTransaction()));
			// add to put key
			pCandidateFile->addPutKey(cEnvironment_,
									  pOIDField);
			// add files for OID and destinations
			Scalar::Field* pOIDChecked = pOIDField->checkPut(cEnvironment_);
			pOIDChecked->getChecked()->getFileSet().foreachElement(
								   boost::bind(&This::setDependency,
											   this,
											   boost::ref(cEnvironment_),
											   _1,
											   pFile_));
			addPutFile(cEnvironment_, pOIDChecked);
		}
	}
	; _SYDNEY_ASSERT(pCandidateFile);

	// add target field to file
	pCandidateFile->addField(cEnvironment_,
							 pField);
}

// FUNCTION private
//	Candidate::TableImpl::Delete::addPutField -- add put fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
addPutField(Opt::Environment& cEnvironment_,
			Candidate::File* pCandidate_)
{
	// add putkey fields
	const ModVector<Schema::Field*>& vecPutKey =
		pCandidate_->getFile()->getSchemaFile()->getPutKey(cEnvironment_.getTransaction());
	FOREACH(vecPutKey,
			boost::bind(&Candidate::File::addPutKey,
						pCandidate_,
						boost::ref(cEnvironment_),
						boost::bind(&This::getUpdateField,
									this,
									boost::ref(cEnvironment_),
									pCandidate_,
									_1)));
	if (cEnvironment_.isRecovery() == false) {
		// add all fields for undo expunge
		const ModVector<Schema::Field*>& vecField =
			pCandidate_->getFile()->getSchemaFile()->getField(cEnvironment_.getTransaction());
		FOREACH(vecField,
				boost::bind(&This::addInsertField,
							this,
							boost::ref(cEnvironment_),
							pCandidate_,
							_1));
	}
}

// FUNCTION private
//	Candidate::TableImpl::Delete::addInsertField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
addInsertField(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	if (pSchemaField_->isPutable() == false) {
		return;
	}
	Scalar::Field* pField = getUpdateField(cEnvironment_,
										   pCandidate_,
										   pSchemaField_);

	; _SYDNEY_ASSERT(pField->isUpdate());

	if (pSchemaField_->isObjectID() == false) {
		// add to undo field
		; _SYDNEY_ASSERT(pField->getUpdate()->getOriginal());
		pCandidate_->addUndoField(cEnvironment_,
								  pField);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Delete::getUpdateField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
TableImpl::Delete::
getUpdateField(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	Scalar::Field* pField = getTable()->getField(pSchemaField_);
	if (pField == 0) {
		pField = Scalar::Field::create(cEnvironment_,
									   pSchemaField_,
									   pCandidate_->getFile(),
									   getTable());
	}

	if (pField->isUpdate() == false) {
		Interface::IScalar* pOriginal = 0;
		Interface::IScalar* pInput = 0;

		if (pSchemaField_->isGetable()) {
			pOriginal =
				getRetrieveField(cEnvironment_,
								 pField);
		}
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField,
											 MAKEPAIR(pInput, pOriginal));
	}
	return pField;
}

// FUNCTION private
//	Candidate::TableImpl::Delete::setDependency -- set file dependency
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Interface::IFile* pSourceFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Delete::
setDependency(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Interface::IFile* pSourceFile_)
{
	if (pFile_ != pSourceFile_) {
		// register file->source relationship in reversed order
		getPutInfo().m_mapDependent[pSourceFile_].PUSHBACK(pFile_);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Delete::generateLock -- add lock action
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
TableImpl::Delete::
generateLock(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		if (getTable()->getSchemaTable()->isTemporary() == false) {
			Execution::Action::LockerArgument cLockerArgument;
			cLockerArgument.setTable(getTable()->getSchemaTable());
			cLockerArgument.m_bIsPrepare = cEnvironment_.isPrepare();
			cLockerArgument.m_bIsUpdate = true;
			Execution::Action::LockerArgument cPrevLockerArgument;
			cPrevLockerArgument.setTable(getTable()->getSchemaTable());
			cPrevLockerArgument.m_bIsPrepare = cEnvironment_.isPrepare();
			cPrevLockerArgument.m_bIsUpdate = true;
			if (Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
																 Lock::Name::Category::Tuple,
																 true, /* read only */
																 cEnvironment_.getTransaction().isBatchMode(),
																 cPrevLockerArgument)
				&& Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
																	Lock::Name::Category::Tuple,
																	false, /* readwrite */
																	cEnvironment_.getTransaction().isBatchMode(),
																	cLockerArgument)) {
				Candidate::AdoptArgument cArgument;
				cArgument.m_pTable = this;
				int iDataID = pRowID->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument);
				pIterator_->addCalculation(cProgram_,
										   Execution::Operator::Locker::ConvertTuple::create(
												  cProgram_,
												  pIterator_,
												  cLockerArgument,
												  cPrevLockerArgument,
												  iDataID),
										   cArgument.m_eTarget);
			}
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Delete::generateLog -- add log action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Delete::
generateLog(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		//////////////////////////////////////////////////
		// Delete logical log:
		// {DELETE, <tableID>, <rowID>, {<columnID>, ...}, {<data>, ...}, {<data>, ...}}
		VECTOR<int> vecLogData;
		vecLogData.reserve(Opt::LogData::Format::SingleValueNum);

		// logtype is used apart
		int iLogType = generateLogType(cEnvironment_, cProgram_);
		Candidate::AdoptArgument cArgument;
		cArgument.m_pTable = this;
		cArgument.setCandidate(this);

		vecLogData.PUSHBACK(iLogType);
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  getTable()->getSchemaTable()->getID())));
		vecLogData.PUSHBACK(pRowID->generate(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument));

		ModVector<unsigned int> vecColumnID;
		VECTOR<int> vecPrevData;
		Opt::MapContainer(getLogged(), vecColumnID,
						  boost::bind(&Schema::Column::getID,
									  boost::bind(&Scalar::Field::getSchemaColumn,
												  _1)));
		Opt::MapContainer(m_vecPrevLogged, vecPrevData,
						  boost::bind(&Interface::IScalar::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument)));
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecColumnID)));
		vecLogData.PUSHBACK(cProgram_.addVariable(vecPrevData));

		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Logger::create(
											cProgram_,
											pIterator_,
											cProgram_.addVariable(vecLogData),
											iLogType),
								   cArgument.m_eTarget);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Delete::generateLogType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
TableImpl::Delete::
generateLogType(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_)
{
	return cProgram_.addVariable(new Common::IntegerData(getPutInfo().m_cUndoFile.isEmpty()
														 ? Opt::LogData::Type::Delete
														 : Opt::LogData::Type::Delete_Undo));
}
// FUNCTION private
//	Candidate::TableImpl::Delete::isStoringNeeded -- 
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
TableImpl::Delete::
isStoringNeeded(Opt::Environment& cEnvironment_)
{
	// cache result because this method is called twice
	if (m_iStoringNeeded < 0) {
		m_iStoringNeeded =
			cEnvironment_.isSubqueryTable(getTable()->getSchemaTable())
			? 1 : 0;
	}
	return m_iStoringNeeded == 1;
}

// FUNCTION private
//	Candidate::TableImpl::Delete::isCheckColumnConstraint -- 
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
TableImpl::Delete::
isCheckColumnConstraint(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Delete::generate -- 
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
TableImpl::Delete::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	generateThis(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 boost::bind(&Candidate::File::createExpunge,
							 _1,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 _2));
	return pIterator_;
}

// FUNCTION private
//	Candidate::TableImpl::Delete::getOriginalField -- get original value for a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Delete::
getOriginalField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_)
{
	Interface::IScalar* pResult = 0;
	if (pField_->isColumn()) {
		// get schema::column for the field
		Schema::Column* pSchemaColumn = pField_->getSchemaColumn();
		; _SYDNEY_ASSERT(pSchemaColumn);

		// create retrieve fields
		pResult = Scalar::Field::create(cEnvironment_,
										pSchemaColumn,
										m_pRetrieve);
	} else {
		Schema::Field* pSchemaField = pField_->getSchemaField();
		_SYDNEY_ASSERT(pSchemaField);

		if (Schema::Column* pSchemaColumn =
			pSchemaField->getRelatedColumn(cEnvironment_.getTransaction())) {
			pResult = Scalar::Field::create(cEnvironment_,
											pSchemaColumn,
											m_pRetrieve);
		} else {
			pResult = Scalar::Field::create(cEnvironment_,
											pSchemaField,
											pField_->getFile(),
											m_pRetrieve);
		}
	}
	; _SYDNEY_ASSERT(pResult);

	// set field as retrieved
	pResult->retrieve(cEnvironment_,
					  getOperand());

	return pResult;
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Update

// FUNCTION public
//	Candidate::TableImpl::Update::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Update::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// enumerate target files to update
	enumerate(cEnvironment_,
			  cFieldSet_);
}

// FUNCTION private
//	Candidate::TableImpl::Update::enumerate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
enumerate(Opt::Environment& cEnvironment_,
		  const Utility::FieldSet& cFieldSet_)
{
	// set rowid using retrieved rowid
	setRowID(m_pRetrieve->getScalar(cEnvironment_, 0)->getField());

	// list up all the files for each target columns
	createTargetColumn(cEnvironment_,
					   cFieldSet_);

	// add file candidate for each target files
	Opt::ForEach(getTargetColumn(),
				 boost::bind(&This::addPutFile,
							 this,
							 boost::ref(cEnvironment_),
							 _1));
	// add target fields
	Opt::ForEachValue(getPutInfo().m_mapFileCandidate,
					  boost::bind(&This::addPutField,
								  this,
								  boost::ref(cEnvironment_),
								  _1));

	// create pre-updated data for logical log
	createPrevLogged(cEnvironment_);
}

// FUNCTION private
//	Candidate::TableImpl::Update::createPrevLogged -- create previous value for logged fields
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
TableImpl::Update::
createPrevLogged(Opt::Environment& cEnvironment_)
{
	m_vecPrevLogged.reserve(getLogged().GETSIZE());
	Opt::MapContainer(getLogged(),
					  m_vecPrevLogged,
					  boost::bind(&This::getRetrieveField,
								  this,
								  boost::ref(cEnvironment_),
								  _1,
								  true /* consider undo */));
}

// FUNCTION private
//	Candidate::TableImpl::Update::getRetrieveField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	bool bConsiderUndo_ = false
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Update::
getRetrieveField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_,
				 bool bConsiderUndo_ /* = false */)
{
	Interface::IScalar* pResult = 0;

	if (bConsiderUndo_
		&& pField_->isColumn()) {
		; _SYDNEY_ASSERT(pField_->isUpdate());

		// check put file
		Scalar::Field* pCheckedField = pField_->checkPut(cEnvironment_);
		; _SYDNEY_ASSERT(pCheckedField->isChecked());

		const Utility::FileSet& cPutFile = pCheckedField->getChecked()->getFileSet();
		Utility::FileSet::ConstIterator found =
			Opt::Find(cPutFile.begin(), cPutFile.end(),
					   boost::bind(&PutInfo::isUndoFile,
								   &getPutInfo(),
								   _1));
		if (found != cPutFile.end()) {
			// update can be undoed
			// -> use OID field for previous value in log instead
			Schema::Field* pOIDField =
				(*found)->getSchemaFile()->getObjectID(cEnvironment_.getTransaction());
			; _SYDNEY_ASSERT(pOIDField);

			pResult = Scalar::Field::create(cEnvironment_,
											pOIDField,
											*found,
											m_pRetrieve);
			// set field as retrieved
			pResult->retrieve(cEnvironment_,
							  getOperand());
		}
	}
	if (pResult == 0) {
		pResult = getOriginalField(cEnvironment_,
								   pField_);
	}
	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Update::addPutFile -- check put files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
addPutFile(Opt::Environment& cEnvironment_,
		   Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_->isChecked());
	; _SYDNEY_ASSERT(pColumn_->isUpdate());

	const Utility::FileSet& cFileSet = pColumn_->getChecked()->getFileSet();
	cFileSet.foreachElement(boost::bind(&This::addFile,
										this,
										boost::ref(cEnvironment_),
										_1,
										pColumn_));
}

// FUNCTION private
//	Candidate::TableImpl::Update::addFile -- add file candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
addFile(Opt::Environment& cEnvironment_,
		Interface::IFile* pFile_,
		Scalar::Field* pField_)
{
	; _SYDNEY_ASSERT(pField_->isChecked() || pField_->getFile() == pFile_);
	; _SYDNEY_ASSERT(pField_->isUpdate());

	Scalar::Field* pField = pField_->getField(pFile_);
	if (pField == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	; _SYDNEY_ASSERT(pField->isUpdate());

	// get candidate::file from ifile
	Candidate::File* pCandidateFile = getPutInfo().getCandidate(pFile_);
	if (pCandidateFile == 0) {
		// create new candidate::file
		pCandidateFile = pFile_->createCandidate(cEnvironment_,
												 this,
												 File::Parameter::create(cEnvironment_));
		// add to putinfo
		getPutInfo().addCandidate(pCandidateFile);

		if (pField_->isColumn()
			&& pFile_->getSchemaFile()->isAbleToUndo()
			&& !pField->isOperation()) {
			getPutInfo().addUndoFile(pFile_);
		}

		// for put operations, hasalltuples should be checked using schema file
		if (pFile_->getSchemaFile()->isKeyGenerated()
			&& pFile_->getSchemaFile()->getObjectID(cEnvironment_.getTransaction())
			&& pFile_->getSchemaFile()->hasAllTuples() == false) {
			// add OID
			Scalar::Field* pOIDField =
				getUpdateField(cEnvironment_,
							   pCandidateFile,
							   pFile_->getSchemaFile()->getObjectID(
										cEnvironment_.getTransaction()));
			// add to put key
			pCandidateFile->addPutKey(cEnvironment_,
									  pOIDField);
			// add files for OID and destinations
			Scalar::Field* pOIDChecked = pOIDField->checkPut(cEnvironment_);
			pOIDChecked->getChecked()->getFileSet().foreachElement(
								   boost::bind(&This::setDependency,
											   this,
											   boost::ref(cEnvironment_),
											   _1,
											   pFile_));
			addPutFile(cEnvironment_, pOIDChecked);
		}
	}
	; _SYDNEY_ASSERT(pCandidateFile);

	if (pField_->isColumn()
		&& pFile_->getSchemaFile()->isAbleToUndo()
		&& m_cLogRequiredPosition.isEmpty() == false) {
		// erase from undoable files if field is required to be logged
		checkUndo(cEnvironment_,
				  pFile_,
				  pField_,
				  m_cLogRequiredPosition);
	}

	// add target field to file
	pCandidateFile->addField(cEnvironment_,
							 pField);
}

// FUNCTION private
//	Candidate::TableImpl::Update::addPutField -- add put fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
addPutField(Opt::Environment& cEnvironment_,
			Candidate::File* pCandidate_)
{
	// for put operations, hasalltuples should be checked using schema file
	if (pCandidate_->getFile()->getSchemaFile()->hasAllTuples() == false) {
		// add all fields for undo expunge
		const ModVector<Schema::Field*>& vecField =
			pCandidate_->getFile()->getSchemaFile()->getField(cEnvironment_.getTransaction());
		FOREACH(vecField,
				boost::bind(&This::addInsertField,
							this,
							boost::ref(cEnvironment_),
							pCandidate_,
							_1));
	}
	// add putkey fields
	const ModVector<Schema::Field*>& vecPutKey =
		pCandidate_->getFile()->getSchemaFile()->getPutKey(cEnvironment_.getTransaction());
	FOREACH(vecPutKey,
			boost::bind(&Candidate::File::addPutKey,
						pCandidate_,
						boost::ref(cEnvironment_),
						boost::bind(&This::getUpdateField,
									this,
									boost::ref(cEnvironment_),
									pCandidate_,
									_1)));
}

// FUNCTION private
//	Candidate::TableImpl::Update::addInsertField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
addInsertField(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	if (pSchemaField_->isPutable() == false) {
		return;
	}
	if (pCandidate_->isUseUndoUpdate(cEnvironment_)
		&& pSchemaField_->isObjectID() == false) {
		return;
	}

	Scalar::Field* pField = getUpdateField(cEnvironment_,
										   pCandidate_,
										   pSchemaField_);

	; _SYDNEY_ASSERT(pField->isUpdate());

	// add to insert field
	pCandidate_->addInsertField(cEnvironment_,
								pField);

	if (pSchemaField_->isObjectID() == false) {
		// add to undo field
		; _SYDNEY_ASSERT(pField->getUpdate()->getOriginal());
		pCandidate_->addUndoField(cEnvironment_,
								  pField);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Update::getUpdateField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Scalar::Field*
//
// EXCEPTIONS

Scalar::Field*
TableImpl::Update::
getUpdateField(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	Scalar::Field* pField = getTable()->getField(pSchemaField_);
	if (pField == 0) {
		pField = Scalar::Field::create(cEnvironment_,
									   pSchemaField_,
									   pCandidate_->getFile(),
									   getTable());
	}

	if (pField->isUpdate() == false) {
		Interface::IScalar* pInput = 0;
		Interface::IScalar* pOriginal = getRetrieveField(cEnvironment_,
														 pField,
														 pSchemaField_);

		if (pOriginal) {
			pInput =
				pSchemaField_->isObjectID()
				? Scalar::Function::create(cEnvironment_,
										   Tree::Node::Copy,
										   pOriginal,
										   pOriginal->getName())
				: pOriginal;
		} else {
			pInput = Scalar::Value::Null::create(cEnvironment_);
		}
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField,
											 MAKEPAIR(pInput, pOriginal));
	}
	return pField;
}

// FUNCTION private
//	Candidate::TableImpl::Update::setDependency -- set file dependency
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	Interface::IFile* pSourceFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Update::
setDependency(Opt::Environment& cEnvironment_,
			  Interface::IFile* pFile_,
			  Interface::IFile* pSourceFile_)
{
	if (pFile_ != pSourceFile_) {
		// register file->source relationship
		getPutInfo().m_mapDependent[pFile_].PUSHBACK(pSourceFile_);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Update::generateLock -- add lock action
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
TableImpl::Update::
generateLock(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		if (getTable()->getSchemaTable()->isTemporary() == false) {

			if (cEnvironment_.isRecovery() == false) {
				generateObjectLock(cEnvironment_, cProgram_, pIterator_, cArgument_);
			}

			Execution::Action::LockerArgument cLockerArgument;
			cLockerArgument.setTable(getTable()->getSchemaTable());
			cLockerArgument.m_bIsPrepare = cEnvironment_.isPrepare();
			cLockerArgument.m_bIsUpdate = true;
			Execution::Action::LockerArgument cPrevLockerArgument;
			cPrevLockerArgument.setTable(getTable()->getSchemaTable());
			cPrevLockerArgument.m_bIsPrepare = cEnvironment_.isPrepare();
			cPrevLockerArgument.m_bIsUpdate = true;
			if (Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
																 Lock::Name::Category::Tuple,
																 true, /* read only */
																 cEnvironment_.getTransaction().isBatchMode(),
																 cPrevLockerArgument)
				&& Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
																	Lock::Name::Category::Tuple,
																	false, /* readwrite */
																	cEnvironment_.getTransaction().isBatchMode(),
																	cLockerArgument)) {
				Candidate::AdoptArgument cArgument;
				cArgument.m_pTable = this;
				int iDataID = pRowID->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument);
				pIterator_->addCalculation(cProgram_,
										   Execution::Operator::Locker::ConvertTuple::create(
												  cProgram_,
												  pIterator_,
												  cLockerArgument,
												  cPrevLockerArgument,
												  iDataID),
										   cArgument.m_eTarget);
			}
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Update::generateLog -- add log action
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Update::
generateLog(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_)
{
	if (Scalar::Field* pRowID = getRowID()) {
		//////////////////////////////////////////////////
		// Update logical log:
		// {UPDATE, <tableID>, <rowID>, {<columnID>, ...}, {<data>, ...}, {<data>, ...}}
		VECTOR<int> vecLogData;
		vecLogData.reserve(Opt::LogData::Format::DoubleValueNum);

		// logtype is used apart
		int iLogType = generateLogType(cEnvironment_, cProgram_);
		Candidate::AdoptArgument cArgument;
		cArgument.m_pTable = this;
		cArgument.setCandidate(this);

		vecLogData.PUSHBACK(iLogType);
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerData(
													  getTable()->getSchemaTable()->getID())));
		vecLogData.PUSHBACK(pRowID->generate(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument));

		ModVector<unsigned int> vecColumnID;
		VECTOR<int> vecData;
		VECTOR<int> vecPrevData;
		Opt::MapContainer(getLogged(), vecColumnID,
						  boost::bind(&Schema::Column::getID,
									  boost::bind(&Scalar::Field::getSchemaColumn,
												  _1)));
		Opt::MapContainer(m_vecPrevLogged, vecPrevData,
						  boost::bind(&Interface::IScalar::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument)));
		Opt::MapContainer(getLogged(), vecData,
						  boost::bind(&Interface::IScalar::generate,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pIterator_,
									  boost::ref(cArgument)));
		vecLogData.PUSHBACK(cProgram_.addVariable(new Common::UnsignedIntegerArrayData(vecColumnID)));
		vecLogData.PUSHBACK(cProgram_.addVariable(vecPrevData));
		vecLogData.PUSHBACK(cProgram_.addVariable(vecData));

		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Logger::create(
											cProgram_,
											pIterator_,
											cProgram_.addVariable(vecLogData),
											iLogType),
								   cArgument.m_eTarget);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Update::generateLogType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
TableImpl::Update::
generateLogType(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_)
{
	return cProgram_.addVariable(new Common::IntegerData(getPutInfo().m_cUndoFile.isEmpty()
														 ? Opt::LogData::Type::Update
														 : Opt::LogData::Type::Update_Undo));
}
// FUNCTION private
//	Candidate::TableImpl::Update::isStoringNeeded -- 
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
TableImpl::Update::
isStoringNeeded(Opt::Environment& cEnvironment_)
{
	// check whether any updated index file is scanned
	return Opt::IsAnyKey(getPutInfo().m_mapFileCandidate,
						 boost::bind(&Opt::Environment::isIndexScan,
									 &cEnvironment_,
									 boost::bind(&Interface::IFile::getSchemaFile,
												 _1)))
		|| cEnvironment_.isSubqueryTable(getTable()->getSchemaTable());
}

// FUNCTION private
//	Candidate::TableImpl::Update::isCheckColumnConstraint -- 
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
TableImpl::Update::
isCheckColumnConstraint(Opt::Environment& cEnvironment_)
{
	return true;
}

// FUNCTION private
//	Candidate::TableImpl::Update::generate -- 
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
TableImpl::Update::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	generateThis(cEnvironment_,
				 cProgram_,
				 pIterator_,
				 cArgument_,
				 boost::bind(&Candidate::File::createUpdate,
							 _1,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 _2));
	return pIterator_;
}

// FUNCTION private
//	Candidate::TableImpl::Update::getOriginalField -- get original value for a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Update::
getOriginalField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_)
{
	Interface::IScalar* pResult = 0;
	if (pField_->isUpdate()) {
		// use updatefield's original
		pResult = pField_->getUpdate()->getOriginal();
	} else if (pField_->isColumn()) {
		// get schema::column for the field
		Schema::Column* pSchemaColumn = pField_->getSchemaColumn();
		; _SYDNEY_ASSERT(pSchemaColumn);

		// create retrieve fields
		pResult = Scalar::Field::create(cEnvironment_,
										pSchemaColumn,
										m_pRetrieve);
	} else {
		Schema::Field* pSchemaField = pField_->getSchemaField();
		_SYDNEY_ASSERT(pSchemaField);

		if (Schema::Column* pSchemaColumn =
			pSchemaField->getRelatedColumn(cEnvironment_.getTransaction())) {
			pResult = Scalar::Field::create(cEnvironment_,
											pSchemaColumn,
											m_pRetrieve);
		} else {
			if (pSchemaField->isGetable() == false
				&& pSchemaField->getSourceID() != Schema::Object::ID::Invalid) {
				// use source field instead
				pSchemaField = pSchemaField->getSource(cEnvironment_.getTransaction());
			}
			if (pSchemaField->isGetable()) {
				pResult = Scalar::Field::create(cEnvironment_,
												pSchemaField,
												pField_->getFile(),
												m_pRetrieve);
			}
		}
	}

	if (pResult) {
		// set field as retrieved
		pResult->retrieve(cEnvironment_,
						  getOperand());
	}
	return pResult;
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Import

// FUNCTION public
//	Candidate::TableImpl::Import::generateTop -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Interface::IRelation* pRelation_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Import::
generateTop(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Interface::IRelation* pRelation_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	// do nothing
	return pIterator_;
}

// FUNCTION public
//	Candidate::TableImpl::Import::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Import::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// enumerate target files to import
	enumerate(cEnvironment_,
			  cFieldSet_);
}

// FUNCTION private
//	Candidate::TableImpl::Import::enumerate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Import::
enumerate(Opt::Environment& cEnvironment_,
		  const Utility::FieldSet& cFieldSet_)
{
	// create target columns
	createTargetColumn(cEnvironment_,
					   cFieldSet_);

	// add each target files
	cFieldSet_.foreachElement(boost::bind(&This::addPutFile,
										  this,
										  boost::ref(cEnvironment_),
										  _1));
	// add target fields
	Opt::ForEachValue(getPutInfo().m_mapFileCandidate,
					  boost::bind(&This::addPutField,
								  this,
								  boost::ref(cEnvironment_),
								  _1));
}

// FUNCTION private
//	Candidate::TableImpl::Import::addPutFile -- check put files
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Import::
addPutFile(Opt::Environment& cEnvironment_,
		   Scalar::Field* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_->isUpdate());
	; _SYDNEY_ASSERT(!pColumn_->isColumn());

	if (pColumn_->isRowID()) {
		setRowID(pColumn_);
	}

	Interface::IFile* pFile = pColumn_->getFile();
	; _SYDNEY_ASSERT(pFile);

	addFile(cEnvironment_, pFile);
}

// FUNCTION private
//	Candidate::TableImpl::Import::addFile -- add imported file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IFile* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Import::
addFile(Opt::Environment& cEnvironment_,
		Interface::IFile* pFile_)
{
	// get candidate::file from ifile
	Candidate::File* pCandidateFile = getPutInfo().getCandidate(pFile_);
	if (pCandidateFile == 0) {
		// create new candidate::file
		pCandidateFile = pFile_->createCandidate(cEnvironment_,
												 this,
												 File::Parameter::create(cEnvironment_));
		// add to putinfo
		getPutInfo().addCandidate(pCandidateFile);

		if (pFile_->getSchemaFile()->isKeyGenerated()) {
			Schema::Field* pObjectID =
				pFile_->getSchemaFile()->getObjectID(cEnvironment_.getTransaction());
			if (pObjectID && getTable()->getField(pObjectID) == 0) {
				// add OID
				Scalar::Field* pOIDField = 
					Scalar::Field::create(cEnvironment_,
										  pObjectID,
										  pFile_,
										  getTable());
				// OID's source value is itself
				Scalar::Field* pOIDUpdate =
					Scalar::UpdateField::create(cEnvironment_,
												pOIDField,
												pOIDField);

				// add files for OID and destinations
				addPutFile(cEnvironment_, pOIDUpdate->checkPut(cEnvironment_));
			}
		}
	}
	; _SYDNEY_ASSERT(pCandidateFile);
}

// FUNCTION private
//	Candidate::TableImpl::Import::addPutField -- add all imported fields
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Import::
addPutField(Opt::Environment& cEnvironment_,
			Candidate::File* pCandidate_)
{
	const ModVector<Schema::Field*>& vecSchemaField =
		pCandidate_->getFile()->getSchemaFile()->getField(cEnvironment_.getTransaction());

	Opt::ForEach(vecSchemaField,
				 boost::bind(&This::addField,
							 this,
							 boost::ref(cEnvironment_),
							 pCandidate_,
							 _1));
}

// FUNCTION private
//	Candidate::TableImpl::Import::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Import::
addField(Opt::Environment& cEnvironment_,
		 Candidate::File* pCandidate_,
		 Schema::Field* pSchemaField_)
{
	// add only for putable fields 
	if (pSchemaField_->isPutable() == false) return;

	Scalar::Field* pField = getTable()->getField(pSchemaField_);
	if (pField == 0) {
		pField = Scalar::Field::create(cEnvironment_,
									   pSchemaField_,
									   pCandidate_->getFile(),
									   getTable());
	}

	if (pField->isUpdate() == false) {
		Interface::IScalar* pInput = getSourceValue(cEnvironment_,
													pCandidate_,
													pSchemaField_);
		pField = Scalar::UpdateField::create(cEnvironment_,
											 pField,
											 pInput);
	}

	; _SYDNEY_ASSERT(pField->isUpdate());
	pCandidate_->addInsertField(cEnvironment_,
								pField);
	if (pField->isPutKey()) {
		pCandidate_->addPutKey(cEnvironment_,
							   pField);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Import::getSourceValue -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pCandidate_
//	Schema::Field* pSchemaField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
TableImpl::Import::
getSourceValue(Opt::Environment& cEnvironment_,
			   Candidate::File* pCandidate_,
			   Schema::Field* pSchemaField_)
{
	Interface::IScalar* pResult = 0;

	Schema::Field* pSourceSchemaField =
		pSchemaField_->getSource(cEnvironment_.getTransaction());
	if (pSourceSchemaField) {
		Scalar::Field* pSourceField = getTable()->getField(pSourceSchemaField);
		if (pSourceField) {
			; _SYDNEY_ASSERT(pSourceField->isUpdate());
			pResult = pSourceField->getUpdate()->getInput();

			// register file->source relationship
			getPutInfo().m_mapDependent[pCandidate_->getFile()].PUSHBACK(pSourceField->getFile());
		}
	}
	if (pResult == 0) {
		pResult = Scalar::Value::Null::create(cEnvironment_);
	}

	return pResult;
}

// FUNCTION private
//	Candidate::TableImpl::Import::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Import::
getField(Opt::Environment& cEnvironment_,
		 Schema::Column* pSchemaColumn_)
{
	// check target fields
	VECTOR<Scalar::Field*>::CONSTITERATOR iterator = getTargetColumn().begin();
	const VECTOR<Scalar::Field*>::CONSTITERATOR last = getTargetColumn().end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)->getSchemaField()
			&& ((*iterator)->getSchemaField()->getRelatedColumn(cEnvironment_.getTransaction())
				== pSchemaColumn_)) {
			return *iterator;
		}
	}
	return 0;
}

// FUNCTION private
//	Candidate::TableImpl::Import::isStoringNeeded -- 
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
TableImpl::Import::
isStoringNeeded(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Import::isCheckColumnConstraint -- 
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
TableImpl::Import::
isCheckColumnConstraint(Opt::Environment& cEnvironment_)
{
	return true;
}

// FUNCTION private
//	Candidate::TableImpl::Import::generate -- 
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
TableImpl::Import::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	cArgument_.m_bBatch = true;

	// list up file candidates according to the order of import
	VECTOR<Candidate::File*> vecCandidate;
	VECTOR<ParallelSplit> vecSplit;
	FileCandidateMap cMap(getPutInfo().m_mapFileCandidate);

	vecCandidate.reserve(getPutInfo().m_mapFileCandidate.getSize());

	Opt::ForEachKey(getPutInfo().m_mapFileCandidate,
					boost::bind(&This::orderCandidate,
								this,
								_1,
								boost::ref(cMap),
								boost::ref(vecCandidate),
								&vecSplit));

	// generate file
	// import don't use parallel plan
	Opt::ForEach(vecCandidate,
				 boost::bind(&Candidate::File::createInsert,
							 _1,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pIterator_,
							 boost::ref(cArgument_)));

	return pIterator_;
}

// FUNCTION private
//	Candidate::TableImpl::Import::getOriginalField -- get original value for a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Import::
getOriginalField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_)
{
	return Scalar::Value::Null::create(cEnvironment_);
}

//////////////////////////////////////////
// Plan::Candidate::TableImpl::Undo

// FUNCTION public
//	Candidate::TableImpl::Undo::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	const Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Undo::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   const Utility::FieldSet& cFieldSet_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addUndoAction -- add undo action for one entry
//
// NOTES
//
// ARGUMENTS
//	Common::Data::Pointer pData_
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
TableImpl::Undo::
addUndoAction(Common::Data::Pointer pData_,
			  Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_)
{
	const Common::DataArrayData* pUndoLog =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_.get());
	; _SYDNEY_ASSERT(pUndoLog);

	// get undolog type
	int iUndoLogType = pUndoLog->getElement(Opt::UndoLog::Format::UndoLogType)->getInt();

	// get schema file corresponds to the undolog
	Schema::Object::ID::Value iFileID =
		pUndoLog->getElement(Opt::UndoLog::Format::FileID)->getUnsignedInt();
	Schema::File* pSchemaFile = Schema::File::get(iFileID,
												  cEnvironment_.getDatabase(),
												  cEnvironment_.getTransaction());
	if (pSchemaFile == 0
		|| pSchemaFile->getTableID() != getTable()->getSchemaTable()->getID()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// create ifile and candidate::file
	Candidate::File* pFile =
		Interface::IFile::create(cEnvironment_, pSchemaFile)
		->createCandidate(cEnvironment_,
						  this,
						  File::Parameter::create(cEnvironment_));

	switch (iUndoLogType) {
	case Opt::UndoLog::Type::Insert:
		{
			addInsert(cEnvironment_,
					  cProgram_,
					  pIterator_,
					  cArgument_,
					  pUndoLog,
					  pFile);
			break;
		}
	case Opt::UndoLog::Type::Expunge:
		{
			addExpunge(cEnvironment_,
					   cProgram_,
					   pIterator_,
					   cArgument_,
					   pUndoLog,
					   pFile);
			break;
		}
	case Opt::UndoLog::Type::Update:
		{
			addUpdate(cEnvironment_,
					  cProgram_,
					  pIterator_,
					  cArgument_,
					  pUndoLog,
					  pFile);
			break;
		}
	case Opt::UndoLog::Type::UndoExpunge:
		{
			addUndoExpunge(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cArgument_,
						   pUndoLog,
						   pFile);
			break;
		}
	case Opt::UndoLog::Type::UndoUpdate:
		{
			addUndoUpdate(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cArgument_,
						  pUndoLog,
						  pFile);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addInsert -- add undo by insert
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addInsert(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Execution::Interface::IIterator* pIterator_,
		  Candidate::AdoptArgument& cArgument_,
		  const Common::DataArrayData* pUndoLog_,
		  Candidate::File* pFile_)
{
	// add insert field
	addField(cEnvironment_,
			 pUndoLog_,
			 pFile_,
			 Opt::UndoLog::Format::FieldID1,
			 Opt::UndoLog::Format::Data1,
			 true /* for insert */);

	// add insert action
	pFile_->createInsert(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addExpunge -- add undo by expunge
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addExpunge(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Execution::Interface::IIterator* pIterator_,
		   Candidate::AdoptArgument& cArgument_,
		   const Common::DataArrayData* pUndoLog_,
		   Candidate::File* pFile_)
{
	// add put key to file
	addPutKey(cEnvironment_,
			  pUndoLog_,
			  pFile_,
			  Opt::UndoLog::Format::FieldID1,
			  Opt::UndoLog::Format::Data1);

	// add expunge action
	pFile_->createExpunge(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addUpdate -- add undo by update
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addUpdate(Opt::Environment& cEnvironment_,
		  Execution::Interface::IProgram& cProgram_,
		  Execution::Interface::IIterator* pIterator_,
		  Candidate::AdoptArgument& cArgument_,
		  const Common::DataArrayData* pUndoLog_,
		  Candidate::File* pFile_)
{
	// add put key to file
	addPutKey(cEnvironment_,
			  pUndoLog_,
			  pFile_,
			  Opt::UndoLog::Format::FieldID1,
			  Opt::UndoLog::Format::Data1);
	// add updated field to file
	addField(cEnvironment_,
			 pUndoLog_,
			 pFile_,
			 Opt::UndoLog::Format::FieldID2,
			 Opt::UndoLog::Format::Data2,
			 false /* not for insert */);

	// add update action
	pFile_->createUpdate(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addUndoExpunge -- add undo by undoExpunge
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addUndoExpunge(Opt::Environment& cEnvironment_,
			   Execution::Interface::IProgram& cProgram_,
			   Execution::Interface::IIterator* pIterator_,
			   Candidate::AdoptArgument& cArgument_,
			   const Common::DataArrayData* pUndoLog_,
			   Candidate::File* pFile_)
{
	// add put key to file
	addPutKey(cEnvironment_,
			  pUndoLog_,
			  pFile_,
			  Opt::UndoLog::Format::FieldID1,
			  Opt::UndoLog::Format::Data1);
	// add undo expunge action
	pFile_->createUndoExpunge(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addUndoUpdate -- add undo by undoUpdate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addUndoUpdate(Opt::Environment& cEnvironment_,
			  Execution::Interface::IProgram& cProgram_,
			  Execution::Interface::IIterator* pIterator_,
			  Candidate::AdoptArgument& cArgument_,
			  const Common::DataArrayData* pUndoLog_,
			  Candidate::File* pFile_)
{
	// add put key to file
	addPutKey(cEnvironment_,
			  pUndoLog_,
			  pFile_,
			  Opt::UndoLog::Format::FieldID1,
			  Opt::UndoLog::Format::Data1);
	// add undo update action
	pFile_->createUndoUpdate(cEnvironment_,
							 cProgram_,
							 pIterator_,
							 cArgument_);
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addPutKey -- add key field to file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	int iFieldPosition_
//	int iDataPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addPutKey(Opt::Environment& cEnvironment_,
		  const Common::DataArrayData* pUndoLog_,
		  Candidate::File* pFile_,
		  int iFieldPosition_,
		  int iDataPosition_)
{
	const Common::UnsignedIntegerArrayData* pFieldID =
		_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData*,
							 pUndoLog_->getElement(iFieldPosition_).get());
	const Common::DataArrayData* pData =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 pUndoLog_->getElement(iDataPosition_).get());

	if (pFieldID == 0 || pData == 0 || pFieldID->getCount() != pData->getCount()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Schema::File* pSchemaFile = pFile_->getFile()->getSchemaFile();
	; _SYDNEY_ASSERT(pSchemaFile);

	int n = pFieldID->getCount();
	for (int i = 0; i < n; ++i) {
		Schema::Field* pSchemaField =
			pSchemaFile->getFieldByID(pFieldID->getElement(i),
									  cEnvironment_.getTransaction());
		if (pSchemaField == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		Scalar::Field* pField =
			Scalar::UpdateField::create(cEnvironment_,
										Scalar::Field::create(cEnvironment_,
															  pSchemaField,
															  pFile_->getFile(),
															  getTable()),
										Scalar::Value::create(cEnvironment_,
															  pData->getElement(i)));
		pFile_->addPutKey(cEnvironment_,
						  pField);
	}
}

// FUNCTION private
//	Candidate::TableImpl::Undo::addField -- add field to file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Common::DataArrayData* pUndoLog_
//	Candidate::File* pFile_
//	int iFieldPosition_
//	int iDataPosition_
//	bool bForInsert_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Undo::
addField(Opt::Environment& cEnvironment_,
		 const Common::DataArrayData* pUndoLog_,
		 Candidate::File* pFile_,
		 int iFieldPosition_,
		 int iDataPosition_,
		 bool bForInsert_)
{
	const Common::UnsignedIntegerArrayData* pFieldID =
		_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData*,
							 pUndoLog_->getElement(iFieldPosition_).get());
	const Common::DataArrayData* pData =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*,
							 pUndoLog_->getElement(iDataPosition_).get());

	if (pFieldID == 0 || pData == 0 || pFieldID->getCount() != pData->getCount()) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Schema::File* pSchemaFile = pFile_->getFile()->getSchemaFile();
	; _SYDNEY_ASSERT(pSchemaFile);

	ModVector<Schema::Field*> vecSchemaField;
	_FieldElementMap mapFieldElement;
	Opt::ForEach_i(pFieldID->getValue(),
				   boost::bind(&_FieldElementMap::addElement,
							   boost::ref(mapFieldElement),
							   _1, _2));
	if (bForInsert_) {
		vecSchemaField = pSchemaFile->getField(cEnvironment_.getTransaction());
	} else {
		Opt::MapContainer(pFieldID->getValue(), vecSchemaField,
						  boost::bind(&Schema::File::getFieldByID,
									  pSchemaFile,
									  _1,
									  boost::ref(cEnvironment_.getTransaction())));
	}

	ModVector<Schema::Field*>::Iterator iterator = vecSchemaField.begin();
	const ModVector<Schema::Field*>::Iterator last = vecSchemaField.end();
	for (; iterator != last; ++iterator) {
		Schema::Field* pSchemaField = *iterator;
		if (pSchemaField == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		if (pSchemaField->isPutable() == false) {
			continue;
		}

		// target field
		Scalar::Field* pField = Scalar::Field::create(cEnvironment_,
													  pSchemaField,
													  pFile_->getFile(),
													  getTable());
		// update value
		Interface::IScalar* pValue = 0;

		// if source field exists and the value is already assigned, use it
		if (Schema::Field* pSource = pSchemaField->getSource(cEnvironment_.getTransaction())) {
			Scalar::Field* pSourceField = getTable()->getField(pSource);
			if (pSourceField && pSourceField->isUpdate()) {
				pValue = pSourceField->getUpdate()->getInput();
			}
		}

		if (pValue == 0) {
			// get from undolog
			if (mapFieldElement.find(pSchemaField->getID()) != mapFieldElement.end()) {
				Common::Data::Pointer pElement =
					pData->getElement(mapFieldElement[pSchemaField->getID()]);
				if (pFile_->isLocatorUsed() && isOperationLogData(pElement)) {
					// create operation node
					pValue = Scalar::Operation::LogData::create(cEnvironment_,
																pField,
																pElement);
				} else {
					pValue = Scalar::Value::create(cEnvironment_,
												   pElement);
				}
			} else {
				Common::Data::Pointer pData = Scalar::DataType(*pSchemaField).createData();
				pData->setNull();
				pValue = Scalar::Value::create(cEnvironment_,
											   pData);
			}
		}

		if (bForInsert_) {
			Scalar::Field* pUpdateField =
				Scalar::UpdateField::create(cEnvironment_,
											pField,
											pValue);

			pFile_->addInsertField(cEnvironment_,
								   pUpdateField);
			if (pUpdateField->isPutKey()) {
				pFile_->addPutKey(cEnvironment_,
								  pUpdateField);
			}
		} else {
			Scalar::Field* pUpdateField =
				Scalar::UpdateField::create(cEnvironment_,
											pField,
											MAKEPAIR(pValue, (Interface::IScalar*)0));
			pFile_->addField(cEnvironment_,
							 pUpdateField);
		}
	}
}

// FUNCTION private
//	Candidate::TableImpl::Undo::isStoringNeeded -- 
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
TableImpl::Undo::
isStoringNeeded(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Undo::isCheckColumnConstraint -- 
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
TableImpl::Undo::
isCheckColumnConstraint(Opt::Environment& cEnvironment_)
{
	return false;
}

// FUNCTION private
//	Candidate::TableImpl::Undo::generate -- 
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
TableImpl::Undo::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// scan over undologs
	const ModVector<Common::Data::Pointer>& vecData = m_pUndoLog->getValue();
	ModVector<Common::Data::Pointer>::ConstIterator iterator = vecData.begin();
	ModVector<Common::Data::Pointer>::ConstIterator last = vecData.end();

	// first element is logtype of previous operation
	int iLogType = (*iterator)->getInt();
	++iterator;

	if (iLogType == Opt::LogData::Type::Update
		|| iLogType == Opt::LogData::Type::Update_Undo) {
		// scan undolog forward direction
		Opt::ForEach(iterator, last,
					 boost::bind(&This::addUndoAction,
								 this,
								 _1,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_)));
	} else {
		// scan undolog backward direction
		Opt::ForEach_r(iterator, last,
					   boost::bind(&This::addUndoAction,
								   this,
								   _1,
								   boost::ref(cEnvironment_),
								   boost::ref(cProgram_),
								   pIterator_,
								   boost::ref(cArgument_)));
	}
	return pIterator_;
}

// FUNCTION private
//	Candidate::TableImpl::TableImpl::Undo::getOriginalField -- get original value for a column
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
TableImpl::Undo::
getOriginalField(Opt::Environment& cEnvironment_,
				 Scalar::Field* pField_)
{
	// never called
	return 0;
}

// FUNCTION private
//	Candidate::TableImpl::Undo::isOperationLogData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data::Pointer& pElement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
TableImpl::Undo::
isOperationLogData(const Common::Data::Pointer& pElement_)
{
	return pElement_->getType() == Common::DataType::Array
		&& pElement_->getElementType() == Common::DataType::Data;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
