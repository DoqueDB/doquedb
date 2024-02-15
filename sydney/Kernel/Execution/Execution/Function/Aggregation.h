// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Function/Aggregation.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_FUNCTION_AGGREGATION_H
#define __SYDNEY_EXECUTION_FUNCTION_AGGREGATION_H

#include "Execution/Function/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Function::Aggregation -- function class for executing other iterator
//
//	NOTES
//		This class is not constructed directly
class Aggregation
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef Aggregation This;

	struct Count
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};

	struct BitSetCount
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	
	struct Sum
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	struct Avg
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	struct Max
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	struct Min
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	struct Distinct
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};
	struct Word
	{
		// constructor
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iOperandID_,
							int iDataID_);
	};	

	// destructor
	virtual ~Aggregation() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Aggregation() : Super() {}

private:
};

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_FUNCTION_AGGREGATION_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
