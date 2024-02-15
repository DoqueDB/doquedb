// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNode.cpp --
// 
// Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/BitmapFile.h"
#ifdef DEBUG
#include "Bitmap/Parameter.h"
#endif
#include "Bitmap/QueryNode.h"
#include "Bitmap/QueryNodeAnd.h"
#include "Bitmap/QueryNodeOr.h"
#include "Bitmap/QueryNodeTerm.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
#ifdef DEBUG
	//	The maximum iterator count for debug
	ParameterInteger _cDebugMaxIteratorCount(
		"Bitmap_DebugMaxIteratorCount", 1000);
#endif
}

//
//	FUNCTION public
//	Bitmap::QueryNode::QueryNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::QueryNode::Type::Value eType
//		タイプ
//	const ModUnicodeChar* p_
//	Condition& cCondition_
//		[YET]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
QueryNode::QueryNode(Type::Value eType_, const ModUnicodeChar* p_,
					 Condition& cCondition_)
	: m_eType(eType_), m_bSuspend(false), m_pQuery(p_),
	  m_cCondition(cCondition_)
{}

//
//	FUNCTION public
//	Bitmap::QueryNode::~QueryNode -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
QueryNode::~QueryNode()
{}

//
//	FUNCTION public
//	Bitmap::QueryNode::validate --
//
//	NOTES
//
//	ARGUMENTS
//	bool bVerify_
//		[YET]
//
//	RETURN
//
//	EXCEPTIONS
//
void
QueryNode::validate(bool bVerify_)
{
	ModUInt32 temp = 0;
	doValidate(m_pQuery, m_cCondition, temp, bVerify_);

	// In a verification, a suspension does NOT occur.
	// When this index is defined to a non-array column,
	// the condition is expressed as just one iterator.
	// When this column is defined to an array column,
	// the condition is expressed as some iterators.
	// The number of iterators is equals to
	// the number of discrete values which is included in one array data.
	// In such case, non-suspension may make a memory allocation fail.
	// But it is NO PROBLEM.
	// Because, if Buffer_NormalPoolSize in the Parameter had been small,
	// such data could not be inserted for a memory allocation failure.
	; _TRMEISTER_ASSERT(bVerify_ == false || getSuspend() == false);
}

//
//	FUNCTION public static
//	Bitmap::QueryNode::getQueryNode -- 文字列から該当するQueryNodeを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* p
//		#and とか #or とかの文字列
//	BitmapFile& cFile_
//		ファイル
//	Condition& cCondition_
//		[YET]
//
//	RETURN
//	QueryNode*
//		該当するQueryNode
//
//	EXCEPTIONS
//
QueryNode*
QueryNode::getQueryNode(const ModUnicodeChar* p, BitmapFile& cFile_,
						Condition& cCondition_)
{
	QueryNode* pNode;
	
	if (*p == '#' && *(p+1) == 'a' && *(p+2) == 'n' && *(p+3) == 'd')
	{
		// #and(...);
		pNode = new QueryNodeAnd(cFile_, p, cCondition_);
	}
	else if (*p == '#' && *(p+1) == 'o' && *(p+2) == 'r')
	{
		// #or(...);
		pNode = new QueryNodeOr(cFile_, p, cCondition_);
	}
	else
	{
		// #eq(...)とかその他
		pNode = new QueryNodeTerm(cFile_, p, cCondition_);
	}

	return pNode;
}

//
//	FUNCTION public
//	Bitmap::QueryNode::get --
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet* pBitSet_
//	ModUInt32 uiMaxRowID_
//		[YET]
//
//	RETURN
//	bool
//		[YET]
//
//	EXCEPTIONS
//
bool
QueryNode::get(Common::BitSet* pBitSet_, ModUInt32 uiMaxRowID_,
			   const Common::BitSet* pNarrowingBitSet_)
{
	// The count of iterators is maximum when the validation is suspended.
	// The count is less than maximum when it is not suspended.
	// And it is no problem that the count is much than the actual count.
	// So the count is set by the maximum count.
	
	ModUInt32 uiIteratorCount = getMaxIteratorCount();
	return convert(pBitSet_, uiMaxRowID_, uiIteratorCount, pNarrowingBitSet_);
}

//
//	FUNCTION public
//	Bitmap::QueryNode::getEstimateCount -- 検索結果件数見積りを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		検索結果件数見積り
//
//	EXCEPTIONS
//
ModSize
QueryNode::getEstimateCount(ModUInt32 uiMaxRowID_)
{
	ModSize count = 0;
	
	// これから取得する Common::BitSet 上の最終のUnitのポジション
	ModUInt32 last = uiMaxRowID_ / (Common::BitSet::UNIT_SIZE * 8);
	
	// Common::BitSet 上のポジション
	ModUInt32 pos = 0;
	do
	{
		// Common::BitSet::UnitType ごとに
		// ビットセットを取り出す
		
		Common::BitSet::UnitType unit = next();
		count += unit.count();

		++pos;
	}
	while (pos <= last);

	return count;
}

//
//	FUNCTION protected static
//	Bitmap::QueryNode::getMaxIteratorCount --
//		Get the maximum number of iterators which is validated at once
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModUInt32
//		[YET]
//
//	EXCEPTIONS
//
ModUInt32
QueryNode::getMaxIteratorCount()
{
#ifdef DEBUG
	return _cDebugMaxIteratorCount.get();
#else
	return MaxIteratorCount;
#endif
}


//
//	FUNCTION protected
//	Bitmap::QueryNode::convert --
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet* pBitSet_
//	ModUInt32 uiMaxRowID_
//	ModUInt32& uiIteratorCount_
//		[YET]
//
//	RETURN
//	bool
//		[YET]
//
//	EXCEPTIONS
//
bool
QueryNode::convert(Common::BitSet* pBitSet_, ModUInt32 uiMaxRowID_,
				   ModUInt32& uiIteratorCount_,
				   const Common::BitSet* pNarrowingBitSet_)
{
	bool result = false;
	
	// これから取得する Common::BitSet 上の最終のUnitのポジション
	ModUInt32 last = uiMaxRowID_ / (Common::BitSet::UNIT_SIZE * 8);
	
	bool first = true;
	for(;;)
	{
		// Common::BitSet 上のポジション
		ModUInt32 pos = 0;
		do
		{
			// Common::BitSet::UnitType ごとに
			// ビットセットを取り出す
			Common::BitSet::UnitType unit = next();

			if (pNarrowingBitSet_)
			{
				// 絞り込み条件があるので、論理積をとる
				unit &= pNarrowingBitSet_->getUnitType(pos);
			}
			
			if (unit.none() == false)
			{
				// 1ビットもたっていないものは挿入しない
				// ここには何かビットが立っている
				if (first == true)
				{
					pBitSet_->insertUnitType(pos, unit);
				}
				else
				{
					pBitSet_->orUnitType(pos, unit);
				}
				result = true;
			}
			
			++pos;
		}
		while (pos <= last);
		
		first = false;
		reset(uiIteratorCount_);

		if (getSuspend() == true)
		{
			// Set next iterators.
			revalidate(m_pQuery, m_cCondition, uiIteratorCount_,
					   pNarrowingBitSet_);
		}
		else
		{
			break;
		}
	}

	return result;
}

//
//	Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
