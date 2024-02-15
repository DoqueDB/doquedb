// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedNgramTokenizer.h -- Ngram 分割器 
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedNgramTokenizer_H__
#define __ModInvertedNgramTokenizer_H__

#include "ModInvertedTokenizer.h"

#ifdef SYD_USE_UNA_V10
#include "ModMap.h"
#endif

class ModCharString;
class ModUnicodeString;

//
// CLASS
// ModInvertedNgramTokenizer --- Ngram による分割器
//
// NOTES
// テキストを、minLength 以上 maxLength 以下の n-gram に分割する。
// 登録時（document モードでは）、該当する全ての n-gram を抽出する。
// なお、テキスト末尾では minLength 以下の n-gram も抽出する。
// 検索時（query モードでは）、maxLength に等しい n-gram のみを抽出する。
// 簡易検索時（simpleQuery モードでは）、可能な限り重複しない maxLength
// に等しい n-gram のみを抽出する。minLength = 2, maxLength = 3 とし、
// 以下に例を示す。
//
//		入力: わたしはたわし
//		document:		わた、わたし、たし、たしは、しは、しはた、
//						はた、はたわ、たわ、たわし、わし、し
//		query:			たわし、たしは、しはた、はたわ、たわし
//		simpleQuery:	たわし、はたわ、たわし
// 
class
ModInvertedNgramTokenizer : public ModInvertedTokenizer {
	friend class ModInvertedTokenizer;
public:
	ModInvertedNgramTokenizer(const ModCharString&,
							  ModInvertedFile*,
							  const ModBoolean = ModFalse);
	ModInvertedNgramTokenizer(const ModSize, const ModSize,
							  ModInvertedFile*,
							  const ModBoolean = ModFalse);
	~ModInvertedNgramTokenizer();

	virtual ModSize tokenizeMulti(const ModUnicodeString&,
								  const TokenizeMode,
#ifdef INV_TOK_USEVECTOR
								  ModVector<QueryTokenizedResult>&
#else
								  QueryTokenizedResult*&
#endif
#ifdef V1_6
								  , const ModLanguageSet&
#endif // V1_6
		);

	virtual void getDescription(ModCharString& description_,
								const ModBoolean withName_ = ModTrue) const;

	virtual ModBoolean isSupported(const ModInvertedFileIndexingType) const;
	virtual ModBoolean isPrenormalizable() const;

	virtual ModUnicodeString* getNormalizedText(
		const ModUnicodeString&,
#ifdef V1_6
		ModVector<ModLanguageSet>*,
#endif
		const ModVector<ModSize>*,
		ModVector<ModSize>*);

#ifdef SYD_USE_UNA_V10
	virtual UNA::ModNlpAnalyzer* getNormalizer(const ModCharString&);
#else
	virtual ModNormalizer* getNormalizer(const ModCharString&);
#endif

protected:
	
	void parse(const ModCharString&);
	ModSize divideIntoToken(ModCharString*, const ModCharString&);

	void clear();
	void setCharOffsets(const ModVector<ModSize>*, ModVector<ModSize>*);
	void prepareNormalizedText(const ModUnicodeString&,
							   const ModVector<ModSize>*,
							   ModVector<ModSize>*);

	virtual void set(const ModUnicodeString&, const TokenizeMode,
					 const ModVector<ModSize>*, ModVector<ModSize>*);


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

#ifdef SYD_USE_UNA_V10
	void setNormalizedFlagForNormalizer(bool isNormalized_);
#endif

	ModBoolean yield(ModUnicodeString&, Occurrence&);
	ModSize getTokenizedEnd();

	ModUnicodeString* target;		// 分割対象
	ModSize targetLength;
	ModBoolean isAllocated;

	TokenizeMode mode;
	ModSize currentLength;
	ModSize headOffset;
	ModSize tailOffset;
	ModSize nextOffset;					// simpleQuery モードで次に ngram 
										// が開始すべき位置
	ModBoolean isShortWord;				// 検索語が短く、OR展開が必要な場合

	ModSize currentMaxLength;
	ModSize minLength;
	ModSize maxLength;

	ModBoolean normalizing;
#ifdef SYD_USE_UNA_V10
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
		m_cUnaParameter;					// UNAに渡すパラメータ
	UNA::ModNlpAnalyzer* normalizer;		// 正規化器
#else
	ModNormalizer* normalizer;				// 正規化器
#endif
	ModUInt32 normRscID;

private:
	static const char tokenizerName[];
};


//
// FUNCTION
// ModInvertedNgramTokenizer::getTokenizedEnd -- 分割処理の末尾位置の取得
//
// NOTES
// 分割処理の末尾位置を取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// 分割処理の末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize 
ModInvertedNgramTokenizer::getTokenizedEnd()
{
	return targetLength;
}

#endif	// __ModInvertedNgramTokenizer_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
