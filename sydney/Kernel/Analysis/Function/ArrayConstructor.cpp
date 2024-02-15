// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/ArrayConstructor.cpp --
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

#include "Analysis/Function/ArrayConstructor.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::ArrayConstructorImpl -- valueExpression analyzer for array constructor
	//
	// NOTES
	class ArrayConstructorImpl
		: public Function::ArrayConstructor
	{
	public:
		typedef ArrayConstructorImpl This;
		typedef Function::ArrayConstructor Super;

		// constructor
		ArrayConstructorImpl() : Super() {}
		// destructor
		~ArrayConstructorImpl() {}

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
	const Impl::ArrayConstructorImpl _analyzer;

} // namespace

///////////////////////////////////////////////
//	Function::Impl::ArrayConstructorImpl

// FUNCTION private
//	Function::Impl::ArrayConstructorImpl::createScalar -- 
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
Impl::ArrayConstructorImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Arrayconst);

	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());

	VECTOR<Plan::Interface::IScalar*> vecScalar;

	if (pVEL) {
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

		pVEL->getAnalyzer2()->addScalar(cEnvironment_,
										pRelation_,
										pVEL,
										vecScalar);
	}

	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::ArrayConstructor,
										  vecScalar,
										  pVE->toSQLStatement());
}

//////////////////////////////
// Function::ArrayConstructor
//////////////////////////////

// FUNCTION public
//	Function::ArrayConstructor::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const ArrayConstructor*
//
// EXCEPTIONS

//static
const ArrayConstructor*
ArrayConstructor::
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
