// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/ChosenSpecification.cpp --
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
const char moduleName[] = "Plan::Order";
}

#include "SyDefault.h"

#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Order/Key.h"

#include "Plan/Candidate/File.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Table.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

#include "Common/Assert.h"

#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

//////////////////////////
// Order::ChosenSpecification

// FUNCTION public
//	Order::ChosenSpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::File* pFile_
//	Interface::IPredicate* pPredicate_
//	Specification* pChosenSpecification_
//	Specification* pOriginalSpecification_
//	
// RETURN
//	ChosenSpecification*
//
// EXCEPTIONS

ChosenSpecification*
ChosenSpecification::
create(Opt::Environment& cEnvironment_,
	   Candidate::File* pFile_,
	   Interface::IPredicate* pPredicate_,
	   Specification* pChosenSpecification_,
	   Specification* pOriginalSpecification_)
{
	AUTOPOINTER<This> pResult =
		new ChosenSpecification(pFile_,
								pPredicate_,
								pChosenSpecification_,
								pOriginalSpecification_);
	
	if (pChosenSpecification_
		&& pChosenSpecification_->isBitSetSort())
		pResult->setBitSetSort();
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Order::ChosenSpecification::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ChosenSpecification::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	int n = getKeySize();
	for (int i = 0; i < n; ++i) {
		vecID.PUSHBACK(getKey(i)->getScalar()->generate(cEnvironment_,
														cProgram_,
														pIterator_,
														cArgument_));
	}
	return cProgram_.addVariable(vecID);
}

// FUNCTION private
//	Order::ChosenSpecification::explainOperand -- 
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
ChosenSpecification::
explainOperand(Opt::Environment* pEnvironment_,
			   Opt::Explain& cExplain_)
{
	int n = getKeySize();
	for (int i = 0; i < n; ++i) {
		if (i > 0) {
			cExplain_.put(",");
		}
		getKey(i)->explain(pEnvironment_,
						   cExplain_);
	}
}

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
