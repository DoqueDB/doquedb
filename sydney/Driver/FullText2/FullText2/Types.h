// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Types.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_TYPES_H
#define __SYDNEY_FULLTEXT2_TYPES_H

#include "FullText2/Module.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	TYPEDEF
//	FullText2::DocumentID -- 文書ID
//
typedef ModUInt32 DocumentID;

//
//	TYPEDEF
//	FullText2::DocumentScore -- 文書スコア
//
typedef double DocumentScore;

//
//	CONST
//	FullText2::UndefinedDocumentID -- 無効な文書IDをあらわす数
//
const DocumentID UndefinedDocumentID = 0x80000000;

//
//	CONST
//	FullText2::UndefinedLocation -- 無効な位置情報をあらわす数
//
const ModSize UndefinedLocation = 0xffffffff;

//
//	CONST
//	FullText2::DocumentIdMask -- 有効な文書IDの範囲
//
const ModUInt32 DocumentIdMask = 0x7fffffff;

//
//	CONST
//	FullText2::UndefinedRowID -- 無効な文書IDをあらわす数
//
const ModUInt32 UndefinedRowID = 0xffffffff;

//
//	CONST
//	FullText2::UndefinedResourceID -- 無効なリソースIDをあらわす数
//
const ModSize UndefinedResourceID = 0x7fffffff;

//
//	NAMESPACE
//	FullText2::ListType -- リスト種別をあらわす数
//
namespace ListType
{
	const ModUInt32 Short		= 0x00000000;
	const ModUInt32 Middle		= 0x80000000;
	const ModUInt32 Batch		= 0x40000000;

	const ModUInt32 TypeMask	= 0xf0000000;
	const ModUInt32 SizeMask	= 0x0fffffff;
}

//
//	STRUCT
//	FullText2::IndexingType -- 索引タイプ
//
//	NOTES
//	この値はFileIDに記録されているので、変更してはいけない
//
struct IndexingType
{
	enum Value
	{
		Undefined = 0,
	
		Ngram	= 1,					// N-gram索引
		Word	= 2,					// Word索引
		Dual	= (Ngram | Word)		// Dual索引
	};
};

//
//	STRUCT
//	FullText2::MatchMode -- 一致モード
//
//	NOTES
//
struct MatchMode
{
	enum Value
	{
		Unknown,
		
		String,				// 文字列検索 (N-GRAM と DUAL)
		WordHead,			// 先頭が単語境界と一致 (DUAL と WORD)
		WordTail,			// 末尾が単語境界と一致 (DUAL)
		SimpleWord,			// 先頭と末尾が単語境界と一致 (DUAL)
		ExactWord,			// すべての単語境界が一致 (DUAL と WORD)
		
		MultiLanguage		// 多言語対応モード (DUAL)
	};
};

//
//	STRUCT
//	FullText2::Delayed
//
//	NOTES
//	この値はFileIDに記録されているので、変更してはいけない
//
struct Delayed
{
	enum Value
	{
		None = 0,
	
		Sync	= 1,		// 同期
		Async	= 2			// 非同期
	};
};

//
//	STRUCT
//	FullText2::SortKey -- 検索結果をソートするキー
//
//	NOTES
//
struct SortKey
{
	enum Value
	{
		DocID,		// 文書ID
		Score		// スコア
	};
};

//
//	STRUCT
//	FullText2::Order -- ソート順
//
//	NOTES
//
struct Order
{
	enum Value
	{
		Asc,		// 昇順
		Desc		// 降順
	};
};

//
//	STRUCT
//	FullText2::AdjustMethod -- スコア調整方法
//
struct AdjustMethod
{
	enum Value
	{
		Unknown,
		
		Multiply,	// 乗算
		Add,		// 加算
		Replace		// 置き換える
	};
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_TYPES_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
