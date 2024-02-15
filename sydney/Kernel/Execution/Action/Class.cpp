// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Class.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Class.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Action/Locker.h"
#include "Execution/Action/NoTypeData.h"
#include "Execution/Action/Thread.h"
#include "Execution/Externalizable.h"

#include "Common/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_ACTION_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Action;
}

// FUNCTION public
//	Action::Class::getClassInstance -- get instance from classid
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
	case Category::NoTypeData:
		{
			pObject = NoTypeData::getInstance(eCategory);
			break;
		}
	case Category::FileAccess:
		{
			pObject = FileAccess::getInstance(eCategory);
			break;
		}
	case Category::LockerNormal:
	case Category::LockerBitSet:
	case Category::LockerBitSetCacheAll:
	case Category::LockerCacheAll:
	case Category::LockerBitSetSort:		
	case Category::LockerUnlocker:
		{
			pObject = Locker::getInstance(eCategory);
			break;
		}
	case Category::Locator:
		{
			pObject = Locator::getInstance(eCategory);
			break;
		}
	case Category::Thread:
		{
			pObject = Thread::getInstance(eCategory);
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
//	Action::Class::getClassID -- get classid
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
//	Copyright (c) 2009, 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
