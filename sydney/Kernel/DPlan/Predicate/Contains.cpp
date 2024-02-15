// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Contains.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Predicate/Contains.h"
#include "DPlan/Predicate/Impl/ContainsImpl.h"


_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_PREDICATE_BEGIN

////////////////////////////////////
//	DPlan::Predicate::Contains

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
	   const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cOperand_,
	   const VECTOR<Plan::Interface::IScalar*>& vecOption_)
{
	AUTOPOINTER<This> pResult =
		new Impl::ContainsImpl(cOperand_, vecOption_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
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
	: Super()
{}


_SYDNEY_DPLAN_PREDICATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
