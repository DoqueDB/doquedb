// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/PlaceHolder.cpp --
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
const char moduleName[] = "Analysis::Value";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Value/PlaceHolder.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Value.h"

#include "Statement/IntegerValue.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Impl::NormalPlaceHolder -- placeHolder analyzer
	//
	// NOTES
	class PlaceHolderImpl
		: public Value::PlaceHolder
	{
	public:
		typedef PlaceHolderImpl This;
		typedef Value::PlaceHolder Super;

		// constructor
		PlaceHolderImpl() : Super() {}
		// destructor
		~PlaceHolderImpl() {}

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
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::PlaceHolderImpl _analyzer;

} // namespace

///////////////////////////
//	Value::Impl::PlaceHolderImpl

// FUNCTION public
//	Value::Impl::PlaceHolderImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::PlaceHolderImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Placeholder);

	Statement::IntegerValue* pIV =
		_SYDNEY_DYNAMIC_CAST(Statement::IntegerValue*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pIV);

	return Plan::Scalar::Value::PlaceHolder::create(cEnvironment_,
													pIV->getValue());
}

// FUNCTION public
//	Value::Impl::PlaceHolderImpl::getRowElement -- generate RowElement from Statement::Object
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
Impl::PlaceHolderImpl::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	return Plan::Relation::RowElement::create(cEnvironment_,
											  0, /* no relation */
											  0, /* no position */
											  getScalar(cEnvironment_,
														pRelation_,
														pStatement_));
}

// FUNCTION public
//	Value::Impl::PlaceHolderImpl::getDegree -- get element numbers added by following methods
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
Impl::PlaceHolderImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

// FUNCTION public
//	Value::Impl::PlaceHolderImpl::addScalar -- add Scalar from Statement::Object
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
Impl::PlaceHolderImpl::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	vecScalar_.PUSHBACK(getScalar(cEnvironment_, pRelation_, pStatement_));
}

//////////////////////////////
// Value::PlaceHolder
//////////////////////////////

// FUNCTION public
//	Value::PlaceHolder::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const PlaceHolder*
//
// EXCEPTIONS

//static
const PlaceHolder*
PlaceHolder::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Placeholder:
		{
			return &_analyzer;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
