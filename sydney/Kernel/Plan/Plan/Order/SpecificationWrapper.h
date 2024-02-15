// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/SpecificationWrapper.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_SPECIFICATIONWRAPPER_H
#define __SYDNEY_PLAN_ORDER_SPECIFICATIONWRAPPER_H

#include "Plan/Order/Specification.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::Order::SpecificationWrapper -- Sort specificationWrapper
//
//	NOTES
class SpecificationWrapper
	: public Specification
{
public:
	typedef Specification Super;
	typedef SpecificationWrapper This;

	// destructor
	virtual ~SpecificationWrapper() {}

//////////////////////////////////
// Specification::
//	virtual void explain(Opt::Environment* pEnvironment_,
//						 Opt::Explain& cExplain_);
	virtual Size getKeySize() {return m_pSpecification->getKeySize();}
	virtual Order::Key* getKey(Position iPos_) {return m_pSpecification->getKey(iPos_);}
//	virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_);
//	virtual const VECTOR<Interface::IScalar*>& getPartitionKey();
	virtual bool isRefering(Interface::IRelation* pRelation_,
							Position iPosition_)
	{return m_pSpecification->isRefering(pRelation_, iPosition_);}
	virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_)
	{return m_pSpecification->hasAlternativeValue(cEnvironment_);}
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
	{m_pSpecification->require(cEnvironment_, pCandidate_);}

	virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
											 Execution::Interface::IProgram& cProgram_,
											 Execution::Interface::IIterator* pIterator_,
											 Candidate::AdoptArgument& cArgument_,
											 Candidate::Row* pRow_,
											 const Candidate::RowDelayArgument& cDelayArgument_)
	{return m_pSpecification->generate(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cArgument_,
									   pRow_,
									   cDelayArgument_);}
	virtual int generateKey(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_)
	{return m_pSpecification->generateKey(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_);}
	virtual Specification* check(Opt::Environment& cEnvironment_,
								 const CheckArgument& cArgument_)
	{return m_pSpecification->check(cEnvironment_, cArgument_);}

	virtual bool isChosen()
	{return m_pSpecification->isChosen();}

	virtual ChosenSpecification* getChosen()
	{return m_pSpecification->getChosen();}
 
	virtual bool isGroupBy() {return m_pSpecification->isGroupBy();}

	virtual bool isBitSetSort() { return m_pSpecification->isBitSetSort(); }
	
	virtual void setBitSetSort() {m_pSpecification->setBitSetSort();}

	virtual bool hasExpandElement() {return m_pSpecification->hasExpandElement();}

	virtual bool isAscending() {return m_pSpecification->isAscending();}

	virtual bool isWordGrouping() {return m_pSpecification->isWordGrouping();} 

	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const
	{return m_pSpecification->toSQLStatement(cEnvironment_, cArgument_);}

	//accessor
	Specification* getSpecification() {return m_pSpecification;}

protected:
	// constructor
	SpecificationWrapper(Specification* pSpecification_)
		: Super(),
		  m_pSpecification(pSpecification_)
	{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
	SpecificationWrapper()
		: Super()
	{}
	void setSpecification(Specification* pSpecification_)
	{m_pSpecification = pSpecification_;}
#endif

private:
	Specification* m_pSpecification;
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_SPECIFICATIONWRAPPER_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
