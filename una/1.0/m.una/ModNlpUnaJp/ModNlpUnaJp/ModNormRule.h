// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormRule.h -- ModNormRule のクラス定義
// 
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModNormRule_H_
#define __ModNormRule_H_

#include "ModCommonDLL.h"
#include "ModHashMap.h"
#include "ModNlpUnaJp/ModNormDLL.h"
#include "ModNlpUnaJp/ModNormType.h"
#include "ModNlpUnaJp/ModNormChar.h"
#include "ModNlpUnaJp/Module.h"
_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModNormalizer;

//
// CONST
// ModNormDecompMapMax -- 分解マップの最大エントリ数
//
// NOTES
// 分解マップの最大エントリ数
//
const ModSize ModNormDecompMapMax = 256;

//
// CONST
// ModNormSurrogateMapMax -- サロゲート文字マップの最大エントリ数
//
// NOTES
// 分解マップの最大エントリ数
//
const ModSize ModNormSurrogateMapMax = 256;

//
// CLASS
// ModNormRule -- Unicode 異表記正規化ルールクラス
//
// NOTES
// 異表記正規化のルールのためのクラス。
//
class ModNormDLL ModNormRule : public ModDefaultObject
{
	friend class ModNormalizer;
public:
	// コンストラクタ(for ModUnicodeString)
	ModNormRule(const ModUnicodeString& dataDirPath,
				const ModBoolean english = ModFalse);

	ModNormRule(const ModUnicodeString& ruleDicPath,
				const ModUnicodeString& ruleAppPath,
				const ModUnicodeString& expandDicPath,
				const ModUnicodeString& expandAppPath,
				const ModUnicodeString& connectTblPath,
				const ModUnicodeString& unknownTblPath,
				const ModUnicodeString& unknownCostPath,
				const ModUnicodeString& normalTblPath,
				const ModUnicodeString& preMapPath,
				const ModUnicodeString& postMapPath,
				const ModUnicodeString& combiMapPath,
				const ModBoolean english = ModFalse);

	// コンストラクタ(for ModCharString : backward compatibility)
	ModNormRule(const ModCharString& dataDirPath,
				const ModBoolean english = ModFalse);

	ModNormRule(const ModCharString& ruleDicPath,
				const ModCharString& ruleAppPath,
				const ModCharString& expandDicPath,
				const ModCharString& expandAppPath,
				const ModCharString& connectTblPath,
				const ModCharString& unknownTblPath,
				const ModCharString& unknownCostPath,
				const ModCharString& normalTblPath,
				const ModCharString& preMapPath,
				const ModCharString& postMapPath,
				const ModCharString& combiMapPath,
				const ModBoolean english = ModFalse);
	// デストラクタ
	~ModNormRule();

	// 既存のNormRuleと比較して、mapだけのデータを保持するように変更
	ModBoolean minimize(const ModNormRule* normRule);

	// リソースを直接返す(for Highlighter)
	// 合字マップ
	const ModNormCombiMap* getCombiMap() const;
	
	// 前処理マップ
	const ModUnicodeChar* getPreMap() const;
	
	// 後処理マップ
	const ModUnicodeChar* getPostMap() const;
	
	// 前処理用分解マップ
	const ModNormChar* getPreDecompMap() const;

	// 文字列展開データがロードされたか否かを返す
	ModBoolean isExpStrDicLoad();
	ModBoolean isExpStrStrDicLoad();
	ModBoolean isExpStrMorDicLoad();

private:
	void load(const ModUnicodeString& ruleDicPath,
			  const ModUnicodeString& ruleAppPath,
			  const ModUnicodeString& expandDicPath,
			  const ModUnicodeString& expandAppPath,
			  const ModUnicodeString& connectTblPath,
			  const ModUnicodeString& unknownTblPath,
			  const ModUnicodeString& unknownCostPath,
			  const ModUnicodeString& normalTblPath,
			  const ModUnicodeString& preMapPath,
			  const ModUnicodeString& postMapPath,
			  const ModUnicodeString& combiMapPath,
			  const ModUnicodeString& metaDefPath,
			  const ModUnicodeString& expStrWrdDicPath = "",
			  const ModUnicodeString& expStrAppDicPath = "");

	void loadMap(const ModUnicodeString& fname,
	             ModUnicodeChar* map, ModNormSurrogateChar* surrogate, ModNormChar* decomp);
	void loadCombiMap(const ModUnicodeString& fname);
	void loadDecompMap(const ModUnicodeString& fname);

	char* ruleDic;				// 正規化ルール: UNA用辞書のイメージ
	char* ruleApp;				// 正規化ルール: UNA用アプリ情報のイメージ
	char* expandDic;			// 展開ルール: UNA用辞書のイメージ
	char* expandApp;			// 展開ルール: UNA用アプリ情報のイメージ
	char* connectTbl;			// UNA接続表のイメージ
	char* unknownTbl;			// UNA未登録語表のイメージ
	char* unknownCost;			// UNA未登録語コストのイメージ
	char* normalTbl;			// UNA文字列標準化表のイメージ
	char* expStrWrdDic;  		// expanding data: Image of the dictionary for UNA
	char* expStrAppDic; 		// expanding data: Image of the application information for UNA

	ModNormCombiMap* combiMap;	// 合字マップ
	ModUnicodeChar* preMap;		// 前処理マップ
	ModUnicodeChar* postMap;	// 後処理マップ
	ModNormSurrogateChar* preSurrogateMap;	// 前処理用サロゲート文字マップ
	ModNormSurrogateChar* postSurrogateMap;	// 後処理用サロゲート文字マップ
	ModNormChar* preDecompMap;	// 前処理用分解マップ
	ModNormChar* postDecompMap;	// 後処理用分解マップ
	ModBoolean normalizeEnglish; // English processing mode
	ModBoolean useMetaDef;		// メタ文字の定義を使うか否か

	// データディレクトリにおけるデフォルトのファイル名
	//
	static const ModUnicodeString ruleDicName;
	static const ModUnicodeString ruleAppName;
	static const ModUnicodeString expandDicName;
	static const ModUnicodeString expandAppName;
	static const ModUnicodeString connectTblName;
	static const ModUnicodeString unknownTblName;
	static const ModUnicodeString unknownCostName;
	static const ModUnicodeString normalTblName;
	static const ModUnicodeString preMapName;
	static const ModUnicodeString postMapName;
	static const ModUnicodeString combiMapName;
	static const ModUnicodeString metaDefName;
	static const ModUnicodeString expStrStrWrdDicName;
	static const ModUnicodeString expStrStrAppDicName;
	static const ModUnicodeString expStrMorWrdDicName;
	static const ModUnicodeString expStrMorAppDicName;

	// The state of the load of user expanding data
	ModBoolean expStrDicLoad;
	ModBoolean expStrStrDicLoad;
	ModBoolean expStrMorDicLoad;

	ModBoolean isMinimize; // チェック用
};

_UNA_UNAJP_END
_UNA_END

#endif // __ModNormRule_H_
//
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
