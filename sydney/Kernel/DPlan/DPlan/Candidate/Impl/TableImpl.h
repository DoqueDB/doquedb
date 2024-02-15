// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/TableImpl.h --
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_IMPL_TABLEIMPL_H
#define __SYDNEY_DPLAN_CANDIDATE_IMPL_TABLEIMPL_H

#include "DPlan/Candidate/Table.h"

#include "Plan/Declaration.h"
#include "Plan/Utility/ObjectSet.h"

#include "Plan/Relation/Table.h"
#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Cascade;
}

_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

namespace TableImpl
{
	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Candidate::TableImpl::Base -- base class
	//
	// NOTES
	class Base
		: public DPlan::Candidate::Table
	{
	public:
		typedef DPlan::Candidate::Table Super;
		typedef Base This;

		// destructor
		virtual ~Base() {}

	/////////////////////////////
	// DPlan::Candidate::Table::
		virtual Plan::Interface::ICandidate* createPlan(Opt::Environment& cEnvironment_,
														Plan::AccessPlan::Source& cPlanSource_,
														const Plan::Utility::FieldSet& cFieldSet_);


	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cDPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();
	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Plan::Scalar::Field* pField_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Plan::Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Plan::Scalar::Field* pField_,
						   Plan::Scalar::DelayArgument& cArgument_);
		virtual bool isReferingRelation(Plan::Interface::IRelation* pRelation_);
		virtual void createReferingRelation(Plan::Utility::RelationSet& cRelationSet_);
		virtual Execution::Interface::IIterator*
							adopt(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Plan::Candidate::AdoptArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);
		
		virtual Plan::Interface::ICandidate::InquiryResult inquiry(Opt::Environment& cEnvironment_,
																  const Plan::Candidate::InquiryArgument& cArgument_);

	protected:
		// constructor
		Base(Plan::Interface::IRelation* pRelation_)
			: Super(),
			  m_pRelation(pRelation_),
			  m_cFieldSet()
		{}

		// accessor
		Plan::Interface::IRelation* getRelation() {return m_pRelation;}
		Plan::Utility::FieldSet& getFieldSet() {return m_cFieldSet;}

		void addIterator(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Plan::Candidate::AdoptArgument& cArgument_,
						 int iQueueID_,
						 Schema::Cascade* pCascade_);
					
		void addOperation(Opt::Environment& cEnvironment_,
						  Execution::Interface::IProgram& cProgram_,
						  Execution::Interface::IIterator* pIterator_,
						  Plan::Candidate::AdoptArgument& cArgument_,
						  int iDataID_,
						  Schema::Cascade* pCascade_,
						  const STRING& cstrSQL_,
						  int iCheckID_);
		int createOutput(Opt::Environment& cEnvironment_,
						 Execution::Interface::IProgram& cProgram_,
						 Execution::Interface::IIterator* pIterator_,
						 Plan::Candidate::AdoptArgument& cArgument_);
		
		int createOutputFromType(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Plan::Candidate::AdoptArgument& cArgument_);
		
		Plan::Utility::FieldSet m_cFieldSet;
		
	private:
	/////////////////////////////
	// Interface::ICandidate::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const Plan::AccessPlan::Source& cDPlanSource_,
								Plan::AccessPlan::Cost& cCost_);
		virtual Plan::Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Plan::Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Plan::Interface::IRelation* m_pRelation;
		
	};

	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Candidate::TableImpl::RetrieveBase -- base class for retrieve
	//
	// NOTES
	class RetrieveBase
		: public Base
	{
	public:
		typedef Base Super;
		typedef RetrieveBase This;

		// destructor
		virtual ~RetrieveBase() {}
	protected:
		// constructor
		RetrieveBase(Plan::Interface::IRelation* pRelation_)
			: Super(pRelation_)
		{}


	/////////////////////////////
	// TableImpl::Base::

	/////////////////////////////
	// Interface::ICandidate
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);		
	private:

	};


	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Candidate::TableImpl::InsertBase -- base class for insert
	//
	// NOTES
	class InsertBase
		: public Base
	{
	public:
		typedef Base Super;
		typedef InsertBase This;

		// destructor
		virtual ~InsertBase() {}

		// constructor
		InsertBase(Plan::Relation::Table* pRelation_,
				   Plan::Interface::ICandidate* pOperand_,
				   bool bRelocateUpdate)
			: Super(pRelation_),
			  m_pOperand(pOperand_),
			  m_pTable(pRelation_),
			  m_bRelocateUpdate(bRelocateUpdate)
		{}

		// accessor
		Plan::Interface::ICandidate* getOperand() {return m_pOperand;}
		Plan::Relation::Table* getRelationTable() {return m_pTable;}


	/////////////////////////////
	// Interface::ICandidate		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
		
	protected:
		virtual Execution::Interface::IIterator* adoptOperand(Opt::Environment& cEnvironment_,
															  Execution::Interface::IProgram& cProgram_,
															  Plan::Candidate::AdoptArgument& cArgument_);
			
	private:
		Plan::Interface::ICandidate* m_pOperand;
		Plan::Relation::Table* m_pTable;
		bool m_bRelocateUpdate;
	};


	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Candidate::TableImpl::InsertBase -- base class for update
	//
	// NOTES
	//	分散Updateのプログラムを生成するクラス。
	//	生成されるプログラムはPreparedStatementを使用して、
	//	全子サーバーに対して更新クエリを投入する。
	//	現バージョンでは、分散キーの更新による子サーバへの配置変更をしないため、
	//	Distribution、Replicationともに同じProgramを生成する。
	//
	class UpdateBase
		: public Base
	{
	public:
		typedef Base Super;
		typedef UpdateBase This;

		// destructor
		virtual ~UpdateBase() {}

		// constructor
		UpdateBase(Plan::Relation::Table* pRelation_,
				   Plan::Interface::ICandidate* pOperand_)
			: Super(pRelation_),
			  m_pOperand(pOperand_),
			  m_pTable(pRelation_)
		{}

	protected:
		// accessor
		Plan::Interface::ICandidate* getOperand() {return m_pOperand;}
		Plan::Relation::Table* getRelationTable() {return m_pTable;}


	/////////////////////////////
	// Interface::ICandidate		
		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);				
	private:
		Plan::Interface::ICandidate* m_pOperand;
		Plan::Relation::Table* m_pTable;
	};
	


	/////////////////////////////////////////////////////////////////////
	// CLASS
	//	Candidate::TableImpl::Cascade -- base class for insertion
	//
	// NOTES
	class Delete
		: public Base
	{
	public:
		typedef Base Super;
		typedef Delete This;

		// destructor
		virtual ~Delete() {}

		// constructor
		Delete(Plan::Relation::Table* pRelation_)
			: Super(pRelation_),
			  m_pTable(pRelation_)
		{}

		// accessor
		Plan::Relation::Table* getRelationTable() {return m_pTable;}
		
		// Interface::ICandidate::

	/////////////////////////////
	// Interface::ICandidate		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);		

	protected:		

	private:
		Plan::Interface::ICandidate* m_pOperand;
		Plan::Relation::Table* m_pTable;
	};


	

	namespace Distribute
	{
		/////////////////////////////////////////////////
		// CLASS
		//	Candidate::TableImpl::Distribute::Retrieve --
		//
		// NOTES
		class Retrieve
			: public RetrieveBase
		{
		public:
			typedef RetrieveBase Super;
			typedef Retrieve This;

			// constructor
			Retrieve(Plan::Interface::IRelation* pRelation_)
				: Super(pRelation_)
			{}
			// destructor
			~Retrieve() {}

		/////////////////////////////
		// DPlan::Candidate::Table::
			virtual Plan::Interface::ICandidate* createPlan(Opt::Environment& cEnvironment_,
															Plan::AccessPlan::Source& cPlanSource_,
															const Plan::Utility::FieldSet& cFieldSet_);
		protected:
		private:			
		};
		
		/////////////////////////////////////////////////
		// CLASS
		//	Candidate::TableImpl::Distribute::Insert --
		//
		// NOTES
		class Insert
			: public InsertBase
		{
		public:
			typedef InsertBase Super;
			typedef Insert This;

			// constructor
			Insert(Plan::Relation::Table* pRelation_,
				   Plan::Interface::ICandidate* pOperand_,
				   Plan::Interface::IScalar* pCheck_,
				   bool bRelocateUpdate)
				: Super(pRelation_, pOperand_, bRelocateUpdate),
				  m_pCheck(pCheck_)
			{}
			// destructor
			~Insert() {}

		/////////////////////////////
		// DPlan::Candidate::Table::

		/////////////////////////////
		// Interface::ICandidate::
			virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Plan::Candidate::AdoptArgument& cArgument_);
						
		protected:
		private:
		/////////////////////////////
		// Interface::ICandidate::

			Plan::Interface::IScalar* m_pCheck;
		};



	} // namespace Distribute

	namespace Replicate
	{
		/////////////////////////////////////////////////
		// CLASS
		//	Candidate::TableImpl::Replicate::Retrieve --
		//
		// NOTES
		class Retrieve
			: public RetrieveBase
		{
		public:
			typedef RetrieveBase Super;
			typedef Retrieve This;

			// constructor
			Retrieve(Plan::Interface::IRelation* pRelation_)
				: Super(pRelation_)
			{}
			// destructor
			~Retrieve() {}

		/////////////////////////////
		// Interface::ICandidate::
			virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Plan::Candidate::AdoptArgument& cArgument_);			
		/////////////////////////////
		// DPlan::Candidate::Table::
		protected:
		private:
		};
		
		/////////////////////////////////////////////////
		// CLASS
		//	Candidate::TableImpl::Replicate::Insert --
		//
		// NOTES
		class Insert
			: public InsertBase
		{
		public:
			typedef InsertBase Super;
			typedef Insert This;

			// constructor
			Insert(Plan::Relation::Table* pRelation_,
				   Plan::Interface::ICandidate* pOperand_)
				: Super(pRelation_, pOperand_, false)
			{}
			// destructor
			~Insert() {}

		/////////////////////////////
		// DPlan::Candidate::Table::

		/////////////////////////////
		// Interface::ICandidate::
			virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Plan::Candidate::AdoptArgument& cArgument_);
		protected:
		private:
		/////////////////////////////
		// Interface::ICandidate::
		};

	} // namespace Replicate
} // namespace TableImpl

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_CANDIDATE_IMPL_TABLEIMPL_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
