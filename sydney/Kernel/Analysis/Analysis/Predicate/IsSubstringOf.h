// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsSubstringOf.h --
// 
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ANALYSIS_PREDICATE_ISSUBSTRINGOF_H
#define __SYDNEY_ANALYSIS_PREDICATE_ISSUBSTRINGOF_H

#include "Analysis/Predicate/Base.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////
// CLASS
//	Predicate::IsSubstringOf -- value expression analyzer for isSubstringOf predicate
//
// NOTES
class IsSubstringOf
	: public Base
{
public:
	typedef IsSubstringOf This;
	typedef Base Super;

	// constuctor
	static const This* create(const Statement::ValueExpression* pStatement_);
	// destructor
	virtual ~IsSubstringOf() {}

protected:
	// constructor
	IsSubstringOf() {}
private:
};

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_PREDICATE_ISSUBSTRINGOF_H

//
//	Copyright (c) 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
