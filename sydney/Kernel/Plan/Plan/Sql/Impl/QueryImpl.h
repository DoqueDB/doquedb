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

#ifndef __SYDNEY_PLAN_SQL_QUERYIMPL_H
#define __SYDNEY_PLAN_SQL_QUERYMPL_H


#include "boost/bind.hpp"
#include "Plan/Sql/Module.h"
#include "Plan/Sql/Query.h"
#include "Plan/Candidate/Argument.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"

#include "Opt/Environment.h"

#include "Schema/Cascade.h"




_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

namespace QueryImpl
{

    // CLASS
    //  QueryImpl::Base --implementation class of Projection
    //
    // NOTES
    class Base
        : public Sql::Query
    {
        typedef Sql::Query Super;
        typedef Base This;
    public:
        Base(Plan::Interface::ISqlNode* pTable_, bool bDistribution)
            :Super(),
             m_cFieldSet(),
             m_pTable(pTable_),
			 m_pRowInfo(0),
			 m_iPreparedData(-1),
			 m_bIterable(false),
			 m_pPredicate(0),
			 m_bDistribution(bDistribution),
			 m_iSqlID(-1),
			 m_pStatement(0)
        {}

    /////////////////////////////
    // Plan::Sql::Query
        virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

		virtual void addUpdateColumn(Plan::Interface::IScalar* pScalar_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

		virtual void setDistinct()
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }
		
        virtual void setPredicate(Plan::Interface::ISqlNode* pPredicate_)
        {
            m_pPredicate = pPredicate_;
        }

        virtual Plan::Interface::ISqlNode* getPredicate()
        {
			return m_pPredicate;
        }				

		virtual void setIterable()
        {
			m_bIterable = true;
        }

		virtual bool isIterable()
		{
			return m_bIterable;
        }		

		virtual Execution::Interface::IIterator* getQueryIterator()
		{
			return 0;
        }


        virtual void setGroupBy(Plan::Order::Specification* pGrouping_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }
		virtual void setCorrelationName(const STRING& cstrTableName_)
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		virtual const STRING& getCorrelationName() const
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		virtual bool isDistribution() const {
			return m_bDistribution;
		}

		virtual void setHaving(Plan::Interface::ISqlNode* pHaving_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

        virtual void setOrderBy(Plan::Order::Specification* pOrder_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

		virtual Plan::Order::Specification* getOrderBy()
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

        virtual void setLimit(const Plan::Interface::ISqlNode* pLimit_)
        {
            _SYDNEY_THROW0(Exception::NotSupported);
        }

		virtual void append(Opt::Environment& cEnvironment_, VECTOR<Query*>& cvecTables_)
		{

			cvecTables_.pushBack(Query::create(cEnvironment_,
											   Plan::Sql::Query::SELECT,
											   m_pTable,
											   true));
			m_pTable = Query::join(cEnvironment_, cvecTables_)->getTable();
		}
		
        virtual Plan::Interface::ISqlNode* getTable() 
        {
            return m_pTable;
        }


        virtual int createOutput(Opt::Environment& cEnvironment_,
                                 Execution::Interface::IProgram& cProgram_,
                                 Execution::Interface::IIterator* pIterator_,
                                 Plan::Candidate::AdoptArgument& cArgument_);


        virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);

		
		virtual Execution::Interface::ICollection* getQueue(Opt::Environment& cEnvironment_,
															Execution::Interface::IProgram& cProgram_,
															Execution::Interface::IIterator* pIterator_,
															Plan::Candidate::AdoptArgument& cArgument_,
															bool bNoUndone_)
		{
			
			Execution::Interface::IIterator* pInput =
				getQueryIterator() ? getQueryIterator() : pIterator_;
			
			Execution::Interface::ICollection* pResult = pInput->getCascadeQueue();
			if (!pResult) {
				pResult = createQueue(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_,
									  bNoUndone_);
				pInput->setCascadeQueue(pResult);
			}

			return pResult;
		}

		virtual Execution::Interface::IIterator* adoptQuery(Opt::Environment& cEnvironment_,
															Execution::Interface::IProgram& cProgram_,
															Plan::Candidate::AdoptArgument& cArgument_,
															bool bNoUndone_);

		virtual void addSqlGenerator(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 bool bConcatinate_);

		virtual int getSqlID(Execution::Interface::IProgram& cProgram_);

		
    protected:
        Plan::Utility::ScalarSet m_cFieldSet;
        Plan::Utility::ScalarSet m_cUpdateSet;
        Plan::Interface::ISqlNode* m_pTable;
        Plan::Candidate::Row* m_pRowInfo;
		Plan::Interface::ISqlNode* m_pPredicate;
		bool m_bDistribution;


	private:
		int m_iPreparedData;
		bool m_bIterable;
		int m_iSqlID;
		DExecution::Action::StatementConstruction* m_pStatement;

		virtual Execution::Interface::ICollection* createQueue(Opt::Environment& cEnvironment_,
															   Execution::Interface::IProgram& cProgram_,
															   Execution::Interface::IIterator* pIterator_,
															   Plan::Candidate::AdoptArgument& cArgument_,
															   bool bNoUndone_);

		void addIterator(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Plan::Candidate::AdoptArgument& cArgument_,
						 int iQueueID_,
						 bool bNoUndone_,
						 Schema::Cascade* pCascade_);		

    };


    // CLASS
    //  QueryImpl::SelectImpl --implementation class of Projection
    //
    // NOTES
    class SelectImpl
        : public Base
    {
        typedef Base Super;
        typedef SelectImpl This;

	public:
        SelectImpl(Plan::Interface::ISqlNode* pTable_, bool bDistribution)
            :Super(pTable_, bDistribution),
             m_pGrouping(0),
			 m_pHaving(0),
             m_pOrder(0),
             m_pLimit(0),
			 m_bDistinct(false),
			 m_cstrCorrelationName()
        {}

		

    /////////////////////////////
    // Plan::Sql::Query
        virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_)
        {
            if (!m_cFieldSet.isContaining(pScalar_)) {
				if (m_cstrCorrelationName.getLength() > 0)
					pScalar_->pushNameScope(this);
				
                m_cFieldSet.add(pScalar_);
            }
        }

		virtual void setCorrelationName(const STRING& cstrTableName_)
		{
			if (m_cstrCorrelationName.getLength() > 0) {
				;_SYDNEY_ASSERT(false);
			}
			
			m_cstrCorrelationName = cstrTableName_;
			m_cFieldSet.foreachElement(boost::bind(&Plan::Interface::ISqlNode::pushNameScope,
												   _1,
												   this));
		}

		virtual const STRING& getCorrelationName() const
		{
			return m_cstrCorrelationName;
		}

		virtual void setPredicate(Plan::Interface::ISqlNode* pPredicate_)
        {
			;_SYDNEY_ASSERT(m_pPredicate == 0);
            m_pPredicate = pPredicate_;
        }


        virtual void setGroupBy(Plan::Order::Specification* pGrouping_)
        {
            m_pGrouping = pGrouping_;
        }
        virtual void setHaving(Plan::Interface::ISqlNode* pHaving_)
        {
            m_pHaving = pHaving_;
        }		

		virtual void setDistinct()
		{
			m_bDistinct = true;
		}
        virtual void setOrderBy(Plan::Order::Specification* pOrder_)
        {
            m_pOrder = pOrder_;
        }

		virtual Plan::Order::Specification* getOrderBy()
        {
			return m_pOrder;
        }

        virtual void setLimit(const Plan::Interface::ISqlNode* pLimit_)
        {
            m_pLimit = pLimit_;
        }

		virtual bool isTable()
		{
			if (m_cFieldSet.getSize() == 0
				&& !m_pGrouping
				&& !m_pOrder
				&& !m_pLimit
				&& !m_pPredicate) {
				return true;
			} else {
				return false;
			}
		}


    /////////////////////////////
    // Plan::Interface::ISqlNode
        virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);		

    /////////////////////////////
    // Plan::Query::Impl::Base


    private:
        Plan::Order::Specification* m_pGrouping;
		Plan::Interface::ISqlNode* m_pHaving;
        Plan::Order::Specification* m_pOrder;
        const Plan::Interface::ISqlNode* m_pLimit;
		bool m_bDistinct;
		STRING m_cstrCorrelationName;

	};


    // CLASS
    //  QueryImpl::InsertImpl --implementation class of Projection
    //
    // NOTES
    class InsertImpl
        : public Base
    {
        typedef Base Super;
        typedef InsertImpl This;
    public:
        InsertImpl(Plan::Interface::ISqlNode* pTable_, bool bDistribution_)
            :Super(pTable_, bDistribution_)
        {}

        virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_)
        {
            ;
        }

        virtual void addUpdateColumn(Plan::Interface::IScalar* pScalar_)
        {
            if (!m_cFieldSet.isContaining(pScalar_)) {
                m_cFieldSet.add(pScalar_);
            }
        }



    /////////////////////////////
    // Plan::Interface::ISqlNode
        virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;
    private:
    };


	    // CLASS
    //  QueryImpl::UpdateImpl --implementation class of Projection
    //
    // NOTES
    class UpdateImpl
        : public Base
    {
        typedef Base Super;
        typedef UpdateImpl This;
    public:
        UpdateImpl(Plan::Interface::ISqlNode* pTable_, bool bDistribution)
            :Super(pTable_, bDistribution)
        {}

		UpdateImpl(Plan::Interface::ISqlNode* pTable_,
				   Plan::Interface::ISqlNode* pPredicate_,
				   bool bDistribution)
            :Super(pTable_, bDistribution)
        {
			setPredicate(pPredicate_);
		}



        virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_)
        {
			if (!m_cFieldSet.isContaining(pScalar_)) {
                m_cFieldSet.add(pScalar_);
            };
        }

        virtual void addUpdateColumn(Plan::Interface::IScalar* pScalar_)
        {
            if (!m_cUpdateSet.isContaining(pScalar_)) {
                m_cUpdateSet.add(pScalar_);
            }
        }


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

    };
	


	// CLASS
    //  QueryImpl::DeleteImpl --implementation class of Projection
    //
    // NOTES
    class DeleteImpl
        : public Base
    {
        typedef Base Super;
        typedef DeleteImpl This;
    public:
        DeleteImpl(Plan::Interface::ISqlNode* pTable_, bool bDistribution)
            :Super(pTable_, bDistribution)

        {}

		DeleteImpl(Plan::Interface::ISqlNode* pTable_,
				   Plan::Interface::ISqlNode* pPredicate_,
				   bool bDistribution)
            :Super(pTable_, bDistribution)

		{
			setPredicate(pPredicate_);
		}

        virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_)
        {
            if (!m_cFieldSet.isContaining(pScalar_)) {
                m_cFieldSet.add(pScalar_);
            }
        }		

        virtual void addUpdateColumn(Plan::Interface::IScalar* pScalar_)
        {
			;
        }


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

    };
}

_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SQL_QUERYIMPL_H_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

