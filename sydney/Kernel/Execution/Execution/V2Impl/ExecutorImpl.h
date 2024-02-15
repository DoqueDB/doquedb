// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// V2Impl/ExecutorImpl.h -- エグゼキュータ(v2)
// 
// Copyright (c) 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_V2IMPL_EXECUTORIMPL_H
#define __SYDNEY_EXECUTION_V2IMPL_EXECUTORIMPL_H

#include "Execution/V2Impl/Module.h"
#include "Execution/Declaration.h"
#include "Execution/Interface/IExecutor.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_V2IMPL_BEGIN

// CLASS
//	Execution::V2Impl::ExecutorImpl -- implementation class for V2
//
// NOTES
class ExecutorImpl
	: public Interface::IExecutor
{
public:
	typedef ExecutorImpl This;
	typedef Interface::IExecutor Super;

	// constructor
	ExecutorImpl() : Super() {}
	// destructor
	~ExecutorImpl() {}

	// V1 implementation
	virtual void execute(Program& cProgram_);
	// V2 implementation
	virtual void execute(Interface::IProgram& cProgram_);
protected:
private:
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);
};

_SYDNEY_EXECUTION_V2IMPL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_V2IMPL_EXECUTORIMPL_H

//
//	Copyright (c) 2008, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
