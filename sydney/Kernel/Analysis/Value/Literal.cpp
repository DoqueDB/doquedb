// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/Literal.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Value/Literal.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Value.h"

#include "Statement/Literal.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	$$$::NormalLiteral -- literal analyzer for normal literal
	//
	// NOTES
	class NormalLiteral
		: public Value::Literal
	{
	public:
		typedef NormalLiteral This;
		typedef Value::Literal Super;

		// constructor
		NormalLiteral() : Super() {}
		// destructor
		~NormalLiteral() {}

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

	// CLASS local
	//	$$$::SpecialLiteral -- literal analyzer for special data
	//
	// NOTES
	//	Special data means null or default
	class SpecialLiteral
		: public NormalLiteral
	{
	public:
		typedef SpecialLiteral This;
		typedef NormalLiteral Super;

		// constructor
		SpecialLiteral() : Super() {}
		// destructor
		~SpecialLiteral() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};

	// CLASS local
	//	$$$::LiteralObject -- literal analyzer for literal object
	//
	// NOTES
	class LiteralObject
		: public NormalLiteral
	{
	public:
		typedef LiteralObject This;
		typedef NormalLiteral Super;

		// constructor
		LiteralObject() : Super() {}
		// destructor
		~LiteralObject() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const Impl::NormalLiteral _analyzerNormal;
	const Impl::SpecialLiteral _analyzerSpecial;
	const Impl::LiteralObject _analyzerLiteral;

} // namespace

///////////////////////////
//	Value::Impl::NormalLiteral

// FUNCTION public
//	Value::Impl::NormalLiteral::getScalar -- generate Scalar from Statement::Object
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
Impl::NormalLiteral::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT((pVE->getOperator() == Statement::ValueExpression::op_Literal)
					 || (pVE->getOperator() == Statement::ValueExpression::op_PathName));

	Statement::Literal* pLIT =
		_SYDNEY_DYNAMIC_CAST(Statement::Literal*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pLIT);

	return Plan::Scalar::Value::create(cEnvironment_, pLIT->createData(), pLIT->toSQLStatement());
}

// FUNCTION public
//	Value::Impl::NormalLiteral::getRowElement -- generate RowElement from Statement::Object
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
Impl::NormalLiteral::
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
//	Value::Impl::NormalLiteral::getDegree -- get element numbers added by following methods
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
Impl::NormalLiteral::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

// FUNCTION public
//	Value::Impl::NormalLiteral::addScalar -- add Scalar from Statement::Object
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
Impl::NormalLiteral::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	vecScalar_.PUSHBACK(getScalar(cEnvironment_, pRelation_, pStatement_));
}

///////////////////////////
//	Value::Impl::SpecialLiteral

// FUNCTION public
//	Value::Impl::SpecialLiteral::getScalar -- generate Scalar from Statement::Object
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
Impl::SpecialLiteral::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	switch (pVE->getOperator()) {
	case Statement::ValueExpression::op_Nullobj:
		{
			return Plan::Scalar::Value::Null::create(cEnvironment_);
		}
	case Statement::ValueExpression::op_Defaultobj:
		{
			return Plan::Scalar::Value::Default::create(cEnvironment_);
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////
//	Value::Impl::LiteralObject

// FUNCTION public
//	Value::Impl::LiteralObject::getScalar -- generate Scalar from Statement::Object
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
Impl::LiteralObject::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Literal* pLIT =
		_SYDNEY_DYNAMIC_CAST(Statement::Literal*, pStatement_);
	; _SYDNEY_ASSERT(pLIT);

	return Plan::Scalar::Value::create(cEnvironment_, pLIT->createData(), pLIT->toSQLStatement());
}

//////////////////////////////
// Value::Literal
//////////////////////////////

// FUNCTION public
//	Value::Literal::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Literal*
//
// EXCEPTIONS

//static
const Literal*
Literal::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Literal:
	case Statement::ValueExpression::op_PathName:
		{
			return &_analyzerNormal;
		}
	case Statement::ValueExpression::op_Nullobj:
	case Statement::ValueExpression::op_Defaultobj:
		{
			return &_analyzerSpecial;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Value::Literal::create -- 
//
// NOTES
//
// ARGUMENTS
//	const Statement::Literal* pStatement_
//	
// RETURN
//	const Literal*
//
// EXCEPTIONS

//static
const Literal*
Literal::
create(const Statement::Literal* pStatement_)
{
	return &_analyzerLiteral;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
