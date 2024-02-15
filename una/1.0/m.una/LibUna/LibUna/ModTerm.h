// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
//  ModTerm.h    -- ModTerm    の宣言
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

#ifndef __ModTerm_H__
#define __ModTerm_H__

#include "ModUnicodeString.h"
#include "ModMap.h"
#include "Module.h"

#include "ModNLPLocal.h"
#include "LibUna/ModTermStringFile.h"

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
#define TERM_NOUN_S     16  /* 名詞.接尾 */

_UNA_BEGIN

//
// CLASS ModTermResource -- 名詞句処理器のリソース群
//
// NOTES
//  名詞句処理器のリソース群
//
class UNA_LOCAL_FUNCTION ModTermResource {
public:
	// コンストラクタ
	ModTermResource(
		const ModUnicodeString& dir);   // リソースの格納ディレクトリ

	// デストラクタ
	~ModTermResource();

	// リソース群
	ModTermParameterFile *parameterDict;  // パラメタ辞書
	ModTermWordFile *stopDict;            // 禁止辞書
	ModTermWordFile *stopRepreDict;       // 禁止表現辞書
	ModTermTypeTable *termTypeTable;      // タイプテーブル
	ModTermTypeFile *stopTypeDict;        // 禁止タイプ辞書

	// バージョン
	// 0 : パタン辞書を用いる版 (デフォルト)
	// 1 : タイプ辞書を用いる版
	int version;
};

//
// CLASS ModTerm -- 名詞句処理器
//
// NOTES
//  名詞句処理器のクラス
//
class UNA_LOCAL_FUNCTION ModTerm {
public:

	// コンストラクタ
	ModTerm(
		const ModTermResource* resource,   	// リソース
		ModNlpLocalAnalyzer* _analyzer,		// 自然言語解析器
		ModSize maxWordLen_ = 32);

	// デストラクタ
	~ModTerm();

	// 検索語のプールと登録
	// 初期検索語のプール
	ModBoolean getTerm(
		ModUnicodeString&				normalized_,
		ModUnicodeString&				original_,
		double&							npCost_,
		ModVector<ModUnicodeString>*	normVector_,
		ModVector<ModUnicodeString>*	origVector_,
		ModVector<int>*					posVector_);

	// プールした名詞句のリセット
	void resetTerm();

	// 解析文字列のセット
	void setStrTarget(ModUnicodeString strTarget_);

	// 解析文字列の取得
	ModUnicodeString getStrTarget();

	// 名詞句の登録
	void insertTerm(const ModTermElement& element);

	//
	// パラメタ群
	//

	// 主要パラメタ
	ModSize		maxCandidate;			// 検索語候補数の上限

	// 名詞句の生成パラメタ
	ModBoolean	useStopDict1;			// 禁止辞書使用の有無
	ModBoolean	failsafe1;				// 失敗回避の有無
	ModSize		maxText1;				// 検索要求の最大長(形態素数)
	ModBoolean	useStopTypeDict1;		// 禁止タイプ辞書使用の有無
	ModSize		calcCostMode;			// 名詞句コストの算出モード
	ModSize		costWeight1;			// 名詞句コスト算出の重み1
	ModSize		costWeight2;			// 名詞句コスト算出の重み2

	// 自然言語解析器
	ModNlpLocalAnalyzer* analyzer;

	// リソース
	const ModTermResource* resource;

protected:

private:
	// NP候補
	ModTermPool candidate;
	ModTermPool::ConstIterator candidateIte;

	ModUnicodeString strTarget;
};

_UNA_END

#endif // __ModTerm_H__

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
