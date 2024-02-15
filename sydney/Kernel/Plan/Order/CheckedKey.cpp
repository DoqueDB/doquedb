// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/CheckedKey.cpp --
// 
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Order";
}

#include "SyDefault.h"

#include "Plan/Order/CheckedKey.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_ORDER_USING

// FUNCTION public
//	Order::CheckedKey::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Key* pKey_
//	const Utility::FileSet& cFile_
//	
// RETURN
//	CheckedKey*
//
// EXCEPTIONS

//static
CheckedKey*
CheckedKey::
create(Opt::Environment& cEnvironment_,
	   Key* pKey_,
	   const Utility::FileSet& cFile_)
{
	AUTOPOINTER<This> pResult = new This(pKey_, cFile_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Order::CheckedKey::CheckedKey -- constructor
//
// NOTES
//
// ARGUMENTS
//	Key* pKey_
//	const Utility::FileSet& cFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

CheckedKey::
CheckedKey(Key* pKey_,
		   const Utility::FileSet& cFile_)
	: Super(),
	  m_pKey(pKey_),
	  m_cFile(cFile_)
{}

//
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
