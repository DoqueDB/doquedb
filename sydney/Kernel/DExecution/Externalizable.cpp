// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Externalizable.cpp --
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
const char moduleName[] = "DExecution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DExecution/Externalizable.h"
#include "DExecution/Action/Class.h"
#include "DExecution/Collection/Class.h"
#include "DExecution/Iterator/Class.h"
#include "DExecution/Operator/Class.h"
#include "DExecution/Predicate/Class.h"

#include "Common/Externalizable.h"

_SYDNEY_USING
_SYDNEY_DEXECUTION_USING

namespace
{
	// function table for getClassInstance
	typedef Common::Externalizable* (*Func)(int);
	Func _funcTable[] =
	{
		&Action::Class::getClassInstance,	 	// action
		&Collection::Class::getClassInstance,	// collection
		0, //&Interface::Class::getClassInstance,	// interface
		&Iterator::Class::getClassInstance,		// iterator
		&Operator::Class::getClassInstance,		// operator
		&Predicate::Class::getClassInstance,	// predicate
		0, //&Control::Class::getClassInstance,		// control
		0, //&Function::Class::getClassInstance,		// function
		0, //&Parallel::Class::getClassInstance,		// parallel
	};
}

// FUNCTION public
//	DExecution::Externalizable::getClassInstance -- get instance from classid
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
	int iBase = iClassID_ - Common::Externalizable::DExecutionClasses;
	if (iBase > 0 && iBase < SubModule::MaxValue) {
		return _funcTable[iBase / 100](iBase);
	}
	return 0;
}

// FUNCTION public
//	DExecution::Externalizable::getClassID -- get classid
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
	return iBase_ + Common::Externalizable::DExecutionClasses;
}

// FUNCTION public
//	DExecution::Externalizable::initialize -- initialize
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
			Common::Externalizable::DExecutionClasses,
			&Externalizable::getClassInstance);
}

// FUNCTION public
//	DExecution::Externalizable::terminate -- terminate
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
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
