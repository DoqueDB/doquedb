// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Selection.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/Selection.h"
#include "Plan/Relation/Filter.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/Combinator.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::SelectionImpl --implementation class of Selection
	//
	// NOTES
	class SelectionImpl
		: public Filter<Relation::Selection>
	{
	public:
		typedef Filter<Relation::Selection> Super;
		typedef SelectionImpl This;

		SelectionImpl(Interface::IPredicate* pPredicate_,
					  Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_pPredicate(pPredicate_)
		{}
		~SelectionImpl() {}

	///////////////////////////////////////
	// Interface::IRelation::
		virtual int estimateUnion(Opt::Environment& cEnvironment_);
		virtual PAIR<Interface::IRelation*, Interface::IPredicate*>
						unwind(Opt::Environment& cEnvironment_);
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);

		virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
		
	protected:
	private:
		bool checkUnionAvailable(Opt::Environment& cEnvironment_,
								 Interface::IPredicate* pPredicate_);

		Interface::IPredicate* m_pPredicate;
	};
} // namespace Impl

/////////////////////////////////////
// Relation::Impl::SelectionImpl

// FUNCTION public
//	Relation::Impl::SelectionImpl::estimateUnion -- 
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
int
Impl::SelectionImpl::
estimateUnion(Opt::Environment& cEnvironment_)
{
	return getOperand()->estimateUnion(cEnvironment_)
		* m_pPredicate->estimateRewrite(cEnvironment_);
}

// FUNCTION public
//	Relation::Impl::SelectionImpl::unwind -- unwind subquery
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	PAIR<Interface::IRelation*, Interface::IPredicate*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IRelation*, Interface::IPredicate*>
Impl::SelectionImpl::
unwind(Opt::Environment& cEnvironment_)
{
	PAIR<Interface::IRelation*, Interface::IPredicate*> cUnwind =
		getOperand()->unwind(cEnvironment_);

	Interface::IPredicate* pPredicate =
		(cUnwind.second) ?
		Predicate::Combinator::create(cEnvironment_,
									  Tree::Node::And,
									  MAKEPAIR(cUnwind.second,
											   m_pPredicate))
		: m_pPredicate;

	Predicate::RewriteArgument cArgument;
	cArgument.m_bCheckUnion = checkUnionAvailable(cEnvironment_,
												  pPredicate);

	Interface::IPredicate::RewriteResult cResult =
		pPredicate->rewrite(cEnvironment_,
							cUnwind.first,
							cArgument);

	cUnwind.first = cResult.getRelation();
	cUnwind.second = cResult.getPredicate();

	// return unwind result
	return cUnwind;
}

// FUNCTION public
//	Relation::Impl::SelectionImpl::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
Impl::SelectionImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::IPredicate* pPredicate = m_pPredicate;
	if (cPlanSource_.getPredicate()) {
		pPredicate = Predicate::Combinator::create(cEnvironment_,
												   Tree::Node::And,
												   MAKEPAIR(cPlanSource_.getPredicate(),
															m_pPredicate));
	}
	Predicate::RewriteArgument cArgument;
	bool bUnionAvailable = checkUnionAvailable(cEnvironment_,
											   pPredicate);
	bool bUnionDisabled = false;

	cArgument.m_bCheckUnion = bUnionAvailable;
	
	Interface::IPredicate::RewriteResult cResult =
		pPredicate->rewrite(cEnvironment_,
							getOperand(),
							cArgument);

	AccessPlan::Source cSource(cPlanSource_, cResult.getPredicate());

	if( isGrouping()) cResult.getRelation()->setGrouping();

 retry:
	Interface::ICandidate* pResult =
		cResult.getRelation()->createAccessPlan(cEnvironment_, cSource);

	if (bUnionDisabled
		&& Order::Specification::isCompatible(pResult->getOrder(),
											  cPlanSource_.getOrder()) == false) {
		// if union is disabled because of ordering specification
		// but result candidate can't sort -> consider union
		Interface::ICandidate::erase(cEnvironment_,
									 pResult);

		bUnionDisabled = false;
		cArgument.m_bCheckUnion = true;
		cResult = pPredicate->rewrite(cEnvironment_,
									  getOperand(),
									  cArgument);
		cSource.setPredicate(cResult.getPredicate());
		cSource.eraseLimit();
		cSource.eraseEstimateLimit();
		cSource.eraseOrder();
		goto retry;
	}
	return pResult;
}

// FUNCTION public
//	Relation::Impl::SelectionImpl::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
Impl::SelectionImpl::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryResult iResult = getOperand()->inquiry(cEnvironment_,
												  cArgument_);
	if (cArgument_.m_iTarget & InquiryArgument::Target::Refering
		&& (iResult & InquiryArgument::Target::Refering) == 0) {
		; _SYDNEY_ASSERT(cArgument_.m_pRelation);

		Utility::RelationSet cRelationSet;
		Utility::RelationSet cUsedRelationSet;
		cArgument_.m_pRelation->getUsedTable(cEnvironment_,
											 cRelationSet);

		m_pPredicate->getUsedTable(cUsedRelationSet);
		if (cRelationSet.isContainingAny(cUsedRelationSet)) {
			iResult |= InquiryArgument::Target::Refering;
		}
	}
	if (cArgument_.m_iTarget & InquiryArgument::Target::Depending
		&& (iResult & InquiryArgument::Target::Depending) == 0) {
		; _SYDNEY_ASSERT(cArgument_.m_pRelation);

		Utility::RelationSet cRelationSet;
		Utility::RelationSet cUsedRelationSet;
		cArgument_.m_pRelation->getUsedTable(cEnvironment_,
											 cRelationSet);

		m_pPredicate->getUsedTable(cUsedRelationSet);
		if (cRelationSet.isContainingAny(cUsedRelationSet)) {
			iResult |= InquiryArgument::Target::Depending;
		}
	}
	return iResult;
}

// FUNCTION public
//	Relation::Impl::SelectionImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SelectionImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pPredicate->require(cEnvironment_,
						  pCandidate_);
	getOperand()->require(cEnvironment_,
						  pCandidate_);
}


// FUNCTION public
//	Relation::TableImpl::Retrieve::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
Impl::SelectionImpl::
generateSQL(Opt::Environment& cEnvironment_)
{

	Plan::Sql::Query* pResult =  Super::generateSQL(cEnvironment_);
	m_pPredicate->retrieveFromCascade(cEnvironment_, pResult);
	return pResult;
}

// FUNCTION private
//	Relation::Impl::SelectionImpl::checkUnionAvailable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::SelectionImpl::
checkUnionAvailable(Opt::Environment& cEnvironment_,
					Interface::IPredicate* pPredicate_)
{
	int iEstimate = pPredicate_->estimateRewrite(cEnvironment_);
	if (iEstimate > 1) {
		iEstimate *= getOperand()->estimateUnion(cEnvironment_);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "Check Union:"
				   << "estimate: " << iEstimate << " "
				   << "max number: " << Opt::Configuration::getUnionMaxNumber().get();
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif

		return iEstimate <= Opt::Configuration::getUnionMaxNumber().get();
	}
	return false;
}

/////////////////////////////////////
// Relation::Selection

// FUNCTION public
//	Relation::Selection::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Selection*
//
// EXCEPTIONS

//static
Selection*
Selection::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pPredicate_,
	   Interface::IRelation* pOperand_)
{
	AUTOPOINTER<This> pResult = new Impl::SelectionImpl(pPredicate_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Selection::Selection -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Selection::
Selection()
	: Super(Tree::Node::Selection)
{}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
