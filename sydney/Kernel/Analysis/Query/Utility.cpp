// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/Utility.cpp --
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/Utility.h"

#include "Common/Assert.h"

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace
{
	// CONST local
	//	_orderingTable --
	//			conversion table from Statement::OrderingSpecification
	//			to Plan::Order::Direction::Value
	// NOTES
	const Plan::Order::Direction::Value _orderingTable[] =
	{
		Plan::Order::Direction::Ascending,
		Plan::Order::Direction::Descending,
	};

} // namespace

//////////////////////////////
// Analysis::Query::Utility
//////////////////////////////

// FUNCTION public
//	Query::Utility::getOrdering -- convert ordering constant
//
// NOTES
//
// ARGUMENTS
//	int iOrdering_
//	
// RETURN
//	Plan::Order::Direction::Value
//
// EXCEPTIONS

//static
Plan::Order::Direction::Value
Utility::
getOrdering(int iOrdering_)
{
	; _SYDNEY_ASSERT(iOrdering_ >= 0 && iOrdering_ < (sizeof(_orderingTable) / sizeof(_orderingTable[0])));

	return _orderingTable[iOrdering_];
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
