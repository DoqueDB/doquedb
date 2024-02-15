// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Expand.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Expand.h"
#include "Plan/Relation/Filter.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::ExpandImpl --implementation class of Expand
	//
	// NOTES

	class ExpandImpl
		: public Relation::Filter<Relation::Expand>
	{
	public:
		typedef Relation::Filter<Relation::Expand> Super;
		typedef ExpandImpl This;

		ExpandImpl(Interface::IRelation* pOperand_,
				   Order::Specification* pSortSpecification_,
				   Interface::IScalar* pLimit_)
			: Super(pOperand_),
			  m_pSortSpecification(pSortSpecification_),
			  m_pLimit(pLimit_)
		{}
		~ExpandImpl() {}

	////////////////////////
	// Interface::IRelation
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);



	//////////////////////////
	// Tree::Node::
		virtual ModSize getOptionSize() const;
		virtual const Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const;

	protected:
	private:
		Order::Specification* m_pSortSpecification;
		Interface::IScalar* m_pLimit;
	};
} // namespace Impl

/////////////////////////////////////
// Relation::Impl::ExpandImpl

// FUNCTION public
//	Relation::Impl::ExpandImpl::createAccessPlan -- create access plan candidate
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
Impl::ExpandImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	return getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);
}

// FUNCTION public
//	Relation::Impl::ExpandImpl::inquiry -- inquiry about relation's attributes
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
Impl::ExpandImpl::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return 0;
}

// FUNCTION public
//	Relation::Impl::ExpandImpl::require -- 
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
Impl::ExpandImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	; // do nothing
}

// FUNCTION public
//	Relation::Impl::ExpandImpl::getUsedTable -- 
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
Impl::ExpandImpl::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	; // do nothing
}




// FUNCTION public
//	Relation::Impl::ExpandImpl::getOptionSize -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
Impl::ExpandImpl::
getOptionSize() const
{
	return (m_pSortSpecification ? 1 : 0)
		+ (m_pLimit ? 1 : 0);
}

// FUNCTION public
//	Relation::Impl::ExpandImpl::getOptionAt -- 
//
// NOTES
//
// ARGUMENTS
//	ModInt32 iPosition_
//	
// RETURN
//	const Tree::Node::Super*
//
// EXCEPTIONS

//virtual
const Tree::Node::Super*
Impl::ExpandImpl::
getOptionAt(ModInt32 iPosition_) const
{
	int i = 0;
	if (m_pSortSpecification)
		if (i++ == iPosition_) return m_pSortSpecification;
	if (m_pLimit)
		if (i++ == iPosition_) return m_pLimit;
	return 0;
}

/////////////////////////////////////
// Relation::Expand

//constructor
//static
Expand*
Expand::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pOperand_,
	   Order::Specification* pSortSpecification_,
	   Interface::IScalar* pLimit_)
{
	AUTOPOINTER<This> pResult = new Impl::ExpandImpl(pOperand_, pSortSpecification_, pLimit_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Expand::Expand -- constructor
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

Expand::
Expand()
	: Super(Tree::Node::Expand)
{}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
