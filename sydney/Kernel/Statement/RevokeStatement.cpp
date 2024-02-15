// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RevokeStatement.cpp --
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

#include "Statement/IdentifierList.h"
#include "Statement/RevokeStatement.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// member's index in m_vecpElements
	enum {
		f_Role,
		f_Grantee,
		f__end_index
	};
}

// FUNCTION public
//	Statement::RevokeStatement::RevokeStatement -- 
//
// NOTES
//
// ARGUMENTS
//	IdentifierList* pRole_
//	IdentifierList* pGrantee_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

RevokeStatement::RevokeStatement(IdentifierList* pRole_, IdentifierList* pGrantee_)
	: Object(ObjectType::RevokeStatement, f__end_index)
{
	setRole(pRole_);
	setGrantee(pGrantee_);
}

// FUNCTION public
//	Statement::RevokeStatement::copy -- create clone
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
RevokeStatement::copy() const
{
	return new RevokeStatement(*this);
}

// FUNCTION public
//	Statement::RevokeStatement::getRole -- get Role
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	IdentifierList*
//
// EXCEPTIONS

IdentifierList*
RevokeStatement::
getRole() const
{
	return _SYDNEY_DYNAMIC_CAST(IdentifierList*, getElement(f_Role, ObjectType::IdentifierList));
}

// FUNCTION public
//	Statement::RevokeStatement::setRole -- set Role
//
// NOTES
//
// ARGUMENTS
//	IdentifierList* pRole_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RevokeStatement::
setRole(IdentifierList* pRole_)
{
	setElement(f_Role, pRole_);
}

// FUNCTION public
//	Statement::RevokeStatement::getGrantee -- get Grantee list
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	IdentifierList*
//
// EXCEPTIONS

IdentifierList*
RevokeStatement::
getGrantee() const
{
	return _SYDNEY_DYNAMIC_CAST(IdentifierList*, getElement(f_Grantee, ObjectType::IdentifierList));
}

// FUNCTION public
//	Statement::RevokeStatement::setGrantee -- set Grantee list
//
// NOTES
//
// ARGUMENTS
//	IdentifierList* pGrantee_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
RevokeStatement::
setGrantee(IdentifierList* pGrantee_)
{
	setElement(f_Grantee, pGrantee_);
}

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
