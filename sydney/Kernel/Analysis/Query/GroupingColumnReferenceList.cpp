// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/GroupingColumnReferenceList.cpp --
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

#include "Analysis/Query/GroupingColumnReferenceList.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"

#include "Statement/GroupingColumnReferenceList.h"
#include "Statement/GroupingColumnReference.h"
#include "Statement/ItemReference.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::GroupingColumnReferenceListImpl --
	//
	// NOTES
	class GroupingColumnReferenceListImpl
		: public Query::GroupingColumnReferenceList
	{
	public:
		typedef GroupingColumnReferenceListImpl This;
		typedef Query::GroupingColumnReferenceList Super;

		// constructor
		GroupingColumnReferenceListImpl() : Super() {}
		// destructor
		~GroupingColumnReferenceListImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual void addScalar(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::Object* pStatement_,
							   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
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
	const Impl::GroupingColumnReferenceListImpl _analyzer;

} // namespace

///////////////////////////////////////////////////////////
// Analysis::Query::Impl::GroupingColumnReferenceListImpl
///////////////////////////////////////////////////////////

// FUNCTION public
//	Query::Impl::GroupingColumnReferenceListImpl::addScalar -- 
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
void
Impl::GroupingColumnReferenceListImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Statement::GroupingColumnReferenceList* pGCRL =
		_SYDNEY_DYNAMIC_CAST(Statement::GroupingColumnReferenceList*, pStatement_);
	; _SYDNEY_ASSERT(pGCRL);

	if (int n = pGCRL->getCount()) {

		// create key list and order specification from grouping column
		int i = 0;
		do {
			Statement::GroupingColumnReference* pGCR =
				_SYDNEY_DYNAMIC_CAST(Statement::GroupingColumnReference*, pGCRL->getAt(i));
			; _SYDNEY_ASSERT(pGCR);

			Plan::Relation::RowElement* pRowElement =			
				pGCR->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pGCR);
			
			cEnvironment_.addGroupingColumn(pRowElement);

			vecScalar_.PUSHBACK(pRowElement->getScalar(cEnvironment_));
		} while (++i < n);
	}
};

//////////////////////////////////////////
// Query::GroupingColumnReferenceList
//////////////////////////////////////////

// FUNCTION public
//	Query::GroupingColumnReferenceList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::GroupingColumnReferenceList* pStatement_
//	
// RETURN
//	const GroupingColumnReferenceList*
//
// EXCEPTIONS

//static
const GroupingColumnReferenceList*
GroupingColumnReferenceList::
create(const Statement::GroupingColumnReferenceList* pStatement_)
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
