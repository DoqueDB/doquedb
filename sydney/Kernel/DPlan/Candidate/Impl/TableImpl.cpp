// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Table.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DPlan/Candidate/Impl/TableImpl.h"
#include "DPlan/Candidate/Table.h"

#include "DPlan/Candidate/Grouping.h"
#include "DPlan/Candidate/WordExtraction.h"
#include "DPlan/Relation/Table.h"

#include "DExecution/Iterator/Server.h"
#include "DExecution/Operator/ServerOperation.h"
#include "DExecution/Predicate/IsValid.h"

#include "Plan/AccessPlan/Source.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Candidate/Sort.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/Key.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Sql/Query.h"
#include "Plan/Sql/Argument.h"


#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Collection/Queue.h"
#include "Execution/Collection/Store.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Iterator/Loop.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Output.h"
#include "Execution/Parallel/Thread.h"

#include "Opt/Environment.h"

#include "Os/AutoCriticalSection.h"

#include "Schema/Cascade.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

namespace
{
	Os::CriticalSection _latch;
	int _iTurn = 0;

	int _getRoundRobin(int iSize_)
	{
		Os::AutoCriticalSection l(_latch);
		return _iTurn = (_iTurn + 1) % iSize_;
	}
}

//////////////////////////////////////////////
// Candidate::TableImpl::Base

// FUNCTION public
//	Candidate::TableImpl::Base::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::AccessPlan::Source& cPlanSource_
//	const Plan::Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Base::
createPlan(Opt::Environment& cEnvironment_,
		   Plan::AccessPlan::Source& cPlanSource_,
		   const Plan::Utility::FieldSet& cFieldSet_)
{
	if (cPlanSource_.getOrder()) {
		cPlanSource_.getOrder()->require(cEnvironment_, this);
		if (!cPlanSource_.getOrder()->hasExpandElement())
			setOrder(cPlanSource_.getOrder());
	}
		
	m_cFieldSet.merge(cFieldSet_);
	if (cPlanSource_.getPredicate()) {
		setPredicate(cPlanSource_.getPredicate());
	}
	
	
	return this;
}

// FUNCTION public
//	Candidate::TableImpl::Base::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
require(Opt::Environment& cEnvironment_,
		Plan::Scalar::Field* pField_)
{
	if (!m_cFieldSet.isContaining(pField_)) {
		m_cFieldSet.add(pField_);
	}
}

// FUNCTION public
//	Candidate::TableImpl::Base::base -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
retrieve(Opt::Environment& cEnvironment_,
		 Plan::Scalar::Field* pField_)
{
	require(cEnvironment_,
			pField_);
}

// FUNCTION public
//	Candidate::TableImpl::Base::delay -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Scalar::Field* pField_
//	Plan::Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Base::
delay(Opt::Environment& cEnvironment_,
	  Plan::Scalar::Field* pField_,
	  Plan::Scalar::DelayArgument& cArgument_)
{
	return false;
}

// FUNCTION public
//	Candidate::TableImpl::Base::isReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
TableImpl::Base::
isReferingRelation(Plan::Interface::IRelation* pRelation_)
{
	return pRelation_ == m_pRelation;
}

// FUNCTION public
//	Candidate::TableImpl::Base::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Plan::Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
createReferingRelation(Plan::Utility::RelationSet& cRelationSet_)
{
	cRelationSet_.add(m_pRelation);
}

// FUNCTION public
//	Candidate::TableImpl::Base::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Base::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	if (!cArgument_.m_pQuery) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Plan::Candidate::AdoptArgument::Target::StartUp;
	
	return cMyArgument.m_pQuery->adoptQuery(cEnvironment_,
											cProgram_,
											cMyArgument,
											true);	
}

// FUNCTION public
//	Candidate::TableImpl::Base::generateDelayed -- 
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
TableImpl::Base::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	; // do nothing
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
Plan::Interface::ICandidate::InquiryResult
TableImpl::Base::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Candidate::InquiryArgument& cArgument_)
{
	Plan::Interface::ICandidate::InquiryResult iResult = Super::inquiry(cEnvironment_, cArgument_);

	if (cArgument_.m_iTarget & Plan::Candidate::InquiryArgument::Target::Distributed) {
		Plan::Relation::InquiryArgument cArgument(Plan::Relation::InquiryArgument::Target::Distributed);
		Plan::Interface::IRelation::InquiryResult iRelationResult = getRelation()->inquiry(cEnvironment_, cArgument);
		if (iRelationResult &
			Plan::Relation::InquiryArgument::Target::Distributed) {
			iResult |= Plan::Candidate::InquiryArgument::Target::Distributed;
		}
	}
	return iResult;
}



// FUNCTION protected
//	Candidate::TableImpl::Base::addIterator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	int iQueueID_
//	Schema::Cascade* pCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Base::
addIterator(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Plan::Candidate::AdoptArgument& cArgument_,
			int iQueueID_,
			Schema::Cascade* pCascade_)

{
	if (cArgument_.m_eTarget == Plan::Candidate::AdoptArgument::Target::Parallel) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(ParallelList,
												  cArgument_.m_eTarget));
	}

	// create server input action
	Plan::Sql::QueryArgument cQueryArg;
	Execution::Interface::IIterator* pInput = DExecution::Iterator::Server::Query::create(cProgram_,
														 pCascade_,
														 cArgument_.m_pQuery->toSQLStatement(cEnvironment_, cQueryArg));
	
	int iDataID = cArgument_.m_pQuery->createOutput(cEnvironment_,
													cProgram_,
													pInput,
													cArgument_);
	pInput->addAction(cProgram_,
					  _ACTION_ARGUMENT1(OutData,
										iDataID));
	pInput->addCalculation(cProgram_,
						   Execution::Operator::Output::create(cProgram_,
															   pInput,
															   iQueueID_,
															   iDataID));
	// add loop action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Iterate::All::create(cProgram_,
																		 pIterator_,
																		 pInput->getID(),
																		 true/* no undone */),
							   cArgument_.m_eTarget);
}

// FUNCTION protected
//	Candidate::TableImpl::Base::addOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	Schema::Cascade* pCascade_
//	const STRING& cstrSQL_
//	int iCheckID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TableImpl::Base::
addOperation(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 int iDataID_,
			 Schema::Cascade* pCascade_,
			 const STRING& cstrSQL_,
			 int iCheckID_)
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(ParallelList,
											  cArgument_.m_eTarget));

	if (iCheckID_ >= 0) {
		Execution::Interface::IAction* pIsValid =
			DExecution::Predicate::IsValid::create(cProgram_,
												   pIterator_,
												   pCascade_,
												   iCheckID_);
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(If,
												  pIsValid->getID(),
												  cArgument_.m_eTarget));
	}

	// create server input action
	Execution::Interface::IAction* pOperation =
		DExecution::Operator::ServerOperation::create(cProgram_,
													  pIterator_,
													  pCascade_,
													  cstrSQL_,
													  iDataID_);

	pIterator_->addCalculation(cProgram_,
							   pOperation,
							   cArgument_.m_eTarget);

	if (iCheckID_ >= 0) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
}

// FUNCTION protected
//	Candidate::TableImpl::Base::createOutput -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
TableImpl::Base::
createOutput(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecData;
	m_cFieldSet.mapElement(vecData,
						   boost::bind(&Plan::Interface::IScalar::generate,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cProgram_),
									   pIterator_,
									   boost::ref(cArgument_)));
	return cProgram_.addVariable(vecData);
}


//	Candidate::TableImpl::Base::createOutput -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
TableImpl::Base::
createOutputFromType(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Plan::Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecData;
	m_cFieldSet.mapElement(vecData,
						   boost::bind(&Plan::Interface::IScalar::generateFromType,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cProgram_),
									   pIterator_,
									   boost::ref(cArgument_)));
	return cProgram_.addVariable(vecData);
}


// FUNCTION private
//	Candidate::Table::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::AccessPlan::Source& cDPlanSource_
//	Plan::AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
TableImpl::Base::
createCost(Opt::Environment& cEnvironment_,
		   const Plan::AccessPlan::Source& cDPlanSource_,
		   Plan::AccessPlan::Cost& cCost_)
{
	// YET
}

// FUNCTION private
//	Candidate::TableImpl::Base::createRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Candidate::Row*
//
// EXCEPTIONS

//virtual

Plan::Candidate::Row*
TableImpl::Base::
createRow(Opt::Environment& cEnvironment_)
{
	Plan::Candidate::Row* pRow = Plan::Candidate::Row::create(cEnvironment_);
	m_cFieldSet.foreachElement(boost::bind(&Plan::Candidate::Row::addScalar,
										   pRow,
										   _1));
	return pRow;
}


// FUNCTION private
//	Candidate::TableImpl::Base::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Plan::Candidate::Row*
//
// EXCEPTIONS

//virtual
Plan::Candidate::Row*
TableImpl::Base::
createKey(Opt::Environment& cEnvironment_)
{
	return Plan::Candidate::Row::create(cEnvironment_);
}

///////////////////////////////////////////
// Candidate::TableImpl::RetrieveBase::


// FUNCTION public
//	Candidate::TableImpl::RetrieveBase::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

Plan::Sql::Query*
TableImpl::RetrieveBase::
generateSQL(Opt::Environment& cEnvironment_)
{

	Plan::Sql::Query* pResult = getRelation()->generateSQL(cEnvironment_);
	if (getPredicate()) {
		getPredicate()->retrieveFromCascade(cEnvironment_, pResult);
	}
	if (getOrder()) {
		pResult->setOrderBy(getOrder());
	}
	
	return pResult;
}



///////////////////////////////////////////
// Candidate::TableImpl::InsertBase::


// FUNCTION public
//	Candidate::TableImpl::InsertBase::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

Plan::Sql::Query*
TableImpl::InsertBase::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getRelation()->generateSQL(cEnvironment_);
	getFieldSet().foreachElement(boost::bind(&Plan::Sql::Query::addUpdateColumn,
											 pResult,
											 _1));
	return pResult;
}


// FUNCTION public
//	Candidate::TableImpl::InsertBase::adoptOpeand --
//
// NOTES
//	operandをadoptして、全ての入力データをCollection貯めるIteratorを返す
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::InsertBase::
adoptOperand(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pQuery = 
		getOperand()->generateSQL(cEnvironment_);
	Execution::Interface::IIterator* pOperand =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);

	int iOperandDataID = createOutput(cEnvironment_,
									  cProgram_,
									  pOperand,
									  cMyArgument);

		
		// generateしたoperand を一度すべてCollectionに貯める
	Execution::Interface::ICollection* pCollection =
		Execution::Collection::Store::create(cProgram_);
	pOperand->addCalculation(cProgram_,
							 Execution::Operator::Output::create(cProgram_,
																 pOperand,
																 pCollection->getID(),
																 iOperandDataID));
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Input::create(cProgram_);
	pResult->addCalculation(cProgram_,
							Execution::Operator::Iterate::All::create(cProgram_,
																	  pResult,
																	  pOperand->getID(),
																	  true),
							Plan::Candidate::AdoptArgument::Target::StartUp);
	int iDataID = createOutputFromType(cEnvironment_,
									   cProgram_,
									   pResult,
									   cArgument_);	
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT2(Input,
										 pCollection->getID(),
										 iDataID));
	if (m_bRelocateUpdate) { // relocate insert の場合
		Plan::Sql::Query* pQuery =
			Plan::Sql::Query::create(cEnvironment_,
									 Plan::Sql::Query::DEL,
									 cMyArgument.m_pQuery->getTable(),
									 true);
		pQuery->setPredicate(cMyArgument.m_pQuery->getPredicate());
		getRelation()->getScalar(cEnvironment_, 0)->
			retrieveFromCascade(cEnvironment_, pQuery);
		
		Execution::Interface::IIterator* pInput =
			Execution::Iterator::Input::create(cProgram_);
		cMyArgument.m_eTarget = Plan::Candidate::AdoptArgument::Target::StartUp;
		Execution::Interface::ICollection* pQueue =
			pQuery->getQueue(cEnvironment_, cProgram_, pInput, cMyArgument, true);

		int iDelID = pQuery->createOutput(cEnvironment_,
										  cProgram_,
										  pInput,
										  cMyArgument);

		pInput->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pQueue->getID(),
											iDelID));
		pResult->addCalculation(cProgram_,
								Execution::Operator::Iterate::All::create(cProgram_,
																		  pResult,
																		  pInput->getID(),
																		  true));
	}


	return pResult;
	
}

//////////////////////////////////////////////////////
// Candidate::TableImpl::Replicate::Insert::

// FUNCTION public
//	Candidate::TableImpl::InsertBase::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

Plan::Sql::Query*
TableImpl::UpdateBase::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getRelation()->generateSQL(cEnvironment_);
	if (getPredicate()) {
		getPredicate()->retrieveFromCascade(cEnvironment_, pResult);
	}


	getFieldSet().foreachElement(boost::bind(&Plan::Sql::Query::addUpdateColumn,
											 pResult,
											 _1));
	return pResult;
}



///////////////////////////////////////////
// Candidate::TableImpl::Delete::

// FUNCTION public
//	Candidate::TableImpl::Delete::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

Plan::Sql::Query*
TableImpl::Delete::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getRelation()->generateSQL(cEnvironment_);
	if (getPredicate()) {
		getPredicate()->retrieveFromCascade(cEnvironment_, pResult);
	}
	
	return pResult;
}

//////////////////////////////////////////////////////
// Candidate::TableImpl::Distribute::Retrieve::

// FUNCTION public
//	Candidate::TableImpl::Distribute::Retrieve::createPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::AccessPlan::Source& cPlanSource_
//	const Plan::Utility::FieldSet& cFieldSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
TableImpl::Distribute::Retrieve::
createPlan(Opt::Environment& cEnvironment_,
		   Plan::AccessPlan::Source& cPlanSource_,
		   const Plan::Utility::FieldSet& cFieldSet_)
{
	Plan::Interface::ICandidate* pResult =
		Super::createPlan(cEnvironment_, cPlanSource_, cFieldSet_);

	if (cPlanSource_.getPredicate()) {
		pResult =  cPlanSource_.getPredicate()->createDistributePlan(cEnvironment_, this, m_cFieldSet);
	}
	return pResult;
}



//////////////////////////////////////////////////////
// Candidate::TableImpl::Distribute::Insert::

// FUNCTION public
//	Candidate::TableImpl::Distribute::Insert::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Distribute::Insert::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Execution::Interface::IIterator* pResult =
		adoptOperand(cEnvironment_, cProgram_, cArgument_);
	int iDataID = createOutput(cEnvironment_,
							   cProgram_,
							   pResult,
							   cArgument_);

	
	// generate check result
	int iCheckID = m_pCheck->generate(cEnvironment_,
									  cProgram_,
									  pResult,
									  cArgument_);

	ModVector<Schema::Cascade*> vecSchemaCascade =
		cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());
	; _SYDNEY_ASSERT(vecSchemaCascade.isEmpty() == false);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));

	// create SQL statement to pass to each cascade server
	Plan::Sql::QueryArgument cQueryArg;
	STRING cstrSQL = cArgument_.m_pQuery->toSQLStatement(cEnvironment_, cQueryArg);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1_T(BeginParallel,
										   Execution::Parallel::Thread::create(cProgram_, pResult)->getID(),
										   cArgument_.m_eTarget));

	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Plan::Candidate::AdoptArgument::Target::Parallel;

	// create input for each cascade server
	Opt::ForEach(vecSchemaCascade,
				 boost::bind(&This::addOperation,
							 this,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pResult,
							 boost::ref(cMyArgument),
							 iDataID,
							 _1,
							 boost::cref(cstrSQL),
							 iCheckID));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(EndParallel,
										   cArgument_.m_eTarget));
	return pResult;
}

//////////////////////////////////////////////////////
// Candidate::TableImpl::Replicate::Retrieve::

// FUNCTION public
//	Candidate::TableImpl::Replicate::Retrieve::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Replicate::Retrieve::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bReplicate = true;
	// get all the cascade for the database
	ModVector<Schema::Cascade*> vecSchemaCascade =
		cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());
	; _SYDNEY_ASSERT(vecSchemaCascade.isEmpty() == false);
		// get next server
	int iIndex = _getRoundRobin(vecSchemaCascade.GETSIZE());
	cMyArgument.m_pCascade = vecSchemaCascade[iIndex];
	Execution::Interface::IIterator* pReuslt =
		Super::adopt(cEnvironment_, cProgram_, cMyArgument);
	
	cMyArgument.m_pQuery->addSqlGenerator(cEnvironment_,
										  cProgram_,
										  pReuslt,
										  cMyArgument.m_bConcatinateIterable);
	return pReuslt;
}


//////////////////////////////////////////////////////
// Candidate::TableImpl::Replicate::Insert::

// FUNCTION public
//	Candidate::TableImpl::Replicate::Insert::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
TableImpl::Replicate::Insert::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	if (!cArgument_.m_pQuery) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Execution::Interface::IIterator* pResult =
		adoptOperand(cEnvironment_, cProgram_, cArgument_);
	int iDataID = createOutput(cEnvironment_,
							   cProgram_,
							   pResult,
							   cArgument_);

	// get all the cascade for the database
	ModVector<Schema::Cascade*> vecSchemaCascade =
		cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());
	; _SYDNEY_ASSERT(vecSchemaCascade.isEmpty() == false);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));

	// create SQL statement to pass to each cascade server
	Plan::Sql::QueryArgument cQueryArg;
	STRING cstrSQL = cArgument_.m_pQuery->toSQLStatement(cEnvironment_, cQueryArg);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1_T(BeginParallel,
										   Execution::Parallel::Thread::create(cProgram_, pResult)->getID(),
										   cArgument_.m_eTarget));

	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Plan::Candidate::AdoptArgument::Target::Parallel;

	// create input for each cascade server
	Opt::ForEach(vecSchemaCascade,
				 boost::bind(&This::addOperation,
							 this,
							 boost::ref(cEnvironment_),
							 boost::ref(cProgram_),
							 pResult,
							 boost::ref(cMyArgument),
							 iDataID,
							 _1,
							 boost::cref(cstrSQL),
							 -1));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(EndParallel,
										   cArgument_.m_eTarget));
	return pResult;
}

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
