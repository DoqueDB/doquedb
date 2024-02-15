// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedDualTokenizer.cpp -- DUAL 分割器の実装
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModAutoPointer.h"
#endif

#include "UNA_UNIFY_TAG.h"

#ifdef V1_4

#ifndef SYD_INVERTED
#include "ModParameter.h"
#endif
#include "ModAlgorithm.h"
#include "ModOsDriver.h"	// log
#include "ModOstrStream.h"
#include "ModString.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"


#include "ModTerm.h"

#include "ModInvertedException.h"
#include "ModInvertedDualTokenizer.h"
#include "ModInvertedSmartLocationList.h"
#include "ModTermElement.h"
#ifdef  SYD_INVERTED // SYDNEY ymu=
#include "Inverted/ModInvertedFile.h"
#include "Inverted/Parameter.h"
#else
#include "ModInvertedFile.h"
#endif

#if defined(UNA_V3_3) || defined(UNA_V3_4)
#include "ModNLP.h"
#else
#include "ModUNA.h"
#include "ModEnglishWordStemmer.h"
#include "ModParameter.h"
#endif

#define LOG(x)	 ModOsDriver::Math::log(x)

namespace {
	//
	//	VARIABLE
	//	UNAのパラメータのキーとバリュー
	//
	ModUnicodeString _DoNorm("donorm");

	ModUnicodeString _True("true");
	ModUnicodeString _False("false");
	class _WeightGreater
	{
	public:
		ModBoolean operator() (const ModInvertedFeatureElement & x,
							   const ModInvertedFeatureElement & y)
			{ return (x.second > y.second)
					? ModTrue : ModFalse; }
	};

	//
	//	VARIABLE
	//	_$$::_MaxOccurrenceCost -- 生起コストの最大値
	//
	_SYDNEY::Inverted::ParameterInteger
	_MaxOccurrenceCost("Inverted_MaxOccurrenceCost", 500);
	
	//
	//	VARIABLE
	//	_$$::_AlphabetOccurrenceCostFactor -- 英文字列の生起コストの係数
	//
	_SYDNEY::Inverted::ParameterInteger
	_AlphabetOccurrenceCostFactor("Inverted_AlphabetOccurrenceCostFactor", 25);
}

//
// CONST
// ModInvertedDualTokenizer::tokenizerName -- 分割器の名称
//
// NOTES
// 分割器の名称を表す
//
/*static*/
const char ModInvertedDualTokenizer::tokenizerName[] = "DUAL";

//
// FUNCTION
// ModInvertedDualTokenizer::ModInvertedDualTokenizer -- コンストラクタ
//
// NOTES
// Dual 分割器をコンストラクトする。
// パラメータ記述の解釈には ModInvertedBlockedNgramTokenizer::parse が
// 使われるので、記述方法も ModInvertedBlockedNgramTokenizer に従う。
// ただし、以下の２つのリソース番号を追加で指定できる。
//
//		<解析器リソース指定> ::= <解析器リソースキー>:<リソース番号>
//		<解析器リソースキー> ::= @UNARSCID
//		<ステマーリソース指定> ::= <ステマーリソースキー>:リソース番号
//		<ステマーリソースキー> ::= @STEMRSCID
//		<リソース番号> ::= 0以上の整数
//
// デフォルトでは、リソース番号は 0 とする。
// 各キーは ModInvertedTokenizer.cpp で定義されている。
//
// ARGUMENTS
// const ModString& description_
//		パラメータ記述
// ModInvertedFile* file_
//		転置ファイル
// const ModBoolean normalizing_
//		正規化処理の指示
// const ModBoolean carriage_
//		改行を跨った正規化を行うかどうか
// ModSize maxWordLen_
//		最大単語長
// ModSize featureSize_
//		特徴語数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedDualTokenizer::ModInvertedDualTokenizer(
	const ModCharString& description_,
	ModInvertedFile* file_,
	const ModBoolean normalizing_,
	const ModBoolean stemming_,
	const ModInvertedUnaSpaceMode unaSpaceMode_,
	const ModBoolean carriage_,
	ModSize maxWordLen_,
	ModSize featureSize_)
	: ModInvertedBlockedNgramTokenizer(description_, file_, normalizing_),
	  una(0),
	  unaRscID(getResourceID(description_, analyzerResourceID)),
	  m_featureSize(featureSize_)
{
	// とりあえず特徴語が必要かどうかわからないので、
	// 特徴語を抽出しない方の関数を割り当てておく
	wordTokenizeSub = &ModInvertedDualTokenizer::_wordTokenizeSub;
	prepareTextSub = &ModInvertedDualTokenizer::_prepareTextSub;
	prepareNormalizedTextSub = &ModInvertedDualTokenizer::_prepareNormalizedTextSub;
	m_pFeature = 0;
	m_pTermResource = 0;
	
#if !(defined(UNA_V3_3) || defined(UNA_V3_4))
	if (normalizing_ == ModTrue) {
		ModParameter p;
		ModBoolean tmp(ModFalse);
		if (p.getBoolean(tmp,
						 "InvertedDualTokenizerSkipStemming") == ModFalse ||
			tmp == ModFalse) {
			// 正規化モードで、ステマーを使わないと指示されていないので
			// なければステミングも行う
			// stemmer へのセットは getStemmer() で行なわれる
			getStemmer(description_);
		}
	}
#endif
#ifdef SYD_USE_UNA_V10
	// UNAのパラメータを設定する
	ModUnicodeOstrStream str;
	
	str << maxWordLen_;
	m_cUnaParam.insert("maxwordlen", str.getString());	// MaxWordLen
	
	
	m_cUnaParam.insert("compound", _True);
	m_cUnaParam.insert("stem", (stemming_ == ModTrue ? _True : _False));
	m_cUnaParam.insert("carriage", (carriage_ == ModTrue ? _True : _False));
	
	switch (unaSpaceMode_) {
	case ModInvertedUnaSpaceAsIs:
		m_cUnaParam.insert("space", "0");		// ModNlpAsIs
		break;
	case ModInvertedUnaSpaceNoNormalize:
		m_cUnaParam.insert("space", "1");		// ModNlpNoNormalize
		break;
	case ModInvertedUnaSpaceDelete:
		m_cUnaParam.insert("space", "2");		// ModNlpDelete
		break;
	case ModInvertedUnaSpaceReset:
		m_cUnaParam.insert("space", "3");		// ModNlpReset
		break;
	}
#else
	if(stemming_ == ModFalse) {
		// 現在の正規化モードからstemmingを除外する。
		switch(unaSetMode) {
		case ModNlpNormStemDiv:
			unaSetMode = ModNlpNormDiv;
			break;
		case ModNlpNormStem:
			unaSetMode = ModNlpNormOnly;
			break;
		case ModNlpNormRetStemDiv:
			unaSetMode = ModNlpNormRetDiv;
			break;
		case ModNlpNormRetStem:
			unaSetMode = ModNlpNormRet;
			break;
		}
	}

	switch(unaSpaceMode_) {
	case ModInvertedUnaSpaceAsIs:
		unaModifierMode.space(ModNlpAsIs);
		break;
	case ModInvertedUnaSpaceNoNormalize:
		unaModifierMode.space(ModNlpNoNormalize);
		break;
	case ModInvertedUnaSpaceDelete:
		unaModifierMode.space(ModNlpDelete);
		break;
	case ModInvertedUnaSpaceReset:
		unaModifierMode.space(ModNlpReset);
		break;
	}
#endif

	try {
		// una へのセットは getAnalyzer() で行なわれる
		getAnalyzer(description_);
	} catch (ModException& e) {
		ModErrorMessage << "una create failed: " << e << ModEndl;
#if !(defined(UNA_V3_3) || defined(UNA_V3_4))
		delete stemmer;
		delete normalizer;		// NgramTokenizer で new しているが
								// エラー処理ではここで delete する
#endif
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::~ModInvertedDualTokenizer -- デストラクタ
//
// NOTES
// Dual 分割器をデストラクトする。
// 正規化器があれば、それもデストラクトする。
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
ModInvertedDualTokenizer::~ModInvertedDualTokenizer()
{
#ifdef SYD_USE_UNA_V10
	if (una) una->releaseResource();
#endif
	delete una;
#if !(defined(UNA_V3_3) || defined(UNA_V3_4))
	delete stemmer;
#endif
}

//
// FUNCTION
// ModInvertedTokenizer::tokenize -- テキストの分割
//
// NOTES
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
// const ModVector<ModSize>* sectionEndByteOffsets
//		バイト位置の配列
// ModVector<ModSize>* sectionEndCharacterOffsets
//		文字位置をセットする配列
// ModVector<ModLanguageSet>* langSet_
//		言語指定（v1_6のみ）
// ModInvertedFeatureList* feature_
//		特徴語配列
// const ModTermResource* pTermResource_
//		TermResource
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorTooLongIndexKey
//		切り出されたキーが長すぎる
//
void
ModInvertedDualTokenizer::tokenize(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
	ModInvertedLocationListMap& result_,
	ModSize& tokenizedEnd_,
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_
#ifdef V1_6
	, ModVector<ModLanguageSet>* langSets_
#endif // V1_6
	, ModInvertedFeatureList* feature_,
	const ModTermResource* pTermResource_)
{
	if (feature_)
	{
		// 特徴語を取得するので、それようの関数を割り当てる
 		wordTokenizeSub = &ModInvertedDualTokenizer::wordTokenizeSubWithFeature;
		prepareTextSub = &ModInvertedDualTokenizer::prepareTextSubWithFeature;
		prepareNormalizedTextSub = &ModInvertedDualTokenizer::prepareNormalizedTextSubWithFeature;
		m_pFeature = feature_;
		m_pTermResource = pTermResource_;
	}
	else
	{
		// 特徴語は不要なので、それようの関数を割り当てる
 		wordTokenizeSub = &ModInvertedDualTokenizer::_wordTokenizeSub;
		prepareTextSub = &ModInvertedDualTokenizer::_prepareTextSub;
		prepareNormalizedTextSub = &ModInvertedDualTokenizer::_prepareNormalizedTextSub;
		m_pFeature = 0;
		m_pTermResource = 0;
	}
	
#ifdef V1_6
	// パラメータで設定された、セクションの数と
	// 言語セットの数が異なる場合はエラー
	if (byteOffsets_ != 0 &&
	   (byteOffsets_->getSize() != langSets_->getSize())){
		ModThrowInvertedFileError(ModInvertedErrorInvalidLanguageSetNum);		
	}
#endif // V1_6

	if ((mode_&wordIndexingOnly) != 0) {
		// 単語分割結果のみの場合
		//		- この中で正規化は行われるが、それを返す必要はない
		wordTokenize(target_, mode_, result_, tokenizedEnd_,
					 byteOffsets_, charOffsets_, 0
#ifdef V1_6
					 , langSets_
#endif // V1_6
					 );
		return;
	}

	ModInvertedLocationList wordLocations;
	ModUnicodeString normalizedTarget;
	ModSize normalizedEnd;

	if (normalizing == ModTrue && (mode_&skipNormalizing) == 0) {
		// 正規化が必要な場合
		//		- 登録時には skipExpansion は無視する
		prepareNormalizedText(target_,
#ifdef V1_6
							  langSets_,
#endif // V1_6
							  byteOffsets_, charOffsets_,
							  wordLocations, normalizedTarget);

		ModInvertedTokenizer::tokenize(normalizedTarget,
									   TokenizeMode(mode_|skipNormalizing),
									   result_, normalizedEnd, 0, 0
#ifdef V1_6
									   , langSets_
#endif // V1_6
									   , feature_, pTermResource_);
	} else {
		// 正規化が不要な場合
		prepareText(target_,
#ifdef V1_6
					langSets_,
#endif // V1_6
					byteOffsets_, charOffsets_, wordLocations);

		ModInvertedTokenizer::tokenize(target_, mode_,
									   result_, tokenizedEnd_, 0, 0
#ifdef V1_6
									   , langSets_
#endif // V1_6
									   , feature_, pTermResource_);
	}

	// 単語の情報を追加する - n-gram分割結果のみの場合には付加しない
	if ((mode_&ngramIndexingOnly) == 0) {
		mergeResult(result_, wordLocations);
	}

	tokenizedEnd_ = getTokenizedEnd();
}

//
// FUNCTION
// ModInvertedDualTokenizer::getDescription -- 分割器記述の取得
//
// NOTES
// 分割器記述を取得する。
//
// ARGUMENTS
// ModCharString& description_
//		出力バッファ
// const ModBoolean withName_
//		分割器名称の付与指示
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedDualTokenizer::getDescription(ModCharString& description_,
										 const ModBoolean withName_) const
{
	description_.clear();
	if (withName_ == ModTrue) {
		description_.append(tokenizerName);
		description_.append(':');
	}

	ModCharString tmp;
	ModInvertedBlockedNgramTokenizer::getDescription(tmp, ModFalse);
	description_.append(tmp);

	ModOstrStream stream;
	stream << ' ' << analyzerResourceID << ':' << unaRscID;
#if !(defined(UNA_V3_3) || defined(UNA_V3_4))
	if (normalizing == ModTrue || stemmerRscID != 0) {
		stream << ' ' << stemmerResourceID << ':' << stemmerRscID;
	}
#endif
	stream << ' ' << analyzerResourceID << ':' << unaRscID;
	description_.append(stream.getString());
}

//
// FUNCTION
// ModInvertedTokenizer::mergeResult -- 結果のマージ
//
// NOTES
// 文字列単位の分割結果と単語境界位置をマージする。
//
// ARGUMENTS
// ModInvertedLocationListMap& result_
//		n-gram分割結果
// const ModInvertedLocationList& wordLocations_
//		単語境界位置
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::mergeResult(
	ModInvertedLocationListMap& result_,
	const ModInvertedLocationList& wordLocations_)
{
	ModUnicodeString nullString;		// 単語境界は空文字列をキーとする

	ModInvertedLocationList::ConstIterator loc(wordLocations_.begin());
	if (loc != wordLocations_.end()) {
		// 単語境界にデータがある場合に追加を行う
		ModInvertedSmartLocationList tmp(file->getLocationCoder(nullString));
		tmp.setFirstValue(*loc);
		ModPair<ModInvertedLocationListMap::Iterator, ModBoolean>
			ires(result_.insert(nullString, tmp));
		if (ires.second == ModFalse) {
			; ModAssert(0);
		}

		// 単語境界を１つずつ追加する
		for (++loc; loc != wordLocations_.end(); ++loc) {
			(*(ires.first)).second.pushBack(*loc);
		}
			
	}
}

//
// FUNCTION
// ModInvertedTokenizer::wordTokenizeSub -- テキストの単語分割の下請け
//
// NOTES
// 渡されたテキスト（文字列）をモードにしたがって分割する。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const ModBoolean normalize_
//		正規化指示
// ModInvertedLocationListMap& result
//		分割結果
// ModSize& currentLocation_
//		通常処理で分割し終えた位置
// const ModVector<ModSize>* sectionEndByteOffsets
//		バイト位置の配列
// ModVector<ModSize>* sectionEndCharacterOffsets
//		文字位置をセットする配列
// ModUnicodeString* normalizedTarget_
//		正規化結果のテキスト
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInverted/rrorTooLongIndexKey
//		切り出されたキーが長すぎる
//
void
ModInvertedDualTokenizer::_wordTokenizeSub(
	const ModUnicodeString& target_,
	const ModBoolean normalize_,
	ModInvertedLocationListMap& result_,
	ModSize& currentLocation_,
	ModUnicodeString* normalizedTarget_
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	ModUnicodeString morph;
	ModInvertedSmartLocationList tmp;	// Vector の領域をここで確保する

#ifdef SYD_USE_UNA_V10
	setUnaParameter(normalize_, langSet_);
	una->set(target_, langSet_);
#ifdef SYD_USE_UNA_V12
	while (una->getWord(morph) == ModTrue)
#else
		ModUnicodeString dummy1;
	int dummy2;
	while (una->getWord(morph, dummy1, dummy2) == ModTrue)
#endif
#else
#if defined(UNA_V3_4)
		una->set(target_, unaSetMode
#ifdef V1_6
				 , langSet_
#endif // V1_6
				 , unaModifierMode
			);
	while (una->getWord(morph, normalize_) == ModTrue)
#elif defined(UNA_V3_3)
		una->set(target_, unaSetMode);
	while (una->getWord(morph, normalize_) == ModTrue)
#else
		una->set(target_, (stemmer != 0));	// ステマーがあれば使う（下位構造も）
	while (una->getMorph(morph, normalize_) == ModTrue)
#endif
#endif
	{
		// 形態素がある限り処理を続ける
		if (morph.getLength() == 0) {
			// これはないはず
#ifdef DEBUG
			ModErrorMessage << "SKIP!" << ModEndl;
#endif
			; ModAssert(0);
			continue;
		}
#ifdef DEBUG
		if (ModInvertedFile::debugLevel > 0) {
			ModDebugMessage << currentLocation_ << ' ' << morph << ModEndl;
		}
#endif // DEBUG

		if (morph.getLength() > ModInvertedIndexKeyLenMax) {
			ModErrorMessage << "token=" << morph
							<< " too long" << ModEndl;
			// wordindexing の場合には例外は返さず、無視するだけとする
			// ただし、このトークンで検索はできない
			// ModThrowInvertedFileError(ModInvertedErrorTooLongIndexKey);
			++currentLocation_;
		} else {

			ModInvertedLocationListMap::Iterator pair(result_.find(morph));
			if (pair == result_.end()) {
				// 見つからない
				tmp.setCoder(file->getLocationCoder(morph));
				tmp.setFirstValue(++currentLocation_);
				result_.insert(morph, tmp);
			} else {
				(*pair).second.pushBack(++currentLocation_);
			}
		}

		if (normalizedTarget_ != 0) {
			normalizedTarget_->append(morph);
		}
	}
}
//
// FUNCTION
// ModInvertedTokenizer::wordTokenizeSub -- テキストの単語分割の下請け
//
// NOTES
// 渡されたテキスト（文字列）をモードにしたがって分割する。
//  wordTokenizeSubの特徴語抽出版
//
void
ModInvertedDualTokenizer::wordTokenizeSubWithFeature(
	const ModUnicodeString& target_,
	const ModBoolean normalize_,
	ModInvertedLocationListMap& result_,
	ModSize& currentLocation_,
	ModUnicodeString* normalizedTarget_,
	const ModLanguageSet& langSet_
	)
{
	ModUnicodeString morph;
	ModInvertedSmartLocationList tmp;	// Vector の領域をここで確保する

	setUnaParameter(normalize_, langSet_);
	una->set(target_, langSet_);
	// 特徴語候補
  	ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >
		featureCandidate;
 	ModVector<ModUnicodeString> formVector;	// 形態素語形ベクトル
	ModVector<ModUnicodeString> ostrVector;	// 形態素語形ベクトル(元表記)
	ModVector<int>				posVector;  // 形態素品詞ベクトル
	ModVector<int>				costVector;	// 単語コストベクトル
	ModVector<int>				uposVector;	// UNA 統合品詞

	while (una->getBlock(formVector, ostrVector,
						 posVector, costVector, uposVector) == ModTrue)
	{
		ModVector<ModUnicodeString>::Iterator form = formVector.begin();
		ModVector<int>::Iterator pos  = posVector.begin();
		ModVector<int>::Iterator cost = costVector.begin();
		ModVector<int>::Iterator upos = uposVector.begin();

		// 形態素がある限り処理を続ける
		for (; pos != posVector.end(); ++pos, ++form, ++cost, ++upos)
		{
			// 形態素を特徴語候補に設定
			setFeatureCandidate(featureCandidate, *pos, *form, *cost, *upos);

			if (form->getLength() == 0) {
				// これはないはず
				; ModAssert(0);
				continue;
			}
			if (form->getLength() > ModInvertedIndexKeyLenMax) {
				ModErrorMessage << "token=" << morph
								<< " too long" << ModEndl;
				// wordindexing の場合には例外は返さず、無視するだけとする
				// ただし、このトークンで検索はできない
				// ModThrowInvertedFileError(ModInvertedErrorTooLongIndexKey);
				++currentLocation_;
			} else {
				ModInvertedLocationListMap::Iterator pair(result_.find(*form));
				if (pair == result_.end()) {
					// 見つからない
					tmp.setCoder(file->getLocationCoder(*form));
					tmp.setFirstValue(++currentLocation_);
					result_.insert(*form, tmp);
				} else {
					(*pair).second.pushBack(++currentLocation_);
				}
			}
			if (normalizedTarget_ != 0) {
				normalizedTarget_->append(*form);
			}
		}
	}
	extractFeature(featureCandidate);
}

//
// FUNCTION
// ModInvertedTokenizer::wordTokenize -- テキストの単語分割
//
// NOTES
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
// const ModVector<ModSize>* sectionEndByteOffsets
//		バイト位置の配列
// ModVector<ModSize>* sectionEndCharacterOffsets
//		文字位置をセットする配列
// ModUnicodeString* normalizedTarget_
//		正規化結果のテキスト
// ModVector<ModLanguageSet>* langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorTooLongIndexKey
//		切り出されたキーが長すぎる
//
void
ModInvertedDualTokenizer::wordTokenize(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
	ModInvertedLocationListMap& result_,
	ModSize& tokenizedEnd_,
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_,
	ModUnicodeString* normalizedTarget_
#ifdef V1_6
	, ModVector<ModLanguageSet>* langSets_
#endif // V1_6
	)
{
	ModSize currentLocation(0);

	// 正規化が必要か
	ModBoolean needNormalize(ModBoolean(normalizing == ModTrue &&
										(mode_&skipNormalizing) == 0 &&
										(mode_&skipExpansion) == 0));

	if (byteOffsets_ == 0) {
		// セクションがない場合
		(this->*wordTokenizeSub)(target_, needNormalize, result_, currentLocation,
								 normalizedTarget_
#ifdef V1_6
								 , langSets_->getBack()
#endif // V1_6
			);

	} else {
		// セクションがある場合
		; ModAssert(charOffsets_ != 0);
		charOffsets_->clear();

		ModUnicodeString section;

		ModSize beginOffset(0), endOffset;
		ModVector<ModSize>::ConstIterator b(byteOffsets_->begin());
#ifdef V1_6
		ModVector<ModLanguageSet>::Iterator
			langSetIter(langSets_->begin());
#endif // V1_6
		for (; b != byteOffsets_->end(); ++b) {
			endOffset = (*b)/2;
			if (endOffset < beginOffset) {
				ModErrorMessage << "invalid character offset: "
								<< beginOffset << ' ' << endOffset << " at "
								<< (b - byteOffsets_->begin()) << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
			if (endOffset == beginOffset) {
				// copy は第２引数が 0 だとまるごとコピーしてしまうので、
				// 分ける。文字オフセットをセットするだけ。
				charOffsets_->pushBack(currentLocation + 1);
#ifdef V1_6
				++langSetIter;
#endif // V1_6
				continue;
			}

			section = target_.copy(beginOffset, endOffset - beginOffset);
			beginOffset = endOffset;

			(this->*wordTokenizeSub)(section, needNormalize, result_, currentLocation,
									 normalizedTarget_
#ifdef V1_6
									 , *langSetIter
#endif // V1_6
				);

			// currentLocation は先頭からの単語数になっている。
			// オフセットは次のセクションの先頭の単語位置でなければ
			// ならなので、+ 1 する必要がある。
			charOffsets_->pushBack(currentLocation + 1);
#ifdef V1_6
			++langSetIter;
#endif // V1_6
		}
	}

	tokenizedEnd_ = currentLocation;
}

//
// FUNCTION
// ModInvertedDualTokenizer::prepareTextSub -- 正規化テキストの用意の下請け
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		もとテキスト
// const ModLanguageSet& langset_
//		言語指定（V1_6のみ）
// ModSize& currentLocation_,
//		現在位置
// ModInvertedLocationList& wordLocations_
//		単語境界位置
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::_prepareTextSub(
	const ModUnicodeString& target_,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	ModSize& currentLocation_,
	ModInvertedLocationList& wordLocations_
	)
{
	ModUnicodeString morph;

#ifdef SYD_USE_UNA_V10
	setUnaParameter(ModFalse, langSet_);
	una->set(target_, langSet_);
#ifdef SYD_USE_UNA_V12
	while (una->getWord(morph) == ModTrue)
#else
		ModUnicodeString dummy1;
	int dummy2;
	while (una->getWord(morph, dummy1, dummy2) == ModTrue)
#endif
#else	
		una->set(target_, unaSetMode
#if defined(UNA_V3_4) && defined(V1_6)
				 , langSet_
#endif // UNA_V3_4 & V1_6
				 , unaModifierMode
			);

#if defined(UNA_V3_3) || defined(UNA_V3_4)
	while (una->getWord(morph, ModFalse) == ModTrue)
#else
		while (una->getMorph(morph, ModFalse) == ModTrue)
#endif
#endif
		{
			// 形態素がある限り処理を続ける
#ifdef DEBUG
			if (ModInvertedFile::debugLevel > 0) {
				ModDebugMessage << morph << ' ' << currentLocation_ << ModEndl;
			}
#endif // DEBUG

			if (morph.getLength() > 0) {
				if (currentLocation_ == 0) {
					// 先頭位置を記録する
					wordLocations_.pushBack(++currentLocation_);
				}
				wordLocations_.pushBack(currentLocation_ += morph.getLength());
			}
		}
}

void
ModInvertedDualTokenizer::prepareTextSubWithFeature(
	const ModUnicodeString& target_,
	const ModLanguageSet& langSet_,
	ModSize& currentLocation_,
	ModInvertedLocationList& wordLocations_
	)
{

	ModUnicodeString morph;

	setUnaParameter(ModFalse, langSet_);
	una->set(target_, langSet_);

	// 特徴語候補
  	ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >
		featureCandidate;
	ModVector<ModUnicodeString> formVector;	// 形態素語形ベクトル
	ModVector<ModUnicodeString> ostrVector;	// 形態素語形ベクトル(元表記)
	ModVector<int>				posVector;  // 形態素品詞ベクトル
	ModVector<int>				costVector;	// 単語コストベクトル
	ModVector<int>				uposVector;	// UNA 統合品詞

	
	while (una->getBlock(formVector, ostrVector,
						 posVector, costVector, uposVector) == ModTrue)
	{
		ModVector<ModUnicodeString>::Iterator form = formVector.begin();
		ModVector<int>::Iterator pos  = posVector.begin();
		ModVector<int>::Iterator cost = costVector.begin();
		ModVector<int>::Iterator upos = uposVector.begin();

		for (; pos != posVector.end(); ++pos, ++form, ++cost, ++upos)
		{
			// 形態素を特徴語候補に設定
			setFeatureCandidate(featureCandidate, *pos, *form, *cost, *upos);

			// 形態素がある限り処理を続ける
			if (form->getLength() > 0) {
				if (currentLocation_ == 0) {
					// 先頭位置を記録する
					wordLocations_.pushBack(++currentLocation_);
				}
				wordLocations_.pushBack(currentLocation_ += form->getLength());
			}
		}
	}
	extractFeature(featureCandidate);
}
//
// FUNCTION
// ModInvertedDualTokenizer::prepareText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		もとテキスト
// ModVector<ModLanguageSet>* langSets_
//		言語指定（V1_6のみ）
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
// ModInvertedLocationList& wordLocations_
//		単語境界位置
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::prepareText(
	const ModUnicodeString& target_,
#ifdef V1_6
	ModVector<ModLanguageSet>* langSets_,
#endif // V1_6
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_,
	ModInvertedLocationList& wordLocations_
	)
{
	ModSize currentLocation(0);

	if (byteOffsets_ == 0) {
		// セクションがない場合
		(this->*prepareTextSub)(target_,
#ifdef V1_6
								langSets_->getBack(),
#endif // V1_6
								currentLocation, wordLocations_);	

	} else {
		// セクションがある場合
		; ModAssert(charOffsets_ != 0);
		charOffsets_->clear();

		ModUnicodeString section;

		ModSize beginOffset(0), endOffset;
		ModVector<ModSize>::ConstIterator b(byteOffsets_->begin());
#ifdef V1_6
		ModVector<ModLanguageSet>::Iterator
			langSetIter(langSets_->begin());
#endif // V1_6
		for (; b != byteOffsets_->end(); ++b) {
			endOffset = (*b)/2;
			if (endOffset < beginOffset) {
				ModErrorMessage << "invalid character offset: "
								<< beginOffset << ' ' << endOffset << " at "
								<< (b - byteOffsets_->begin()) << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
			if (endOffset == beginOffset) {
				// copy は第２引数が 0 だとまるごとコピーしてしまうので、
				// 分ける。文字オフセットをセットするだけ。
				if (currentLocation == 0) {
					// 最初の場合、文字オフセットは文字数＋１なので１を記録
					charOffsets_->pushBack(1);
				} else {
					charOffsets_->pushBack(currentLocation);
				}
#ifdef V1_6
				++langSetIter;
#endif // V1_6
				continue;
			}

			section = target_.copy(beginOffset, endOffset - beginOffset);
			beginOffset = endOffset;

			(this->*prepareTextSub)(section, 
#ifdef V1_6
									*langSetIter,
#endif // V1_6
									currentLocation, wordLocations_);

			; ModAssert(currentLocation == endOffset + 1);
			charOffsets_->pushBack(currentLocation);
#ifdef V1_6
			++langSetIter;
#endif // V1_6
		}
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::prepareNormalizedText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		もとテキスト
// ModSize& currentLocation_,
//		現在位置
// ModVector<ModLanguageSet>* langSets_
//		言語指定（V1_6のみ）
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
// ModInvertedLocationList& wordLocations_,
//		単語境界位置
// ModUnicodeString& normalized_
//		正規化テキスト
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::_prepareNormalizedTextSub(
	const ModUnicodeString& target_,
	ModSize& currentLocation_,
#ifdef V1_6
	const ModLanguageSet& langSet_,
#endif // V1_6
	ModInvertedLocationList& wordLocations_,
	ModUnicodeString& normalized_)
{
	ModUnicodeString morph;

#ifdef SYD_USE_UNA_V10
	setUnaParameter(ModTrue, langSet_);
	una->set(target_, langSet_);
#ifdef SYD_USE_UNA_V12
	while (una->getWord(morph) == ModTrue)
#else
		ModUnicodeString dummy1;
	int dummy2;
	while (una->getWord(morph, dummy1, dummy2) == ModTrue)
#endif
#else
#if defined(UNA_V3_4)
		una->set(target_, unaSetMode
#ifdef V1_6
				 , langSet_
#endif // V1_6
				 , unaModifierMode
			);
	while (una->getWord(morph, ModTrue) == ModTrue)
#elif defined(UNA_V3_3)
		una->set(target_, unaSetMode);
	while (una->getWord(morph, ModTrue) == ModTrue)
#else
		una->set(target_, (stemmer != 0));	// ステマーがあれば使う（下位構造も）
	while (una->getMorph(morph, ModTrue) == ModTrue)
#endif
#endif
	{
		// 形態素がある限り処理を続ける
		if (morph.getLength() == 0) {
			// 正規化で長さ 0 となったものは無視する
			continue;
		}

		if (currentLocation_ == 0) {
			// 先頭位置を記録する
			wordLocations_.pushBack(++currentLocation_);
		}

		wordLocations_.pushBack(currentLocation_ += morph.getLength());
		normalized_.append(morph);

#ifdef DEBUG
		if (ModInvertedFile::debugLevel > 0) {
			ModDebugMessage << morph << ' ' << currentLocation_ << ModEndl;
		}
#endif // DEBUG
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::prepareNormalizedTextWithFeature 
//
// NOTES
// 正規化テキストの用意する(特徴語抽出付き)。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//

void
ModInvertedDualTokenizer::prepareNormalizedTextSubWithFeature(
	const ModUnicodeString& target_,
	ModSize& currentLocation_,
	const ModLanguageSet& langSet_,
	ModInvertedLocationList& wordLocations_,
	ModUnicodeString& normalized_)
{
	ModUnicodeString morph;

	setUnaParameter(ModTrue, langSet_);
	una->set(target_, langSet_);
	
	// 特徴語候補
  	ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >
		featureCandidate;
  	ModVector<ModUnicodeString> formVector;	// 形態素語形ベクトル
	ModVector<ModUnicodeString> ostrVector;	// 形態素語形ベクトル(元表記)
	ModVector<int>				posVector;  // 形態素品詞ベクトル
	ModVector<int>				costVector;	// 単語コストベクトル
	ModVector<int>				uposVector;	// UNA 統合品詞
	
  	while (una->getBlock(formVector, ostrVector,
						 posVector, costVector, uposVector) == ModTrue)
	{
		ModVector<ModUnicodeString>::Iterator form = formVector.begin();
		ModVector<int>::Iterator pos  = posVector.begin();
		ModVector<int>::Iterator cost  = costVector.begin();
		ModVector<int>::Iterator upos = uposVector.begin();

		for (; pos != posVector.end(); ++pos, ++form, ++cost, ++upos)
		{
			// 形態素を特徴語候補に設定
			setFeatureCandidate(featureCandidate, *pos, *form, *cost, *upos);

			// 形態素がある限り処理を続ける
			if (form->getLength() > 0) {
				if (currentLocation_ == 0) {
					// 先頭位置を記録する
					wordLocations_.pushBack(++currentLocation_);
				}
				wordLocations_.pushBack(currentLocation_ += form->getLength());
				normalized_.append(*form);
			}
		}
	}
	extractFeature(featureCandidate);
}

void
ModInvertedDualTokenizer::extractFeature(
	ModMap<ModUnicodeString, ModPair<int, int>, ModLess<ModUnicodeString> >&
	featureCandidate_)
{
	// 特徴語領域のclear
	m_pFeature->clear();
	
	ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >
		::Iterator i0 = featureCandidate_.begin();
	ModMap<ModUnicodeString,ModPair<int,int>,ModLess<ModUnicodeString> >
		::Iterator e0 = featureCandidate_.end();
	
	for (; i0 != e0; ++i0)
	{
		// 特徴語の重みは、log(TF+1)*単語生起コスト
		// [NOTE] ここでは重みを正規化しない。
		//  参照 FullText::LogicalInterface::makeFeatureFieldData()
		m_pFeature->pushBack(
			ModInvertedFeatureElement(
				(*i0).first,
				LOG((*i0).second.first + 1) * (*i0).second.second));
	}
	
	// 特徴語の重みをキーとして降順にソート
	ModSort(m_pFeature->begin(), m_pFeature->end(), _WeightGreater());
	
	// ここで上位n件を特徴語とする。
	// m_featureSizeは、システムパラメータとして、index作成時に指定する
	// defaultは10
	// 特徴語の重みが等しい場合は、途中でcutしない
	ModInvertedFeatureList::Iterator b1 = m_pFeature->begin();
	ModInvertedFeatureList::Iterator i1 = m_pFeature->begin();
	ModInvertedFeatureList::Iterator e1 = m_pFeature->end();
	for (; i1 != e1; ++i1)
	{
		// 特徴語の重みが前後で等しい場合は、指定された特徴語数(n)に達した場合
		// でもループからぬけない
		// ただし、特徴語数の２倍を超えたらぬける
		
		if ((i1 + 1) < e1 && (*(i1 + 1)).second == (*i1).second
			&& (i1 - b1) < static_cast<int>(m_featureSize) * 2)
			continue;
		if ((i1 - b1) >= static_cast<int>(m_featureSize))
			break;
	}
	
	// 残りの特長語を削除する
	if (i1 != e1)
		m_pFeature->erase(i1, e1);
}

//
// FUNCTION
// ModInvertedDualTokenizer::prepareNormalizedText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& target_
//		もとテキスト
// ModVector<ModLanguageSet>* langSet_
//		言語指定（v1_6のみ）
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
// ModInvertedLocationList& wordLocations_,
//		単語境界位置
// ModUnicodeString& normalized_
//		正規化テキスト
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::prepareNormalizedText(
	const ModUnicodeString& target_,
#ifdef V1_6
	ModVector<ModLanguageSet>* langSets_,
#endif // V1_6
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_,
	ModInvertedLocationList& wordLocations_,
	ModUnicodeString& normalized_)
{
	// 正規化表記の領域確保
	normalized_.reallocate(target_.getLength());

	ModSize currentLocation(0);

	if (byteOffsets_ == 0) {
		// セクションがない場合

		(this->*prepareNormalizedTextSub)(target_, currentLocation,
#ifdef V1_6
								 langSets_->getBack(),
#endif // V1_6
								 wordLocations_, normalized_);
	} else {
		// セクションがある場合
		; ModAssert(charOffsets_ != 0);
		charOffsets_->clear();

		ModUnicodeString section;

		ModSize beginOffset(0), endOffset;
		ModVector<ModSize>::ConstIterator b(byteOffsets_->begin());
#ifdef V1_6
		ModVector<ModLanguageSet>::Iterator
			langSetIter(langSets_->begin());
#endif // V1_6
		for (; b != byteOffsets_->end(); ++b) {
			endOffset = (*b)/2;
			if (endOffset < beginOffset) {
				ModErrorMessage << "invalid character offset: "
								<< beginOffset << ' ' << endOffset << " at "
								<< (b - byteOffsets_->begin()) << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorInternal);
			}
			if (endOffset == beginOffset) {
				// copy は第２引数が 0 だとまるごとコピーしてしまうので、
				// 分ける。文字オフセットをセットするだけ。
				if (currentLocation == 0) {
					// 最初の場合、文字オフセットは文字数＋１なので１を記録
					charOffsets_->pushBack(1);
				} else {
					charOffsets_->pushBack(currentLocation);
				}
#ifdef V1_6
				++langSetIter;
#endif // V1_6
				continue;
			}

			section = target_.copy(beginOffset, endOffset - beginOffset);
			beginOffset = endOffset;

			(this->*prepareNormalizedTextSub)(section, currentLocation,
#ifdef V1_6
									 *langSetIter,
#endif // V1_6
									 wordLocations_, normalized_);

			charOffsets_->pushBack(currentLocation);
#ifdef V1_6
			++langSetIter;
#endif // V1_6
		}
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::getNormalizedText -- 正規化テキストの用意
//
// NOTES
// 正規化テキストの用意する。
// 必要であれば、バイト位置の配列から文字位置の配列への変換も行う。
//
// ARGUMENTS
// const ModUnicodeString& orignal_
//		もとテキスト
// ModVector<ModLanguageSet>* langSet_
//		言語指定（v1_6のみ）
// const ModVector<ModSize>* byteOffsets_
//		バイト位置の配列
// ModVector<ModSize>* characterOffsets_
//		文字位置をセットする配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModUnicodeString*
ModInvertedDualTokenizer::getNormalizedText(
	const ModUnicodeString& original_,
#ifdef V1_6
	ModVector<ModLanguageSet>* langSet_,
#endif
	const ModVector<ModSize>* byteOffsets_,
	ModVector<ModSize>* charOffsets_)
{
	ModAutoPointer<ModUnicodeString> result(new ModUnicodeString);
	ModInvertedLocationList wordLocations;

	prepareNormalizedText(original_,
#ifdef V1_6
						  langSet_,
#endif // V1_6
						  byteOffsets_, charOffsets_,
						  wordLocations, *result);
	
	return result.release();
}

//
// FUNCTION
// ModInvertedDualTokenizer::tokenizeSub -- トークン分割の下請
//
// NOTES
// 分割できた場合は ModTrue が返る。
// 分割対象が短すぎて分割できない場合は ModFalse が返り、分割対象を
// OR展開で検索すべき先頭および末尾の文字列を from, to で返す。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// ModInvertedLocationListMap& result_
//		分割結果
// ModSize& tokenizedEnd_
//		通常処理の終了位置
// ModUnicodeString& shortWord_
//		ショートワード部分の文字列
// ModUnicodeString& from_
//		ショートワード処理で追加的に展開すべき先頭の文字列
// ModUnicodeString& to_
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
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedDualTokenizer::tokenizeSub(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
	ModInvertedLocationListMap& result_,
	ModSize& tokenizedEnd_,
	ModUnicodeString& shortWord_,
	ModUnicodeString& from_,
	ModUnicodeString& to_
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	return tokenizeSub2(target_,
						mode_,
						result_,
						tokenizedEnd_,
						shortWord_,
						from_,
						to_,
						0
#ifdef V1_6
						, langSet_
#endif // V1_6
						);
}

//
// FUNCTION
// ModInvertedDualTokenizer::tokenizeSub2 -- トークン分割の下請
//
// NOTES
// 分割できた場合は ModTrue が返る。
// 分割対象が短すぎて分割できない場合は ModFalse が返り、分割対象を
// OR展開で検索すべき先頭および末尾の文字列を from, to で返す。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// ModInvertedLocationListMap& result_
//		分割結果
// ModSize& tokenizedEnd_
//		通常処理の終了位置
// ModUnicodeString& shortWord_
//		ショートワード部分の文字列
// ModUnicodeString& from_
//		ショートワード処理で追加的に展開すべき先頭の文字列
// ModUnicodeString& to_
//		ショートワード処理で追加的に展開すべき末尾の文字列
// QueryTokenizedResult*& result
//		分割結果の配列
// ModUnicodeString* normalizedTarget_
//		正規化結果のテキスト
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// ショートワード処理が不要ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModBoolean
ModInvertedDualTokenizer::tokenizeSub2(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
	ModInvertedLocationListMap& result_,
	ModSize& tokenizedEnd_,
	ModUnicodeString& shortWord_,
	ModUnicodeString& from_,
	ModUnicodeString& to_,
	ModUnicodeString* normalizedTarget_
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
#ifdef V1_6
	ModVector<ModLanguageSet> langSets;
	langSets.pushBack(langSet_);
#endif // V1_6

	if ((mode_&wordIndexingOnly) != 0) {
		// 単語分割結果のみの場合
		//		- この中で正規化は行われる（ここに来るのは展開が不要な場合）
		wordTokenize(target_, mode_, result_, tokenizedEnd_, 0, 0,
					 normalizedTarget_
#ifdef V1_6
					, &langSets
#endif // V1_6
					);

		return ModTrue;	// ショートワード処理は不要
	}

	ModBoolean ret;
	ModInvertedLocationList wordLocations;

	if (normalizing == ModTrue && (mode_&skipNormalizing) == 0) {
		// 正規化する場合

		// 正規化テキストを作成する -- wordLocations は不要かもしれない
		prepareNormalizedText(target_, 
#ifdef V1_6
							  &langSets,
#endif // V1_6
							  0, 0, wordLocations, *normalizedTarget_);
	
		// n-gram 分割
		ret = ModInvertedBlockedNgramTokenizer::tokenizeSub(
			*normalizedTarget_,
			TokenizeMode(mode_|skipNormalizing),
			result_, tokenizedEnd_, shortWord_, from_, to_
#ifdef V1_6
			, langSet_
#endif // V1_6
			);

	} else {
		// 正規化しない場合

		if ((mode_&ngramIndexingOnly) == 0) {
			// 単語の情報を追加する
			prepareText(target_,
#ifdef V1_6
						&langSets,
#endif // V1_6
						0, 0, wordLocations);
		}

		// n-gram 分割
		ret = ModInvertedBlockedNgramTokenizer::tokenizeSub(
			target_, 
			TokenizeMode(mode_|skipNormalizing),
			result_, tokenizedEnd_, shortWord_, from_, to_
#ifdef V1_6
			, langSet_
#endif // V1_6
			);
	}

	if ((mode_&ngramIndexingOnly) == 0) {
		mergeResult(result_, wordLocations);

#ifdef DEBUG
		ModDebugMessage << "WordLocation: ";
		for (ModInvertedLocationList::Iterator i(wordLocations.begin());
			 i != wordLocations.end(); ++i) {
			ModDebugMessage << ' ' << *i;
		}
		ModDebugMessage << ModEndl;
#endif // DEBUG
	}

	return ret;
}

//
// FUNCTION
// ModInvertedDualTokenizer::expandSubSingle -- 展開の下請け（単数用）
//
// NOTES
// expand の下請け関数。
// 新たな単語（形態素）が展開されない場合、それまでの結果の各々に
// 単語をアペンドする。
// 
// ARGUMENTS
// const ModUnicodeString& morph_
//		単語表記
// ModVector<ModUnicodeString>& textVector_
//		展開文字列の配列
// ModVector<ModInvertedLocationList>& wlocVector_
//		分割結果の配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::expandSubSingle(
	const ModUnicodeString& morph_,
	ModVector<ModUnicodeString>& textVector_,
	ModVector<ModInvertedLocationList>& wlocVector_)
{
	// 展開表記をアペンドする
	for (ModVector<ModUnicodeString>::Iterator t(textVector_.begin());
		 t != textVector_.end(); ++t) {
		(*t).append(morph_);
	}
	// 出現位置をアペンドする
	for (ModVector<ModInvertedLocationList>::Iterator
			 w(wlocVector_.begin());
		 w != wlocVector_.end(); ++w) {
		; ModAssert((*w).getSize() != 0);
		(*w).pushBack(*((*w).end() - 1) + morph_.getLength());
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::expandSubMulti -- 展開の下請け（複数用）
//
// NOTES
// expand の下請け関数。
// 新たな単語（形態素）が複数の表記に展開された場合、それまでの結果の各々に
// 展開表記の各々をアペンドする。
// 
// ARGUMENTS
// const ModSize num_
//		単語の展開数
// const ModUnicodeString* const expanded_
//		単語の展開結果
// ModVector<ModUnicodeString>& textVector_
//		展開文字列の配列
// ModVector<ModInvertedLocationList>& wlocVector_
//		分割結果の配列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::expandSubMulti(
#ifdef SYD_USE_UNA_V10
	const ModVector<ModUnicodeString>& expanded_,
#else
	const ModSize num_,
	const ModUnicodeString* const expanded_,
#endif
	ModVector<ModUnicodeString>& textVector_,
	ModVector<ModInvertedLocationList>& wlocVector_)
{
#ifdef SYD_USE_UNA_V10
	ModSize num_ = expanded_.getSize();
#endif
	ModVector<ModUnicodeString> ttmpVector(textVector_);
	ModVector<ModInvertedLocationList> wtmpVector(wlocVector_);

	// 展開表記をアペンドする
	for (ModVector<ModUnicodeString>::Iterator t(textVector_.begin());
		 t != textVector_.end(); ++t) {
		(*t).append(expanded_[0]);
	}
	// 出現位置をアペンドする
	for (ModVector<ModInvertedLocationList>::Iterator
			 w(wlocVector_.begin());
		 w != wlocVector_.end(); ++w) {
		; ModAssert((*w).getSize() != 0);
		(*w).pushBack(*((*w).end() - 1) + expanded_[0].getLength());
	}

	for (ModSize n(1); n < num_; ++n) {
		// 展開表記をアペンドする
		for (ModVector<ModUnicodeString>::Iterator t(ttmpVector.begin());
			 t != ttmpVector.end(); ++t) {
			// 配列の要素を増やし
			ModVector<ModUnicodeString>::Iterator
				t2(textVector_.insert(textVector_.end(), *t));
			// 増やしたところに展開表記をアペンド
			(*t2).append(expanded_[n]);
		}
		// 出現位置をアペンドする
		for (ModVector<ModInvertedLocationList>::Iterator
				 w(wtmpVector.begin());
			 w != wtmpVector.end(); ++w) {
			// 配列の要素を増やし
			ModVector<ModInvertedLocationList>::Iterator
				w2(wlocVector_.insert(wlocVector_.end(), *w));
			; ModAssert((*w).getSize() != 0);
			// 増やしたところに出現位置をアペンド
			(*w2).pushBack(*((*w2).end() - 1) + expanded_[n].getLength());
		}
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::expand -- 展開
//
// NOTES
// デュアル索引時に異表記正規化モードで展開が必要な場合に、テキストを単語に
// 分割した上で、異表記展開およびステマー展開を行なう。
//
// ab が a,b に分割され、a, b がそれぞれ {a,A}, {b, B} に展開される場合、
// {ab, Ab, aB, AB} が出力される。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// QueryTokenizedResult*& result_
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// 展開数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedDualTokenizer::expand(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
#ifdef INV_TOK_USEVECTOR
	ModVector<QueryTokenizedResult>& result_
#else
	ModInvertedQueryTokenizedResult*& result_
#endif
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	// ここに来るのは正規化する場合だけ

	ModVector<ModUnicodeString>
		textVector(1, ModUnicodeString());
	ModVector<ModInvertedLocationList>
		wlocVector(1, ModInvertedLocationList(1, 1));
	ModUnicodeString morph;
	ModSize num(0);

#ifdef SYD_USE_UNA_V10
	ModVector<ModUnicodeString> expanded;

	ModUnicodeString dummy1;
	int dummy2;

	// UNAパラメータを設定する
	setUnaParameter(ModTrue, langSet_);
	una->set(target_, langSet_);
	while (una->getExpandWords(expanded, dummy1, dummy2) == ModTrue)
	{
		// 形態素がある限り処理を続ける
		if (expanded.getSize() > 1) {
			// 異表記で展開されるのはカタカナ文字列なので、ステマーの
			// 展開関数をあらためて呼ぶことはしない
			expandSubMulti(expanded, textVector, wlocVector);
		} else {
			// 展開されない場合
			expandSubSingle(expanded[0], textVector, wlocVector);
		}
	}
#else
#if defined(UNA_V3_3) || defined(UNA_V3_4)
	ModUnicodeString* expanded = 0;

	una->set(target_, unaSetMode
#if defined(UNA_V3_4) && defined(V1_6)
			 , langSet_
#endif // UNA_V3_4 & V1_6	
			 , unaModifierMode
			);
	while ((num = una->getExpandWords(expanded)) != 0) {
		// 形態素がある限り処理を続ける
#ifdef DEBUG
		ModDebugMessage << morph << " num=" << num << ModEndl;
#endif

		if (num > 1) {
			// 異表記で展開されるのはカタカナ文字列なので、ステマーの
			// 展開関数をあらためて呼ぶことはしない
			expandSubMulti(num, expanded, textVector, wlocVector);
		} else {
			if (num == 1) {
				// 正規化された表記が 0 番目の要素に入っているので、
				// morph を置き換える（これはあり得ない気もする）
				morph = expanded[0];
			}
			// 展開されない場合
			expandSubSingle(morph, textVector, wlocVector);
		}
		delete [] expanded;
	}
#else // UNA_V3_3 || UNA_V3_4
	una->set(target_, (stemmer != 0));	// ステマーがあれば使う（下位構造も）
	while (una->getMorph(morph, ModBoolean(!(mode_&skipNormalizing)))
		   == ModTrue) {
		// 形態素がある限り処理を続ける
		if (morph.getLength() == 0) {
			// 正規化で長さ 0 となったものは無視する
			continue;
		}

		// 正規化はスキップして、展開だけ行なう
		ModUnicodeString* expanded = 0;
		try {
			num = normalizer->expandBuf(morph, expanded, ModNormExpNoChk,
										0, 0, ModTrue);
#ifdef DEBUG
			ModDebugMessage << morph << " num=" << num << ModEndl;
#endif

			if (num > 1) {
				// 異表記で展開されるのはカタカナ文字列なので、ステマーの
				// 展開関数をあらためて呼ぶことはしない
				expandSubMulti(num, expanded, textVector, wlocVector);
			} else {
				if (num == 1) {
					// 正規化された表記が 0 番目の要素に入っているので、
					// morph を置き換える（これはあり得ない気もする）
					morph = expanded[0];
				}
				ModVector<ModUnicodeString> reexpanded;
				if (stemmer->expand(morph, reexpanded) == ModTrue) {
					// 展開された場合
					expandSubMulti(reexpanded.getSize(),
								   reexpanded.begin(),
								   textVector, wlocVector);
				} else {
					// 展開されない場合
					expandSubSingle(morph, textVector, wlocVector);
				}
			}
			delete [] expanded;

		} catch (ModException& e) {
			ModErrorMessage << "wordExpandSubXXX failed: " << e << ModEndl;
			delete [] expanded;
			ModRethrow(e);
		}
	}
#endif // UNA_V3_3 || UNA_V3_4
#endif // SYD_USE_UNA_V10

	// ここで OR 展開する必要のないものがあるかを調べるとよい

	num = textVector.getSize();
	; ModAssert(num != 0);
	; ModAssert(num == wlocVector.getSize());
	; ModAssert((mode_&skipExpansion) == 0);

#ifdef INV_TOK_USEVECTOR
	if (result_.getSize() > num) {
		result_.clear();
	}
#else
	result_ = new QueryTokenizedResult[num];
#endif

	for (ModSize n(0); n < num; ++n) {
#ifdef INV_TOK_USEVECTOR
		if (result_.getSize() < n) {
			result_.pushBack(QueryTokenizedResult());
		}
#endif
#ifdef DEBUG
		ModDebugMessage << "expanded: " << n << ' ' << textVector[n];
		ModInvertedLocationList& wloc = wlocVector[n];
		for (ModInvertedLocationList::Iterator w(wloc.begin());
			 w != wloc.end(); ++w) {
			ModDebugMessage << ' ' << *w;
		}
#if defined(UNA_V3_4) && defined(V1_6)
		ModDebugMessage << " langSet:"
						<< langSet_.getName().getString(ModKanjiCode::literalCode);
#endif // UNA_V3_4 & V1_6
		ModDebugMessage << ModEndl;
#endif // DEBUG

		// この場合、expand() のなかで正規化されているので、正規化を抑制する
		result_[n].isNormalWord
			= ModInvertedBlockedNgramTokenizer::tokenizeSub(
				textVector[n],
				TokenizeMode(mode_|skipNormalizing),
				result_[n].locationListMap,
				result_[n].tokenizedEnd,
				result_[n].shortWord,
				result_[n].fromWord,
				result_[n].toWord
#ifdef V1_6
				, langSet_
#endif // V1_6
				);
		result_[n].target = textVector[n];

		if ((mode_&ngramIndexingOnly) == 0) {
			// 単語境界位置をマージする
			mergeResult(result_[n].locationListMap, wlocVector[n]);
		}
	}

	return num;
}

//
// FUNCTION
// ModInvertedDualTokenizer::wordExpandSubSingle -- 単語展開の下請け（単数用）
//
// NOTES
// wordExpand の下請け関数。
// 新たな単語（形態素）が展開されない場合、それまでの結果の各々に
// 単語をアペンドする。
// 
// ARGUMENTS
// const ModUnicodeString morph_
//		単語表記
// ModSize& rnum_
//		分割結果の個数
// QueryTokenizedResult*& result_
//		分割結果の配列
// const ModSize currentLocation_
//		現在位置（先頭からの単語オフセット）
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::wordExpandSubSingle(
	const ModUnicodeString& morph_,
	const ModSize rnum_,
	ModInvertedQueryTokenizedResult*& result_,
	const ModSize currentLocation_)
{
	ModInvertedSmartLocationList tmp;

	for (ModSize r(0); r < rnum_; ++r) {
		// 出現位置をアペンドする（単語オフセットなので全てで同じ）
		ModInvertedLocationListMap::Iterator
			pair(result_[r].locationListMap.find(morph_));
		if (pair == result_[r].locationListMap.end()) {
			// 見つからない
			tmp.setCoder(file->getLocationCoder(morph_));
			tmp.setFirstValue(currentLocation_);
			result_[r].locationListMap.insert(morph_, tmp);
		} else {
			(*pair).second.pushBack(currentLocation_);
		}

		// 展開表記をアペンドする
		result_[r].target.append(morph_);
	}
}

//
// FUNCTION
// ModInvertedDualTokenizer::wordExpandSubMulti -- 単語展開の下請け（複数用）
//
// NOTES
// wordExpand の下請け関数。
// 新たな単語（形態素）が複数の表記に展開された場合、それまでの結果の各々に
// 展開表記の各々をアペンドする。
// 
// ARGUMENTS
// const ModSize num_
//		単語の展開数
// const ModUnicodeString* const expanded_
//		単語の展開結果
// ModSize& rnum_
//		分割結果の個数
// QueryTokenizedResult*& result_
//		分割結果の配列
// const ModSize currentLocation_
//		現在位置（先頭からの単語オフセット）
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedDualTokenizer::wordExpandSubMulti(
#ifdef SYD_USE_UNA_V10
	const ModVector<ModUnicodeString>& expanded_,
#else
	const ModSize num_,
	const ModUnicodeString* const expanded_,
#endif
	ModSize& rnum_,
	ModInvertedQueryTokenizedResult*& result_,
	const ModSize currentLocation_)
{
#ifdef SYD_USE_UNA_V10
	ModSize num_ = expanded_.getSize();
#endif
	ModInvertedSmartLocationList tmp;
	ModAutoPointer<ModInvertedQueryTokenizedResult>
		tmpResult(new QueryTokenizedResult[rnum_*num_]);

	for (ModSize n(0); n < num_; ++n) {
		for (ModSize r(0); r < rnum_; ++r) {
			tmpResult[n*rnum_ + r] = result_[r];

			// 出現位置をアペンドする（単語オフセットなので全てで同じ）
			ModInvertedLocationListMap::Iterator
				pair(tmpResult[n*rnum_ + r].locationListMap.find(
					expanded_[n]));
			if (pair == tmpResult[n*rnum_ + r].locationListMap.end()) {
				// 見つからない
				tmp.setCoder(file->getLocationCoder(expanded_[n]));
				tmp.setFirstValue(currentLocation_);
				tmpResult[n*rnum_ + r].locationListMap.insert(
					expanded_[n], tmp);
			} else {
				(*pair).second.pushBack(currentLocation_);
			}

			// 展開表記をアペンドする
			tmpResult[n*rnum_ + r].target.append(expanded_[n]);
		}
	}

	delete [] result_;
	result_ = tmpResult.release();
	rnum_ *= num_;
}

//
// FUNCTION
// ModInvertedDualTokenizer::wordExpand -- 単語展開
//
// NOTES
// 単語索引時に異表記正規化モードで展開が必要な場合に、テキストを単語に
// 分割した上で、異表記展開およびステマー展開を行なう。
//
// ab が a,b に分割され、a, b がそれぞれ {a,A}, {b, B} に展開される場合、
// {ab, Ab, aB, AB} が出力される。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// QueryTokenizedResult*& result_
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// 展開数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedDualTokenizer::wordExpand(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
#ifdef INV_TOK_USEVECTOR
	ModVector<QueryTokenizedResult>& result_
#else
	ModInvertedQueryTokenizedResult*& result_
#endif
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	// ここに来るのは正規化する場合だけ

	ModVector<ModUnicodeString> textVector(1, ModUnicodeString());

	ModUnicodeString morph;
	ModSize num(0), rnum(1), n, currentLocation(0);
	ModInvertedSmartLocationList tmp;

	result_ = new QueryTokenizedResult[rnum];		// とりあえず１にする
	result_[0].isNormalWord = ModTrue;

#ifdef SYD_USE_UNA_V10
	ModVector<ModUnicodeString> expanded;

	ModUnicodeString dummy1;
	int dummy2;

	// UNAパラメータを設定する
	setUnaParameter(ModTrue, langSet_);
	una->set(target_, langSet_);
	while (una->getExpandWords(expanded, dummy1, dummy2) == ModTrue)
	{
		// 形態素がある限り処理を続ける

		++currentLocation;
		
		if (expanded.getSize() > 1) {
			// 異表記で展開されるのはカタカナ文字列なので、ステマーの
			// 展開関数をあらためて呼ぶことはしない
			wordExpandSubMulti(expanded, rnum, result_,
							   currentLocation);
		} else {
			// 展開されない場合
			wordExpandSubSingle(expanded[0], rnum, result_, currentLocation);
		}
	}
#else
#if defined(UNA_V3_3) || defined(UNA_V3_4)
	ModUnicodeString* expanded = 0;

	una->set(target_, unaSetMode
#if defined(UNA_V3_4) && defined(V1_6)
			 , langSet_
#endif // UNA_V3_4 & V1_6
			 , unaModifierMode
			);

	while ((num = una->getExpandWords(expanded)) != 0) {
		// 形態素がある限り処理を続ける
#ifdef DEBUG
		ModDebugMessage << "num=" << num << ':';
		for (n = 0; n < num; ++n) {
			ModDebugMessage << ' ' << expanded[n];
		}
		ModDebugMessage << ModEndl;
#endif
		++currentLocation;

		if (num > 1) {
			// 異表記で展開されるのはカタカナ文字列なので、ステマーの
			// 展開関数をあらためて呼ぶことはしない
			wordExpandSubMulti(num, expanded, rnum, result_,
							   currentLocation);
		} else {
			if (num == 1) {
				// 正規化された表記が 0 番目の要素に入っているので、
				// morph を置き換える（これはあり得ない気もする）
				morph = expanded[0];
			}
			// 展開されない場合
			wordExpandSubSingle(morph, rnum, result_, currentLocation);
		}
		delete [] expanded;
	}
#else // UNA_V3_3 || UNA_V3_4
	una->set(target_, (stemmer != 0));	// ステマーがあれば使う（下位構造も）

	while (una->getMorph(morph, ModBoolean(!(mode_&skipNormalizing)))
		   == ModTrue) {
		// 形態素がある限り処理を続ける
		if (morph.getLength() == 0) {
			// 正規化で長さ 0 となったものは無視する
			continue;
		}

		// 正規化はスキップして、展開だけ行なう
		ModUnicodeString* expanded = 0;
		try {
			num = normalizer->expandBuf(morph, expanded, ModNormExpNoChk,
										0, 0, ModTrue);
#ifdef DEBUG
			ModDebugMessage << morph << " num=" << num << ModEndl;
#endif
			++currentLocation;

			if (num > 1) {
				// 異表記で展開されるのはカタカナ文字列なので、ステマーの
				// 展開関数をあらためて呼ぶことはしない
				wordExpandSubMulti(num, expanded, rnum, result_,
								   currentLocation);
			} else {
				if (num == 1) {
					// 正規化された表記が 0 番目の要素に入っているので、
					// morph を置き換える（これはあり得ない気もする）
					morph = expanded[0];
				}
				ModVector<ModUnicodeString> reexpanded;
				if (stemmer->expand(morph, reexpanded) == ModTrue) {
					// 展開された場合
					wordExpandSubMulti(reexpanded.getSize(),
									   reexpanded.begin(),
									   rnum, result_, currentLocation);
				} else {
					// 展開されない場合
					wordExpandSubSingle(morph, rnum, result_, currentLocation);
				}
			}
			delete [] expanded;

		} catch (ModException& e) {
			ModErrorMessage << "wordExpandSubXXX failed: " << e << ModEndl;
			delete [] expanded;
			ModRethrow(e);
		}
	}
#endif // UNA_V3_3 || UNA_V3_4
#endif // SYD_USE_UNA_V10

	// ここで OR 展開する必要のないものがあるかを調べるとよい

	for (n = 0; n < rnum; ++n) {
		result_[n].tokenizedEnd = currentLocation;
#ifdef DEBUG
		ModDebugMessage << "expanded: " << n << ' ' << result_[n].target
						<< ": lnum=" << result_[n].locationListMap.getSize()
						<< ':';
		ModInvertedLocationListMap::Iterator
			pair(result_[n].locationListMap.begin());
		for (; pair != result_[n].locationListMap.end(); ++pair) {
			ModDebugMessage << ' ' << (*pair).first << " (";
			ModAutoPointer<ModInvertedLocationListIterator> w((*pair).second.begin());
			for (; w->isEnd() == ModFalse; w->next()) {
				ModDebugMessage << ' ' << w->getLocation();
			}
			ModDebugMessage << ')';
		}
		ModDebugMessage << " end=" << result_[n].tokenizedEnd << ModEndl;
#endif // DEBUG
	}

	return rnum;
}

//
// FUNCTION
// ModInvertedDualTokenizer::tokenizeMulti -- トークン分割
//
// NOTES
// トークン分割を行う。
// 異表記展開が行われた場合、その個数分の結果を生成する。
// 分割結果を呼び出し側が delete しなければならない。
// 
// ARGUMENTS
// const ModUnicodeString& target_
//		分割対象テキスト
// const TokenizeMode mode_
//		分割モード
// QueryTokenizedResult*& result_
//		分割結果の配列
// const ModLanguageSet& langSet_
//		言語指定（v1_6のみ）
//
// RETURN
// 分割結果の配列の個数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedDualTokenizer::tokenizeMulti(
	const ModUnicodeString& target_,
	const ModInvertedTokenizer::TokenizeMode mode_,
#ifdef INV_TOK_USEVECTOR
	ModVector<QueryTokenizedResult>& result_
#else
	ModInvertedQueryTokenizedResult*& result_
#endif
#ifdef V1_6
	, const ModLanguageSet& langSet_
#endif // V1_6
	)
{
	ModInvertedLocationList wordLocations;
	ModUnicodeString normalizedTarget;

	if (normalizing == ModTrue) {

		if ((mode_&skipExpansion) == 0) {
			// 展開を行う
			// 第６引数で正規化を行うか否かを制御（true なら展開だけを実施）

			if ((mode_&wordIndexingOnly) == 0) {
				// 単語分割結果以外の場合
				return expand(target_, mode_, result_
#ifdef V1_6
							  , langSet_
#endif // V1_6
							  );

			} else {
				// 単語分割結果のみの場合
				return wordExpand(target_, mode_, result_
#ifdef V1_6
								  , langSet_
#endif // V1_6
								  );
			}
		}

#ifdef INV_TOK_USEVECTOR
		switch (result_.getSize()) {
		case 0:
			result_.pushBack(QueryTokenizedResult());
			break;
		case 1:
			// 何もしない
			break;
		default:
			result_.clear();
			result_.pushBack(QueryTokenizedResult());
			break;
		}
		; ModAssert(result_.getSize() == 1);
#else
		result_ = new QueryTokenizedResult[1];
#endif

		result_[0].isNormalWord
			= tokenizeSub2(target_,
						   mode_,
						   result_[0].locationListMap,
						   result_[0].tokenizedEnd,
						   result_[0].shortWord,
						   result_[0].fromWord,
						   result_[0].toWord,
						   &(result_[0].target)
#ifdef V1_6
						   , langSet_
#endif // V1_6
						   );
	} else {
		// 正規化しない場合
#ifdef INV_TOK_USEVECTOR
		switch (result_.getSize()) {
		case 0:
			result_.pushBack(QueryTokenizedResult());
			break;
		case 1:
			// 何もしない
			break;
		default:
			result_.clear();
			result_.pushBack(QueryTokenizedResult());
			break;
		}
		; ModAssert(result_.getSize() == 1);
#else
		result_ = new QueryTokenizedResult[1];
#endif

		result_[0].isNormalWord
			= tokenizeSub2(target_,
						   mode_,
						   result_[0].locationListMap,
						   result_[0].tokenizedEnd,
						   result_[0].shortWord,
						   result_[0].fromWord,
						   result_[0].toWord,
						   0
#ifdef V1_6
						   , langSet_
#endif // V1_6
						   );

		result_[0].target = target_;
	}

	return 1;
}

//
// FUNCTION
// ModInvertedDualTokenizer::isSupported -- 索引付けタイプの判定
//
// NOTES
// 与えられた索引付けタイプをサポートするか否かを判定する。
//
// ARGUMENTS
// const ModInvertedFileIndexingType indexingType_
//		索引付けタイプ
//
// RETURN
// サポートする索引付けタイプなら ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedDualTokenizer::isSupported(
	const ModInvertedFileIndexingType indexingType_) const
{
	return ModTrue;
}

//
// FUNCTION
// ModInvertedDualTokenizer::isPrenormalizable -- プレノーマライズが適用可能かの判定
//
// NOTES
// プレノーマライズが適用可能か否かを判定する。
// このクラスは常に適用不可能。
//
// ARGUMENTS
// なし
//
// RETURN
// 常に ModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModInvertedDualTokenizer::isPrenormalizable() const
{
	return ModFalse;
}

//
// FUNCTION
// ModInvertedDualTokenizer::getTokenLength -- トークン長の取得
//
// NOTES
// 渡されたトークン（文字列）の長さ（文字数）を取得する。
// n-gram 索引用。単語索引用には、サブクラスでオーバーライトすること。
//
// ARGUMENTS
// const ModUnicodeString& string
//		トークン
//
// RETURN
// トークンの長さ
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize
ModInvertedDualTokenizer::getTokenLength(const ModUnicodeString& string) const
{
	if (file->getIndexingType() == ModInvertedFileWordIndexing) {
		return (string.getLength() == 0) ? 0 : 1;
	}
	return ModInvertedTokenizer::getTokenLength(string);
}

//
// FUNCTION
// ModInvertedDualTokenizer::getAnalyzer -- 解析器の作成
//
// NOTES
// 解析器を作成する。
// 引数で与えられた文字列中のリソースIDを、解析器作成関数を用いて作成する。
//
// ARGUMENTS
// const ModString& description_
//		記述文字列
//
// RETURN
// 解析器
//
// EXCEPTIONS
// ModInvertedErrorGetAnalyzerFail
//		解析器作成に失敗した
//
#if defined(UNA_V3_3) || defined(UNA_V3_4)
#ifdef SYD_USE_UNA_V10
UNA::ModNlpAnalyzer*
#else
ModNlpAnalyzer*
#endif
ModInvertedDualTokenizer::getAnalyzer(const ModCharString& description_)
{
	if (una == 0) {
		if (unaRscID == 0) {
			unaRscID = getResourceID(description_, analyzerResourceID);
		}
#ifdef DEBUG
		ModDebugMessage << analyzerResourceID << '=' << unaRscID << ModEndl;
#endif
		if (getAnalyzerFunc == 0 ||
			(una = (*getAnalyzerFunc)(unaRscID)) == 0) {
			ModErrorMessage << "getAnalyzer failed: "
							<< intptr_t(getAnalyzerFunc) << ' '
							<< intptr_t(una) << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorGetNormalizerFail);
		}
	}

	; ModAssert(una != 0);
	return una;
}
#else // UNA_V3_3 || UNA_V3_4
ModUnaAnalyzer*
ModInvertedDualTokenizer::getAnalyzer(const ModCharString& description_)
{
	if (una == 0) {
		if (unaRscID == 0) {
			unaRscID = getResourceID(description_, analyzerResourceID);
		}
#ifdef DEBUG
		ModDebugMessage << analyzerResourceID << '=' << unaRscID << ModEndl;
#endif
		if (getAnalyzerFunc == 0 ||
			(una = (*getAnalyzerFunc)(unaRscID, normalizer, stemmer)) == 0) {
			ModErrorMessage << "getAnalyzer failed: "
							<< intptr_t(getAnalyzerFunc) << ' '
							<< intptr_t(una) << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorGetNormalizerFail);
		}
	}

	; ModAssert(una != 0);
	return una;
}
#endif // UNA_V3_3 || UNA_V3_4

#if !(defined(UNA_V3_3) || defined(UNA_V3_4))
//
// FUNCTION
// ModInvertedDualTokenizer::getStemmer -- ステマーの作成
//
// NOTES
// ステマーを作成する。
// 引数で与えられた文字列中のリソースIDを、ステマー作成関数を用いて作成する。
//
// ARGUMENTS
// const ModString& description_
//		記述文字列
//
// RETURN
// ステマー
//
// EXCEPTIONS
// ModInvertedErrorGetStemmerFail
//		ステマー作成に失敗した
//
ModEnglishWordStemmer*
ModInvertedDualTokenizer::getStemmer(const ModCharString& description_)
{
	if (stemmer == 0) {
		if (stemmerRscID == 0) {
			stemmerRscID = getResourceID(description_, stemmerResourceID);
		}
#ifdef DEBUG
		ModDebugMessage << stemmerResourceID << '=' << stemmerRscID << ModEndl;
#endif
		if (getStemmerFunc == 0 ||
			(stemmer = (*getStemmerFunc)(stemmerRscID)) == 0) {
			ModErrorMessage << "getStemmer failed: "
							<< intptr_t(getStemmerFunc) << ' '
							<< intptr_t(stemmer) << ModEndl;
			ModThrowInvertedFileError(ModInvertedErrorGetStemmerFail);
		}
	}

	; ModAssert(stemmer != 0);
	return stemmer;
}
#endif // ! UNA_V3_3 || UNA_V3_4

#endif // V1_4

#ifdef SYD_USE_UNA_V10
//
//	FUNCTION protected
//	ModInvertedDualTokenizer::setUnaParameter
//		-- UNAのパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModBoolean isNormalized_
//		正規化するかどうか
//	const ModLanguageSet& cLang_
//		言語
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ModInvertedDualTokenizer::setUnaParameter(ModBoolean isNormalized_,
										  const ModLanguageSet& cLang_)
{
	m_cUnaParam[_DoNorm] = ((isNormalized_ == ModTrue) ? _True : _False);
#ifdef SYD_USE_UNA_V12
	una->prepare(m_cUnaParam);
#else
	una->prepare(cLang_, m_cUnaParam);
#endif
}
#endif


//
//	FUNCTION private
//	ModInvertedDualTokenizer::setFeatureCandidate --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ModInvertedDualTokenizer::setFeatureCandidate(
	FeatureCandidateMap& mapFeatureCandidate_,
	int iPOS_,
	const ModUnicodeString& cstrMorpheme_,
	int iCost_,
	int iUnifiedPOS_) const
{
	// 名詞が特徴語候補
	// ただし、日本語中の英語が UNA_UNKNOWN になるので、それも考慮する。
	if (iUnifiedPOS_ & UNA_NOUN || iUnifiedPOS_ == UNA_UNKNOWN)
	{
		ModSize uiMorphemeSize = cstrMorpheme_.getLength();

		ModTermType iTermType = TERM_ALPHABET;
		if (m_pTermResource->termTypeTable != 0)
		{
			iTermType = (*m_pTermResource->termTypeTable)[iPOS_];
		}
		// [NOTE] 変換テーブルがない場合、全ての未定義語を英文字列とみなす。
		
		if (uiMorphemeSize > 1 &&
			(iUnifiedPOS_ & UNA_NOUN || iTermType == TERM_ALPHABET))
		{
			// 2文字以上、かつ、名詞または英文字、の場合
			
			FeatureCandidateMap::Iterator i =
				mapFeatureCandidate_.find(cstrMorpheme_);
			if (i == mapFeatureCandidate_.end())
			{
				// 初めてなので、1のTFとコストを登録
				
				// TF項とのバランスを考えてコストを補正
				int iCost = 0;
				if (iUnifiedPOS_ & UNA_NOUN)
				{
					iCost = ModMin(_MaxOccurrenceCost.get(), iCost_);
				}
				else
				{
					// 英文字列の場合
					; ModAssert(iUnifiedPOS_ == UNA_UNKNOWN &&
								iTermType == TERM_ALPHABET);
					// 文字列長に依存
					iCost = static_cast<int>(_AlphabetOccurrenceCostFactor.get()
											 * LOG(uiMorphemeSize));
				}
				
				// 登録
				mapFeatureCandidate_.insert(cstrMorpheme_,
											FeatureCandidateElement(1,iCost));
			}
			else
			{
				// TFをインクリメント
				++((*i).second.first);
			}
		}
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
