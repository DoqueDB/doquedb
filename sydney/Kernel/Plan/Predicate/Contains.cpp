// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Contains.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Contains.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Impl/ContainsImpl.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Sql/Argument.h"

#include "DPlan/Predicate/Contains.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Contains

// FUNCTION public
//	Predicate::Contains::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	
// RETURN
//	Contains*
//
// EXCEPTIONS

//static
Contains*
Contains::
create(Opt::Environment& cEnvironment_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_)
{
	
	if (cEnvironment_.hasCascade()) {
		return DPlan::Predicate::Contains::create(cEnvironment_,
												  cOperand_,
												  vecOption_);
	} else {
		AUTOPOINTER<This> pResult = new Impl::ContainsImpl(cOperand_, vecOption_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}

}


////////////////////////////////////
//	Plan::Predicate::Contains

// FUNCTION public
//	Predicate::Contains::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	
// RETURN
//	Contains*
//
// EXCEPTIONS

//static
Contains*
Contains::
create(Opt::Environment& cEnvironment_,
	   const PAIR< VECTOR<Interface::IScalar*>, Interface::IScalar* >& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_)
{
	
	Interface::IScalar* pOperand0;
	if (cOperand_.first.GETSIZE() == 1) {
		pOperand0 = cOperand_.first[0];
	} else {
		OSTRSTREAM cStream;
		cStream << "(";
		char c = ' ';
		Plan::Sql::QueryArgument cArgument;
		for (int i = 0; i < cOperand_.first.GETSIZE(); ++i, c = ',') 
			cStream << c << cOperand_.first[i]->toSQLStatement(cEnvironment_,
															   cArgument);
		
		cStream << ")";
		pOperand0 = Scalar::Function::create(cEnvironment_,
											 Tree::Node::List,
											 cOperand_.first,
											 cStream.getString());
	}

	return create(cEnvironment_, MAKEPAIR(pOperand0, cOperand_.second), vecOption_);

}
	

// FUNCTION public
//	Predicate::Contains::isNeedIndex -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Contains::
isNeedIndex()
{
	return true;
}

// FUNCTION protected
//	Predicate::Contains::Contains -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::
Contains()
	: Super(Tree::Node::Contains)
{}

// FUNCTION private
//	Predicate::Contains::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	addToEnvironment(cEnvironment_);
	Super::registerToEnvironment(cEnvironment_);
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
