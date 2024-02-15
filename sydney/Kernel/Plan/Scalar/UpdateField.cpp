// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/UpdateField.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2016, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/UpdateField.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"
#include "Plan/Scalar/Impl/UpdateFieldImpl.h"

#include "Plan/Sql/Argument.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Relation/Table.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::UpdateField::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Field* pField_
//	Interface::IScalar* pInput_
//	
// RETURN
//	UpdateField*
//
// EXCEPTIONS

//static
UpdateField*
UpdateField::
create(Opt::Environment& cEnvironment_,
	   Field* pField_,
	   Interface::IScalar* pInput_)
{
	AUTOPOINTER<This> pResult = new UpdateFieldImpl::Insert(pField_, pInput_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::UpdateField::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Field* pField_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cInput_
//	
// RETURN
//	UpdateField*
//
// EXCEPTIONS

//static
UpdateField*
UpdateField::
create(Opt::Environment& cEnvironment_,
	   Field* pField_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cInput_)
{
	AUTOPOINTER<This> pResult;
	if (cInput_.first == 0) {
		pResult = new UpdateFieldImpl::Delete(pField_, cInput_.second);
	} else {
		Interface::IScalar* pInput = cInput_.first;
		Interface::IScalar* pOriginal = cInput_.second;
		if (pInput->checkOperation(cEnvironment_, pOriginal)) {
			// Lob column can be updated by operation
			pResult =
				(new UpdateFieldImpl::UpdateByOperation(pField_, MAKEPAIR(pInput, pOriginal)))
					->setOperation(cEnvironment_);
		} else {
			// normal update
			pResult = new UpdateFieldImpl::Update(pField_, MAKEPAIR(pInput, pOriginal));
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
