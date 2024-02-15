// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLP.h -- Definition of language processing class
// 
// Copyright (c) 2001-2010, 2023 Ricoh Company, Ltd.
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
#ifndef	__UNA_UNAJP_MODNLPO_H
#define __UNA_UNAJP_MODNLPO_H

#include "ModCommonDLL.h"
#include "ModConfig.h"
#include "ModDefaultManager.h"
#include "ModVector.h"
#include "ModLanguageSet.h"
#include "ModAutoPointer.h"
#include "ModCharString.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModUNA.h"
#include "LibUna/ModWordStemmer.h"
#include "ModNlpUnaJp/ModNormalizer.h"
#include "ModNlpUnaJp/Module.h"
#include "ModNlpUnaJp/ModNlpExpStr.h"
#include "LibUna/ModTerm.h"

_UNA_BEGIN

class ModWordStemmer;

_UNA_UNAJP_BEGIN

// declare necessary class
class ModUnaAnalyzer;
class ModUnaResource;
class ModNlpAnalyzerX;
class ModNlpResourceX;


//  ENUM
//  ModNlpExpMode --  展開モード
//
//  NOTES
//      ModNlpAnalyzer::getExpand() の引数として利用される。
//
//      ModNlpExpChkOrigStr -- 別の展開文字列を含む展開文字列を省略する
//      ModNlpExpNoChk   -- 省略しない~(デフォルト)
//
enum ModNlpExpMode
{
	ModNlpExpChkOrigStr,	// for expandBuf()
	ModNlpExpNoChk		// default
};


//  ENUM
//  ModNlpNormMode --  正規化モード
//
//  NOTES
//      ModNlpAnalyzer::set() の引数として利用される。
//
//      ModNlpNormOnly       -- 正規化のみを実施する
//      ModNlpNormStemDiv    -- 正規化,stemming,下位構造分割
//      ModNlpNormDiv        -- 正規化,下位構造分割
//      ModNlpNormStem       -- 正規化,stemming
//      ModNlpNormRet        -- 正規化,改行を含む単語検出
//      ModNlpNormRetStemDiv -- 正規化,改行を含む単語検出,stemming,下位構造分割
//      ModNlpNormRetDiv     -- 正規化,改行を含む単語検出,下位構造分割
//      ModNlpNormRetStem    -- 正規化,改行を含む単語検出,stemming
//
enum ModNlpNormMode
{
	ModNlpNormOnly = 0,  // default
	ModNlpNormStemDiv,
	ModNlpNormDiv,
	ModNlpNormStem,
	ModNlpNormRet,
	ModNlpNormRetStemDiv,
	ModNlpNormRetDiv,
	ModNlpNormRetStem
};

//  ENUM
//  ModNlpAreaTreatment --  正規化対象のエリアごとの扱い
//
//  NOTES
//      ModNlpNormModifier.xxxx()の引数として利用される。
//		xxxx は対象エリアを示す
//      ModNlpAsIs        -- 何もしない
//      ModNlpNoNormalize -- 対象エリアの文字を正規化しない
//      ModNlpDelete      -- 対象エリアの文字を削除する
//      ModNlpReset       -- 初期の扱いに戻す
//
enum ModNlpAreaTreatment
{
	ModNlpAsIs = 0,  // default
	ModNlpNoNormalize,
	ModNlpDelete,
	ModNlpReset
};

//
// CLASS
// ModNlpNormArea -- 正規化する文字コードエリアの動作定義
//
// NOTES
// 動作定義を提供するクラス
//
class UNA_UNAJP_FUNCTION ModNlpNormModifier : public ModDefaultObject
{
	friend class ModNlpAnalyzerX;
public:
	ModNlpNormModifier();
	~ModNlpNormModifier();
	void space(ModNlpAreaTreatment x);
private:
	int areaData;
};

//
// CLASS
// ModNLPAnalyzer -- UNA 解析器クラス（表記正規化機能付き）
//
// NOTES
// 言語解析機能を提供するクラス。
//
class UNA_UNAJP_FUNCTION ModNlpAnalyzerX : public ModDefaultObject
{
public:

	// NOTE コンストラクタ
	ModNlpAnalyzerX(
		const ModNlpResourceX* resource_,	// リソース
		DicSet	*dicSet_ = 0,
		ModSize maxWordLen_=0);				// 最大単語長指定(0は指定無し)

	// NOTE	デストラクタ
	virtual ~ModNlpAnalyzerX();

	// NOTE	処理対象テキストのセット2
	void set(
		const ModUnicodeString& target_, 		// 処理対象テキスト
		const ModNlpNormMode mode_,				// 動作モード
		const ModLanguageSet& lang_,			// 言語指定
		const ModNlpNormModifier& modify_ = ModNlpNormModifier());

	// It is confirmed whether the data of the development function set by the parameter is loaded.
	// If the data was loaded, if ModTrue, it is not, ModFalse
	ModBoolean isExpStrDicLoad();
	ModBoolean isExpStrStrDicLoad();
	ModBoolean isExpStrMorDicLoad();

	// NOTE   形態素解析結果１単語を異表記正規化し取得する
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getWord(
		ModUnicodeString &normalized,   // 異表記正規化済み表記
		ModUnicodeChar* &original,      // 原表記開始位置へのポインタ
		ModSize &len,                   // 原表記の長さ
		int &pos);                      // 形態素品詞番号

	// NOTE   形態素解析結果１単語を取得する
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getWord(
		ModUnicodeString& result,		// 異表記正規化済み表記
		const ModBoolean normalize);	// 異表記正規化有無の指定
		
	// NOTE   形態素解析結果１単語を原表記付きで取得する
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getWord(
		ModUnicodeString& result,		// 異表記正規化済み表記
		ModUnicodeString& original,		// 原表記
		const ModBoolean normalize);	// 異表記正規化有無の指定
		
	// NOTE   形態素解析結果１単語を表記展開を行いつつ取得する
	//        呼ぶ側で、delete [] expanded_;を実施すること
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModSize getExpandWords(
		ModUnicodeString*& expanded, 	// 展開表記の配列
		ModUnicodeString& original_,
		int& pos_,
		const ModNlpExpMode chkOrg = ModNlpExpNoChk);
										// 展開モードの指定
										//  ModNlpExpChkOrigStr 
										//	-- 別の展開文字列を含む
										//     展開文字列を省略する
										//  ModNlpExpNoChk
										//  -- 省略しない(デフォルト)

	// NOTE   解析結果のブロック取得(for ModTermVersion 1)
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getBlock(
		ModUnicodeString& result, 			// 解析結果の文字列
		const ModUnicodeChar separator1,	// フィールド区切り
		const ModUnicodeChar separator2,	// レコード区切り
		const ModBoolean normalize);		// 異表記正規化の有無

	// NOTE   解析結果のブロック取得(for ModTermVersion 2)
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getBlock(
		ModVector<ModUnicodeString>& formVector,	// 解析結果表記のベクター
		ModVector<int>& posVector,					// 解析結果品詞のベクター
		const ModBoolean normalize);				// 異表記正規化の有無

	// NOTE   解析結果のブロック取得 原表記付き(for ModTermVersion 2)
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getBlock(
			ModVector<ModUnicodeString>& formVector_,	// 解析結果表記のベクター
			ModVector<ModUnicodeString>& ostrVector_,	// 原表記のベクター
			ModVector<int>& posVector_,					// 解析結果品詞のベクター
			const ModBoolean normalize_,				// 異表記正規化の有無
			ModVector<int> *costVector_ = 0,
			ModVector<int> *uposVector_ = 0,
			ModBoolean ignore = ModTrue);

	// NOTE   解析結果の辞書ベース名取得
	// RETURN 解析対象テキストが残っていればModTrue, なければModFalse
	ModBoolean getDicName(
		ModVector<ModUnicodeString>& dicNameVector_);	// 辞書ベース名のベクター

	void getNormalizeBuf(const ModUnicodeString& cstrInputStr_,
						 ModUnicodeString& cstrOutputStr_,
						 ModSize ulLenOfNormalization_);

	ModSize getExpandBuf(const ModUnicodeString& cstrInputStr_,
						 ModUnicodeString*& pcstrExpanded_,
						 ModSize ulLenOfNormalization_,
						 ModBoolean eExpandOnly_);

	ModSize getExpandStrings(ModVector<ModUnicodeString>& expanded_, ModSize maxExpPatternNum_);
	ModSize getExpandStrings(const ModUnicodeString& cstrInputStr_, 
							 ModVector<ModUnicodeString>& expanded_,
							 ModSize maxExpPatternNum_);

	ModTermResource* getTermResource();

private:
// functions

	void setLanguage(const ModLanguageSet&, ModNlpAreaTreatment);

	// analyzer for UNA
	ModUnaAnalyzer*			unaAnalyzer;
    ModUnaMiddleAnalyzer*	jakanaAnalyzer;	// for ja_kana
	ModNormalizer*			subAnalyzer;
	ModWordStemmer*			defaultStemmer;
	ModWordStemmer*			optionalStemmer;
	ModNlpExpStr*			expStr;

	// pointer to the nlp resource object (dictionaries)
	const ModNlpResourceX*		resource;

	ModNlpAreaTreatment normModify;
	ModLanguageSet languageSetOnSettingDocument;

	ModBoolean     skipHighSpeed;
};


//
// CLASS
// ModNlpResourceX -- NLP 資源クラス
//
// NOTES
// 言語解析器が使用する資源を保持する。
//
class UNA_UNAJP_FUNCTION ModNlpResourceX : public ModDefaultObject
{
	friend class ModNlpAnalyzerX;

public:

	// NOTE コンストラクタ1
	ModNlpResourceX(
		const ModUnicodeString&,	// リソースディレクトリへのパス 
		const ModLanguageSet&,		// 言語指定
		ModSize reduceMemory);		// 省メモリ指定
						// 0: 省メモリしない
						// 1: 低速動作(省メモリ)
						// 2: 低速動作&必要時辞書ロード

	// NOTE デストラクタ
	virtual ~ModNlpResourceX();

	const ModLanguageSet& getLanguage() const
		{ return languageSetForNlpResource; }

private:
	void setResource(const ModUnicodeString&,
					 const ModLanguageSet&,
					 ModSize memRedudeLevel);

	// check supported language
	ModBoolean checkLanguage(const ModLanguageSet& cLang_) const;

	ModLanguageSet languageSetForNlpResource;

	// この実装は多言語対応しにくいので、対応が増えたら再構成必要
	ModUnaResource*		unaResource;
	ModNormRule*		normRule;
	ModWordStemmer*		engWordStemmer;
	ModTermResource*	termResource;

	ModBoolean			memSwitch;

	ModBoolean 		skipHighSpeedSwitch;

	// Is Japanese processing possible?
	ModBoolean		canProcessJapanese;
	// Is Chinese processing prossible?
	ModBoolean		canProcessChinese;
	// Identify Hiraoka to Katakana?
	ModBoolean		canIdentifyingHiraganaKatakana;
};

_UNA_UNAJP_END
_UNA_END

#endif // __UNA_UNAJP_MODNLPO_H

//
// Copyright (c) 2001-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
