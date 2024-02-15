// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Fetch.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Predicate/Fetch.h"
#include "Plan/Predicate/Impl/FetchImpl.h"
#include "Plan/Predicate/CheckedInterface.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_PREDICATE_USING

////////////////////////////////////
//	Plan::Predicate::Fetch

// FUNCTION public
//	Predicate::Fetch::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pOriginal_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	
// RETURN
//	Fetch*
//
// EXCEPTIONS

//static
Fetch*
Fetch::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pOriginal_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_)
{
	PAIR<Interface::IScalar*, Interface::IScalar*> cOperand(cOperand_);
	cOperand.second = cOperand_.second->createCast(cEnvironment_,
												   cOperand_.first->getDataType(),
												   true, /* for comparison */
												   Tree::Node::Fetch);
	AUTOPOINTER<This> pResult =
		new Impl::SingleFetch(pOriginal_, cOperand);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Fetch::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pOriginal_
//	const VECTOR<Interface::IScalar*>& vecKey_
//	const VECTOR<Interface::IScalar*>& vecValue_
//	
// RETURN
//	Fetch*
//
// EXCEPTIONS

//static
Fetch*
Fetch::
create(Opt::Environment& cEnvironment_,
	   Interface::IPredicate* pOriginal_,
	   const VECTOR<Interface::IScalar*>& vecKey_,
	   const VECTOR<Interface::IScalar*>& vecValue_)
{
	; _SYDNEY_ASSERT(vecKey_.GETSIZE() == vecValue_.GETSIZE());
	SIZE n = vecKey_.GETSIZE();

	if (n == 1) {
		return create(cEnvironment_,
					  pOriginal_,
					  MAKEPAIR(vecKey_[0], vecValue_[0]));
	}

	VECTOR<Interface::IScalar*> vecValue;
	vecValue.reserve(n);
	for (SIZE i = 0; i < n; ++i) {
		vecValue.PUSHBACK(vecValue_[i]->createCast(cEnvironment_,
												   vecKey_[i]->getDataType(),
												   true, /* for comparison */
												   Tree::Node::Fetch));
	}
	AUTOPOINTER<This> pResult =
		new Impl::MultipleFetch(pOriginal_, vecKey_, vecValue);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Fetch::merge -- merge multiple fetch
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pOriginal_
//	const VECTOR<Interface::IPredicate*>& vecChecked_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Fetch::
merge(Opt::Environment& cEnvironment_,
	  Interface::IPredicate* pOriginal_,
	  const VECTOR<Interface::IPredicate*>& vecChecked_)
{
	Interface::IPredicate* pResult = 0;
	if (vecChecked_.GETSIZE() > 1) {
		VECTOR<Interface::IScalar*> vecKey;
		VECTOR<Interface::IScalar*> vecValue;

		VECTOR<Interface::IPredicate*>::CONSTITERATOR iterator = vecChecked_.begin();
		const VECTOR<Interface::IPredicate*>::CONSTITERATOR last = vecChecked_.end();
		for (; iterator != last; ++iterator) {
			if ((*iterator)->getType() != Tree::Node::Fetch) {
				break;
			}
			Interface::IPredicate* pPredicate = (*iterator)->getChecked()->getPredicate();
			while (pPredicate->isChecked()) pPredicate = pPredicate->getChecked()->getPredicate();

			This* pFetch = _SYDNEY_DYNAMIC_CAST(This*, pPredicate);
			; _SYDNEY_ASSERT(pFetch);

			if (!pFetch->getKeyAndValue(vecKey, vecValue)) {
				break;
			}
		}

		if (iterator == last) {
			// all are processed
			pResult = create(cEnvironment_,
							 pOriginal_,
							 vecKey,
							 vecValue);
		}
	}
	return pResult;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
