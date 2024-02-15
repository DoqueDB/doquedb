// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cost.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
#include "Plan/AccessPlan/Cost.h"
#include "Plan/Declaration.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "Exception/Unexpected.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/Math.h"
#include "Os/Limits.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_ACCESSPLAN_USING

namespace
{
	// max double value for comparison
	const double _MaxDouble = Os::Limits<double>::getMax();
}

///////////////////////////////////////
//	Plan::AccessPlan::Cost::Value

// FUNCTION public
//	AccessPlan::Cost::Value::log -- get log(value)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
log() const
{
	if (isInfinity()) return Cost::Value();
	return Cost::Value(Os::Math::log(m_dblValue + 1.0));
}

// FUNCTION public
//	AccessPlan::Cost::Value::getByInt -- get in int type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Cost::Value::
getByInt() const
{
	if (isInfinity() || m_dblValue > Os::Limits<int>::getMax())
		return Os::Limits<int>::getMax();
	return static_cast<int>(m_dblValue);
}

// FUNCTION public
//	AccessPlan::Cost::Value::get -- get value by double
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	double
//
// EXCEPTIONS

double
Cost::Value::
get() const
{
	if (isInfinity()) {
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return m_dblValue;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator=(const Value& cOther_)
{
	if (this != &cOther_) {
		m_bInfinity = cOther_.m_bInfinity;
		m_dblValue = cOther_.m_dblValue;
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator= -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator=(double dblValue_)
{
	m_bInfinity = false;
	m_dblValue = (dblValue_ > 0) ? dblValue_ : 0;
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator+= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator+=(const Value& cOther_)
{
	if (cOther_.isInfinity()) {
		m_bInfinity = true;
	} else {
		(*this) += cOther_.get();
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator-= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator-=(const Value& cOther_)
{
	if (cOther_.isInfinity()) {
		m_bInfinity = true;
	} else {
		(*this) -= cOther_.get();
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator*= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator*=(const Value& cOther_)
{
	if (cOther_.isInfinity()) {
		m_bInfinity = true;
	} else {
		(*this) *= cOther_.get();
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator/= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator/=(const Value& cOther_)
{
	if (cOther_.isInfinity()) {
		m_bInfinity = true;
	} else {
		(*this) /= cOther_.get();
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator+= -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator+=(double dblValue_)
{
	if (!isInfinity()) {
		if (_MaxDouble - dblValue_ <= m_dblValue) {
			m_bInfinity = true;
		} else {
			m_dblValue += dblValue_;
		}
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator-= -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator-=(double dblValue_)
{
	if (!isInfinity()) {
		if (m_dblValue < dblValue_) {
			m_dblValue = 0;
		} else {
			m_dblValue -= dblValue_;
		}
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator*= -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator*=(double dblValue_)
{
	if (!isInfinity()) {
		if ((dblValue_ > 1) && (_MaxDouble / dblValue_ <= m_dblValue)) {
			m_bInfinity = true;
		} else {
			m_dblValue *= dblValue_;
		}
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator/= -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value&
//
// EXCEPTIONS

Cost::Value&
Cost::Value::
operator/=(double dblValue_)
{
	if (!isInfinity() && m_dblValue != 0.0) {
		if ((dblValue_ < 1) && (_MaxDouble * dblValue_ <= m_dblValue)) {
			SydErrorMessage << "Cost value divided by zero" << ModEndl;
			m_bInfinity = true;
		} else {
			m_dblValue /= dblValue_;
		}
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator+ -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator+(const Value& cOther_) const
{
	Cost::Value cResult(*this);
	return cResult += cOther_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator- -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator-(const Value& cOther_) const
{
	Cost::Value cResult(*this);
	return cResult -= cOther_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator* -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator*(const Value& cOther_) const
{
	Cost::Value cResult(*this);
	return cResult *= cOther_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator/ -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator/(const Value& cOther_) const
{
	Cost::Value cResult(*this);
	return cResult /= cOther_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator+ -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator+(double dblValue_) const
{
	Cost::Value cResult(*this);
	return cResult += dblValue_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator- -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator-(double dblValue_) const
{
	Cost::Value cResult(*this);
	return cResult -= dblValue_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator* -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator*(double dblValue_) const
{
	Cost::Value cResult(*this);
	return cResult *= dblValue_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator/ -- 
//
// NOTES
//
// ARGUMENTS
//	double dblValue_
//	
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::Value::
operator/(double dblValue_) const
{
	Cost::Value cResult(*this);
	return cResult /= dblValue_;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator== -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::Value::
operator==(const Value& cOther_) const
{
	return (isInfinity() == cOther_.isInfinity())
		&& (isInfinity() || m_dblValue == cOther_.m_dblValue);
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator<= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::Value::
operator<=(const Value& cOther_) const
{
	return cOther_.isInfinity()
		|| (!isInfinity() && m_dblValue <= cOther_.m_dblValue);
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator>= -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::Value::
operator>=(const Value& cOther_) const
{
	return cOther_ <= *this;
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator< -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::Value::
operator<(const Value& cOther_) const
{
	return !(cOther_ <= *this);
}

// FUNCTION public
//	AccessPlan::Cost::Value::operator> -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::Value::
operator>(const Value& cOther_) const
{
	return !(*this <= cOther_);
}

#ifndef NO_TRACE
_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ACCESSPLAN_BEGIN
// FUNCTION public
//	AccessPlan::Cost::Value::operator<< -- 
//
// NOTES
//
// ARGUMENTS
//	OSTREAM& cStream_
//	const Value& cValue_
//	
// RETURN
//	OSTREAM&
//
// EXCEPTIONS

OSTREAM&
operator<<(OSTREAM& cStream_, const Cost::Value& cValue_)
{
	if (cValue_.isInfinity()) {
		cStream_ << "Inf.";
	} else {
		cStream_ << ModScientific << cValue_.m_dblValue;
	}
	return cStream_;
}
_SYDNEY_PLAN_ACCESSPLAN_END
_SYDNEY_PLAN_END
_SYDNEY_END
#endif

// FUNCTION public
//	AccessPlan::Cost::Value::explain -- get string for explain
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

void
Cost::Value::
explain(Opt::Explain& cExplain_) const
{
	if (isInfinity()) {
		cExplain_.put("Inf.");
	} else {
		cExplain_.put(m_dblValue);
	}
}

//////////////////////////////
//	Plan::AccessPlan::Cost

// FUNCTION public
//	AccessPlan::Cost::Cost -- constructor
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

Cost::
Cost()
	: m_cOverhead(),
	  m_cStartup(0),
	  m_cTotalCost(),
	  m_cTupleCount(),
	  m_cTupleSize(static_cast<double>(sizeof(Common::DataArrayData))),
	  m_cRetrieveCost(0),
	  m_cLimitCount(),
	  m_cRate(),
	  m_cTableCount(),
	  m_bFetch(false),
	  m_bSetRate(true),
	  m_bSetCount(true)
{}

// FUNCTION public
//	AccessPlan::Cost::Cost -- constructor
//
// NOTES
//
// ARGUMENTS
//	double dblOverhead_
//	double dblTotalCost_
//	double dblTupleCount_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Cost::
Cost(double dblOverhead_,
	 double dblTotalCost_,
	 double dblTupleCount_)
	: m_cOverhead(dblOverhead_),
	  m_cStartup(0),
	  m_cTotalCost(dblTotalCost_),
	  m_cTupleCount(dblTupleCount_),
	  m_cTupleSize(static_cast<double>(sizeof(Common::DataArrayData))),
	  m_cRetrieveCost(0),
	  m_cLimitCount(),
	  m_cRate(),
	  m_cTableCount(),
	  m_bFetch(false),
	  m_bSetRate(true),
	  m_bSetCount(true)
{}

// FUNCTION public
//	AccessPlan::Cost::Cost -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Cost::
Cost(const Cost& cCost_)
	: m_cOverhead(cCost_.m_cOverhead),
	  m_cStartup(cCost_.m_cStartup),
	  m_cTotalCost(cCost_.m_cTotalCost),
	  m_cTupleCount(cCost_.m_cTupleCount),
	  m_cTupleSize(cCost_.m_cTupleSize),
	  m_cRetrieveCost(cCost_.m_cRetrieveCost),
	  m_cLimitCount(cCost_.m_cLimitCount),
	  m_cRate(cCost_.m_cRate),
	  m_cTableCount(cCost_.m_cTableCount),
	  m_bFetch(cCost_.m_bFetch),
	  m_bSetRate(cCost_.m_bSetRate),
	  m_bSetCount(cCost_.m_bSetCount)
{}

// FUNCTION public
//	AccessPlan::Cost::operator= --
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	Cost&
//
// EXCEPTIONS

Cost&
Cost::
operator=(const Cost& cOther_)
{
	if (this != &cOther_) {
		m_cOverhead = cOther_.m_cOverhead;
		m_cStartup = cOther_.m_cStartup;
		m_cTotalCost = cOther_.m_cTotalCost;
		m_cTupleCount = cOther_.m_cTupleCount;
		m_cTupleSize = cOther_.m_cTupleSize;
		m_cRetrieveCost = cOther_.m_cRetrieveCost;
		m_cLimitCount = cOther_.m_cLimitCount;
		m_cRate = cOther_.m_cRate;
		m_cTableCount = cOther_.m_cTableCount;
		m_bFetch = cOther_.m_bFetch;
		// setXXX flag is not copied
		//m_bSetRate = cOther_.m_bSetRate;
		//m_bSetCount = cOther_.m_bSetCount;
	}
	return *this;
}

// FUNCTION public
//	AccessPlan::Cost::operator== -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::
operator==(const Cost& cOther_) const
{
	return calculateValue() == cOther_.calculateValue();
}

// FUNCTION public
//	AccessPlan::Cost::operator<= -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::
operator<=(const Cost& cOther_) const
{
	return calculateValue() <= cOther_.calculateValue();
}

// FUNCTION public
//	AccessPlan::Cost::operator>= -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::
operator>=(const Cost& cOther_) const
{
	return calculateValue() >= cOther_.calculateValue();
}

// FUNCTION public
//	AccessPlan::Cost::operator< -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::
operator<(const Cost& cOther_) const
{
	return calculateValue() < cOther_.calculateValue();
}

// FUNCTION public
//	AccessPlan::Cost::operator> -- 
//
// NOTES
//
// ARGUMENTS
//	const Cost& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Cost::
operator>(const Cost& cOther_) const
{
	return calculateValue() > cOther_.calculateValue();
}

// FUNCTION public
//	AccessPlan::Cost::getResultCount -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Cost::Value&
//
// EXCEPTIONS

const Cost::Value&
Cost::
getResultCount() const
{
	return m_cLimitCount < m_cTupleCount ? m_cLimitCount : m_cTupleCount;
}

// FUNCTION public
//	AccessPlan::Cost::setLimitCount -- 
//
// NOTES
//
// ARGUMENTS
//	const Value& cValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Cost::
setLimitCount(const Value& cValue_)
{
	if (m_cRate.isInfinity() || m_cRate == 0) {
		m_cLimitCount = cValue_;
	} else {
		m_cLimitCount = cValue_ / m_cRate;
	}
}

// FUNCTION public
//	AccessPlan::Cost::calculateValue -- calculate total cost
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Value
//
// EXCEPTIONS

Cost::Value
Cost::
calculateValue() const
{
	return getOverhead() + getRepeatCost();
}

// FUNCTION public
//	AccessPlan::Cost::getRepeatCost -- calculate total cost repeating in join
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::
getRepeatCost() const
{
	return (m_bSetCount && getResultCount() > 0)
		? getStartup() + (getProcessCost() + getRetrieveCost()) * getResultCount()
		: getStartup() + (getProcessCost() + getRetrieveCost());
}

// FUNCTION public
//	AccessPlan::Cost::getProcessCost -- calculate process cost for one tuple
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Cost::Value
//
// EXCEPTIONS

Cost::Value
Cost::
getProcessCost() const
{
	return (m_bSetCount && m_cTupleCount > 0) ? m_cTotalCost / m_cTupleCount : m_cTotalCost;
}

// FUNCTION public
//	AccessPlan::Cost::addSortingCost -- add sorting penalty
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
Cost::
addSortingCost()
{
	if (m_cTupleCount > Value(1)) {
		double dblMemorySpeed = LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory);
		double dblFileSpeed = LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);

		Value cTupleCount = MIN(m_cTupleCount, m_cLimitCount);
		m_cStartup += getProcessCost() * cTupleCount;
		if (m_cRetrieveCost > Value(0)) {
			m_cStartup += m_cRetrieveCost * cTupleCount;
		}
		Value cTotalSize = cTupleCount * m_cTupleSize;
		if (cTotalSize > Value(Opt::Configuration::getCollectionThreshold())) {
			// use filespeed instead of memoryspeed because too much memory consumed
			dblMemorySpeed = dblFileSpeed;
		}
		m_cStartup += (m_cTupleSize / dblMemorySpeed) * cTupleCount * cTupleCount.log();
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "addSortingCost:\n"
				   << "start up(" << m_cStartup << ") = "
				   << getProcessCost()
				   << " * "
				   << cTupleCount
				   << " + "
				   << (m_cTupleSize / dblMemorySpeed) * cTupleCount * cTupleCount.log();
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif
		// process cost become memory cost
		m_cTotalCost *= dblFileSpeed / dblMemorySpeed;
		m_cRetrieveCost *= dblFileSpeed / dblMemorySpeed;
	}
}

// FUNCTION public
//	AccessPlan::Cost::addDistinctCost -- add distinct penalty
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
Cost::
addDistinctCost()
{
	// limit count become larger
	m_cLimitCount *= 100;
}

#ifndef NO_TRACE
_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ACCESSPLAN_BEGIN
// FUNCTION public
//	AccessPlan::Cost::operator<< -- 
//
// NOTES
//
// ARGUMENTS
//	OSTREAM& cStream_
//	const Cost& cCost_
//	
// RETURN
//	OSTREAM&
//
// EXCEPTIONS

OSTREAM&
operator<<(OSTREAM& cStream_, const Cost& cCost_)
{
	cStream_ << "Cost(" << cCost_.calculateValue() << ")"
			 << "[overhead=" << cCost_.getOverhead()
			 << " cost=" << cCost_.getProcessCost()
			 << " count=" << cCost_.getTupleCount()
			 << " size=" << cCost_.getTupleSize()
			 << " limit=" << cCost_.getLimitCount()
			 << " startup=" << cCost_.getStartup()
			 << " retrieve=" << cCost_.getRetrieveCost()
			 << " rate=" << cCost_.getRate()
			 << " tablecount=" << cCost_.getTableCount()
			 << " fetch=" << (cCost_.isFetch()?"true":"false")
			 << " setrate=" << (cCost_.isSetRate()?"true":"false")
			 << " setcount=" << (cCost_.isSetCount()?"true":"false")
			 << "]";
	return cStream_;
}
_SYDNEY_PLAN_ACCESSPLAN_END
_SYDNEY_PLAN_END
_SYDNEY_END
#endif

// FUNCTION public
//	AccessPlan::Cost::explain -- get string for explain
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

void
Cost::
explain(Opt::Explain& cExplain_) const
{
	if (!isInfinity()) {
		cExplain_.pushNoNewLine();
		calculateValue().explain(cExplain_.put("[cost:"));
		getOverhead().explain(cExplain_.put(" =("));
		getProcessCost().explain(cExplain_.put("+"));
		getTupleCount().explain(cExplain_.put("x"));
		cExplain_.put(")");
		if (getLimitCount().isInfinity() == false) {
			getLimitCount().explain(cExplain_.put(" limit="));
		}
		if (isFetch()) {
			cExplain_.put(" (fetch)");
		}
		cExplain_.put("]");
		cExplain_.popNoNewLine();
	}
}

//
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
