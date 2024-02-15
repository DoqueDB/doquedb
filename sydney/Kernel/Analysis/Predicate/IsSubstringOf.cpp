// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsSubstringOf.cpp --
// 
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/IsSubstringOf.h"

#include "Exception/InvalidSubstringOf.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/IsSubstringOf.h"
#include "Plan/Interface/IRelation.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::IsSubstringOfImpl --
	//
	// NOTES
	class IsSubstringOfImpl
		: public Predicate::IsSubstringOf
	{
	public:
		typedef IsSubstringOfImpl This;
		typedef Predicate::IsSubstringOf Super;

		// constructor
		IsSubstringOfImpl() : Super() {}
		// destructor
		~IsSubstringOfImpl() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzer -- instances
	//
	// NOTES
	const Impl::IsSubstringOfImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::IsSubstringOfImpl

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::IsSubstringOfImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);

	STRING cName = pVE->toSQLStatement();
	Plan::Interface::IPredicate* pResult = cEnvironment_.searchPredicate(cName);
	if (pResult) {
		return pResult;
	}

	Statement::ValueExpression* pLeft = pVE->getLeft();
	Statement::ValueExpression* pRight = pVE->getRight();

	Plan::Interface::IScalar* pLeftOperand = 0;
	{
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.pushStatus(Opt::Environment::Status::NoArbitraryElement);
		pLeftOperand =
			pLeft->getAnalyzer2()->getScalar(cEnvironment_,
											 pRelation_,
											 pLeft);
		if (pLeftOperand->getDataType().isArrayType() == false) {
			// left operand should be array type
			_SYDNEY_THROW0(Exception::InvalidSubstringOf);
		}
	}
	Plan::Interface::IScalar* pRightOperand = 0;
	{
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.pushStatus(Opt::Environment::Status::NoArbitraryElement);
		pRightOperand =
			pRight->getAnalyzer2()->getScalar(cEnvironment_,
											  pRelation_,
											  pRight);
		if (pRightOperand->getDataType().isArrayType() == false) {
			// right operand should be array type
			_SYDNEY_THROW0(Exception::InvalidSubstringOf);
		}
	}

	pResult =
		Plan::Predicate::IsSubstringOf::create(cEnvironment_,
											   MAKEPAIR(pLeftOperand, pRightOperand));

	if (pResult->hasParameter() == false) {
		// add to namemap
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}
	return pResult;
}

//////////////////////////////
// Predicate::IsSubstringOf::

// FUNCTION public
//	Predicate::IsSubstringOf::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const IsSubstringOf*
//
// EXCEPTIONS

//static
const IsSubstringOf*
IsSubstringOf::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
