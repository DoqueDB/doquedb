// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Predicate/Class.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2015, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/Class.h"
#include "Execution/Predicate/Between.h"
#include "Execution/Predicate/CheckUnknown.h"
#include "Execution/Predicate/CollectionCheck.h"
#include "Execution/Predicate/Combinator.h"
#include "Execution/Predicate/Comparison.h"
#include "Execution/Predicate/In.h"
#include "Execution/Predicate/IsEmpty.h"
#include "Execution/Predicate/IsSubstringOf.h"
#include "Execution/Predicate/Like.h"
#include "Execution/Predicate/RowIDCheck.h"
#include "Execution/Predicate/Similar.h"

#include "Execution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_PREDICATE_USING

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

	switch (eCategory) {
	case Category::CombinatorAnd:
	case Category::CombinatorAndDyadic:
	case Category::CombinatorOr:
	case Category::CombinatorOrDyadic:
	case Category::CombinatorNot:
		{
			pObject = Combinator::getInstance(eCategory);
			break;
		}
	case Category::ComparisonDyadic:
	case Category::ComparisonMonadic:
	case Category::ComparisonDistinct:
	case Category::ComparisonDyadicAnyElement:
	case Category::ComparisonMonadicAnyElement:
	case Category::ComparisonDyadicAllElement:
	case Category::ComparisonMonadicAllElement:
	case Category::ComparisonDyadicRow:
		{
			pObject = Comparison::getInstance(eCategory);
			break;
		}
	case Category::Between:
	case Category::NotBetween:
	case Category::BetweenAnyElement:
	case Category::NotBetweenAnyElement:
		{
			pObject = Between::getInstance(eCategory);
			break;
		}
	case Category::RowIDCheckByBitSet:
	case Category::RowIDCheckByCollection:
		{
			pObject = RowIDCheck::getInstance(eCategory);
			break;
		}
	case Category::CollectionCheck:
		{
			pObject = CollectionCheck::getInstance(eCategory);
			break;
		}
	case Category::IsEmptyBitSet:
	case Category::IsEmptyIterator:
		{
			pObject = IsEmpty::getInstance(eCategory);
			break;
		}
	case Category::CheckUnknownMonadic:
	case Category::CheckUnknownNadic:
		{
			pObject = CheckUnknown::getInstance(eCategory);
			break;
		}
	case Category::Like:
	case Category::LikeAnyElement:
		{
			pObject = Like::getInstance(eCategory);
			break;
		}
	case Category::Similar:
	case Category::SimilarAnyElement:
		{
			pObject = Similar::getInstance(eCategory);
			break;
		}
	case Category::In:
	case Category::NotIn:
	case Category::InAnyElement:
	case Category::NotInAnyElement:
		{
			pObject = In::getInstance(eCategory);
			break;
		}
	case Category::IsSubstringOf:
		{
			pObject = IsSubstringOf::getInstance(eCategory);
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
//	Copyright (c) 2009, 2010, 2011, 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
