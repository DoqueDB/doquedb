// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Numeric.cpp --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/Numeric.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Scalar/Function.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::DyadicNumeric -- implementation class for numeric function analyzer
	//
	// NOTES
	class DyadicNumeric
		: public Function::Numeric
	{
	public:
		typedef DyadicNumeric This;
		typedef Function::Numeric Super;

		// constructor
		DyadicNumeric() : Super() {}
		// destructor
		~DyadicNumeric() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};

	// CLASS local
	//	Function::Impl::MonadicNumeric -- implementation class for numeric function analyzer
	//
	// NOTES
	class MonadicNumeric
		: public Function::Numeric
	{
	public:
		typedef MonadicNumeric This;
		typedef Function::Numeric Super;

		// constructor
		MonadicNumeric() : Super() {}
		// destructor
		~MonadicNumeric() {}

	protected:
	private:
	/////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace Impl

namespace
{
	// FUNCTION local
	//	$$$::_getNodeType -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	int iFunc_
	//	
	// RETURN
	//	Plan::Tree::Node::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getNodeType(int iOp_)
	{
		switch (iOp_) {
		case Statement::ValueExpression::op_Add:
			{
				return Plan::Tree::Node::Add;
			}
		case Statement::ValueExpression::op_Sub:
			{
				return Plan::Tree::Node::Subtract;
			}
		case Statement::ValueExpression::op_Mul:
			{
				return Plan::Tree::Node::Multiply;
			}
		case Statement::ValueExpression::op_Div:
			{
				return Plan::Tree::Node::Divide;
			}
		case Statement::ValueExpression::op_Neg:
			{
				return Plan::Tree::Node::Negative;
			}
		case Statement::ValueExpression::op_Abs:
			{
				return Plan::Tree::Node::Absolute;
			}
		case Statement::ValueExpression::op_Mod:
			{
				return Plan::Tree::Node::Modulus;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzerXXX -- instances
	//
	// NOTES
	const Impl::DyadicNumeric _analyzerdyadic;
	const Impl::MonadicNumeric _analyzermonadic;

} // namespace

//////////////////////////////////////
//	Function::Impl::DyadicNumeric

// FUNCTION private
//	Function::Impl::DyadicNumeric::createScalar -- 
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
Impl::DyadicNumeric::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpression* pLeft = pVE->getLeft();
	Statement::ValueExpression* pRight = pVE->getRight();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand0 =
		pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);
	Plan::Interface::IScalar* pOperand1 =
		pRight->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pRight);

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE->getOperator()),
										  MAKEPAIR(pOperand0, pOperand1),
										  pVE->toSQLStatement());
}

//////////////////////////////////////
//	Function::Impl::MonadicNumeric

// FUNCTION private
//	Function::Impl::MonadicNumeric::createScalar -- 
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
Impl::MonadicNumeric::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpression* pLeft = pVE->getLeft();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	Plan::Interface::IScalar* pOperand =
		pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);

	return Plan::Scalar::Function::create(cEnvironment_,
										  _getNodeType(pVE->getOperator()),
										  pOperand,
										  pVE->toSQLStatement());
}

//////////////////////////////
// Function::Numeric
//////////////////////////////

// FUNCTION public
//	Function::Numeric::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Numeric*
//
// EXCEPTIONS

//static
const Numeric*
Numeric::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Add:
	case Statement::ValueExpression::op_Sub:
	case Statement::ValueExpression::op_Mul:
	case Statement::ValueExpression::op_Div:
	case Statement::ValueExpression::op_Mod:
		{
			return &_analyzerdyadic;
		}
	case Statement::ValueExpression::op_Neg:
	case Statement::ValueExpression::op_Abs:
		{
			return &_analyzermonadic;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
