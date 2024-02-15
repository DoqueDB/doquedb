// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Action/ServerAccess.h --
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

#ifndef __SYDNEY_DEXECUTION_ACTION_SERVERACCESS_H
#define __SYDNEY_DEXECUTION_ACTION_SERVERACCESS_H

#include "DExecution/Action/Module.h"

#include "Common/Object.h"

#include "Execution/Declaration.h"
#include "Execution/Interface/IAction.h"

#include "DServer/Declaration.h"

#include "Opt/Environment.h"

#include "Schema/Cascade.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	DExecution::Action::ServerAccess -- wrapping class for server access in various actions
//
//	NOTES
//		IAction is used as the superclass so that IProgram can register this class.

class ServerAccess
	: public Execution::Interface::IAction
{
public:
	typedef Execution::Interface::IAction Super;
	typedef ServerAccess This;

	// constructor
	static This* create(Execution::Interface::IProgram& cProgram_,
						Schema::Cascade* pSchemaCascade_,
						const STRING& cstrSQL_);

	static This* create(Execution::Interface::IProgram& cProgram_,
						Schema::Cascade* pSchemaCascade_,
						int iDataID_);

	// destructor
	~ServerAccess() {}

	// execute query
	virtual DServer::ResultSet* executeQuery(Execution::Interface::IProgram& cProgram_);

	virtual DServer::ResultSet* getResultSet(Execution::Interface::IProgram& cProgram_);

	virtual Execution::Action::Status::Value
	execute(Execution::Interface::IProgram& cProgram_);

	virtual void resetResultSet() {m_pResultSet = 0;}
	
	
	// open
	void open(Execution::Interface::IProgram& cProgram_);
	// prepare
	bool prepare(Execution::Interface::IProgram& cProgram_);
	// execute prepared
	DServer::ResultSet* executePrepare(Execution::Interface::IProgram& cProgram_,
									   const Common::DataArrayData& cData_);

	// accessor
	const STRING& getCascadeName() {return m_cCascadeName;}

///////////////////////////////////////
// Execution::Interface::IAction::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_);
	virtual void initialize(Execution::Interface::IProgram& cProgram_);
	virtual void terminate(Execution::Interface::IProgram& cProgram_);

	virtual Execution::Action::Status::Value
				execute(Execution::Interface::IProgram& cProgram_,
						Execution::Action::ActionList& cActionList_);
	virtual void finish(Execution::Interface::IProgram& cProgram_);
	
	virtual void reset(Execution::Interface::IProgram& cProgram_);

	// for serialize
	static This* getInstance(int iCategory_);



///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

protected:
	//constructor
	ServerAccess()
		: Super(),
		  m_cCascadeName(),
		  m_pSession(0),
		  m_pPrepare(0),
		  m_pResultSet(0)
	{}
	ServerAccess(Schema::Cascade* pSchemaCascade_)
		: Super(),
		  m_cCascadeName(pSchemaCascade_->getName()),
		  m_pSession(0),
		  m_pPrepare(0),
		  m_pResultSet(0)
	{}
	
	virtual const STRING& getSQL() = 0;

	virtual void clearSQL() = 0;

	virtual const Common::DataArrayData& getParameter() = 0;

	virtual void explainThis(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_) = 0;
	
private:
	// close
	void close(Execution::Interface::IProgram& cProgram_);

	// register to program
	void registerToProgram(Execution::Interface::IProgram& cProgram_);

	Schema::Object::Name m_cCascadeName;
	DServer::Session* m_pSession;
	DServer::PrepareStatement* m_pPrepare;
	DServer::ResultSet* m_pResultSet;
};

///////////////////////////////////
// CLASS
//	Action::ServerAccessHolder --
//
// NOTES

class ServerAccessHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef ServerAccessHolder This;

	ServerAccessHolder()
		: Super(),
		  m_iID(-1),
		  m_pServerAccess(0)
	{}
	ServerAccessHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pServerAccess(0)
	{}
	~ServerAccessHolder() {}

	void explain(Opt::Environment* pEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);

	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize ServerAccess instance
	void initialize(Execution::Interface::IProgram& cProgram_);
	// terminate ServerAccess instance
	void terminate(Execution::Interface::IProgram& cProgram_);

	// -> operator
	ServerAccess* operator->() const {return m_pServerAccess;}

	// accessor
	ServerAccess* get() const {return m_pServerAccess;}
	bool isInitialized() {return m_pServerAccess != 0;}

	// serializer
	void serialize(ModArchive& archiver_);

protected:
private:
	int m_iID;
	ServerAccess* m_pServerAccess;
};

_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_ACTION_SERVERACCESS_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
