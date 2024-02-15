// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/ValueExpressionList.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Value";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Value/ValueExpressionList.h"

#include "Exception/NotSupported.h"

#include "Plan/Relation/RowInfo.h"

#include "Statement/ValueExpressionList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Value::Impl::ValueExpressionListImpl -- valueExpressionList analyzer
	//
	// NOTES
	class ValueExpressionListImpl
		: public Value::ValueExpressionList
	{
	public:
		typedef ValueExpressionListImpl This;
		typedef Value::ValueExpressionList Super;

		// constructor
		ValueExpressionListImpl() : Super() {}
		// destructor
		~ValueExpressionListImpl() {}

	/////////////////////////////
	// Interface::IAnalyzer::
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
		virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;

		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addScalar(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::Object* pStatement_,
							   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
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
	const Impl::ValueExpressionListImpl _analyzer;

} // namespace

///////////////////////////////////////////
//	Value::Impl::ValueExpressionListImpl

// FUNCTION public
//	Value::Impl::ValueExpressionListImpl::getScalar -- generate Scalar from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::ValueExpressionListImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pStatement_);
	; _SYDNEY_ASSERT(pVEL);

	if (pVEL->getCount() != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(0);
	return pElement->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pElement);
}

// FUNCTION public
//	Value::Impl::ValueExpressionListImpl::getRowElement -- generate RowElement from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowElement*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::ValueExpressionListImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pStatement_);
	; _SYDNEY_ASSERT(pVEL);

	if (pVEL->getCount() != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(0);
	return pElement->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pElement);
}

// FUNCTION public
//	Value::Impl::ValueExpressionListImpl::getDegree -- 
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
Impl::ValueExpressionListImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pStatement_);
	; _SYDNEY_ASSERT(pVEL);

	return pVEL->getCount();
}

// FUNCTION public
//	Value::Impl::ValueExpressionListImpl::addScalar -- add Scalar from Statement::Object
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
Impl::ValueExpressionListImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pStatement_);
	; _SYDNEY_ASSERT(pVEL);

	if (int n = pVEL->getCount()) {
		vecScalar_.reserve(vecScalar_.GETSIZE() + n);
		int i = 0;
		do {
			Statement::ValueExpression* pVEE = pVEL->getValueExpressionAt(i);

			// create scalar from value expression
			Plan::Interface::IScalar* pScalar =
				pVEE->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pVEE);
			; _SYDNEY_ASSERT(pScalar);

			vecScalar_.PUSHBACK(pScalar);

		} while (++i < n);
	}
}

// FUNCTION public
//	Value::Impl::ValueExpressionListImpl::addColumns -- 
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
Impl::ValueExpressionListImpl::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pStatement_);
	; _SYDNEY_ASSERT(pVEL);

	if (int n = pVEL->getCount()) {
		int i = 0;
		do {
			Statement::ValueExpression* pVEE = pVEL->getValueExpressionAt(i);

			// create rowelement from value expression
			Plan::Relation::RowElement* pElement =
				pVEE->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pVEE);
			// add to result with no specific name
			pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pElement));
		} while (++i < n);
	}
}

//////////////////////////////
// Value::ValueExpressionList
//////////////////////////////

// FUNCTION public
//	Value::ValueExpressionList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpressionList* pStatement_
//	
// RETURN
//	const ValueExpressionList*
//
// EXCEPTIONS

//static
const ValueExpressionList*
ValueExpressionList::
create(const Statement::ValueExpressionList* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
