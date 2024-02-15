// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/GeneratedSpecification.cpp --
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
const char moduleName[] = "Plan::Order";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Order/Argument.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Order/Key.h"

#include "Plan/Candidate/File.h"
#include "Plan/Candidate/Table.h"
#include "Plan/File/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Predicate/ChosenInterface.h"
#include "Plan/Relation/Table.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

#include "Common/Assert.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"
#include "Schema/File.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

/////////////////////////////////////
// Order::GeneratedSpecification

// FUNCTION public
//	Order::GeneratedSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Specification* pSpecification_
//	const VECTOR<int>& vecDataID_
//	const VECTOR<int>& vecPosition_
//	const VECTOR<int>& vecDirection_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	const VECTOR<int>& vecWordPosition_
//	
// RETURN
//	GeneratedSpecification*
//
// EXCEPTIONS

//static
GeneratedSpecification*
GeneratedSpecification::
create(Opt::Environment& cEnvironment_,
	   Specification* pSpecification_,
	   const VECTOR<int>& vecDataID_,
	   const VECTOR<int>& vecPosition_,
	   const VECTOR<int>& vecDirection_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   const VECTOR<int>& vecWordPosition_)
{
	AUTOPOINTER<This> pResult =
		new GeneratedSpecification(pSpecification_,
								   vecDataID_,
								   vecPosition_,
								   vecDirection_,
								   vecScalar_,
								   vecWordPosition_);
	
	if (pSpecification_
		&& pSpecification_->isBitSetSort())
		pResult->setBitSetSort();
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Order::GeneratedSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Specification* pSpecification_
//	const VECTOR<int>& vecDataID_
//	const VECTOR<int>& vecPosition_
//	const VECTOR<int>& vecDirection_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	
// RETURN
//	GeneratedSpecification*
//
// EXCEPTIONS

//static
GeneratedSpecification*
GeneratedSpecification::
create(Opt::Environment& cEnvironment_,
	   Specification* pSpecification_,
	   const VECTOR<int>& vecDataID_,
	   const VECTOR<int>& vecPosition_,
	   const VECTOR<int>& vecDirection_,
	   const VECTOR<Interface::IScalar*>& vecScalar_)
{
	VECTOR<int> vecWordPosition;
	AUTOPOINTER<This> pResult =
		new GeneratedSpecification(pSpecification_,
								   vecDataID_,
								   vecPosition_,
								   vecDirection_,
								   vecScalar_,
								   vecWordPosition);
	
	if (pSpecification_
		&& pSpecification_->isBitSetSort())
		pResult->setBitSetSort();
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION private
//	Order::GeneratedSpecification::explainOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GeneratedSpecification::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	; // never called
}

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
