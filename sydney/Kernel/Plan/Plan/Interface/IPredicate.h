// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IPredicate.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_IPREDICATE_H
#define __SYDNEY_PLAN_INTERFACE_IPREDICATE_H

#include "Plan/Interface/Module.h"
#include "Plan/Interface/IScalar.h"

#include "Plan/AccessPlan/Cost.h" 
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Declaration.h"
#include "Plan/Sql/Query.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Interface::IPredicate -- Base class for the classes which represents predicate data information
//
//	NOTES
//		This class is not constructed directly
class IPredicate
	: public IScalar
{
public:
	typedef IScalar Super;
	typedef IPredicate This;

	// CLASS
	//	Plan::Interface::IPredicate::RewriteResult --
	//
	// NOTES
	class RewriteResult
		: public PAIR<IRelation*, IPredicate*>
	{
	public:
		typedef PAIR<IRelation*, IPredicate*> Super;
		typedef RewriteResult This;

		RewriteResult() : Super() {}
		RewriteResult(IRelation* pRelation_, IPredicate* pPredicate_)
			: Super(pRelation_, pPredicate_)
		{}

		IRelation* getRelation() {return first;}
		IPredicate* getPredicate() {return second;}

		void setRelation(IRelation* pRelation_) {first = pRelation_;}
		void setPredicate(IPredicate* pPredicate_) {second = pPredicate_;}

	private:
	};

	// destructor
	virtual ~IPredicate() {}

	// try to eliminate NOT if the predicate P is used in the form of NOT(P)
	virtual IPredicate* convertNot(Opt::Environment& cEnvironment_);

	// try to rewrite relation
	virtual int estimateRewrite(Opt::Environment& cEnvironment_);

	// try to rewrite relation
	virtual RewriteResult rewrite(Opt::Environment& cEnvironment_,
								  IRelation* pRelation_,
								  Predicate::RewriteArgument& cArgument_);

	// search for applicable index file
	virtual IPredicate* check(Opt::Environment& cEnvironment_,
							  const Predicate::CheckArgument& cArgument_);

	// is index file checked?
	virtual bool isChecked();

	// get predicate which is checked for index file
	virtual Predicate::CheckedInterface* getChecked();
	// get predicate which is not checked for index file
	virtual IPredicate* getNotChecked();

	// get checkunknown predicate
	virtual bool getCheckUnknown(Opt::Environment& cEnvironment_,
								 Predicate::CheckUnknownArgument& cArgument_);

	// is index file chosen?
	virtual bool isChosen();
	// get predicate for which a index file is chosen
	virtual Predicate::ChosenInterface* getChosen();

	// is using subquery?
	virtual bool hasSubquery();

	// is fetching by index?
	virtual bool isFetch();
	// get fetch key
	virtual bool getFetchKey(Opt::Environment& cEnvironment_,
							 Utility::ScalarSet& cFetchKey_);
	// create fetch form
	virtual Interface::IPredicate* createFetch(Opt::Environment& cEnvironment_,
											   Utility::ScalarSet& cFetchKey_);
	// is need index?
	virtual bool isNeedIndex();

	// estimate cost
	virtual bool estimateCost(Opt::Environment& cEnvironment_,
							  AccessPlan::Cost& cResult_);

	// set estimate rate cache
	virtual void setEstimateRate(const AccessPlan::Cost::Value& cValue_)
	{m_cEstimateRate = cValue_;}

	// get estimate count cache
	virtual AccessPlan::Cost::Value getEstimateRate()
	{return m_cEstimateRate;}

	// set cached cost
	virtual void setCost(const AccessPlan::Cost& cCost_)
	{m_cCachedCost = cCost_;}

	// get cached cost
	virtual const AccessPlan::Cost& getCost()
	{return m_cCachedCost;}

	// estimate effective rate for a set of tables
	virtual AccessPlan::Cost::Value checkRate(Opt::Environment& cEnvironment_,
											  const Utility::RelationSet& cTable_);

	// generate key for fetching
	virtual int generateKey(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_);

	// create actions specific to predicate
	virtual void adoptIndex(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Action::FileAccess* pFileAccess_,
							Candidate::File* pFile_,
							Candidate::AdoptArgument& cArgument_);
	
	virtual Interface::ICandidate* createDistributePlan(Opt::Environment& cEnvironment_,
														Interface::ICandidate* pOperand_,
														Utility::FieldSet& cPlanSource_);
	
////////////////////////
// Interface::IScalar
//	virtual void explain(Opt::Environment* pEnvironment_,
//						 Opt::Explain& cExplain_);
	virtual const STRING& getName();
	virtual Interface::IScalar* setExpectedType(Opt::Environment& cEnvironment_,
												const Scalar::DataType& cType_);
	virtual Interface::IScalar* createCast(Opt::Environment& cEnvironment_,
										   const Scalar::DataType& cToType_,
										   bool bForComparison_,
										   Tree::Node::Type eType_ = Tree::Node::Undefined);
	virtual Check::Value check(Opt::Environment& cEnvironment_,
							   const Scalar::CheckArgument& cArgument_);
	virtual bool isRefering(Interface::IRelation* pRelation_);
	virtual bool isKnownNull(Opt::Environment& cEnvironment_);
	virtual bool isKnownNotNull(Opt::Environment& cEnvironment_);
	virtual bool hasParameter();
	virtual bool isArbitraryElement();
	virtual bool isField();
	virtual Scalar::Field* getField();
	virtual bool hasField(Interface::IFile* pFile_);
	virtual bool isOperation();
	virtual bool isEquivalent(Interface::IScalar* pScalar_);
	virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pOption_);
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_);
	virtual void retrieve(Opt::Environment& cEnvironment_);
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Interface::ICandidate* pCandidate_);
	virtual void retrieveFromCascade(Opt::Environment& cEnvironment_,
									 Sql::Query* pQuery);
	virtual void use(Opt::Environment& cEnvironment_,
					 Interface::ICandidate* pCandidate_);
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Interface::ICandidate* pCandidate_,
					   Scalar::DelayArgument& cArgument_);
	virtual Common::Data::Pointer preCalculate(Opt::Environment& cEnvironment_);
	virtual void setMetaData(Opt::Environment& cEnvironment_,
							 Common::ColumnMetaData& cMetaData_);
//	virtual int generate(Opt::Environment& cEnvironment_,
//						 Execution::Interface::IProgram& cProgram_,
//						 Execution::Interface::IIterator* pIterator_,
//						 Candidate::AdoptArgument& cArgument_);

	virtual bool isSubquery() const {return false;}

	virtual bool equalsOperand(const Plan::Interface::IScalar* arg) const
	{return false;}

protected:
	// constructor
	IPredicate(IPredicate::Type eOperator_);

private:
	AccessPlan::Cost::Value m_cEstimateRate;
	AccessPlan::Cost m_cCachedCost;
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_IPREDICATE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
