// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Field.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FIELD_H
#define __SYDNEY_PLAN_SCALAR_FIELD_H

#include "Plan/Scalar/Base.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class Field;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Field -- Interface for schema field
//
//	NOTES
class Field
	: public Base
{
public:
	typedef Base Super;
	typedef Field This;

	// constructor

	// by schema::field
	static This* create(Opt::Environment& cEnvironment_,
						Schema::Field* pSchemaField_,
						Interface::IFile* pFile_,
						Relation::Table* pTable_,
						Scalar::Field* pColumn_ = 0);

	// by schema::field with option
	static This* create(Opt::Environment& cEnvironment_,
						Schema::Field* pSchemaField_,
						Interface::IFile* pFile_,
						Relation::Table* pTable_,
						Scalar::Field* pColumn_,
						Interface::IScalar* pOption_);

	
	// by schema::column
	static This* create(Opt::Environment& cEnvironment_,
						Schema::Column* pSchemaColumn_,
						Relation::Table* pTable_);
	// by function field
	static This* create(Opt::Environment& cEnvironment_,
						Schema::Field::Function::Value eFunction_,
						const Utility::FileSet& cFileSet_,
						Relation::Table* pTable_,
						Interface::IScalar* pFunction_,
						Interface::IScalar* pOperand_ = 0);
	// by schema::field with option
	static This* create(Opt::Environment& cEnvironment_,
						Scalar::Field* pField_,
						Scalar::Field* pColumn_,
						Interface::IScalar* pOption_);
	// by schema::column with option
	static This* create(Opt::Environment& cEnvironment_,
						Schema::Column* pSchemaColumn_,
						Relation::Table* pTable_,
						Scalar::Field* pColumn_,
						Interface::IScalar* pOption_);
	// by a set of Field
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eType_,
						const VECTOR<Scalar::Field*>& vecOperand_,
						Relation::Table* pTable_,
						const STRING& cstrName_);

	// destructor
	virtual ~Field() {}

	// operator
	bool operator==(This& cOther_)
	{return equals(cOther_);}
	bool operator<(This& cOther_)
	{return less(cOther_);}

	// get fetchable files
	static bool getFetchFile(Opt::Environment& cEnvironment_,
							 const GetFileArgument& cArgument_);

	// get searchable files
	static bool getSearchFile(Opt::Environment& cEnvironment_,
							  const GetFileArgument& cArgument_);

	// get scannable files
	static bool getScanFile(Opt::Environment& cEnvironment_,
							const GetFileArgument& cArgument_);

	// get sortable files
	static bool getSortFile(Opt::Environment& cEnvironment_,
							const GetFileArgument& cArgument_);
	
	// get groupable files
	static bool getGroupingFile(Opt::Environment& cEnvironment_,
								const GetFileArgument& cArgument_);

	// get retrieving files
	static bool getRetrieveFile(Opt::Environment& cEnvironment_,
								const GetFileArgument& cArgument_);

	// get put files
	static bool getPutFile(Opt::Environment& cEnvironment_,
						   const GetFileArgument& cArgument_);

	// find corresponding table candidate
	static Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										  Interface::IScalar* pField_,
										  Interface::ICandidate* pCandidate_);
	// check a field is unique?
	static bool checkIsUnique(Opt::Environment& cEnvironment_,
							  Interface::IScalar* pField_);

	// get corresponding table object
	virtual Relation::Table* getTable() = 0;
	// get corresponding file object
	virtual Interface::IFile* getFile() = 0;
	// get corresponding field object for a file
	virtual Scalar::Field* getField(Interface::IFile* pFile_) = 0;
	// get corresponding schema field object
	virtual Schema::Field* getSchemaField() = 0;
	// get corresponding schema column object
	virtual Schema::Column* getSchemaColumn() = 0;
	// is column object?
	virtual bool isColumn() = 0;
	// is rowid column?
	virtual bool isRowID() = 0;
	
	// is rowid BitSet?
	virtual bool isBitSetRowID(){ return false; };

	// is lob column?
	virtual bool isLob() = 0;
	// is objectid field?
	virtual bool isObjectID() = 0;
	// is unique column?
	virtual bool isUnique(Opt::Environment& cEnvironment_) = 0;
	// is delayed retrievable?
	virtual bool isDelayable() = 0;
	// is put key field?
	virtual bool isPutKey() = 0;

	// is really retrieved delayed?
	virtual bool isDelayed(Candidate::Table* pCandidate_) = 0;

	// get key field for delayed retrieval
	virtual Field* getDelayedKey(Opt::Environment& cEnvironment_) = 0;

	// is rowid able to be retrieved?
	virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_) = 0;

	// is not known as null?
	bool isNotKnownNull(Opt::Environment& cEnvironment_)
	{return !isKnownNull(cEnvironment_);}
	// is not known as not nullable?
	bool isNotKnownNotNull(Opt::Environment& cEnvironment_)
	{return !isKnownNotNull(cEnvironment_);}

	// get alternative value if exists
	virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_) = 0;
	virtual Interface::IScalar* getAlternativeValue(Opt::Environment& cEnvironment_) = 0;
	int generateAlternative(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_);

	// get fetch key field
	virtual Field* createFetchKey(Opt::Environment& cEnvironment_) = 0;

	// create CheckedField
	virtual Field* checkRetrieve(Opt::Environment& cEnvironment_) = 0;
	// create ChcekedField for put
	virtual Field* checkPut(Opt::Environment& cEnvironment_) = 0;

	// create set of retrieve files
	virtual void createRetrieveFile(Opt::Environment& cEnvironment_,
									Utility::FileSet& cFileSet_) = 0;

	// is CheckedField?
	virtual bool isChecked() {return false;}
	// get CheckedField
	virtual CheckedField* getChecked() {return 0;}
	// is UpdateField?
	virtual bool isUpdate() {return false;}
	// get UpdateField
	virtual UpdateField* getUpdate() {return 0;}
	// is FieldWrapper?
	virtual bool isWrapper() {return false;}
	// get FieldWrapper
	virtual FieldWrapper* getWrapper() {return 0;}

	// is virtual column object?
	virtual bool isFunction() {return false;}
	// get function
	virtual Interface::IScalar* getFunction() {return 0;}

	// is including choice of more than one fields
	virtual bool isChoice() {return false;}

	virtual bool isExpandElement() { return false;}

	// check a function field is available with a predicate
	virtual bool checkAvailability(Interface::IPredicate* pPredicate_)
	{return false;}

	// get original
	Interface::IScalar* getOriginal()
	{
		return isFunction() ? getFunction() : this;
	}

	// get option
	virtual Interface::IScalar* getOption() {return 0;}

	virtual Field* translateToBitSetField(Opt::Environment& cEnvironment_) {return this;}

	// add to field set
	virtual void addField(Interface::IFile* pFile_,
						  Utility::FieldSet& cFieldSet_);

	// add locator to environment
	virtual void addLocator(Opt::Environment& cEnvironment_,
							Execution::Interface::IIterator* pIterator_,
							int iLocatorID_);
	// get locator from environment
	virtual int getLocator(Opt::Environment& cEnvironment_,
						   Execution::Interface::IIterator* pIterator_);

/////////////////////////////////////
// Interface::IScalar::
//	virtual const STRING& getName();
//	virtual Check::Value check(Opt::Environment& cEnvironment_,
//							   const CheckArgument& cArgument_);
//	virtual bool isRefering(Interface::IRelation* pRelation_);
	virtual void getUsedTable(Utility::RelationSet& cResult_);
	virtual void getUsedField(Utility::FieldSet& cResult_);
	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
							   Predicate::CheckUnknownArgument& cResult_);
	virtual bool isField()
	{return true;}
	virtual Scalar::Field* getField()
	{return this;}
//	virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
//										  Interface::IScalar* pOption_);
//	virtual void require(Opt::Environment& cEnvironment_,
//						 Interface::ICandidate* pCandidate_);
//	virtual void retrieve(Opt::Environment& cEnvironment_);
//	virtual void retrieve(Opt::Environment& cEnvironment_,
//						  Interface::ICandidate* pCandidate_);
//	virtual void use(Opt::Environment& cEnvironment_,
//					 Interface::ICandidate* pCandidate_);
//	virtual bool delay(Opt::Environment& cEnvironment_,
//					   Interface::ICandidate* pCandidate_,
//					   Scalar::DelayArgument& cArgument_);
//	virtual void setMetaData(Opt::Environment& cEnvironment_,
//							 Common::ColumnMetaData& cMetaData_);
//	virtual int generate(Opt::Environment& cEnvironment_,
//						 Execution::Interface::IProgram& cProgram_,
//						 Execution::Interface::IIterator* pIterator_,
//						 Candidate::AdoptArgument& cArgument_);

/////////////////////////////////////
// Node::
//	virtual ModUnicodeString getValue() const;
//	virtual const Common::Data* getData() const;

protected:
	// constructor
	Field()
		: Super(Super::Field)
	{}
	explicit Field(const DataType& cDataType_)
		: Super(Super::Field, cDataType_)
	{}

	// comparator for MAP
	virtual bool equals(This& cOther_);
	virtual bool less(This& cOther_);

///////////////////////////////
// Interface::IScalar::
	virtual int generateData(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

	// generate for this node
	virtual int generateThis(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_,
							 int iDataID_);

	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

private:

};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FIELD_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
