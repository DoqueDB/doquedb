// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExplainOption.cpp --
// 
// Copyright (c) 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/ExplainOption.h"
#include "Statement/Hint.h"
#include "Statement/Type.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// members' index in m_vecpElements
	enum {
		f_Hint,
		f__end_index
	};
}

// FUNCTION public
//	Statement::ExplainOption::ExplainOption -- constructor
//
// NOTES
//
// ARGUMENTS
//	ExplainOption::Value iOption_
//	Hint* pHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ExplainOption::
ExplainOption(Value iOption_, Hint* pHint_)
	: Object(ObjectType::ExplainOption, f__end_index)
{
	setOption(iOption_);
	setHint(pHint_);
}

// FUNCTION public
//	Statement::ExplainOption::ExplainOption -- constructor
//
// NOTES
//
// ARGUMENTS
//	const ExplainOption& cStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ExplainOption::
ExplainOption(const ExplainOption& cOption_)
	: Object(cOption_),
	  m_iOption(cOption_.m_iOption)
{
}

// FUNCTION public
//	Statement::ExplainOption::~ExplainOption -- destructor
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
ExplainOption::
~ExplainOption()
{
}

// FUNCTION public
//	Statement::ExplainOption::getHint -- get hint object
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Statement::Hint*
//
// EXCEPTIONS

Hint*
ExplainOption::
getHint() const
{
	return _SYDNEY_DYNAMIC_CAST(Hint*, getElement(f_Hint, ObjectType::Hint));
}

// FUNCTION public
//	Statement::ExplainOption::setHint -- set hint object
//
// NOTES
//
// ARGUMENTS
//	Hint* pHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ExplainOption::
setHint(Hint* pHint_)
{
	setElement(f_Hint, pHint_);
}

// FUNCTION public
//	Statement::ExplainOption::copy -- copy object
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
ExplainOption::
copy() const
{
	return new ExplainOption(*this);
}

// FUNCTION public
//	Statement::ExplainOption::toSQLStatement -- get SQL statement
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
ExplainOption::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	if (m_iOption & Execute) {
		cStream << " EXECUTE";
	}
	if (Hint* pHint = getHint()) {
		cStream << " " << pHint->toSQLStatement(bForCascade_);
	}
	return cStream.getString();
}

// FUNCTION public
//	Statement::ExplainOption::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ExplainOption::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_iOption);
}

//
//	Copyright (c) 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
