// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Class.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Iterator";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/Class.h"
#include "Execution/Iterator/Array.h"
#include "Execution/Iterator/BitSet.h"
#include "Execution/Iterator/CascadeInput.h"
#include "Execution/Iterator/EmptyNull.h"
#include "Execution/Iterator/Exists.h"
#include "Execution/Iterator/File.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Iterator/Loop.h"
#include "Execution/Iterator/MergeSort.h"
#include "Execution/Iterator/NestedLoop.h"
#include "Execution/Iterator/Tuples.h"
#include "Execution/Iterator/UnionDistinct.h"

#include "Execution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_ITERATOR_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Iterator;
}

// FUNCTION public
//	Iterator::Class::getClassInstance -- get instance from classid
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
	case Category::BitSet:
		{
			pObject = BitSet::getInstance(eCategory);
			break;
		}
	case Category::SingleFile:
		{
			pObject = File::getInstance(eCategory);
			break;
		}
	case Category::Input:
	case Category::InputThread:
		{
			pObject = Input::getInstance(eCategory);
			break;
		}
	case Category::MergeSort:
		{
			pObject = MergeSort::getInstance(eCategory);
			break;
		}
	case Category::UnionDistinct:
		{
			pObject = UnionDistinct::getInstance(eCategory);
			break;
		}
	case Category::CascadeInput:
		{
			pObject = CascadeInput::getInstance(eCategory);
			break;
		}
	case Category::NestedLoop:
		{
			pObject = NestedLoop::getInstance(eCategory);
			break;
		}
	case Category::Filter:
	case Category::DistributionFilter:		
		{
			pObject = Filter::getInstance(eCategory);
			break;
		}
	case Category::Exists:
	case Category::NotExists:
		{
			pObject = Exists::getInstance(eCategory);
			break;
		}
	case Category::Tuples:
		{
			pObject = Tuples::getInstance(eCategory);
			break;
		}
	case Category::EmptyNull:
		{
			pObject = EmptyNull::getInstance(eCategory);
			break;
		}
	case Category::LoopForEver:
	case Category::LoopOnce:
		{
			pObject = Loop::getInstance(eCategory);
			break;
		}
	case Category::Array:
		{
			pObject = Array::getInstance(eCategory);
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
//	Iterator::Class::getClassID -- get classid
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
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
