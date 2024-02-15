// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/TableValueConstructor.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Query/TableValueConstructor.h"

#include "Common/Assert.h"

#include "Exception/InvalidRowValue.h"
#include "Exception/NotSupported.h"

#include "Plan/Relation/ValueList.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::TableValueConstructorImpl --
	//			implementation class of tablevalueconstructor analyzer
	//
	// NOTES
	class TableValueConstructorImpl
		: public Query::TableValueConstructor
	{
	public:
		typedef TableValueConstructorImpl This;
		typedef Query::TableValueConstructor Super;

		// constructor
		TableValueConstructorImpl() : Super() {}
		// destructor
		~TableValueConstructorImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
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
	const Impl::TableValueConstructorImpl _analyzer;

} // namespace

///////////////////////////////////////////////////
// Analysis::Query::TableValueConstructorImpl::Primitive
///////////////////////////////////////////////////

// FUNCTION public
//	Query::Impl::TableValueConstructorImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::TableValueConstructorImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Tblconst);

	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());

	VECTOR< VECTOR<Plan::Interface::IScalar*> > vecValues;
	if (int n = pVEL->getCount()) {
		vecValues.reserve(n);
		int i = 0;
		SIZE iDegree = 0;
		do {
			Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(i);
			VECTOR<Plan::Interface::IScalar*> vecScalar;
			pElement->getAnalyzer2()->addScalar(cEnvironment_,
												0,
												pElement,
												vecScalar);
			if (i == 0) {
				iDegree = vecScalar.GETSIZE();
			} else if (iDegree != vecScalar.GETSIZE()) {
				_SYDNEY_THROW0(Exception::InvalidRowValue);
			}

			vecValues.PUSHBACK(vecScalar);

			++i;
		} while (i < n);
	}
	return Plan::Relation::ValueList::create(cEnvironment_,
											 vecValues);
}

// FUNCTION public
//	Query::Impl::TableValueConstructorImpl::getDistributeRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::TableValueConstructorImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::Impl::TableValueConstructorImpl::getDegree -- 
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
Impl::TableValueConstructorImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Tblconst);

	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());

	// use first element
	Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(0);
	return pElement->getAnalyzer2()->getDegree(cEnvironment_,
											   pElement);
}

/////////////////////////////
// Query::TableValueConstructor
/////////////////////////////

// FUNCTION public
//	Query::TableValueConstructor::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const TableValueConstructor*
//
// EXCEPTIONS

//static
const TableValueConstructor*
TableValueConstructor::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
