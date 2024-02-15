// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/UpdateSetClauseList.cpp --
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
const char moduleName[] = "Analysis::Operation";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Operation/UpdateSetClauseList.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/UpdateSetClause.h"
#include "Statement/UpdateSetClauseList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Operation::Impl::UpdateSetClauseListImpl --
	//			implementation class of update set clause list analyzer
	//
	// NOTES
	class UpdateSetClauseListImpl
		: public Operation::UpdateSetClauseList
	{
	public:
		typedef UpdateSetClauseListImpl This;
		typedef Operation::UpdateSetClauseList Super;

		// constructor
		UpdateSetClauseListImpl() : Super() {}
		// destructor
		~UpdateSetClauseListImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		// get element numbers added by following methods
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;	
		// add RowInfo::Element from Statement::Object
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
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
	const Impl::UpdateSetClauseListImpl _analyzer;

} // namespace

///////////////////////////////////////////////////////////
// Analysis::Operation::Impl::UpdateSetClauseListImpl
///////////////////////////////////////////////////////////

// FUNCTION public
//	Operation::getDegree -- get element numbers added by following methods
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::UpdateSetClauseListImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::UpdateSetClauseList* pUSCL =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateSetClauseList*, pStatement_);
	; _SYDNEY_ASSERT(pUSCL);

	// count numbers
	int n = pUSCL->getCount();
	int iResult = 0;
	for (int i = 0; i < n; ++i) {
		Statement::UpdateSetClause* pUSC = pUSCL->getSetClauseAt(i);
		iResult += pUSC->getAnalyzer2()->getDegree(cEnvironment_,
												   pUSC);
	}
	return iResult;
}

// FUNCTION public
//	Operation::Impl::UpdateSetClauseListImpl::addColumns -- add RowInfo::Element from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UpdateSetClauseListImpl::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::UpdateSetClauseList* pUSCL =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateSetClauseList*, pStatement_);
	; _SYDNEY_ASSERT(pUSCL);

	// add to rowinfo
	int n = pUSCL->getCount();
	for (int i = 0; i < n; ++i) {
		Statement::UpdateSetClause* pUSC = pUSCL->getSetClauseAt(i);
		pUSC->getAnalyzer2()->addColumns(cEnvironment_,
										 pRowInfo_,
										 pRelation_,
										 pUSC);
	}
}

////////////////////////////////////
// Operation::UpdateSetClauseList
////////////////////////////////////

// FUNCTION public
//	Operation::UpdateSetClauseList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::UpdateSetClauseList* pStatement_
//	
// RETURN
//	const UpdateSetClauseList*
//
// EXCEPTIONS

//static
const UpdateSetClauseList*
UpdateSetClauseList::
create(const Statement::UpdateSetClauseList* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
