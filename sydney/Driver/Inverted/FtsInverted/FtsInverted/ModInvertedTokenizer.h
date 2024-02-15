// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedTokenizer.h -- 分割処理器の定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedTokenizer_H__
#define __ModInvertedTokenizer_H__

// MACRO
// UNA_V3_3
//
// NOTES
// これを定義すると UNA のバージョンが 3.3 としてコンパイルされる。
// conf で定義することとしたので、ここで定義することはない。
//
// #define UNA_V3_3

class ModUnicodeString;
class ModCharString;

#include "ModOs.h"
#include "ModVector.h"
#include "ModMap.h"
#include "ModAlgorithm.h"
#include "ModSerial.h"
#include "ModInvertedManager.h"
#include "ModInvertedTypes.h"
#include "ModInvertedSmartLocationList.h"
#include "ModTerm.h"
#ifdef V1_6
#include "ModLanguageSet.h"
#endif // V1_6

#ifndef SYD_INVERTED	// 定義されてなかったら
class ModInvertedFile;
#endif

#ifdef SYD_USE_UNA_V10
namespace UNA {
class ModNlpAnalyzer;
}
#else
class ModNormalizer;
class ModEnglishWordStemmer;
class ModUnaAnalyzer;
class ModNlpAnalyzer;
#endif

#ifdef SYD_INVERTED
#include "ModUnicodeString.h"
class ModInvertedFile;
#endif



// ModInvertedTokenizer で ModVector<ModInvertedLocationListMap> を使いたいが
// SPARC C++ V4.2 ではテンプレートのネストが深すぎるらしく正しく処理できない。
// 上記クラスを用いる場合には以下の define を有効にすればよい。
// #define INV_TOK_USEVECTOR

// #define INV_TOK_USETOKEN


#ifdef INV_TOK_USETOKEN

#ifndef SYD_INVERTED	// 定義されてなかったら
#include "ModUnicodeString.h"
#endif

//
// CLASS
// ModInvertedToken -- トークン
//
// NOTES
// 分割単位であるトークンを表すクラス。
// 単語単位の分割のために、長さを独利したメンバーに持つ。
//
class
ModInvertedToken : public ModInvertedObject
{
public:
	ModInvertedToken() {}
	ModInvertedToken(const ModInvertedToken& token);
		: string(token.string), length(token.length) {}
	ModInvertedToken(const ModUnicodeString& string_,
					 const ModSize length_ = 0);
		: string(string_), length(length_) {}

	const ModUnicodeString& getString() const { return string; }
	ModSize getLength() const { return length; }
	void setLength(const ModSize length_) { length = length_; }

	ModBoolean operator<(const ModInvertedToken& token) const {
		return string < token.string; }
	ModBoolean operator>(const ModInvertedToken& token) const {
		return string > token.string; }
	ModBoolean operator<=(const ModInvertedToken& token) const {
		return string <= token.string; }
	ModBoolean operator>=(const ModInvertedToken& token) const {
		return string >= token.string; }
	ModBoolean operator==(const ModInvertedToken& token) const {
		return string == token.string; }

protected:
	ModUnicodeString string;
	ModSize length;
};

typedef ModMap<ModInvertedToken, ModInvertedSmartLocationList,
	ModLess<ModInvertedToken> > ModInvertedLocationListMap;

#else

typedef ModMap<ModUnicodeString, ModInvertedSmartLocationList,
	ModInvertedSimpleLess > ModInvertedLocationListMap;

#endif


//
// CLASS
// ModInvertedTokenOccurrence -- トークンの出現
//
// NOTES
// トークンの各出現に関する情報を表現する基本クラス。文書の先頭からの文字単位
// のオフセットのみを持つ。
// 具体的な分割器が文字単位のオフセット以外の情報を返す場合には、このサブ
// クラスを適宜作成すること。
// 
class
ModInvertedTokenOccurrence : public ModInvertedObject
{
public:
	ModInvertedTokenOccurrence();
	virtual ~ModInvertedTokenOccurrence() {}

	void setOffset(const ModSize);
	ModSize getOffset() const;
protected:
	// 文字単位のオフセット
	ModSize offset;
};


//
// FUNCTION
// ModInvertedTokenOccurrence::ModInvertedTokenOccurrence -- コンストラクタ
//
// NOTES
// パラメータのコンストラクタ。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline
ModInvertedTokenOccurrence::ModInvertedTokenOccurrence()
	: offset(0)
{}


//
// FUNCTION
// ModInvertedTokenOccurrence::setOffset -- オフセットの設定
//
// NOTES
// 文字単位のオフセットを設定する。
//
// ARGUMENTS
// const ModSize offset_
//		オフセット
//
// RETURN
// なし
// 
// EXCEPTIONS
// なし
//
inline void
ModInvertedTokenOccurrence::setOffset(const ModSize offset_)
{
	offset = offset_;
}


//
// FUNCTION
// ModInvertedTokenOccurrence::getOffset -- オフセットの取得
//
// NOTES
// 文字単位のオフセットを取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// オフセット
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedTokenOccurrence::getOffset() const
{
	return offset;
}


//
// CLASS
// ModInvertedQueryTokenizedResult -- テキスト分割結果
//
// NOTES
// 検索語に対するテキスト分割結果のクラス。
//
class
ModInvertedQueryTokenizedResult : public ModInvertedObject
{
public:
	ModInvertedQueryTokenizedResult() {}
	ModInvertedQueryTokenizedResult(const ModInvertedQueryTokenizedResult& orig) :
		target(orig.target), locationListMap(orig.locationListMap),
		tokenizedEnd(orig.tokenizedEnd), shortWord(orig.shortWord),
		fromWord(orig.fromWord), toWord(orig.toWord),
		isNormalWord(orig.isNormalWord)
	{}
	ModUnicodeString target;
	ModInvertedLocationListMap locationListMap;
	ModSize tokenizedEnd;
	ModUnicodeString shortWord;
	ModUnicodeString fromWord;
	ModUnicodeString toWord;
	ModBoolean isNormalWord;
};


//
// CLASS
// ModInvertedTokenizer -- テキスト分割器
//
// NOTES
// テキスト分割器のベースクラス。
// 具体的な分割器は、このクラスを継承して作成すること。
//
class
ModInvertedTokenizer : public ModInvertedObject
{
public:
	// 分割モード
	enum TokenizeMode { 
		document				= 0x0000,
		query					= 0x0001,
		simpleQuery				= 0x0002,
		baseModeMask			= 0x000f,
		wordIndexingOnly		= 0x0010,
		ngramIndexingOnly		= 0x0020,
		indexingMask			= 0x00f0,
		skipNormalizing			= 0x1000,
		skipExpansion			= 0x2000,
		skipNormalizingDocument	= skipNormalizing|document,
		skipNormalizingQuery	= skipNormalizing|query,
		skipNormalizingSimple	= skipNormalizing|simpleQuery,
		skipExpansionQuery		= skipExpansion|query,
		skipExpansionSimple		= skipExpansion|simpleQuery,
		normalizingMask			= 0x0fff
	};

	// 検索用分割結果
	typedef ModInvertedQueryTokenizedResult QueryTokenizedResult;

	// 各種リソース生成関数
#ifdef SYD_USE_UNA_V10
	typedef UNA::ModNlpAnalyzer* GetAnalyzer(const ModUInt32);
	virtual UNA::ModNlpAnalyzer* getAnalyzer(const ModCharString&)
		{ return 0; }
	typedef UNA::ModNlpAnalyzer* GetNormalizer(const ModUInt32);
	virtual UNA::ModNlpAnalyzer* getNormalizer(const ModCharString&)
		{ return 0; }
#else
	typedef ModNormalizer* GetNormalizer(const ModUInt32);
	virtual ModNormalizer* getNormalizer(const ModCharString&) { return 0; }

#if defined(UNA_V3_3) || defined(UNA_V3_4)
	typedef ModNlpAnalyzer* GetAnalyzer(const ModUInt32);
	virtual ModNlpAnalyzer* getAnalyzer(const ModCharString&) { return 0; }
#else
	typedef ModUnaAnalyzer* GetAnalyzer(const ModUInt32,
										ModNormalizer*,
										ModEnglishWordStemmer*);
	virtual ModUnaAnalyzer* getAnalyzer(const ModCharString&) { return 0; }
#endif

	typedef ModEnglishWordStemmer* GetStemmer(const ModUInt32);
	virtual ModEnglishWordStemmer* getStemmer(const ModCharString&) { return 0; }
#endif

	// 各種リソース指定文字列/生成関数設定用関数
	static const char* normalizerResourceID;
	static void setGetNormalizer(GetNormalizer*);

	static const char* analyzerResourceID;
	static void setGetAnalyzer(GetAnalyzer*);
	static GetAnalyzer* getGetAnalyzer() { return getAnalyzerFunc; }

#ifndef SYD_USE_UNA_V10
	static const char* stemmerResourceID;
	static void setGetStemmer(GetStemmer*);
#endif

	// パラメータにあった分割器を生成する
	static ModInvertedTokenizer* create(const ModCharString&,
										ModInvertedFile*,
										const ModBoolean = ModFalse,
										const ModBoolean = ModTrue,
										const ModInvertedUnaSpaceMode
												= ModInvertedUnaSpaceAsIs,
										const ModBoolean = ModFalse,
										ModSize = 0,
										ModSize = 10);

	// デストラクタ
	virtual ~ModInvertedTokenizer() {}

	// 分割 -- 登録時に使用する
	virtual void tokenize(
		const ModUnicodeString&,				// 解析対象
		const TokenizeMode,						// 分割モード
		ModInvertedLocationListMap&,			// 分割結果
		ModSize&,								// 終了位置
		const ModVector<ModSize>*,				// セクション末尾のバイト位置
		ModVector<ModSize>*,					// セクション末尾の文字位置
		ModVector<ModLanguageSet>*,				// セクションの言語
		ModInvertedFeatureList*,				// 特徴語
		const ModTermResource*);				// TermResource

	// 分割 -- 検索時に使用する
	virtual ModSize tokenizeMulti(const ModUnicodeString&,			// 解析対象
								  const TokenizeMode,				// 分割モード
#ifdef INV_TOK_USEVECTOR
								  ModVector<QueryTokenizedResult>&	// 分割結果
#else
								  QueryTokenizedResult*&
#endif
#ifdef V1_6
								  , const ModLanguageSet&			// セクションの言語
#endif // V1_6
								  );

	virtual ModSize getTokenLength(const ModUnicodeString&) const;

	virtual void getDescription(ModCharString& description_,
								const ModBoolean withName_ = ModTrue) const = 0;

	static ModUInt32 getResourceID(const ModCharString&, const ModCharString&);

	virtual ModBoolean isSupported(const ModInvertedFileIndexingType) const = 0;
	virtual ModBoolean isPrenormalizable() const = 0;
	static ModBoolean isPrenormalizable(const ModCharString&);

	virtual ModUnicodeString* getNormalizedText(
		const ModUnicodeString&,
#ifdef V1_6
		ModVector<ModLanguageSet>*,
#endif
		const ModVector<ModSize>*,
		ModVector<ModSize>*) = 0;

protected:
	typedef ModInvertedTokenOccurrence Occurrence;

	// 分割対象の設定
    virtual void set(const ModUnicodeString& target, const TokenizeMode mode,
					 const ModVector<ModSize>*, ModVector<ModSize>*) = 0;
	// 分割 -- 検索時の分割の下請（short word 対応）
	virtual ModBoolean tokenizeSub(const ModUnicodeString&,
								   const TokenizeMode,
								   ModInvertedLocationListMap&,
								   ModSize&,
								   ModUnicodeString&,
								   ModUnicodeString&,
								   ModUnicodeString&
#ifdef V1_6
								   , const ModLanguageSet&
#endif // V1_6
								   );
	// 一つのトークンを返す
    virtual ModBoolean yield(ModUnicodeString&, Occurrence&) = 0;
	// 処理の終了位置を返す
	virtual ModSize getTokenizedEnd() = 0;

	ModInvertedTokenizer(ModInvertedFile* file_) : file(file_) {}
	ModInvertedFile* file;

	static GetNormalizer* getNormalizerFunc;	// 正規化器取得関数
	static GetAnalyzer* getAnalyzerFunc;		// 解析器取得関数
#ifndef SYD_USE_UNA_V10
	static GetStemmer* getStemmerFunc;			// ステマー取得関数
#endif
private:

};


//
// FUNCTION
// ModInvertedTokenizer::tokenizeSub -- テキストの分割の下請け
//
// NOTES
// テキストの分割の下請け関数。
// 渡されたテキスト（文字列）をモードにしたがって分割する。
//
// ARGUMENTS
// const ModUnicodeString& targetString
//		分割対象テキスト
// const TokenizeMode mode
//		分割モード
// ModInvertedLocationListMap& result
//		分割結果
// ModSize& tokenizedEnd
//		通常処理で分割し終えた位置
// ModUnicodeString& shortWord
//		ショートワード部分の文字列
// ModUnicodeString& fromWord
//		ショートワード処理で追加的に展開すべき先頭の文字列
// ModUnicodeString& toWord
//		ショートワード処理で追加的に展開すべき末尾の文字列
// QueryTokenizedResult*& result
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// ショートワード処理が不要ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外はそのまま再送出する
//
inline ModBoolean
ModInvertedTokenizer::tokenizeSub(const ModUnicodeString& target,
								  const TokenizeMode mode,
								  ModInvertedLocationListMap& result,
								  ModSize& tokenizedEnd,
								  ModUnicodeString& shortWord,
								  ModUnicodeString& fromWord,
								  ModUnicodeString& toWord,
								  const ModLanguageSet& langSet_)
{
	ModVector<ModLanguageSet> langSets;
	langSets.pushBack(langSet_);

	tokenize(target, mode, result, tokenizedEnd, 0, 0, &langSets, 0, 0);
	
	return ModTrue;
}

//
// FUNCTION
// ModInvertedTokenizer::tokenizeMulti -- テキストの分割（検索用）
//
// NOTES
// 検索用に渡されたテキスト（文字列）をモードにしたがって分割する。
// 異表記展開が行われた場合、その個数分の結果を生成する。
// 分割結果を呼び出し側が delete しなければならない。
//
// ARGUMENTS
// const ModUnicodeString& targetString
//		分割対象テキスト
// const TokenizeMode mode
//		分割モード
// QueryTokenizedResult*& result
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// 分割結果の配列の個数
//
// EXCEPTIONS
// 下位からの例外はそのまま再送出する
//
inline ModSize
ModInvertedTokenizer::tokenizeMulti(const ModUnicodeString& target,
									const TokenizeMode mode,
#ifdef INV_TOK_USEVECTOR
									ModVector<QueryTokenizedResult>& result
#else
									QueryTokenizedResult*& result
#endif
									, const ModLanguageSet& langSet_
									)
{
#ifdef INV_TOK_USEVECTOR
	switch (result.getSize()) {
	case 0:
		result.pushBack(QueryTokenizedResult());
		break;
	case 1:
		// 何もしない
		break;
	default:
		result.clear();
		result.pushBack(QueryTokenizedResult());
		break;
	}
	; ModAssert(result.getSize() == 1);
#else
	result = new QueryTokenizedResult[1];
#endif

	ModVector<ModLanguageSet> langSets;
	langSets.pushBack(langSet_);

	result[0].isNormalWord = ModTrue;
	tokenize(target, mode,
			 result[0].locationListMap, result[0].tokenizedEnd, 0, 0,
			 &langSets, 0, 0);
	
	return 1;
}

#endif	// __ModInvertedTokenizer_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
