// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/GroupingColumnReference.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/GroupingColumnReference.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"

#include "Statement/GroupingColumnReference.h"
#include "Statement/ItemReference.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::GroupingColumnReferenceImpl --
	//
	// NOTES
	class GroupingColumnReferenceImpl
		: public Query::GroupingColumnReference
	{
	public:
		typedef GroupingColumnReferenceImpl This;
		typedef Query::GroupingColumnReference Super;

		// constructor
		GroupingColumnReferenceImpl() : Super() {}
		// destructor
		~GroupingColumnReferenceImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
	virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;		


	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::GroupingColumnReferenceImpl _analyzer;

} // namespace

///////////////////////////////////////////////////////////
// Analysis::Query::Impl::GroupingColumnReferenceImpl
///////////////////////////////////////////////////////////

// FUNCTION public
//	Query::Impl::GroupingColumnReferenceImpl::addScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	VECTOR<Plan::Interface::IScalar*>& vecScalar_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual


Plan::Relation::RowElement*
Impl::GroupingColumnReferenceImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const

{
	Statement::GroupingColumnReference* pGCR =
		_SYDNEY_DYNAMIC_CAST(Statement::GroupingColumnReference*, pStatement_);
	; _SYDNEY_ASSERT(pGCR);	

	Statement::Object* pObj = pGCR->getReference()
	; _SYDNEY_ASSERT(pObj);
	Plan::Relation::RowElement* pResult =
		pObj->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pObj);
	return pResult;
};

//////////////////////////////////////////
// Query::GroupingColumnReference
//////////////////////////////////////////

// FUNCTION public
//	Query::GroupingColumnReference::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::GroupingColumnReference* pStatement_
//	
// RETURN
//	const GroupingColumnReference*
//
// EXCEPTIONS

//static
const GroupingColumnReference*
GroupingColumnReference::
create(const Statement::GroupingColumnReference* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
