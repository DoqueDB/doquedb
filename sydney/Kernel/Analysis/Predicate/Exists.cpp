// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Exists.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Predicate/Exists.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Exists.h"
#include "Plan/Utility/ObjectSet.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::ExistsImpl --
	//
	// NOTES
	class ExistsImpl
		: public Predicate::Exists
	{
	public:
		typedef ExistsImpl This;
		typedef Predicate::Exists Super;

		// constructor
		ExistsImpl() : Super() {}
		// destructor
		virtual ~ExistsImpl() {}

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
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::ExistsImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::ExistsImpl

// FUNCTION public
//	Predicate::Impl::ExistsImpl::getPredicate -- 
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

Plan::Interface::IPredicate*
Impl::ExistsImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	Statement::Object* pSubQuery = pVE->getPrimary();

	Plan::Interface::IRelation* pSubRelation = 0;
	Plan::Utility::RelationSet cOuterRelation;

	{
		// create new namescope
		Opt::Environment::AutoPop cAutoPop0 =
			cEnvironment_.pushNameScope(Opt::Environment::Scope::Exists);
		Opt::Environment::AutoPop cAutoPop1 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Exists
									 | Opt::Environment::Status::Subquery
									 | Opt::Environment::Status::Reset);

		// convert subquery into relation node
		pSubRelation = pSubQuery->getAnalyzer2()->getRelation(cEnvironment_,
															  pSubQuery);
		cOuterRelation = cEnvironment_.getOuterRelation();
	}

	// create exists node
	return Plan::Predicate::Exists::create(cEnvironment_,
										   pSubRelation,
										   cOuterRelation,
										   cEnvironment_.checkStatus(
											 Opt::Environment::Status::Exists));
}

//////////////////////////////
// Predicate::Exists::

// FUNCTION public
//	Predicate::Exists::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Exists*
//
// EXCEPTIONS

//static
const Exists*
Exists::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
