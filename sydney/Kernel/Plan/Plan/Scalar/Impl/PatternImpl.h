// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/PatternImpl.h --
// 
// Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_PATTERNIMPL_H
#define __SYDNEY_PLAN_SCALAR_PATTERNIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace PatternImpl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::PatternImpl::Monadic -- implementation class for length operations
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


		// destructor
		~Monadic() {}

	/////////////////////////
	// Interface::ISqlNode::
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);


	protected:
	private:
	};

	class MonadicWithMonadicOption
		: public FunctionImpl::MonadicWithMonadicOption
	{
	public:
		typedef FunctionImpl::MonadicWithMonadicOption Super;
		typedef MonadicWithMonadicOption This;

		// constructor
		MonadicWithMonadicOption(Tree::Node::Type eType_,
			 const STRING& cstrName_,
			 Interface::IScalar* pOperand_,
			 Interface::IScalar* pOption_)
			: Super(eType_, cstrName_, pOperand_, pOption_)
			{}

		// destructor
		virtual ~MonadicWithMonadicOption() {}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);
		
		protected:
		private:

		};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_PATTERNIMPL_H

//
//	Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
