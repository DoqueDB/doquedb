// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StartExplainStatement.cpp --
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Statement/StartExplainStatement.h"
#include "Statement/ExplainOption.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// members' index in m_vecpElements
	enum {
		f_Option,
		f__end_index
	};
}

// FUNCTION public
//	Statement::StartExplainStatement::StartExplainStatement -- constructor
//
// NOTES
//
// ARGUMENTS
//	ExplainOption* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

StartExplainStatement::
StartExplainStatement(ExplainOption* pOption_)
	: Object(ObjectType::StartExplainStatement, f__end_index, Object::Optimize)
{
	setOption(pOption_);
}

// FUNCTION public
//	Statement::StartExplainStatement::~StartExplainStatement -- destructor
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

//virtual
StartExplainStatement::
~StartExplainStatement()
{
}

// FUNCTION public
//	Statement::StartExplainStatement::getOption -- get explain option
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ExplainOption*
//
// EXCEPTIONS

ExplainOption*
StartExplainStatement::
getOption() const
{
	return _SYDNEY_DYNAMIC_CAST(ExplainOption*, getElement(f_Option, ObjectType::ExplainOption));
}

// FUNCTION public
//	Statement::StartExplainStatement::setOption -- set explain option
//
// NOTES
//
// ARGUMENTS
//	ExplainOption* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
StartExplainStatement::
setOption(ExplainOption* pOption_)
{
	setElement(f_Option, pOption_);
}

// FUNCTION public
//	Statement::StartExplainStatement::copy -- copy object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
StartExplainStatement::
copy() const
{
	return new StartExplainStatement(*this);
}

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
