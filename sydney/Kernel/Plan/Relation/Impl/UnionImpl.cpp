// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/UnionImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Relation/Impl/UnionImpl.h"
#include "Plan/Relation/Impl/SortImpl.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Union.h"
#include "Plan/Candidate/Sort.h"
#include "Plan/Candidate/Distinct.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/Selection.h"
#include "Plan/Scalar/Field.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

namespace
{
	class _Inquirer
	{
	public:
		_Inquirer(Opt::Environment& cEnvironment_,
				  const InquiryArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_bPropagate(cArgument_.m_iTarget & (InquiryArgument::Target::Depending
												   | InquiryArgument::Target::Refering)),
			  m_iResult(0)
		{}

		void operator()(Interface::IRelation* pRelation_);

		Interface::IRelation::InquiryResult getResult()
		{return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const InquiryArgument& m_cArgument;
		bool m_bPropagate;
		Interface::IRelation::InquiryResult m_iResult;
	};
}

////////////////////////
// $$$::_Inquirer

// FUNCTION local
//	$$$::_Inquirer::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Inquirer::
operator()(Interface::IRelation* pRelation_)
{
	if (m_bPropagate) {
		m_iResult |= pRelation_->inquiry(m_cEnvironment,
										 m_cArgument);
	}
}

////////////////////////////////////////
// Relation::UnionImpl::Nadic

// FUNCTION public
//	Relation::UnionImpl::Nadic::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//virtual
PAIR<Interface::IRelation*, Interface::IPredicate*>
UnionImpl::Nadic::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IPredicate* pPredicate_,
		Predicate::RewriteArgument& cArgument_)
{
	mapOperand(boost::bind(&This::rewriteOperand,
						   this,
						   boost::ref(cEnvironment_),
						   pPredicate_,
						   boost::ref(cArgument_),
						   _1));
	return MAKEPAIR(static_cast<Interface::IRelation*>(this),
					static_cast<Interface::IPredicate*>(0));
}

// FUNCTION public
//	Relation::UnionImpl::Nadic::createAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
UnionImpl::Nadic::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	AccessPlan::Source cMySource(cPlanSource_);
	if (cMySource.getOrder()
		&& !hasSortKey(cEnvironment_, cMySource.getOrder())) {
		cMySource.eraseOrder();
	}
		
	// limit specification is converted to distribute spec
	AccessPlan::Source cOperandSource(cMySource,
									  *cMySource.getLimit().getDistributeSpec(cEnvironment_));
	//cOperandSource.erasePredicate();
	cOperandSource.estimateLimit(cEnvironment_);

	// create each operand's plan
	MapResult<Interface::ICandidate*> vecCandidate;
	mapOperand(vecCandidate,
			   boost::bind(&This::createOperandAccessPlan,
						   this,
						   boost::ref(cEnvironment_),
						   boost::ref(cOperandSource),
						   _1));
	
	Interface::ICandidate* pResult =
		Candidate::Union::create(cEnvironment_,
								 this,
								 vecCandidate,
								 cMySource);
	if (cMySource.getOrder()) {
		cMySource.getOrder()->require(cEnvironment_, pResult);
	}
	pResult->checkPredicate(cEnvironment_,
							cMySource);
	return pResult;
}


// FUNCTION private
//	Relation::UnionImpl::Nadic::createOperandAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
UnionImpl::Nadic::
createOperandAccessPlan(Opt::Environment& cEnvironment_,
						AccessPlan::Source& cPlanSource_,
						Operand* pOperand_)
{
	AccessPlan::Source cOperandSource(cPlanSource_);

	return Impl::SortImpl::createSortAccessPlan(cEnvironment_,
												pOperand_,
												cOperandSource,
												cPlanSource_.getOrder());
}

// FUNCTION public
//	Relation::UnionImpl::Nadic::inquiry -- 
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
UnionImpl::Nadic::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION public
//	Relation::UnionImpl::Nadic::require -- 
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
UnionImpl::Nadic::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::UnionImpl::Nadic::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UnionImpl::Nadic::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

// FUNCTION private
//	Relation::UnionImpl::Nadic::rewriteOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	Predicate::RewriteArgument& cArgument_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

Interface::IRelation*
UnionImpl::Nadic::
rewriteOperand(Opt::Environment& cEnvironment_,
			   Interface::IPredicate* pPredicate_,
			   Predicate::RewriteArgument& cArgument_,
			   Interface::IRelation* pOperand_)
{
	Interface::IPredicate::RewriteResult cResult =
		pPredicate_->rewrite(cEnvironment_,
							 pOperand_,
							 cArgument_);
	if (cResult.getPredicate()) {
		return Selection::create(cEnvironment_,
								 cResult.getPredicate(),
								 cResult.getRelation());
	} else {
		return cResult.getRelation();
	}
}


// FUNCTION private
//	Relation::UnionImpl::Nadic::hasSortKey -- 
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pSpecification_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
UnionImpl::Nadic::
hasSortKey(Opt::Environment& cEnvironment_,
		   Order::Specification* pSpecification_)
{
	Utility::RelationSet cUsedTable;
	getUsedTable(cEnvironment_, cUsedTable);
	return cUsedTable.isAny(boost::bind(&Order::Specification::isRefering,
										pSpecification_,
										_1,
										0));
}


//
// Copyright (c) 2010, 2011, 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
