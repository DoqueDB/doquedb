// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExplainStatement.cpp --
// 
// Copyright (c) 2007, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/ExplainStatement.h"
#include "Statement/ExplainOption.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// members' index in m_vecpElements
	enum {
		f_Statement,
		f_Option,
		f__end_index
	};
}

// FUNCTION public
//	Statement::ExplainStatement::ExplainStatement -- constructor
//
// NOTES
//
// ARGUMENTS
//	Object* pStatement_
//	ExplainOption* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ExplainStatement::
ExplainStatement(Object* pStatement_, ExplainOption* pOption_)
	: Object(ObjectType::ExplainStatement, f__end_index, Object::Optimize)
{
	setStatement(pStatement_);
	setOption(pOption_);
}

// FUNCTION public
//	Statement::ExplainStatement::~ExplainStatement -- destructor
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
ExplainStatement::
~ExplainStatement()
{
}

// FUNCTION public
//	Statement::ExplainStatement::getStatement -- get statement object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Statement::Object*
//
// EXCEPTIONS

Statement::Object*
ExplainStatement::
getStatement() const
{
	return m_vecpElements[f_Statement];
}

// FUNCTION public
//	Statement::ExplainStatement::getOption -- get explain option
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
ExplainStatement::
getOption() const
{
	return _SYDNEY_DYNAMIC_CAST(ExplainOption*, getElement(f_Option, ObjectType::ExplainOption));
}

// FUNCTION public
//	Statement::ExplainStatement::setStatement -- set statement object
//
// NOTES
//
// ARGUMENTS
//	Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ExplainStatement::
setStatement(Object* pStatement_)
{
	setElement(f_Statement, pStatement_);
}

// FUNCTION public
//	Statement::ExplainStatement::setOption -- set explain option
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
ExplainStatement::
setOption(ExplainOption* pOption_)
{
	setElement(f_Option, pOption_);
}

// FUNCTION public
//	Statement::ExplainStatement::copy -- copy object
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
ExplainStatement::
copy() const
{
	return new ExplainStatement(*this);
}

// FUNCTION public
//	Statement::ExplainStatement::toSQLStatement -- get SQL statement
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ExplainStatement::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << "EXPLAIN " << getOption()->toSQLStatement(bForCascade_)
			<< " " << getStatement()->toSQLStatement(bForCascade_);
	return cStream.getString();
}

//
//	Copyright (c) 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
