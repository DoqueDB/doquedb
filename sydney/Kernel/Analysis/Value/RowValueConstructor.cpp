// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/RowValueConstructor.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Value/RowValueConstructor.h"

#include "Exception/NotSupported.h"

#include "Statement/ValueExpressionList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Value::Impl::RowValueConstructorImpl -- rowValueConstructor analyzer
	//
	// NOTES
	class RowValueConstructorImpl
		: public Value::RowValueConstructor
	{
	public:
		typedef RowValueConstructorImpl This;
		typedef Value::RowValueConstructor Super;

		// constructor
		RowValueConstructorImpl() : Super() {}
		// destructor
		~RowValueConstructorImpl() {}

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
	const Impl::RowValueConstructorImpl _analyzer;

} // namespace

///////////////////////////////////////////
//	Value::Impl::RowValueConstructorImpl

// FUNCTION public
//	Value::Impl::RowValueConstructorImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::RowValueConstructorImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pVEL);

	if (pVEL->getCount() != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(0);
	return pElement->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pElement);
}

// FUNCTION public
//	Value::Impl::RowValueConstructorImpl::getRowElement -- generate RowElement from Statement::Object
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
Impl::RowValueConstructorImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pVEL);

	if (pVEL->getCount() != 1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(0);
	return pElement->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pElement);
}

// FUNCTION public
//	Value::Impl::RowValueConstructorImpl::getDegree -- 
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
Impl::RowValueConstructorImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pVEL);

	return pVEL->getCount();
}

// FUNCTION public
//	Value::Impl::RowValueConstructorImpl::addScalar -- add Scalar from Statement::Object
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
Impl::RowValueConstructorImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	pVE->getPrimary()->getAnalyzer2()->addScalar(cEnvironment_,
												 pRelation_,
												 pVE->getPrimary(),
												 vecScalar_);
}

// FUNCTION public
//	Value::Impl::RowValueConstructorImpl::addColumns -- 
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
Impl::RowValueConstructorImpl::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	pVE->getPrimary()->getAnalyzer2()->addColumns(cEnvironment_,
												  pRowInfo_,
												  pRelation_,
												  pVE->getPrimary());
}

//////////////////////////////
// Value::RowValueConstructor
//////////////////////////////

// FUNCTION public
//	Value::RowValueConstructor::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const RowValueConstructor*
//
// EXCEPTIONS

//static
const RowValueConstructor*
RowValueConstructor::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
