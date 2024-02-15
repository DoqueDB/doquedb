// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/UpdateField.cpp --
// 
// Copyright (c) 2012, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Scalar";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DPlan/Scalar/UpdateField.h"
#include "DPlan/Scalar/Impl/UpdateFieldImpl.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::UpdateField::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Scalar::Field* pField_
//	Plan::Interface::IScalar* pInput_
//	
// RETURN
//	UpdateField*
//
// EXCEPTIONS

//static
UpdateField::Super*
UpdateField::
create(Opt::Environment& cEnvironment_,
	   Plan::Scalar::Field* pField_,
	   Plan::Interface::IScalar* pInput_)
{
	AUTOPOINTER<Super> pResult = new UpdateFieldImpl::Insert(pField_, pInput_);
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
//	Plan::Scalar::Field* pField_
//	const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cInput_
//	
// RETURN
//	UpdateField*
//
// EXCEPTIONS

//static
UpdateField::Super*
UpdateField::
create(Opt::Environment& cEnvironment_,
	   Plan::Scalar::Field* pField_,
	   const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cInput_)
{
	AUTOPOINTER<Super> pResult;
	if (cInput_.first == 0) {
		pResult = new UpdateFieldImpl::Delete(pField_, cInput_.second);
	} else {
		pResult = new UpdateFieldImpl::Update(pField_, cInput_);
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
