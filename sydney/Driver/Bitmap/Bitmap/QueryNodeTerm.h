// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeTerm.h
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

#ifndef __SYDNEY_BITMAP_QUERYNODETERM_H
#define __SYDNEY_BITMAP_QUERYNODETERM_H

#include "Bitmap/Module.h"
#include "Bitmap/QueryNode.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapIterator;

//
//	CLASS
//	Bitmap::QueryNodeTerm -- 検索条件を処理するノード
//
//	NOTES
//
class QueryNodeTerm : public QueryNode
{
public:
	// コンストラクタ
	QueryNodeTerm(BitmapFile& cFile_, const ModUnicodeChar* p_,
				  Condition& cCondition_);
	// デストラクタ
	virtual ~QueryNodeTerm();

	// 有効化する
	// This function is pulbic, but used only QueryNode*.
	void doValidate(const ModUnicodeChar*& p, Condition& cCondition_,
					ModUInt32& uiIteratorCount_, bool bVerify_);

	// 次を得る
	Common::BitSet::UnitType next();

	// 該当するUnitTypeまで移動する
	void seek(ModSize offset_);

	// [YET]
	void reset(ModUInt32& uiIteratorCount_);
	void revalidate(const ModUnicodeChar*& p_,
					Condition& cCondition_,
					ModUInt32& uiIteratorCount_,
					const Common::BitSet* pNarrowingBitSet_)
		{ setIterator(uiIteratorCount_, false); }

	// [YET]
	bool isConverted() const { return m_bConvert; }
	void convert(ModUInt32& uiIteratorCount_,
				 const Common::BitSet* pNarrowingBitSet_);
	void merge(const QueryNodeTerm* pOther_);

private:
	// [YET]
	void setIterator(ModUInt32& uiIteratorCount_, bool bVerify_);

	// ファイル
	BitmapFile& m_cFile;

	// ビットマップのイテレータ
	ModVector<BitmapIterator*> m_vecpIterator;

	// [YET]
	bool m_bConvert;
	Common::BitSet m_cBitSet;
	ModUInt32 m_uiPos;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_QUERYNODETERM_H

//
//	Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
