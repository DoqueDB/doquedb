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

#ifndef __SYDNEY_PLAN_SQL_QUERY_H
#define __SYDNEY_PLAN_SQL_QUERY_H

#include "Plan/Sql/Module.h"
#include "Plan/Tree/Node.h"

#include "Common/Object.h"

#include "Plan/Interface/ISqlNode.h"
#include "Plan/Interface/IPredicate.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

class Query
	: public Common::Object, public Plan::Interface::ISqlNode
{
	typedef Query This;
	typedef Common::Object Super;

public:

   	enum Type
	{
		SELECT,
		INSERT,
		UPDATE,
		DEL // DELETEは使用できない
	};
	
	virtual void addProjectionColumn(Plan::Interface::IScalar* pScalar_) = 0;
	virtual void addUpdateColumn(Plan::Interface::IScalar* pScalar_) = 0;
	virtual void setPredicate(Plan::Interface::ISqlNode* pPredicate_) = 0;
	virtual Plan::Interface::ISqlNode* getPredicate() = 0;
	virtual void setIterable() = 0;
	virtual bool isIterable() = 0;
	virtual Execution::Interface::IIterator* getQueryIterator()	 = 0;


	virtual void setDistinct() = 0;
	
	virtual void setGroupBy(Plan::Order::Specification* pGrouping_) = 0;

	virtual void setCorrelationName(const STRING& cstrTableName_) = 0;
	virtual const STRING& getCorrelationName() const = 0;
	
	virtual void setHaving(Plan::Interface::ISqlNode* pHaving_) = 0;
	virtual void setOrderBy(Plan::Order::Specification* pOrder_) = 0;
	virtual Plan::Order::Specification* getOrderBy() = 0;
	virtual void setLimit(const Plan::Interface::ISqlNode* pLimit_) = 0;


	virtual bool isDistribution() const = 0;

	//
	// iSqlDataIDで指定したSQL文を各カスケードでの実行結果をputするICollectionを生成し、返します。
	// cArgument_->m_pCascadeをセットした場合は、そのカスケードのみでの実行結果となり、
	// セットしない場合は、指定したデータベースで登録されている全てカスケードの実行結果となります。
	virtual Execution::Interface::ICollection* getQueue(Opt::Environment& cEnvironment_,
														Execution::Interface::IProgram& cProgram_,
														Execution::Interface::IIterator* pIterator_,
														Plan::Candidate::AdoptArgument& cArgument_,
														bool bNoUndone_) = 0;

	virtual Execution::Interface::IIterator*  adoptQuery(Opt::Environment& cEnvironment_,
														 Execution::Interface::IProgram& cProgram_,
														 Plan::Candidate::AdoptArgument& cArgument_,
														 bool bNoUndone_) = 0;
	

	// SQL文を生成するActionをpIterator_に追加します。
	// SQL文は戻り値で返すintのDataIDにStringData型でセットされます。
	// QueryがisIterable==trueの場合（繰り返し実行する）、ActionはExecutionに追加します。
	// QueryがisIterable==falseの場合ActionはStartUPに追加します。
	virtual void addSqlGenerator(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 bool bConcatinate_) = 0;

	// 実行時に生成されたSQL文がセットされるデータID
	// 配列型で第一要素がString型のSQL
	// 第二要素が配列型でパラメーターがセットされる。
	virtual int getSqlID(Execution::Interface::IProgram& cProgram_) = 0;

	
	virtual int createOutput(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pResult,
							 Plan::Candidate::AdoptArgument& cArgument_) = 0;

	virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_) = 0;

	virtual void append(Opt::Environment& cEnvironment_, VECTOR<Query*>& cvecJoinTable_) = 0;

	virtual Plan::Interface::ISqlNode* getTable() = 0;

	virtual bool isQuery() const {return true;}
	
	virtual Query* getQuery() {return this;}


	virtual ~Query(){}
	

	static Query* create(Opt::Environment& cEnvironment_,
						 Type eType_,
						 Plan::Interface::ISqlNode* pTable_,
						 bool isDist);

	static Query* join(Opt::Environment& cEnvironment_,
					   Tree::Node::Type eOperator_,
					   Plan::Interface::ISqlNode* pPredicate_,
					   Query* pLeft_,
					   Query* pRight_);

	static Query* join(Opt::Environment& cEnvironment_,
					   VECTOR<Query*>& cVecOperands_);
	
					   

private:
	void registerToEnvironment(Opt::Environment& cEnvironment_);

	
};

_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SQL_QUERY_H

//
// Copyright (c) 2007, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
