// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Nadic.cpp --
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

#include "Analysis/Function/Nadic.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Plan/Scalar/Choice.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Tree/Node.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::NadicImpl -- base class for nadic function analyzer
	//
	// NOTES
	class NadicImpl
		: public Function::Nadic
	{
	public:
		typedef NadicImpl This;
		typedef Function::Nadic Super;

		// constructor
		NadicImpl() : Super() {}
		// destructor
		~NadicImpl() {}

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
	_getNodeType(int iFunc_)
	{
		switch (iFunc_) {
		case Statement::ValueExpression::func_Coalesce:
			{
				return Plan::Tree::Node::Coalesce;
			}
		case Statement::ValueExpression::func_GetMax:
			{
				return Plan::Tree::Node::GetMax;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// VARIABLE local
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::NadicImpl _analyzer;

} // namespace

//////////////////////////////////////
//	Function::Impl::NadicImpl

// FUNCTION private
//	Function::Impl::NadicImpl::createScalar -- 
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
Impl::NadicImpl::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);
	Statement::ValueExpressionList* pVEL =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpressionList*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pVEL);

	VECTOR<Plan::Interface::IScalar*> vecOperand;
	pVEL->getAnalyzer2()->addScalar(cEnvironment_,
									pRelation_,
									pVEL,
									vecOperand);

	switch (pVE->getFunction()) {
	case Statement::ValueExpression::func_GetMax:
		{
			return Plan::Scalar::Choice::create(cEnvironment_,
												_getNodeType(pVE->getFunction()),
												vecOperand,
												pVE->toSQLStatement());
		}
	default:
		{
			return Plan::Scalar::Function::create(cEnvironment_,
												  _getNodeType(pVE->getFunction()),
												  vecOperand,
												  pVE->toSQLStatement());
		}
	}
}

//////////////////////////////
// Function::Nadic
//////////////////////////////

// FUNCTION public
//	Function::Nadic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Nadic*
//
// EXCEPTIONS

//static
const Nadic*
Nadic::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
