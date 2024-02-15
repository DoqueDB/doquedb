// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IExecutor.h --
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IEXECUTOR_H
#define __SYDNEY_EXECUTION_INTERFACE_IEXECUTOR_H

#include "Execution/Interface/Module.h"
#include "Execution/Declaration.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

// CLASS
//	Execution::Interface::IExecutor -- interface of execution class
//
// NOTES
class IExecutor
	: public Common::Object
{
public:
	typedef IExecutor This;
	typedef Common::Object Super;

	virtual ~IExecutor() {}

	// V1 implementation
	virtual void execute(Program& cProgram_) = 0;
	// V2 implementation
	virtual void execute(Interface::IProgram& cProgram_) = 0;

protected:
	IExecutor() : Super() {}
private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IEXECUTOR_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
