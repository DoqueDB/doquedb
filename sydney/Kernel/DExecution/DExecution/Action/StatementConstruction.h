// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Action/StatementConstruction.h --
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

#ifndef __SYDNEY_DEXECUTION_ACTION_SQLGENERATOR_H
#define __SYDNEY_DEXECUTION_ACTION_SQLGENERATOR_H

#include "SyDynamicCast.h"

#include "DExecution/Action/Module.h"

#include "Common/Object.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/ActionList.h"

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
//	DExecution::Action::StatementConstruction -- wrapping class for server access in various actions
//
//	NOTES
//		IAction is used as the superclass so that IProgram can register this class.

class StatementConstruction
	: public Execution::Interface::IAction
{
public:
	typedef Execution::Interface::IAction Super;
	typedef StatementConstruction This;

	enum DataType {
		STATEMENT,
		DATA_ID,
		PLACEHOLDER,
		ARRAY_PLACEHOLDER
	};
	
	// constructor
	static This* create(Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						int iSqlID_,
						bool bConcatinate_ = false);

	// destructor
	virtual ~StatementConstruction() {}


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

	//////////////////////////////////
	// StatementConstruction
	//
	This& append(int iDataID_, Execution::Interface::IAction* pAction_);
	This& append(const STRING& cstrSql_);
	This& appendPlaceHolder(int iDataID_, Execution::Interface::IAction* pAction_ = 0);
	This& appendArrayPlaceHolder(int iDataID_, Execution::Interface::IAction* pAction_ = 0);

	void readyToSet();
	void toStatement(Execution::Interface::IProgram& cProgram_);
	

	// for serialize
	static This* getInstance(int iCategory_);

	///////////////////////////////
	// Common::Externalizable
	int getClassID() const;


	///////////////////////////////
	// ModSerializer
	void serialize(ModArchive& archiver_);
	

protected:
private:
	//constructor
	StatementConstruction()
		: Super(),
		  m_cSqlData(),
		  m_bConcatinate(false),
		  m_vecStatement(),
		  m_vecData(),
		  m_cStream(),
		  m_vecType(),
		  m_cPreActions()
	{}
	
	StatementConstruction(int iSqlID_, bool bConcatinate_)
		: Super(),
		  m_cSqlData(iSqlID_),
		  m_bConcatinate(bConcatinate_),
		  m_vecStatement(),
		  m_vecData(),
		  m_cStream(),
		  m_vecType(),
		  m_cPreActions()
	{}

	void createStringParameter(const STRING& in, STRING& out) const;

	Execution::Action::ArrayDataHolder m_cSqlData;

	bool m_bConcatinate;
	OSTRSTREAM m_cStream;
	
	Execution::Action::ActionList m_cPreActions;
	
	VECTOR<STRING> m_vecStatement;
	VECTOR<int> m_vecType;
	VECTOR<Execution::Action::DataHolder> m_vecData;
	VECTOR<Execution::Action::DataHolder> m_vecPlaceHolder;
};



_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_ACTION_SQLGENERATOR_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

