// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Predicate/Class.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DExecution/Predicate/Class.h"
#include "DExecution/Predicate/IsValid.h"
#include "DExecution/Predicate/HasNextCandidate.h"

#include "DExecution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_DEXECUTION_USING
_SYDNEY_DEXECUTION_PREDICATE_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Predicate;
}

// FUNCTION public
//	Predicate::Class::getClassInstance -- get instance from classid
//
// NOTES
//
// ARGUMENTS
//	int iClassID_
//	
// RETURN
//	Common::Externalizable*
//
// EXCEPTIONS

// static
Common::Externalizable*
Class::
getClassInstance(int iClassID_)
{
	Common::Externalizable* pObject = 0;

	Category::Value eCategory =
		static_cast<Category::Value>(iClassID_ - _iSubModule);

	switch ( eCategory ) {
	case Category::IsValid:
		{
			pObject = IsValid::getInstance(eCategory);
			break;
		}
case Category::HasNextCandidate:
		{
			pObject = HasNextCandidate::getInstance(eCategory);
			break;
		}		
	default:
		{
			break;
		}
	}
	return pObject;
}

// FUNCTION public
//	Predicate::Class::getClassID -- get classid
//
// NOTES
//
// ARGUMENTS
//	int iBase_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
Class::
getClassID(Category::Value eCategory_)
{
	return Externalizable::getClassID(eCategory_ + _iSubModule);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
