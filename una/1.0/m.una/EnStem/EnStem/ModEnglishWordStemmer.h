// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModEnglishWordStemmer.h -- 英単語正規化器
// 
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModEnglishWordStemmer_H__
#define __ModEnglishWordStemmer_H__

#include "ModOs.h"
#include "ModFile.h"
#include "ModSerial.h"
#include "ModArchive.h"
#include "ModAlgorithm.h"
#include "ModVector.h"
#include "ModCharTrait.h"
#include "ModCharString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"
#include "ModError.h"
#include "ModMessage.h"
#include "ModOstrStream.h"
#include "ModManager.h"
#include "ModException.h"
#include "ModCommonDLL.h"
#include "EnStem/Module.h"

#include "LibUna/ModStemDLL.h"
#include "LibUna/ModWordStemmer.h"

_UNA_BEGIN
_UNA_ENSTEM_BEGIN

//
// CONST
//  ModEnglishWordStemmerInitCharNum -- 見出し先頭文字数
//
// NOTES
//  見出し語形先頭文字の異なり数（小文字アルファベット26文字）
//
const ModSize ModEnglishWordStemmerInitCharNum = 26;


//
// STRUCT
// 	ModEnglishWordStemmerDataPath -- 辞書・規則ソースファイルへのパス
//
// NOTES
//  辞書・規則データの各ソースファイルへのパスを表す構造体
//
struct ModXXStemDLL ModEnglishWordStemmerDataPath
	: public ModSerializer
{
    ModEnglishWordStemmerDataPath(const ModUnicodeString& filename);
    ModEnglishWordStemmerDataPath(const ModCharString& filename);
    void serialize(ModArchive& archiver);

	ModUnicodeString dictKeyPath;		// 辞書見出し語形ファイルへのパス
	ModUnicodeString dictExpandPath;	// 辞書展開語形ファイルへのパス
	ModUnicodeString dictIndexPath;	// 辞書インデックスファイルへのパス
	ModUnicodeString ruleKeyPath;		// 規則見出し語形ファイルへのパス
	ModUnicodeString ruleStemPath;		// 規則正規化語形ファイルへのパス
	ModUnicodeString ruleIndexPath;	// 規則インデックスファイルへのパス
};

//
// CLASS
//  ModEnglishWordStemmer -- 英単語正規化器
//
// NOTES
//  正規化器の本体。以下の処理を行なう
//      look
//          辞書引き（見出し登録されているかどうかの確認）
//      stem
//          正規化
//      expand
//          正規化＆展開
//

class ModXXStemDLL ModEnglishWordStemmer
	: public ModSerializer,public ModWordStemmer
{
public:
	ModEnglishWordStemmer(
		const ModEnglishWordStemmerDataPath& path,
		const ModUnicodeString& filename);
	ModEnglishWordStemmer(
		const ModEnglishWordStemmerDataPath& path,
		const ModCharString& filename);
	ModEnglishWordStemmer(const ModUnicodeString& filename);
	ModEnglishWordStemmer(const ModCharString& filename);
	ModEnglishWordStemmer();
	~ModEnglishWordStemmer();

	// 辞書引き
	ModBoolean look(const ModUnicodeString& target);

	// 正規化
	ModBoolean stem(const ModUnicodeString& target,
					ModUnicodeString& result);
	// 正規化＆展開
	ModBoolean expand(const ModUnicodeString& target,
					  ModVector<ModUnicodeString>& result);

private:
	void
	setResource(
		const ModEnglishWordStemmerDataPath& path,
		const ModUnicodeString& filename);
    static const char SepRecord;	// レコード区切り('\n')
    static const char SepField;		// フィールド区切り(' ')
    static const char FirstChar;	// アルファベット先頭文字('a')
    static const char LastChar;		// アルファベット末尾文字('z')
    static const char* DataVerifier;	// データ認識文字列

	// 辞書インデックス
	struct DictIndex {
		DictIndex()
			: dictKeyOffset(-1), dictStemOffset(-1),
			  dictExpandOffset(-1) {}
		int dictKeyOffset;		// 見出し語形オフセット
		int dictStemOffset;		// 正規化語形オフセット
		int dictExpandOffset;	// 展開語形オフセット
	};

	// 規則インデックス
	struct RuleIndex {
		RuleIndex()
			: ruleKeyOffset(-1), ruleStemOffset(-1) {}
		int ruleKeyOffset;		// 見出し語形オフセット
		int ruleStemOffset;		// 正規化語形オフセット
	};

	// The first letter index
	struct InitCharIndex {
		InitCharIndex()
            : headIndex(-1), tailIndex(-1), minKeyLen(0) {}
		int headIndex;			// 先頭位置
		int tailIndex;			// 末尾位置
        int minKeyLen;      	// 見出し長最小値
	};

	char*	dictKey;			// 辞書見出し語形文字列
	char*	dictExpand;			// 辞書展開語形文字列

	ModUnicodeChar* ruleKey;	// 規則見出し語形文字列
	ModUnicodeChar* ruleStem;	// 規則正規化語形文字列

	int dictKeyLen;				// 辞書見出し語形文字列長
	int dictExpandLen;			// 辞書展開語形文字列長
	int ruleKeyLen;				// 規則見出し語形文字列長
	int ruleStemLen;			// 規則正規化語形文字列長

	int dictKeyNum;				// 辞書見出し語形数
	int ruleKeyNum;				// 規則見出し語形数

	DictIndex* dictIndex;		// 辞書インデックス配列
	RuleIndex* ruleIndex;		// 規則インデックス配列

	// 先頭文字インデックス配列
	InitCharIndex dictInit[ModEnglishWordStemmerInitCharNum];
	InitCharIndex ruleInit[ModEnglishWordStemmerInitCharNum];

	// シリアライザ
    void serialize(ModArchive& archiver);

	// データ設定関数
	void setDictKey(ModArchive& archiver);
	void setDictExpand(ModArchive& archiver);
	void setRuleKey(ModArchive& archiver);
	void setRuleStem(ModArchive& archiver);
	void setDictIndex(ModArchive& archiver);
	void setRuleIndex(ModArchive& archiver);

	// 検索関数
	DictIndex* searchDict(const ModUnicodeString& target);
	RuleIndex* searchRule(const ModUnicodeString& target);

	// 文字列比較関数
	int compare(const ModUnicodeChar*, const ModUnicodeChar*);
	int compare(const ModUnicodeChar*, const char*);
	int compare(const char*, const ModUnicodeChar*);
	int compare(const char*, const char*);

	// 処理対象文字列のチェック
	ModBoolean verifyTarget(const ModUnicodeString&);
	// 展開語形の分解
	void breakExpand(char*, ModVector<ModUnicodeString>&);
};
_UNA_ENSTEM_END
_UNA_END
#endif	// __ModEnglishWordStemmer_H__
//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
