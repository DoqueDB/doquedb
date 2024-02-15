// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/InvokeImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_INVOKEIMPL_H
#define __SYDNEY_PLAN_SCALAR_INVOKEIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"
#include "Plan/Scalar/Invoke.h"

#include "Schema/Function.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace Impl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::InvokeImpl::Monadic -- implementation class for invoke operations
	//
	// NOTES
	class InvokeImpl
		: public FunctionImpl::Base< Tree::Nadic<Scalar::Invoke, Interface::IScalar> >
	{
	public:
		typedef FunctionImpl::Base< Tree::Nadic<Scalar::Invoke, Interface::IScalar> > Super;
		typedef InvokeImpl This;

		// constructor
		InvokeImpl(const Schema::Function* pSchemaFunction_,
				   const VECTOR<Interface::IScalar*>& vecOperand_,
				   const STRING& cstrName_)
			: Super(Tree::Node::Procedure,
					DataType(pSchemaFunction_->getReturnType()),
					cstrName_,
					vecOperand_),
			  m_pSchemaFunction(pSchemaFunction_)
		{}

		// destructor
		~InvokeImpl() {}

	/////////////////////////
	// Interface::IScalar::

	protected:
		// generate
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		const Schema::Function* m_pSchemaFunction;
	};

} // namespace InvokeImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_INVOKEIMPL_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
