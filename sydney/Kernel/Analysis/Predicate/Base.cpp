// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Base.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Predicate/Base.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Selection.h"
#include "DPlan/Relation/Selection.h"

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

// FUNCTION public
//	Predicate::Base::getFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Base::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Interface::IPredicate* pPredicate =
		pStatement_->getAnalyzer2()->getPredicate(cEnvironment_,
												  pRelation_,
												  pStatement_);
	return Plan::Relation::Selection::create(cEnvironment_,
											 pPredicate,
											 pRelation_);
}



// FUNCTION public
//	Predicate::Base::getDistributeFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual

Plan::Interface::IRelation*
Base::
getDistributeFilter(Opt::Environment& cEnvironment_,
					Plan::Interface::IRelation* pRelation_,
					Statement::Object* pStatement_) const
{
	Plan::Interface::IPredicate* pPredicate =
		pStatement_->getAnalyzer2()->getPredicate(cEnvironment_,
												  pRelation_,
												  pStatement_);
	return DPlan::Relation::Selection::create(cEnvironment_,
											 pPredicate,
											 pRelation_);
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
