// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldType.h --
// 
// Copyright (c) 2004, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_INVERTED_FIELDTYPE_H
#define __SYDNEY_INVERTED_FIELDTYPE_H

#include "Inverted/Module.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

namespace FieldType
{
	// 転置索引で扱うフィールド型
	// ここで定義されているのは固定フィールド部分。
	
	// [NOTE] FullText::FileID::check() 内の FieldTypeValue に一致させること。
	//  FullText::LogicalInterface::setGetField() 内で呼ばれる
	//  FullText::FieldMask::getFieldType() は、この順番が一致していることを
	//  前提としている。
	
	// [YET] 全文で扱うフィールドもここで定義している。
	//  切り分けがうまくいってない。
	
	enum Value
	{
		Rowid = 1,
		Score,
		Section,
		Word,
		WordDf,
		WordScale,
		AverageLength,
		AverageCharLength,
		AverageWordCount,
		Tf,
		Count,
		Cluster,		// 検索結果のクラスリング用のカラム

		// 全文で扱うフィールド
		FeatureValue,	// 特徴語
		RoughKwicPosition,	// 検索結果に適合する範囲の開始位置
		
		// 外部インターフェースで使用する値は、この値の前に挿入する必要がある		
		Last,
		
		// Sydney内部で使用する
		// FTSInverted::ModInvertedSearchResult::factory を参照
		Internal = 31
	};
}

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_FIELDTYPE_H

//
//	Copyright (c) 2004, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
