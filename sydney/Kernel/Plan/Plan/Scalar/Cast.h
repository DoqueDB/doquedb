// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Cast.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_CAST_H
#define __SYDNEY_PLAN_SCALAR_CAST_H

#include "Plan/Scalar/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Cast -- Interface for cast
//
//	NOTES
//	This class is not created directly.
//	Implementation class of this interface is described in cpp file.
class Cast
	: public Function
{
public:
	typedef Function Super;
	typedef Cast This;

	// constructor
	static Interface::IScalar*
					create(Opt::Environment& cEnvironment_,
						   Interface::IScalar* pOperand_,
						   const DataType& cToType_,
						   bool bForComparison_,
						   bool bNoThrow_);
	// destructor
	virtual ~Cast() {}

protected:
	// constructor
	Cast() : Super() {}
	Cast(Tree::Node::Type eType_,
		 const STRING& cstrName_)
		: Super(eType_, cstrName_)
	{}
	Cast(Tree::Node::Type eType_,
		 const DataType& cDataType_,
		 const STRING& cstrName_)
		: Super(eType_, cDataType_, cstrName_)
	{}

private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CAST_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
