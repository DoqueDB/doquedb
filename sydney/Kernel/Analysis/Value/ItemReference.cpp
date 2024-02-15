// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/ItemReference.cpp --
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

#include "Analysis/Value/ItemReference.h"

#include "Common/Assert.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/NonGroupingColumn.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"

#include "Statement/ItemReference.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	///////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ItemReferenceImpl -- implementation class of itemreference analyzer
	//
	// NOTES
	class ItemReferenceImpl
		: public Value::ItemReference
	{
	public:
		typedef ItemReferenceImpl This;
		typedef Value::ItemReference Super;

		// constructor
		ItemReferenceImpl() : Super() {}
		// destructor
		virtual ~ItemReferenceImpl() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;

		// generate RowElement from Statement::Object
		virtual Plan::Relation::RowElement*
					getRowElement(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;

		// get element numbers added by following methods
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;	

		// add Scalar from Statement::Object
		virtual void addScalar(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::Object* pStatement_,
							   VECTOR<Plan::Interface::IScalar*>& vecScalar_) const;
	protected:
	private:
	};

	///////////////////////////////////////////
	// CLASS
	//	Analysis::Value::Impl::ItemReferenceVEImpl -- implementation class of itemreference analyzer
	//
	// NOTES
	class ItemReferenceVEImpl
		: public ItemReferenceImpl
	{
	public:
		typedef ItemReferenceVEImpl This;
		typedef ItemReferenceImpl Super;

		// constructor
		ItemReferenceVEImpl() : Super() {}
		// destructor
		virtual ~ItemReferenceVEImpl() {}

	//	virtual Plan::Interface::IScalar*
	//				getScalar(Opt::Environment& cEnvironment_,
	//						  Plan::Interface::IRelation* pRelation_,
	//						  Statement::Object* pStatement_) const;

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
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const Impl::ItemReferenceImpl _analyzer;
	const Impl::ItemReferenceVEImpl _analyzerVE;
}

/////////////////////////////////////
//	Value::Impl::ItemReferenceImpl

// FUNCTION public
//	Value::Impl::ItemReferenceImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::ItemReferenceImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Relation::RowElement* pElement = getRowElement(cEnvironment_, pRelation_, pStatement_);
	Plan::Interface::IScalar* pResult = pElement->getScalar(cEnvironment_);
	if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
		if (cEnvironment_.checkStatus(Opt::Environment::Status::KnownNotNull)) {
			cEnvironment_.addKnownNotNull(pResult);
		}
		if (cEnvironment_.checkStatus(Opt::Environment::Status::KnownNull)) {
			cEnvironment_.addKnownNull(pResult);
		}
	}
	return pResult;
}

// FUNCTION public
//	Value::Impl::ItemReferenceImpl::getRowElement -- generate RowElement from Statement::Object
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
Impl::ItemReferenceImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	Statement::ItemReference* pIR = _SYDNEY_DYNAMIC_CAST(Statement::ItemReference*, pStatement_);
	; _SYDNEY_ASSERT(pIR);

	const ModUnicodeString* pQualifierName = pIR->getItemQualifierString();
	const ModUnicodeString* pItemName = pIR->getItemNameString();
	; _SYDNEY_ASSERT(pItemName);

	Plan::Relation::RowElement* pResult = 0;
	if (pQualifierName == 0) {
		// search for column spec from current scope to top
		pResult = cEnvironment_.searchScalar(*pItemName);
	} else {
		// get column spec with relation name
		pResult = cEnvironment_.searchScalar(*pQualifierName, *pItemName);
	}
	if (pResult == 0) {
		_SYDNEY_THROW1(Exception::ColumnNotFound, pIR->toSQLStatement());
	}
	if (cEnvironment_.isGrouping()
		&& cEnvironment_.checkStatus(Opt::Environment::Status::SetFunction) == false
		&& cEnvironment_.isGroupingColumn(pResult) == false) {
		// non-grouping column is used outside of set function
		_SYDNEY_THROW1(Exception::NonGroupingColumn, pIR->toSQLStatement());
	}
	return pResult;
}

// FUNCTION public
//	Value::Impl::ItemReferenceImpl::getDegree -- get element numbers added by following methods
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
Impl::ItemReferenceImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

// FUNCTION public
//	Value::Impl::ItemReferenceImpl::addScalar -- generate Plan::Tree::Node from Statement::Object
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
Impl::ItemReferenceImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	vecScalar_.PUSHBACK(getScalar(cEnvironment_, pRelation_, pStatement_));
}

/////////////////////////////////////
//	Value::Impl::ItemReferenceVEImpl

// FUNCTION public
//	Value::Impl::ItemReferenceVEImpl::getRowElement -- generate RowElement from Statement::Object
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
Impl::ItemReferenceVEImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Itemref);
	Statement::ItemReference* pIR = _SYDNEY_DYNAMIC_CAST(Statement::ItemReference*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pIR);

	return Super::getRowElement(cEnvironment_, pRelation_, pIR);
}

//////////////////////////////
// Value::ItemReference

// FUNCTION public
//	Value::ItemReference::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ItemReference* pStatement_
//	
// RETURN
//	const ItemReference*
//
// EXCEPTIONS

//static
const ItemReference*
ItemReference::
create(const Statement::ItemReference* pStatement_)
{
	return &_analyzer;
}

// FUNCTION public
//	Value::ItemReference::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const ItemReference*
//
// EXCEPTIONS

//static
const ItemReference*
ItemReference::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzerVE;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
