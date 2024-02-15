// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Externalizable.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Externalizable.h"
#include "Execution/Action/Class.h"
#include "Execution/Collection/Class.h"
#include "Execution/Control/Class.h"
#include "Execution/Function/Class.h"
#include "Execution/Interface/Class.h"
#include "Execution/Iterator/Class.h"
#include "Execution/Operator/Class.h"
#include "Execution/Parallel/Class.h"
#include "Execution/Predicate/Class.h"

#include "Common/Externalizable.h"

#include "Opt/LogData.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING

namespace
{
	// function table for getClassInstance
	typedef Common::Externalizable* (*Func)(int);
	Func _funcTable[] =
	{
		&Action::Class::getClassInstance,		// action
		&Collection::Class::getClassInstance,	// collection
		&Interface::Class::getClassInstance,	// interface
		&Iterator::Class::getClassInstance,		// iterator
		&Operator::Class::getClassInstance,		// operator
		&Predicate::Class::getClassInstance,	// predicate
		&Control::Class::getClassInstance,		// control
		&Function::Class::getClassInstance,		// function
		&Parallel::Class::getClassInstance,		// parallel
	};
}

// FUNCTION public
//	Execution::Externalizable::getClassInstance -- get instance from classid
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
Externalizable::
getClassInstance(int iClassID_)
{
	int iBase = iClassID_ - Common::Externalizable::ExecutionV2Classes;
	if (iBase > 0 && iBase < SubModule::MaxValue) {
		return _funcTable[iBase / 100](iBase);
	}
	return 0;
}

// FUNCTION public
//	Execution::Externalizable::getClassInstance0 -- get instance from classid
//
// NOTES
//	This method is used for backward compatibility(~v14)
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
Externalizable::
getClassInstance0(int iClassID_)
{
	switch (iClassID_ - Common::Externalizable::ExecutionClasses) {
	case Category::LogData:
		{
			// use recent class
			return new Opt::LogData();
		}
	default:
		{
			break;
		}
	}
	return 0;
}

// FUNCTION public
//	Execution::Externalizable::getClassInstance1 -- get instance from classid
//
// NOTES
//	This method is used for backward compatibility(v15~)
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
Externalizable::
getClassInstance1(int iClassID_)
{
	switch (iClassID_ - Common::Externalizable::PlanClasses) {
	case Category::LogData:
		{
			// use recent class
			return new Opt::LogData();
		}
	default:
		{
			break;
		}
	}
	return 0;
}

// FUNCTION public
//	Execution::Externalizable::getClassID -- get classid
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
Externalizable::
getClassID(int iBase_)
{
	return iBase_ + Common::Externalizable::ExecutionV2Classes;
}

// FUNCTION public
//	Execution::Externalizable::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Externalizable::
initialize()
{
	// register getclassinstance
	Common::Externalizable::insertFunction(
			Common::Externalizable::ExecutionV2Classes,
			&Externalizable::getClassInstance);
	Common::Externalizable::insertFunction(
			Common::Externalizable::ExecutionClasses,
			&Externalizable::getClassInstance0);
	Common::Externalizable::insertFunction(
			Common::Externalizable::PlanClasses,
			&Externalizable::getClassInstance1);
}

// FUNCTION public
//	Execution::Externalizable::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Externalizable::
terminate()
{
	; // do nothing
}

//
//	Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
