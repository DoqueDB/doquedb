// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/CheckedInterface.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_CHECKEDINTERFACE_H
#define __SYDNEY_PLAN_PREDICATE_CHECKEDINTERFACE_H

#include "Plan/Predicate/InterfaceWrapper.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::CheckedInterface -- Wrapper interface for representing index of predicates
//
//	NOTES
//		This class is not constructed directly
class CheckedInterface
	: public InterfaceWrapper
{
public:
	typedef InterfaceWrapper Super;
	typedef CheckedInterface This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Candidate::Table* pTable_,
						const Utility::FileSet& cFile_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						const VECTOR<Interface::IPredicate*>& vecChecked_,
						Candidate::Table* pTable_,
						const Utility::FileSet& cFile_,
						bool bNoTop_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						const VECTOR<Interface::IPredicate*>& vecChecked_,
						Interface::IPredicate* pNotChecked_,
						Candidate::Table* pTable_,
						const Utility::FileSet& cFile_,
						bool bNoTop_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pPredicate_,
						Interface::IPredicate* pOperand_);

	// destructor
	virtual ~CheckedInterface() {}

//////////////////
// accessors
//	Interface::IPredicate* getPredicate();
	virtual const Utility::FileSet& getFile() = 0;
	virtual Candidate::Table* getTable() = 0;
	virtual bool isUseIndex() = 0;

////////////////////////
// Interface::
	virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
										 const CheckArgument& cArgument_)
	{return this;}
	virtual bool isChecked()
	{return true;}
	virtual CheckedInterface* getChecked()
	{return this;}
	virtual Interface::IPredicate* getNotChecked()
	{return 0;}

	virtual Interface::IPredicate* choose(Opt::Environment& cEnvironment_,
										  ChooseArgument& cArgument_) = 0;
	virtual bool isChosen()
	{return false;}
	virtual ChosenInterface* getChosen()
	{return 0;}

////////////////////////
// Interface::IScalar
//	virtual const STRING& getName();
//	virtual bool isRefering(Interface::IRelation* pRelation_);

////////////////////////
// Tree::Node::
//	Interface::Type getType() const;
//	virtual ModUnicodeString getValue() const;
//	virtual const Common::Data* getData() const;
//	virtual ModSize getOptionSize() const;
//	virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const;
//	virtual ModSize getOperandSize() const;
//	virtual const Tree::Node::Super* getOperandAt(ModInt32 iPosition_) const

protected:
	// constructor
	CheckedInterface(Interface::IPredicate* pPredicate_);
private:
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_CHECKEDINTERFACE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
