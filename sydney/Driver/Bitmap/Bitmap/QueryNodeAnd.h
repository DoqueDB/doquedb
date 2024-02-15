// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeAnd.h
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

#ifndef __SYDNEY_BITMAP_QUERYNODEAND_H
#define __SYDNEY_BITMAP_QUERYNODEAND_H

#include "Bitmap/Module.h"
#include "Bitmap/QueryNodeOperator.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;

//
//	CLASS
//	Bitmap::QueryNodeAnd -- 検索を処理するノード
//
//	NOTES
//
class QueryNodeAnd : public QueryNodeOperator
{
public:
	// コンストラクタ
	QueryNodeAnd(BitmapFile& cFile_, const ModUnicodeChar* p_,
				 Condition& cCondition_);
	// デストラクタ
	virtual ~QueryNodeAnd();

	// 有効化
	// This function is pulbic, but used only QueryNode*.
	void doValidate(const ModUnicodeChar*& p, Condition& cCondition_,
					ModUInt32& uiIteratorCount_, bool bVerify_);

	// 次の結果を得する
	Common::BitSet::UnitType next();

	// [YET]
	void reset(ModUInt32& uiIteratorCount_);
	void revalidate(const ModUnicodeChar*& p_,
					Condition& cCondition_,
					ModUInt32& uiIteratorCount_,
					const Common::BitSet* pNarrowingBitSet_);
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_QUERYNODEAND_H

//
//	Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
