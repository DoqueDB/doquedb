// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IRow.h --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_INTERFACE_IROW_H
#define __SYDNEY_PLAN_INTERFACE_IROW_H

#include "Plan/Interface/Module.h"

#include "Plan/Declaration.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Node.h"
#include "Plan/Interface/ISqlNode.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Interface::IRow -- class for row information
//
//	NOTES
class IRow
	: public Tree::Nadic<Tree::Node, IScalar>, public ISqlNode
{
public:
	typedef Tree::Nadic<Tree::Node, IScalar> Super;
	typedef IRow This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Super::Argument cArgument_);

	// destructor
	~IRow() {}

	// explain
	void explain(Opt::Environment* pEnvironment_,
				 Opt::Explain& cExplain_);

	// check used tables
	void getUsedTable(Utility::RelationSet& cResult_);
	// check used scalars
	void getUsedField(Utility::FieldSet& cResult_);
	// check unknown keys
	void getUnknownKey(Opt::Environment& cEnvironment_,
					   Predicate::CheckUnknownArgument& cResult_);

	// has unassigned parameters?
	bool hasParameter();

	// set refered scalars as required
	void require(Opt::Environment& cEnvironment_,
				 Interface::ICandidate* pCandidate_);
	// set refered scalars as retrieved
	void retrieve(Opt::Environment& cEnvironment_);
	void retrieve(Opt::Environment& cEnvironment_,
				  Interface::ICandidate* pCandidate_);
	// set refered scalars as used
	void use(Opt::Environment& cEnvironment_,
			 Interface::ICandidate* pCandidate_);
	// set refered scalars as delayable
	bool delay(Opt::Environment& cEnvironment_,
			   Interface::ICandidate* pCandidate_,
			   Scalar::DelayArgument& cArgument_);

	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const;
protected:
private:
	// costructor
	IRow(Super::Argument cArgument_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		: Super(cArgument_)
	{
		setType(Super::List);
	}
#else
	: Super(Super::List, cArgument_)
	{}
#endif
};

_SYDNEY_PLAN_INTERFACE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_INTERFACE_IROW_H

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
