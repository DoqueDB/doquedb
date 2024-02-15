// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/CheckedField.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/CheckedField.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::CheckedField::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Field* pField_
//	const Utility::FileSet& cFileSet_
//	
// RETURN
//	CheckedField*
//
// EXCEPTIONS

//static
CheckedField*
CheckedField::
create(Opt::Environment& cEnvironment_,
	   Field* pField_,
	   const Utility::FileSet& cFileSet_)
{
	AUTOPOINTER<This> pResult = new This(pField_, cFileSet_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
