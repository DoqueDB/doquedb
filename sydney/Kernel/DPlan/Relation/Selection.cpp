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
const char moduleName[] = "DPlan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "DPlan/Relation/Selection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Filter.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/Combinator.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::SelectionImpl --implementation class of Selection
	//
	// NOTES
	class SelectionImpl
		: public Plan::Relation::Filter<Relation::Selection>
	{
	public:
		typedef Plan::Relation::Filter<Relation::Selection> Super;
		typedef SelectionImpl This;

		SelectionImpl(Plan::Interface::IPredicate* pPredicate_,
					  Plan::Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_pPredicate(pPredicate_)
		{}
		~SelectionImpl() {}

	///////////////////////////////////////
	// Interface::IRelation::

		virtual Plan::Interface::ICandidate*
		createAccessPlan(Opt::Environment& cEnvironment_,
						 Plan::AccessPlan::Source& cPlanSource_);
		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);


		
	protected:
	private:
		Plan::Interface::IPredicate* m_pPredicate;
	};
} // namespace Impl




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
Plan::Interface::ICandidate*
Impl::SelectionImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Interface::IPredicate* pPredicate = m_pPredicate;
	if (cPlanSource_.getPredicate()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
/*	
	Plan::Predicate::RewriteArgument cArgument;
	Plan::Interface::IPredicate::RewriteResult cResult =
		pPredicate->rewrite(cEnvironment_, getOperand(), cArgument);
*/
	Plan::AccessPlan::Source cSource(cPlanSource_, pPredicate);
	/*
	Plan::Interface::ICandidate* pResult =
		cResult.getRelation()->createAccessPlan(cEnvironment_, cSource);
	*/
	
	return getOperand()->createAccessPlan(cEnvironment_, cSource);
}

// FUNCTION public
//	Impl::SelectionImpl::generateSQL -- 
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
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	m_pPredicate->retrieveFromCascade(cEnvironment_, pResult);
	return pResult;
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
	   Plan::Interface::IPredicate* pPredicate_,
	   Plan::Interface::IRelation* pOperand_)
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
	: Super(Plan::Tree::Node::Selection)
{}

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
