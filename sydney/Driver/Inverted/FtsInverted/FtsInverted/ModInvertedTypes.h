// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedTypes.h -- 転置ファイル全体で共通の typedef の定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedTypes_H__
#define __ModInvertedTypes_H__

#ifdef SYD_USE_LARGE_VECTOR
#include "Common/Internal.h"
#include "Common/LargeVector.h"
#endif

#include "ModMap.h"
#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

//
// TYPEDEF
// ModInvertedDocumentID -- 文書ID
//
// NOTES
// 転置ファイルにおける文書IDのデータ型。
//
typedef ModUInt32 ModInvertedDocumentID;

//
// CONST
// ModInvertedUpperBoundDocumentID -- 文書IDの上限値
//
// NOTES
// 文書IDの上限値。有効な文書IDはこの値未満である。
//
const ModInvertedDocumentID ModInvertedUpperBoundDocumentID = 0x80000000;

//
// CONST
// ModInvertedUndefinedDocumentID -- 文書IDの未定義値
//
// NOTES
// 文書IDの未定義値。
//
const ModInvertedDocumentID ModInvertedUndefinedDocumentID = ModUInt32Max;

//
// TYPEDEF
// ModInvertedDocumentScore -- ランキング検索用文書スコア
//
// NOTES
// ランキング検索用の文書スコアのデータ型。
//
typedef double ModInvertedDocumentScore;

//
// TYPEDEF
// ModInvertedTermScore -- 検索語スコア
//
// NOTES
// 検索語スコアのデータ型。
//
typedef double ModInvertedTermScore;

//
// TYPEDEF
// ModInvertedTermNo -- 検索語番号
//
// NOTES
// 検索語番号のデータ型。
//
typedef ModUInt32 ModInvertedTermNo;

//
// TYPEDEF
// ModInvertedTermFrequency -- 検索語頻度
//
// NOTES
// 検索語頻度のデータ型。
//
typedef ModUInt32 ModInvertedTermFrequency;

//
// TYPEDEF
// ModInvertedDataUnit -- データユニット
//
// NOTES
// 圧縮データの内部表現単位のデータ型。
//
typedef ModUInt32 ModInvertedDataUnit;

//
// CONST
// ModInvertedDataUnitBitSize -- データユニットのビット数
//
// NOTES
// データユニットのビット数。
//
const ModSize ModInvertedDataUnitBitSize = sizeof(ModInvertedDataUnit)*8;

//
//	TYPEDEF
//	ModInvertedVector -- 転置モジュール内で使用するベクター
//
//	NOTES
//
#ifdef SYD_USE_LARGE_VECTOR
#define ModInvertedVector				_SYDNEY::Common::LargeVector
#else
#define ModInvertedVector				ModVector
#endif

//
// TYPEDEF
// ModInvertedLocationList -- 位置リスト
//
// NOTES
// 文書内オフセットのリストを表現するデータ型。
//
typedef	ModInvertedVector<ModSize> ModInvertedLocationList;

//
// CONST
// ModInvertedIndexKeyLenMax -- 索引文字列の長さの上限値
//
// NOTES
// 索引文字列の長さの上限値。
//
const ModSize ModInvertedIndexKeyLenMax = 254;

//
// CONST
// ModInvertedUndefinedCachePageNum -- 未定義のキャッシュページ数
//
// NOTES
// 未定義のキャッシュページ数を表す値。
//
const ModSize ModInvertedUndefinedCachePageNum = ModSizeMax;

//
// ENUM
// ModInvertedFileVersion -- 転置ファイルバージョン
//
// NOTES
// 転置ファイルバージョンを表す。
//
enum ModInvertedFileVersion {
	ModInvertedFileVersionUndefined = (-1),
	ModInvertedFileVersionFirst = 0,
	ModInvertedFileVersionSecond = 1,
	ModInvertedFileVersionThird = 2,
	ModInvertedFileVersionNum,
	ModInvertedFileCurrentVersion = ModInvertedFileVersionThird
};

//
// ENUM
// ModInvertedFileIndexingType -- 転置ファイルの索引付けタイプ
//
// NOTES
// 転置ファイルバージョンを表す。
//
enum ModInvertedFileIndexingType {
	ModInvertedFileUndefinedIndexingType = 0,
	ModInvertedFileNgramIndexing = 1,
	ModInvertedFileWordIndexing = 2,
	ModInvertedFileDualIndexing = (ModInvertedFileNgramIndexing|ModInvertedFileWordIndexing)
};

//
// CONST
// ModInvertedFileOverflowFileNumMax -- オーバーフローファイルの最大数
//
// NOTES
// オーバーフローファイルの最大数
//
const ModSize ModInvertedFileOverflowFileNumMax = 7;

//
// ENUM
// ModInvertedListAccessMode -- 転置リストアクセスモード
//
// NOTES
// 転置リストへのアクセスのモードを識別する。
//		ModInvertedListCreateMode		なければ新規作成
//		ModInvertedListSearchMode		完全一致検索
//		ModInvertedListLowerBoundMode	下限検索
//
enum ModInvertedListAccessMode {
	ModInvertedListCreateMode,
	ModInvertedListSearchMode,
	ModInvertedListLowerBoundMode
};

//
// ENUM
// ModInvertedTermMatchMode -- 検索文字列照合モード
//
// NOTES
// 検索文字列の照合モードを識別する。
//
enum ModInvertedTermMatchMode {
	ModInvertedTermStringMode	= 0x0001,		// 文字列検索
	ModInvertedTermWordHead		= 0x0002,		// 先頭が単語境界と一致
	ModInvertedTermWordTail		= 0x0004,		// 末尾が単語境界と一致
	ModInvertedTermCheckAll		= 0x0008,		// 構成単語を全て調べる
#ifdef V1_4
	ModInvertedTermApproximateMode				// あいまいモード
		= 0x0010,
	ModInvertedTermMultiLanguageMode			// 多言語対応モード
		= 0x0016,
#endif // V1_4
	ModInvertedTermSimpleWordMode				// 単純単語検索
		= (ModInvertedTermWordHead|ModInvertedTermWordTail),
	ModInvertedTermExactWordMode				// 厳格単語検索
		= (ModInvertedTermSimpleWordMode|ModInvertedTermCheckAll),
	ModInvertedTermWordMode						// 単語検索（デフォルト）
		= ModInvertedTermSimpleWordMode,
	ModInvertedTermUndefinedMode = 0x0000
};

//
// TYPEDEF
// ModInvertedQueryValidateMode -- クエリ有効化モード
//
// NOTES
// クエリの有効化モードのデータ型。
//
typedef ModUInt32 ModInvertedQueryValidateMode;

//
// TYPEDEF
// ModInvertedQueryEvaluateMode -- クエリ評価化モード
//
// NOTES
// クエリの評価化モードのデータ型。
//
typedef ModUInt32 ModInvertedQueryEvaluateMode;

#ifdef V1_5
//
// TYPEDEF
// ModInvertedTermWeightPair -- 検索語と重みの組
//
// NOTES
// 検索語と重みの組のデータ型。
//
typedef ModPair<ModUnicodeString, double> ModInvertedTermWeightPair;
#endif

//
// TYPEDEF
// ModInvertedFeatureElement -- 特徴語と重みの組
//
// NOTES
// 特徴語と重みの組のデータ型。
//
typedef ModPair<ModUnicodeString, double> ModInvertedFeatureElement;

//
// TYPEDEF
// ModInvertedFeatureElement -- 特徴語と重みの組のリスト
//
// NOTES
// 特徴語と重みの組のリストのデータ型。
//
typedef ModVector<ModInvertedFeatureElement> ModInvertedFeatureList;

//
// TYPEDEF
// ModInvertedFeatureElement -- 特徴語と重みの組のマップ
//
// NOTES
// 特徴語と重みの組のマップのデータ型。
//
typedef ModMap<ModUnicodeString, double, ModLess<ModUnicodeString> > ModInvertedFeatureMap;

//
// CLASS
// ModInvertedSimpleLess -- 簡易比較クラス
//
// NOTES
// ModUnicodeString をキーとするマップの比較のためのクラス
//
class ModInvertedSimpleLess
{
public:
	ModBoolean operator()(const ModUnicodeString& value1,
						  const ModUnicodeString& value2) const
	{
		const ModUnicodeChar* l = (const ModUnicodeChar*)value1;
		const ModUnicodeChar* r = (const ModUnicodeChar*)value2;

		// 特許データでの評価により比較順序を入れ換えて高速化
		// for (; *l != 0 && *r != 0 && *l == *r; ++l, ++r)
		for (; *l == *r && *l != 0 && *r != 0; ++l, ++r)
			;

		return ModBoolean(*l < *r);
	}
};

//
// ENUM
// ModInvertedUnaSpaceMode -- 正規化時の空白文字の扱い方
//
// NOTES
// UNAへ渡す、空白文字の扱い方を表す。
//
enum ModInvertedUnaSpaceMode {
	ModInvertedUnaSpaceAsIs = 0,  // default
	ModInvertedUnaSpaceNoNormalize,
	ModInvertedUnaSpaceDelete,
	ModInvertedUnaSpaceReset
};

#endif	// __ModInvertedTypes_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
