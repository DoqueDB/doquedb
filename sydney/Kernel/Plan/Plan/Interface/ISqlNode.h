// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SqlEncoder.h -- definition for explain plan tree
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_ISQLNODE_H
#define __SYDNEY_PLAN_INTERFACE_ISQLNODE_H

#include "Plan/Interface/Module.h"
#include "Plan/Declaration.h"
#include "Opt/Algorithm.h"
#include "Execution/Declaration.h"

_SYDNEY_BEGIN
namespace DExecution
{
	namespace Action
	{
		class StatementConstruction;
	}
}
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

class ISqlNode
{
public:
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const = 0;
	
	virtual void setParameter(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Execution::Interface::IIterator* pIterator_,
							  DExecution::Action::StatementConstruction& cExec_,
							  const Plan::Sql::QueryArgument& cArgument_);
		
	virtual bool isQuery() const {return false;}
	virtual bool isTable() {return false;}

	// QueryへのCast用
	virtual Plan::Sql::Query* getQuery() {return 0;}
	
	virtual void pushNameScope(Plan::Sql::Query* pQuery_);
	
	virtual void nextScope();
	virtual void reverseScope();

	virtual const STRING& getCorrelationName() const;
	ISqlNode()
		: m_iPosition(-1),
		  m_vecNameScope()
	{}

	virtual ~ISqlNode(){}
	static ISqlNode* createSimpleNode(Opt::Environment& cEnvironment_,
									  const STRING& cstrNode_,
									  int iDataID_);
private:
	VECTOR<Plan::Sql::Query*> m_vecNameScope;
	int m_iPosition;
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_ISQLNODE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
