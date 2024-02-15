// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Limit.h -- holding limit estimation information
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ACCESSPLAN_LIMIT_H
#define __SYDNEY_PLAN_ACCESSPLAN_LIMIT_H

#include "Plan/AccessPlan/Module.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/Interface/ISqlNode.h"

#include "Common/Object.h"

#include "Execution/Declaration.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Common
{
	class IntegerArrayData;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ACCESSPLAN_BEGIN

////////////////////////////////////////////////////////////////////////
// CLASS
//	Plan::AccessPlan::Limit -- class representing limit specification
//
// NOTES
class Limit
	: public Common::Object, public Interface::ISqlNode
{
public:
	// constructor
	Limit()
		: m_pLimit(0),
		  m_pOffset(0),
		  m_bIntermediate(false)
	{}
	Limit(Interface::IScalar* pLimit_,
		  Interface::IScalar* pOffset_ = 0)
		: m_pLimit(pLimit_),
		  m_pOffset(pOffset_),
		  m_bIntermediate(false)
	{}
	Limit(const Limit& cLimit_)
		: m_pLimit(cLimit_.m_pLimit),
		  m_pOffset(cLimit_.m_pOffset),
		  m_bIntermediate(cLimit_.m_bIntermediate)
	{}
	// destructor
	~Limit() {}

	// has specification?
	bool isSpecified() const;

	// same specification?
	bool isEquivalent(const Limit& cLimit_) const;

	// is intermediate?
	bool isIntermediate() const;

	// set no-top (offset is added to limit in execution)
	void setIntermediate();

	// explain
	void explain(Opt::Environment* pEnvironment_,
				 Opt::Explain& cExplain_);

	// create SQL
	virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
								  const Plan::Sql::QueryArgument& cArgument_) const;

	virtual const AccessPlan::Limit* getDistributeSpec(Opt::Environment& cEnvironment_) const;

	// estimate limit count
	bool estimateCount(Opt::Environment& cEnvironment_,
					   Cost::Value& cValue_) const;
	// get limit specification for checking file
	bool getValue(Opt::Environment& cEnvironment_,
				  Common::IntegerArrayData& cValue_) const;

	// generate variable denoting limit specification
	PAIR<int, int> generate(Opt::Environment& cEnvironment_,
							Execution::Interface::IProgram& cProgram_,
							Execution::Interface::IIterator* pIterator_,
							Candidate::AdoptArgument& cArgument_);

protected:
private:
	Interface::IScalar* m_pLimit;
	Interface::IScalar* m_pOffset;

	bool m_bIntermediate;
};

_SYDNEY_PLAN_ACCESSPLAN_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ACCESSPLAN_LIMIT_H

//
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
