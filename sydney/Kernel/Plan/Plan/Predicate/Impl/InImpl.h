// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/InImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_INIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_INIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/In.h"

#include "Exception/NotSupported.h"

#include "Plan/Tree/Dyadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace InImpl
{
	namespace SubQuery
	{
		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::SubQuery::Base -- in predicate
		//
		// NOTES
		class Base
			: public Predicate::In
		{
		public:
			typedef Predicate::In Super;
			typedef Base This;

			Base(Interface::IRelation* pOperand_,
				 const Utility::RelationSet& cOuterRelation_,
				 bool bIsNot_)
				: Super(),
				  m_pOperand(pOperand_),
				  m_cOuterRelation(cOuterRelation_),
				  m_bIsNot(bIsNot_)
			{}
			virtual ~Base() {}

		////////////////////////////
		// Predicate::In::

		///////////////////////////////
		// Interface::IPredicate::
			virtual int estimateRewrite(Opt::Environment& cEnvironment_);
			virtual Interface::IPredicate::RewriteResult
							rewrite(Opt::Environment& cEnvironment_,
									Interface::IRelation* pRelation_,
									Predicate::RewriteArgument& cArgument_);
			virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
												 const CheckArgument& cArgument_);
			virtual bool hasSubquery();

		///////////////////////////
		// Interface::IScalar::
			virtual void explain(Opt::Environment* pEnvironment_,
								 Opt::Explain& cExplain_);
			virtual int generate(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);

		///////////////////////////
		// Interface::ISqlNode::			
			virtual STRING toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const
			{_SYDNEY_THROW0(Exception::NotSupported);};


		//////////////////////////////////////////
		// Tree::Node::Super::

		protected:
			Interface::IRelation* getOperand() {return m_pOperand;}
			const Utility::RelationSet& getOuterRelation() {return m_cOuterRelation;}
			bool isNot() {return m_bIsNot;}

		private:
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_) = 0;
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_) = 0;

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_) = 0;

			void checkOuterRelation(Interface::IRelation* pRelation_,
									Opt::Environment& cEnvironment_,
									const CheckArgument& cArgument_,
									Check::Value* pValue_);

			Interface::IRelation* m_pOperand;
			Utility::RelationSet m_cOuterRelation;
			bool m_bIsNot;
		};

		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::SubQuery::SingleKey -- in predicate
		//
		// NOTES
		class SingleKey
			: public Base
		{
		public:
			typedef Base Super;
			typedef SingleKey This;

			SingleKey(Interface::IScalar* pScalar_,
					  Interface::IRelation* pOperand_,
					  const Utility::RelationSet& cOuterRelation_,
					  bool bIsNot_)
				: Super(pOperand_, cOuterRelation_, bIsNot_),
				  m_pScalar(pScalar_)
			{}
			~SingleKey() {}

		////////////////////////////
		// Interface::IScalar::
			virtual void getUsedTable(Utility::RelationSet& cResult_);
			virtual void getUsedField(Utility::FieldSet& cResult_);
			virtual void getUnknownKey(Opt::Environment& cEnvironment_,
									   Predicate::CheckUnknownArgument& cResult_);
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

			virtual void setParameter(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  DExecution::Action::StatementConstruction& cExec,
									  const Plan::Sql::QueryArgument& cArgument_);
		protected:
		private:
		////////////////////////////
		// InImpl::SubQuery::Base::
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_);
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_);

			Interface::IScalar* m_pScalar;
		};

		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::SubQuery::MultiKey -- in predicate
		//
		// NOTES
		class MultiKey
			: public Base
		{
		public:
			typedef Base Super;
			typedef MultiKey This;

			MultiKey(const VECTOR<Interface::IScalar*>& vecScalar_,
					 Interface::IRelation* pOperand_,
					 const Utility::RelationSet& cOuterRelation_,
					 bool bIsNot_)
				: Super(pOperand_, cOuterRelation_, bIsNot_),
				  m_vecScalar(vecScalar_)
			{}
			~MultiKey() {}

		////////////////////////////
		// Interface::IScalar::
			virtual void getUsedTable(Utility::RelationSet& cResult_);
			virtual void getUsedField(Utility::FieldSet& cResult_);
			virtual void getUnknownKey(Opt::Environment& cEnvironment_,
									   Predicate::CheckUnknownArgument& cResult_);
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

			virtual void setParameter(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  DExecution::Action::StatementConstruction& cExec,
									  const Plan::Sql::QueryArgument& cArgument_);
		protected:
		private:
		////////////////////////////
		// InImpl::SubQuery::Base::
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_);
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_);

			VECTOR<Interface::IScalar*> m_vecScalar;
		};
	} // namespace SubQuery

	namespace ValueList
	{
		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::ValueList::Base -- in predicate
		//
		// NOTES
		class Base
			: public Predicate::In
		{
		public:
			typedef Predicate::In Super;
			typedef Base This;

			Base(Interface::IRelation* pOperand_,
				 const Utility::RelationSet& cOuterRelation_,
				 bool bIsNot_)
				: Super(),
				  m_pOperand(pOperand_),
				  m_cOuterRelation(cOuterRelation_),
				  m_bIsNot(bIsNot_)
			{}
			virtual ~Base() {}

		////////////////////////////
		// Predicate::In::

		///////////////////////////////
		// Interface::IPredicate::
			virtual Interface::IPredicate::RewriteResult
							rewrite(Opt::Environment& cEnvironment_,
									Interface::IRelation* pRelation_,
									Predicate::RewriteArgument& cArgument_);
			virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
												 const CheckArgument& cArgument_);

		///////////////////////////
		// Interface::IScalar::
			virtual void explain(Opt::Environment* pEnvironment_,
								 Opt::Explain& cExplain_);
			virtual int generate(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);

		///////////////////////////
		// Interface::ISqlNode::				
			virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
										  const Plan::Sql::QueryArgument& cArgument_) const
			{_SYDNEY_THROW0(Exception::NotSupported);}

		//////////////////////////////////////////
		// Tree::Node::Super::

		protected:
			Interface::IRelation* getOperand() {return m_pOperand;}
			const Utility::RelationSet& getOuterRelation() {return m_cOuterRelation;}
			bool isNot() {return m_bIsNot;}

		private:
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_,
										   int iPosition_) = 0;
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_) = 0;

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_) = 0;

			virtual int generateKey(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_) = 0;

			virtual int generateOperand(Opt::Environment& cEnvironment_,
										Execution::Interface::IProgram& cProgram_,
										Execution::Interface::IIterator* pIterator_,
										Candidate::AdoptArgument& cArgument_) = 0;

			void checkOuterRelation(Interface::IRelation* pRelation_,
									Opt::Environment& cEnvironment_,
									const CheckArgument& cArgument_,
									Check::Value* pValue_);

			Interface::IRelation* m_pOperand;
			Utility::RelationSet m_cOuterRelation;
			bool m_bIsNot;
		};

		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::ValueList::SingleKey -- in predicate
		//
		// NOTES
		class SingleKey
			: public Base
		{
		public:
			typedef Base Super;
			typedef SingleKey This;

			SingleKey(Interface::IScalar* pScalar_,
					  Interface::IRelation* pOperand_,
					  const Utility::RelationSet& cOuterRelation_,
					  bool bIsNot_)
				: Super(pOperand_, cOuterRelation_, bIsNot_),
				  m_pScalar(pScalar_)
			{}
			~SingleKey() {}

		////////////////////////////
		// Interface::IScalar::
		//	virtual bool hasParameter();
			virtual bool isArbitraryElement();
			virtual void getUsedTable(Utility::RelationSet& cResult_);
			virtual void getUsedField(Utility::FieldSet& cResult_);
			virtual void getUnknownKey(Opt::Environment& cEnvironment_,
									   Predicate::CheckUnknownArgument& cResult_);
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

			virtual void setParameter(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  DExecution::Action::StatementConstruction& cExec,
									  const Plan::Sql::QueryArgument& cArgument_);
		protected:
		private:
		//////////////////////////////////////////
		// InImpl::ValueList::Base::
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_,
										   int iPosition_);
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_);

			virtual int generateKey(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_);

			virtual int generateOperand(Opt::Environment& cEnvironment_,
										Execution::Interface::IProgram& cProgram_,
										Execution::Interface::IIterator* pIterator_,
										Candidate::AdoptArgument& cArgument_);

			Interface::IScalar* m_pScalar;
		};

		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::ValueList::MultiKey -- in predicate
		//
		// NOTES
		class MultiKey
			: public Base
		{
		public:
			typedef Base Super;
			typedef MultiKey This;

			MultiKey(const VECTOR<Interface::IScalar*>& vecScalar_,
					 Interface::IRelation* pOperand_,
					 const Utility::RelationSet& cOuterRelation_,
					 bool bIsNot_)
				: Super(pOperand_, cOuterRelation_, bIsNot_),
				  m_vecScalar(vecScalar_)
			{}
			~MultiKey() {}

		////////////////////////////
		// Interface::IScalar::
		//	virtual bool hasParameter();
		//	virtual bool isArbitraryElement();
			virtual void getUsedTable(Utility::RelationSet& cResult_);
			virtual void getUsedField(Utility::FieldSet& cResult_);
			virtual void getUnknownKey(Opt::Environment& cEnvironment_,
									   Predicate::CheckUnknownArgument& cResult_);
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

			virtual void setParameter(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  DExecution::Action::StatementConstruction& cExec,
									  const Plan::Sql::QueryArgument& cArgument_);
		protected:
		private:
		//////////////////////////////////////////
		// InImpl::ValueList::Base::
			virtual Interface::IPredicate*
					createRewritePredicate(Opt::Environment& cEnvironment_,
										   int iPosition_);
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);

			virtual Check::Value checkKey(Opt::Environment& cEnvironment_,
										  const CheckArgument& cArgument_);

			virtual int generateKey(Opt::Environment& cEnvironment_,
									Execution::Interface::IProgram& cProgram_,
									Execution::Interface::IIterator* pIterator_,
									Candidate::AdoptArgument& cArgument_);

			virtual int generateOperand(Opt::Environment& cEnvironment_,
										Execution::Interface::IProgram& cProgram_,
										Execution::Interface::IIterator* pIterator_,
										Candidate::AdoptArgument& cArgument_);

			VECTOR<Interface::IScalar*> m_vecScalar;
		};
	} // namespace ValueList

	namespace Variable
	{
		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::Variable::Base -- in predicate
		//
		// NOTES
		class Base
			: public Predicate::In
		{
		public:
			typedef Predicate::In Super;
			typedef Base This;

			Base()
				: Super()
			{}
			virtual ~Base() {}

		////////////////////////////
		// Predicate::In::

		///////////////////////////////
		// Interface::IPredicate::
		//	virtual Interface::IPredicate::RewriteResult
		//					rewrite(Opt::Environment& cEnvironment_,
		//							Interface::IRelation* pRelation_,
		//							Predicate::RewriteArgument& cArgument_);
		//	virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
		//										 const CheckArgument& cArgument_);

		///////////////////////////
		// Interface::IScalar::
			virtual void explain(Opt::Environment* pEnvironment_,
								 Opt::Explain& cExplain_);
			virtual int generate(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_,
								 Candidate::AdoptArgument& cArgument_);

		///////////////////////////
		// Interface::ISqlNode::				
			virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
										  const Plan::Sql::QueryArgument& cArgument_) const
			{_SYDNEY_THROW0(Exception::NotSupported);}

			
			
		//////////////////////////////////////////
		// Tree::Node::Super::

		protected:
			Interface::IRelation* getOperand() {return 0;}
			const Utility::RelationSet& getOuterRelation() {return m_cOuterRelation;}

		private:
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_) = 0;
			virtual void explainOperand(Opt::Environment* pEnvironment_,
										Opt::Explain& cExplain_) = 0;

			Utility::RelationSet m_cOuterRelation; // always empty
		};

		//////////////////////////////////////////////////////////////
		// CLASS
		//	Plan::Predicate::InImpl::Variable::SingleKey -- in predicate
		//
		// NOTES
		class SingleKey
			: public Tree::Dyadic<Base, Interface::IScalar>
		{
		public:
			typedef Tree::Dyadic<Base, Interface::IScalar> Super;
			typedef SingleKey This;

			SingleKey(Interface::IScalar* pScalar_,
					  Interface::IScalar* pOperand_)
				: Super(pScalar_, pOperand_)
			{}
			~SingleKey() {}

		////////////////////////////
		// Interface::IPredicate::
			virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
												 const CheckArgument& cArgument_);
			virtual bool isNeedIndex();

		////////////////////////////
		// Interface::IScalar::
		//	virtual bool hasParameter();
			virtual bool isArbitraryElement();
			virtual void getUsedTable(Utility::RelationSet& cResult_);
			virtual void getUsedField(Utility::FieldSet& cResult_);
			virtual void getUnknownKey(Opt::Environment& cEnvironment_,
									   Predicate::CheckUnknownArgument& cResult_);
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
		/////////////////////////////////////
		// Interface::ISqlNode
			virtual void setParameter(Opt::Environment& cEnvironment_,
									  Execution::Interface::IProgram& cProgram_,
									  Execution::Interface::IIterator* pIterator_,
									  DExecution::Action::StatementConstruction& cExec,
									  const Plan::Sql::QueryArgument& cArgument_);			

		protected:
		private:
		//////////////////////////////////////////
		// InImpl::Variable::Base::
			virtual void explainKey(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
			virtual void explainOperand(Opt::Environment* pEnvironment_,
										Opt::Explain& cExplain_);
		};
	} // namespace Variable
} // namespace InImpl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_INIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
