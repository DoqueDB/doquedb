// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CombinatorImpl.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_COMBINATORIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_COMBINATORIMPL_H

#include "boost/bind.hpp"

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Combinator.h"

#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Relation/Union.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/Combinator.h"

#include "Opt/Algorithm.h"

#include "Opt/Declaration.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	// explain name
	// defined here because AND/OR/CHOICE is implemented as template
	struct _Explain
	{
		enum Value {
			And = 0,
			Or,
			Not,
			Choice,
		};
	};
	const char* const _pszOperatorName[] = {
		"and ",
		"or ",
		"not ",
		"choice ",
	};
} // namespace

namespace Impl
{
	////////////////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::CombinatorBase -- Base class for combined predicate
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class CombinatorBase
		: public Base<Handle_>
	{
	public:
		typedef Base<Handle_> Super;
		typedef CombinatorBase<Handle_> This;
		typedef typename Handle_::Operand Operand;

		// destructor
		virtual ~CombinatorBase() {}

	//////////////////////////
	// Interface::IPredicate
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_)
		{
			VECTOR<Interface::IPredicate*> vecResult;
			if (isAll(boost::bind(&This::convertNotOperand,
								  boost::ref(cEnvironment_),
								  _1,
								  &vecResult))) {
				return Combinator::create(cEnvironment_,
										  This::getNotType(getType()),
										  vecResult);
			}
			return 0;
		}
		virtual bool isNeedIndex()
		{
			return isAny(boost::bind(&Operand::isNeedIndex,
									 _1));
		}

		Plan::Interface::ICandidate* createDistributePlan(Opt::Environment& cEnvironment_,
														  Interface::ICandidate* pOperand_,
														  Utility::FieldSet& cFieldSet_)
		{
			Interface::ICandidate* pCandidate = pOperand_;
			for (int i = 0; i < getSize(); i++ ) {
				pCandidate = getOperandi(i)->createDistributePlan(cEnvironment_,
																 pCandidate,
																 cFieldSet_);
				
			}
			return pCandidate;
		}
		

	//////////////////////////
	// Interface::IScalar
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_)
		{
			cExplain_.pushIndent();
			cExplain_.put("(");
			joinOperand(boost::bind(&Interface::IPredicate::explain,
									_1,
									pEnvironment_,
									boost::ref(cExplain_)),
						boost::bind(&This::putOperator,
									this,
									boost::ref(cExplain_)));
			cExplain_.put(")");
			cExplain_.popIndent(true /* force new line */);
		}
		virtual bool hasSubquery()
		{
			return isAny(boost::bind(&Operand::hasSubquery,
									 _1));
		}

	//////////////////////////
	// Interface::ISqlNode
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{
			OSTRSTREAM cStream;
			STRING cOperator = " ";
			for (int i = 0; i < getSize(); ++i) {
				cStream << cOperator;
				cStream << '(';
				cStream << getOperandi(i)->toSQLStatement(cEnvironment_, cArgument_);
				cStream << ')';
				cOperator = explainOperator();
			}
			return cStream.getString();
		}

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_)
		{
			STRING cOperator = " ";
			for (int i = 0; i < getSize(); ++i) {
				cExec.append(cOperator).append('(');
				getOperandi(i)->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
				cExec.append(')');
				cOperator = explainOperator();
			}
		}

		using Super::getSize;
		using Super::isAll;
		using Super::getType;
		using Super::getOperandi;
		using Super::joinOperand;
		using Super::isAny;

	protected:
		// constructor
		CombinatorBase(Tree::Node::Type eOperator_,
						   typename Handle_::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cArgument_)
		{
			setArgument(eOperator_);
		}
#else
			: Super(eOperator_, cArgument_)
		{}
#endif

	private:
		// get operator name for explain
		virtual const char* explainOperator() const = 0;

		// convert not
		static bool convertNotOperand(Opt::Environment& cEnvironment_,
									  Interface::IPredicate* p_,
									  VECTOR<Interface::IPredicate*>* pvecResult_)
		{
			if (Interface::IPredicate* pResult = p_->convertNot(cEnvironment_)) {
				pvecResult_->PUSHBACK(pResult);
				return true;
			}
			return false;
		}
		// not type
		static Tree::Node::Type getNotType(Tree::Node::Type eType_)
		{
			switch (eType_) {
			case Tree::Node::And:	return Tree::Node::Or;
			case Tree::Node::Or:	return Tree::Node::And;
			default:				break;
			}
			return eType_;
		}
		// explain
		void putOperator(Opt::Explain& cExplain_)
		{
			cExplain_.newLine().put(explainOperator());
		}
			
	};

	////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::AndImpl -- and predicate
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class AndImpl
		: public CombinatorBase<Handle_>
	{
	public:
		typedef CombinatorBase<Handle_> Super;
		typedef AndImpl<Handle_> This;

		// constructor
		AndImpl(typename Handle_::Argument cArgument_)
			: Super(Tree::Node::And, cArgument_)
		{}
		// destructor
		~AndImpl() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual int estimateRewrite(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate::RewriteResult
						rewrite(Opt::Environment& cEnvironment_,
								Interface::IRelation* pRelation_,
								Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getCheckUnknown(Opt::Environment& cEnvironment_,
									 Predicate::CheckUnknownArgument& cArgument_);
		virtual bool isFetch();
		virtual bool getFetchKey(Opt::Environment& cEnvironment_,
								 Utility::ScalarSet& cFetchKey_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);
		virtual AccessPlan::Cost::Value
						checkRate(Opt::Environment& cEnvironment_,
								  const Utility::RelationSet& cTable_);

	///////////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		using Super::foreachOperand;
		using Super::getType;
		using Super::isAll;
		using Super::getMinOperand;
		using Super::getNodeVariable;
		using Super::setNodeVariable;
		using Super::mapOperand;
		using Super::isAny;

	protected:
	private:
	///////////////////////////////
	// CombinatorBase::
		virtual const char* explainOperator() const;
	};

	/////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::OrImpl -- or predicate
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class OrImpl
		: public CombinatorBase<Handle_>
	{
	public:
		typedef CombinatorBase<Handle_> Super;
		typedef OrImpl<Handle_> This;

		// constructor
		OrImpl(typename Handle_::Argument cArgument_)
			: Super(Tree::Node::Or, cArgument_)
		{}
		// destructor
		~OrImpl() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual int estimateRewrite(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate::RewriteResult
						rewrite(Opt::Environment& cEnvironment_,
								Interface::IRelation* pRelation_,
								Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getCheckUnknown(Opt::Environment& cEnvironment_,
									 Predicate::CheckUnknownArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);
		virtual AccessPlan::Cost::Value
						checkRate(Opt::Environment& cEnvironment_,
								  const Utility::RelationSet& cTable_);

	///////////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		using Super::foreachOperand;
		using Super::getType;
		using Super::getOperandSize;
		using Super::isAll;
		using Super::getMaxOperand;
		using Super::getNodeVariable;
		using Super::setNodeVariable;
		using Super::mapOperand;

	protected:
	private:
	///////////////////////////////
	// CombinatorBase::
		virtual const char* explainOperator() const;
	};

	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::NotImpl -- not predicate
	//
	// NOTES
	class NotImpl
		: public CombinatorBase< Tree::Monadic<Predicate::Combinator,
											   Interface::IPredicate> >
	{
	public:
		typedef CombinatorBase< Tree::Monadic<Predicate::Combinator,
											  Interface::IPredicate> > Super;
		typedef NotImpl This;

		// constructor
		NotImpl(Interface::IPredicate* pOperand_)
			: Super(Tree::Node::Not, pOperand_)
		{}
		// destructor
		~NotImpl() {}

	////////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_)
		{
			// NOT(NOT(P)) = P
			return getOperand();
		}
		virtual Interface::IPredicate::RewriteResult
						rewrite(Opt::Environment& cEnvironment_,
								Interface::IRelation* pRelation_,
								Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getCheckUnknown(Opt::Environment& cEnvironment_,
									 Predicate::CheckUnknownArgument& cArgument_)
		{return getOperand()->getCheckUnknown(cEnvironment_, cArgument_);}
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);
		virtual AccessPlan::Cost::Value
						checkRate(Opt::Environment& cEnvironment_,
								  const Utility::RelationSet& cTable_);

	///////////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_)
		{
			cExplain_.pushIndent();
			cExplain_.put(explainOperator());
			getOperand()->explain(pEnvironment_, cExplain_);
			cExplain_.popIndent(true /* force new line */);
		}
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		
	///////////////////////////////
	// Interface::ISqlNode::
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	private:
	///////////////////////////////
	// CombinatorBase::
		virtual const char* explainOperator() const;
	};

	//////////////////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Plan::Predicate::Impl::ChoiceImpl -- choice of predicates
	//
	// TEMPLATE ARGUMENTS
	//	class Handle_
	//
	// NOTES
	//	This class is used for enumerating choices of predicate patterns
	template <class Handle_>
	class ChoiceImpl
		: public CombinatorBase<Handle_>
	{
	public:
		typedef CombinatorBase<Handle_> Super;
		typedef ChoiceImpl<Handle_> This;

		// constructor
		ChoiceImpl(typename Handle_::Argument cArgument_)
			: Super(Tree::Node::Choice, cArgument_)
		{}
		// destructor
		~ChoiceImpl() {}

	////////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool getCheckUnknown(Opt::Environment& cEnvironment_,
									 Predicate::CheckUnknownArgument& cArgument_);

	///////////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		using Super::foreachOperand;
		using Super::getType;
		using Super::getOperandi;

	protected:
	private:
	///////////////////////////////
	// CombinatorBase::
		virtual const char* explainOperator() const;
	};

////////////////////////////////////////
////////////////////////////////////////

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::AndRewriter --
	//
	// NOTES
	class AndRewriter
	{
	public:
		AndRewriter(Opt::Environment& cEnvironment_,
					Interface::IRelation** ppRelation_,
					VECTOR<Interface::IPredicate*>* pvecPredicate_,
					Predicate::RewriteArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_ppRelation(ppRelation_),
			  m_pvecPredicate(pvecPredicate_),
			  m_cArgument(cArgument_)
		{}

		void operator()(Interface::IPredicate* p_);
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		Interface::IRelation** m_ppRelation;
		VECTOR<Interface::IPredicate*>* m_pvecPredicate;
		Predicate::RewriteArgument& m_cArgument;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::AndRewriteEstimator --
	//
	// NOTES
	class AndRewriteEstimator
	{
	public:
		AndRewriteEstimator(Opt::Environment& cEnvironment_)
			: m_cEnvironment(cEnvironment_),
			  m_iVal(1)
		{}

		void operator()(Interface::IPredicate* p_);
		int getVal() {return m_iVal;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		int m_iVal;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::OrRewriter --
	//
	// NOTES
	class OrRewriter
	{
	public:
		typedef MAP<Utility::FieldSet,
					VECTOR<Interface::IPredicate*>,
					LESS<Utility::FieldSet> > PredicateMap;
		typedef VECTOR<Utility::FieldSet> FieldSetVector;

		typedef	PAIR< Interface::IRelation*,
					  PAIR<PredicateMap, FieldSetVector> > Element;
		typedef MAP<Interface::IRelation*,
					SIZE,
					Utility::RelationSet::Comparator> Map;
		typedef MAP<Utility::RelationSet,
					SIZE,
					LESS<Utility::RelationSet> > UseTableMap;

		OrRewriter(Opt::Environment& cEnvironment_,
				   Interface::IRelation* pRelation_,
				   VECTOR<Element>* pvecResult_,
				   Predicate::RewriteArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_pRelation(pRelation_),
			  m_pvecResult(pvecResult_),
			  m_cArgument(cArgument_),
			  m_mapRewrite(),
			  m_mapUseTable()
		{}

		void operator()(Interface::IPredicate* p_);
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		Interface::IRelation* m_pRelation;
		VECTOR<Element>* m_pvecResult;
		Predicate::RewriteArgument& m_cArgument;
		Map m_mapRewrite;
		UseTableMap m_mapUseTable;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::OrRewriteEstimator --
	//
	// NOTES
	class OrRewriteEstimator
	{
	public:
		OrRewriteEstimator(Opt::Environment& cEnvironment_)
			: m_cEnvironment(cEnvironment_),
			  m_cSingle(),
			  m_iVal(1)
		{}

		void operator()(Interface::IPredicate* p_);
		int getVal() {calculate(); return m_iVal;}

	protected:
	private:
		void calculate();

		Opt::Environment& m_cEnvironment;
		Utility::RelationSet m_cSingle;
		int m_iVal;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::OrMerger --
	//
	// NOTES
	class OrMerger
	{
	public:
		OrMerger(Opt::Environment& cEnvironment_,
				 VECTOR<Interface::IRelation*>* pvecResult_)
			: m_cEnvironment(cEnvironment_),
			  m_pvecResult(pvecResult_)
		{}

		void operator()(OrRewriter::Element& cElement_);
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		VECTOR<Interface::IRelation*>* m_pvecResult;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::PredicateMerger --
	//
	// NOTES
	class PredicateMerger
	{
	public:
		PredicateMerger(Opt::Environment& cEnvironment_,
						const OrRewriter::PredicateMap& mapPredicate_,
						VECTOR<Interface::IPredicate*>* pvecResult_)
			: m_cEnvironment(cEnvironment_),
			  m_mapPredicate(mapPredicate_),
			  m_pvecResult(pvecResult_)
		{}

		void operator()(const Utility::FieldSet& cFieldSet_);
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const OrRewriter::PredicateMap& m_mapPredicate;
		VECTOR<Interface::IPredicate*>* m_pvecResult;
	};

	/////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::IndexMap --
	//
	// NOTES
	class IndexMap
		: public MAP<Interface::IFile*,
					 VECTOR<Interface::IPredicate*>,
					 Utility::ReferingLess<Interface::IFile> >
	{
	public:
		typedef MAP<Interface::IFile*,
					VECTOR<Interface::IPredicate*>,
					Utility::ReferingLess<Interface::IFile> > Super;
		typedef IndexMap This;
		IndexMap() : Super() {}

		// add file->predicate relationship
		void add(Interface::IFile* pFile_,
				 Interface::IPredicate* pPredicate_);
	};

	///////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::IndexChecker --
	//
	// NOTES
	//	Function class for dividing Interface::IPredicate into checked and non-checked

	class IndexChecker
	{
	public:
		IndexChecker(Opt::Environment& cEnvironment_,
					 const CheckArgument& cArgument_,
					 VECTOR<Interface::IPredicate*>* pvecChecked_,
					 VECTOR<Interface::IPredicate*>* pvecNotChecked_,
					 IndexMap* pIndexMap_,
					 Candidate::Table** ppCandidate_,
					 Utility::FileSet* pFileSet_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_pvecChecked(pvecChecked_),
			  m_pvecNotChecked(pvecNotChecked_),
			  m_pIndexMap(pIndexMap_),
			  m_ppCandidate(ppCandidate_),
			  m_pFileSet(pFileSet_)
		{}

		void operator()(Interface::IPredicate* p_);

	private:
		Opt::Environment& m_cEnvironment;
		const CheckArgument& m_cArgument;
		VECTOR<Interface::IPredicate*>* m_pvecChecked;
		VECTOR<Interface::IPredicate*>* m_pvecNotChecked;
		IndexMap* m_pIndexMap;
		Candidate::Table** m_ppCandidate;
		Utility::FileSet* m_pFileSet;
	};

	//////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::IndexRecomposer --
	//
	// NOTES
	//	Function class used to re-compose operands

	class IndexRecomposer
	{
	public:
		IndexRecomposer(Opt::Environment& cEnvironment_,
						Tree::Node::Type eType_,
						const IndexMap& cIndexMap_,
						VECTOR<Interface::IPredicate*>* pvecChecked_,
						Utility::FileSet* pFileSet_,
						bool bNoTop_)
			: m_cEnvironment(cEnvironment_),
			  m_eType(eType_),
			  m_cIndexMap(cIndexMap_),
			  m_pvecChecked(pvecChecked_),
			  m_pFileSet(pFileSet_),
			  m_bNoTop(bNoTop_),
			  m_cDetermined()
		{}

		void operator()(Interface::IPredicate* p_);

	private:
		CheckedInterface* createChecked(const VECTOR<Interface::IPredicate*>& vecChecked_,
										Candidate::Table* pTable_,
										const Utility::FileSet& cFileSet_,
										bool bMergeFetch_ = false);

		Opt::Environment& m_cEnvironment;
		Tree::Node::Type m_eType;
		const IndexMap& m_cIndexMap;
		VECTOR<Interface::IPredicate*>* m_pvecChecked;
		Utility::FileSet* m_pFileSet;
		bool m_bNoTop;
		Utility::PredicateSet m_cDetermined;
	};

	//////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::AndUnknownChecker --
	//
	// NOTES
	//	Function class used to check unknown case

	class AndUnknownChecker
	{
	public:
		AndUnknownChecker(Opt::Environment& cEnvironment_,
						  Utility::ScalarSet& cKey_)
			: m_cEnvironment(cEnvironment_),
			  m_cKey(cKey_),
			  m_bFirst(true)
		{}

		bool operator()(Interface::IPredicate* p_);

	private:
		Opt::Environment& m_cEnvironment;
		Utility::ScalarSet& m_cKey;
		bool m_bFirst;
	};

	//////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::OrUnknownChecker --
	//
	// NOTES
	//	Function class used to check unknown case

	class OrUnknownChecker
	{
	public:
		OrUnknownChecker(Opt::Environment& cEnvironment_,
						 Utility::ScalarSet& cKey_)
			: m_cEnvironment(cEnvironment_),
			  m_cKey(cKey_)
		{}

		bool operator()(Interface::IPredicate* p_);

	private:
		Opt::Environment& m_cEnvironment;
		Utility::ScalarSet& m_cKey;
	};

	class AndCostEstimator
	{
	public:
		AndCostEstimator(Opt::Environment& cEnvironment_,
						 AccessPlan::Cost& cResult_)
			: m_cEnvironment(cEnvironment_),
			  m_cResult(cResult_),
			  m_cTemp(),
			  m_bResult(false)
		{}

		void operator()(Interface::IPredicate* p_);
		bool getResult();

	private:
		Opt::Environment& m_cEnvironment;
		AccessPlan::Cost& m_cResult;
		AccessPlan::Cost m_cTemp;
		bool m_bResult;
	};
	class OrCostEstimator
	{
	public:
		OrCostEstimator(Opt::Environment& cEnvironment_,
						AccessPlan::Cost& cResult_)
			: m_cEnvironment(cEnvironment_),
			  m_cResult(cResult_),
			  m_cTemp(),
			  m_bResult(false)
		{}

		void operator()(Interface::IPredicate* p_);
		bool getResult();

	private:
		Opt::Environment& m_cEnvironment;
		AccessPlan::Cost& m_cResult;
		AccessPlan::Cost m_cTemp;
		bool m_bResult;
	};

} // namespace Impl

////////////////////////////////////
//	Plan::Predicate::Impl::AndImpl

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::estimateRewrite -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
template <class Handle_>
int
Impl::AndImpl<Handle_>::
estimateRewrite(Opt::Environment& cEnvironment_)
{
	return foreachOperand(AndRewriteEstimator(cEnvironment_)).getVal();
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::rewrite -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
template <class Handle_>
Interface::IPredicate::RewriteResult
Impl::AndImpl<Handle_>::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	VECTOR<Interface::IPredicate*> vecPredicate;
	Interface::IRelation* pResult = pRelation_;
	foreachOperand(AndRewriter(cEnvironment_,
							   &pResult,
							   &vecPredicate,
							   cArgument_));
	Interface::IPredicate* pPredicate =
		Combinator::create(cEnvironment_,
						   getType(),
						   vecPredicate) ;
	if (pPredicate
		&& pResult != pRelation_) {
		// rewrite again
		PAIR<Interface::IRelation*, Interface::IPredicate*> cResult =
			pResult->rewrite(cEnvironment_,
							 pPredicate,
							 cArgument_);
		pResult = cResult.first;
		pPredicate = cResult.second;
	}
	return Interface::IPredicate::RewriteResult(
							 pResult,
							 pPredicate);
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::check -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
template <class Handle_>
Interface::IPredicate*
Impl::AndImpl<Handle_>::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	VECTOR<Interface::IPredicate*> vecChecked;
	VECTOR<Interface::IPredicate*> vecNotChecked;
	Utility::FileSet cFileSet;
	IndexMap cIndexMap;
	Candidate::Table* pCandidate = 0;

	// divide operands according to refering tables
	foreachOperand(IndexChecker(cEnvironment_,
								cArgument_,
								&vecChecked,
								&vecNotChecked,
								&cIndexMap,
								&pCandidate,
								&cFileSet));

	if (vecChecked.ISEMPTY()) {
		// no relationships with pRelation_
		; _SYDNEY_ASSERT(vecNotChecked.ISEMPTY() == false);
		return Combinator::create(cEnvironment_,
								  getType(),
								  vecNotChecked);
	}

	if (cArgument_.m_bNoTop
		&& !vecNotChecked.ISEMPTY()) {
		// if partly not checked under non-top predicate,
		// whole operands should be treated as non-checked
		VECTOR<Interface::IPredicate*> vecResult(vecChecked);
		vecResult.insert(vecResult.end(),
						 vecNotChecked.begin(),
						 vecNotChecked.end());
		return Combinator::create(cEnvironment_,
								  getType(),
								  vecResult);
	}

	if (vecChecked[0]->isFetch()
		|| (cFileSet.isEmpty() && cIndexMap.GETSIZE() > 0)) {
		//   fetch is included
		// OR
		//   no common index file
		//   and more than one files can be used
		// -> re-compose operand according to index files
		VECTOR<Interface::IPredicate*> vecOldChecked(vecChecked);
		vecChecked.erase(vecChecked.begin(), vecChecked.end());

		FOREACH(vecOldChecked,
				IndexRecomposer(cEnvironment_,
								getType(),
								cIndexMap,
								&vecChecked,
								&cFileSet,
								cArgument_.m_bNoTop));
	}
	Interface::IPredicate* pNotChecked = 0;
	if (!vecNotChecked.ISEMPTY()) {
		; _SYDNEY_ASSERT(cArgument_.m_bNoTop == false);
		// set non-checked part to candidate
		pNotChecked = Combinator::create(cEnvironment_,
										 getType(),
										 vecNotChecked);
	}
	return CheckedInterface::create(cEnvironment_,
									Combinator::create(cEnvironment_,
													   getType(),
													   vecChecked),
									vecChecked,
									pNotChecked,
									pCandidate,
									cFileSet,
									cArgument_.m_bNoTop);
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::getCheckUnknown -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::AndImpl<Handle_>::
getCheckUnknown(Opt::Environment& cEnvironment_,
				Predicate::CheckUnknownArgument& cArgument_)
{
	Utility::ScalarSet cKey;
	AndUnknownChecker cChecker(cEnvironment_, cKey);
	if (isAll(cChecker)) {
		cArgument_.m_cKey = cKey;
		cArgument_.m_pPredicate = this;
		return true;
	} else {
		return false;
	}
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::isFetch -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::AndImpl<Handle_>::
isFetch()
{
	return isAny(boost::bind(&Handle_::Operand::isFetch,
							 _1));
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::getFetchKey -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::AndImpl<Handle_>::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	Utility::ScalarSet cTmpFetchKey;
	bool bResult = isAny(boost::bind(&Handle_::Operand::getFetchKey,
									 _1,
									 boost::ref(cEnvironment_),
									 boost::ref(cTmpFetchKey)));
	bResult = bResult && (cFetchKey_.isContaining(cTmpFetchKey) == false);
	if (bResult) {
		cFetchKey_.merge(cTmpFetchKey);
	}
	return bResult;
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::estimateCost -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::AndImpl<Handle_>::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	return foreachOperand(AndCostEstimator(cEnvironment_,
										   cResult_)).getResult();
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::checkRate -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::RelationSet& cTable_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
template <class Handle_>
AccessPlan::Cost::Value
Impl::AndImpl<Handle_>::
checkRate(Opt::Environment& cEnvironment_,
		  const Utility::RelationSet& cTable_)
{
	AccessPlan::Cost::Value cInit(1.0);
	return getMinOperand(cInit,
						 boost::bind(&Interface::IPredicate::checkRate,
									 _1,
									 boost::ref(cEnvironment_),
									 boost::cref(cTable_)));
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::AndImpl<Handle_>::generate -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
template <class Handle_>
int
Impl::AndImpl<Handle_>::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the predicate
	int iID = getNodeVariable(pIterator_, cArgument_);
	if (iID < 0) {
		typename Handle_::MapIntResult vecID;

		// generate operands
		mapOperand(vecID,
				   boost::bind(&Handle_::Operand::generate,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cProgram_),
							   pIterator_,
							   boost::ref(cArgument_)));

		Execution::Interface::IPredicate* pAnd =
			Execution::Predicate::Combinator::And::create(cProgram_,
														  pIterator_,
														  vecID);
		iID = pAnd->getID();

		// register predicate <-> id relationship
		setNodeVariable(pIterator_, cArgument_, iID);
	}
	return iID;
}

// TEMPLATE FUNCTION private
//	Predicate::Impl::AndImpl<Handle_>::explainOperator -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
template <class Handle_>
const char*
Impl::AndImpl<Handle_>::
explainOperator() const
{
	return _pszOperatorName[_Explain::And];
}

////////////////////////////////////
//	Predicate::Impl::OrImpl

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::estimateRewrite -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
template <class Handle_>
int
Impl::OrImpl<Handle_>::
estimateRewrite(Opt::Environment& cEnvironment_)
{
	return foreachOperand(OrRewriteEstimator(cEnvironment_)).getVal();
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::rewrite -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
template <class Handle_>
Interface::IPredicate::RewriteResult
Impl::OrImpl<Handle_>::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	VECTOR<OrRewriter::Element> vecResult;
	foreachOperand(OrRewriter(cEnvironment_,
							  pRelation_,
							  &vecResult,
							  cArgument_));

	if (cArgument_.m_bCheckUnion && vecResult.GETSIZE() != 1) {
		VECTOR<Interface::IRelation*> vecRelation;
		vecRelation.reserve(vecResult.GETSIZE());

		FOREACH(vecResult,
				OrMerger(cEnvironment_,
						 &vecRelation));
		return Interface::IPredicate::RewriteResult(
						Relation::Union::DistinctByKey::create(cEnvironment_,
															   vecRelation),
						0);
	} else {
		VECTOR<Interface::IPredicate*> vecPredicate;
		VECTOR<OrRewriter::Element>::ITERATOR iterator = vecResult.begin();
		const VECTOR<OrRewriter::Element>::ITERATOR last = vecResult.end();
		for (; iterator != last; ++iterator) {
			Opt::ForEach((*iterator).second.second,
						 PredicateMerger(cEnvironment_,
										 (*iterator).second.first,
										 &vecPredicate));
		}
		return Interface::IPredicate::RewriteResult(
						pRelation_,
						Combinator::create(cEnvironment_,
										   getType(),
										   vecPredicate));
	}
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::check -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
template <class Handle_>
Interface::IPredicate*
Impl::OrImpl<Handle_>::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	VECTOR<Interface::IPredicate*> vecChecked;
	VECTOR<Interface::IPredicate*> vecNotChecked;
	Utility::FileSet cFileSet;
	IndexMap cIndexMap;
	Candidate::Table* pCandidate = 0;

	CheckArgument cOperandArgument(cArgument_);
	cOperandArgument.m_bNoTop = true;
	// divide operands according to refering tables
	foreachOperand(IndexChecker(cEnvironment_,
								cOperandArgument,
								&vecChecked,
								&vecNotChecked,
								0, /* index map is not used */
								&pCandidate,
								&cFileSet));

	if (vecChecked.ISEMPTY()) {
		// no relationships with pTable_
		; _SYDNEY_ASSERT(vecNotChecked.ISEMPTY() == false);
		return Combinator::create(cEnvironment_,
								  getType(),
								  vecNotChecked);
	}

	if (!vecNotChecked.ISEMPTY()) {
		// As for OR, it is not checked unless all the operands are checked
		// create new node using checked and non-checked operands
		VECTOR<Interface::IPredicate*> vecResult(vecChecked);
		vecResult.reserve(getOperandSize());
		vecResult.insert(vecResult.end(),
						 vecNotChecked.begin(),
						 vecNotChecked.end());
		return Combinator::create(cEnvironment_,
								  getType(),
								  vecResult);
	} else {
		return CheckedInterface::create(cEnvironment_,
										Combinator::create(cEnvironment_,
														   getType(),
														   vecChecked),
										vecChecked,
										pCandidate,
										cFileSet,
										cArgument_.m_bNoTop);
	}
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::getCheckUnknown -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::OrImpl<Handle_>::
getCheckUnknown(Opt::Environment& cEnvironment_,
				Predicate::CheckUnknownArgument& cArgument_)
{
	Utility::ScalarSet cKey;
	OrUnknownChecker cChecker(cEnvironment_, cKey);
	if (isAll(cChecker)) {
		cArgument_.m_cKey = cKey;
		cArgument_.m_pPredicate = this;
		return true;
	} else {
		return false;
	}
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::estimateCost -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::OrImpl<Handle_>::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	return foreachOperand(OrCostEstimator(cEnvironment_,
										  cResult_)).getResult();
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::checkRate -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Utility::RelationSet& cTable_
//	
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
template <class Handle_>
AccessPlan::Cost::Value
Impl::OrImpl<Handle_>::
checkRate(Opt::Environment& cEnvironment_,
		  const Utility::RelationSet& cTable_)
{
	AccessPlan::Cost::Value cInit(0.0);
	return getMaxOperand(cInit,
						 boost::bind(&Interface::IPredicate::checkRate,
									 _1,
									 boost::ref(cEnvironment_),
									 boost::cref(cTable_)));
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::OrImpl<Handle_>::generate -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
template <class Handle_>
int
Impl::OrImpl<Handle_>::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the predicate
	int iID = getNodeVariable(pIterator_, cArgument_);
	if (iID < 0) {
		typename Handle_::MapIntResult vecID;

		// generate operands
		mapOperand(vecID,
				   boost::bind(&Handle_::Operand::generate,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cProgram_),
							   pIterator_,
							   boost::ref(cArgument_)));

		Execution::Interface::IPredicate* pOr =
			Execution::Predicate::Combinator::Or::create(cProgram_,
														 pIterator_,
														 vecID);
		iID = pOr->getID();
		// register predicate <-> id relationship
		setNodeVariable(pIterator_, cArgument_, iID);
	}
	return iID;
}

// TEMPLATE FUNCTION private
//	Predicate::Impl::OrImpl<Handle_>::explainOperator -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
template <class Handle_>
const char*
Impl::OrImpl<Handle_>::
explainOperator() const
{
	return _pszOperatorName[_Explain::Or];
}

////////////////////////////////////
//	Predicate::Impl::ChoiceImpl

// TEMPLATE FUNCTION public
//	Predicate::Impl::ChoiceImpl<Handle_>::check -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
template <class Handle_>
Interface::IPredicate*
Impl::ChoiceImpl<Handle_>::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	VECTOR<Interface::IPredicate*> vecChecked;
	VECTOR<Interface::IPredicate*> vecNotChecked;

	// divide operands according to refering tables
	foreachOperand(IndexChecker(cEnvironment_,
								cArgument_,
								&vecChecked,
								&vecNotChecked,
								0,
								0,
								0));

	if (vecChecked.ISEMPTY()) {
		// no relationships with pTable_
		; _SYDNEY_ASSERT(vecNotChecked.ISEMPTY() == false);
		return Combinator::create(cEnvironment_,
								  getType(),
								  vecNotChecked);
	}

	// As for CHOICE, it is enough to use only checked operands
	return CheckedInterface::create(cEnvironment_,
									Combinator::create(cEnvironment_,
													   getType(),
													   vecChecked));
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::ChoiceImpl<Handle_>::getCheckUnknown -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Handle_>
bool
Impl::ChoiceImpl<Handle_>::
getCheckUnknown(Opt::Environment& cEnvironment_,
				Predicate::CheckUnknownArgument& cArgument_)
{
	return getOperandi(0)->getCheckUnknown(cEnvironment_, cArgument_);
}

// TEMPLATE FUNCTION public
//	Predicate::Impl::ChoiceImpl<Handle_>::generate -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
template <class Handle_>
int
Impl::ChoiceImpl<Handle_>::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// use first choice
	return getOperandi(0)->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

// TEMPLATE FUNCTION private
//	Predicate::Impl::ChoiceImpl<Handle_>::explainOperator -- 
//
// TEMPLATE ARGUMENTS
//	class Handle_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
template <class Handle_>
const char*
Impl::ChoiceImpl<Handle_>::
explainOperator() const
{
	return _pszOperatorName[_Explain::Choice];
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_COMBINATORIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
