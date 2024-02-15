// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V2Impl/Executor.cpp -- エグゼキュータ(v2)
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::V2Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/V2Impl/ExecutorImpl.h"

#include "Execution/Interface/IExecutor.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V2IMPL_BEGIN

///////////////////////////////////
// Execution::V2Impl::Executor

// FUNCTION public
//	Execution::V2Impl::Executor::execute -- execute
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
ExecutorImpl::
execute(Program& cProgram_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Execution::V2Impl::Executor::execute -- execute main
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
ExecutorImpl::
execute(Interface::IProgram& cProgram_)
{
	int n = cProgram_.getExecuteIteratorCount();
	if (n > 0) {
		initialize(cProgram_);

		try {
			for (int i = 0; i < n; ++i) {
				// get execute iterator
				Interface::IIterator* pIterator = cProgram_.getExecuteIterator(i);

				// do actions which are done once before iteration loop
				if (cProgram_.startUp(pIterator) != Action::Status::Break) {

					// main iteration loop
					while (cProgram_.next(pIterator)) {}
				}

				// do actions which are done once after iteration loop
				cProgram_.finish(pIterator);
			}
		} catch (...) {
			try {
				// clear objects converted by initialize
				terminate(cProgram_);
			} catch (...) {
				// ignore
			}
			_SYDNEY_RETHROW;
		}
		// clear objects converted by initialize
		terminate(cProgram_);
	}
}

// FUNCTION private
//	V2Impl::ExecutorImpl::initialize -- 
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

void
ExecutorImpl::
initialize(Interface::IProgram& cProgram_)
{
	// convert ID into objects
	int n = cProgram_.getExecuteIteratorCount();
	if (n > 0) {
		for (int i = 0; i < n; ++i) {
			// get execute iterator
			Interface::IIterator* pIterator = cProgram_.getExecuteIterator(i);
			cProgram_.initialize(pIterator);
		}
	}
}

// FUNCTION private
//	V2Impl::ExecutorImpl::terminate -- 
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

void
ExecutorImpl::
terminate(Interface::IProgram& cProgram_)
{
	// clear objects converted by initialize
	int n = cProgram_.getExecuteIteratorCount();
	if (n > 0) {
		for (int i = 0; i < n; ++i) {
			// get execute iterator
			Interface::IIterator* pIterator = cProgram_.getExecuteIterator(i);
			cProgram_.terminate(pIterator);
		}
	}
}

_SYDNEY_EXECUTION_V2IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
