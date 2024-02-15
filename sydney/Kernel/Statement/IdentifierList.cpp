// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IdentifierList.cpp -- IdentifierList
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
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/IdentifierList.h"
#include "Statement/Identifier.h"

#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

// FUNCTION public
//	Statement::IdentifierList::IdentifierList -- 
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

IdentifierList::IdentifierList()
	: ObjectList(ObjectType::IdentifierList)
{
}

// FUNCTION public
//	Statement::IdentifierList::IdentifierList -- 
//
// NOTES
//
// ARGUMENTS
//	Identifier* pIdentifier_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IdentifierList::IdentifierList(Identifier* pIdentifier_)
	: ObjectList(ObjectType::IdentifierList)
{
	// add identifier
	append(pIdentifier_);
}

// FUNCTION public
//	Statement::IdentifierList::getIdentifierAt -- 
//
// NOTES
//
// ARGUMENTS
//	int iAt_
//	
// RETURN
//	Identifier*
//
// EXCEPTIONS

Identifier*
IdentifierList::getIdentifierAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(Identifier*, getElement(iAt_, ObjectType::Identifier));
}

// FUNCTION public
//	Statement::IdentifierList::copy -- 
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
IdentifierList::copy() const
{
	return new IdentifierList(*this);
}

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
