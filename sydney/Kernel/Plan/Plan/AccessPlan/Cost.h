// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cost.h -- holding cost estimation information
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ACCESSPLAN_COST_H
#define __SYDNEY_PLAN_ACCESSPLAN_COST_H

#include "Plan/AccessPlan/Module.h"

#include "Common/Object.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ACCESSPLAN_BEGIN

/////////////////////////////////////////////
// CLASS
//	Plan::AccessPlan::Cost -- class representing estimated cost
//
// NOTES
class Cost
	: public Common::Object
{
public:
	///////////////////////////
	// CLASS
	//	Plan::AccessPlan::Cost::Value -- class representing cost value
	//
	// NOTES
	class Value
		: public Common::Object
	{
	public:
		// constructor
		Value()
			: m_bInfinity(true),
			  m_dblValue(0)
		{}
		Value(double dblValue_)
			: m_bInfinity(false),
			  m_dblValue(dblValue_ > 0 ? dblValue_ : 0)
		{}
		Value(const Value& cOther_)
			: m_bInfinity(cOther_.m_bInfinity),
			  m_dblValue(cOther_.m_dblValue)
		{}

		// destructor
		~Value() {}

		// infinity check
		bool isInfinity() const {return m_bInfinity;}

		// get log(value)
		Value log() const;
		// get in int type
		int getByInt() const;
		// get in double type
		double get() const;

		// operators
		Value& operator=(const Value& cOther_);
		Value& operator=(double dblValue_);

		Value& operator+=(const Value& cOther_);
		Value& operator-=(const Value& cOther_);
		Value& operator*=(const Value& cOther_);
		Value& operator/=(const Value& cOther_);

		Value& operator+=(double dblValue_);
		Value& operator-=(double dblValue_);
		Value& operator*=(double dblValue_);
		Value& operator/=(double dblValue_);

		Value operator+(const Value& cOther_) const;
		Value operator-(const Value& cOther_) const;
		Value operator*(const Value& cOther_) const;
		Value operator/(const Value& cOther_) const;

		Value operator+(double dblValue_) const;
		Value operator-(double dblValue_) const;
		Value operator*(double dblValue_) const;
		Value operator/(double dblValue_) const;

		bool operator==(const Value& cOther_) const;
		bool operator<=(const Value& cOther_) const;
		bool operator>=(const Value& cOther_) const;
		bool operator<(const Value& cOther_) const;
		bool operator>(const Value& cOther_) const;

#ifndef NO_TRACE
		friend OSTREAM& operator<<(OSTREAM& cStream_, const Value& cValue_);
#endif
		// get string for explain
		void explain(Opt::Explain& cExplain_) const;

	protected:
	private:
		bool m_bInfinity;
		double m_dblValue;
	};
	// Cost::Value
	/////////////////////////////////////

	// constructor
	Cost();
	Cost(double dblOverhead_,
		 double dblTotalCost_,
		 double dblTupleCount_);
	Cost(const Cost& cCost_);
	// destructor
	~Cost() {}

	// infinity check
	bool isInfinity() const {return calculateValue().isInfinity();}

	// operators
	Cost& operator=(const Cost& cOther_);
	bool operator==(const Cost& cOther_) const;
	bool operator<=(const Cost& cOther_) const;
	bool operator>=(const Cost& cOther_) const;
	bool operator<(const Cost& cOther_) const;
	bool operator>(const Cost& cOther_) const;

	// accessors
	const Value& getOverhead() const {return m_cOverhead;}
	const Value& getStartup() const {return m_cStartup;}
	const Value& getTotalCost() const {return m_cTotalCost;}
	const Value& getTupleCount() const {return m_cTupleCount;}
	const Value& getTupleSize() const {return m_cTupleSize;}
	const Value& getRetrieveCost() const {return m_cRetrieveCost;}
	const Value& getLimitCount() const {return m_cLimitCount;}
	const Value& getRate() const {return m_cRate;}
	const Value& getTableCount() const {return m_cTableCount;}
	bool isFetch() const {return m_bFetch;}
	bool isSetRate() const {return m_bSetRate;}
	bool isSetCount() const {return m_bSetCount;}

	const Value& getResultCount() const;

	void setOverhead(const Value& cValue_) {m_cOverhead = cValue_;}
	void setStartup(const Value& cValue_) {m_cStartup = cValue_;}
	void setTotalCost(const Value& cValue_) {m_cTotalCost = cValue_;}
	void setTupleCount(const Value& cValue_) {m_cTupleCount = cValue_;}
	void setTupleSize(const Value& cValue_) {m_cTupleSize = cValue_;}
	void setRetrieveCost(const Value& cValue_) {m_cRetrieveCost = cValue_;}
	void setLimitCount(const Value& cValue_);
	void setRate(const Value& cValue_) {m_cRate = cValue_;}
	void setTableCount(const Value& cValue_) {m_cTableCount = cValue_;}
	void setIsFetch(bool b_ = true) {m_bFetch = b_;}
	void setIsSetRate(bool b_ = true) {m_bSetRate = b_;}
	void setIsSetCount(bool b_ = true) {m_bSetCount = b_;}

	// calculate total cost
	Value calculateValue() const;
	// calculate total cost repeating in join
	Value getRepeatCost() const;
	// calculate process cost for one tuple
	Value getProcessCost() const;

	// reset by zero
	void reset()
	{
		m_cTotalCost = m_cOverhead = m_cStartup = 0;
		m_bFetch = false;
		m_bSetRate = true;
		m_bSetCount = true;
	}

	// add sorting penalty
	void addSortingCost();
	// add distinct penalty
	void addDistinctCost();

#ifndef NO_TRACE
	friend OSTREAM& operator<<(OSTREAM& cStream_, const Cost& cCost_);
#endif
	// get string for explain
	void explain(Opt::Explain& cExplain_) const;

protected:
private:
	Value m_cOverhead;
	Value m_cStartup;
	Value m_cTotalCost;
	Value m_cTupleCount;
	Value m_cTupleSize;
	Value m_cRetrieveCost;
	Value m_cLimitCount;
	Value m_cRate;
	Value m_cTableCount;
	bool m_bFetch;
	bool m_bSetRate;
	bool m_bSetCount;
};

_SYDNEY_PLAN_ACCESSPLAN_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ACCESSPLAN_COST_H

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
