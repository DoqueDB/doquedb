// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Base.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"


#include "Common/Assert.h"

#include "Communication/Connection.h"

#include "Exception/NotSupported.h"

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Connection.h"
#include "Execution/Collection/Variable.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Operator/Locker.h"
#include "Execution/Operator/Output.h"
#include "Execution/Utility/Transaction.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

// FUNCTION public
//	Candidate::Base::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_)
{
	createCost(cEnvironment_,
			   cPlanSource_,
			   m_cCost);
	; _SYDNEY_ASSERT(!m_cCost.isInfinity());
}

// FUNCTION public
//	Candidate::Base::getCost -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const AccessPlan::Cost&
//
// EXCEPTIONS

//virtual
const AccessPlan::Cost&
Base::
getCost()
{
	return m_cCost;
}

// FUNCTION public
//	Candidate::Base::use -- 
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
Base::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	require(cEnvironment_, pField_);
}

// FUNCTION public
//	Candidate::Base::setPredicate -- set used predicate
//
// NOTES
//
// ARGUMENTS
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setPredicate(Interface::IPredicate* pPredicate_)
{
	m_pPredicate = pPredicate_;
}

// FUNCTION public
//	Candidate::Base::setOrder -- set used order
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pOrder_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setOrder(Order::Specification* pOrder_)
{
	m_pOrder = pOrder_;
}

// FUNCTION public
//	Candidate::Base::setLimit -- 
//
// NOTES
//
// ARGUMENTS
//	const AccessPlan::Limit& cLimit_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
setLimit(const AccessPlan::Limit& cLimit_)
{
	m_cLimit = cLimit_;
}

// FUNCTION public
//	Candidate::Base::checkPredicate -- 
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
Base::
checkPredicate(Opt::Environment& cEnvironment_,
			   AccessPlan::Source& cPlanSource_)
{
	if (Interface::IPredicate* pPredicate = cPlanSource_.getPredicate()) {
		Predicate::CheckArgument cCheckArgument(this,
												cPlanSource_.getPrecedingCandidate());
		setPredicate(pPredicate->check(cEnvironment_,
									   cCheckArgument));
		if (getPredicate()) {
			getPredicate()->require(cEnvironment_, this);
		}
	}
}

// FUNCTION public
//	Candidate::Base::isLimitAvailable -- 
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
Base::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	// default: can't use limit as hint
	return false;
}

// FUNCTION public
//	Candidate::Base::getNotCheckedPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Base::
getNotCheckedPredicate()
{
	if (getPredicate()) {
		return getPredicate()->getNotChecked();
	}
	return 0;
}

// FUNCTION public
//	Candidate::Base::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Base::
getPredicate()
{
	return m_pPredicate;
}

// FUNCTION public
//	Candidate::Base::getOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Order::Specification*
//
// EXCEPTIONS

//virtual
Order::Specification*
Base::
getOrder()
{
	return m_pOrder;
}

// FUNCTION public
//	Candidate::Base::getLimit -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const AccessPlan::Limit&
//
// EXCEPTIONS

//virtual
const AccessPlan::Limit&
Base::
getLimit()
{
	return m_cLimit;
}

// FUNCTION public
//	Candidate::Base::isReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isReferingRelation(Interface::IRelation* pRelation_)
{
	return false;
}


// FUNCTION public
//	Candidate::Base::isAbleToDistributeSort
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isMergeSortAvailable()
{
	return true;
}

// FUNCTION public
//	Candidate::Base::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Base::
createReferingRelation(Utility::RelationSet& cRelationSet_)
{
	; // do nothing
}

// FUNCTION public
//	Candidate::Base::getCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//virtual
Candidate::Table*
Base::
getCandidate(Opt::Environment& cEnvironment_,
			 Interface::IRelation* pRelation_)
{
	return 0;
}

// FUNCTION public
//	Candidate::Base::generateTop -- 
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
Base::
generateTop(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Interface::IRelation* pRelation_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	if (pIterator_ == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (getPredicate()
		&& getPredicate()->isChecked() == false
		&& getPredicate()->isChosen() == false) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// result is passed iterator
	Execution::Interface::IIterator* pResult = pIterator_;

	// create delayed retrieving program if needed
	generateDelayed(cEnvironment_,
					cProgram_,
					pResult);

	// insert lock table actions to startup
	generateLockTable(cEnvironment_,
					  cProgram_,
					  pResult);

	cArgument_.setCandidate(this);

	// generate result tuple



		// generate output action to client
		
	if (pRelation_->isOutputToVariable()) {
		generateVariableOutput(cEnvironment_, cProgram_, pRelation_, pResult, cArgument_);
	} else {
		int iDataID = generateRow(cEnvironment_,
								  cProgram_,
								  pResult,
								  cArgument_,
								  pRelation_);
		if (iDataID >= 0) {
			generateClientOutput(cEnvironment_, cProgram_, pRelation_, pResult, iDataID);
		}
	}

	return pResult;
}

// FUNCTION public
//	Candidate::Base::getUsedField -- 
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
Base::
getUsedField(Opt::Environment& cEnvironment_,
			 Scalar::Field* pField_)
{
	return pField_;
}


// FUNCTION public
//	Candidate::Base::isGetByBitSetRowID --
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Base::
isGetByBitSetRowID()
{
	return false;
}


// FUNCTION public
//	Candidate::Base::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Candidate::InquiryArgument& cArgument_
//	
// RETURN
//	Interface::ICandidate::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::ICandidate::InquiryResult
Base::
inquiry(Opt::Environment& cEnvironment_,
		const Candidate::InquiryArgument& cArgument_)
{
	return 0;
}


// FUNCTION public
//	Candidate::Base::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Sql::Query
//
// EXCEPTIONS

//virtual
Sql::Query*
Base::
generateSQL(Opt::Environment& cEnvironment_)
{
	return 0;
}



// FUNCTION public
//	Candidate::Base::addCheckPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
addCheckPredicate(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_,
				  int iID_)
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1_T(Unless,
											  iID_,
											  cArgument_.m_eTarget));
	generateContinue(cEnvironment_,
					 cProgram_,
					 pIterator_,
					 cArgument_);

	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(EndIf,
											  cArgument_.m_eTarget));
}


// FUNCTION public
//	Candidate::Base::generateCheckPredicate -- generate check predicate action
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
//	Nothing
//
// EXCEPTIONS

void
Base::
generateCheckPredicate(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_,
					   Interface::IPredicate* pPredicate_)
{
	int iID = createCheckPredicate(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   pPredicate_);
	if (iID >= 0) {
		addCheckPredicate(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cArgument_,
						  iID);
	}
}

// FUNCTION protected
//	Candidate::Base::generateLockTable -- insert lock table actions to startup
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

void
Base::
generateLockTable(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_)
{
	const VECTOR<Schema::Table*>& vecSchemaTable = cEnvironment_.getReferedTable();
	FOREACH_r(vecSchemaTable,
			  boost::bind(&This::addLockTable,
						  this,
						  boost::ref(cEnvironment_),
						  boost::ref(cProgram_),
						  pIterator_,
						  _1));
}

// FUNCTION protected
//	Candidate::Base::generateClientOutput -- add output to client
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Interface::IRelation* pRelation_
//	Execution::Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
generateClientOutput(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Interface::IRelation* pRelation_,
					 Execution::Interface::IIterator* pIterator_,
					 int iDataID_)
{
	// prepare the place on which output connection object will be set
	int iConnectionID =
		cProgram_.prepareOutputConnection();

	// create new Collection output to the connection
	Execution::Interface::ICollection* pCollection =
		Execution::Collection::Connection::create(cProgram_, iConnectionID);

	if (cEnvironment_.getProtocolVersion() >= 1) {
		// create column meta data using RowInfo of the top relation.
		// * column meta data is supported from v15 (= masterID:1)
		pIterator_->setMetaData(cProgram_,
								pCollection->getID(),
								pRelation_->getRowInfo(cEnvironment_)->createMetaData(cEnvironment_));
	}

	// generate Common::Data to which result tuple will be stored.
	// set data ID to the main iterator for output data
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT(Output,
										   pCollection->getID(),
										   iDataID_));
}


// FUNCTION protected
//	Candidate::Base::generateClientOutput -- add output to client
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Interface::IRelation* pRelation_
//	Execution::Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
generateVariableOutput(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Interface::IRelation* pRelation_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_)
{
	Plan::Utility::RelationSet cRelationSet;
	pRelation_->getUsedTable(cEnvironment_, cRelationSet);
	if (cRelationSet.getSize() != 1
		|| (*cRelationSet.begin())->getType() != Tree::Node::Table ) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Relation::Table* pTable = _SYDNEY_DYNAMIC_CAST(Relation::Table*, *cRelationSet.begin());
	Schema::Table* pSchemaTable = pTable->getSchemaTable();



	// create new Collection output to the connection
	Execution::Interface::ICollection* pCollection =
		Execution::Collection::Variable::create(cProgram_,
												cEnvironment_.getTransaction().getSessionID(),
												pRelation_->getOutputVariableName(),
												pSchemaTable->getID());
	

	int iDataID = -1;
	Interface::IScalar* pScalar = pRelation_->getScalar(cEnvironment_, 0);
	if (pScalar->isField()) {
		Scalar::Field* pField = _SYDNEY_DYNAMIC_CAST(Scalar::Field*, pScalar);
		if (pField->isRowID()) {
			iDataID = pField->generate(cEnvironment_,  cProgram_, pIterator_, cArgument_);
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	if (iDataID == -1) _SYDNEY_THROW0(Exception::NotSupported);
			
	// generate Common::Data to which result tuple will be stored.
	// set data ID to the main iterator for output data
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT(Output,
										   pCollection->getID(),
										   iDataID));
}

// FUNCTION protected
//	Candidate::Base::generateRow -- generate result row
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Interface::IRelation* pRelation_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Base::
generateRow(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_,
			Interface::IRelation* pRelation_)
{
	return pRelation_->getRowInfo(cEnvironment_)->generate(cEnvironment_,
														   cProgram_,
														   pIterator_,
														   cArgument_);
}

// FUNCTION protected
//	Candidate::Base::setLockMode -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Schema::Table* pSchemaTable_
//	Lock::Name::Category::Value eTarget_
//	Lock::Name::Category::Value eManipulation_
//	Execution::Action::LockerArgument& cLockerArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Base::
setLockMode(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Schema::Table* pSchemaTable_,
			Lock::Name::Category::Value eTarget_,
			Lock::Name::Category::Value eManipulation_,
			Execution::Action::LockerArgument& cLockerArgument_)
{
	cLockerArgument_.setTable(pSchemaTable_);
	cLockerArgument_.m_bIsPrepare = cEnvironment_.isPrepare();
	cLockerArgument_.m_bIsUpdate = cEnvironment_.isUpdateTable(pSchemaTable_);
	cLockerArgument_.m_bIsSimple = cEnvironment_.isSimpleTable(pSchemaTable_);
	bool bReadOnly = !cProgram_.isUpdate()
		|| (!cEnvironment_.isUpdateTable(pSchemaTable_)
			&& !cEnvironment_.isInsertTable(pSchemaTable_));
	return Execution::Utility::Transaction::getAdequateLock(cEnvironment_.getTransaction(),
															eTarget_,
															eManipulation_,
															bReadOnly,
															cEnvironment_.getTransaction().isBatchMode(),
															cLockerArgument_);
}

// FUNCTION protected
//	Candidate::Base::destruct -- destruct sub
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
Base::
destruct(Opt::Environment& cEnvironment_)
{
	Candidate::Row::erase(cEnvironment_, m_pRow);
	m_pRow = 0;
}

// FUNCTION private
//	Candidate::Base::createLockTable -- create lock table action to startup for one table
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Schema::Table* pSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Base::
addLockTable(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Schema::Table* pSchemaTable_)
{
	// create lock only when schema table is not temporary table
	if (pSchemaTable_->isTemporary() == false
		&& pSchemaTable_->isVirtual() == false) {
		Execution::Action::LockerArgument cArgument;
		if (setLockMode(cEnvironment_,
						cProgram_,
						pSchemaTable_,
						Lock::Name::Category::Table,
						Lock::Name::Category::Tuple,
						cArgument)) {
			Execution::Operator::Locker* pLock =
				Execution::Operator::Locker::Table::create(cProgram_,
														   pIterator_,
														   cArgument);
			// insert lock action to head of startup action
			pIterator_->insertStartUp(cProgram_,
									  _ACTION_ARGUMENT1(Calculation,
														pLock->getID()));
		}
	}
}

// FUNCTION private
//	Candidate::Base::createCheckPredicate -- create check predicate action
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
Base::
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
//	Candidate::Base::generateContinue -- 
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
Base::
generateContinue(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(Continue,
											  cArgument_.m_eTarget));
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
