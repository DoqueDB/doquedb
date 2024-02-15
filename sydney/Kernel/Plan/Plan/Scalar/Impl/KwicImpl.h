// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/KwicImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_KWICIMPL_H
#define __SYDNEY_PLAN_SCALAR_KWICIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"
#include "Execution/Function/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace Impl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::Impl::KwicImpl -- implementation class for kwic
	//
	// NOTES
	class KwicImpl
		: public FunctionImpl::DyadicWithNadicOption
	{
	public:
		typedef FunctionImpl::DyadicWithNadicOption Super;
		typedef KwicImpl This;

		// constructor
		KwicImpl(Tree::Node::Type eType_,
				 const STRING& cstrName_,
				 const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
				 const VECTOR<Interface::IScalar*>& vecOption_)
			: Super(eType_, cstrName_, cOperand_, vecOption_)
		{}

		// destructor
		~KwicImpl() {}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			cExec_.append(toSQLStatement(cEnvironment_, cArgument_));
		}
	protected:
	/////////////////////////
	// Scalar::Function::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		void generateKwic(Opt::Environment& cEnvironment_,
						  Execution::Interface::IProgram& cProgram_,
						  Execution::Interface::IIterator* pIterator_,
						  Candidate::AdoptArgument& cArgument_,
						  Predicate::Contains* pContains_,
						  Execution::Function::Kwic* pKwic_,
						  bool bSecondary_);

	/////////////////////////
	// Scalar::Function::
		virtual void createDataType(Opt::Environment& cEnvironment_);
	};
} // namespace Impl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_KWICIMPL_H

//
//	Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
