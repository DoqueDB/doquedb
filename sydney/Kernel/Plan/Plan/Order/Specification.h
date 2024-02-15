// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/Specification.h --
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

#ifndef __SYDNEY_PLAN_ORDER_SPECIFICATION_H
#define __SYDNEY_PLAN_ORDER_SPECIFICATION_H

#include "Plan/Order/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Tree/Node.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Interface/ISqlNode.h"

#include "Execution/Declaration.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::Order::Specification -- Sort specification
//
//	NOTES
class Specification
	: public Tree::Node, public Interface::ISqlNode
{
public:
	typedef Tree::Node Super;
	typedef Specification This;
	typedef int Position;
	typedef Position Size;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Order::Key* pKey_);
	static This* create(Opt::Environment& cEnvironment_,
						const VECTOR<Order::Key*>& vecKey_, bool bGroupBy_);

	// destructor
	virtual ~Specification() {}

	// get compatible specification
	static Specification* getCompatible(Opt::Environment& cEnvironment_,
										Specification* pSpecification1_,
										Specification* pSpecification2_);
	// check compatibility
	static bool isCompatible(Specification* pSpecification1_,
							 Specification* pSpecification2_);
	// has same partition key?
	static bool hasSamePartitionKey(Specification* pSpecification1_,
									Specification* pSpecification2_);

	// explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_);

	// get key size
	virtual Size getKeySize() = 0;
	// get key at specified position
	virtual Order::Key* getKey(Position iPos_) = 0;

	virtual Interface::IScalar* getGroupByOption() {return 0;}
	
	// set partition by key
	virtual void setPartitionKey(const VECTOR<Interface::IScalar*>& vecPartitionKey_) = 0;
	// get partition by key
	virtual const VECTOR<Interface::IScalar*>& getPartitionKey() = 0;

	// is refering specified relation?
	virtual bool isRefering(Interface::IRelation* pRelation_,
							Position iPosition_) = 0;

	// does key have alternative value?
	virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_) = 0;

	// require sorting key
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_) = 0;

	// generate tuple data and position/direction vector
	virtual GeneratedSpecification* generate(Opt::Environment& cEnvironment_,
											 Execution::Interface::IProgram& cProgram_,
											 Execution::Interface::IIterator* pIterator_,
											 Candidate::AdoptArgument& cArgument_,
											 Candidate::Row* pRow_,
											 const Candidate::RowDelayArgument& cDelayArgument_) = 0;

	// generate key data
	virtual int generateKey(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_) = 0;

	// check available sort key
	virtual Specification* check(Opt::Environment& cEnvironment_,
								 const CheckArgument& cArgument_) = 0;

	// is index is cheked
	virtual bool isChecked() {return false;}

	// get checked interface
	virtual CheckedSpecification* getChecked() {return 0;}

	// is index is checked
	virtual bool isChosen() {return false;}

	// get chosen interface
	virtual ChosenSpecification* getChosen() {return 0;}

	// is generated
	virtual bool isGenerated() {return false;}

	// get generated interface
	virtual GeneratedSpecification* getGenerated() {return 0;}

	virtual bool isGroupBy() {return false;}

	virtual bool isBitSetSort() = 0;
	
	virtual void setBitSetSort() = 0;

	virtual bool hasExpandElement() = 0;

	virtual bool hasFunctionKey() = 0;

	virtual bool isAscending() = 0;

	virtual bool isWordGrouping() = 0;

protected:
	// constructor
	Specification() : Super(Super::OrderBy) {}
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

private:
	virtual void explainOperand(Opt::Environment* pEnvironment_,
								Opt::Explain& cExplain_) = 0;
	
	virtual void explainOption(Opt::Environment* pEnvironment_,
							   Opt::Explain& cExplain_){};
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_SPECIFICATION_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
