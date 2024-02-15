// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/TableImpl.h --
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

#ifndef __SYDNEY_PLAN_RELATION_TABLEIMPL_H
#define __SYDNEY_PLAN_RELATION_TABLEIMPL_H

#include "boost/bind.hpp"

#include "Plan/Relation/Table.h"
#include "Plan/Utility/ObjectSet.h"

#include "Opt/Algorithm.h"
#include "Opt/Argument.h"

#include "Schema/ObjectID.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class Constraint;
	class Field;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace
{
	struct _CreateTargetFieldArgument;
}

namespace TableImpl
{
	///////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Relation::TableImpl::Base -- base class of implementation of table interface
	//
	// NOTES
	class Base
		: public Relation::Table
	{
	public:
		typedef Base This;
		typedef Relation::Table Super;

		// constructor
		Base()
			: Super(),
			  m_mapColumnField(),
			  m_mapSchemaField()
		{}
		Base(Schema::Table* pSchemaTable_)
			: Super(pSchemaTable_),
			  m_mapColumnField(),
			  m_mapSchemaField()
		{}

		// destructor
		virtual ~Base() {}

	//////////////////////////////
	// Relation::Table::
		virtual void addField(Scalar::Field* pField_);
		virtual Scalar::Field* getField(Schema::Object* pSchemaObject_);
		virtual Scalar::Field* getField(Schema::Object* pSchemaObject_,
										Interface::IScalar* pOption_);

		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pColumn_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_);

		virtual Scalar::Field* getField(Interface::IFile* pFile_,
										Scalar::Field* pColumn_);

		virtual void addRetrieved(Scalar::Field* pField_);

	/////////////////////////////
	// Interface::IRelation::
	//	virtual Interface::ICandidate*
	//				createAccessPlan(Opt::Environment& cEnvironment_,
	//								 AccessPlan::Source& cPlanSource_);
	//	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
	//								  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);

		
	protected:
		// check whether schema object is added
		bool hasField(Schema::Object* pSchemaObject_);

	private:
		typedef MAP< Interface::IFile*,
					 MAP< Scalar::Field*,
						  Scalar::Field*,
						  Utility::ReferingLess<Scalar::Field> >,
					 Utility::ReferingLess<Interface::IFile> > ColumnFieldMap;

		typedef MAP< Schema::Object*,
					 Scalar::Field*,
					 Utility::SchemaLess<Schema::Object> > SchemaFieldMap;

		typedef MAP< int,
					 MAP< Schema::Object*,
						  Scalar::Field*,
						  Utility::SchemaLess<Schema::Object> >,
					 LESS<int> > ColumnOptionMap;

		ColumnFieldMap m_mapColumnField;
		SchemaFieldMap m_mapSchemaField;
		ColumnOptionMap m_mapColumnOption;
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Relation::TableImpl::Retrieve -- implementation of table interface
	//
	// NOTES
	class Retrieve
		: public Base
	{
	public:
		typedef Retrieve This;
		typedef Base Super;
		using Super::getField;

		// constructor
		Retrieve(Schema::Table* pSchemaTable_)
			: Super(pSchemaTable_),
			  m_pEstimateFile(0),
			  m_pEstimatePredicate(0),
			  m_cEstimateCount(),
			  m_pAdoptCandidate(0),
			  m_cRetrievedColumn()
		{}

		// destructor
		virtual ~Retrieve() {}

	//////////////////////////////
	// Relation::Table::
		// estimating
		virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
		virtual Interface::IFile* getEstimateFile();
		virtual void resetEstimateFile();

		virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
		virtual Interface::IPredicate* getEstimatePredicate();
		virtual void resetEstimatePredicate();

		virtual bool isEstimating();
		virtual Scalar::Field* getField(Scalar::Field* pColumn_);

		virtual void addRetrieved(Scalar::Field* pField_);

		virtual void setEstimateCount(const AccessPlan::Cost::Value& cValue_);
		virtual AccessPlan::Cost::Value getEstimateCount();

		// adopting
		virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
		virtual Candidate::Table* getAdoptCandidate();

		// get retrieved columns
		const Utility::FieldSet& getRetrievedColumn() {return m_cRetrievedColumn;}

	/////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
							createAccessPlan(Opt::Environment& cEnvironment_,
											 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);

		
	protected:
	private:
	//////////////////////////
	// Interface::IRelation
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);

		bool checkDistinct(Opt::Environment& cEnvironment_,
						   Utility::RowElementSet& cKey_);

		Interface::IFile* m_pEstimateFile;
		Interface::IPredicate* m_pEstimatePredicate;
		AccessPlan::Cost::Value m_cEstimateCount;
		Candidate::Table* m_pAdoptCandidate;
		Utility::FieldSet m_cRetrievedColumn;
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Relation::TableImpl::Refer -- implementation of table interface
	//
	// NOTES
	class Refer
		: public Retrieve
	{
	public:
		typedef Refer This;
		typedef Retrieve Super;
		using Super::getField;

		// constructor
		Refer(Schema::Table* pSchemaTable_,
			  Relation::Table* pTargetTable_)
			: Super(pSchemaTable_),
			  m_pTargetTable(pTargetTable_)
		{}

		// destructor
		virtual ~Refer() {}

	//////////////////////////////
	// Relation::Table::

	/////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
							createAccessPlan(Opt::Environment& cEnvironment_,
											 AccessPlan::Source& cPlanSource_);
		virtual const STRING& getCorrelationName(Opt::Environment& cEnvironment_);

	protected:
	private:
		Relation::Table* m_pTargetTable; // other table refering the table
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Put -- base class of implementation class for put operations
	//
	// NOTES
	class Put
		: public Base
	{
	public:
		typedef Base Super;
		typedef Put This;

		Put(Schema::Table* pSchemaTable_,
			Interface::IRelation* pOperand_)
			: Super(pSchemaTable_),
			  m_pOperand(pOperand_),
			  m_pAdoptCandidate(0),
			  m_mapInput()
		{}
		~Put() {}

	/////////////////////////////
	// Relation::Table::
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
		virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
		virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addRetrieved(Scalar::Field* pField_);

		virtual void addInput(Opt::Environment& cEnvironment_,
							  Position iPosition_,
							  Interface::IScalar* pSource_);
		virtual Interface::IScalar*
					getInput(Opt::Environment& cEnvironment_,
							 Position iPosition_);

		virtual RowInfo* getGeneratedColumn(Opt::Environment& cEnvironment_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	protected:
		typedef MAP<Schema::Table*,
					Candidate::Table*,
					Utility::SchemaTableSet::Comparator> ReferMap;

		// accessor
		Interface::IRelation* getOperand() {return m_pOperand;}

		// check whether a column can be specified value
		bool isExplicitValueAllowed(Opt::Environment& cEnvironment_,
									Schema::Column* pSchemaColumn_);
		// check whether input data can be default value
		bool isConsiderDefault(Opt::Environment& cEnvironment_,
							   Interface::IScalar* pInput_);
		// check whether a column value is generated
		bool isValueGenerated(Schema::Column* pSchemaColumn_);
		// get iscalar denoting default/generated value for the i-th column
		Interface::IScalar* getDefault(Opt::Environment& cEnvironment_,
									   Schema::Column* pSchemaColumn_,
									   Interface::IScalar* pInput_,
									   bool bForInsert_);
		// create input scalar data when data is specified
		Interface::IScalar* createInputData(Opt::Environment& cEnvironment_,
											Schema::Column* pSchemaColumn_,
											Interface::IScalar* pInput_,
											Interface::IScalar* pDefault_);

		// has input data?
		bool hasInput(Opt::Environment& cEnvironment_,
					  const Schema::Column* pSchemaColumn_);
		// add position of logged column
		void addLogColumn(Opt::Environment& cEnvironment_,
						  const ModVector<Schema::Column*>& vecSchemaColumn_,
						  Utility::UIntSet& cLogRequiredPosition_);
		// add reference to other table according to constraints
		void addReferenceByForeignKey(Opt::Environment& cEnvironment_,
									  VECTOR<Candidate::Table*>& vecReference_);
		void addReferenceByReferedKey(Opt::Environment& cEnvironment_,
									  Interface::ICandidate* pOperandCandidate_,
									  Relation::Table* pRetrieve_,
									  VECTOR<Candidate::Table*>& vecReference_);

		virtual bool isInsert() {return false;}
		virtual bool isDelete() {return false;}
		// get rowelement for put result of identity
		Relation::RowElement* getIdentity(Opt::Environment& cEnvironment_);
		// get rowelement for put result of current_timestamp
		Relation::RowElement* getCurrentTimestamp(Opt::Environment& cEnvironment_);
		
	private:
		// get column type of i-th field
		virtual Scalar::DataType getColumnType(Opt::Environment& cEnvironment_,
											   Position iPosition_);
		// get operation type


		// get rowelement for put result of rowid
		virtual Relation::RowElement* getRowID(Opt::Environment& cEnvironment_);

		// check whether a column should be added to put result
		bool isAddToPutResult(Opt::Environment& cEnvironment_,
							  const Schema::Column* pSchemaColumn_);

		// create refer relation corresponding to a constraint
		void addReference(Opt::Environment& cEnvironment_,
						  Schema::Constraint* pSchemaConstraint_,
						  ReferMap& cMap_);
		// get refer relations corresponding to constraints
		void getReference(const ReferMap& cMap_,
						  VECTOR<Candidate::Table*>& vecReference_);
		// require key fields for refered constraint
		void requireReferedKey(Opt::Environment& cEnvironment_,
							   Schema::Constraint* pSchemaConstraint_,
							   Interface::ICandidate* pOperandCandidate_,
							   Relation::Table* pRetrieve_);

	//////////////////////////
	// Interface::IRelation
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);

		typedef MAP<Position,
					Interface::IScalar*,
					LESS<Position> > InputMap;
		Interface::IRelation* m_pOperand;
		Candidate::Table* m_pAdoptCandidate;
		InputMap m_mapInput;
	};

	///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Insert -- implementation class of Insert
	//
	// NOTES
	class Insert
		: public Put
	{
	public:
		typedef Put Super;
		typedef Insert This;

		Insert(Schema::Table* pSchemaTable_,
			   Interface::IRelation* pOperand_)
			: Super(pSchemaTable_, pOperand_)
		{}
		~Insert() {}

	/////////////////////////////
	// Relation::Table::
		using Super::addField;
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pColumn_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_);
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
	//	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	//	virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addInput(Opt::Environment& cEnvironment_,
	//						  Position iPosition_,
	//						  Interface::IScalar* pSource_);
	//	virtual Interface::IScalar*
	//				getInput(Opt::Environment& cEnvironment_,
	//						 Position iPosition_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 AccessPlan::Source& cPlanSource_);
	protected:
		virtual bool isInsert() {return true;}
	//	virtual bool isDelete();
		
	private:
		void createTargetField(Opt::Environment& cEnvironment_,
							   Utility::FieldSet& cTarget_,
							   VECTOR<Scalar::Field*>& vecLogged_);

	/////////////////////////////////
	// Relation::TableImpl::Base::

	};

	///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Delete -- implementation class of Delete
	//
	// NOTES
	class Delete
		: public Put
	{
	public:
		typedef Put Super;
		typedef Delete This;

		Delete(Relation::Table* pRetrieve_,
			   Interface::IRelation* pOperand_)
			: Super(pRetrieve_->getSchemaTable(),
					pOperand_),
			  m_pRetrieve(pRetrieve_)
		{}
		~Delete() {}

	/////////////////////////////
	// Relation::Table::
		using Super::addField;
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pColumn_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_);
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
	//	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	//	virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addInput(Opt::Environment& cEnvironment_,
	//						  Position iPosition_,
	//						  Interface::IScalar* pSource_);
	//	virtual Interface::IScalar*
	//				getInput(Opt::Environment& cEnvironment_,
	//						 Position iPosition_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 AccessPlan::Source& cPlanSource_);
	protected:
		virtual bool isDelete() {return true;}
		
	private:
		void createTargetField(Opt::Environment& cEnvironment_,
							   Interface::ICandidate* pOperandCandidate_,
							   Utility::FieldSet& cTarget_,
							   VECTOR<Scalar::Field*>& vecLogged_);

	/////////////////////////////////
	// Relation::TableImpl::Put::
	//	virtual bool isInsert();

		virtual Relation::RowElement* getRowID(Opt::Environment& cEnvironment_);

		Relation::Table* m_pRetrieve;
	};

	///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Update -- implementation class of Update
	//
	// NOTES
	class Update
		: public Put
	{
	public:
		typedef Put Super;
		typedef Update This;

		Update(Relation::Table* pRetrieve_,
			   Interface::IRelation* pOperand_)
			: Super(pRetrieve_->getSchemaTable(),
					pOperand_),
			  m_pRetrieve(pRetrieve_)
		{}
		~Update() {}

	/////////////////////////////
	// Relation::Table::
		using Super::addField;
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pColumn_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_);
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
	//	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	//	virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addInput(Opt::Environment& cEnvironment_,
	//						  Position iPosition_,
	//						  Interface::IScalar* pSource_);
	//	virtual Interface::IScalar*
	//				getInput(Opt::Environment& cEnvironment_,
	//						 Position iPosition_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 AccessPlan::Source& cPlanSource_);
	protected:
	private:
		void createTargetField(Opt::Environment& cEnvironment_,
							   Interface::ICandidate* pOperandCandidate_,
							   Utility::FieldSet& cTarget_,
							   VECTOR<Scalar::Field*>& vecLogged_,
							   Utility::UIntSet& cLogRequiredPosition_);
		void setLogColumn(Opt::Environment& cEnvironment_,
						  Utility::UIntSet& cLogRequiredPosition_);

		bool createTargetFieldForRecovery(Opt::Environment& cEnvironment_,
										  Schema::Column* pSchemaColumn_,
										  _CreateTargetFieldArgument& cArgument_);
		bool createTargetFieldForNormal(Opt::Environment& cEnvironment_,
										Schema::Column* pSchemaColumn_,
										_CreateTargetFieldArgument& cArgument_);
		void createTargetFieldForBoth(Opt::Environment& cEnvironment_,
									  Schema::Column* pSchemaColumn_,
									  Interface::ICandidate* pOperandCandidate_,
									  _CreateTargetFieldArgument& cArgument_);

	/////////////////////////////////
	// Relation::TableImpl::Put::
		virtual Relation::RowElement* getRowID(Opt::Environment& cEnvironment_);

		Relation::Table* m_pRetrieve;
	};

	///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Import -- implementation class of Import
	//
	// NOTES
	class Import
		: public Put
	{
	public:
		typedef Put Super;
		typedef Import This;

		Import(const Opt::ImportArgument& cArgument_,
			   Interface::IRelation* pOperand_)
			: Super(cArgument_.m_pTargetTable, pOperand_),
			  m_cArgument(cArgument_)
		{}
		~Import() {}

	/////////////////////////////
	// Relation::Table::
		using Super::addField;
		virtual void addField(Opt::Environment& cEnvironment_,
							  Scalar::Field* pColumn_,
							  Interface::IFile* pFile_,
							  Scalar::Field* pField_);
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
	//	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	//	virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addInput(Opt::Environment& cEnvironment_,
	//						  Position iPosition_,
	//						  Interface::IScalar* pSource_);
	//	virtual Interface::IScalar*
	//				getInput(Opt::Environment& cEnvironment_,
	//						 Position iPosition_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 AccessPlan::Source& cPlanSource_);
	protected:
	//	virtual bool isInsert();
	//	virtual bool isDelete();
		
	private:
		// create scalar node providing input value
		Interface::IScalar* createInput(Opt::Environment& cEnvironment_,
										Scalar::Field* pTargetField_);
		Scalar::Field* createField(Opt::Environment& cEnvironment_,
								   Schema::Field* pSchemaField_);

	/////////////////////////////////
	// Relation::Table::
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
	/////////////////////////////////
	// Relation::TableImpl::Base::

		virtual Scalar::DataType getColumnType(Opt::Environment& cEnvironment_,
											   Position iPosition_);

		Opt::ImportArgument m_cArgument;
	};

	///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Undo -- implementation class of Undo
	//
	// NOTES
	class Undo
		: public Put
	{
	public:
		typedef Put Super;
		typedef Undo This;

		Undo(Schema::Table* pSchemaTable_,
			 const Common::DataArrayData* pUndoLog_)
			: Super(pSchemaTable_, 0),
			  m_pUndoLog(pUndoLog_)
		{}
		~Undo() {}

	/////////////////////////////
	// Relation::Table::
	//	virtual void addField(Opt::Environment& cEnvironment_,
	//						  Scalar::Field* pColumn_,
	//						  Interface::IFile* pFile_,
	//						  Scalar::Field* pField_);
	//	virtual AutoReset setEstimateFile(Interface::IFile* pFile_);
	//	virtual void resetEstimateFile();
	//	virtual AutoReset setEstimatePredicate(Interface::IPredicate* pPredicate_);
	//	virtual void resetEstimatePredicate();
	//	virtual bool isEstimating();
	//	virtual void setAdoptCandidate(Candidate::Table* pCandidate_);
	//	virtual Candidate::Table* getAdoptCandidate();
	//	virtual void addInput(Opt::Environment& cEnvironment_,
	//						  Position iPosition_,
	//						  Interface::IScalar* pSource_);
	//	virtual Interface::IScalar*
	//				getInput(Opt::Environment& cEnvironment_,
	//						 Position iPosition_);

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 AccessPlan::Source& cPlanSource_);
	protected:
	private:
		const Common::DataArrayData* m_pUndoLog;
	};
} // namespace Impl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_TABLEIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
