// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Operator/Locker.h --
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

#ifndef __SYDNEY_EXECUTION_OPERATOR_LOCKER_H
#define __SYDNEY_EXECUTION_OPERATOR_LOCKER_H

#include "Execution/Operator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "Common/Object.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Table;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Operator::Locker -- operator class for lock/unlock
//
//	NOTES
//		This class is not constructed directly
class Locker
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef Locker This;

	struct Table
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const Action::LockerArgument& cArgument_);
	};
	struct ConvertTable
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const Action::LockerArgument& cArgument_,
							const Action::LockerArgument& cPrevArgument_);
	};
	struct Tuple
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const Action::LockerArgument& cArgument_,
							int iRowIDID_);
	};
	struct BitSet
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const Action::LockerArgument& cArgument_,
							int iBitSetID_);
	};
	struct ConvertTuple
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const Action::LockerArgument& cArgument_,
							const Action::LockerArgument& cPrevArgument_,
							int iRowIDID_);
	};
	struct UnlockTuple
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iLockerID_);
	};

	// destructor
	virtual ~Locker() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Locker() : Super() {}

private:
};

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_OPERATOR_LOCKER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
