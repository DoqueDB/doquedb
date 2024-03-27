// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FieldImpl.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_FIELDIMPL_H
#define __SYDNEY_PLAN_SCALAR_FIELDIMPL_H

#include "Plan/Candidate/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Utility/ObjectSet.h"

#include "Opt/Explain.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

namespace FieldImpl
{
	class Base;
	class SchemaColumn;
	class RowID;
	class LobColumn;
	class SchemaField;
	class BitSetField;
	class SystemColumn;

	//////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::Base -- base implementation class of Field/Column
	//
	// NOTES

	class Base
		: public Scalar::Field
	{
	public:
		typedef Base This;
		typedef Scalar::Field Super;

		// destructor
		virtual ~Base() {}

	///////////////////
	// Field::
		virtual Relation::Table* getTable() {return m_pTable;}
		virtual Interface::IFile* getFile() {return 0;}
		virtual Scalar::Field* getField(Interface::IFile* pFile_);
		virtual Schema::Field* getSchemaField() {return 0;}
		virtual Schema::Column* getSchemaColumn() {return 0;}
		virtual bool isFunction() {return false;}
		virtual bool isDelayable() {return m_bDelayable;}
		virtual bool isPutKey() {return false;}
		virtual bool isDelayed(Candidate::Table* pCandidate_);
		virtual Scalar::Field* getDelayedKey(Opt::Environment& cEnvironment_);
		virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_)
		{return false;}
		virtual Interface::IScalar* getAlternativeValue(Opt::Environment& cEnvironment_)
		{return 0;}
		virtual bool isRowID() {return m_bRowID && !isFunction();}
		virtual bool isBitSet() {return false;}
		virtual bool isLob() {return false;}
		virtual bool isUnique(Opt::Environment& cEnvironment_) {return m_bRowID && !isFunction();}
		virtual bool isObjectID() {return false;}
		virtual Field* checkRetrieve(Opt::Environment& cEnvironment_);
		virtual Field* checkPut(Opt::Environment& cEnvironment_);
		virtual void createRetrieveFile(Opt::Environment& cEnvironment_,
										Utility::FileSet& cFileSet_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual Check::Value check(Opt::Environment& cEnvironment_,
								   const CheckArgument& cArgument_);
		virtual bool isRefering(Interface::IRelation* pRelation_);
	//	virtual void getUsedTable(Utility::RelationSet& cResult_);
	//	virtual void getUsedField(Utility::FieldSet& cResult_);
	//	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
	//							   Predicate::CheckUnknownArgument& cResult_);
		virtual bool isKnownNull(Opt::Environment& cEnvironment_);
		virtual bool isKnownNotNull(Opt::Environment& cEnvironment_);
	//	virtual bool isField() {return true;}
	//	virtual Scalar::Field* getField() {return this;}
		virtual bool hasField(Interface::IFile* pFile_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void retrieve(Opt::Environment& cEnvironment_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);

	///////////////////////////
	// Interface::ISqlNode::			
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{ return const_cast<Base*>(this)->getName();}
			

	protected:
		// constructor
		Base()
			: Super(),
			  m_pTable(0),
			  m_bDelayable(false),
			  m_bRowID(false),
			  m_pChecked(0)
		{}
		Base(const DataType& cDataType_,
			 Relation::Table* pTable_,
			 bool bDelayable_,
			 bool bRowID_)
			: Super(cDataType_),
			  m_pTable(pTable_),
			  m_bDelayable(bDelayable_),
			  m_bRowID(bRowID_),
			  m_pChecked(0)
		{}
		// set member
		void setArgument(const DataType& cDataType_,
						 Relation::Table* pTable_,
						 bool bDelayable_,
						 bool bRowID_)
		{
			setDataType(cDataType_);
			m_pTable = pTable_;
			m_bDelayable = bDelayable_;
			m_bRowID = bRowID_;
		}
		// set delayability
		void setDelayable(bool bDelayable_)
		{
			m_bDelayable = bDelayable_;
		}

	///////////////////
	// Field::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		Relation::Table* m_pTable;
		bool m_bDelayable;
		bool m_bRowID;
		CheckedField* m_pChecked;
	};

	//////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::SchemaColumn -- implementation class of Field interface for schema::column
	//
	// NOTES

	class SchemaColumn
		: public Base
	{
	public:
		typedef SchemaColumn This;
		typedef Base Super;

		// constructor
		SchemaColumn(Schema::Column* pSchemaColumn_,
					 Relation::Table* pTable_);
		// destructor
		virtual ~SchemaColumn() {}

	///////////////////
	// Field::
		virtual Schema::Field* getSchemaField(); // implemented in cpp file
		virtual Schema::Column* getSchemaColumn() {return m_pSchemaColumn;}
		virtual bool isUnique(Opt::Environment& cEnvironment_);
		virtual bool isColumn() {return true;}
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_);
		virtual void addField(Interface::IFile* pFile_,
							  Utility::FieldSet& cFieldSet_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();
		virtual bool isKnownNotNull(Opt::Environment& cEnvironment_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
		virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
											  Interface::IScalar* pOption_);
		virtual Interface::IScalar* convertFunction(Opt::Environment& cEnvironment_,
													Interface::IRelation* pRelation_,
													Interface::IScalar* pFunction_,
													Schema::Field::Function::Value eFunction_);
		virtual int getPosition(Interface::IRelation* pRelation_);

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

	protected:
		SchemaColumn();
		void setArgument(const DataType& cDataType_,
						 Schema::Column* pSchemaColumn_,
						 Relation::Table* pTable_);
	private:
		// get delayability
		static bool getDelayable(Schema::Column* pSchemaColumn_,
								 Relation::Table* pTable_);

		Schema::Column* m_pSchemaColumn;
	};

	//////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::RowID -- implementation class of Field interface for rowid column
	//
	// NOTES

	class RowID
		: public SchemaColumn
	{
	public:
		typedef RowID This;
		typedef SchemaColumn Super;

		// constructor
		RowID(Schema::Column* pSchemaColumn_,
			  Relation::Table* pTable_)
			: Super(pSchemaColumn_, pTable_)
		{}
		// destructor
		virtual ~RowID() {}

	///////////////////
	// Field::
		virtual Scalar::Field* getDelayedKey(Opt::Environment& cEnvironment_);

	protected:
	///////////////////
	// Field::
	//	virtual int generateThis(Opt::Environment& cEnvironment_,
	//							 Execution::Interface::IProgram& cProgram_,
	//							 Execution::Interface::IIterator* pIterator_,
	//							 Candidate::AdoptArgument& cArgument_,
	//							 int iDataID_);
	};

	//////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::LobColumn -- implementation class of Field interface for lob column
	//
	// NOTES

	class LobColumn
		: public SchemaColumn
	{
	public:
		typedef LobColumn This;
		typedef SchemaColumn Super;

		// constructor
		LobColumn(Schema::Column* pSchemaColumn_,
				  Relation::Table* pTable_)
			: Super(pSchemaColumn_, pTable_)
		{}
		// destructor
		virtual ~LobColumn() {}

	///////////////////
	// Field::
		virtual bool isLob() {return true;}

	///////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

	protected:
	///////////////////
	// Field::

	/////////////////////////
	// Interface::IScalar::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);

	private:
		// use locator?
		bool isUseLocator(Opt::Environment& cEnvironment_,
						  Candidate::AdoptArgument& cArgument_);
		// get locatorID
		int getLocator(Opt::Environment& cEnvironment_,
					   Execution::Interface::IIterator* pIterator_,
					   Candidate::AdoptArgument& cArgument_);
	};

	/////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::SchemaField -- implementation class of Field interface for schema::field
	//
	// NOTES

	class SchemaField
		: public Base
	{
	public:
		typedef SchemaField This;
		typedef Base Super;

		// constructor
		SchemaField(Schema::Field* pSchemaField_,
					Interface::IFile* pFile_,
					Relation::Table* pTable_,
					Scalar::Field* pColumn_);

		SchemaField(Schema::Field* pSchemaField_,
					Interface::IFile* pFile_,
					Relation::Table* pTable_,
					Scalar::Field* pColumn_,
					const DataType& cDataType_);
		
		// destructor
		virtual ~SchemaField() {}

	///////////////////
	// Field::
		virtual Interface::IFile* getFile() {return m_pFile;}
		virtual Scalar::Field* getField(Interface::IFile* pFile_);
		virtual Schema::Field* getSchemaField() {return m_pSchemaField;}
		virtual bool isColumn() {return false;}
		virtual bool isDelayed(Candidate::Table* pCandidate_);
		virtual bool isPutKey();
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual bool isUnique(Opt::Environment& cEnvironment_);
		virtual bool isObjectID();
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_);
		virtual bool isExpandElement();
//		virtual Field* translateToBitSetField(Opt::Environment& cEnvironment_);

		virtual void addLocator(Opt::Environment& cEnvironment_,
								Execution::Interface::IIterator* pIterator_,
								int iLocatorID_);
		virtual int getLocator(Opt::Environment& cEnvironment_,
							   Execution::Interface::IIterator* pIterator_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();
		virtual bool isKnownNull(Opt::Environment& cEnvironment_);
		virtual bool isKnownNotNull(Opt::Environment& cEnvironment_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
											  Interface::IScalar* pOption_);

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

	protected:
		SchemaField()
			: Super(),
			  m_pFile(0),
			  m_pSchemaField(0),
			  m_pColumn(0),
			  m_cstrValue()
		{}
		void setArgument(const DataType& cDataType_,
						 Schema::Field* pSchemaField_,
						 Interface::IFile* pFile_,
						 Relation::Table* pTable_,
						 Scalar::Field* pColumn_);
	private:
		// get delayability
		static bool getDelayable(Schema::Field* pSchemaField_,
								 Relation::Table* pTable_,
								 Scalar::Field* pColumn_);

		Interface::IFile* m_pFile;
		Schema::Field* m_pSchemaField;
		Scalar::Field* m_pColumn;
		ModUnicodeString m_cstrValue;
	};


    /////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::BitSetField -- implementation class of Field interface for schema::field
	//
	// NOTES

	class BitSetField
		: public SchemaField
	{
	public:
		typedef BitSetField This;
		typedef SchemaField Super;

		// constructor
		BitSetField(Schema::Field* pSchemaField_,
					Interface::IFile* pFile_,
					Relation::Table* pTable_,
					Scalar::Field* pColumn_);

		// destructor
		~BitSetField() {}


	/////////////////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		bool isBitSetRowID() { return true; }

	protected:
	private:
	};

	//////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::SystemColumn -- implementation class of system column interface
	//
	// NOTES

	class SystemColumn
		: public SchemaColumn
	{
	public:
		typedef SystemColumn This;
		typedef SchemaColumn Super;

		// constructor
		SystemColumn(Schema::Column* pSchemaColumn_,
					   Field* pRowID_,
					   Relation::Table* pTable_)
			: Super(pSchemaColumn_, pTable_),
			  m_pRowID(pRowID_)
		{
			setDelayable(false);
		}
		// destructor
		~SystemColumn() {}

	protected:
	///////////////////
	// Field::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		Field* m_pRowID;
	};

	/////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::FunctionField -- implementation class of Field interface for function field
	//
	// NOTES

	class FunctionField
		: public Base
	{
	public:
		typedef FunctionField This;
		typedef Base Super;

		// constructor
		FunctionField(Schema::Field::Function::Value eFunction_,
					  const Utility::FileSet& cFileSet_,
					  Relation::Table* pTable_,
					  Interface::IScalar* pFunction_,
					  Interface::IScalar* pOperand_);
		// destructor
		~FunctionField() {}

		// create FileSet for function field
		static void createFunctionFile(Opt::Environment& cEnvironment_,
									   Schema::Field* pSchemaField_,
									   Utility::FileSet& cFileSet_,
									   Relation::Table* pTable_,
									   Interface::IScalar* pOperand_);

	///////////////////
	// Field::
		virtual Interface::IFile* getFile() {return 0;}
		virtual Scalar::Field* getField(Interface::IFile* pFile_);
		virtual Schema::Field* getSchemaField() {return 0;}
		virtual Schema::Column* getSchemaColumn() {return 0;}
		virtual bool isUnique(Opt::Environment& cEnvironment_) {return false;}
		virtual bool isColumn() {return false;}
		virtual bool isDelayed(Candidate::Table* pCandidate_) {return false;}
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_);
		virtual Interface::IScalar* getAlternativeValue(Opt::Environment& cEnvironment_);
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_);
		virtual bool isFunction() {return true;}
		virtual Interface::IScalar* getFunction();
		virtual bool checkAvailability(Interface::IPredicate* pPredicate_);
		virtual void createRetrieveFile(Opt::Environment& cEnvironment_,
										Utility::FileSet& cFileSet_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();
		virtual bool hasField(Interface::IFile* pFile_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
		virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
											  Interface::IScalar* pOption_);

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

	protected:
	///////////////////
	// Field::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
	private:
		Schema::Field::Function::Value m_eFunction;
		Utility::FileSet m_cFileSet;
		Interface::IScalar* m_pOperand;
		Interface::IScalar* m_pFunction;
	};

	/////////////////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::GetMax -- implementation class of Field interface for get max field
	//
	// NOTES

	class GetMax
		: public Tree::Nadic<Base, Scalar::Field>
	{
	public:
		typedef GetMax This;
		typedef Tree::Nadic<Base, Scalar::Field> Super;

		// constructor
		GetMax(const VECTOR<Scalar::Field*>& vecOperand_,
			   Relation::Table* pTable_,
			   Interface::IScalar* pNullData_,
			   const STRING& cstrName_);
		// destructor
		~GetMax() {}

	///////////////////
	// Field::
		virtual Interface::IFile* getFile() {return 0;}
		virtual Scalar::Field* getField(Interface::IFile* pFile_);
		virtual Schema::Field* getSchemaField() {return 0;}
		virtual Schema::Column* getSchemaColumn() {return 0;}
		virtual bool isUnique(Opt::Environment& cEnvironment_) {return false;}
		virtual bool isColumn() {return false;}
		virtual bool isDelayed(Candidate::Table* pCandidate_) {return false;}
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual bool hasAlternativeValue(Opt::Environment& cEnvironment_);
		virtual Interface::IScalar* getAlternativeValue(Opt::Environment& cEnvironment_);
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_);
		virtual bool isFunction() {return true;}
		virtual Interface::IScalar* getFunction();
		virtual bool isChoice() {return true;}
		virtual bool checkAvailability(Interface::IPredicate* pPredicate_);
		virtual void createRetrieveFile(Opt::Environment& cEnvironment_,
										Utility::FileSet& cFileSet_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
		virtual bool hasField(Interface::IFile* pFile_);
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
		virtual Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
											  Interface::IScalar* pOption_);

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

	protected:
	///////////////////
	// Field::
		virtual int generateData(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	private:
		Interface::IScalar* m_pNullData;
		STRING m_cstrName;
	};

	//////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Scalar::FieldImpl::OptionBase -- implementation class of column interface with option
	//
	// TEMPLATE ARGUMENTS
	//	class Base_
	//
	// NOTES

	template <class Base_>
	class OptionBase
		: public Tree::MonadicOption<Base_, Interface::IScalar>
	{
	public:
		typedef OptionBase<Base_> This;
		typedef Tree::MonadicOption<Base_, Interface::IScalar> Super;

		// constructor
		OptionBase(Scalar::Field* pColumn_,
				   Interface::IScalar* pOption_)
			: Super(pOption_),
			  m_pColumn(pColumn_)
		{
			setDelayable(m_pColumn->isDelayable() &&
						 isArbitraryElement() == false);
		}
		// destructor
		~OptionBase() {}

	/////////////////////////////
	// Scalar::Field::
		virtual void addField(Interface::IFile* pFile_,
							  Utility::FieldSet& cFieldSet_)
		{
			if (isArbitraryElement()) {
				m_pColumn->addField(pFile_,
									cFieldSet_);
			} else {
				Super::addField(pFile_,
								cFieldSet_);
			}
		}

	/////////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_)
		{
			if (isArbitraryElement()
				||isExpandElement()) {
				m_pColumn->explain(pEnvironment_, cExplain_);
				cExplain_.put("[]");
			} else {
				Super::explain(pEnvironment_, cExplain_);
			}
		}
		virtual bool isKnownNull(Opt::Environment& cEnvironment_)
		{
			if (isArbitraryElement()) {
				return m_pColumn->isKnownNull(cEnvironment_);
			} else {
				return Super::isKnownNull(cEnvironment_);
			}
		}
		virtual bool isKnownNotNull(Opt::Environment& cEnvironment_)
		{
			if (isArbitraryElement()) {
				return m_pColumn->isKnownNotNull(cEnvironment_);
			} else {
				return Super::isKnownNotNull(cEnvironment_);
			}
		}
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			if (isArbitraryElement()) {
				m_pColumn->require(cEnvironment_, pCandidate_);
			} else {
				Super::require(cEnvironment_, pCandidate_);
			}
		}
		virtual void retrieve(Opt::Environment& cEnvironment_)
		{
			if (isArbitraryElement()) {
				m_pColumn->retrieve(cEnvironment_);
			} else {
				Super::retrieve(cEnvironment_);
			}
		}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_)
		{
			if (isArbitraryElement()) {
				m_pColumn->retrieve(cEnvironment_, pCandidate_);
			} else {
				Super::retrieve(cEnvironment_, pCandidate_);
			}
		}
		virtual void use(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_)
		{
			if (isArbitraryElement()) {
				m_pColumn->use(cEnvironment_, pCandidate_);
			} else {
				Super::use(cEnvironment_, pCandidate_);
			}
		}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_)
		{
			if (isArbitraryElement()) {
				return m_pColumn->delay(cEnvironment_, pCandidate_, cArgument_);
			} else {
				return Super::delay(cEnvironment_, pCandidate_, cArgument_);
			}
		}

		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_)
		{
			if (isArbitraryElement()) {
				return m_pColumn->generate(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cArgument_);
			} else {
				return Super::generate(cEnvironment_,
									   cProgram_,
									   pIterator_,
									   cArgument_);
			}
		}

		virtual int generateFromType(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_)
		{
			if (isArbitraryElement()) {
				return m_pColumn->generateFromType(cEnvironment_,
												   cProgram_,
												   pIterator_,
												   cArgument_);
			} else {
				return Super::generateFromType(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_);
			}
		}

		virtual bool isArbitraryElement()
		{
			return getOption()
				&& getOption()->getType() == Tree::Node::All;
		}


		virtual bool isExpandElement()
		{
			return getOption()
				&& getOption()->getType() == Tree::Node::Expand;
		}		

		// register to environment
		void registerToEnvironment(Opt::Environment& cEnvironment_)
		{
			if (isArbitraryElement()||isExpandElement()) {
				// skip Field's implementation
				Interface::IScalar::registerToEnvironment(cEnvironment_);
			} else {
				Super::registerToEnvironment(cEnvironment_);
			}
		}

		using Super::getOption;
		using Super::setDelayable;

	protected:
	private:
		Scalar::Field* m_pColumn;


	};

	//////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::OptionColumn -- implementation class of column interface with option
	//
	// NOTES

	class OptionColumn
		: public OptionBase<SchemaColumn>
	{
	public:
		typedef OptionColumn This;
		typedef OptionBase<SchemaColumn> Super;

		// constructor
		OptionColumn(Schema::Column* pSchemaColumn_,
					 Relation::Table* pTable_,
					 Scalar::Field* pColumn_,
					 Interface::IScalar* pOption_);
		// destructor
		~OptionColumn() {}

	protected:
	private:
	};

	//////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::OptionField -- implementation class of field interface with option
	//
	// NOTES

	class OptionField
		: public OptionBase<SchemaField>
	{
	public:
		typedef OptionField This;
		typedef OptionBase<SchemaField> Super;

		// constructor
		OptionField(Schema::Field* pSchemaField_,
					Interface::IFile* pFile_,
					Relation::Table* pTable_,
					Scalar::Field* pColumn_,
					Interface::IScalar* pOption_);
		// destructor
		~OptionField() {}

	protected:
	private:
	};
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_FIELDIMPL_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
