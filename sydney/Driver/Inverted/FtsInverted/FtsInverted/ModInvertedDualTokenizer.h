// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedDualTokenizer.h -- Dual 分割器 
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedDualTokenizer_H__
#define __ModInvertedDualTokenizer_H__

#ifdef V1_4

#include "ModInvertedTokenizer.h"
#include "ModInvertedBlockedNgramTokenizer.h"

class ModCharString;
class ModUnicodeString;
#if defined(UNA_V3_4)
#include "ModNLP.h"						// enum ModNlpNormMode を使うため
										// インクルードが必要
#elif defined(UNA_V3_3)
class ModNlpAnalyzer;
#else
class ModUnaAnalyzer;
class ModEnglishWordStemmer;
#endif

//
// CLASS
// ModInvertedDualTokenizer --- Dual による分割器
//
// NOTES
// テキストを、単語および n-gram の両方に分割する。
// 動作はモードによって異なる。
//

class
ModInvertedDualTokenizer : public ModInvertedBlockedNgramTokenizer {
	friend class ModInvertedTokenizer;
public:
#ifndef SYD_USE_UNA_V10
#if defined(UNA_V3_4)
	typedef ModNlpNormMode NlpNormMode;
#elif defined(UNA_V3_3)
	typedef int NlpNormMode;
#endif
#endif

	ModInvertedDualTokenizer(const ModCharString&,
							 ModInvertedFile*,
							 const ModBoolean = ModFalse,
							 const ModBoolean = ModTrue,
							 const ModInvertedUnaSpaceMode
									= ModInvertedUnaSpaceAsIs,
							 const ModBoolean = ModFalse
#ifdef SYD_USE_UNA_V10
							 , ModSize maxWordLen_ = 0
#endif
							// for Clustering
							, ModSize featureSize = 10
		);

	~ModInvertedDualTokenizer();

	// 分割 -- 登録時に使用する
	virtual void tokenize(const ModUnicodeString&,
						  const ModInvertedTokenizer::TokenizeMode,
						  ModInvertedLocationListMap&,
						  ModSize&,
						  const ModVector<ModSize>*,
						  ModVector<ModSize>*
#ifdef V1_6
						  , ModVector<ModLanguageSet>*
#endif // V1_6
						  // for Clustering
						  , ModInvertedFeatureList*,
						  const ModTermResource*);

	// 分割 -- 検索時に使用する
	virtual ModSize tokenizeMulti(const ModUnicodeString&,
								  const ModInvertedTokenizer::TokenizeMode,
#ifdef INV_TOK_USEVECTOR
								  ModVector<QueryTokenizedResult>&
#else
								  ModInvertedQueryTokenizedResult*&
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

	virtual ModSize getTokenLength(const ModUnicodeString&) const;

#ifdef SYD_USE_UNA_V10
	virtual UNA::ModNlpAnalyzer* getAnalyzer(const ModCharString&);
#else
#if defined(UNA_V3_3) || defined(UNA_V3_4)
	virtual ModNlpAnalyzer* getAnalyzer(const ModCharString&);
#else
	virtual ModEnglishWordStemmer* getStemmer(const ModCharString&);
	virtual ModUnaAnalyzer* getAnalyzer(const ModCharString&);
#endif
#endif

protected:
	void wordTokenize(const ModUnicodeString&,
					  const ModInvertedTokenizer::TokenizeMode,
					  ModInvertedLocationListMap&,
					  ModSize&,
					  const ModVector<ModSize>*,
					  ModVector<ModSize>*,
					  ModUnicodeString*
#ifdef V1_6
					  , ModVector<ModLanguageSet>*					  
#endif // V1_6
					  );
	void prepareNormalizedText(const ModUnicodeString&,
#ifdef V1_6
							   ModVector<ModLanguageSet>*,
#endif // V1_6
							   const ModVector<ModSize>*,
							   ModVector<ModSize>*,
							   ModInvertedLocationList&,
							   ModUnicodeString&);

	void mergeResult(ModInvertedLocationListMap& ,
					 const ModInvertedLocationList&);
	void prepareText(const ModUnicodeString&,
#ifdef V1_6
					 ModVector<ModLanguageSet>*,
#endif // V1_6
					 const ModVector<ModSize>*,
					 ModVector<ModSize>*,
					 ModInvertedLocationList&);
	void expandSubSingle(const ModUnicodeString&,
						 ModVector<ModUnicodeString>&,
						 ModVector<ModInvertedLocationList>&);
	void expandSubMulti(
#ifdef SYD_USE_UNA_V10
						const ModVector<ModUnicodeString>&,
#else
						const ModSize, const ModUnicodeString* const,
#endif
						ModVector<ModUnicodeString>&,
						ModVector<ModInvertedLocationList>&);
	ModSize wordExpand(const ModUnicodeString&,
					   const ModInvertedTokenizer::TokenizeMode,
#ifdef INV_TOK_USEVECTOR
					   ModVector<QueryTokenizedResult>&
#else
					   ModInvertedQueryTokenizedResult*&
#endif
#ifdef V1_6
					   , const ModLanguageSet&
#endif // V1_6
		);
	void wordExpandSubSingle(const ModUnicodeString&,
							 const ModSize,
							 ModInvertedQueryTokenizedResult*&,
							 const ModSize);
	void wordExpandSubMulti(
#ifdef SYD_USE_UNA_V10
							const ModVector<ModUnicodeString>&,
#else
							const ModSize, const ModUnicodeString* const,
#endif
							ModSize&,
							ModInvertedQueryTokenizedResult*&,
							const ModSize);

	virtual ModBoolean tokenizeSub(const ModUnicodeString&,
								   const ModInvertedTokenizer::TokenizeMode,
								   ModInvertedLocationListMap&,
								   ModSize&,
								   ModUnicodeString&,
								   ModUnicodeString&,
								   ModUnicodeString&
#ifdef V1_6
								   , const ModLanguageSet&
#endif // V1_6
								   );
	ModBoolean tokenizeSub2(const ModUnicodeString&,
							const ModInvertedTokenizer::TokenizeMode,
							ModInvertedLocationListMap&,
							ModSize&,
							ModUnicodeString&,
							ModUnicodeString&,
							ModUnicodeString&,
							ModUnicodeString*
#ifdef V1_6
							, const ModLanguageSet&
#endif // V1_6
							);

#ifdef SYD_USE_UNA_V10
	// UNAパラメータを設定する
	void setUnaParameter(ModBoolean isNormalized_,
						 const ModLanguageSet& cLang_);
#endif



 	void (ModInvertedDualTokenizer::*wordTokenizeSub)(
		const ModUnicodeString& target_,
		const ModBoolean normalize_,
		ModInvertedLocationListMap& result_,
		ModSize& currentLocation_,
		ModUnicodeString* normalizedTarget_,
		const ModLanguageSet& langSet_
		);

	void (ModInvertedDualTokenizer::*prepareTextSub)(
		const ModUnicodeString& target_,
		const ModLanguageSet& langSet_,
		ModSize& currentLocation_,
		ModInvertedLocationList& wordLocations_
		);

	void (ModInvertedDualTokenizer::*prepareNormalizedTextSub)(
		const ModUnicodeString& target_,
		ModSize& currentLocation_,
		const ModLanguageSet& langSet_,
		ModInvertedLocationList& wordLocations_,
		ModUnicodeString& normalized_);


	void _wordTokenizeSub(const ModUnicodeString&,
						 const ModBoolean,
						 ModInvertedLocationListMap&,
						 ModSize&,
						 ModUnicodeString*
#ifdef V1_6
						 , const ModLanguageSet&
#endif // V1_6
						 );

	void _prepareTextSub(const ModUnicodeString&,
#ifdef V1_6
						const ModLanguageSet&,
#endif // V1_6
						ModSize&, ModInvertedLocationList&);

	void _prepareNormalizedTextSub(const ModUnicodeString&, ModSize&,
#ifdef V1_6
								  const ModLanguageSet&,
#endif // V1_6
								  ModInvertedLocationList&,
								  ModUnicodeString&);
	ModSize expand(const ModUnicodeString&,
				   const ModInvertedTokenizer::TokenizeMode,
#ifdef INV_TOK_USEVECTOR
				   ModVector<QueryTokenizedResult>&
#else
				   ModInvertedQueryTokenizedResult*&
#endif
#ifdef V1_6
				   , const ModLanguageSet&
#endif // V1_6
				   );

	// 特徴語抽出サポート関数
	void wordTokenizeSubWithFeature(
		const ModUnicodeString& target_,
		const ModBoolean normalize_,
		ModInvertedLocationListMap& result_,
		ModSize& currentLocation_,
		ModUnicodeString* normalizedTarget_,
		const ModLanguageSet& langSet_);

	void prepareTextSubWithFeature(
		const ModUnicodeString& target_,
		const ModLanguageSet& langSet_,
		ModSize& currentLocation_,
		ModInvertedLocationList& wordLocations_);

	void prepareNormalizedTextSubWithFeature(
		const ModUnicodeString& target_,
		ModSize& currentLocation_,
		const ModLanguageSet& langSet_,
		ModInvertedLocationList& wordLocations_,
		ModUnicodeString& normalized_);

	void extractFeature(
		ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >&
		featureCandidate_);

private:
	//
	// TYPEDEF
	// FeatureCandidateElement -- 特徴語候補情報
	//
	// NOTE
	// 第一要素：TF
	// 第二要素：生起コスト
	//
	typedef ModPair<int, int> FeatureCandidateElement;

	//
	// TYPEDEF
	// FeatureCandidateMap -- 特徴語候補のマップ
	//
	typedef ModMap<ModUnicodeString,
				   FeatureCandidateElement,
				   ModLess<ModUnicodeString> > FeatureCandidateMap;

	// FeatureCandidateを設定
	void setFeatureCandidate(FeatureCandidateMap& mapFeatureCandidate_,
							 int iPOS_,
							 const ModUnicodeString& cstrMorpheme_,
							 int iCost_,
							 int iUnifiedPOS_) const;
	
	ModUInt32 unaRscID;
#ifdef SYD_USE_UNA_V10
	ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
		m_cUnaParam;
	UNA::ModNlpAnalyzer* una;					// NLPハンドル
#else
#if defined(UNA_V3_4)
	ModNlpAnalyzer* una;						// NLPハンドル
#elif defined(UNA_V3_3)
	ModNlpAnalyzer* una;						// NLPハンドル
#else
	ModUnaAnalyzer* una;						// UNAハンドル

	ModUInt32 stemmerRscID;
	ModEnglishWordStemmer *stemmer;				// ステマー

#endif
#endif
#ifndef SYD_USE_UNA_V10
	ModNlpNormModifier unaModifierMode;			// 空白文字の削除指定
	static NlpNormMode defaultUnaSetMode;
	NlpNormMode unaSetMode;
#ifndef SYD_INVERTED
	static NlpNormMode getUnaSetMode();
#endif
#endif

	ModSize m_featureSize;							// 特徴語数
	
	ModInvertedFeatureList* m_pFeature;			// 特徴語配列
	//【注意】
	// メソッドの引数にすると関数ポインターで切り替えられなくなるので、
	// tokenizeに渡された引数がここに代入される

	// TermResource
	// 設定方法等はm_pFeatureに合わせた。
	const ModTermResource* m_pTermResource;

	static const char tokenizerName[];
};

#endif // V1_4

#endif	// __ModInvertedDualTokenizer_H__

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
