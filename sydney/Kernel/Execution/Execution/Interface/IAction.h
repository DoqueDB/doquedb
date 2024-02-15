// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IAction.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IACTION_H
#define __SYDNEY_EXECUTION_INTERFACE_IACTION_H

#include "Execution/Interface/IObject.h"
#include "Execution/Action/Status.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Interface::IAction -- Base class for the classes which represents action
//
//	NOTES
//		This class is not constructed directly
class IAction
	: public IObject
{
public:
	typedef IObject Super;
	typedef IAction This;

	// destructor
	virtual ~IAction() {}

	// output explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_) = 0;

	// initialize
	virtual void initialize(Interface::IProgram& cProgram_) = 0;
	virtual void terminate(Interface::IProgram& cProgram_) = 0;

	// do action at startup
	virtual Action::Status::Value
				startUp(Interface::IProgram& cProgram_,
						Action::ActionList& cActionList_);
	// do action
	virtual Action::Status::Value
				execute(Interface::IProgram& cProgram_,
						Action::ActionList& cActionList_) = 0;
	// do accumulation
	virtual void accumulate(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	// do after all iteration
	virtual void finish(Interface::IProgram& cProgram_) = 0;
	// do when iteration is reset
	virtual void reset(Interface::IProgram& cProgram_) = 0;

	// do when iteration proceed
	virtual void undone(Interface::IProgram& cProgram_) {m_bDone = false;}

protected:
	// constructor
	IAction() : Super(), m_bDone(false) {}
	// register to program
	void registerToProgram(Interface::IProgram& cProgram_,
						   Interface::IIterator* pIterator_);

	void done() {m_bDone = true;}
	bool isDone() {return m_bDone;}

private:
	bool m_bDone;
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IACTION_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
