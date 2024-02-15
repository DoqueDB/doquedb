// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/LengthImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_LENGTHIMPL_H
#define __SYDNEY_PLAN_SCALAR_LENGTHIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace LengthImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::LengthImpl::Monadic -- implementation class for length operations
	//
	// NOTES
	class Monadic
		: public FunctionImpl::Monadic
	{
	public:
		typedef FunctionImpl::Monadic Super;
		typedef Monadic This;

		// constructor
		Monadic(Tree::Node::Type eType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cstrName_, pOperand_)
		{}
		Monadic(Tree::Node::Type eType_,
				const DataType& cDataType_,
				const STRING& cstrName_,
				Interface::IScalar* pOperand_)
			: Super(eType_, cDataType_, cstrName_, pOperand_)
		{}

		// destructor
		~Monadic() {}

	/////////////////////////
	// Interface::IScalar::
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_);
	protected:
	private:
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_LENGTHIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
