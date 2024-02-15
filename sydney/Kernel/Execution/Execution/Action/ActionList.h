// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/ActionList.h --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_ACTIONLIST_H
#define __SYDNEY_EXECUTION_ACTION_ACTIONLIST_H

#include "Execution/Action/Module.h"
#include "Execution/Action/Status.h"

#include "Execution/Declaration.h"

#include "Common/Object.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

class ModArchive;

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::ActionList -- wrapping class for actionList access in various actions
//
//	NOTES

class ActionList
	: public Common::Object
{
public:
	// constructor
	ActionList()
		: m_bSetNext(false), m_iNext(0) {}
	// descructor
	~ActionList() {}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);
	Action::Status::Value execute(Interface::IProgram& cProgram_);
	void accumulate(Interface::IProgram& cProgram_);
	void finish(Interface::IProgram& cProgram_);
	void setPointer(int iPosition_);

	// serialize
	void serialize(ModArchive& archiver_);

	// accessor
	SIZE getSize() {return m_vecID.GETSIZE();}
	void addID(int iID_) {m_vecID.PUSHBACK(iID_);}
	void insertID(int iID_) {m_vecID.PUSHFRONT(iID_);}

protected:
private:
	Action::Status::Value startUp(Interface::IProgram& cProgram_);

	VECTOR<int> m_vecID;
	VECTOR<Interface::IAction*> m_vecAction;

	bool m_bSetNext;
	int m_iNext;
};


_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_ACTIONLIST_H

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
