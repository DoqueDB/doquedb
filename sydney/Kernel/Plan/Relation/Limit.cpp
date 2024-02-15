// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Limit.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Limit.h"
#include "Plan/Relation/Filter.h"

#include "Plan/AccessPlan/Limit.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Limit.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::LimitImpl --implementation class of Limit
	//
	// NOTES

	class LimitImpl
		: public Relation::Filter<Relation::Limit>
	{
	public:
		typedef Relation::Filter<Relation::Limit> Super;
		typedef LimitImpl This;

		LimitImpl(Interface::IScalar* pLimit_,
				  Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_cLimit(pLimit_)
		{}
		LimitImpl(Interface::IScalar* pLimit_,
				  Interface::IScalar* pOffset_,
				  Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_cLimit(pLimit_, pOffset_)
		{}
		~LimitImpl() {}

	////////////////////////
	// Relation::Limit::
		virtual const AccessPlan::Limit& getLimit() {return m_cLimit;}

	////////////////////////
	// Interface::IRelation
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	protected:
	private:
		AccessPlan::Limit m_cLimit;
	};
} // namespace Impl

/////////////////////////////////////
// Relation::Impl::LimitImpl

// FUNCTION public
//	Relation::Impl::LimitImpl::createAccessPlan -- create access plan candidate
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
Impl::LimitImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	AccessPlan::Source cSource1(cPlanSource_, m_cLimit);
	cSource1.erasePredicate();
	cSource1.estimateLimit(cEnvironment_);
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cSource1);

	Interface::ICandidate* pResult = 0;
	if (pOperandCandidate->getLimit().isEquivalent(m_cLimit)) {
		// operand can process limit
		pResult = pOperandCandidate;
	} else {
		// need limit candidate
		pResult =  Candidate::Limit::create(cEnvironment_,
											this,
											pOperandCandidate);
	}
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

// FUNCTION public
//	Relation::Impl::LimitImpl::inquiry -- 
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
Impl::LimitImpl::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryArgument cMyArgument(cArgument_);
	if (cArgument_.m_iTarget & InquiryArgument::Target::Depending) {
		cMyArgument.m_iTarget |= InquiryArgument::Target::Refering;
	}
	// inquiry operand
	InquiryResult iResult = getOperand()->inquiry(cEnvironment_,
												  cMyArgument);

	if (cArgument_.m_iTarget & InquiryArgument::Target::Depending
		&& (iResult & InquiryArgument::Target::Refering)) {
		iResult |= InquiryArgument::Target::Depending;
	}
	return (iResult & cArgument_.m_iTarget);
}

/////////////////////////////////////
// Relation::Limit

// FUNCTION public
//	Relation::Limit::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pLimit_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pLimit_,
	   Interface::IRelation* pOperand_)
{
	AUTOPOINTER<This> pResult = new Impl::LimitImpl(pLimit_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::Limit::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pLimit_
//	Interface::IScalar* pOffset_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pLimit_,
	   Interface::IScalar* pOffset_,
	   Interface::IRelation* pOperand_)
{
	if (pOffset_ == 0) return create(cEnvironment_, pLimit_, pOperand_);

	AUTOPOINTER<This> pResult = new Impl::LimitImpl(pLimit_, pOffset_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Limit::Limit -- constructor
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

Limit::
Limit()
	: Super(Tree::Node::Limit)
{}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
