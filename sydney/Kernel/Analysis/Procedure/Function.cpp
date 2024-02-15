// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Procedure/Function.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Procedure";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Procedure/Function.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Procedure.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"

#include "Schema/Function.h"

#include "Statement/FunctionDefinition.h"
#include "Statement/Identifier.h"
#include "Statement/ParameterDeclarationList.h"
#include "Statement/ReturnsClause.h"
#include "Statement/RoutineBody.h"
#include "Statement/ValueExpression.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PROCEDURE_BEGIN

namespace Impl
{
	/////////////////////////////////////////////
	// CLASS
	//	Procedure::Impl::FunctionDefinitionImpl -- class for function definition analyzer
	//
	// NOTES
	class FunctionDefinitionImpl
		: public Procedure::Function
	{
	public:
		typedef FunctionDefinitionImpl This;
		typedef Procedure::Function Super;

		// constructor
		FunctionDefinitionImpl() {}
		// destructor
		virtual ~FunctionDefinitionImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzerXXX -- instance
	//
	// NOTES
	const Impl::FunctionDefinitionImpl _analyzerDefinition;

} //namespace

////////////////////////////////////////////////
// Procedure::Impl::FunctionDefinitionImpl
////////////////////////////////////////////////

// FUNCTION public
//	Procedure::Impl::FunctionDefinitionImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::FunctionDefinitionImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::FunctionDefinition* pFD =
		_SYDNEY_DYNAMIC_CAST(Statement::FunctionDefinition*, pStatement_);
	; _SYDNEY_ASSERT(pFD);

	Statement::ParameterDeclarationList* pParam = pFD->getParam();
	Statement::ReturnsClause* pReturns = pFD->getReturns();
	Statement::RoutineBody* pRoutine = pFD->getRoutine();

	VECTOR<Plan::Interface::IScalar*> vecParams;
	// get parameter variables
	pParam->getAnalyzer2()->addScalar(cEnvironment_,
									  0,
									  pParam,
									  vecParams);
	// get return value variables
	Plan::Interface::IScalar* pReturnValue =
		pReturns->getAnalyzer2()->getScalar(cEnvironment_,
											0,
											pReturns);

	// create relation
	Plan::Relation::Procedure* pRelation =
		Plan::Relation::Procedure::create(cEnvironment_,
										  vecParams,
										  pReturnValue);

	Statement::Identifier* pName = pFD->getFunctionName();
	; _SYDNEY_ASSERT(pName->getIdentifier());

	pRelation->setCorrelationName(cEnvironment_,
								  *(pName->getIdentifier()),
								  VECTOR<STRING>());

	// create routine body
	Plan::Interface::IScalar* pFunctionBody =
		pRoutine->getAnalyzer2()->getScalar(cEnvironment_,
											pRelation,
											pRoutine);
	pRelation->setRoutine(pFunctionBody);

	// add projection to denote no results as relation
	return Plan::Relation::Projection::create(cEnvironment_,
											  Plan::Relation::RowInfo::create(cEnvironment_),
											  pRelation);
}

// FUNCTION public
//	Procedure::Impl::FunctionDefinitionImpl::getDistributeRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::FunctionDefinitionImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_,
					   pStatement_);
}

// FUNCTION public
//	Procedure::Impl::FunctionDefinitionImpl::getDegree -- 
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
Impl::FunctionDefinitionImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

//////////////////////////////////
// Procedure::Function
//////////////////////////////////

// FUNCTION public
//	Procedure::Function::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::FunctionDefinition* pStatement_
//	
// RETURN
//	const Function*
//
// EXCEPTIONS

//static
const Function*
Function::
create(const Statement::FunctionDefinition* pStatement_)
{
	return &_analyzerDefinition;
}

_SYDNEY_ANALYSIS_PROCEDURE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
