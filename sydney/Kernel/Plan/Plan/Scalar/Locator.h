// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Locator.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_LOCATOR_H
#define __SYDNEY_PLAN_SCALAR_LOCATOR_H

#include "Plan/Scalar/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Locator -- Interface for locator
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Locator
	: public Function
{
public:
	typedef Function Super;
	typedef Locator This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const STRING& cstrName_);

	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_,
						const DataType& cDataType_,
						const STRING& cstrName_);

	// destructor
	virtual ~Locator() {}

/////////////////////////
// Interface::IScalar::

protected:
	// constructor
	Locator()
		: Super()
	{}
	Locator(Tree::Node::Type eOperator_,
			const STRING& cstrName_)
		: Super(eOperator_, cstrName_)
	{}
	Locator(Tree::Node::Type eOperator_,
			const DataType& cDataType_,
			const STRING& cstrName_)
		: Super(eOperator_, cDataType_, cstrName_)
	{}

private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_LOCATOR_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
