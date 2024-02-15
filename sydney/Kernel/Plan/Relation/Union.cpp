// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Union.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "Plan/Relation/Union.h"
#include "Plan/Relation/Impl/UnionImpl.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

////////////////////////////////////
// Relation::Union::DistinctByKey::

// FUNCTION public
//	Relation::Union::DistinctByKey::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IRelation*>& vecOperand_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//static
Interface::IRelation*
Union::DistinctByKey::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IRelation*>& vecOperand_)
{
	switch (vecOperand_.GETSIZE()) {
	case 0:
		{
			; _SYDNEY_ASSERT(false);
			return 0;
		}
	case 1:
		{
			return vecOperand_[0];
		}
	default:
		{
			AUTOPOINTER<This> pResult =
				new UnionImpl::Nadic(UnionImpl::Type::DistinctByKey,
									 vecOperand_);
			pResult->registerToEnvironment(cEnvironment_);
			return pResult.release();
		}
	}
}

////////////////////////////////////
// Relation::Union::Distinct::

// FUNCTION public
//	Relation::Union::Distinct::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IRelation*>& vecOperand_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//static
Interface::IRelation*
Union::Distinct::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IRelation*>& vecOperand_)
{
	switch (vecOperand_.GETSIZE()) {
	case 0:
		{
			; _SYDNEY_ASSERT(false);
			return 0;
		}
	case 1:
		{
			return vecOperand_[0];
		}
	default:
		{
			AUTOPOINTER<This> pResult =
				new UnionImpl::Nadic(UnionImpl::Type::Distinct,
									 vecOperand_);
			pResult->registerToEnvironment(cEnvironment_);
			return pResult.release();
		}
	}
}

////////////////////////////////////
// Relation::Union::All::

// FUNCTION public
//	Relation::Union::All::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IRelation*>& vecOperand_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//static
Interface::IRelation*
Union::All::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IRelation*>& vecOperand_)
{
	switch (vecOperand_.GETSIZE()) {
	case 0:
		{
			; _SYDNEY_ASSERT(false);
			return 0;
		}
	case 1:
		{
			return vecOperand_[0];
		}
	default:
		{
			AUTOPOINTER<This> pResult =
				new UnionImpl::Nadic(UnionImpl::Type::All,
									 vecOperand_);
			pResult->registerToEnvironment(cEnvironment_);
			return pResult.release();
		}
	}
}

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
