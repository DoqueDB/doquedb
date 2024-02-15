// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/UpdateFieldImpl.h --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_SCALAR_UPDATEFIELDIMPL_H
#define __SYDNEY_DPLAN_SCALAR_UPDATEFIELDIMPL_H

#include "DPlan/Scalar/UpdateField.h"
#include "Plan/Scalar/Impl/UpdateFieldImpl.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

namespace UpdateFieldImpl
{
	////////////////////////////////////////
	// TEMPLATE CLASS
	//	DPlan::Scalar::UpdateFieldImpl::GenerateMix --
	//
	// TEMPLATE ARGUMENTS
	//	clas Base_
	//
	// NOTES
	template <class Base_>
	class GenerateMix
		: public Base_
	{
	public:
		typedef Base_ Super;
		typedef GenerateMix This;

		// constructor
		GenerateMix(Plan::Scalar::Field* pField_,
					typename Super::Argument cArgument_)
			: Super(pField_, cArgument_)
		{}

		// destructor
		virtual ~GenerateMix() {}

	protected:
		// generate operand for updating operations
		int generateOperand(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Plan::Candidate::AdoptArgument& cArgument_,
							Plan::Interface::IScalar* pOperand_)
		{
			return pOperand_->generate(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cArgument_);
		}
	private:
	};

	////////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::UpdateFieldImpl::Insert -- implementation class for insert updateField
	//
	// NOTES
	typedef GenerateMix<Plan::Scalar::UpdateFieldImpl::Insert> Insert;

	////////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::UpdateFieldImpl::Delete -- implementation class for delete updateField
	//
	// NOTES
	typedef GenerateMix<Plan::Scalar::UpdateFieldImpl::Delete> Delete;

	////////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::UpdateFieldImpl::Update -- implementation class for update updateField
	//
	// NOTES
	typedef GenerateMix<Plan::Scalar::UpdateFieldImpl::Update> Update;
}

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_SCALAR_UPDATEFIELDIMPL_H

//
//	Copyright (c) 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
