// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessPlan/Limit.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::AccessPlan";
}

#include "SyDefault.h"
#include "SyInclude.h"
#include "Plan/AccessPlan/Limit.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/Value.h"

#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"

#include "Exception/NotSupported.h"

#include "Opt/Algorithm.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_ACCESSPLAN_USING

namespace
{
	const int _DefaultLimit = 1000;
}

// FUNCTION public
//	AccessPlan::Limit::isSpecified -- has specification?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Limit::
isSpecified() const
{
	return m_pLimit != 0;
}

// FUNCTION public
//	AccessPlan::Limit::isEquivalent -- same specification?
//
// NOTES
//
// ARGUMENTS
//	const Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Limit::
isEquivalent(const Limit& cLimit_) const
{
	return m_pLimit == cLimit_.m_pLimit
		&& m_pOffset == cLimit_.m_pOffset
		&& m_bIntermediate == cLimit_.m_bIntermediate;
}

// FUNCTION public
//	AccessPlan::Limit::isIntermediate -- is intermediate?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Limit::
isIntermediate() const
{
	return m_bIntermediate;
}

// FUNCTION public
//	AccessPlan::Limit::setIntermediate -- set no-top (offset is added to limit in execution)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Limit::
setIntermediate()
{
	m_bIntermediate = true;
}

// FUNCTION public
//	AccessPlan::Limit::explain -- explain
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// explain
void
Limit::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	if (m_pLimit) {
		cExplain_.pushNoNewLine();
		cExplain_.put("limit ");
		m_pLimit->explain(pEnvironment_, cExplain_);
		if (m_pOffset) {
			cExplain_.put(" offset ");
			m_pOffset->explain(pEnvironment_, cExplain_);
		}
		if (m_bIntermediate) {
			cExplain_.put(" (intermediate)");
		}
		cExplain_.popNoNewLine();
	}
}


// FUNCTION public
//	AccessPlan::Limit::createSQL
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING&
//
// EXCEPTIONS

// explain
STRING
Limit::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	if (m_pLimit) {
		cStream << " limit ";
		cStream << m_pLimit->toSQLStatement(cEnvironment_, cArgument_);
		if (m_pOffset) {
			cStream << " offset ";
			cStream << m_pOffset->toSQLStatement(cEnvironment_, cArgument_);
		}
	}
	return cStream.getString();
}


// FUNCTION public
//	AccessPlan::Limit::getDistributeSpec
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING&
//
// EXCEPTIONS

// explain
const AccessPlan::Limit*
Limit::
getDistributeSpec(Opt::Environment& cEnvironment_) const
{

	Interface::IScalar* pLimit;
	Interface::IScalar* pOffset;
	if (m_pLimit
		&& m_pOffset) {
		int iLimitValue = m_pLimit->preCalculate(cEnvironment_)->getInt();
		int iOffsetValue = m_pOffset->preCalculate(cEnvironment_)->getInt();
		if (iOffsetValue > 0) iOffsetValue--;
		Common::Data::Pointer pData = new Common::IntegerData(iLimitValue + iOffsetValue);
		
		pLimit = Plan::Scalar::Value::create(cEnvironment_, pData);
		pData = new Common::IntegerData(1);
		pOffset = Plan::Scalar::Value::create(cEnvironment_, pData);
		AUTOPOINTER<Limit> pResult = new Limit(pLimit, pOffset);
		cEnvironment_.addObject(pResult.get());
		return pResult.release();
	} else {
		return this;
	}
}



// FUNCTION public
//	AccessPlan::Limit::estimateCount -- estimate limit count
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Cost::Value& cValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Limit::
estimateCount(Opt::Environment& cEnvironment_,
			  Cost::Value& cValue_) const
{
	Common::Data::Pointer pLimit = m_pLimit->preCalculate(cEnvironment_);
	if (pLimit.get() && pLimit->isNull() == false) {
		if (m_pOffset) {
			Common::Data::Pointer pOffset = m_pOffset->preCalculate(cEnvironment_);
			if (pOffset.get() && pOffset->isNull() == false) {
				cValue_ = pLimit->getInt() + pOffset->getInt() - 1;
				return true;
			}
		} else {
			cValue_ = pLimit->getInt();
			return true;
		}
	}
	// set fake value and return false
	cValue_ = static_cast<double>(_DefaultLimit);
	return false;
}

// FUNCTION public
//	AccessPlan::Limit::getValue -- get limit specification for checking file
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::IntegerArrayData& cValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Limit::
getValue(Opt::Environment& cEnvironment_,
		 Common::IntegerArrayData& cValue_) const
{
	Common::Data::Pointer pLimit = m_pLimit->preCalculate(cEnvironment_);
	if (pLimit.get() && pLimit->isNull() == false) {
		if (pLimit->getInt() <= 0) {
			// invalid limit value
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		if (m_pOffset) {
			Common::Data::Pointer pOffset = m_pOffset->preCalculate(cEnvironment_);
			if (pOffset.get() && pOffset->isNull() == false) {
				if (pOffset->getInt() <= 0) {
					// invalid offset value
					_SYDNEY_THROW0(Exception::NotSupported);
				}
				if (m_bIntermediate) {
					cValue_.setElement(0, pLimit->getInt() + pOffset->getInt() - 1);
				} else {
					cValue_.setElement(0, pLimit->getInt());
					cValue_.setElement(1, pOffset->getInt());
				}
				return true;
			}
		} else {
			cValue_.setElement(0, pLimit->getInt());
			return true;
		}
	}
	cValue_.setElement(0, _DefaultLimit);
	return false;
}

// FUNCTION public
//	AccessPlan::Limit::generate -- generate variable denoting limit specification
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	PAIR<int, int>
//
// EXCEPTIONS

PAIR<int, int>
Limit::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	return MAKEPAIR(m_pLimit ? m_pLimit->generate(cEnvironment_,
												  cProgram_,
												  pIterator_,
												  cArgument_)
							 : -1,
					m_pOffset ? m_pOffset->generate(cEnvironment_,
													cProgram_,
													pIterator_,
													cArgument_)
							  : -1);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
