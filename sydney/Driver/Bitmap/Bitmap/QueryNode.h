// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNode.h
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

#ifndef __SYDNEY_BITMAP_QUERYNODE_H
#define __SYDNEY_BITMAP_QUERYNODE_H

#include "Bitmap/Condition.h"
#include "Bitmap/Module.h"

#include "Common/BitSet.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;
class Condition;

//
//	CLASS
//	Bitmap::QueryNode -- 検索を処理するノード
//
//	NOTES
//
class QueryNode
{
public:
	//
	//	ENUM
	//	Bitmap::QueryNode::Type::Value -- ノード種別
	//
	struct Type
	{
		enum Value
		{
			And,
			Or,
			Term
		};
	};

	// コンストラクタ
	QueryNode(Type::Value eType, const ModUnicodeChar* p_,
			  Condition& cCondition_);

	// デストラクタ
	virtual ~QueryNode();

	// タイプを得る
	Type::Value getType() const { return m_eType; }

	// 有効化する
	void validate(bool bVerify_);
	// This function is pulbic, but used only QueryNode*.
	virtual void doValidate(const ModUnicodeChar*& p, Condition& cCondition_,
							ModUInt32& uiIteratorCount_, bool bVerify_) = 0;

	// 次を得る
	virtual Common::BitSet::UnitType next() = 0;

	// 該当するUnitTypeまで移動する
	virtual void seek(ModSize offset_) = 0;

	// 文字列からQueryNodeのインスタンスを得る
	static QueryNode* getQueryNode(const ModUnicodeChar* p,
								   BitmapFile& cFile_,
								   Condition& cCondition_);

	// [YET]
	bool get(Common::BitSet* pBitSet_, ModUInt32 uiMaxRowID_,
			 const Common::BitSet* pNarrowingBitSet_);

	// 検索結果件数見積り
	ModSize getEstimateCount(ModUInt32 uiMaxRowID_);

	// This function is pulbic, but used only QueryNode*.
	bool getSuspend() const { return m_bSuspend; }

	// [YET]
	virtual void reset(ModUInt32& uiIteratorCount_) = 0;
	virtual void revalidate(const ModUnicodeChar*& p_,
							Condition& cCondition_,
							ModUInt32& uiIteratorCount_,
							const Common::BitSet* pNarrowingBitSet_) = 0;

protected:
	// Get the maximum number of iterators which is validated at once
	static ModUInt32 getMaxIteratorCount();

	// Set suspend.
	void setSuspend(bool bSuspend_) { m_bSuspend = bSuspend_; }

	// [YET]
	bool convert(Common::BitSet* pBitSet_, ModUInt32 uiMaxRowID_,
				 ModUInt32& uiIteratorCount_,
				 const Common::BitSet* pNarrowingBitSet_);
	
private:
	//	CONST private
	//	Bitmap::QueryNode::MaxIteratorCount
	//		-- The maximum number of iterators which is validated at once
	//
#ifdef SYD_ARCH64
	enum { MaxIteratorCount = 8000 };
#else
	enum { MaxIteratorCount = 1000 };
#endif
	
	// ノード種別
	Type::Value m_eType;

	// Is the node suspended to set iterators?
	bool m_bSuspend;

	// The position of the unanalyzed query
	const ModUnicodeChar* m_pQuery;

	// The reference of Condition for reuse in QueryNodeTerm::doValidate()
	Condition& m_cCondition;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_QUERYNODE_H

//
//	Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
