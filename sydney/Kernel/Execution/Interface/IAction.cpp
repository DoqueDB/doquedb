// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/Action.cpp --
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
const char moduleName[] = "Execution::Interface";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Interface/IAction.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_INTERFACE_USING

//////////////////////////////////////////
// Execution::Interface::IAction

// FUNCTION public
//	Interface::IAction::startUp -- do action at startup
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
IAction::
startUp(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// by default, do nothing
	return Action::Status::Success;
}

// FUNCTION public
//	Interface::IAction::accumulate -- do accumulation
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
IAction::
accumulate(Interface::IProgram& cProgram_,
		   Action::ActionList& cActionList_)
{
	// by default, execute
	execute(cProgram_, cActionList_);
	undone(cProgram_);
}

// FUNCTION protected
//	Interface::IAction::registerToProgram -- register to program
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IAction::
registerToProgram(Interface::IProgram& cProgram_,
				  Interface::IIterator* pIterator_)
{
	// Instance ID is obtained by registerAction method.
	setID(cProgram_.registerAction(this));
	if (pIterator_) {
		pIterator_->registerAction(getID());
	}
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
