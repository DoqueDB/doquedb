// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Collection/Class.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Collection";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Collection/Class.h"
#include "Execution/Collection/Bulk.h"
#include "Execution/Collection/BitsetDisintegration.h"
#include "Execution/Collection/Connection.h"
#include "Execution/Collection/Distinct.h"
#include "Execution/Collection/Grouping.h"
#include "Execution/Collection/Partitioning.h"
#include "Execution/Collection/Queue.h"
#include "Execution/Collection/Sort.h"
#include "Execution/Collection/Store.h"
#include "Execution/Collection/VirtualTable.h"
#include "Execution/Collection/Variable.h"

#include "Execution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_COLLECTION_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Collection;
}

// FUNCTION public
//	Collection::Class::getClassInstance -- get instance from classid
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
	case Category::Connection:
		{
			pObject = Connection::getInstance(eCategory);
			break;
		}
	case Category::Sort:
		{
			pObject = Sort::getInstance(eCategory);
			break;
		}
	case Category::Grouping:
		{
			pObject = Grouping::getInstance(eCategory);
			break;
		}
	case Category::Distinct:
	case Category::DistinctByRowID:
		{
			pObject = Distinct::getInstance(eCategory);
			break;
		}
	case Category::Store:
		{
			pObject = Store::getInstance(eCategory);
			break;
		}
	case Category::Queue:
	case Category::QueueSafe:
		{
			pObject = Queue::getInstance(eCategory);
			break;
		}
	case Category::Bulk:
		{
			pObject = Bulk::getInstance(eCategory);
			break;
		}
	case Category::VirtualTableUser:
	case Category::VirtualTableSession:
		{
			pObject = VirtualTable::getInstance(eCategory);
			break;
		}
	case Category::Partitioning:
		{
			pObject = Partitioning::getInstance(eCategory);
			break;
		}
	case Category::BitsetDisintegration:
	    {
			pObject = BitsetDisintegration::getInstance(eCategory);
			break;
		}
	case Category::BitSetVariable:
	    {
			pObject = Variable::getInstance(eCategory);
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
//	Collection::Class::getClassID -- get classid
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
//	Copyright (c) 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
