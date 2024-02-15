// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tree/Option.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_TREE_OPTION_H
#define __SYDNEY_PLAN_TREE_OPTION_H

#include "Plan/Tree/Node.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_TREE_BEGIN

//	CLASS
//	Plan::Tree::Option -- special tree node for getSearchParameter with option
//
//	NOTES
class Option
	: public Node
{
public:
	typedef Node Super;
	typedef Option This;

	// constructor
	Option()
		: Super(Node::Undefined)
	{}

	// destructor
	~Option() {}
protected:
private:
};

_SYDNEY_PLAN_TREE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_TREE_OPTION_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
