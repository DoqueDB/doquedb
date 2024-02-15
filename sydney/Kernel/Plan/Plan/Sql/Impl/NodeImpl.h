// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SqlEncoder.h -- definition for explain plan tree
// 
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SQL_NODEIMPL_H
#define __SYDNEY_PLAN_SQL_NODEIMPL_H

#include "Plan/Sql/Module.h"
#include "Plan/Sql/Node.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

namespace NodeImpl
{
    class SimpleStringNode
        : public Sql::Node
    {

	public:
		SimpleStringNode(const STRING& cstrName_)
			:m_cstrName(cstrName_)
		{}

		virtual ~SimpleStringNode() {}
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
		
	private:
		STRING m_cstrName;
	};


	class SimpleDataNode
        : public Sql::Node
    {

	public:
		SimpleDataNode(int iDataID_)
			:m_iDataID(iDataID_)
		{}
		
		virtual	~SimpleDataNode() {}
		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:

	private:
		int m_iDataID;
	};

	class ArrayNode
        : public Sql::Node
    {

	public:
		ArrayNode()
			:m_vecNode()
		{}

		virtual ~ArrayNode() {}
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

		virtual void appendNode(Plan::Interface::ISqlNode* pNode_)
		{m_vecNode.PUSHBACK(pNode_);}

	protected:
		
	private:
		VECTOR<Plan::Interface::ISqlNode*> m_vecNode;
	};

	class ArrayPlaceHolderNode
		:public Sql::Node
	{
	public:
		ArrayPlaceHolderNode(int iDataID_)
			:m_iDataID(iDataID_)
		{}

		~ArrayPlaceHolderNode() {}
		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

		
	protected:


	private:
		int m_iDataID;
	};
}
_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SQL_NODEIMPL_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
