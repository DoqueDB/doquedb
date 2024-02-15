// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Control/Conditional.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_CONTROL_CONDITIONAL_H
#define __SYDNEY_EXECUTION_CONTROL_CONDITIONAL_H

#include "Execution/Control/Base.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_CONTROL_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Control::Conditional -- control class for conditional
//
//	NOTES
//		This class is not constructed directly
class Conditional
	: public Base
{
public:
	typedef Base Super;
	typedef Conditional This;

	struct If
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iPredicateID_);
	};
	struct Unless
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iPredicateID_);
	};

	// destructor
	virtual ~Conditional() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Conditional() : Super() {}

private:
};

_SYDNEY_EXECUTION_CONTROL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_CONTROL_CONDITIONAL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
