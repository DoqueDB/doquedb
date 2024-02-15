// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Base.h --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_BASE_H
#define __SYDNEY_PLAN_CANDIDATE_BASE_H

#include "Plan/Candidate/Module.h"

#include "Plan/AccessPlan/Limit.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Utility/ObjectSet.h"

#include "Lock/Name.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Base -- base class of ICandidate implementation classes
//
// NOTES
class Base
	: public Interface::ICandidate
{
public:
	typedef Base This;
	typedef Interface::ICandidate Super;

/////////////////////////////
// Interface::ICandidate::
 	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_);
 	virtual const AccessPlan::Cost& getCost();

	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_)
	{return (m_pRow == 0) ? (m_pRow = createRow(cEnvironment_)) : m_pRow;}
	virtual Candidate::Row* getKey(Opt::Environment& cEnvironment_)
	{return (m_pKey == 0) ? (m_pKey = createKey(cEnvironment_)) : m_pKey;}

// 	virtual void require(Opt::Environment& cEnvironment_,
//						 Scalar::Field* pField_);
// 	virtual void retrieve(Opt::Environment& cEnvironment_,
//						  Scalar::Field* pField_);
 	virtual void use(Opt::Environment& cEnvironment_,
					 Scalar::Field* pField_);
// 	virtual bool delay(Opt::Environment& cEnvironment_,
// 					   Scalar::Field* pField_,
// 					   Scalar::DelayArgument& cArgument_);
 	virtual void setPredicate(Interface::IPredicate* pPredicate_);
 	virtual void setOrder(Order::Specification* pOrder_);
	virtual void setLimit(const AccessPlan::Limit& cLimit_);

	virtual void checkPredicate(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_);
	virtual bool isLimitAvailable(Opt::Environment& cEnvironment_);

	virtual Interface::IPredicate* getNotCheckedPredicate();
 	virtual Interface::IPredicate* getPredicate();
 	virtual Order::Specification* getOrder();
	virtual const AccessPlan::Limit& getLimit();

 	virtual bool isReferingRelation(Interface::IRelation* pRelation_);
 	virtual bool isMergeSortAvailable();
 	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										   Interface::IRelation* pRelation_);

//
// 	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
//												   Execution::Interface::IProgram& cProgram_,
//												   Candidate::AdoptArgument& cArgument_);
	virtual Execution::Interface::IIterator*
					generateTop(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Interface::IRelation* pRelation_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
// 	virtual void generateDelayed(Opt::Environment& cEnvironment_,
// 								 Execution::Interface::IProgram& cProgram_,
// 								 Execution::Interface::IIterator* pIterator_);
	virtual Scalar::Field* getUsedField(Opt::Environment& cEnvironment_,
										Scalar::Field* pField_);
	virtual bool isGetByBitSetRowID();
	
	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
								  const Candidate::InquiryArgument& cArgument_);

	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

/////////////////////////////

	// add check predicate action
	void addCheckPredicate(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_,
						   Candidate::AdoptArgument& cArgument_,
						   int iID_);
protected:
	// constructor
	Base()
		: Super(),
		  m_cCost(),
		  m_pRow(0),
		  m_pKey(0),
		  m_pPredicate(0),
		  m_pOrder(0),
		  m_cLimit()
	{}

	// destructor
	virtual ~Base() {}

	// insert lock table actions to startup
	void generateLockTable(Opt::Environment& cEnvironment_,
						   Execution::Interface::IProgram& cProgram_,
						   Execution::Interface::IIterator* pIterator_);

	// generate output to client
	void generateClientOutput(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Interface::IRelation* pRelation_,
							  Execution::Interface::IIterator* pIterator_,
							  int iDataID_);

	void generateVariableOutput(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Interface::IRelation* pRelation_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		
	// generate check predicate action
	void generateCheckPredicate(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_,
								Interface::IPredicate* pPredicate_);

	// generate result row
	int generateRow(Opt::Environment& cEnvironment_,
					Execution::Interface::IProgram& cProgram_,
					Execution::Interface::IIterator* pIterator_,
					Candidate::AdoptArgument& cArgument_,
					Interface::IRelation* pRelation_);

	// set table lock mode
	bool setLockMode(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Schema::Table* pSchemaTable_,
					 Lock::Name::Category::Value eTarget_,
					 Lock::Name::Category::Value eManipulation_,
					 Execution::Action::LockerArgument& cLockerArgument_);

////////////////////////////
// Interface::ICandidate::
	virtual void destruct(Opt::Environment& cEnvironment_);

private:
	// create lock table action to startup for one table
	void addLockTable(Opt::Environment& cEnvironment_,
					  Execution::Interface::IProgram& cProgram_,
					  Execution::Interface::IIterator* pIterator_,
					  Schema::Table* pSchemaTable_);

	// create cost
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_,
							AccessPlan::Cost& cCost_) = 0;
	// create result row
	virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_) = 0;
	// create key row
	virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_) = 0;
	// create check predicate action
	virtual int createCheckPredicate(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 Interface::IPredicate* pPredicate_);
	// add continue action when predicate doesn't hold
	virtual void generateContinue(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  Candidate::AdoptArgument& cArgument_);

	AccessPlan::Cost m_cCost;
	Candidate::Row* m_pRow;
	Candidate::Row* m_pKey;

	Interface::IPredicate* m_pPredicate;
	Order::Specification* m_pOrder;
	AccessPlan::Limit m_cLimit;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_BASE_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
