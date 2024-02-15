// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTerm.h    -- ModTerm    の宣言
// 
// Copyright (c) 2000, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __ModTerm_H__
#define __ModTerm_H__

#include "ModNLP.h"                     // 自然言語解析器
#include "ModInvertedSearchResult.h"   // 検索結果

#include "ModTermException.h"
#include "ModTermStringFile.h"
#include "ModTermElement.h"
#include "ModTermPattern.h"
#include "ModTermMap.h"

#include "ModUnicodeString.h"
#include "ModMap.h"

// 検索語タイプ定義
#define TERM_STOP       0   /* 禁止語 */
#define TERM_ALPHABET   1   /* ユーザ定義1 */
#define TERM_SYMBOL     2   /* 記号.一般|未登録語.記号 */
#define TERM_NUMBER     3   /* 数詞 */
#define TERM_PREFIX     4   /* 接頭 */
#define TERM_SUFFIX     5   /* 接尾|助数詞 */
#define TERM_NOUN       6   /* 名詞.一般 */
#define TERM_NOUN_V     7   /* 名詞.サ変|未登録語.名詞,サ変 */
#define TERM_NOUN_A     8   /* 名詞.形動 */
#define TERM_NOUN_D     9   /* 名詞.副詞 */
#define TERM_ADJ_V      10  /* 形容動詞 */
#define TERM_VERB_N     11  /* 動詞.連用 */
#define TERM_PROPER     12  /* 名詞.固有|未登録語.名詞,固有 */
#define TERM_UNKNOWN    13  /* ？|未登録語.一般 */
#define TERM_DELIM      14  /* 記号.中点 */
#define TERM_NOUN_P     15  /* 名詞.接頭 */
#define TERM_NOUN_S     16  /* 名詞.接尾 */

//
// CLASS ModTermResource -- 検索語処理器のリソース群
//
// NOTES
//  検索語処理器のリソース群
//
class ModTermResource {
public:

	// コンストラクタ
	ModTermResource(
		const ModUnicodeString& dir);   // リソースの格納ディレクトリ

	// デストラクタ
	~ModTermResource();

	// リソース群
	ModTermParameterFile *parameterDict;  // パラメタ辞書
	ModTermWordFile *stopDict;            // 禁止辞書
	ModTermTypeFile *stopTypeDict;        // 禁止タイプ辞書
	ModTermWordFile *stopExpansionDict;   // 禁止拡張語辞書
	ModTermPatternFile *patternDict;      // パタン辞書
	ModTermTypeTable *termTypeTable;      // 検索語タイプテーブル

	// バージョン
	// 0 : パタン辞書を用いる版 (デフォルト)
	// 1 : タイプ辞書を用いる版
	int version;

protected:

private:

};

#ifndef SYD_USE_UNA_V10
// 検索語単位
typedef ModNlpNormMode ModTermUnit;
#endif

// 検索語生成法
typedef int ModTermMethod;

//
// CLASS ModTerm -- 検索語処理器
//
// NOTES
//  検索語処理器のクラス
//
class ModTerm {
	 typedef ModInvertedVector<ModInvertedDocumentID>::ConstIterator ConstIterator;
public:

	// 検索語生成法(ModTermMethod)の値
	enum {                              
		defaultMethod = 0, // デフォルト
		mgramMethod   = 1, // 形態素n-gram
		cgramMethod   = 2  // 文字n-gram
	};

	// 検索語カテゴリ (ModInvertedTermCategoryに対応)
	enum {
		ModTermCategoryUndefined = 0,           // 未定義
		ModTermCategoryEssential,               // 必須
		ModTermCategoryImportant,               // 重要
		ModTermCategoryHelpful,                 // 有用
		ModTermCategoryEssentialRelated,        // 必須関連語
		ModTermCategoryImportantRelated,        // 重要関連語
		ModTermCategoryHelpfulRelated           // 有用関連語
	};

	// コンストラクタ
	ModTerm(
		const ModTermResource* resource,   	// リソース
#ifdef SYD_USE_UNA_V10
		UNA::ModNlpAnalyzer* analyzer = 0,	// 自然言語解析器
#else
		ModNlpAnalyzer* analyzer = 0,	// 自然言語解析器
#endif
		ModBoolean isOwner = ModFalse,	// 自然言語解析器のオーナーか
		ModSize _collectionSize = 0, 	// 登録文書の総数
		double _averageLength = 0,		// 登録文書の平均長
		ModSize maxWordLen_ = 32
		);


	// デストラクタ
	~ModTerm();

	// 検索語のプールとテーブル登録
	void poolTerm(
		const ModUnicodeString&      text, // テキスト
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		const ModBoolean       useUniGram, // 単独語生成の有無
		const ModBoolean        useBiGram, // 隣接語生成の有無
		const ModBoolean      useStopDict, // 禁止辞書使用の有無
		const ModBoolean    useNormalizer, // 正規化処理の有無
#ifdef SYD_USE_UNA_V10
		const ModUnicodeString&	termUnit, // 検索語単位
#else
		const ModTermUnit		termUnit, // 検索語単位
#endif
		const ModSize			maxText,// テキストの最大長(形態素数,0なら無制限)
		ModTermPool&			pool, // 検索語プール
		ModTermTable*			table = 0); // 検索語テーブル

	// 初期検索語のプール
	void poolTerm(
		const ModUnicodeString&     text,  // 検索要求
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		ModTermPool&                pool,  // 初期検索語群
		ModTermTable*          table = 0); // 検索語テーブル

	// 初期検索語のプール (termMethod1 == defaultMethodの場合)
	void poolTermDefault(
		const ModUnicodeString&     text,  // 検索要求
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		ModTermPool&                pool,  // 初期検索語群
		ModTermTable*          table = 0); // 検索語テーブル

	// 初期検索語のプール (termMethod1 == mgramMethodの場合)
	void poolTermMgram(
		const ModUnicodeString&     text,  // 検索要求
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		ModTermPool&                pool); // 初期検索語群

	// 初期検索語のプール (termMethod1 == cgramMethodの場合)
	void poolTermCgram(
		const ModUnicodeString&     text,  // 検索要求
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		ModTermPool&                pool); // 初期検索語群

	// 拡張検索語候補のプール
	void poolTerm(
		ModTermMap&   map,         // 検索語マップ
		ModTermPool&  pool);       // 拡張語の候補群

	// 初期検索語の重みづけ
	void weightTerm(
		ModTermMap&   map,         // 検索語マップ
		ModTermPool&  pool);       // 初期検索語群

	// 拡張語の選択
	void selectTerm(
		ModTermMap&   map,         // 検索語マップ
		ModTermPool&  candidate,   // 拡張語の候補群
		ModTermPool&  pool);       // 拡張語群

	// 初期検索結果の分析
	void analyzeResult(                  
		ModTermElement&              term,  // 初期検索語
		ModInvertedSearchResult*  result); // 初期検索結果

	// 検索語のマッピング
	void mapTerm(
		const ModUnicodeString&       doc,  // シード文書
#ifdef V1_6
		const ModLanguageSet&  langSpec,    // 言語指定
#endif
		const ModSize               docID,  // シード文書番号
		ModTermMap&                   map,  // 検索語マップ
		ModTermTable*           table = 0); // 検索語テーブル

	// 検索語のマッピング(混合適合性フィードバック用)
	void mapTerm(
		const ModUnicodeString&       doc,  // シード文書
#ifdef V1_6
		const ModLanguageSet&  langSpec,    // 言語指定
#endif
		const ModSize               docID,  // シード文書番号
		ModTermMap&                   map,  // 検索語マップ
		ModTermPool&              support); // 検索語プール(支持集合)

	// 検索結果のマージ
	void mergeResult(
		ModInvertedRankingResult* x,  // 検索結果(文書番号順)
		ModInvertedRankingResult* y,  // 検索結果(文書番号順)
		ModInvertedRankingResult* z); // マージ結果(文書番号順)

	// マージランク
	void mergeRank(
		ModInvertedRankingResult* x,  // 初期検索結果(スコア順)
		ModInvertedRankingResult* y); // 拡張検索結果(スコア順)

	// 検索語プールの有効化
	void validatePool(
		ModTermPool&  pool);                // 検索語プール

	// 正規化文字列の取得 (将来はanalyzerが機能提供してほしい)
	ModUnicodeString getNormalizedString(
#ifdef V1_6
		const ModLanguageSet&  langSpec,   // 言語指定
#endif
		const ModUnicodeString& string);   // 入力文字列

	//
	// パラメタ群
	//

	// 主要パラメタ
	ModSize      maxTerm1;                // 初期検索語数の上限
	ModSize      maxTerm2;                // 拡張検索語数の上限
	ModSize      minTerm2;                // 拡張検索語数の下限
	ModSize      maxCandidate;            // 検索語候補数の上限
	ModSize      maxSeed;                 // シード文書数の上限

	// 初期検索語の生成パラメタ
	ModBoolean   useUniGram1;             // 単独語生成の有無
	ModBoolean   useBiGram1;              // 隣接語生成の有無
	ModBoolean   useStopDict1;            // 禁止辞書使用の有無
	ModBoolean   useNormalizer1;          // 正規化処理の有無
	ModBoolean   failsafe1;               // 失敗回避の有無
#ifdef SYD_USE_UNA_V10
	ModUnicodeString	termUnit1;		  // 検索語単位
#else
	ModTermUnit  termUnit1;               // 検索語単位
#endif
	ModSize      maxText1;                // 検索要求の最大長(形態素数)

	// 初期検索語のパラメタ
	double       paramScale1;             // スケールパラメタ
	double       paramWeight1;            // 重みパラメタ (ALPHA, k4)
	double       paramScore1;             // 文書スコアパラメタ (KAPPA, k1)
	double       paramLength1;            // 文書長パラメタ (LAMBDA, b)
	int          paramProximity1;         // 隣接語の近接パラメタ
	double       adaptWeight1;            // 重みパラメタ調整用 (RHO)

	// シード検索語の生成パラメタ
	ModBoolean   useUniGram2;             // 単独語生成の有無
	ModBoolean   useBiGram2;              // 隣接語生成の有無
	ModBoolean   useStopDict2;            // 禁止辞書使用の有無
	ModBoolean   useNormalizer2;          // 正規化処理の有無
	ModBoolean   failsafe2;               // 失敗回避の有無
#ifdef SYD_USE_UNA_V10
	ModUnicodeString	termUnit2;		  // 検索語単位
#else
	ModTermUnit  termUnit2;               // 検索語単位
#endif
	ModSize      maxText2;                // 各シード文書の最大長(形態素数)

	// 拡張語候補の選択パラメタ
	ModSize      minSeedDf2;              // 出現シード文書数の下限
	ModBoolean   useSingleChar2;          // 一文字語の使用の有無
	ModBoolean   useUniGramExpansion2;    // 単独語の使用の有無
	ModBoolean   useBiGramExpansion2;     // 隣接語の使用の有無

	// 拡張語のパラメタ
	double       paramScale2;             // スケールパラメタ
	double       paramWeight2;            // 重みパラメタ (ALPHA, k4)
	double       paramScore2;             // 文書スコアパラメタ (KAPPA, k1)
	double       paramLength2;            // 文書長パラメタ (LAMBDA, b)
	int          paramProximity2;         // 隣接語の近接パラメタ

	// 重みの混合パラメタ
	double       paramMixUniGram1;        // 初期単独語の重み混合パラメタ (CHI1)
	double       paramMixBiGram1;         // 初期隣接語の重み混合パラメタ (CHI2)
	double       paramMixUniGram2;        // 拡張単独語の重み混合パラメタ (CHI3)
	double       paramMixBiGram2;         // 拡張隣接語の重み混合パラメタ (CHI2)

	// 検索語の結合パラメタ
	double       paramCombineBiGram1;     // 初期隣接語の結合パラメタ (PSI)
	double       paramCombineBiGram2;     // 拡張隣接語の結合パラメタ (PSI)
	double       paramCombineUniGram2;    // 拡張単独語の結合パラメタ (XI)

	// マージランクのパラメタ
	ModSize      maxRank1;                // 初期検索結果の最大マージ数 
	ModSize      maxRank2;                // 拡張検索結果の最大マージ数 
	double       paramMixRank;            // ランクの混合パラメタ (BETA)

	// 検索語の重要度(スケール)のパラメタ
	double	   scaleUndefined;          // 追加された検索語のスケール
	double	   scaleImportant1;			// 重要な初期検索語のスケール
	double	   scaleImportant2;			// 重要な拡張検索語のスケール

	// コレクションパラメタ
	ModSize      collectionSize;          // 登録文書の総数
	double       averageLength;           // 登録文書の平均長

	// 検索語生成法パラメタ
	ModTermMethod termMethod1;            // 検索語生成法
	ModSize       maxTermLength1;         // 検索語長の上限

	// 自然言語解析器
#ifdef SYD_USE_UNA_V10
	UNA::ModNlpAnalyzer* analyzer;
#else
	ModNlpAnalyzer* analyzer;
#endif
	ModBoolean		isOwner;				// オーナーか

	// リソース
	const ModTermResource* resource;

	// Pattern
	ModVector<ModTermPattern> patternSet;

#ifdef SYD_USE_UNA_V10
	// UNAのパラメータ
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
		m_cUnaParam;
#endif

protected:

private:
	void parseNormMode(const ModUnicodeString& termUnit);
};

#endif // __ModTerm_H__

//
// Copyright (c) 2000, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
