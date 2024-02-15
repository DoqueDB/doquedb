// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Choice.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_CHOICE_H
#define __SYDNEY_PLAN_SCALAR_CHOICE_H

#include "Plan/Scalar/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace ChoiceImpl
{
	struct ConvertOperandArgument;
}

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Choice -- Interface for choice
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Choice
	: public Function
{
public:
	typedef Function Super;
	typedef Choice This;

	// constructor
	static Interface::IScalar*
				create(Opt::Environment& cEnvironment_,
					   Tree::Node::Type eOperator_,
					   const VECTOR<Interface::IScalar*>& vecOperand_,
					   const STRING& cstrName_);

/////////////////////////
// Interface::IScalar::

protected:
private:
	// never constructed
	Choice() {}
	~Choice() {}

	static bool convertOperand(Opt::Environment& cEnvironment_,
							   Interface::IScalar* pOperand_,
							   ChoiceImpl::ConvertOperandArgument& cArgument_);
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CHOICE_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
