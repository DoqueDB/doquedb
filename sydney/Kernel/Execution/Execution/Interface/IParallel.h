// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IParallel.h --
// 
// Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IPARALLEL_H
#define __SYDNEY_EXECUTION_INTERFACE_IPARALLEL_H

#include "Execution/Interface/IAction.h"
#include "Execution/Action/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

///////////////////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Interface::IParallel -- Base class for the classes which represents parallel execution
//
//	NOTES
//		This class is not constructed directly
class IParallel
	: public IAction
{
public:
	typedef IAction Super;
	typedef IParallel This;

	// destructor
	virtual ~IParallel() {}

	// add new action list
	virtual int addList(Interface::IProgram& cProgram_) = 0;
	// get current list
	virtual Action::ActionList& getList(Interface::IProgram& cProgram_) = 0;
	// set returned data
	virtual void setReturnData(Interface::IProgram& cProgram_,
							   int iDataID_) = 0;

protected:
	// constructor
	IParallel() : Super() {}

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IPARALLEL_H

//
//	Copyright (c) 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
