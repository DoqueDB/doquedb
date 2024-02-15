// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Array.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Function/Array.h"

#include "Common/Assert.h"

#include "Exception/InvalidCardinality.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Scalar/Function.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace ArrayImpl
{
	// CLASS local
	//	Function::ArrayImpl::Cardinality -- implementation class for array function analyzer
	//
	// NOTES
	class Cardinality
		: public Function::Array
	{
	public:
		typedef Cardinality This;
		typedef Function::Array Super;

		// constructor
		Cardinality() : Super() {}
		// destructor
		~Cardinality() {}

	protected:
	private:
	////////////////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace ArrayImpl

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const ArrayImpl::Cardinality _analyzer;

} // namespace

//////////////////////////////////////
//	Function::ArrayImpl::Cardinality

// FUNCTION private
//	Function::ArrayImpl::Cardinality::createScalar -- 
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
ArrayImpl::Cardinality::
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

	if (pOperand->getDataType().isArrayType() == false
		&& pOperand->getDataType().isNoType() == false) {
		// left operand should be array type
		_SYDNEY_THROW0(Exception::InvalidCardinality);
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Cardinality,
										  pOperand,
										  pVE->toSQLStatement());
}

//////////////////////////////
// Function::Array
//////////////////////////////

// FUNCTION public
//	Function::Array::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Array*
//
// EXCEPTIONS

//static
const Array*
Array::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Func:
		{
			switch (pStatement_->getFunction()) {
			case Statement::ValueExpression::func_Cardinality:
				{
					return &_analyzer;
				}
			default:
				{
					break;
				}
			}
			break;
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
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
