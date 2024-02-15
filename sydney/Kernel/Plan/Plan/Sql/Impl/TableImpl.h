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

#ifndef __SYDNEY_PLAN_SQL_TABLEIMPL_H
#define __SYDNEY_PLAN_SQL_TABLEIMPL_H

#include "Plan/Sql/Module.h"
#include "Plan/Sql/Table.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

namespace TableImpl
{
    class SimpleTable
        : public Sql::Table
    {

    public:
        SimpleTable(const STRING& cstrTable_,
					const STRING& cstrCorrelationName_)
            :m_cstrTable(cstrTable_),
			 m_cstrCorrelationName(cstrCorrelationName_),
             Sql::Table()
            {}

        virtual ~SimpleTable() {}

        virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;


    private:
        STRING m_cstrTable;
		STRING m_cstrCorrelationName;
    };


    class DyadicJoin
        : public Sql::Table
    {
    public:
        DyadicJoin(Tree::Node::Type eOperator_,
                   Interface::ISqlNode* pPredicate_,
                   Interface::ISqlNode* pLeft_,
                   Interface::ISqlNode* pRight_)
            : m_eOperator(eOperator_),
              m_pPredicate(pPredicate_),
              m_pLeft(pLeft_),
              m_pRight(pRight_),
              Sql::Table()
        {}

        virtual ~DyadicJoin() {}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);
		
        virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

    private:
        Tree::Node::Type m_eOperator;
        Interface::ISqlNode* m_pPredicate;
        Interface::ISqlNode* m_pLeft;
        Interface::ISqlNode* m_pRight;
    };

	class NadicJoin
        : public Sql::Table
    {
    public:
        NadicJoin(VECTOR<Interface::ISqlNode*> cVecOperands_)
           : m_vecTable(cVecOperands_),
              m_pPredicate(0),
              Sql::Table()
        {}

        virtual ~NadicJoin() {}
    /////////////////////////////
    // Plan::Interface::ISqlNode		
        virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);		


    private:
        VECTOR<Interface::ISqlNode*> m_vecTable;
        Interface::ISqlNode* m_pPredicate;
    };
}
_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SQL_TABLEIMPL_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
