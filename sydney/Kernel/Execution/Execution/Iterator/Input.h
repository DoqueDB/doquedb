// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Input.h --
// 
// Copyright (c) 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_INPUT_H
#define __SYDNEY_EXECUTION_ITERATOR_INPUT_H

#include "Execution/Iterator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Iterator/Base.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Iterator::Input -- iterator class for input from collection
//
//	NOTES
//		This class is not constructed directly
class Input
	: public Base
{
public:
	typedef Base Super;
	typedef Input This;

	// constructor
	static This* create(Interface::IProgram& cProgram_);

	static This* create(Interface::IProgram& cProgram_,
						int iCollectionID_);	

	struct Thread
	{
		static This* create(Interface::IProgram& cProgram_,
							int iThreadID_);
	};

	// destructor
	virtual ~Input() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Input() : Super() {}

private:
};

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_INPUT_H

//
//	Copyright (c) 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
