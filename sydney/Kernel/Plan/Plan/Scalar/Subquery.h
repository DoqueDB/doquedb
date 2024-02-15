// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Subquery.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_SUBQUERY_H
#define __SYDNEY_PLAN_SCALAR_SUBQUERY_H

#include "Plan/Scalar/Base.h"

#include "Plan/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Subquery -- Interface for constant value
//
//	NOTES
class Subquery
	: public Base
{
public:
	typedef Base Super;
	typedef Subquery This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pSubRelation_,
						const Utility::RelationSet& cOuterRelation_,
						int iPosition_);

	// destructor
	virtual ~Subquery() {}

protected:
	// constructor
	Subquery()
		: Super(Node::RowSubquery)
	{}
private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_SUBQUERY_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
