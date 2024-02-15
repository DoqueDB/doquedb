// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V2Executor.cpp -- エグゼキュータ(v2)
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/V2Executor.h"
#include "Execution/Program.h"
#include "Execution/Interface/IExecutor.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Message.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN

///////////////////////////////////
// Execution::V2Executor

// FUNCTION public
//	Execution::V2Executor::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Program& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
V2Executor::
execute(Program& cProgram_)
{
	execute(*cProgram_.getInterface());
}

// FUNCTION private
//	Execution::V2Executor::execute -- execute main
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
V2Executor::
execute(Interface::IProgram& cProgram_)
{
	Interface::IIterator* pIterator = cProgram_.getIterator();
	cProgram_.initialize(pIterator);
	try {
		cProgram_.startUp(pIterator);
		while (cProgram_.next(pIterator)) {}
		cProgram_.finish(pIterator);
	} catch (...) {
		try {
			cProgram_.terminate(pIterator);
		} catch (...) {
			// ignore
		}
		_SYDNEY_RETHROW;
	}
	cProgram_.terminate(pIterator);
}

_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
