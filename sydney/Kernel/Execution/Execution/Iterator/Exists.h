// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Exists.h --
// 
// Copyright (c) 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_EXISTS_H
#define __SYDNEY_EXECUTION_ITERATOR_EXISTS_H

#include "Execution/Iterator/Base.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Iterator::Exists -- iterator class for exists
//
//	NOTES
//		This class is not constructed directly
class Exists
	: public Base
{
public:
	typedef Base Super;
	typedef Exists This;

	// constructor
	static This* create(Interface::IProgram& cProgram_);

	struct Not
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_);
	};

	// destructor
	virtual ~Exists() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Exists() : Super() {}

private:
};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_EXISTS_H

//
//	Copyright (c) 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
