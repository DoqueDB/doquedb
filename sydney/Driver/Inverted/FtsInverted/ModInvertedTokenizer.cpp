// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedTokenizer.cpp -- 分割処理器の実装
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#endif

#include "ModMessage.h"
#include "ModError.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"

#include "ModInvertedException.h"
#include "ModInvertedTokenizer.h"
#include "ModInvertedNgramTokenizer.h"
#include "ModInvertedBlockedNgramTokenizer.h"
#ifdef V1_4
#include "ModInvertedDualTokenizer.h"
#endif // V1_4
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedFile.h"
#else
#include "ModInvertedFile.h"
#endif

// CONST
// ModInvertedTokenizer::normalizerResourceID -- 正規化器リソースID指定文字列
//
// NOTES
// 正規化器リソースIDを指定するキーとなる文字列。
//
/*static*/ const char* 
ModInvertedTokenizer::normalizerResourceID = "@NORMRSCID";

#ifndef SYD_USE_UNA_V10
// CONST
// ModInvertedTokenizer::stemmerResourceID -- ステマーリソースID指定文字列
//
// NOTES
// ステマーリソースIDを指定するキーとなる文字列。
//
/*static*/ const char*
ModInvertedTokenizer::stemmerResourceID = "@STEMRSCID";
#endif

// CONST
// ModInvertedTokenizer::analyzerResourceID -- 解析器リソースID指定文字列
//
// NOTES
// 解析器リソースIDを指定するキーとなる文字列。
//
/*static*/ const char* 
ModInvertedTokenizer::analyzerResourceID = "@UNARSCID";


// VARIABLE
// ModInvertedTokenizer::getNormalizerFunc -- 正規化器取得関数
//
// NOTES
// 正規化器取得関数へのポインタ。
//
/*static*/ ModInvertedTokenizer::GetNormalizer*
ModInvertedTokenizer::getNormalizerFunc = 0;

#ifndef SYD_USE_UNA_V10
// VARIABLE
// ModInvertedTokenizer::getStemmerFunc -- ステマー取得関数
//
// NOTES
// ステマー取得関数へのポインタ。
//
/*static*/ ModInvertedTokenizer::GetStemmer*
ModInvertedTokenizer::getStemmerFunc = 0;
#endif

// VARIABLE
// ModInvertedTokenizer::getAnalyzerFunc -- 解析器取得関数
//
// NOTES
// 解析器取得関数へのポインタ。
//
/*static*/ ModInvertedTokenizer::GetAnalyzer*
ModInvertedTokenizer::getAnalyzerFunc = 0;


//
// FUNCTION 
// ModInvertedFile::setGetNormalizer -- 正規化器取得関数の設定
//
// NOTES
// 正規化器取得関数を設定する。
//
// ARGUMENTS
// GetNormalizer* getNormalizerFunc_
//		正規化器取得関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
/*static*/ void
ModInvertedTokenizer::setGetNormalizer(GetNormalizer* getNormalizerFunc_)
{
	getNormalizerFunc = getNormalizerFunc_;
}

#ifndef SYD_USE_UNA_V10
//
// FUNCTION 
// ModInvertedFile::setGetStemmer -- ステマー取得関数の設定
//
// NOTES
// ステマー取得関数を取得する。
//
// ARGUMENTS
// GetStemmer* getStemmerFunc_
//		ステマー取得関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
/*static*/ void
ModInvertedTokenizer::setGetStemmer(GetStemmer* getStemmerFunc_)
{
	getStemmerFunc = getStemmerFunc_;
}
#endif

//
// FUNCTION 
// ModInvertedFile::setGetAnalyzer -- 解析器取得関数の設定
//
// NOTES
// 解析器取得関数を設定する。
//
// ARGUMENTS
// GetAnalyzer* getAnalyzerFunc_
//		正規化器取得関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
/*static*/ void
ModInvertedTokenizer::setGetAnalyzer(GetAnalyzer* getAnalyzerFunc_)
{
	getAnalyzerFunc = getAnalyzerFunc_;
}


//
// FUNCTION
// ModInvertedTokenizer::create -- 分割器の生成
//
// NOTES
// 渡されたパラメータに合致する分割器を生成する。
//
// ARGUMENTS
// const ModCharString& description_
//		トークナイザのためのパラメータ
// ModInvertedFile* file_
//		転置ファイル
// const ModBoolean normalizing
//		表記正規化指示
// const ModBoolean stemming
//		ステミング実行指示
// const ModInvertedUnaSpaceMode
//		正規化時の空白の扱い方
// const ModBoolean carriage_
//		改行を跨った正規化を行うかどうか
//	ModSize maxWordLen_
//		最大単語長
//	ModSize fearureSize_
//	   	特徴語数
//
// RETURN
// 分割器
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerType
//		トークナイザタイプが不正である
//
ModInvertedTokenizer*
ModInvertedTokenizer::create(const ModCharString& description_,
							 ModInvertedFile* file_,
							 const ModBoolean normalizing_,
							 const ModBoolean stemming_,
							 const ModInvertedUnaSpaceMode unaSpaceMode_,
							 const ModBoolean carriage_,
							 ModSize maxWordLen_,
							 ModSize featureSize_)

{
	ModInvertedTokenizer* tokenizer = 0;

	try {
		ModSize length(description_.getLength()), nameLen;

		// n-gram 分割器
		nameLen = ModCharTrait::length(
			ModInvertedNgramTokenizer::tokenizerName);
		if (description_.compare(ModInvertedNgramTokenizer::tokenizerName,
								 nameLen) == 0 &&
			(length == nameLen ||
			 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
			tokenizer = new ModInvertedNgramTokenizer(
				description_.copy(nameLen + 1), file_, normalizing_);
		}

		// ブロック化 n-gram 分割器
		nameLen = ModCharTrait::length(
			ModInvertedBlockedNgramTokenizer::tokenizerName);
		if (description_.compare(ModInvertedBlockedNgramTokenizer::tokenizerName,
								 nameLen) == 0 &&
			(length == nameLen ||
			 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
			tokenizer = new ModInvertedBlockedNgramTokenizer(
				description_.copy(nameLen + 1), file_, normalizing_);
		}

#ifdef V1_4
		// 単語文字列混在分割器	- とりあえず固定的に扱う
		nameLen = ModCharTrait::length(
			ModInvertedDualTokenizer::tokenizerName);
		if (description_.compare(ModInvertedDualTokenizer::tokenizerName,
								 nameLen) == 0 &&
			(length == nameLen ||
			 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
			tokenizer = new ModInvertedDualTokenizer(
				description_.copy(nameLen + 1), file_,
				normalizing_, stemming_, unaSpaceMode_, carriage_,
				maxWordLen_, featureSize_);
		}
#endif

		if (tokenizer == 0) {
			// TokenizerType が指定されていない場合
			ModErrorMessage << "invalid description: "
							<< description_ << ModEndl;
			ModThrowInvertedFileError(
				ModInvertedErrorInvalidTokenizerArgument);
		}

		return tokenizer;

	} catch (ModException& e) {
		ModErrorMessage << "create failed: " << e << ModEndl;
		ModRethrow(e);
#ifndef SYD_INVERTED
	} catch (...) {
/* purecov:begin deadcode */
		ModUnexpectedThrow(ModModuleInvertedFile);
/* purecov:end deadcode */
#endif
	}
	return 0;
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
// ModVector<ModLanguageSet>* langSets_
//		言語指定 (v1_6のみ)
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
ModInvertedTokenizer::tokenize(
	const ModUnicodeString& targetString,
	const TokenizeMode mode,
	ModInvertedLocationListMap& result,
	ModSize& tokenizedEnd,
	const ModVector<ModSize>* sectionEndByteOffsets,
	ModVector<ModSize>* sectionEndCharacterOffsets
#ifdef V1_6
	, ModVector<ModLanguageSet>* langSets_
#endif // V1_6
	, ModInvertedFeatureList* feature_,
	const ModTermResource* pTermResource_)
{
	ModUnicodeString tokenString;
	Occurrence occurrence;

	try {
		set(targetString, mode,
			sectionEndByteOffsets, sectionEndCharacterOffsets);

		// Vector の領域をここで確保する
		ModInvertedSmartLocationList tmp;
		while (yield(tokenString, occurrence) == ModTrue) {
#ifdef DEBUG
			if (ModInvertedFile::debugLevel > 1) {
				ModDebugMessage << "token='" << tokenString
								<< "' offset=" << occurrence.getOffset()
								<< ModEndl;
			}
#endif
			if (tokenString.getLength() > ModInvertedIndexKeyLenMax) {
				ModErrorMessage << "token=" << tokenString
								<< " too long" << ModEndl;
				ModThrowInvertedFileError(ModInvertedErrorTooLongIndexKey);
			}
#ifdef INV_TOK_USETOKEN
			ModInvertedToken token(tokenString);
			ModInvertedLocationListMap::Iterator pair(result.find(token));
#else
			ModInvertedLocationListMap::Iterator pair(result.find(tokenString));
#endif
			if (pair == result.end()) {
				// 見つからない
				// Tokenizer の外では location は 1 origin とするので + 1 する
				tmp.setCoder(file->getLocationCoder(tokenString));
				tmp.setFirstValue(occurrence.getOffset() + 1);
#ifdef INV_TOK_USETOKEN
				token.setLength(getTokenLength(tokenString));
				result.insert(token, tmp);
#else
				result.insert(tokenString, tmp);
#endif
			} else {
				(*pair).second.pushBack(occurrence.getOffset() + 1);
			}
		}

		if (sectionEndCharacterOffsets != 0) {
			// location は 1 origin なのでインクリメントする
			for (ModVector<ModSize>::Iterator i(sectionEndCharacterOffsets->begin());
				 i != sectionEndCharacterOffsets->end(); ++i) {
				++(*i);
			}
		}

		tokenizedEnd = getTokenizedEnd();
		
	} catch (ModException& exception) {
		ModErrorMessage << "tokenize failed: target=" << targetString
						<< " mode=" << mode 
						<< " token=" << tokenString
						<< " offset=" << occurrence.getOffset()
						<< ": " << exception << ModEndl;
		ModRethrow(exception);
#ifndef SYD_INVERTED
	} catch (...) {
/* purecov:begin deadcode */
		ModErrorMessage << "target=" << targetString << ModEndl;
		ModUnexpectedThrow(ModModuleInvertedFile);
/* purecov:end */
#endif
	}
}

//
// FUNCTION
// ModInvertedTokenizer::getTokenLength -- トークン長の取得
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
ModInvertedTokenizer::getTokenLength(const ModUnicodeString& string) const
{
	return string.getLength();
}

//
// FUNCTION
// ModInvertedTokenizer::isPrenormalizable -- プレノーマライズ適用可能かの判定
//
// NOTES
// 渡されたパラメータに合致する分割器がプレノーマライズ適用可能か判定する。
//
// ARGUMENTS
// const ModCharString& description_
//		トークナイザのためのパラメータ
//
// RETURN
// 適用可能であれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorInvalidTokenizerType
//		トークナイザタイプが不正である
//
/*static*/ ModBoolean
ModInvertedTokenizer::isPrenormalizable(const ModCharString& description_)
{
	ModSize length(description_.getLength()), nameLen;

	// n-gram 分割器
	nameLen = ModCharTrait::length(
		ModInvertedNgramTokenizer::tokenizerName);
	if (description_.compare(ModInvertedNgramTokenizer::tokenizerName,
							 nameLen) == 0 &&
		(length == nameLen ||
		 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
		return ModTrue;
	}

	// ブロック化 n-gram 分割器
	nameLen = ModCharTrait::length(
		ModInvertedBlockedNgramTokenizer::tokenizerName);
	if (description_.compare(ModInvertedBlockedNgramTokenizer::tokenizerName,
							 nameLen) == 0 &&
		(length == nameLen ||
		 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
		return ModTrue;
	}

#ifdef V1_4
	// 単語文字列混在分割器	- とりあえず固定的に扱う
	nameLen = ModCharTrait::length(
		ModInvertedDualTokenizer::tokenizerName);
	if (description_.compare(ModInvertedDualTokenizer::tokenizerName,
							 nameLen) == 0 &&
		(length == nameLen ||
		 description_[nameLen] == ':' || description_[nameLen] == ' ')) {
		return ModFalse;
	}
#endif

	// TokenizerType が指定されていない場合
	ModErrorMessage << "invalied description: "
					<< description_ << ModEndl;
	ModThrowInvertedFileError(
		ModInvertedErrorInvalidTokenizerArgument);

/* purecov:begin deadcode */
	return ModFalse;
/* purecov:end deadcode */
}

//
// FUNCTION
// ModInvertedTokenizer::getResourceID -- リソースIDの取得
//
// NOTES
// 渡された文字列からリソースIDを取得する。
// 指定されていない場合は 0 を返す。
//
// ARGUMENTS
// const ModCharString& target_
//		処理対象文字列
// const ModCharString& name_
//		リソースID指定文字列
//
// RETURN
// リソースID
//
// EXCEPTIONS
// なし
//
/*static*/ ModUInt32
ModInvertedTokenizer::getResourceID(const ModCharString& target_,
									const ModCharString& name_)
{
	char* res = target_.search(name_);
	if (res != 0 && res[name_.getLength()] == ':') {
		// 見つかった時は "name:X" の X の部分を数値に変換して返す
		return ModCharTrait::toInt(res + name_.getLength() + 1);
	}
	// 上記以外の場合は 0 を返す
	return 0;
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
