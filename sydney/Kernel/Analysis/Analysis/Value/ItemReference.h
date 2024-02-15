// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Value/ItemReference.h --
// 
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ANALYSIS_VALUE_ITEMREFERENCE_H
#define __SYDNEY_ANALYSIS_VALUE_ITEMREFERENCE_H

#include "Analysis/Value/ValueExpression.h"

_SYDNEY_BEGIN

namespace Statement
{
	class ItemReference;
}

_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

////////////////////////////////////////////////////////////////////////////
// CLASS
//	Value::ItemReference -- value expression analyzer for item reference
//
// NOTES
class ItemReference
	: public ValueExpression
{
public:
	typedef ItemReference This;
	typedef ValueExpression Super;

	// constructor
	static const This* create(const Statement::ItemReference* pStatement_);
	static const This* create(const Statement::ValueExpression* pStatement_);
	// destructor
	virtual ~ItemReference() {}

protected:
	// constructor
	ItemReference() : Super() {}
private:
};

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

#endif // __SYDNEY_ANALYSIS_VALUE_ITEMREFERENCE_H

//
//	Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
