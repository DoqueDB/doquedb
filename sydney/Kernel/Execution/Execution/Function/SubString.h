// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Function/SubString.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_FUNCTION_SUBSTRING_H
#define __SYDNEY_EXECUTION_FUNCTION_SUBSTRING_H

#include "Execution/Function/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Function::SubString -- function class for subString data
//
//	NOTES
//		This class is not constructed directly
class SubString
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef SubString This;

	struct String
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iDataID_,
							int iOptionID0_,
							int iOptionID1_,
							int iOutDataID_);
	};
	struct Binary
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iDataID_,
							int iOptionID0_,
							int iOptionID1_,
							int iOutDataID_);
	};
	struct Array
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iDataID_,
							int iOptionID0_,
							int iOptionID1_,
							int iOutDataID_);
	};

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Interface::IIterator* pIterator_,
						int iDataID_,
						int iOptionID0_,
						int iOptionID1_,
						int iOutDataID_);

	// destructor
	virtual ~SubString() {}

	// for serialize
	static This* getInstance(int iCategory_);

	// check substring argument (commonly used with overlay)
	static void checkArgument(int* piStart_,
							  int* piLength_,
							  int iMaxLength_);

protected:
	// constructor
	SubString() : Super() {}

private:
};

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_FUNCTION_SUBSTRING_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
