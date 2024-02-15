// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Argument.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_ARGUMENT_H
#define __SYDNEY_PLAN_CANDIDATE_ARGUMENT_H

#include "Plan/Candidate/Module.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Limit.h"
#include "Plan/File/Argument.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Sql/Query.h"

#include "Execution/Action/Argument.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Table;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////
// STRUCT
//	Plan::Candidate::InquiryArgument -- argument class for ICandidate::inquiry
//
// NOTES
struct InquiryArgument
{
	struct Target
	{
		typedef unsigned int Value;
		enum _Value
		{
			None				= 0,
			ReferTable			= 1,
			Distributed			= 1 << 1,
			ValueNum
		};
	};

	InquiryArgument(Target::Value iTarget_)
		: m_iTarget(iTarget_),
		  m_pSchemaTable(0)
	{}
	InquiryArgument(Target::Value iTarget_,
					Schema::Table* pSchemaTable_)
		: m_iTarget(iTarget_),
		  m_pSchemaTable(pSchemaTable_)
	{}

	Target::Value m_iTarget;
	Schema::Table* m_pSchemaTable;
};

///////////////////////////
// shortcut

struct Inquiry
{
	static bool isReferTable(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_,
							 Schema::Table* pSchemaTable_);
};

// STRUCT
//	Plan::Candidate::AdoptArgument --
//
// NOTES

struct AdoptArgument
{
	typedef Execution::Action::Argument::Target Target;

	Execution::Interface::IIterator* m_pInput;
	Candidate::Table* m_pTable;
	Candidate::File* m_pScanFile;
	Interface::ICandidate* m_pCandidate; // target candidate in generation
	VECTOR<Interface::ICandidate*> m_vecPrecedingCandidate;// class representing preceding candidate
	Schema::Cascade* m_pCascade;
	Sql::Query* m_pQuery;
	bool m_bCollecting;					// set true if tuple will be collected
	bool m_bOriginal;					// set true if original data is used for UpdateField
	bool m_bSkipCheck;					// set true if data is used for skip check in update operations
	bool m_bOperation;					// set true if operation is used for update
	bool m_bBatch;						// set true if used in batch mode
	bool m_bLocator;					// set true if locator is used
	bool m_bForceRowID;					// RowIDをBitSet型からshort型に変換するフィルタをセットする場合にtrue
	bool m_bForceRowIDSet;				// RowIDをBitSetg型からshort型に変換するフィルタをセットした場合にtrue
	bool m_bGenerateForPredicate;		// predicate処理用データのgenerateの場合にtrue
	bool m_bConcatinateIterable;		// 分散時にパラメータを変更した文をConcatinateする場合にtrue
	bool m_bReplicate;					// レプリケーションの場合にtrue
	bool m_bForLock;					// set true if generating for object-lock
	bool m_bForUpdate;					// set true if generating for update
	Target::Value m_eTarget;			// target to add action
	AccessPlan::Limit m_cLimit;			// limit hint at adopting

	// for if-else scope
	VECTOR<MAP<int, int, LESS<int> > > m_vecmapNodeVariable;

	AdoptArgument()
		: m_pInput(0),
		  m_pTable(0),
		  m_pScanFile(0),
		  m_pCandidate(0),
		  m_vecPrecedingCandidate(),
		  m_pCascade(0),
		  m_pQuery(0),
		  m_bCollecting(false),
		  m_bOriginal(false),
		  m_bSkipCheck(false),
		  m_bOperation(false),
		  m_bBatch(false),
		  m_bLocator(false),
		  m_bForceRowID(false),
		  m_bForceRowIDSet(false),
		  m_bGenerateForPredicate(false),
		  m_bConcatinateIterable(false),
		  m_bReplicate(false),
		  m_bForLock(false),
		  m_bForUpdate(false),
		  m_eTarget(Target::Execution),
		  m_cLimit(),
		  m_vecmapNodeVariable()
	{}
	AdoptArgument(const AdoptArgument& cArgument_)
		: m_pInput(cArgument_.m_pInput),
		  m_pTable(cArgument_.m_pTable),
		  m_pScanFile(cArgument_.m_pScanFile),
		  m_pCandidate(cArgument_.m_pCandidate),
		  m_vecPrecedingCandidate(cArgument_.m_vecPrecedingCandidate),
		  m_pCascade(cArgument_.m_pCascade),
		  m_pQuery(cArgument_.m_pQuery),		  
		  m_bCollecting(cArgument_.m_bCollecting),
		  m_bOriginal(cArgument_.m_bOriginal),
		  m_bSkipCheck(cArgument_.m_bSkipCheck),
		  m_bOperation(cArgument_.m_bOperation),
		  m_bBatch(cArgument_.m_bBatch),
		  m_bLocator(cArgument_.m_bLocator),
		  m_bForceRowID(cArgument_.m_bForceRowID),
		  m_bForceRowIDSet(cArgument_.m_bForceRowIDSet),
		  m_bGenerateForPredicate(cArgument_.m_bGenerateForPredicate),
		  m_bConcatinateIterable(cArgument_.m_bConcatinateIterable),
		  m_bReplicate(cArgument_.m_bReplicate),
		  m_bForLock(cArgument_.m_bForLock),
		  m_bForUpdate(cArgument_.m_bForUpdate),
		  m_eTarget(cArgument_.m_eTarget),
		  m_cLimit(cArgument_.m_cLimit),
		  m_vecmapNodeVariable(cArgument_.m_vecmapNodeVariable)
	{}

	// add using candidate
	void setCandidate(Interface::ICandidate* pCandidate_);
	
	// push if-else scope
	void pushScope();
	// pop if-else scope
	void popScope();
	// get node-variable correspondence
	int getNodeVariable(int iNodeID_);
	// set node-variable correspondence
	bool setNodeVariable(int iNodeID_, int iDataID_);
};

// STRUCT
//	Plan::Candidate::CheckIndexArgument --
//
// NOTES

struct CheckIndexArgument
{
	VECTOR<Predicate::ChosenInterface*> m_vecNeedScan;
	VECTOR<Predicate::ChosenInterface*> m_vecBitSet;
	VECTOR<Predicate::ChosenInterface*> m_vecSearchBitSet;
	VECTOR<Predicate::ChosenInterface*> m_vecIndexScan;
	VECTOR<Predicate::ChosenInterface*> m_vecFetch;
	Predicate::ChosenInterface* m_pOrderScan;

	Utility::FileSet m_cCheckedFile;
	bool m_bUseGetBitSet;
	bool m_bUseSearchBitSet;
	bool m_bTop;
	bool m_bInAnd;
	bool m_bInOr;
	bool m_bIgnoreField;

	Candidate::Table* m_pTable;
	Interface::ICandidate* m_pCandidate;
	
	bool m_bChecked;

	CheckIndexArgument()
		: m_vecNeedScan(),
		  m_vecBitSet(),
		  m_vecSearchBitSet(),
		  m_vecIndexScan(),
		  m_vecFetch(),
		  m_pOrderScan(0),
		  m_cCheckedFile(),
		  m_bUseGetBitSet(true),
		  m_bUseSearchBitSet(true),
		  m_bTop(true),
		  m_bInAnd(false),
		  m_bInOr(false),
		  m_bIgnoreField(false),
		  m_pTable(0),
		  m_bChecked(false)
	{}
	CheckIndexArgument(const CheckIndexArgument& cArgument_)
		: m_vecNeedScan(cArgument_.m_vecNeedScan),
		  m_vecBitSet(cArgument_.m_vecBitSet),
		  m_vecSearchBitSet(cArgument_.m_vecSearchBitSet),
		  m_vecIndexScan(cArgument_.m_vecIndexScan),
		  m_vecFetch(cArgument_.m_vecFetch),
		  m_pOrderScan(cArgument_.m_pOrderScan),
		  m_cCheckedFile(cArgument_.m_cCheckedFile),
		  m_bUseGetBitSet(cArgument_.m_bUseGetBitSet),
		  m_bUseSearchBitSet(cArgument_.m_bUseSearchBitSet),
		  m_bTop(cArgument_.m_bTop),
		  m_bInAnd(cArgument_.m_bInAnd),
		  m_bInOr(cArgument_.m_bInOr),
		  m_bIgnoreField(cArgument_.m_bIgnoreField),
		  m_pTable(cArgument_.m_pTable),
		  m_bChecked(cArgument_.m_bChecked)
	{}

	void clear()
	{
		bool bTop = m_bTop;
		bool bInAnd = m_bInAnd;
		bool bInOr = m_bInOr;

		(*this) = CheckIndexArgument();

		m_bTop = bTop;
		m_bInAnd = bInAnd;
		m_bInOr = bInOr;
	}

	void setAnd();
	void setOr();
	bool isUnset() {return !m_bInAnd && !m_bInOr;}

	bool isScanBitSet(Opt::Environment& cEnvironment_,
					  Candidate::Table* pTable_);
	bool isSearchBitSet(Opt::Environment& cEnvironment_);

	bool isEmpty();

	bool isOnlyBitSet();

	bool hasBitSet();

	bool isLimitAvailable();
};

// STRUCT
//	Plan::Candidate::AdoptIndexArgument --
//
// NOTES

struct AdoptIndexArgument
{
	struct Order
	{
		enum Type {
			None,
			ByRowID,
			ValueNum
		};
	};

	struct Element
	{
		Order::Type m_eOrder;
		bool m_bForceBitSet;
		int m_iBitSetID;				// output bitset
		int m_iPrevBitSetID;			// previous result bitset
		int m_iSearchBitSetID;			// bitset for seachbybitset

		Element()
			: m_eOrder(Order::None),
			  m_bForceBitSet(false),
			  m_iBitSetID(-1),
			  m_iPrevBitSetID(-1),
			  m_iSearchBitSetID(-1)
		{}
		Element(const Element& cElement_)
			: m_eOrder(cElement_.m_eOrder),
			  m_bForceBitSet(cElement_.m_bForceBitSet),
			  m_iBitSetID(cElement_.m_iBitSetID),
			  m_iPrevBitSetID(cElement_.m_iPrevBitSetID),
			  m_iSearchBitSetID(cElement_.m_iSearchBitSetID)
		{}

		int prepareBitSetData(Execution::Interface::IProgram& cProgram_);
	};

	bool m_bForceGetBitSet;
	MAP< Candidate::Table*, Element, LESS<Candidate::Table*> > m_cMap;

	AdoptIndexArgument()
		: m_bForceGetBitSet(false),
		  m_cMap()
	{}
	AdoptIndexArgument(const AdoptIndexArgument& cArgument_)
		: m_bForceGetBitSet(cArgument_.m_bForceGetBitSet),
		  m_cMap(cArgument_.m_cMap)
	{}

	void clear()
	{
		m_bForceGetBitSet = false;
		m_cMap.clear();
	}

	Element& getElement(Candidate::Table* pTable_)
	{
		return m_cMap[pTable_];
	}
};

// STRUCT
//	Plan::Candidate::RowDelayArgument --
//
// NOTES

struct RowDelayArgument
	: public Scalar::DelayArgument
{
	typedef Scalar::DelayArgument Super;
	typedef RowDelayArgument This;

	bool m_bGenerate;
	Utility::ScalarSet m_cNotDelayed;

	RowDelayArgument(bool bMinimum_ = false)
		: Super(bMinimum_),
		  m_bGenerate(true),
		  m_cNotDelayed()
	{}
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_ARGUMENT_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
