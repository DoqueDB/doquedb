// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Function/Class.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Function/Class.h"
#include "Execution/Function/Aggregation.h"
#include "Execution/Function/Arithmetic.h"
#include "Execution/Function/Cardinality.h"
#include "Execution/Function/Case.h"
#include "Execution/Function/Cast.h"
#include "Execution/Function/CharJoin.h"
#include "Execution/Function/Choice.h"
#include "Execution/Function/Coalesce.h"
#include "Execution/Function/Concatenate.h"
#include "Execution/Function/Copy.h"
#include "Execution/Function/CurrentTimestamp.h"
#include "Execution/Function/Distinct.h"
#include "Execution/Function/ElementReference.h"
#include "Execution/Function/ExpandSynonym.h"
#include "Execution/Function/Invoke.h"
#include "Execution/Function/Kwic.h"
#include "Execution/Function/Length.h"
#include "Execution/Function/Normalize.h"
#include "Execution/Function/Overlay.h"
#include "Execution/Function/SubString.h"

#include "Execution/Externalizable.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_FUNCTION_USING

namespace
{
	// CONST
	//	_iSubModule --  submodule number
	//
	// NOTES
	const int _iSubModule = Externalizable::SubModule::Function;
}

// FUNCTION public
//	Function::Class::getClassInstance -- get instance from classid
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
	case Category::ArithmeticMonadic:
	case Category::ArithmeticDyadic:
		{
			pObject = Arithmetic::getInstance(eCategory);
			break;
		}
	case Category::Concatenate:
	case Category::ConcatenateBinary:
	case Category::ConcatenateArray:
	case Category::ConcatenateAnyType:
		{
			pObject = Concatenate::getInstance(eCategory);
			break;
		}
	case Category::SubString:
	case Category::SubStringBinary:
	case Category::SubStringArray:
	case Category::SubStringAnyType:
		{
			pObject = SubString::getInstance(eCategory);
			break;
		}
	case Category::Overlay:
	case Category::OverlayBinary:
	case Category::OverlayArray:
	case Category::OverlayAnyType:
		{
			pObject = Overlay::getInstance(eCategory);
			break;
		}
	case Category::CharLength:
	case Category::OctetLength:
		{
			pObject = Length::getInstance(eCategory);
			break;
		}
	case Category::Count:
	case Category::Sum:
	case Category::Avg:
	case Category::Max:
	case Category::Min:
	case Category::Distinct:
	case Category::BitSetCount:
	case Category::Word:
		{
			pObject = Aggregation::getInstance(eCategory);
			break;
		}
	case Category::Cast:
		{
			pObject = Cast::getInstance(eCategory);
			break;
		}
	case Category::CurrentTimestamp:
		{
			pObject = CurrentTimestamp::getInstance(eCategory);
			break;
		}
	case Category::Coalesce:
	case Category::CoalesceDefault:
	case Category::CoalesceNadic:
		{
			pObject = Coalesce::getInstance(eCategory);
			break;
		}
	case Category::Copy:
		{
			pObject = Copy::getInstance(eCategory);
			break;
		}
	case Category::Kwic:
		{
			pObject = Kwic::getInstance(eCategory);
			break;
		}
	case Category::ExpandSynonym:
		{
			pObject = ExpandSynonym::getInstance(eCategory);
			break;
		}
	case Category::Normalize:
		{
			pObject = Normalize::getInstance(eCategory);
			break;
		}
	case Category::ElementReference:
		{
			pObject = ElementReference::getInstance(eCategory);
			break;
		}
	case Category::Case:
		{
			pObject = Case::getInstance(eCategory);
			break;
		}
	case Category::Cardinality:
		{
			pObject = Cardinality::getInstance(eCategory);
			break;
		}
	case Category::CharJoin:
		{
			pObject = CharJoin::getInstance(eCategory);
			break;
		}
	case Category::Invoke:
		{
			pObject = Invoke::getInstance(eCategory);
			break;
		}
	case Category::GetMax:
		{
			pObject = Choice::getInstance(eCategory);
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
//	Function::Class::getClassID -- get classid
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
