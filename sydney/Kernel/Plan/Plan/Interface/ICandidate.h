// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/ICandidate.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_ICANDIDATE_H
#define __SYDNEY_PLAN_INTERFACE_ICANDIDATE_H

#include "Plan/Interface/Module.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Sql/Query.h"

#include "Common/Object.h"

#include "Execution/Declaration.h"

#include "Opt/Declaration.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

///////////////////////////////////////////////////////////
// CLASS
//	Plan::Interface::ICandidate -- Candidate information for determining access plan
//
// NOTES
//	This class is not constructed directly
//	Implementation of subclasses are defined in Candidate/XXX.cpp
class ICandidate
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef ICandidate This;

	typedef unsigned int InquiryResult;

	// destructor
	static void erase(Opt::Environment& cEnvironment_,
					  This* pThis_);

	// set estimated cost for chosen candidate
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_) = 0;
	virtual const AccessPlan::Cost& getCost() = 0;

	// get result row
	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_) = 0;
	// get key row
	virtual Candidate::Row* getKey(Opt::Environment& cEnvironment_) = 0;

	// add required columns
	virtual void require(Opt::Environment& cEnvironment_,
						 Plan::Scalar::Field* pField_) = 0;
	// add retrieved columns
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Plan::Scalar::Field* pField_) = 0;
	// add used columns
	virtual void use(Opt::Environment& cEnvironment_,
					 Plan::Scalar::Field* pField_) = 0;
	// add retrieved columns as delayable
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Plan::Scalar::Field* pField_,
					   Scalar::DelayArgument& cArgument_) = 0;

	// set used predicate
	virtual void setPredicate(Interface::IPredicate* pPredicate_) = 0;
	// set used order
	virtual void setOrder(Order::Specification* pOrder_) = 0;
	// set used limit
	virtual void setLimit(const AccessPlan::Limit& cLimit_) = 0;

	// check predicate
	virtual void checkPredicate(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_) = 0;

	// check whether limit can be considered in adopting
	virtual bool isLimitAvailable(Opt::Environment& cEnvironment_) = 0;

	// get not checked predicate
	virtual Interface::IPredicate* getNotCheckedPredicate() = 0;
	// get choosed predicate
	virtual Interface::IPredicate* getPredicate() = 0;
	// get choosed order
	virtual Order::Specification* getOrder() = 0;
	// get choosed limit
	virtual const AccessPlan::Limit& getLimit() = 0;

	// inquiry about relation's attributes
	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
								  const Candidate::InquiryArgument& cArgument_) = 0;

	// get refering relations
	virtual bool isReferingRelation(Interface::IRelation* pRelation_) = 0;

	// 子サーバを使用した分散ソートが使用可能か？
	virtual bool isMergeSortAvailable() = 0;
	
	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_) = 0;

	// get corresponding table candidate for a field
	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										   Interface::IRelation* pRelation_) = 0;

	// adopt the candidate and create iterator object
	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Candidate::AdoptArgument& cArgument_) = 0;
	virtual Execution::Interface::IIterator*
					generateTop(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Interface::IRelation* pRelation_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_) = 0;

	// generate additional action to obtain delayed scalars
	virtual void generateDelayed(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_) = 0;

	virtual Scalar::Field* getUsedField(Opt::Environment& cEnvironment_,
										Scalar::Field* pField_) = 0;

	virtual bool isGetByBitSetRowID() = 0;

	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_) = 0;




protected:
	// constructor
	ICandidate()
		: Super(),
		  m_iID(-1)
	{}
	// destructor
	virtual ~ICandidate() {}

	virtual void destruct(Opt::Environment& cEnvironment_) = 0;

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
	// erase from environment
	void eraseFromEnvironment(Opt::Environment& cEnvironment_);

private:
	int m_iID;
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_ICANDIDATE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
