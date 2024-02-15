// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Environment.h -- Environment in analysis and optimization
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

#ifndef __SYDNEY_OPT_ENVIRONMENT_H
#define __SYDNEY_OPT_ENVIRONMENT_H

#include "Opt/Module.h"
#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

#include "Common/AutoCaller.h"

#include "Execution/Declaration.h"

#include "Plan/Declaration.h"
#include "Plan/Utility/ObjectSet.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace Communication
{
	class Connection;
}
namespace Schema
{
	class Database;
	class File;
	class Table;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_OPT_BEGIN

////////////////////////////////////////////////////////////////////
// CLASS
//	Opt::Environment -- Environment in analysis and optimization
//
//	NOTES
class Environment
{
public:
	typedef Environment This;

	// ENUM
	//	Opt::Environment::Scope::Value -- type of scope
	// NOTES
	struct Scope
	{
		enum Value
		{
			Normal				= 0,
			Exists,							// subquery in exists
			JoinedTable,					// joined tables
			ValueNum
		};
	};
	// ENUM
	//	Opt::Environment::Status::Value -- status in analysis
	// NOTES
	struct Status
	{
		typedef unsigned int Value; // [QAC++] flag constant should be unsigned
		enum _Value
		{
			Normal					= 0,
			Reset					= 1,		// reset status value
			SetFunction				= 1 << 1,	// in a set function
			SelectList				= 1 << 2,	// in select list
			GroupBy					= 1 << 3,	// in group by clause
			Having					= 1 << 4,	// in having clause
			NoTopPredicate			= 1 << 5,	// not top predicate
			SimpleForm				= 1 << 6,	// simple table expression
			ArbitraryElementAllowed	= 1 << 7,	// arbitrary element specification is allowed
			NoArbitraryElement		= 1 << 8,	// arbitrary element specification is not allowed
			Insert					= 1 << 9,	// in insert statement
			FixJoinOrder			= 1 << 10,	// do not change join order
			OrderBy					= 1 << 11,	// in order by clause
			KnownNotNull			= 1 << 12,	// scalar value can be regarded as not null
			KnownNull				= 1 << 13,	// scalar value can be regarded as null
			Exists					= 1 << 14,	// under exists subquery
			Subquery				= 1 << 15,	// under subquery
			ContainsOperand			= 1 << 16,	// contains operand
			ExpandElementAllowed	= 1 << 17,	// expand element specification is allowed			
			RankFrom				= 1 << 18,	// under rank from subquery
			Routine					= 1 << 19,	// under routine body
			In						= 1 << 20,	// under in predicate
			ValueNum
		};
	};

	// constructor
	static AUTOPOINTER<This>
				create(const EnvironmentArgument& cArgument_);
	// destructor
	virtual ~Environment() {}

	// accessor
	virtual Schema::Database* getDatabase() = 0;
	virtual Trans::Transaction& getTransaction() = 0;
	virtual Execution::Interface::IProgram* getProgram() = 0;
	virtual Communication::Connection* getConnection() = 0;
	virtual Common::DataArrayData* getParameter() = 0;
	virtual bool isPrepare() = 0;
	virtual bool isRecovery() = 0;
	virtual bool isRollback() = 0;
	virtual bool isUndo() = 0;
	virtual int getProtocolVersion() = 0;
	virtual bool hasCascade() = 0;

	virtual void setProgram(Execution::Interface::IProgram* pProgram_) = 0;

	// interface
	typedef Common::AutoCaller0<This> AutoPop;

	// add new status for analysis
	virtual AutoPop pushStatus(Status::Value iStatus_) = 0;
	// erase one element from current status
	virtual AutoPop eraseStatus(Status::Value iStatus_) = 0;
	// erase status value pushed
	virtual void popStatus() = 0; // called by AutoPop destructor
	// add one element to current status
	virtual void addStatus(Status::Value iStatus_) = 0;
	// check whether specified status value is set
	virtual bool checkStatus(Status::Value iStatus_) = 0;

	// add new name scope
	virtual AutoPop pushNameScope(Scope::Value iScope_ = Scope::Normal) = 0;
	// erase name scope
	virtual void popNameScope() = 0; // called by AutoPop destructor

	// current scope is exists subquery
	virtual bool isInExists() = 0;
	// current scope is grouping
	virtual bool isGrouping() = 0;
	// set current scope is grouping
	virtual void setGrouping() = 0;

	// add grouping column
	virtual void addGroupingColumn(Plan::Relation::RowElement* pRowElement_) = 0;
	// add grouping column
	virtual void removeGroupingColumn(Plan::Interface::IScalar* pRowElement_) = 0;	
	// check whether a rowelement is grouping column
	virtual bool isGroupingColumn(Plan::Relation::RowElement* pRowElement_) = 0;

	// get grouping column
	virtual Plan::Utility::RowElementSet& getGroupingColumn() = 0;

	// add outer reference
	virtual void addOuterReference(Plan::Interface::IRelation* pRelation_,
								   Plan::Relation::RowElement* pRowElement_) = 0;
	// set retrieved for all outer reference
	virtual void retrieveOuterReference() = 0;
	// has outer reference?
	virtual bool hasOuterReference() = 0;

	// get outer references' relation
	virtual const Plan::Utility::RelationSet& getOuterRelation() = 0;

	// add new plan::tree::node
	virtual int addNode(Plan::Tree::Node* pNode_) = 0;
	// add rowinfo to pool
	virtual void addRowInfo(Plan::Relation::RowInfo* pRowInfo_) = 0;
	// add table info for validity check
	virtual void addTable(Plan::Relation::Table* pTable_) = 0;
	// add column info for validity check
	virtual void addColumn(Plan::Scalar::Field* pColumn_) = 0;

	// add file object
	virtual void addFile(Plan::Interface::IFile* pFile_) = 0;
	// add Common::Object to memory management
	virtual int addObject(Common::Object* pObject_) = 0;

	// add contains predicate
	virtual void addContains(Plan::Interface::IScalar* pColumn_,
							 Plan::Predicate::Contains* pContains_) = 0;
	// get contains predicate
	virtual const VECTOR<Plan::Predicate::Contains*>&
				getContains(Plan::Interface::IScalar* pColumn_) = 0;
	virtual const VECTOR<Plan::Predicate::Contains*>&
				getContainsByAnyOperand(Plan::Interface::IScalar* pColumn_) = 0;

	// erase a plan::tree::node
	virtual void eraseNode(int iID_) = 0;
	// erase Common::Object
	virtual void eraseObject(int iID_) = 0;

	// add one scalar known as null
	virtual void addKnownNull(Plan::Interface::IScalar* pScalar_) = 0;
	// add one scalar known as not null
	virtual void addKnownNotNull(Plan::Interface::IScalar* pScalar_) = 0;

	// check whether a scalar is known as null
	virtual bool isKnownNull(Plan::Interface::IScalar* pScalar_) = 0;
	// check whether a scalar is known as not null
	virtual bool isKnownNotNull(Plan::Interface::IScalar* pScalar_) = 0;

	// check whether a table is used in simple aggregation
	virtual bool isSimpleTable(Schema::Table* pSchemaTable_) = 0;
	// set a table is used in simple aggregation
	virtual void setSimpleTable(Schema::Table* pSchemaTable_) = 0;

	// check whether a table is target of updating
	virtual bool isUpdateTable(Schema::Table* pSchemaTable_) = 0;
	// set a table to be a target of updating
	virtual void setUpdateTable(Schema::Table* pSchemaTable_) = 0;

	// check whether a table is target of inserting
	virtual bool isInsertTable(Schema::Table* pSchemaTable_) = 0;
	// set a table to be a target of inserting
	virtual void setInsertTable(Schema::Table* pSchemaTable_) = 0;

	// check whether a table is refered
	virtual bool isReferedTable(Schema::Table* pSchemaTable_) = 0;
	// add a table to be refered
	virtual void addReferedTable(Schema::Table* pSchemaTable_) = 0;
	// get all the table refered in order of schema::id
	virtual const VECTOR<Schema::Table*>& getReferedTable() = 0;

	// check whether a table is refered in subquery
	virtual bool isSubqueryTable(Schema::Table* pSchemaTable_) = 0;
	// add a table to be refered in subquery
	virtual void addSubqueryTable(Schema::Table* pSchemaTable_) = 0;

	// add an index file to be scanned
	virtual void addIndexScan(Schema::File* pSchemaFile_) = 0;
	// check whether  an index file is scanned
	virtual bool isIndexScan(Schema::File* pSchemaFile_) = 0;

	// get relation object corresponding to specified name
	virtual Plan::Interface::IRelation*
					getRelation(const STRING& cstrName_) = 0;

	// get namemap for current namescope
	virtual NameMap* getNameMap() = 0;
	// get namemap related to specified relation
	virtual NameMap* getNameMap(Plan::Interface::IRelation* pRelation_) = 0;
	// register new namemap to object pool
	virtual void addNameMap(NameMap* pNameMap_) = 0;

	// search scalar using table name and column name
	virtual Plan::Relation::RowElement*
					searchScalar(const STRING& cstrTableName_,
								 const STRING& cstrColumnName_) = 0;
	// search scalar using column name
	virtual Plan::Relation::RowElement*
					searchScalar(const STRING& cstrColumnName_) = 0;

	// add general scalar with name
	virtual void addScalar(Plan::Interface::IRelation* pRelation_,
						   const STRING& cstrName_,
						   Plan::Relation::RowElement* pRowElement_) = 0;

	// search predicate using statement
	virtual Plan::Interface::IPredicate*
					searchPredicate(const STRING& cstrStatement_) = 0;
	// add predicate with statement
	virtual void addPredicate(Plan::Interface::IRelation* pRelation_,
							  const STRING& cstrStatement_,
							  Plan::Interface::IPredicate* pPredicate_) = 0;

	// get correlation name of a relation
	virtual const STRING& getCorrelationName(Plan::Interface::IRelation* pRelation_) = 0;

	// set alias name of a rowelement
	virtual void setAliasName(Plan::Relation::RowElement* pRowElement_,
							  const STRING& cstrName_) = 0;
	// get alias name of a rowelement
	virtual const STRING& getAliasName(Plan::Relation::RowElement* pRowElement_) = 0;

	// get file object related to a schema::file
	virtual Plan::Interface::IFile* getFile(const Schema::File* pSchemaFile_) = 0;
	// get file object related to a variable
	virtual Plan::Interface::IFile* getFile(Plan::Interface::IScalar* pVariable_) = 0;

	// set placeholder object
	virtual void setPlaceHolder(int iNumber_,
								Plan::Scalar::Value* pVariable_) = 0;
	// get placeholder object
	virtual Plan::Scalar::Value* getPlaceHolder(int iNumber_) = 0;
	virtual const VECTOR<Plan::Scalar::Value*>& getPlaceHolder() = 0;

	// set session variable object
	virtual void setSessionVariable(const STRING& cstrName_,
									Plan::Scalar::Value* pVariable_) = 0;
	// get session variable object
	virtual Plan::Scalar::Value* getSessionVariable(const STRING& cstrName_) = 0;

	// set distinct function
	virtual Plan::Interface::IScalar*
					setDistinctFunction(Plan::Interface::IScalar* pOperand_,
										Plan::Interface::IScalar* pDistinct_) = 0;
	// get distinct function
	virtual Plan::Interface::IScalar*
					getDistinctFunction(Plan::Interface::IScalar* pOperand_) = 0;

	// set new null constant object
	virtual void setNullConstant(Plan::Scalar::Value* pValue_) = 0;
	// set new default constant object
	virtual void setDefaultConstant(Plan::Scalar::Value* pValue_) = 0;
	// get null constant object
	virtual Plan::Scalar::Value* getNullConstant() = 0;
	// get default constant object
	virtual Plan::Scalar::Value* getDefaultConstant() = 0;

	// set locator relationship
	virtual void addLocator(Plan::Interface::IScalar* pScalar_,
							Execution::Interface::IIterator* pIterator_,
							int iLocatorID_) = 0;
	// get locatorID related to a scalar
	virtual int getLocator(Plan::Interface::IScalar* pScalar_,
						   Execution::Interface::IIterator* pIterator_) = 0;

					

protected:
	// constructor
	Environment() {}
private:
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_ENVIRONMENT_H

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
