// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Argument.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_ARGUMENT_H
#define __SYDNEY_PLAN_PREDICATE_ARGUMENT_H

#include "Plan/Predicate/Module.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Cost.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Utility/ObjectSet.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

// STRUCT
//	Plan::Predicate::RewriteArgument -- argument for IPredicate::rewrite method
//
// NOTES
struct RewriteArgument
{
	bool m_bCheckUnion;
	bool m_bNoRelationChange;

	RewriteArgument()
		: m_bCheckUnion(true),
		  m_bNoRelationChange(false)
	{}
};

// STRUCT
//	Plan::Predicate::CheckArgument -- argument for Interface::check method
//
// NOTES
struct CheckArgument
	: public Scalar::CheckArgument
{
	bool m_bNoTop;

	CheckArgument(Interface::ICandidate* pCandidate_,
				  const VECTOR<Interface::ICandidate*>& vecPrecedingCandidate_,
				  bool bNoTop_ = false)
		: Scalar::CheckArgument(pCandidate_, vecPrecedingCandidate_),
		  m_bNoTop(bNoTop_)
	{}
	CheckArgument(const CheckArgument& cArgument_)
		: Scalar::CheckArgument(cArgument_),
		  m_bNoTop(cArgument_.m_bNoTop)
	{}
};

// STRUCT
//	Plan::Predicate::ChooseArgument -- argument for choose method
//
// NOTES
struct ChooseArgument
{
	// Candidate creating
	Interface::ICandidate* m_pCandidate;
	// Order specification in the candidate
	Order::CheckedSpecification* m_pCheckedOrder;
	// Chosen interface for fetching
	Interface::IPredicate* m_pFetch;
	// Cost if no indexes are used
	AccessPlan::Cost m_cScanCost;
	// Limit estimation in cost calculation
	AccessPlan::Cost::Value m_cEstimateLimit;
	// Repeating count of iteration in joining
	AccessPlan::Cost::Value m_cRepeatCount;
	// if true, order specification should be checked if any
	bool m_bForceOrder;
	// if true, unknown case is considered in using index
	bool m_bCheckUnknown;

	ChooseArgument(Interface::ICandidate* pCandidate_,
				   Order::CheckedSpecification* pCheckedOrder_,
				   const AccessPlan::Cost& cScanCost_,
				   const AccessPlan::Cost::Value& cEstimateLimit_,
				   const AccessPlan::Cost::Value& cRepeatCount_)
		: m_pCandidate(pCandidate_),
		  m_pCheckedOrder(pCheckedOrder_),
		  m_pFetch(0),
		  m_cScanCost(cScanCost_),
		  m_cEstimateLimit(cEstimateLimit_),
		  m_cRepeatCount(cRepeatCount_),
		  m_bForceOrder(false),
		  m_bCheckUnknown(false)
	{}
	ChooseArgument(const ChooseArgument& cArgument_)
		: m_pCandidate(cArgument_.m_pCandidate),
		  m_pCheckedOrder(cArgument_.m_pCheckedOrder),
		  m_pFetch(cArgument_.m_pFetch),
		  m_cScanCost(cArgument_.m_cScanCost),
		  m_cEstimateLimit(cArgument_.m_cEstimateLimit),
		  m_cRepeatCount(cArgument_.m_cRepeatCount),
		  m_bForceOrder(cArgument_.m_bForceOrder),
		  m_bCheckUnknown(cArgument_.m_bCheckUnknown)
	{}
};

// STRUCT
//	Plan::Predicate::CheckUnknownArgument -- argument for checkUnknown method
//
// NOTES
struct CheckUnknownArgument
{
	Utility::ScalarSet m_cKey;
	bool m_bArray;
	Interface::IPredicate* m_pPredicate;

	CheckUnknownArgument()
		: m_cKey(),
		  m_bArray(false),
		  m_pPredicate(0)
	{}
};

// STRUCT
//	Plan::Predicate::CheckRetrievableArgument -- argument for isRetrievable/addRetrieve method
//
// NOTES
struct CheckRetrievableArgument
{
	Scalar::Field* m_pField; // target field
	bool m_bNeedScan;		 // if true, retrieve from scanned file

	///////////////////
	// results
	///////////////////
	Interface::IPredicate* m_pPredicate;
	Candidate::File* m_pFile;

	CheckRetrievableArgument()
		: m_pField(0),
		  m_bNeedScan(false),
		  m_pPredicate(0),
		  m_pFile(0)
	{}
	CheckRetrievableArgument(Scalar::Field* pField_)
		: m_pField(pField_),
		  m_bNeedScan(false),
		  m_pPredicate(0),
		  m_pFile(0)
	{}
};

// STRUCT
//	Plan::Predicate::CheckNeedScanArgument -- argument for isNeedScan method
//
// NOTES
struct CheckNeedScanArgument
{
	Candidate::Table* m_pTable; // target table
	bool m_bIsTop;				// if false, it is under OR or NOT
	bool m_bInAnd;				// if true, it is under AND
	bool m_bNeedScan;			// if false, intermediately result is false

	CheckNeedScanArgument()
		: m_pTable(0),
		  m_bIsTop(true),
		  m_bInAnd(false),
		  m_bNeedScan(true)
	{}
	CheckNeedScanArgument(Candidate::Table* pTable_,
						  bool bIsTop_ = true,
						  bool bInAnd_ = false,
						  bool bNeedScan_ = true)
		: m_pTable(pTable_),
		  m_bIsTop(bIsTop_),
		  m_bInAnd(bInAnd_),
		  m_bNeedScan(bNeedScan_)
	{}
	CheckNeedScanArgument(const CheckNeedScanArgument& cOther_)
		: m_pTable(cOther_.m_pTable),
		  m_bIsTop(cOther_.m_bIsTop),
		  m_bInAnd(cOther_.m_bInAnd),
		  m_bNeedScan(cOther_.m_bNeedScan)
	{}
	CheckNeedScanArgument(const CheckNeedScanArgument& cOther_,
						  bool bIsTop_)
		: m_pTable(cOther_.m_pTable),
		  m_bIsTop(bIsTop_),
		  m_bInAnd(cOther_.m_bInAnd),
		  m_bNeedScan(cOther_.m_bNeedScan)
	{}
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_ARGUMENT_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
