// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Table.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_TABLE_H
#define __SYDNEY_PLAN_RELATION_TABLE_H

#include "Plan/Interface/IRelation.h"
#include "Plan/AccessPlan/Cost.h"

#include "Common/AutoCaller.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace LogicalFile
{
	class OpenOption;
}

namespace Schema
{
	class Constraint;
	class Field;
	class Column;
	class Object;
	class Table;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Table -- Interface::IRelation implementation for schema table
//
//	NOTES
class Table
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Table This;

	struct Retrieve
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrTableName_);
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Table* pSchemaTable_);
	};
	struct Refer
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Table* pSchemaTable_,
							Relation::Table* pTargetTable_);
	};
	struct Insert
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrTableName_,
							Interface::IRelation* pOperand_);
	};
	struct Delete
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							This* pRetrieve_,
							Interface::IRelation* pOperand_);
	};
	struct Update
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							This* pRetrieve_,
							Interface::IRelation* pOperand_);
	};
	struct Import
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							const Opt::ImportArgument& cArgument_,
							Interface::IRelation* pOperand_);
	};
	struct Undo
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							Schema::Table* pSchemaTable_,
							const Common::DataArrayData* pUndoLog_);
	};

	// destructor
	virtual ~Table() {}

	// get schema table object from name
	static Schema::Table* getSchemaTable(Opt::Environment& cEnvironment_,
										 const STRING& cstrTableName_);

	// get corresponding schema table object
	Schema::Table* getSchemaTable() {return m_pSchemaTable;}

	// create field
	Scalar::Field* createField(Opt::Environment& cEnvironment_,
							   Schema::Field* pSchemaField_,
							   Interface::IFile* pFile_,
							   Scalar::Field* pColumn_ = 0);
	virtual Scalar::Field* createField(Opt::Environment& cEnvironment_,
									   Schema::Column* pSchemaColumn_);

	// register field object
	virtual void addField(Scalar::Field* pField_) = 0;
	// get registered field object
	virtual Scalar::Field* getField(Schema::Object* pSchemaObject_) = 0;
	// get field with column and option
	virtual Scalar::Field* getField(Schema::Object* pSchemaObject_,
									Interface::IScalar* pOption_) = 0;

	// register field corresponding to a column
	virtual void addField(Opt::Environment& cEnvironment_,
						  Scalar::Field* pColumn_,
						  Interface::IFile* pFile_,
						  Scalar::Field* pField_) = 0;
	// get field corresponding to a column
	virtual Scalar::Field* getField(Interface::IFile* pFile_,
									Scalar::Field* pColumn_) = 0;

	// add field as retrieved
	virtual void addRetrieved(Scalar::Field* pField_) = 0;

	////////////////////////////
	// for retrieve
	////////////////////////////

	// estimating
	typedef Common::AutoCaller0<This> AutoReset;

	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	virtual Interface::IFile* getEstimateFile();
	virtual void resetEstimateFile(); // called by AutoReset destructor

	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	virtual Interface::IPredicate* getEstimatePredicate();
	virtual void resetEstimatePredicate(); // called by AutoReset destructor

	virtual bool isEstimating();
	virtual Scalar::Field* getField(Scalar::Field* pColumn_);

	// set estimate count cache
	virtual void setEstimateCount(const AccessPlan::Cost::Value& cValue_);
	// get estimate count cache
	virtual AccessPlan::Cost::Value getEstimateCount();

	// adopting
	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	virtual Candidate::Table* getAdoptCandidate();

	////////////////////////////
	// for data operation
	////////////////////////////

	// log output is needed?
	bool isNeedLog(Opt::Environment& cEnvironment_);

	// add column <-> input data correspondence
	virtual void addInput(Opt::Environment& cEnvironment_,
						  Position iPosition_,
						  Interface::IScalar* pSource_);
	// get input data from column
	virtual Interface::IScalar*
				getInput(Opt::Environment& cEnvironment_,
						 Position iPosition_);

	// get generate columns for result to client
	virtual RowInfo* getGeneratedColumn(Opt::Environment& cEnviroment_);

//////////////////////////////
// Interface::IRelation::
	virtual void setCorrelationName(Opt::Environment& cEnvironment_,
									const STRING& cstrTableName_,
									const VECTOR<STRING>& vecColumnName_);

protected:
	// constructor
	Table();
	explicit Table(Schema::Table* pSchemaTable_);

	// set schema::table after construct
	void setSchemaTable(Schema::Table* pSchemaTable_);

	// check updatability
	static void checkUpdatability(Opt::Environment& cEnvironment_,
								  Schema::Table* pSchemaTable_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

private:
//////////////////////////////
// Interface::IRelation::
	virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
	virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
	virtual int setDegree(Opt::Environment& cEnvironment_);
	virtual int setMaxPosition(Opt::Environment& cEnvironment_);
	virtual void createScalarName(Opt::Environment& cEnvironment_,
								  VECTOR<STRING>& vecName_,
								  Position iPosition_);
	virtual void createScalar(Opt::Environment& cEnvironment_,
							  VECTOR<Interface::IScalar*>& vecScalar_,
							  Position iPosition_);
	virtual void createScalarType(Opt::Environment& cEnvironment_,
								  VECTOR<Super::Type>& vecType_,
								  Position iPosition_);
//	virtual void setRetrieved(Opt::Environment& cEnvironment_,
//							  Position iPosition_);
	virtual int addAggregation(Opt::Environment& cEnvironment_,
							   Interface::IScalar* pScalar_,
							   Interface::IScalar* pOperand_);

	Schema::Table* m_pSchemaTable;
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_TABLE_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
