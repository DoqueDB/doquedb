// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/ValueList.cpp --
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/ValueList.h"
#include "Plan/Relation/Impl/ValueListImpl.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

////////////////////////////////////
// Relation::ValueList::

// FUNCTION public
//	Relation::ValueList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR< VECTOR<Interface::IScalar*> >& vecvecValue_
//	
// RETURN
//	ValueList*
//
// EXCEPTIONS

//static
ValueList*
ValueList::
create(Opt::Environment& cEnvironment_,
	   const VECTOR< VECTOR<Interface::IScalar*> >& vecvecValue_)
{
	AUTOPOINTER<This> pResult;
	if (vecvecValue_.GETSIZE()) {
		switch (vecvecValue_[0].GETSIZE()) {
		case 1:
			{
				VECTOR<Interface::IScalar*> vecScalar;
				VECTOR< VECTOR<Interface::IScalar*> >::ConstIterator iterator = vecvecValue_.begin();
				const VECTOR< VECTOR<Interface::IScalar*> >::ConstIterator last = vecvecValue_.end();
				for (; iterator != last; ++iterator) {
					vecScalar.PUSHBACK((*iterator).getFront());
				}
				pResult = new ValueListImpl::SingleColumn(vecScalar);
				break;
			}
		default:
			{
				pResult = new ValueListImpl::MultiColumn(vecvecValue_);
				break;
			}
		}
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();
	}
	return 0;
}

// FUNCTION protected
//	Relation::ValueList::ValueList -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::ValueList* pValueList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ValueList::
ValueList()
	: Super(Node::Input)
{}

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
