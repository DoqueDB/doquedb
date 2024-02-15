// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/TableImpl.h --
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

#ifndef __SYDNEY_DPLAN_RELATION_TABLEIMPL_H
#define __SYDNEY_DPLAN_RELATION_TABLEIMPL_H

#include "boost/bind.hpp"

#include "DPlan/Relation/Table.h"

#include "Plan/Relation/Impl/TableImpl.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/BitSet.h"

#include "Opt/Algorithm.h"
#include "Opt/Argument.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace TableImpl
{
	///////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Relation::TableImpl::Base -- base class of implementation of table interface
	//
	// NOTES
	class Base
		: public Plan::Relation::TableImpl::Base
	{
	public:
		typedef Base This;
		typedef Plan::Relation::TableImpl::Base Super;

		// constructor
		Base()
			: Super()
		{}
		Base(Schema::Table* pSchemaTable_)
			: Super(pSchemaTable_)
		{}

		// destructor
		virtual ~Base() {}
		
		
		// register to environment
		void registerToEnvironment(Opt::Environment& cEnvironment_);

	/////////////////////////////
	// Relation::Table::
		virtual Plan::Scalar::Field* createField(Opt::Environment& cEnvironment_,
												 Schema::Column* pSchemaColumn_);
		virtual Plan::Interface::IFile* getEstimateFile();

	/////////////////////////////////
	// Plan::Interface::IRelation::
	protected:
	private:
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	DPlan::Relation::TableImpl::Retrieve -- implementation of table interface
	//
	// NOTES
	class Retrieve
		: public Base
	{
	public:
		typedef Retrieve This;
		typedef Base Super;

		// constructor
		Retrieve(Schema::Table* pSchemaTable_)
			: Super(pSchemaTable_),
			  m_cFieldSet()
		{}

		// destructor
		virtual ~Retrieve() {}
		
	//////////////////////////////
	// Relation::Table::
		virtual void addRetrieved(Plan::Scalar::Field* pField_);
		
	/////////////////////////////
	// Interface::IRelation::
		virtual Plan::Interface::ICandidate*
							createAccessPlan(Opt::Environment& cEnvironment_,
											 Plan::AccessPlan::Source& cDPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const Plan::Relation::InquiryArgument& cArgument_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	protected:
	private:
	//////////////////////////
	// Interface::IRelation
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);

		Plan::Utility::FieldSet m_cFieldSet;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Put -- base class of implementation class for put operations
	//
	// NOTES
	class Put
		: public Plan::Relation::TableImpl::Put
	{
	public:
		typedef Plan::Relation::TableImpl::Put Super;
		typedef Put This;

		Put(Schema::Table* pSchemaTable_,
			Plan::Interface::IRelation* pOperand_,
			Plan::Relation::Table* pLocalTable_)
			: Super(pSchemaTable_,
					pOperand_),
			  m_pLocalTable(pLocalTable_)
		{}
		~Put() {}

		// register to environment
		void registerToEnvironment(Opt::Environment& cEnvironment_);

	/////////////////////////////
	// Relation::Table::
		virtual Plan::Scalar::Field* createField(Opt::Environment& cEnvironment_,
												 Schema::Column* pSchemaColumn_);

		virtual void addInput(Opt::Environment& cEnvironment_,
							  Position iPosition_,
							  Plan::Interface::IScalar* pSource_);


	protected:
		// get iscalar denoting default/generated value for the i-th column
		Plan::Interface::IScalar* getDefault(Opt::Environment& cEnvironment_,
											 Schema::Column* pSchemaColumn_,
											 Plan::Interface::IScalar* pInput_,
											 bool bForInsert_);
		// create input scalar data when data is specified
		Plan::Interface::IScalar* createInputData(Opt::Environment& cEnvironment_,
												  Schema::Column* pSchemaColumn_,
												  Plan::Interface::IScalar* pInput_,
												  Plan::Interface::IScalar* pDefault_);
		Plan::Interface::IScalar* createCheck(Opt::Environment& cEnvironment_,
											  Schema::Partition* pRule_,
											  VECTOR<Plan::Scalar::Field*>& cTarget_);

		Plan::Relation::Table* getLocalTable() {return m_pLocalTable;}
	private:
		Plan::Relation::Table* m_pLocalTable;
		Common::BitSet m_cUniqueColumn;

		Plan::Interface::IScalar* getLocalInput(Opt::Environment& cEnvironment_,
												Position iPosition_,
												Plan::Interface::IScalar* pSource_);
	};


	/////////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Delete -- そのまま子サーバーにCascadeするためのクラス
	//
	// NOTES
	class Delete
		: public Base
	{
	public:
		typedef Base Super;
		typedef Delete This;

		Delete(Plan::Relation::Table* pTable_,
				Plan::Interface::IRelation* pOperand_)
			: Super(pTable_->getSchemaTable()),
			  m_cFieldSet(),
			  m_pRetrieve(pOperand_)
		{}
		virtual ~Delete() {}

	//////////////////////////////
	// Relation::Table::
		virtual void addRetrieved(Plan::Scalar::Field* pField_);
		
	/////////////////////////////
	// Interface::IRelation::		
		virtual Plan::Interface::ICandidate*
							createAccessPlan(Opt::Environment& cEnvironment_,
											 Plan::AccessPlan::Source& cDPlanSource_);

		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const Plan::Relation::InquiryArgument& cArgument_);		

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
		
	/////////////////////////////
	// Relation::Table::
		
		virtual Plan::Relation::RowInfo* getGeneratedColumn(Opt::Environment& cEnvironment_);		
				
	protected:
		virtual bool isDelete() {return true;}
		
	private:
	//////////////////////////
	// Interface::IRelation
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);
		
		Plan::Utility::FieldSet m_cFieldSet;
		Plan::Interface::IRelation* m_pRetrieve;
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
			   Plan::Interface::IRelation* pOperand_,
			   Plan::Relation::Table* pLocalInsert_,
			   bool bRelocateUpdate_)
			: Super(pSchemaTable_, pOperand_, pLocalInsert_),
			  m_bRelocateUpdate(bRelocateUpdate_)
		{}
		~Insert() {}

	/////////////////////////////
	// Relation::Table::

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Plan::Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 Plan::AccessPlan::Source& cDPlanSource_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

		virtual Plan::Relation::RowInfo* getGeneratedColumn(Opt::Environment& cEnvironment_);
		
	protected:
		virtual bool isInsert() {return true;}
	//	virtual bool isDelete();
		
	private:
		bool m_bRelocateUpdate;
		void createTargetField(Opt::Environment& cEnvironment_,
							   VECTOR<Plan::Scalar::Field*>& cTarget_);

	/////////////////////////////////
	// Relation::TableImpl::Base::

	};


		///////////////////////////////////////////////////////////////////////
	// CLASS
	//	Relation::TableImpl::Insert -- implementation class of Insert
	//
	// NOTES
	class Update
		: public Put
	{
	public:
		typedef Put Super;
		typedef Update This;

		Update(Plan::Relation::Table* pTable_,
			   Plan::Interface::IRelation* pOperand_)
			: Super(pTable_->getSchemaTable(),
					pOperand_,
					0),
			  m_pTable(pTable_),
			  m_pRetrieve(pOperand_)
		{}
		~Update() {}

	/////////////////////////////
	// Relation::Table::

	///////////////////////////////////////
	// Interface::IRelation::
		virtual Plan::Interface::ICandidate*
					createAccessPlan(Opt::Environment& cEnvironment_,
									 Plan::AccessPlan::Source& cDPlanSource_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

		virtual Plan::Relation::RowInfo* getGeneratedColumn(Opt::Environment& cEnvironment_);
		
	protected:
	private:
		void createTargetField(Opt::Environment& cEnvironment_,
							   VECTOR<Plan::Scalar::Field*>& cTarget_);

		Plan::Interface::IRelation* m_pRetrieve;
		Plan::Relation::Table* m_pTable;
	};

	
} // namespace Impl

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_RELATION_TABLEIMPL_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
