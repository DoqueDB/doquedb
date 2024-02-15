// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/CaseImpl.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_CASEIMPL_H
#define __SYDNEY_PLAN_SCALAR_CASEIMPL_H

#include "Plan/Scalar/Impl/FunctionImpl.h"
#include "Plan/Scalar/Case.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace Impl
{
	////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::Impl::CaseImpl -- implementation class for case operations
	//
	// NOTES
	class CaseImpl
		: public FunctionImpl::BaseWithOption<
					Tree::NadicOption<
						FunctionImpl::Base<
							Tree::Nadic<Scalar::Case,
										Interface::IScalar> >,
						Interface::IPredicate> >
	{
	public:
		typedef FunctionImpl::BaseWithOption<
					Tree::NadicOption<
						FunctionImpl::Base<
							Tree::Nadic<Scalar::Case,
										Interface::IScalar> >,
						Interface::IPredicate> > Super;
		typedef CaseImpl This;

		// constructor
		CaseImpl(const VECTOR<Interface::IPredicate*>& vecCaseList_,
				 const VECTOR<Interface::IScalar*>& vecResultList_,
				 const STRING& cstrName_)
			: Super(Tree::Node::Case, cstrName_, vecResultList_, vecCaseList_)
		{}

		// destructor
		~CaseImpl() {}

	/////////////////////////
	// Interface::IScalar::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	protected:
	private:
	};

} // namespace CaseImpl

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CASEIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
