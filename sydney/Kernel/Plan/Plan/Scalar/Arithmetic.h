// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Arithmetic.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_ARITHMETIC_H
#define __SYDNEY_PLAN_SCALAR_ARITHMETIC_H

#include "Plan/Scalar/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Arithmetic -- Interface for arithmetic
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Arithmetic
	: public Function
{
public:
	typedef Function Super;
	typedef Arithmetic This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_,
						const STRING& cstrName_);

	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const VECTOR<Interface::IScalar*>& vecOperand_,
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
						const DataType& cDataType_,
						const STRING& cstrName_);

	// destructor
	virtual ~Arithmetic() {}

/////////////////////////
// Interface::IScalar::

protected:
	// constructor
	Arithmetic()
		: Super()
	{}
	Arithmetic(Tree::Node::Type eOperator_,
				const STRING& cstrName_)
		: Super(eOperator_, cstrName_)
	{}
	Arithmetic(Tree::Node::Type eOperator_,
				const DataType& cDataType_,
				const STRING& cstrName_)
		: Super(eOperator_, cDataType_, cstrName_)
	{}

	// convert operand to expected type
	Interface::IScalar* castOperand(Opt::Environment& cEnvironment_,
									Interface::IScalar* pOperand_);

private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_ARITHMETIC_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
