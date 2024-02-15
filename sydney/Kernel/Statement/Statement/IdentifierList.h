// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::IdentifierList -- IdentifierList
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

#ifndef __SYDNEY_STATEMENT_IDENTIFIERLIST_H
#define __SYDNEY_STATEMENT_IDENTIFIERLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
	
//	CLASS
//	IdentifierList -- IdentifierList
//
//	NOTES

class SYD_STATEMENT_FUNCTION IdentifierList : public Statement::ObjectList
{
public:
	// constructor
	IdentifierList();
	// constructor(2)
	explicit IdentifierList(Identifier* pIdentifier_);

	// accessor
	Identifier* getIdentifierAt(int iAt_) const;

///////////////////
// ObjectList::
//	Object* getAt(int iAt_) const;
//	void setAt(int iAt_, Object* pObject_);
//	int getCount() const;
//	void append(Object* pObject_);

	//create clone
	Object* copy() const;

private:
	// can't copy implicitly
	IdentifierList& operator=(const IdentifierList& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_IDENTIFIERLIST_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
