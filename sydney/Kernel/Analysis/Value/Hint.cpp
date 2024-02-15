// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/Hint.cpp --
// 
// Copyright (c) 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Value/Hint.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"

#include "Statement/Hint.h"
#include "Statement/HintElement.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace Impl
{
	// CLASS local
	//	Value::Impl::HintImpl -- hint analyzer
	//
	// NOTES
	class HintImpl
		: public Value::Hint
	{
	public:
		typedef HintImpl This;
		typedef Value::Hint Super;

		// constructor
		HintImpl() : Super() {}
		// destructor
		~HintImpl() {}

	///////////////////////////
	// Interface::IAnalyzer::
		virtual Plan::Interface::IScalar*
						getScalar(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation* pRelation_,
								  Statement::Object* pStatement_) const;

		virtual Plan::Relation::RowElement*
						getRowElement(Opt::Environment& cEnvironment_,
									  Plan::Interface::IRelation* pRelation_,
									  Statement::Object* pStatement_) const;
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
	const Impl::HintImpl _analyzer;

} // namespace

///////////////////////////////////////////
//	Value::Impl::HintImpl

// FUNCTION public
//	Value::Impl::HintImpl::getScalar -- 
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
Impl::HintImpl::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Hint* pHint =
		_SYDNEY_DYNAMIC_CAST(Statement::Hint*, pStatement_);
	; _SYDNEY_ASSERT(pHint);

	VECTOR<Plan::Interface::IScalar*> vecOperand;
	int n = pHint->getHintElementCount();
	for (int i = 0; i < n; ++i) {
		Statement::HintElement* pElement = pHint->getHintElementAt(i);
		int nPrimary = pElement->getHintPrimaryCount();
		for (int iPrimary = 0; iPrimary < nPrimary; ++iPrimary) {
			Statement::Object* pPrimary = pElement->getHintPrimaryAt(iPrimary);
			pPrimary->getAnalyzer2()->addScalar(cEnvironment_, pRelation_, pPrimary, vecOperand);
		}
	}
	return Plan::Scalar::Function::create(cEnvironment_,
										  Plan::Tree::Node::Hint,
										  vecOperand,
										  pHint->toSQLStatement());
}

// FUNCTION public
//	Value::Impl::HintImpl::getRowElement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
Plan::Relation::RowElement*
Impl::HintImpl::
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

//////////////////////////////
// Value::Hint
//////////////////////////////

// FUNCTION public
//	Value::Hint::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::Hint* pStatement_
//	
// RETURN
//	const Hint*
//
// EXCEPTIONS

//static
const Hint*
Hint::
create(const Statement::Hint* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
