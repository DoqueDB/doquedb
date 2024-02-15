// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Base.cpp --
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

#include "Analysis/Function/Base.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Relation/RowElement.h"

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

// FUNCTION public
//	Function::Base::getScalar -- generate Scalar from Statement::Object
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
Base::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	return getRowElement(cEnvironment_, pRelation_, pStatement_)->getScalar();
}

// FUNCTION public
//	Function::Base::getRowElement -- generate RowElement from Statement::Object
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
Base::
getRowElement(Opt::Environment& cEnvironment_,
			  Plan::Interface::IRelation* pRelation_,
			  Statement::Object* pStatement_) const
{
	const STRING& cstrName = pStatement_->toSQLStatement();
	Plan::Relation::RowElement* pResult = 0;

	// search from namemap
	pResult = cEnvironment_.searchScalar(cstrName);

	if (pResult == 0) {
		pResult = Plan::Relation::RowElement::create(cEnvironment_,
													 0, /* no relation */
													 0, /* no position */
													 createScalar(cEnvironment_, pRelation_, pStatement_));
		// add to namemap
		cEnvironment_.addScalar(pRelation_,
								cstrName,
								pResult);
	}
	return pResult;
}

// FUNCTION public
//	Function::Base::addScalar -- generate Plan::Tree::Node from Statement::Object
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
Base::
addScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_,
		  VECTOR<Plan::Interface::IScalar*>& vecScalar_) const
{
	vecScalar_.PUSHBACK(getScalar(cEnvironment_, pRelation_, pStatement_));
}

// FUNCTION public
//	Function::Base::getDegree -- 
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
Base::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
