// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Invoke.cpp --
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

#include "Analysis/Function/Invoke.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Scalar/Invoke.h"

#include "Statement/Identifier.h"
#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::InvokeImpl -- implementation class for invoke expression analyzer
	//
	// NOTES
	class InvokeImpl
		: public Function::Invoke
	{
	public:
		typedef InvokeImpl This;
		typedef Function::Invoke Super;

		// constructor
		InvokeImpl() : Super() {}
		// destructor
		~InvokeImpl() {}

	protected:
	private:
	//////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;
	};
} // namespace Impl

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instances
	//
	// NOTES
	const Impl::InvokeImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::InvokeImpl

// FUNCTION private
//	Function::Impl::InvokeImpl::createScalar -- 
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
Impl::InvokeImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::Identifier* pName =
		_SYDNEY_DYNAMIC_CAST(Statement::Identifier*, pVE->getPrimary());
	Statement::ValueExpressionList* pArgs = pVE->getOperandList();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	VECTOR<Plan::Interface::IScalar*> vecOperand;
	pArgs->getAnalyzer2()->addScalar(cEnvironment_,
									 pRelation_,
									 pArgs,
									 vecOperand);

	return Plan::Scalar::Invoke::create(cEnvironment_,
										*pName->getIdentifier(),
										vecOperand,
										pVE->toSQLStatement());
}

//////////////////////////////
// Function::Invoke
//////////////////////////////

// FUNCTION public
//	Function::Invoke::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Invoke*
//
// EXCEPTIONS

//static
const Invoke*
Invoke::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
