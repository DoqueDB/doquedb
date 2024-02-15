// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/ChosenSpecification.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_CHOSENSPECIFICATION_H
#define __SYDNEY_PLAN_ORDER_CHOSENSPECIFICATION_H

#include "Plan/Order/SpecificationWrapper.h"

#include "Common/IntegerArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::Order::ChosenSpecification -- Sort chosenSpecification
//
//	NOTES
class ChosenSpecification
	: public SpecificationWrapper
{
public:
	typedef SpecificationWrapper Super;
	typedef ChosenSpecification This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Candidate::File* pFile_,
						Interface::IPredicate* pPredicate_,
						Specification* pChosenSpecification_,
						Specification* pOriginalSpecification_);

	// destructor
	virtual ~ChosenSpecification() {}

	// is partial sort?
	bool isPartial()
	{return getKeySize() < getSpecification()->getKeySize();}

//////////////////////////////////
// Specification::
//	virtual void explain(Opt::Environment* pEnvironment_,
//						 Opt::Explain& cExplain_);
	virtual Size getKeySize() {return m_pChosenSpecification->getKeySize();}
//	virtual Order::Key* getKey(Position iPos_);
	virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_)
	{m_vecPartitionKey = vecPartitionKey_;}
	virtual const VECTOR<Interface::IScalar*>& getPartitionKey()
	{return m_vecPartitionKey;}
//	virtual bool isRefering(Interface::IRelation* pRelation_,
//							Position iPosition_);
//	virtual void require(Opt::Environment& cEnvironment_,
//						 Interface::ICandidate* pCandidate_);
//	virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
//											 Execution::Interface::IProgram& cProgram_,
//											 Execution::Interface::IIterator* pIterator_,
//											 Candidate::AdoptArgument& cArgument_,
//											 Candidate::Row* pRow_,
//											 const Candidate::RowDelayArgument& cDelayArgument_);
	virtual int generateKey(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_);

	virtual Specification* check(Opt::Environment& cEnvironment_,
								 const CheckArgument& cArgument_)
	{return 0;}

	// is index is cheked
	virtual bool isChosen()
	{return true;}

	// get chosen interface
	virtual ChosenSpecification* getChosen()
	{return this;}

	virtual bool hasFunctionKey()
	{
		return m_pChosenSpecification->hasFunctionKey();
	}
 
	//accessors
	Candidate::File* getFile() {return m_pFile;}
	Interface::IPredicate* getPredicate() {return m_pPredicate;}
	Specification* getChosenSpecification() {return m_pChosenSpecification;}
	bool isBitSetSort() {return getChosenSpecification()->isBitSetSort();}

protected:
	// constructor
	ChosenSpecification(Candidate::File* pFile_,
						Interface::IPredicate* pPredicate_,
						Specification* pChosenSpecification_,
						Specification* pOriginalSpecification_)
		: Super(pOriginalSpecification_),
		  m_pFile(pFile_),
		  m_pPredicate(pPredicate_),
		  m_pChosenSpecification(pChosenSpecification_)
	{}

private:
///////////////////////////
// Order::Specification::
	virtual void explainOperand(Opt::Environment* pEnvironment_,
								Opt::Explain& cExplain_);

	Candidate::File* m_pFile;
	Interface::IPredicate* m_pPredicate;
	Specification* m_pChosenSpecification;

	VECTOR<Interface::IScalar*> m_vecPartitionKey;
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_CHOSENSPECIFICATION_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
