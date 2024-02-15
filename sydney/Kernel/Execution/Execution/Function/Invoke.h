// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Function/Invoke.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_FUNCTION_INVOKE_H
#define __SYDNEY_EXECUTION_FUNCTION_INVOKE_H

#include "Execution/Function/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IAction.h"

#include "Common/DataOperation.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Function;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Function::Invoke -- function class for invoke data
//
//	NOTES
//		This class is not constructed directly
class Invoke
	: public Interface::IAction
{
public:
	typedef Interface::IAction Super;
	typedef Invoke This;

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Interface::IIterator* pIterator_,
						const Schema::Function* pSchemaFunction_,
						int iOperandDataID_,
						int iOutDataID_);

	// destructor
	virtual ~Invoke() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Invoke() : Super() {}

private:
};

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_FUNCTION_INVOKE_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
