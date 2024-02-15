// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/GeneratedSpecification.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_GENERATEDSPECIFICATION_H
#define __SYDNEY_PLAN_ORDER_GENERATEDSPECIFICATION_H

#include "Plan/Order/SpecificationWrapper.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::Order::GeneratedSpecification -- Sort generatedSpecification
//
//	NOTES
class GeneratedSpecification
	: public SpecificationWrapper
{
public:
	typedef SpecificationWrapper Super;
	typedef GeneratedSpecification This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Specification* pSpecification_,
						const VECTOR<int>& vecDataID_,
						const VECTOR<int>& vecPosition_,
						const VECTOR<int>& vecDirection_,
						const VECTOR<Interface::IScalar*>& vecScalar_);
	
	static This* create(Opt::Environment& cEnvironment_,
						Specification* pSpecification_,
						const VECTOR<int>& vecDataID_,
						const VECTOR<int>& vecPosition_,
						const VECTOR<int>& vecDirection_,
						const VECTOR<Interface::IScalar*>& vecScalar_,
						const VECTOR<int>& vecWordPosition_);
	

	// destructor
	virtual ~GeneratedSpecification() {}

//////////////////////////////////
// Specification::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_)
	{getSpecification()->explain(pEnvironment_, cExplain_);}
//	virtual Size getKeySize();
//	virtual Order::Key* getKey(Position iPos_);
	virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_)
	{; /* do nothing */}
	virtual const VECTOR<Interface::IScalar*>& getPartitionKey()
	{return getSpecification()->getPartitionKey();}
//	virtual bool isRefering(Interface::IRelation* pRelation_,
//							Position iPosition_);
//	virtual void require(Opt::Environment& cEnvironment_,
//						 Interface::ICandidate* pCandidate_);
	virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
											 Execution::Interface::IProgram& cProgram_,
											 Execution::Interface::IIterator* pIterator_,
											 Candidate::AdoptArgument& cArgument_,
											 Candidate::Row* pRow_)
	{return this;}
//	virtual Specification* check(Opt::Environment& cEnvironment_,
//							 	 const CheckArgument& cArgument_);

	// is generated
	virtual bool isGenerated()
	{return true;}

	// get generated interface
	virtual GeneratedSpecification* getGenerated()
	{return this;}

	virtual bool hasFunctionKey()
	{
		return getSpecification()->hasFunctionKey();
	}
 
	//accessors
	const VECTOR<int>& getDataID()
	{return m_vecDataID;}
	const VECTOR<int>& getPosition()
	{return m_vecPosition;}
	const VECTOR<int>& getDirection()
	{return m_vecDirection;}
	const VECTOR<Interface::IScalar*>& getTuple()
	{return m_vecScalar;}
	const VECTOR<int>& getWordPosition()
	{return m_vecWordPosition;}	

protected:
	// constructor
	GeneratedSpecification(Specification* pSpecification_,
						   const VECTOR<int>& vecDataID_,
						   const VECTOR<int>& vecPosition_,
						   const VECTOR<int>& vecDirection_,
						   const VECTOR<Interface::IScalar*>& vecScalar_,
						   const VECTOR<int>& vecWordPosition_)
		: Super(pSpecification_),
		  m_vecDataID(vecDataID_),
		  m_vecPosition(vecPosition_),
		  m_vecDirection(vecDirection_),
		  m_vecScalar(vecScalar_),
		  m_vecWordPosition(vecWordPosition_)
	{}

private:
///////////////////////////
// Order::Specification::
	virtual void explainOperand(Opt::Environment* pEnvironment_,
								Opt::Explain& cExplain_);

	VECTOR<int> m_vecDataID;
	VECTOR<int> m_vecPosition;
	VECTOR<int> m_vecDirection;
	VECTOR<int> m_vecWordPosition;
	VECTOR<Interface::IScalar*> m_vecScalar;
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_GENERATEDSPECIFICATION_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
