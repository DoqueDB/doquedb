// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/UpdateSetClause.cpp --
// 
// Copyright (c) 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Operation/UpdateSetClause.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowInfo.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/UpdateSetClause.h"
#include "Statement/UpdateSetClause.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace UpdateSetClauseImpl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Operation::UpdateSetClauseImpl::Single --
	//			implementation class of update set clause analyzer
	//
	// NOTES
	class Single
		: public Operation::UpdateSetClause
	{
	public:
		typedef Single This;
		typedef Operation::UpdateSetClause Super;

		// constructor
		Single() : Super() {}
		// destructor
		~Single() {}

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

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Operation::UpdateSetClauseImpl::Multiple --
	//			implementation class of update set clause analyzer
	//
	// NOTES
	class Multiple
		: public Operation::UpdateSetClause
	{
	public:
		typedef Multiple This;
		typedef Operation::UpdateSetClause Super;

		// constructor
		Multiple() : Super() {}
		// destructor
		~Multiple() {}

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
	const UpdateSetClauseImpl::Single _analyzerSingle;
	const UpdateSetClauseImpl::Multiple _analyzerMultiple;

} // namespace

///////////////////////////////////////////////////////////
// Analysis::Operation::UpdateSetClauseImpl::Single
///////////////////////////////////////////////////////////

// FUNCTION public
//	Operation::UpdateSetClauseImpl::Single::getDegree -- get element numbers added by following methods
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
UpdateSetClauseImpl::Single::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}
	

// FUNCTION public
//	Operation::UpdateSetClauseImpl::Single::addColumns -- add RowInfo::Element from Statement::Object
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
UpdateSetClauseImpl::Single::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::UpdateSetClause* pUSC =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateSetClause*, pStatement_);
	; _SYDNEY_ASSERT(pUSC);
	; _SYDNEY_ASSERT(pUSC->getColumnName());

	// single value
	Statement::ValueExpression* pVE = pUSC->getSource();
	; _SYDNEY_ASSERT(pVE);

	Plan::Relation::RowElement* pElement =
		pVE->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pVE);

	pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pElement));
}

///////////////////////////////////////////////////////////
// Analysis::Operation::UpdateSetClauseImpl::Multiple
///////////////////////////////////////////////////////////

// FUNCTION public
//	Operation::UpdateSetClauseImpl::Multiple::getDegree -- get element numbers added by following methods
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
UpdateSetClauseImpl::Multiple::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::UpdateSetClause* pUSC =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateSetClause*, pStatement_);
	; _SYDNEY_ASSERT(pUSC);
	; _SYDNEY_ASSERT(pUSC->getColumnNameList());

	// multiple value
	Statement::ValueExpression* pVE = pUSC->getSource();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Rowconst
					 || pVE->getOperator() == Statement::ValueExpression::op_RowSubquery);

	return pVE->getAnalyzer2()->getDegree(cEnvironment_,
										  pVE);
}

// FUNCTION public
//	Operation::UpdateSetClauseImpl::Multiple::addColumns -- add RowInfo::Element from Statement::Object
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
UpdateSetClauseImpl::Multiple::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::UpdateSetClause* pUSC =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateSetClause*, pStatement_);
	; _SYDNEY_ASSERT(pUSC);
	; _SYDNEY_ASSERT(pUSC->getColumnNameList());

	// multiple value
	Statement::ValueExpression* pVE = pUSC->getSource();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pUSC->getColumnNameList()->getCount() == 1
					 || (pVE->getOperator() == Statement::ValueExpression::op_Rowconst
						 || pVE->getOperator() == Statement::ValueExpression::op_RowSubquery));

	if (pVE->getOperator() == Statement::ValueExpression::op_Rowconst
		|| pVE->getOperator() == Statement::ValueExpression::op_RowSubquery) {
		pVE->getAnalyzer2()->addColumns(cEnvironment_,
										pRowInfo_,
										pRelation_,
										pVE);
	} else {
		Plan::Relation::RowElement* pElement =
			pVE->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pVE);

		pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pElement));
	}
}

////////////////////////////////////
// Operation::UpdateSetClause
////////////////////////////////////

// FUNCTION public
//	Operation::UpdateSetClause::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::UpdateSetClause* pStatement_
//	
// RETURN
//	const UpdateSetClause*
//
// EXCEPTIONS

//static
const UpdateSetClause*
UpdateSetClause::
create(const Statement::UpdateSetClause* pStatement_)
{
	if (pStatement_->getColumnName()) {
		return &_analyzerSingle;
	} else {
		return &_analyzerMultiple;
	}
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
