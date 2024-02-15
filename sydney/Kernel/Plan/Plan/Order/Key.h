// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/Key.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_KEY_H
#define __SYDNEY_PLAN_ORDER_KEY_H

#include "Plan/Order/Module.h"

#include "Plan/Declaration.h"
#ifdef USE_OLDER_VERSION
#include "Plan/TypeDef.h"
#endif
#include "Plan/Tree/Node.h"

#include "Plan/Interface/ISqlNode.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

////////////////////////////////////
//	ENUM
//	Plan::Order::Direction::Value -- constant value which represents ordering direction
//
//	NOTES
struct Direction
{
	enum Value
	{
		Ascending = 0,
		Descending,
		Unknown,
		ValueNum
	};
};

////////////////////////////////////
//	CLASS
//	Plan::Order::Key -- Base class for the classes which represents order key information
//
//	NOTES
//		This class is not constructed directly
class Key
	: public Tree::Node , public Interface::ISqlNode
{
public:
	typedef Tree::Node Super;
	typedef Key This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IScalar* pScalar_,
						Direction::Value eDirection_ = Direction::Unknown);

	// destructor
	static void erase(Opt::Environment& cEnvironment_,
					  This* pKey_);
	virtual ~Key() {}

	// is key compatible to another key?
	static int compare(Key* pKey1_, Key* pKey2_);

	// explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_) = 0;

	// require sorting key
	virtual void require(Opt::Environment& cEnvironment_,
						 Interface::ICandidate* pCandidate_) = 0;

	// search for applicable index file
	virtual Key* check(Opt::Environment& cEnvironment_,
					   const CheckArgument& cArgument_) = 0;

	// is index is cheked
	virtual bool isChecked();

	// get checked interface
	virtual CheckedKey* getChecked();

	// accessor
	virtual Interface::IScalar* getScalar() = 0;
	virtual Direction::Value getDirection() = 0;

	virtual bool isFunction() = 0;

protected:
	// constructor
	Key() : Super(Super::SortKey) {}

private:
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_KEY_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
