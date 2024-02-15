// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Thread.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_THREAD_H
#define __SYDNEY_EXECUTION_ACTION_THREAD_H

#include "Execution/Interface/IObject.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Parallel/Program.h"

#include "Common/Thread.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Action::Thread -- Base class for the classes which represents thread
//
//	NOTES
//		This class is not constructed directly
class Thread
	: public Common::Thread,
	  public Interface::IObject
{
public:
	typedef Common::Thread Super;
	typedef Interface::IObject Object;
	typedef Thread This;

	using Super::create;
	using Super::join;

	// constructor
	static This* create(Interface::IProgram& cProgram_);

	// destructor
	~Thread() {destruct();}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);

	// add ActionID to be executed
	void addActionID(int iActionID_);

	// get action list to be executed
	Action::ActionList& getList();

	// initialize
	void initialize(Interface::IProgram& cProgram_);
	void terminate(Interface::IProgram& cProgram_);

	// start thread
	void start(Interface::IProgram& cProgram_);
	// join thread
	void join(Interface::IProgram& cProgram_);
	// join thread
	void checkExitStatus(Interface::IProgram& cProgram_);

	// for serialize
	static This* getInstance(int iCategory_);

///////////////////////////////
// Common::Externalizable
	int getClassID() const;

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

protected:
	// constructor
	Thread()
		: m_cActionList(),
		  m_cProgram()
	{}

	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);

////////////////////////
// Common::Thread::
	virtual void runnable();

private:
	void destruct();

	Action::ActionList m_cActionList;
	Parallel::Program m_cProgram;
};

///////////////////////////////////
// CLASS
//	Action::ThreadHolder --
//
// NOTES

class ThreadHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef ThreadHolder This;

	ThreadHolder()
		: Super(),
		  m_iID(-1),
		  m_pThread(0)
	{}
	ThreadHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pThread(0)
	{}
	~ThreadHolder() {}

	void explain(Opt::Environment* pEnvironment_,
				 Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);

	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize Thread instance
	void initialize(Interface::IProgram& cProgram_);
	// terminate Thread instance
	void terminate(Interface::IProgram& cProgram_);

	// -> operator
	Thread* operator->() const {return m_pThread;}

	// accessor
	Thread* get() const {return m_pThread;}
	bool isInitialized() {return m_pThread != 0;}

	// serializer
	void serialize(ModArchive& archiver_);

protected:
private:
	int m_iID;
	Thread* m_pThread;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_THREAD_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
