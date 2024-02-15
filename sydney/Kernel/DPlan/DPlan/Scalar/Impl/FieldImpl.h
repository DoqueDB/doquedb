// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/FieldImpl.h --
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

#ifndef __SYDNEY_DPLAN_SCALAR_FIELDIMPL_H
#define __SYDNEY_DPLAN_SCALAR_FIELDIMPL_H

#include "DPlan/Scalar/Field.h"

#include "Plan/Scalar/Impl/FieldImpl.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

namespace FieldImpl
{
	//////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::FieldImpl::Base -- base class for Field interface
	//
	// NOTES

	class Base
		: public Plan::Scalar::FieldImpl::Base
	{
	public:
		typedef Base This;
		typedef Plan::Scalar::FieldImpl::Base Super;

		// register to environment
		void registerToEnvironment(Opt::Environment& cEnvironment_);

	protected:
		// constructor
		Base()
			: Super()
		{}
		Base(const Plan::Scalar::DataType& cDataType_,
			 Plan::Relation::Table* pTable_,
			 bool bDelayable_,
			 bool bRowID_)
			: Super(cDataType_,
					pTable_,
					bDelayable_,
					bRowID_)
		{}
	private:
	};

	//////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::FieldImpl::SchemaColumn -- implementation class of Field interface for schema::column
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
					 Plan::Relation::Table* pTable_);

		// destructor
		virtual ~SchemaColumn() {}

	///////////////////////////
	// Plan::Scalar::Field::
		virtual Schema::Field* getSchemaField() {return 0;}
		virtual Schema::Column* getSchemaColumn() {return m_pSchemaColumn;}
		virtual bool isUnique(Opt::Environment& cEnvironment_);
		virtual bool isColumn() {return true;}
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_);
		virtual void addField(Plan::Interface::IFile* pFile_,
							  Plan::Utility::FieldSet& cFieldSet_);

	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();
		virtual void setMetaData(Opt::Environment& cEnvironment_,
								 Common::ColumnMetaData& cMetaData_);
		virtual Plan::Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
													Plan::Interface::IScalar* pOption_);

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

	///////////////////////////
	// Interface::ISqlNode::			
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

		
	protected:
		SchemaColumn();

		void setArgument(const Plan::Scalar::DataType& cDataType_,
						 Schema::Column* pSchemaColumn_,
						 Plan::Relation::Table* pTable_);
	/////////////////////////////////////
	// Interface::IScalar::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Plan::Candidate::AdoptArgument& cArgument_,
								 int iDataID_);

	private:
		Schema::Column* m_pSchemaColumn;
		STRING m_cstrTableName;
		STRING m_cstrColumnName;
	};


		//////////////////////////////////////
	// CLASS
	//	DPlan::Scalar::FieldImpl::FunctionField -- implementation class of Field interface for function
	//
	// NOTES

	class FunctionField
		: public Base
	{
	public:
		typedef FunctionField This;
		typedef Base Super;

		// constructor
		FunctionField(Plan::Relation::Table* pTable_,
					  Plan::Interface::IScalar* pFunction_);
		// destructor
		virtual ~FunctionField() {}
		
	///////////////////////////
	// Tree::Node
		virtual int getID() const;
		

	///////////////////////////
	// Plan::Scalar::Field::
		virtual Schema::Field* getSchemaField() {return 0;}
		virtual Schema::Column* getSchemaColumn() {return 0;}
		virtual bool isUnique(Opt::Environment& cEnvironment_) { return false;}
		virtual bool isColumn() {return false;}
		virtual bool isRowIDAvailable(Opt::Environment& cEnvironment_);
		virtual Field* createFetchKey(Opt::Environment& cEnvironment_) { return 0;}
		virtual bool isFunction() {return true;}		
		virtual Plan::Interface::IScalar* getFunction() {return m_pFunction;}
	/////////////////////////////////////
	// Interface::IScalar::
		virtual const STRING& getName();

		virtual Plan::Interface::IScalar* addOption(Opt::Environment& cEnvironment_,
													Plan::Interface::IScalar* pOption_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Plan::Candidate::AdoptArgument& cArgument_);		

	/////////////////////////////////////
	// Node::
		virtual ModUnicodeString getValue() const;

		
	protected:
		
	/////////////////////////////////////
	// Interface::IScalar::
		virtual int generateThis(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Plan::Candidate::AdoptArgument& cArgument_,
								 int iDataID_);
	protected:
		FunctionField();		

	private:
		Plan::Interface::IScalar* m_pFunction;
		Plan::Interface::IScalar* m_pConvertedForDist;				
	};
	
	//////////////////////////////
	// CLASS
	//	Plan::Scalar::FieldImpl::OptionColumn -- implementation class of column interface with option
	//
	// NOTES

	class OptionColumn
		: public Plan::Scalar::FieldImpl::OptionBase<SchemaColumn>
	{
	public:
		typedef OptionColumn This;
		typedef Plan::Scalar::FieldImpl::OptionBase<SchemaColumn> Super;

		// constructor
		OptionColumn(Schema::Column* pSchemaColumn_,
					 Plan::Relation::Table* pTable_,
					 Plan::Scalar::Field* pColumn_,
					 Plan::Interface::IScalar* pOption_);
		// destructor
		~OptionColumn() {}

		/////////////////////////////
		// Plan::Interface::ISqlNode
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec_,
								  const Plan::Sql::QueryArgument& cArgument_);
				
	protected:
	private:
	};
}

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_SCALAR_FIELDIMPL_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
