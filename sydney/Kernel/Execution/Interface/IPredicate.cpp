// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/Predicate.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Interface";
}

#include "SyDefault.h"

#include "Execution/Interface/IPredicate.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_INTERFACE_USING

///////////////////////////////////////////////
// Execution::Interface::IPredicate::Boolean

// FUNCTION public
//	Interface::IPredicate::Boolean::boolAnd -- 
//
// NOTES
//
// ARGUMENTS
//	Value v1_
//	Value v2_
//	
// RETURN
//	IPredicate::Boolean::Value
//
// EXCEPTIONS

//static
IPredicate::Boolean::Value
IPredicate::Boolean::
boolAnd(Value v1_, Value v2_)
{
#define F False
#define T True
#define U Unknown
#define N NeverTrue
	const Value _AndTable[ValueNum][ValueNum] =
	{//  f  t  u  n
		{F, F, F, N},					// false
		{F, T, U, N},					// true
		{F, U, U, N},					// unknown
		{N, N, N, N},					// never true
	};
	const Value _AndTableNoUnknown[ValueNum][ValueNum] =
	{//  f  t  u  n
		{F, F, F, N},					// false
		{F, T, F, N},					// true
		{F, F, F, N},					// unknown
		{N, N, N, N},					// never true
	};
#undef F
#undef T
#undef U
#undef N
	if (Opt::Configuration::getNoUnknown().get()) {
		return _AndTableNoUnknown[v1_][v2_];
	} else {
		return _AndTable[v1_][v2_];
	}
}

// FUNCTION public
//	Interface::IPredicate::Boolean::boolOr -- 
//
// NOTES
//
// ARGUMENTS
//	Value v1_
//	Value v2_
//	
// RETURN
//	IPredicate::Boolean::Value
//
// EXCEPTIONS

//static
IPredicate::Boolean::Value
IPredicate::Boolean::
boolOr(Value v1_, Value v2_)
{
#define F False
#define T True
#define U Unknown
#define N NeverTrue
	const Value _OrTable[ValueNum][ValueNum] =
	{//  f  t  u  n
		{F, T, U, F},					// false
		{T, T, T, T},					// true
		{U, T, U, U},					// unknown
		{F, T, U, N},					// never true
	};
	const Value _OrTableNoUnknown[ValueNum][ValueNum] =
	{//  f  t  u  n
		{F, T, F, F},					// false
		{T, T, T, T},					// true
		{F, T, F, F},					// unknown
		{F, T, F, N},					// never true
	};
#undef F
#undef T
#undef U
#undef N
	if (Opt::Configuration::getNoUnknown().get()) {
		return _OrTableNoUnknown[v1_][v2_];
	} else {
		return _OrTable[v1_][v2_];
	}
}

// FUNCTION public
//	Interface::IPredicate::Boolean::boolNot -- 
//
// NOTES
//
// ARGUMENTS
//	Value v_
//	
// RETURN
//	IPredicate::Boolean::Value
//
// EXCEPTIONS

//static
IPredicate::Boolean::Value
IPredicate::Boolean::
boolNot(Value v_)
{
	const Value _NotTable[ValueNum] =
	{
		True,							// false
		False,							// true
		Unknown,						// unknown
		True,							// never true
	};
	const Value _NotTableNoUnknown[ValueNum] =
	{
		True,							// false
		False,							// true
		True,							// unknown
		True,							// never true
	};
	if (Opt::Configuration::getNoUnknown().get()) {
		return _NotTableNoUnknown[v_];
	} else {
		return _NotTable[v_];
	}
}

///////////////////////////////////////////////
// Execution::Interface::IPredicate::

// FUNCTION public
//	Interface::IPredicate::checkByData -- get predicate result for a specified data
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
IPredicate::Boolean::Value
IPredicate::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	// default: not supported
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
