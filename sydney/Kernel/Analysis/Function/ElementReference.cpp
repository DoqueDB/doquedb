// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/ElementReference.cpp --
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
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/ElementReference.h"

#include "Common/Assert.h"

#include "Exception/ArbitraryElementNotAllowed.h"
#include "Exception/InvalidElementReference.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Value.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::ElementReferenceImpl -- valueExpression analyzer for element reference
	//
	// NOTES
	class ElementReferenceImpl
		: public Function::ElementReference
	{
	public:
		typedef ElementReferenceImpl This;
		typedef Function::ElementReference Super;

		// constructor
		ElementReferenceImpl() : Super() {}
		// destructor
		~ElementReferenceImpl() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	////////////////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::ElementReferenceImpl _analyzer;

} // namespace

///////////////////////////////////////////////
//	Function::Impl::ElementReferenceImpl

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::getScalar -- generate Scalar from Statement::Object
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
Impl::ElementReferenceImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Arrayref);

	Statement::ValueExpression* pLeft = pVE->getLeft();
	Statement::ValueExpression* pRight = pVE->getRight();



	Plan::Interface::IScalar* pOperand = 0;
	{
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.eraseStatus(Opt::Environment::Status::KnownNotNull
									  | Opt::Environment::Status::KnownNull);

		pOperand = pLeft->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft);
	}

	if (pOperand->getDataType().isArrayType() == false) {
		// left operand should be array type
		_SYDNEY_THROW0(Exception::InvalidElementReference);
	}

	Plan::Interface::IScalar* pOption = 0;
	if (pRight) {
		pOption = pRight->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pRight);

		if (pOption->getDataType().isExactNumericType() == false) {
			// right operand should be exact numeric type 
			_SYDNEY_THROW0(Exception::InvalidElementReference);
		}
	} else {
		if (cEnvironment_.checkStatus(Opt::Environment::Status::ArbitraryElementAllowed)) {
			// arbitrary element is not allowed
			pOption = Plan::Scalar::Value::create(cEnvironment_, STRING("all"))
				->setNodeType(Plan::Tree::Node::All);
		} else if(cEnvironment_.checkStatus(Opt::Environment::Status::ExpandElementAllowed)){			
			pOption = Plan::Scalar::Value::create(cEnvironment_, STRING("expand"))
				->setNodeType(Plan::Tree::Node::Expand);
			cEnvironment_.removeGroupingColumn(pOperand);			
		} else {
			_SYDNEY_THROW0(Exception::ArbitraryElementNotAllowed);
		}

		if (pOperand->getType() == Plan::Tree::Node::Field) {
			// column reference with arbitrary element use operand directly
			return pOperand->addOption(cEnvironment_, pOption);
		} else {
			_SYDNEY_THROW0(Exception::ArbitraryElementNotAllowed);
		}
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::ElementReference,
										  pOperand,
										  pOption,
										  pVE->toSQLStatement());
}

// FUNCTION private
//	Function::Impl::ElementReferenceImpl::createScalar -- 
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
Impl::ElementReferenceImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	return getScalar(cEnvironment_,
					 pRelation_,
					 pStatement_);
}

//////////////////////////////
// Function::ElementReference
//////////////////////////////

// FUNCTION public
//	Function::ElementReference::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const ElementReference*
//
// EXCEPTIONS

//static
const ElementReference*
ElementReference::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
