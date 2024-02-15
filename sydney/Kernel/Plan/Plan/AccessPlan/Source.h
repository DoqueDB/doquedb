// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessPlan/Source.h --
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

#ifndef __SYDNEY_PLAN_ACCESSPLAN_SOURCE_H
#define __SYDNEY_PLAN_ACCESSPLAN_SOURCE_H

#include "Plan/AccessPlan/Module.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Limit.h"

#include "Plan/Declaration.h"

#include "Common/Object.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ACCESSPLAN_BEGIN

///////////////////////////////////////////////////////////
//	CLASS
//	Plan::AccessPlan::Source -- Source information for determining access plan
//
//	NOTES
class Source
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef Source This;

	// constructor
	Source()
		: Super(),
		  m_vecPrecedingCandidate(),
		  m_pPredicate(0),
		  m_pOrder(0),
		  m_cLimit(),
		  m_cEstimateLimit(),
		  m_iJoinMaxCandidates(-1),
		  m_bSimple(false),
		  m_bExists(false),
		  m_bTop(true),					// default constructor is used only for TOP
		  m_bCheckPartial(false),
		  m_bGrouping(false)
	{}
	Source(const VECTOR<Interface::ICandidate*>& vecPrecedingCandidate_,
		   Interface::IPredicate* pPredicate_,
		   Order::Specification* pOrder_)
		: Super(),
		  m_vecPrecedingCandidate(vecPrecedingCandidate_),
		  m_pPredicate(pPredicate_),
		  m_pOrder(pOrder_),
		  m_cLimit(),
		  m_cEstimateLimit(),
		  m_iJoinMaxCandidates(-1),
		  m_bSimple(false),
		  m_bExists(false),
		  m_bTop(false),
		  m_bCheckPartial(false),
		  m_bGrouping(false)
	{}
	Source(const Source& cSource_,
		   Interface::ICandidate* pPrecedingCandidate_)
		: Super(),
		  m_vecPrecedingCandidate(1, pPrecedingCandidate_),
		  m_pPredicate(cSource_.m_pPredicate),
		  m_pOrder(cSource_.m_pOrder),
		  m_cLimit(cSource_.m_cLimit),
		  m_cEstimateLimit(cSource_.m_cEstimateLimit),
		  m_iJoinMaxCandidates(cSource_.m_iJoinMaxCandidates),
		  m_bSimple(false),
		  m_bExists(cSource_.m_bExists),
		  m_bTop(false),
		  m_bCheckPartial(cSource_.m_bCheckPartial),
		  m_bGrouping(cSource_.m_bGrouping)
	{}
	Source(const Source& cSource_,
		   Interface::IPredicate* pPredicate_)
		: Super(),
		  m_vecPrecedingCandidate(cSource_.m_vecPrecedingCandidate),
		  m_pPredicate(pPredicate_),
		  m_pOrder(cSource_.m_pOrder),
		  m_cLimit(cSource_.m_cLimit),
		  m_cEstimateLimit(cSource_.m_cEstimateLimit),
		  m_iJoinMaxCandidates(cSource_.m_iJoinMaxCandidates),
		  m_bSimple(false),
		  m_bExists(cSource_.m_bExists),
		  m_bTop(false),
		  m_bCheckPartial(cSource_.m_bCheckPartial),
		  m_bGrouping(cSource_.m_bGrouping)
	{}
	Source(const Source& cSource_,
		   Order::Specification* pOrder_)
		: Super(),
		  m_vecPrecedingCandidate(cSource_.m_vecPrecedingCandidate),
		  m_pPredicate(cSource_.m_pPredicate),
		  m_pOrder(pOrder_),
		  m_cLimit(cSource_.m_cLimit),
		  m_cEstimateLimit(cSource_.m_cEstimateLimit),
		  m_iJoinMaxCandidates(cSource_.m_iJoinMaxCandidates),
		  m_bSimple(cSource_.m_bSimple),
		  m_bExists(cSource_.m_bExists),
		  m_bTop(false),
		  m_bCheckPartial(isCheckPartial(pOrder_, cSource_.m_cLimit)),
		  m_bGrouping(cSource_.m_bGrouping)
	{}
	Source(const Source& cSource_,
		   const Limit& cLimit_)
		: Super(),
		  m_vecPrecedingCandidate(cSource_.m_vecPrecedingCandidate),
		  m_pPredicate(cSource_.m_pPredicate),
		  m_pOrder(cSource_.m_pOrder),
		  m_cLimit(cLimit_),
		  m_cEstimateLimit(),
		  m_iJoinMaxCandidates(cSource_.m_iJoinMaxCandidates),
		  m_bSimple(cSource_.m_bSimple),
		  m_bExists(cSource_.m_bExists),
		  m_bTop(cSource_.m_bTop),
		  m_bCheckPartial(isCheckPartial(cSource_.m_pOrder, cLimit_)),
		  m_bGrouping(cSource_.m_bGrouping)
	{}

	// destructor
	~Source() {}

	// accessor
	const VECTOR<Interface::ICandidate*>& getPrecedingCandidate() const {return m_vecPrecedingCandidate;}
	Interface::IPredicate* getPredicate() const {return m_pPredicate;}
	Order::Specification* getOrder() const {return m_pOrder;}
	const Limit& getLimit() const {return m_cLimit;}
	const Cost::Value& getEstimateLimit() const {return m_cEstimateLimit;}
	bool isSimple() const {return m_bSimple;}
	bool isExists() const {return m_bExists;}
	bool isTop() const {return m_bTop;}
	bool isGrouping() const {return m_bGrouping;}

	bool isCheckPartial() const {return m_bCheckPartial;}
	bool isCheckPartial(Order::Specification* pOrder_,
						const Limit& cLimit_) const;

	int getJoinMaxCandidates() const {return m_iJoinMaxCandidates;}
	bool checkJoinMaxCandidates();

	void addPrecedingCandidate(Interface::ICandidate* pCandidate_);
	void addPredicate(Opt::Environment& cEnvironment_,
					  Interface::IPredicate* pPredicate_);
	void setPredicate(Interface::IPredicate* pPredicate_) {m_pPredicate = pPredicate_;}

	void estimateLimit(Opt::Environment& cEnvironment_);
	void setEstimateLimit(const Cost::Value& cValue_) {m_cEstimateLimit = cValue_;}
	void setSimple() {m_bSimple = true;}
	void setExists() {m_bExists = true;}
	void setGrouping() {m_bGrouping = true;}

	void erasePredicate() {m_pPredicate = 0; m_bTop = false;}
	void eraseOrder() {m_pOrder = 0; m_bTop = false;}
	void eraseLimit() {m_cLimit = Limit(); m_bTop = false;}
	void eraseEstimateLimit() {m_cEstimateLimit = Cost::Value();}
	void eraseExists() {m_bExists = false;}

	void setIntermediateLimit() {m_cLimit.setIntermediate();}

protected:
private:
	VECTOR<Interface::ICandidate*> m_vecPrecedingCandidate;// class representing preceding candidate
	Interface::IPredicate* m_pPredicate;		// required predicate
	Order::Specification* m_pOrder;				// required sorting order
	Limit m_cLimit;								// limit specification
	Cost::Value m_cEstimateLimit;				// limit estimated value
	bool m_bSimple;								// true if target relation is Simple
	bool m_bExists;								// true if target relation is under exists subquery
	bool m_bTop;								// true if target relation is TOP
	bool m_bCheckPartial;						// true if partial sort should be checked
	int m_iJoinMaxCandidates;					// allowable max candidates in considering join order
	bool m_bGrouping;							// true if target relation is Faset
};

_SYDNEY_PLAN_ACCESSPLAN_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ACCESSPLAN_SOURCE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
