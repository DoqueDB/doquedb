// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/FetchImpl.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_FETCHIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_FETCHIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Fetch.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////
	// CLASS
	//	Predicate::Impl::MonadicKey
	//
	// NOTES
	class MonadicKey
		: public Tree::Monadic<Tree::Node, Interface::IScalar>
	{
	public:
		typedef Tree::Monadic<Tree::Node, Interface::IScalar> Super;
		typedef MonadicKey This;

		MonadicKey(Interface::IScalar* pScalar_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pScalar_)
		{
			setType(Tree::Node::List);
		}
#else
			: Super(Tree::Node::List, pScalar_)
		{}
#endif
		~MonadicKey() {}
	};
	///////////////////////////////////
	// CLASS
	//	Predicate::Impl::NadicKey
	//
	// NOTES
	class NadicKey
		: public Tree::Nadic<Tree::Node, Interface::IScalar>
	{
	public:
		typedef Tree::Nadic<Tree::Node, Interface::IScalar> Super;
		typedef NadicKey This;

		NadicKey(const VECTOR<Interface::IScalar*> vecKey_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(vecKey_)
		{
			setType(Tree::Node::List);
		}
#else
			: Super(Tree::Node::List, vecKey_)
		{}
#endif
		~NadicKey() {}
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Predicate::Impl::SingleFetch -- fetch with single key
	//
	// NOTES
	class SingleFetch
		: public Impl::Base< Tree::Dyadic<Fetch, Interface::IScalar> >
	{
	public:
		typedef Impl::Base< Tree::Dyadic<Fetch, Interface::IScalar> > Super;
		typedef SingleFetch This;

		SingleFetch(Interface::IPredicate* pOriginal_,
					const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_)
			: Super(cOperand_),
			  m_pOriginal(pOriginal_),
			  m_cKey(cOperand_.first)
		{}
		~SingleFetch() {}

	///////////////////////////////
	// Predicate::Fetch::
		virtual bool getKeyAndValue(VECTOR<Interface::IScalar*>& vecKey_,
									VECTOR<Interface::IScalar*>& vecValue_);

	///////////////////////////////
	// Interface::IPredicate::
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						  Interface::ICandidate* pCandidate_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Interface::ICandidate* pCandidate_,
						   Scalar::DelayArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
		virtual void getUsedTable(Utility::RelationSet& cResult_);
		virtual void getUsedField(Utility::FieldSet& cResult_);
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	/////////////////////////////
	// Tree::Node::Interface::
		virtual ModSize getOptionSize() const
		{return getOperandSize();}
		virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const
		{return iPosition_ == 0 ? &m_cKey : getOperandAt(iPosition_);}

	protected:
	private:
		Interface::IPredicate* m_pOriginal;
		MonadicKey m_cKey;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Predicate::Impl::MultipleFetch -- fetch with multiple key
	//
	// NOTES
	class MultipleFetch
		: public Fetch
	{
	public:
		typedef Fetch Super;
		typedef MultipleFetch This;
		typedef Interface::IScalar Operand;

		MultipleFetch(Interface::IPredicate* pOriginal_,
					  const VECTOR<Interface::IScalar*>& vecKey_,
					  const VECTOR<Interface::IScalar*>& vecValue_)
			: Super(),
			  m_pOriginal(pOriginal_),
			  m_cKey(vecKey_),
			  m_vecValue(vecValue_)
		{}
		~MultipleFetch() {}

	///////////////////////////////
	// Predicate::Fetch::
		virtual bool getKeyAndValue(VECTOR<Interface::IScalar*>& vecKey_,
									VECTOR<Interface::IScalar*>& vecValue_);

	///////////////////////////////
	// Interface::IPredicate::
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
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);
		virtual int generateKey(Opt::Environment& cEnvironment_,
								Execution::Interface::IProgram& cProgram_,
								Execution::Interface::IIterator* pIterator_,
								Candidate::AdoptArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
		virtual void getUsedTable(Utility::RelationSet& cResult_);
		virtual void getUsedField(Utility::FieldSet& cResult_);
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;
			
		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);
		
	/////////////////////////////
	// Tree::Node::Interface::
		virtual ModSize getOperandSize() const
		{return 2;}
		virtual const LogicalFile::TreeNodeInterface* getOperandAt(ModInt32 iPosition_) const
		{return &m_cKey;}
		virtual ModSize getOptionSize() const
		{return getOperandSize();}
		virtual const LogicalFile::TreeNodeInterface* getOptionAt(ModInt32 iPosition_) const
		{return getOperandAt(iPosition_);}

		template <class Function_>
		Function_
		foreachOperand(Function_ function_)
		{
			m_cKey.foreachOperand(function_);
			FOREACH(m_vecValue, function_);
			return function_;
		}

	protected:
	private:
		Interface::IPredicate* m_pOriginal;
		NadicKey m_cKey;
		VECTOR<Interface::IScalar*> m_vecValue;
	};
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_FETCHIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
