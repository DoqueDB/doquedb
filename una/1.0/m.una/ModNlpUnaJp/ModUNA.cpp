// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModUNA.cpp -- UNA ラッパークラス（表記正規化機能付き）の実装
// 
// Copyright (c) 2000-2010, 2023 Ricoh Company, Ltd.
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

#include "ModMessage.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "ModParameter.h"
#include "ModException.h"

#include "LibUna/ModWordStemmer.h"
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModUNA.h"
#include "ModNlpUnaJp/unakapi.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"

#include "UnaReinterpretCast.h"

#if 0
#include <stdio.h>
#endif
_UNA_USING
_UNA_UNAJP_USING

//
// FUNCTION private
// ctrl_c_stop -- 中断処理関数
//
// NOTES
// unaKApi_moAna に渡す中断処理関数。今回は空関数。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//
#if !defined(CC_SUN5_3)
extern "C" {
static long ctrl_c_stop()
{
  return 0;
}
};
#else
static long ctrl_c_stop()
{
  return 0;
}
#endif

//
// FUNCTION
// ModUnaAnalyzer::ModUnaAnalyzer -- コンストラクタ
//
// NOTES
// 解析器のコンストラクタ
//
// ARGUMENTS
// ModUnaResource* const resource_
//    リソース
// ModNormalizer* const normalizer_
//    異表記正規化器
// ModWordStemmer* const stemmer_
//    英単語正規化器
// ModNlpExpStr* const expstr_
//	 Synonym expander
// DicSet *dicSet_
//	 DicSet
// ModBoolean useJpn_
//	 Is Japanese processing possible?
// ModSize maxWordLen_
//    最大単語長の指定(0で指定なし)
//
// RETURN
// なし
//
// EXCEPTIONS
//
ModUnaAnalyzer::ModUnaAnalyzer( ModUnaResource* const resource_,
	ModNormalizer* const normalizer_,
	ModWordStemmer* const stemmer_,
	ModNlpExpStr* expstr_,
	DicSet *dicSet_,
	ModBoolean useJpn_, 
	ModSize maxWordLen_):
	normalizer(normalizer_),
	tokenType(0),
	stemmer(stemmer_),
	expstr(expstr_),
	ModUnaMiddleAnalyzer(resource_, dicSet_,((normalizer_ != 0) ? ModTrue : ModFalse), useJpn_, maxWordLen_),
	m_dicSet(dicSet_)
{
  if ( resource_->optionStatus){
    execQuick = ModTrue;
  }
}

void ModUnaAnalyzer::setExecQuick(ModBoolean x)
{
  execQuick = x;
}

// NOTES ステマーの変更
void 
ModUnaAnalyzer::setStemmer(ModWordStemmer* const stemmer_) 
{
	stemmer = stemmer_; 
}

// NOTES ノーマライザの変更
void 
ModUnaAnalyzer::setNormalizer(ModNormalizer* const normalizer_) 
{
	normalizer = normalizer_; 
}

//
// FUNCTION
// ModUnaAnalyzer::getNormalized -- 表記正規化した解析結果の取得
//
// NOTES
// 解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeString& original_
//    原表記
// ModBoolean& getOriginal_
//    原表記取得の指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getNormalized( ModUnicodeString& result_,
	ModUnicodeString& original_, ModBoolean getOriginal_)
{
	if ( doSub(mode)){
		ModBoolean execStemming=ModFalse;
		if ( doStem(mode)){
			execStemming = ModTrue;
		}
		if ( resource->optionStatus && execQuick){ // 高速化データあり
			ModUnicodeChar *original;
			ModSize len;
			ModBoolean res;
			int pos;
			res = getCheckNormalizedSpeed(result_,original,len,getOriginal_,pos,
				execStemming);
			if ( getOriginal_ ==ModTrue){
				original_.clear();
				if ( len>0)
					original_.append(original,len);
			}
			return res;
		}
		else{ // 高速化データなし
			int pos=0;
			return getCheckNormalized(result_,original_,getOriginal_, execStemming,pos);
		}
	}
	else{
		int pos=0;
		return getSimpleNormalized(result_,original_,getOriginal_,pos);
	}
}

//
// FUNCTION
// ModUnaAnalyzer::getNormalized -- 表記正規化した解析結果の取得
//
// NOTES
// 解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModNormalizer* const normalizer_
//    異表記正規化器
// ModWordStemmer* const stemmer_
//    stemmer
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModUnaAnalyzer::getNormalized( ModUnicodeString& result_,
	ModNormalizer* const normalizer_, ModWordStemmer* const stemmer_)
{
	normalizer = normalizer_;
	stemmer = stemmer_;
	return getNormalized( result_, work3, ModFalse);
}

// FUNCTION
// ModUnaAnalyzer::getSimpleNormalized -- 単純な解析結果の取得
//
// NOTES
// getNormalized の下請関数。
// 英語トークンが見つかった場合、下位構造の有無にかかわらず、マージした
// 結果をトークンとする。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeString& original_
//    原表記
// ModBoolean& getOriginal_
//    原表記取得の指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getSimpleNormalized( ModUnicodeString& result_,
	ModUnicodeString& original_, ModBoolean getOriginal_, int& pos_)

{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	result_.clear();
	if ( getOriginal_){
		original_.clear();
		pos_=0;
	}

retry:
	if (morphCurrent == morphNum) {
		morphCurrent = 0;
		morphNum = 0;
		analyze(ModTrue);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
						 &hyouki, &hyoukiLength) < 0) {
		ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
	}

	// 異表記正規化する
	ModUnicodeString original;
	original.append(hyouki, hyoukiLength);

	if ( doStem(mode) && stemmer != 0) { // ステミングする
		ModUnicodeString normalized;
		normalizer->normalizeBuf(original, normalized, 0, 0, ModNormalized);

		if (normalized.getLength() == 0) {
			// 正規化で長さが 0 になったものは無視し、次の形態素に移る
			++morphCurrent;
			goto retry;
		}

		// ASCII だけで構成されているかは必ず検査される
		try {
			stemmer->stem(normalized, result_);
		} catch (ModException& e) {
			if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
				ModErrorMessage << e << ModEndl;
#endif
				ModRethrow(e);
			}
			ModErrorHandle::reset();
		}
#ifdef UNA_DEBUG
		ModMessage << original << ' ' << normalized << ' '
				   << result_ << ModEndl;
#endif
	} else {
		// ステミングしない
		normalizer->normalizeBuf(original, result_, 0, 0, ModNormalized);

		if (result_.getLength() == 0) {
			// 正規化で長さが 0 になったものは無視し、次の形態素に移る
			result_.clear();
			++morphCurrent;
			goto retry;
		}

#ifdef UNA_DEBUG
		ModMessage << original << ' ' << result_ << ModEndl;
#endif
	}
	if ( getOriginal_){
		ModUnicodeChar* ohyouki;
		ModSize ohyoukiLength;
		if (unaKApi_getOriginalHyoki(handle, targetStart,
			(morphBuffer + morphCurrent), &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
		 		ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(ohyoukiLength != 0);
		original_.append(ohyouki, ohyoukiLength);
		pos_ = (morphBuffer + morphCurrent)->hinshi;
	}

	++morphCurrent;

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getCheckNormalizedSpeed -- 下位構造を考慮した解析結果の取得
//
// NOTES
// getNormalized の下請関数。高速動作版
// 英語トークンが見つかった場合、ステマー辞書に登録されていない単語については
// 下位構造の有無を調べ、下位構造があれば個別のトークンを返す。
// 原表記取得の指示がある場合、posもつけて返す
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeChar* &original_
//    原表記
// ModSize &len_
//    長さ
// ModSize &getOriginal_
//    原表記取得の指示
// int &pos_
//    品詞
// ModBoolean execStemming_
//    ステミングを実施するか否か
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getCheckNormalizedSpeed( ModUnicodeString& result_,
	ModUnicodeChar* &original_, ModSize &len_, ModBoolean getOriginal_,
	int& pos_, ModBoolean execStemming_)
{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}
	if ( execStemming_ == ModTrue && stemmer == 0) {
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

 	ModUnicodeString& tmp = work1;
 	ModUnicodeString& tmp2 = work2;
 	tmp.clear();
	ModInt32 res;
	unaAppInfoT* appInfo;

	len_ = 0; // 初期化

retry:
	result_.clear();

	if (subMorphCurrent < subMorphNum) { // 下位構造がある場合
		res = unaKApi_getAppInfo(handle,
				subMorphBuffer + subMorphCurrent, &appInfo);

		// アプリ情報の長さは１以上
		; ModAssert(appInfo->len != 0);

		if (tokenType == UNA_ENG_TOKEN) { // 下位構造が英語トークン
			// 英語トークンは同じ表記であっても改行にまたがっているか
			// 否かによってステミング結果が変わるので、正規化までをキャッシュ
			tmp.append(
				una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				(appInfo->len)/sizeof(ModUnicodeChar));
			// cache を使って高速化
			long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
			if ( tmp == cache1[cnum]){ // キャッシュにヒット
				tmp2 = cache2[cnum];
			}
			else{ // キャッシュにヒットしなかった
				tmp2.clear();
				normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
				cache1[cnum] = tmp;
				cache2[cnum] = tmp2;
			}
			if (tmp2.getLength() == 0) {
				tmp.clear();
				++subMorphCurrent;
				goto retry;
			}

			try {
				if ( execStemming_ ){
					stemmer->stem(tmp2, result_);
				}
				else{
					result_ = tmp2;
				}
			} catch (ModException& e) {
				if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
					ModErrorMessage << e << ModEndl;
#endif
					ModRethrow(e);
				}
				ModErrorHandle::reset();
			}
		} else { // 下位構造が英語トークン以外の辞書語の場合
			; ModAssert(tokenType == UNA_KNOWN_WORD);

			result_.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));
			; ModAssert(result_.getLength() > 0);

			// ステミングは不要
		}
		if ( getOriginal_){
			if (unaKApi_getOriginalHyoki(handle,targetStart,
				(subMorphBuffer + subMorphCurrent), &original_, &len_) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(len_ != 0);
			pos_ = (subMorphBuffer+subMorphCurrent)->hinshi;
		}
		++subMorphCurrent;
		return ModTrue;
	} // end of 下位構造がある場合

	// 下位構造が無い場合
	subMorphNum = subMorphCurrent = 0;

	if (morphCurrent == morphNum) { // 残り形態素が無い
		// 解析実行
		morphCurrent = 0;
		morphNum = 0;
		analyze(ModTrue);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	// 処理すべき形態素が残っている場合
	tokenType
		= unaKApi_getAppInfo(handle, morphBuffer + morphCurrent, &appInfo);
	if (tokenType == UNA_ENG_TOKEN) { // 残り形態素が英語トークン 
		; ModAssert(appInfo->len != 0);
		tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
		// cache を使って高速化
		long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
		if ( tmp == cache1[cnum]){ // キャッシュにヒットした
			tmp2 = cache2[cnum];
		}
		else{ // キャッシュにヒットしなかった
			tmp2.clear();
			normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
			cache1[cnum] = tmp;
			cache2[cnum] = tmp2;
		}
		if (tmp2.getLength() == 0) {
			tmp.clear();
			++morphCurrent;
			goto retry;
		}
		if (!execStemming_ || stemmer->look(tmp2) == ModFalse) { // stem辞書にない単語 
			; ModAssert(subMorphCurrent == 0);
			res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
					  subMorphBuffer, &subMorphNum, unaLocalKeitaisoSize);
			; ModAssert (res == UNA_ENG_TOKEN);

#ifdef UNA_DEBUG
			ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
			if (subMorphNum != 0) { // 下位構造があった 
				// とんだ先で下位構造を得る処理を実施する
				tmp.clear();
				tmp2.clear();
				++morphCurrent;
				goto retry;
			}
		}
		// ステミング結果をつくる
		try {
			if ( execStemming_ ){
				stemmer->stem(tmp2, result_);
			}
			else{
				result_ = tmp2;
			}
		} catch (ModException& e) {
			if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
				ModErrorMessage << e << ModEndl;
#endif
				ModRethrow(e);
			}
			ModErrorHandle::reset();
		}

	} else {  // 残り形態素が英語トークンでない
		if (tokenType == UNA_KNOWN_WORD) { // 辞書語 
			// 下位構造があれば取り出す
			; ModAssert(subMorphCurrent == 0);
			res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
									  subMorphBuffer, &subMorphNum,
									  unaLocalKeitaisoSize);
			; ModAssert (res == UNA_KNOWN_WORD);
#ifdef UNA_DEBUG
			ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
			if (subMorphNum != 0) { // T下位構造があった 
				// とんだ先で下位構造を得る処理を実施する
				tmp.clear();
				tmp2.clear();
				++morphCurrent;
				goto retry;
			}

		} else if (tokenType != UNA_UNKNOWN_WORD) { // エラーチェック
			// 英語トークンでなく辞書語/非辞書語でもないことはあり得ない
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getAppInfo error: "
				<< unaKApi_getErrMsg(tokenType) << ": " << target << ModEndl;
			; ModAssert(tokenType < 0 /*&& tokenType == UNA_NO_APP_DIC*/);
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		// ここに来るのは英語トークンでなく下位構造がない場合

		// 辞書語のアプリ情報の長さは０もあり得る
		result_.clear();
		if ( appInfo->len > 0 ){
			result_.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
		}
		// 未登録語の場合は従来の処理
		else if (tokenType == UNA_UNKNOWN_WORD) {
			ModUnicodeChar* hyouki;
			ModSize hyoukiLength;
			if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
				&hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
				ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(hyoukiLength != 0);

			tmp.append(hyouki, hyoukiLength);
			// cache を使って高速化
			long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
			if ( tmp == cache1[cnum]){
				result_ = cache2[cnum];
			}
			else{
				normalizer->normalizeBuf(tmp, result_, 0, 0, ModNormalized);
				cache1[cnum] = tmp;
				cache2[cnum] = result_;
			}
		}
		if (result_.getLength() == 0) {
			tmp.clear();
			++morphCurrent;
			goto retry;
		}

		// ステミングは不要
	}
	if ( getOriginal_){
		if (unaKApi_getOriginalHyoki(handle,targetStart,
			(morphBuffer + morphCurrent), &original_, &len_) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
				ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(len_ != 0);
		pos_ = (morphBuffer+morphCurrent)->hinshi;
	}

	++morphCurrent;

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getCheckNormalized -- 下位構造を考慮した解析結果の取得
//
// NOTES
// getNormalized の下請関数。
// 英語トークンが見つかった場合、ステマー辞書に登録されていない単語については
// 下位構造の有無を調べ、下位構造があれば個別のトークンを返す。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeString& original_
//    原表記
// ModBoolean& getOriginal_
//    原表記取得の指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getCheckNormalized( ModUnicodeString& result_,
	ModUnicodeString& original_, ModBoolean getOriginal_, ModBoolean execStemming_, int& pos_)
{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	if (execStemming_ && stemmer == 0) {
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

 	ModUnicodeString& tmp = work1;
 	ModUnicodeString& normalized = work2;
 	tmp.clear();
 	normalized.clear();
	ModInt32 res;
	unaAppInfoT* appInfo;

retry:
	result_.clear();
	if ( getOriginal_){
		pos_ = 0;
		original_.clear();
	}

	if (subMorphCurrent < subMorphNum) { // 下位構造がある場合
		if (tokenType == UNA_ENG_TOKEN) { // 下位構造が英語トークン
			res = unaKApi_getAppInfo(handle, subMorphBuffer + subMorphCurrent,
									 &appInfo);
			if (res != UNA_ENG_TOKEN) {
#ifdef DEBUG
				ModErrorMessage << unaKApi_getErrMsg(res) << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}

			// アプリ情報の長さは１以上
			; ModAssert(appInfo->len != 0);

			tmp.append(
				una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				(appInfo->len)/sizeof(ModUnicodeChar));
			normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
			if (normalized.getLength() == 0) {
				tmp.clear();
				++subMorphCurrent;
				goto retry;
			}

			try {
				if ( execStemming_ ){
					stemmer->stem(normalized, result_);
				}
				else{
					result_ = normalized;
				}
			} catch (ModException& e) {
				if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
					ModErrorMessage << e << ModEndl;
#endif
					ModRethrow(e);
				}
				ModErrorHandle::reset();
			}

		} else { // 下位構造が英語トークン以外の辞書語の場合
			; ModAssert(tokenType == UNA_KNOWN_WORD);
			ModUnicodeChar* hyouki;
			ModSize hyoukiLength;
			if (unaKApi_getHyoki(handle,subMorphBuffer + subMorphCurrent,
								 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(hyoukiLength != 0);

			tmp.append(hyouki, hyoukiLength);
			normalizer->normalizeBuf(tmp, result_, 0, 0, ModNormalized);
			if (result_.getLength() == 0) {
				// 2002/02現在 辞書にこのデータは存在しない
				tmp.clear();
				++subMorphCurrent;
				goto retry;
			}

			// ステミングは不要
		}

		if ( getOriginal_){
			ModUnicodeChar* ohyouki;
			ModSize ohyoukiLength;
			if (unaKApi_getOriginalHyoki(handle,targetStart,
				(subMorphBuffer + subMorphCurrent),
							 &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(ohyoukiLength != 0);

			original_.append(ohyouki, ohyoukiLength);
			pos_ = (subMorphBuffer + subMorphCurrent)->hinshi;
		}

		++subMorphCurrent;
		return ModTrue;
	} // end of 下位構造がある場合

	// 下位構造が無い場合
	subMorphNum = subMorphCurrent = 0;

	if (morphCurrent == morphNum) { // 残り形態素が無い
		// 解析実行
		morphCurrent = 0;
		morphNum = 0;
		analyze(ModTrue);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	// 処理すべき形態素が残っている場合
	tokenType
		= unaKApi_getAppInfo(handle, morphBuffer + morphCurrent, &appInfo);
	if (tokenType == UNA_ENG_TOKEN) { // 残り形態素が英語トークン 
		; ModAssert(appInfo->len != 0);
		tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
		normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
		if (normalized.getLength() == 0) {
			tmp.clear();
			++morphCurrent;
			goto retry;
		}

#ifdef UNA_DEBUG
		ModDebugMessage << "ENG token: " << tmp << ' '
						<< normalized << ModEndl;
#endif
		if ( execStemming_ && stemmer->look(normalized) == ModTrue) { // stem辞書にある単語 
			// ステミング結果を返す
			stemmer->stem(normalized, result_);
			if ( getOriginal_){
				ModUnicodeChar* ohyouki;
				ModSize ohyoukiLength;
				if (unaKApi_getOriginalHyoki(handle,targetStart,
					(morphBuffer+morphCurrent),&ohyouki, &ohyoukiLength) < 0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				; ModAssert(ohyoukiLength != 0);

				original_.append(ohyouki, ohyoukiLength);
				pos_ = (morphBuffer+morphCurrent)->hinshi;
			}
			++morphCurrent;
			return ModTrue;
		}

		; ModAssert(subMorphCurrent == 0);

		res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
				  subMorphBuffer, &subMorphNum, unaLocalKeitaisoSize);
		if (res != UNA_ENG_TOKEN) { // エラー
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getSubMorph error: "
				<< unaKApi_getErrMsg(res) << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
#ifdef UNA_DEBUG
		ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
		if (subMorphNum != 0) { // 下位構造があった 
			// とんだ先で下位構造を得る処理を実施する
			tmp.clear();
			normalized.clear();
			++morphCurrent;
			goto retry;
		}

		// 下位構造がない -- ステミングして終り
		try {
			if ( execStemming_ ){
				stemmer->stem(normalized, result_);
			}
			else{
				result_ = normalized;
			}
		} catch (ModException& e) {
			if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
				ModErrorMessage << e << ModEndl;
#endif
				ModRethrow(e);
			}
			ModErrorHandle::reset();
		}

	} else {  // 残り形態素が英語トークンでない
		if (tokenType == UNA_KNOWN_WORD) { // 辞書語 
			// 下位構造があれば取り出す
			; ModAssert(subMorphCurrent == 0);

			res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
									  subMorphBuffer, &subMorphNum,
									  unaLocalKeitaisoSize);
			if (res != UNA_KNOWN_WORD) { // エラー
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getSubMorph error: "
					<< unaKApi_getErrMsg(res) << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
#ifdef UNA_DEBUG
			ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
			if (subMorphNum != 0) { // 下位構造があった 
				// とんだ先で下位構造を得る処理を実施する
				tmp.clear();
				normalized.clear();
				++morphCurrent;
				goto retry;
			}

		} else if (tokenType != UNA_UNKNOWN_WORD) { // エラーチェック
			// 英語トークンでなく辞書語/非辞書語でもないことはあり得ない
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getAppInfo error: "
				<< unaKApi_getErrMsg(tokenType) << ": " << target << ModEndl;

			; ModAssert(tokenType < 0 /*&& tokenType == UNA_NO_APP_DIC*/);
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		// ここに来るのは英語トークンでなく下位構造がない場合
		// - 表記を取り出して正規化する
		ModUnicodeChar* hyouki;
		ModSize hyoukiLength;
		if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
							 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(hyoukiLength != 0);

		tmp.append(hyouki, hyoukiLength);

		normalizer->normalizeBuf(tmp, result_, 0, 0, ModNormalized);
		if (result_.getLength() == 0) {
			tmp.clear();
			++morphCurrent;
			goto retry;
		}

		// ステミングは不要
	}

	if ( getOriginal_){
		ModUnicodeChar* ohyouki;
		ModSize ohyoukiLength;
		if (unaKApi_getOriginalHyoki(handle,targetStart,
			(morphBuffer + morphCurrent),
					 &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(ohyoukiLength != 0);
		original_.append(ohyouki, ohyoukiLength);
		pos_ = (morphBuffer + morphCurrent)->hinshi;
	}

	++morphCurrent;

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getCheck -- 下位構造を考慮した解析結果の取得(非正規化)
//
// NOTES
// get の下請関数。正規化は行なわない。
// 英語トークンは分解せず、辞書語のみ分解する。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeString& original_
//    原表記
// ModBoolean& getOriginal_
//    原表記取得の指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getCheck( ModUnicodeString& result_,
	ModUnicodeString& original_, ModBoolean getOriginal_)
{
	ModUnicodeString tmp;
	ModInt32 res;
	unaAppInfoT* appInfo;

retry:
	result_.clear();
	if ( getOriginal_){
		original_.clear();
	}

	if (subMorphCurrent < subMorphNum) {
		// 下位構造がある場合

		// ここに来るのは辞書語のみ
		; ModAssert(tokenType == UNA_KNOWN_WORD);

		ModUnicodeChar* hyouki;
		ModSize hyoukiLength;
		if (unaKApi_getHyoki(handle,subMorphBuffer + subMorphCurrent,
							 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(hyoukiLength != 0);

		result_.append(hyouki, hyoukiLength);

		if ( getOriginal_){
			ModUnicodeChar* ohyouki;
			ModSize ohyoukiLength;
			if (unaKApi_getOriginalHyoki(handle,targetStart,
				(subMorphBuffer + subMorphCurrent),
							 &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(ohyoukiLength != 0);

			original_.append(ohyouki, ohyoukiLength);
		}
		++subMorphCurrent;
		return ModTrue;
	}
	subMorphNum = subMorphCurrent = 0;

	if (morphCurrent == morphNum) {
		morphCurrent = 0;
		morphNum = 0;
		analyze(ModFalse);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	tokenType
		= unaKApi_getAppInfo(handle, morphBuffer + morphCurrent, &appInfo);
	if (tokenType == UNA_KNOWN_WORD) {
		// 辞書語 -- 下位構造があれば取り出す

		; ModAssert(subMorphCurrent == 0);

		res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
								  subMorphBuffer, &subMorphNum,
								  unaLocalKeitaisoSize);
		if (res != UNA_KNOWN_WORD) {
			// エラー
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getSubMorph error: "
				<< unaKApi_getErrMsg(res) << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
#ifdef UNA_DEBUG
		ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
		if (subMorphNum != 0) {
			// 下位構造があった -- とんだ先で下位構造を得る処理を実施する
			++morphCurrent;
			goto retry;
		}

	}

	// ここに来るのは下位構造がない場合 - 表記を取り出す

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
						 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
		ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorUndefined, ModErrorLevelError);
	}
	; ModAssert(hyoukiLength != 0);

	result_.append(hyouki, hyoukiLength);
	if ( getOriginal_){
		ModUnicodeChar* ohyouki;
		ModSize ohyoukiLength;
		if (unaKApi_getOriginalHyoki(handle,targetStart,
			(morphBuffer + morphCurrent),
						 &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		; ModAssert(ohyoukiLength != 0);
		original_.append(ohyouki, ohyoukiLength);
	}

	++morphCurrent;
	return ModTrue;
}


//
// FUNCTION
// ModUnaAnalyzer::getBlockNormalized -- 異表記正規化した解析結果の取得
//
// NOTES
// かかり受け解析したブロックごとの解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果のアプリケーション情報
// const ModUnicodeChar sep1_
//    セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//    セパレータ２（レコード区切り）
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlockNormalized( ModUnicodeString& result_,
	const ModUnicodeChar sep1_, const ModUnicodeChar sep2_)
{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	if ( doStem(mode) && stemmer == 0){
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	if (targetLength == 0) { // 解析対象が残っていない
		return ModFalse;
	}

	if ( doSub(mode)) { // 下位構造にばらす
		return getBlockCheck(result_, sep1_, sep2_);
	}
	// 下位構造にばらす
	return getBlockSimple(result_, sep1_, sep2_);
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockSimple -- 単純な解析結果の取得
//
// NOTES
// getBlockNormalized の下請関数。
// 英語トークンが見つかった場合、下位構造の有無にかかわらず、マージした
// 結果をトークンとする。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果のアプリケーション情報
// const ModUnicodeChar sep1_
//    セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//    セパレータ２（レコード区切り）
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlockSimple( ModUnicodeString& result_,
	const ModUnicodeChar sep1_, const ModUnicodeChar sep2_)
{
	ModUnicodeOstrStream stream;
	stream.setEncodingType(ModKanjiCode::unknown);	// 高速化のため

	analyze(ModTrue, ModTrue);

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	ModInt32 b(0), btmpNum(0);

	bunsetsuCurrent = 0;

	for (morphCurrent = 0; morphCurrent < morphNum; ++morphCurrent, ++b) {
		// 形態素数だけループする

		if (b >= btmpNum) {
			btmpNum = bunsetsuBuffer[bunsetsuCurrent++].len;
			b = 0;
		}

		// 形態素の表記
		if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
							 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		//
		ModUnicodeString original, normalized;
		original.append(hyouki, hyoukiLength);

		normalizer->normalizeBuf(original, normalized, 0, 0, ModNormalized);
		if (normalized.getLength() == 0) {
			// 正規化で長さが 0 になったものは無視し、次の形態素に移る
			continue;
		}

		if (doStem(mode)) {
			// ステミングする
			ModUnicodeString tmp(normalized);
			normalized.clear();

			// ASCII だけで構成されているかは必ず検査される
			try {
				stemmer->stem(tmp, normalized);
			} catch (ModException& e) {
				if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
					ModErrorMessage << e << ModEndl;
#endif
					ModRethrow(e);
				}
				ModErrorHandle::reset();
			}
#ifdef UNA_DEBUG
			ModMessage << original << ' ' << normalized << ' '
					   << result_ << ModEndl;
#endif
		}

		// 形態素番号（何番目の形態素か; 1 origin）
		//		正規化表記を得る段階でスキップすることがあるので、ここでアペンド
		stream << (morphCurrent + 1)
			   << sep1_;

		// 表記
		stream << normalized
			   << sep1_;

		// 形態素の品詞
		stream << unaKApi_getHinName(handle,(morphBuffer+morphCurrent))
			   << sep1_;

		// 文節番号（何番目の文節か; 1 origin）
		stream << ((b) ? 0 : bunsetsuCurrent)
			   << sep1_;

		// 係り先の文節番号（何番目の文節か; 1 origin）
		stream << ((b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].target + 1)
			   << sep1_;

		// 係り受け関係の番号
		stream << ((b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].kuRel + 1)
			   << sep2_;
	}
	
	result_ = stream.getString();

	// 形態素の辞書ベース名をdicNameVectorに保存
	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);
	for (int n = 0; n < morphNum; n++) {
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + n);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockCheck -- 下位構造を考慮した解析結果の取得
//
// NOTES
// getBlockNormalized の下請関数。
// 英語トークンが見つかった場合、ステマー辞書に登録されていない単語については
// 下位構造の有無を調べ、下位構造があれば個別のトークンを返す。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果のアプリケーション情報
// const ModUnicodeChar sep1_
//    セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//    セパレータ２（レコード区切り）
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlockCheck( ModUnicodeString& result_,
	const ModUnicodeChar sep1_, const ModUnicodeChar sep2_)
{
	ModUnicodeOstrStream stream;
	stream.setEncodingType(ModKanjiCode::unknown);	// 高速化のため

	analyze(ModTrue, ModTrue);
	
	ModUnicodeString tmp, tmp2, normalized, hinshi;
	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	ModInt32 b(0), btmpNum(0);
	unaAppInfoT* appInfo;

	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);

	bunsetsuCurrent = 0;
	subMorphNum = subMorphCurrent = 0;

	for (morphCurrent = 0; morphCurrent < morphNum || subMorphNum>0; ++morphCurrent, ++b) {
		// 形態素数だけループする

		if (subMorphNum != 0) {
			// 前回の処理で下位構造があった
			; ModAssert(morphCurrent != 0 && b != 0);
			--morphCurrent;
			--b;
		submorp:
			if (tokenType == UNA_ENG_TOKEN) {
				// 英語トークンの場合
				ModInt32 res;
				res = unaKApi_getAppInfo(handle,
										 subMorphBuffer + subMorphCurrent,
										 &appInfo);
				if (res != UNA_ENG_TOKEN) {
#ifdef DEBUG
					ModErrorMessage << unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				// アプリ情報の長さは１以上
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(
					una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));

				tmp2.clear();
				normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
				if (tmp2.getLength() == 0) {
					if (++subMorphCurrent == subMorphNum) {
						subMorphCurrent = subMorphNum = 0;
					}
					continue;
				}

				if ( doStem(mode)){
					try {
						stemmer->stem(tmp2, normalized);
					} catch (ModException& e) {
						if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
							ModErrorMessage << e << ModEndl;
#endif
							ModRethrow(e);
						}
						ModErrorHandle::reset();
					}
				}
				else{
					normalized = tmp2;
				}

			} else {
				// 英語トークン以外の辞書語の場合
				; ModAssert(tokenType == UNA_KNOWN_WORD);

				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if (unaKApi_getHyoki(handle,subMorphBuffer + subMorphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				; ModAssert(hyoukiLength != 0);

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);
				normalized.clear();
				normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
				if (normalized.getLength() == 0) {
					// 2002/02現在 辞書にこのデータは存在しない
					if (++subMorphCurrent == subMorphNum) {
						subMorphCurrent = subMorphNum = 0;
					}
					continue;
				}

				// ステミングは不要
			}

			stream << (morphCurrent + 1) << "." << (subMorphCurrent + 1) << sep1_;
			hinshi = unaKApi_getHinName(handle,(subMorphBuffer+subMorphCurrent));

			if (++subMorphCurrent == subMorphNum) {
				// すべての下位構造を処理し終えたらクリアする
				subMorphCurrent = subMorphNum = 0;
			}

		} else {
			// 通常の処理
			; ModAssert( morphCurrent < morphNum);

			if (b >= btmpNum) {
				btmpNum = bunsetsuBuffer[bunsetsuCurrent++].len;
				b = 0;
			}

			tokenType = unaKApi_getAppInfo(handle, morphBuffer + morphCurrent,
										   &appInfo);
			if (tokenType == UNA_ENG_TOKEN) {
				// 英語トークン -- 下位構造をチェックする
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						   (appInfo->len)/sizeof(ModUnicodeChar));

				if ( !doStem(mode)){
					normalized.clear();
					normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
					if (normalized.getLength() == 0) {
						continue;
					}
				}
				else{
					tmp2.clear();
					normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
					if (tmp2.getLength() == 0) {
						continue;
					}
					if (stemmer->look(tmp2) == ModTrue) {
						// 辞書にある単語 -- ステミング結果を返す
						stemmer->stem(tmp2, normalized);
					} else {
						// 辞書にない単語 -- 下位構造を得る
						ModInt32 res;
						res = unaKApi_getSubMorph(handle, morphBuffer
								+ morphCurrent, subMorphBuffer, &subMorphNum,
								  unaLocalKeitaisoSize);
						if (res != UNA_ENG_TOKEN) {
							// エラー
#ifdef DEBUG
							ModErrorMessage << "unaKApi_getSubMorph error: "
								<< unaKApi_getErrMsg(res) << ModEndl;
#endif
							ModThrow(ModModuleStandard,
								 ModCommonErrorUndefined, ModErrorLevelError);
						}
#ifdef UNA_DEBUG
						ModDebugMessage << "substructure num: " << subMorphNum
									<< ModEndl;
#endif
						if (subMorphNum != 0) {
							// 下位構造がある
							goto submorp;
						}

						// 下位構造がない
						try {
							stemmer->stem(tmp2, normalized);
						} catch (ModException& e) {
							if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
								ModErrorMessage << e << ModEndl;
#endif
								ModRethrow(e);
							}
							ModErrorHandle::reset();
						}  // try
					} // if (stemmer->look(tmp2) == ModTrue)
				} // doStem
			} else if (tokenType == UNA_KNOWN_WORD) {
				// 英語トークンでない辞書語 -- 下位構造があれば取り出す

				; ModAssert(subMorphCurrent == 0);
				ModInt32 res;
				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
				if (res != UNA_KNOWN_WORD) {
					// エラー
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getSubMorph error: "
									<< unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
#ifdef UNA_DEBUG
				ModDebugMessage << "substructure num: " << subMorphNum
								<< ModEndl;
#endif
				if (subMorphNum != 0) {
					// 下位構造がある
					goto submorp;
				}

				// 下位構造がない -- 表記を得る
				if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

				normalized.clear();
				normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
				if (tmp.getLength() == 0) {
					// 正規化で長さが 0 になったものは無視し、次の形態素に移る
					continue;
				}

#ifdef UNA_DEBUG
				ModMessage << tmp << ' ' << normalized << ' '
						   << result_ << ModEndl;
#endif
			} else if (tokenType == UNA_UNKNOWN_WORD) {
				// 英語トークンでない非辞書語 -- 表記を取り出せばよい

				// 形態素の表記
				if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

				normalized.clear();
				normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
				if (normalized.getLength() == 0) {
					// 正規化で長さが 0 になったものは無視し、次の形態素に移る
					continue;
				}

#ifdef UNA_DEBUG
				ModMessage << tmp << " '" << normalized << "' "
						   << result_ << ModEndl;
#endif
			} else {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(tokenType) << ": " << target << ModEndl;
				; ModAssert(tokenType < 0 /*&& tokenType == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}

			// 形態素番号（何番目の形態素か; 1 origin）
			//		正規化表記を得る段階でスキップすることがあるので、ここでアペンド
			stream << (morphCurrent + 1)
				   << sep1_;
			hinshi = unaKApi_getHinName(handle,(morphBuffer+morphCurrent));
		}

		// 表記
		stream << normalized
			   << sep1_;

		// 形態素の品詞
		stream << hinshi
			   << sep1_;

		// 文節番号（何番目の文節か; 1 origin）
		stream << ((b) ? 0 : bunsetsuCurrent)
			   << sep1_;

		// 係り先の文節番号（何番目の文節か; 1 origin）
		stream << ((b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].target + 1)
			   << sep1_;

		// 係り受け関係の番号
		stream << ((b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].kuRel + 1)
			   << sep2_;

		// 形態素の辞書ベース名をdicNameVectorに保存
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + morphCurrent);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	result_ = stream.getString();
	
	return ModTrue;
}
//
// FUNCTION
// ModUnaAnalyzer::getExpand -- 形態素解析結果を取得し、展開して返す
//
// NOTES
// 形態素解析結果を取得し、展開して返す
// 呼び出す前、規則と正規化の初期化が必要
// 呼ぶ側で、delete [] expanded_;をしないと、メモリリークが発生する。
//
// ARGUMENTS
// ModUnicodeString& expanded_
//    展開した解析結果
// const ModNormExpMode chkorg_
//    展開の指示(展開モード)
//
// RETURN
// expanded_配列の要素数
//
// EXCEPTIONS
//
ModSize
ModUnaAnalyzer::getExpand( ModUnicodeString*& expanded_,
	const ModUnaExpMode chkOrg, ModUnicodeString& ostr, int& pos_)
{
	ModUnicodeString normalized;
	ModSize num(0);
	// はたしてここでのチェックは方針に沿っているのか？
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard, 
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}
	if ( doStem(mode) && stemmer == 0) { 
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard, 
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	// 正規化してgetMorph(ただしstemmingはしない)
	if ( doSub(mode)){
		if (getCheckNormalized(normalized, ostr, ModTrue, ModFalse, pos_) != ModTrue)
								//3rd arg:get orgStr. This equals v3.4.3.
			return 0;
	}
	else {
		// ここには mode=0,3,4,7 がくる
		// 3 -> 0 7->4が必要
		ModSize tMode = mode;
		mode = ( mode==3? 0 :(mode==7? 4:mode));
		if (getSimpleNormalized(normalized, ostr, ModTrue, pos_) != ModTrue){
								//3rd arg:get orgStr. This equals v3.4.3.
			mode = tMode;
			return 0;
		}
		mode = tMode;
	}
	
	// getMorphでは、形態素はあるが長さが0の場合はMorFalse以外はありえない
	; ModAssert(normalized.getLength()>0);

	// 展開を行う
	expanded_ = 0;
	try {
		// ModUnaExpMode → ModNormExpMode
		if ( chkOrg == ModUnaExpNoChk ){
			num = normalizer->expandBuf(normalized, expanded_, ModNormExpNoChk, 0, 0, ModTrue);
		}
		else{
			num = normalizer->expandBuf(normalized, expanded_, ModNormExpChkOrigStr, 0, 0, ModTrue);
		}

		// 展開されたということはカタカナなので、そのまま返す
		if ( num>1){
			return num;
		}
		// stemmer の使用が指定されていればステマーによる展開を実施
		else if ( doStem(mode) ){

			// そうでなければステマーの展開関数を呼んでみる
			if (num == 1) {
				// 正規化された表記が 0 番目の要素に入っているので、
				// normalized を置き換える（もともとの処理を継承）
				normalized = expanded_[0];
				delete [] expanded_; 
				num = 0; // 展開されなかったことと同じ
			}

			// ここには num=0,1でやってくる、いずれもexpandedはdelete済
			ModVector<ModUnicodeString> reexpanded;
			if ( stemmer->expand(normalized, reexpanded) == ModTrue){ // 展開された
				// 文字列分の確保を行う
				expanded_ = new ModUnicodeString[reexpanded.getSize()];
				ModSize i;
				for ( i = 0; i < reexpanded.getSize(); ++i){
					expanded_[i] = reexpanded.begin()[i];
				}
				num = reexpanded.getSize();
			}
		}
	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "getExpand failed: " << e << ModEndl;
#endif
		// eelete [] expanded; これは後の処理の責任
		ModRethrow(e);
	}

	// 単語列の末尾でなければ、最低限、元の形態素は返す
	if ( num == 0){
		expanded_ = new ModUnicodeString[1];	// １文字列分の確保
		expanded_[0] = normalized;
		num = 1;
	}
	return num;
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockNormalized -- 異表記正規化した解析結果の取得
//
// NOTES
// 形態素解析したブロックごとの解析結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の異表記正規化済み表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    解析結果の原表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// ModBoolean getOriginal_
//    原表記の取得指示
// ModVector<int>* costVector_
//	  vector including words cost
// ModVector<int>* uposVector_
//	  vector including unified part of speech
// ModBoolean ignore_
//	  ModTrue: the character string deleted by regularization
//             is disregarded. Default setting.
//	  ModFalse:it doesn't disregard it.
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlockNormalized( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	ModBoolean getOriginal_,
	ModVector<int>* costVector_ = 0,
	ModVector<int>* uposVector_ = 0,
	ModBoolean ignore_)
{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	if ( doStem(mode) && stemmer == 0) {
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	if (targetLength == 0) { // 解析対象が残っていない
		return ModFalse;
	}

	if ( doSub(mode)){
		if ( doStem(mode)){
			if ( resource->optionStatus && execQuick){ // 高速化データあり
				return getBlockCheckSpeed(formVector_, ostrVector_,
					posVector_, getOriginal_, costVector_, uposVector_, ignore_);
			}
			else{ // 高速化データなし
				return getBlockCheck(formVector_, ostrVector_,
					posVector_, getOriginal_, costVector_, uposVector_, ignore_);
			}
		}
		else{
			// 下位構造にばらす
			return getBlockCheck(formVector_, ostrVector_,
				posVector_, getOriginal_, costVector_, uposVector_, ignore_);
		}
	}
	else{
		// 下位構造にばらさない
		return getBlockSimple(formVector_, ostrVector_, posVector_, getOriginal_, costVector_, uposVector_, ignore_);
	}
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockSimple -- 単純な解析結果の取得
//
// NOTES
// getBlockNormalizedの下請け関数
// 英語トークンが見つかった場合、下位構造の有無にかかわらず、マージした
// 結果をトークンとする。
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の異表記正規化済み表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    解析結果の原表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// ModBoolean getOriginal_
//    原表記の取得指示
// ModVector<int> * costVector_
// 	  cost vector of analytical result
// ModVector<int> * uposVector_
//	  unified part of speech vector of analytical result
// ModBoolean ignore_
//	  ModTrue: the character string deleted by regularization
//	    	   is disregarded. Default setting.
//	  ModFalse:it doesn't disregard it.
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// ModCommonErrorUndefined
//	  Execution error of unakapi function
//
ModBoolean
ModUnaAnalyzer::getBlockSimple( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	ModBoolean getOriginal_,
	ModVector<int> *costVector_ = 0,
	ModVector<int> *uposVector_ = 0,
	ModBoolean ignore_)
{
	ModInt32 res;

	// 解析の実行
	analyze(ModTrue);

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);
	if ( getOriginal_){
		ostrVector_.clear();
		ostrVector_.reserve(morphNum);
	}
	if(costVector_){
		costVector_->clear();				/* temp */
		costVector_->reserve(morphNum);		/* temp */
	}
	if(uposVector_){
		uposVector_->clear();
		uposVector_->reserve(morphNum);		/* temp */
	}
	ModUnicodeString originalString;
	ModSize ohyoukiLength;
	ModUnicodeChar* ohyouki;

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	for (morphCurrent = 0; morphCurrent < morphNum; ++morphCurrent) {

		// 形態素の表記
		if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
							 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		ModUnicodeString original, normalized;
		original.append(hyouki, hyoukiLength);

		normalizer->normalizeBuf(original, normalized, 0, 0, ModNormalized);
		if(ignore_) {
			if (normalized.getLength() == 0) { // 正規化で長さが 0
				continue;
			}
		}

		if (doStem(mode)) { // ステミングする
			ModUnicodeString tmp(normalized);
			normalized.clear();

			// ASCII だけで構成されているかは必ず検査される
			try {
				stemmer->stem(tmp, normalized);
			} catch (ModException& e) {
				if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
					ModErrorMessage << e << ModEndl;
#endif
					ModRethrow(e);
				}
				ModErrorHandle::reset();
			}
		}

		if ( getOriginal_){
			if (res = unaKApi_getOriginalHyoki(handle, targetStart,
				(morphBuffer+morphCurrent), &ohyouki,&ohyoukiLength)<0){
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
			originalString.clear();
			originalString.append(ohyouki, ohyoukiLength);
			ostrVector_.pushBack(originalString);
		}

		formVector_.pushBack(normalized);
		int pos = (morphBuffer + morphCurrent)->hinshi;
		posVector_.pushBack(pos);

		if(costVector_) {
			costVector_->pushBack((morphBuffer + morphCurrent)->cost);
#if 0
			printf("ModUnaAnalyzer::getBlockSimple cost=%d",(morphBuffer + morphCurrent)->cost);
#endif
		}

		if(uposVector_)
			uposVector_->pushBack(m_dicSet->getTypeCode(pos));
	}

	// 形態素の辞書ベース名をdicNameVectorに保存
	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);
	for (int n = 0; n < morphNum; n++) {
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + n);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}


//
// FUNCTION
// ModUnaAnalyzer::getBlockCheckSpeed -- 高速な解析結果の取得
//
// NOTES
// getBlockNormalizedの下請け関数
// 異表記正規化結果を辞書を用いることにより高速に生成する
// stemmingあり、下位構造分割ありのモードで使用
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の異表記正規化済み表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    解析結果の原表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// ModBoolean getOriginal_
//    原表記の取得指示
// ModVector<int > *costVector_
//	  cost vector of analytical result
// ModVector<int> *uposVector_
//	  unified part of speech vector of analytical result
// ModBoolean ignore_
//	  ModTrue: the character string deleted by regularization
//		       is disregarded. Default setting.
//	  ModFalse:it doesn't disregard it.
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// ModCommonErrorUndefined
//	  Execution error of unakapi function
//
ModBoolean
ModUnaAnalyzer::getBlockCheckSpeed( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	ModBoolean getOriginal_,
	ModVector<int > *costVector_ = 0,
	ModVector<int> *uposVector_ = 0,
	ModBoolean ignore_)
{
	ModInt32 res;

	// 解析の実行
	analyze(ModTrue);

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);
	if ( getOriginal_){
		ostrVector_.clear();
		ostrVector_.reserve(morphNum);
	}
	if(costVector_){
		costVector_->clear();				/* temp */
		costVector_->reserve(morphNum);		/* temp */
	}
	if(uposVector_){
		uposVector_->clear();
		uposVector_->reserve(morphNum);		/* temp */
	}
	ModUnicodeString originalString;
	ModSize ohyoukiLength;
	ModUnicodeString normalized;
	ModUnicodeChar* hyouki;
	ModUnicodeChar* ohyouki;
	ModSize hyoukiLength;
	int pos;
	ModUInt32 cost = 255;		/* temp */
	unaAppInfoT* appInfo;
 	ModUnicodeString& tmp = work1;
 	ModUnicodeString& tmp2 = work2;

	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);

	subMorphNum = subMorphCurrent = 0;

	// 形態素数だけループする
	for (morphCurrent = 0; morphCurrent < morphNum ||
		subMorphNum>0; ++morphCurrent) {

		// 前回の処理で下位構造があった
		if (subMorphNum != 0) {
			; ModAssert(morphCurrent != 0);
			--morphCurrent;
		submorp:
			// アプリ情報をとってくる
			res = unaKApi_getAppInfo(handle, subMorphBuffer+subMorphCurrent,
						  &appInfo);
			// アプリ情報の長さは１以上
			; ModAssert(appInfo->len != 0);

			if (tokenType == UNA_ENG_TOKEN) { // 英語トークンの場合
				tmp.clear();
				tmp.append(
					una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));

				// cache を使って高速化
				long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
				if ( tmp == cache1[cnum]){ // キャッシュにヒット
					normalized = cache2[cnum];
				}
				else{ // キャッシュにヒットしなかった
					normalized.clear();
					normalizer->normalizeBuf(tmp,normalized,0,0,ModNormalized);
					cache1[cnum] = tmp;
					cache2[cnum] = normalized;
				}
				if (normalized.getLength() == 0) {
					if (++subMorphCurrent == subMorphNum) {
						subMorphCurrent = subMorphNum = 0;
					}
					if(ignore_) {
						continue;
					}
				} else{
					tmp2.clear();
					tmp2.append(normalized);
					normalized.clear();
					try {
						stemmer->stem(tmp2, normalized);
					} catch (ModException& e) {
						if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
							ModErrorMessage << e << ModEndl;
#endif
							ModRethrow(e);
						}
						ModErrorHandle::reset();
					}
					ModErrorHandle::reset();
				}
			} else {  // 英語トークン以外の辞書語の場合
				; ModAssert(tokenType == UNA_KNOWN_WORD);
				normalized.clear();
				normalized.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));
				; ModAssert(normalized.getLength() > 0);
			}


			if ( getOriginal_){
				res = unaKApi_getOriginalHyoki(handle, targetStart,
					(subMorphBuffer+subMorphCurrent), &ohyouki, &ohyoukiLength);
				if (res <0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos=(subMorphBuffer+subMorphCurrent)->hinshi;
			cost = (subMorphBuffer+subMorphCurrent)->cost;
#if 0
			printf("ModUnaAnalyzer::getBlockCheckSpeed(1) cost=%d\n",cost);	
#endif
			if (++subMorphCurrent == subMorphNum) {
				// すべての下位構造を処理し終えたらクリアする
				subMorphCurrent = subMorphNum = 0;
			}

		} else {
			// 通常の処理
			; ModAssert( morphCurrent < morphNum);

			tokenType = unaKApi_getAppInfo(handle, morphBuffer + morphCurrent,
										   &appInfo);
			if (tokenType == UNA_ENG_TOKEN) {
				// 英語トークン -- 下位構造をチェックする
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						   (appInfo->len)/sizeof(ModUnicodeChar));

				// cache を使って高速化
				long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
				if ( tmp == cache1[cnum]){ // キャッシュにヒットした
					normalized = cache2[cnum];
				}
				else{
					normalized.clear();
					normalizer->normalizeBuf(tmp, normalized,0,0,ModNormalized);
	            	cache1[cnum] = tmp;
					cache2[cnum] = normalized;
				}

				if (normalized.getLength() == 0) {
					if(ignore_) {
						continue;
					}
				} else {
					tmp2.clear();
					tmp2.append(normalized);

					if (stemmer->look(tmp2) == ModFalse) { // 辞書にない単語
						res = unaKApi_getSubMorph(handle,
									morphBuffer + morphCurrent,
									subMorphBuffer, &subMorphNum,
									unaLocalKeitaisoSize);
						; ModAssert( res == UNA_ENG_TOKEN);
						if (subMorphNum != 0) {
							// 下位構造がある
							goto submorp;
						}
					}

					// 辞書にあるか、下位構造が無い単語
					normalized.clear();
					try {
						stemmer->stem(tmp2, normalized);
					} catch (ModException& e) {
						if (e.getErrorNumber()!=ModCommonErrorBadArgument) {
#ifdef DEBUG
							ModErrorMessage << e << ModEndl;
#endif
							ModRethrow(e);
						}
						ModErrorHandle::reset();
					}
					ModErrorHandle::reset();
				}
			} else if (tokenType == UNA_KNOWN_WORD) {
				// 英語トークンでない辞書語 -- 下位構造があれば取り出す

				; ModAssert(subMorphCurrent == 0);

				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
				if (res != UNA_KNOWN_WORD) { // エラー
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getSubMorph error: "
						<< unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				if (subMorphNum != 0) { // 下位構造がある
					goto submorp;
				}

				// 下位構造がない -- 既取得のアプリ情報を使う
				normalized.clear();
				if ( appInfo->len > 0 ){
					normalized.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
				}

				if(ignore_) {
					if (normalized.getLength() == 0) {
						continue;
					}
				}

			// 英語トークンでない非辞書語 -- 表記を取り出せばよい
			} else if (tokenType == UNA_UNKNOWN_WORD) {

				// 形態素の表記
				if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

				// cache を使って高速化
				long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
				if ( tmp == cache1[cnum]){
					normalized = cache2[cnum];
				}
				else{
					normalized.clear();
					normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
					if (normalized.getLength() == 0) {
						if(ignore_) {
							continue;
						}
					} else {
						cache1[cnum] = tmp;
						cache2[cnum] = normalized;
					}
				}

			// その他
			} else {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(res) << ": " << target << ModEndl;
				; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}


			if ( getOriginal_){
				res = unaKApi_getOriginalHyoki(handle, targetStart,
					(morphBuffer+morphCurrent), &ohyouki,&ohyoukiLength);
				if (res < 0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos = (morphBuffer + morphCurrent)->hinshi;
			cost = (morphBuffer + morphCurrent)->cost;
#if 0
			printf("ModUnaAnalyzer::getBlockCheckSpeed(2) cost=%d\n",cost);	
#endif
		}
		formVector_.pushBack(normalized);
		posVector_.pushBack(pos);
		
		if ( getOriginal_){
			ostrVector_.pushBack(originalString);
		}

		if(costVector_) {
			costVector_->pushBack(cost);
#if 0
			printf("ModUnaAnalyzer::getBlockCheckSpeed cost=%d\n",cost);
#endif
		}

		if(uposVector_)
			uposVector_->pushBack(m_dicSet->getTypeCode(pos));

		// 形態素の辞書ベース名をdicNameVectorに保存
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + morphCurrent);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockCheck -- Acquisition of the analytical result which considers subordinate structure
//
// NOTES
//	 getBlockNormalizedの下請け関数
//	 When the English token is found, concerning the word which is not registered to the stemmer dictionary
//	 If you inspect the presence of subordinate structure and there is a subordinate structure the individual token is returned.
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の異表記正規化済み表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    解析結果の原表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// ModBoolean getOriginal_
//    原表記の取得指示
// ModVector<int > *costVector_
//	  cost vector of analytical result
// ModVector<int> *uposVector_
//	  unified part of speech vector of analytical result
// ModBoolean ignore_
//	  ModTrue: the character string deleted by regularization
//	  		   is disregarded. Default setting.
//	  ModFalse:it doesn't disregard it.
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//	ModCommonErrorUndefined
//	  Execution error of unakapi function
//
ModBoolean
ModUnaAnalyzer::getBlockCheck( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	ModBoolean getOriginal_,
	ModVector<int> *costVector_ = 0,
	ModVector<int> *uposVector_ = 0,
	ModBoolean ignore_)
{
	ModInt32 res;

	// 解析の実行
	analyze(ModTrue);

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);

	if (getOriginal_){
		ostrVector_.clear();
		ostrVector_.reserve(morphNum);
	}

	if(costVector_){
		costVector_->clear();				/* temp */
		costVector_->reserve(morphNum);		/* temp */
	}

	if(uposVector_){
		uposVector_->clear();
		uposVector_->reserve(morphNum);		/* temp */
	}

	ModUnicodeString originalString;
	ModSize ohyoukiLength;
	ModUnicodeString tmp, normalized;
	ModUnicodeChar* hyouki;
	ModUnicodeChar* ohyouki;
	ModSize hyoukiLength;
	int pos;
	ModUInt32 cost = 255;
	unaAppInfoT* appInfo;

	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);

	subMorphNum = subMorphCurrent = 0;

	// 形態素数だけループする
	for (morphCurrent = 0; morphCurrent < morphNum || subMorphNum>0; ++morphCurrent) {

		// 前回の処理で下位構造があった
		if (subMorphNum != 0) {
			; ModAssert(morphCurrent != 0);
			--morphCurrent;
		submorp:
			if (tokenType == UNA_ENG_TOKEN) { // 英語トークンの場合
				res = unaKApi_getAppInfo(handle,
							 subMorphBuffer + subMorphCurrent,
							 &appInfo);
				if (res != UNA_ENG_TOKEN) {
#ifdef DEBUG
					ModErrorMessage << unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				// アプリ情報の長さは１以上
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(
					una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));

			} else {  // 英語トークン以外の辞書語の場合
				; ModAssert(tokenType == UNA_KNOWN_WORD);

				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if (unaKApi_getHyoki(handle, subMorphBuffer + subMorphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				; ModAssert(hyoukiLength != 0);

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

			}
			normalized.clear();
			normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
			if (normalized.getLength() == 0) {
				if (++subMorphCurrent == subMorphNum) {
					subMorphCurrent = subMorphNum = 0;
				}
				if(ignore_) {
					continue;
				}
			}

			if ( tokenType == UNA_ENG_TOKEN && doStem(mode)){
				ModUnicodeString tmp2(normalized);
				normalized.clear();

				try {
					stemmer->stem(tmp2, normalized);
				} catch (ModException& e) {
					if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
						ModErrorMessage << e << ModEndl;
#endif
						ModRethrow(e);
					}
					ModErrorHandle::reset();
				}
			}
			if ( getOriginal_){
				if (res = unaKApi_getOriginalHyoki(handle, targetStart,
					(subMorphBuffer+subMorphCurrent), &ohyouki,&ohyoukiLength)<0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos=(subMorphBuffer+subMorphCurrent)->hinshi;
			cost=(subMorphBuffer+subMorphCurrent)->cost;
			if (++subMorphCurrent == subMorphNum) {
				// すべての下位構造を処理し終えたらクリアする
				subMorphCurrent = subMorphNum = 0;
			}

		} else {
			// 通常の処理
			; ModAssert( morphCurrent < morphNum);

			tokenType = unaKApi_getAppInfo(handle, morphBuffer + morphCurrent,
										   &appInfo);
			if (tokenType == UNA_ENG_TOKEN) {
				// 英語トークン -- 下位構造をチェックする
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						   (appInfo->len)/sizeof(ModUnicodeChar));

			} else if (tokenType == UNA_KNOWN_WORD) {
				// 英語トークンでない辞書語 -- 下位構造があれば取り出す

				; ModAssert(subMorphCurrent == 0);

				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
				if (res != UNA_KNOWN_WORD) { // エラー
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getSubMorph error: "
						<< unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				if (subMorphNum != 0) { // 下位構造がある
					goto submorp;
				}

				// 下位構造がない -- 表記を得る
				if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

			// 英語トークンでない非辞書語 -- 表記を取り出せばよい
			} else if (tokenType == UNA_UNKNOWN_WORD) {

				// 形態素の表記
				if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

			// その他
			} else {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(res) << ": " << target << ModEndl;
				; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}


			normalized.clear();
			normalizer->normalizeBuf(tmp, normalized, 0, 0, ModNormalized);
			if(ignore_) {
				if (normalized.getLength() == 0) {
					continue;
				}
			}

			if ( tokenType == UNA_ENG_TOKEN && doStem(mode)){
				ModUnicodeString tmp2(normalized);
				normalized.clear();
				if (stemmer->look(tmp2) == ModTrue) 
				{
					// 辞書にある単語 -- ステミング結果を返す
					stemmer->stem(tmp2, normalized);

				} else {
					// 辞書にない単語 -- 下位構造を得る
					res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
					if (res != UNA_ENG_TOKEN) {// エラー
#ifdef DEBUG
						ModErrorMessage << "unaKApi_getSubMorph error: "
							<< unaKApi_getErrMsg(res) << ModEndl;
#endif
						ModThrow(ModModuleStandard,
								 ModCommonErrorUndefined, ModErrorLevelError);
					}
					if (subMorphNum != 0) {
						// 下位構造がある
						goto submorp;
					}

					// 下位構造がない
					try {
						stemmer->stem(tmp2, normalized);
					} catch (ModException& e) {
						if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
							ModErrorMessage << e << ModEndl;
#endif
							ModRethrow(e);
						}
						ModErrorHandle::reset();
					}
				}
			}
			if ( getOriginal_){
				if (res = unaKApi_getOriginalHyoki(handle, targetStart,
					(morphBuffer+morphCurrent), &ohyouki,&ohyoukiLength)<0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos = (morphBuffer + morphCurrent)->hinshi;
			cost = (morphBuffer + morphCurrent)->cost;

		}
		formVector_.pushBack(normalized);
		posVector_.pushBack(pos);
		if ( getOriginal_){
			ostrVector_.pushBack(originalString);
		}
		if(costVector_)
		{
			costVector_->pushBack(cost);
#if 0
			printf("ModUnaAnalyzer::getBlockCheck cost=%d\n",cost);
#endif
		}
		if(uposVector_)
			uposVector_->pushBack(m_dicSet->getTypeCode(pos));

		// 形態素の辞書ベース名をdicNameVectorに保存
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + morphCurrent);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getBlockCompoundDivOnly -- 解析結果の取得
//
// NOTES
// getBlockNormalizedの下請け関数
// stemmingあり、下位構造分割ありのモードで使用
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//	  解析結果の異表記正規化済み表記ベクター
// ModVector<int>& posVector_
//	  解析結果の品詞ベクター
// ModVector<ModUnicodeString> *ostrVector_
//	  解析結果の原表記ベクター
//	ModVector<int> *costVector_
//	  cost arrangement
//	ModVector<int> *uposVector_
//	  Unified part of speech arrangement
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlockCompoundDivOnly( ModVector<ModUnicodeString>& formVector_,
		ModVector<int>& posVector_,
		ModVector<ModUnicodeString> *ostrVector_,
		ModVector<int > *costVector_,
		ModVector<int> *uposVector_)
{
	if (targetLength == 0) { // 解析対象が残っていない
		return ModFalse;
	}

	ModInt32 res;

	// 解析の実行
	analyze(ModTrue);

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);

	if (ostrVector_){
		ostrVector_->clear();
		ostrVector_->reserve(morphNum);
	}

	if(costVector_){
		costVector_->clear();
		costVector_->reserve(morphNum);
	}

	if(uposVector_){
		uposVector_->clear();
		uposVector_->reserve(morphNum);
	}

	ModUnicodeString originalString;
	ModSize ohyoukiLength;
	ModUnicodeString tmp;
	ModUnicodeChar* hyouki;
	ModUnicodeChar* ohyouki;
	ModSize hyoukiLength;
	int pos;
	ModUInt32 cost = 255;
	unaAppInfoT* appInfo;

	ModUnaMiddleAnalyzer::dicNameVector.clear();
	ModUnaMiddleAnalyzer::dicNameVector.reserve(morphNum);

	subMorphNum = subMorphCurrent = 0;

	// 形態素数だけループする
	for (morphCurrent = 0; morphCurrent < morphNum || subMorphNum>0; ++morphCurrent) {

		// 前回の処理で下位構造があった
		if (subMorphNum != 0) {
			; ModAssert(morphCurrent != 0);
			--morphCurrent;
		submorp:
			if (tokenType == UNA_ENG_TOKEN) { // 英語トークンの場合
				res = unaKApi_getAppInfo(handle,
							 subMorphBuffer + subMorphCurrent,
							 &appInfo);
				if (res != UNA_ENG_TOKEN) {
#ifdef DEBUG
					ModErrorMessage << unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				// アプリ情報の長さは１以上
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(
					una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));

			} else {  // 英語トークン以外の辞書語の場合
				; ModAssert(tokenType == UNA_KNOWN_WORD);

				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if (unaKApi_getHyoki(handle, subMorphBuffer + subMorphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				; ModAssert(hyoukiLength != 0);

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);
			}
			
			if ( ostrVector_){
				if (res = unaKApi_getOriginalHyoki(handle, targetStart,
					(subMorphBuffer+subMorphCurrent), &ohyouki,&ohyoukiLength)<0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos=(subMorphBuffer+subMorphCurrent)->hinshi;
			cost=(subMorphBuffer+subMorphCurrent)->cost;
			if (++subMorphCurrent == subMorphNum) {
				// すべての下位構造を処理し終えたらクリアする
				subMorphCurrent = subMorphNum = 0;
			}

		} else {
			// 通常の処理
			; ModAssert( morphCurrent < morphNum);

			tokenType = unaKApi_getAppInfo(handle, morphBuffer + morphCurrent,
										   &appInfo);
			if (tokenType == UNA_ENG_TOKEN) {
				// 英語トークン -- 下位構造をチェックする
				; ModAssert(appInfo->len != 0);

				tmp.clear();
				tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						   (appInfo->len)/sizeof(ModUnicodeChar));

			} else if (tokenType == UNA_KNOWN_WORD) {
				// 英語トークンでない辞書語 -- 下位構造があれば取り出す

				; ModAssert(subMorphCurrent == 0);

				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
				if (res != UNA_KNOWN_WORD) { // エラー
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getSubMorph error: "
						<< unaKApi_getErrMsg(res) << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				if (subMorphNum != 0) { // 下位構造がある
					goto submorp;
				}

				// 下位構造がない -- 表記を得る
				if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

			// 下位構造がない -- 表記を得る
			} else if (tokenType == UNA_UNKNOWN_WORD) {

				// 形態素の表記
				if (unaKApi_getHyoki(handle, morphBuffer + morphCurrent,
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}

				tmp.clear();
				tmp.append(hyouki, hyoukiLength);

			// その他
			} else {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(res) << ": " << target << ModEndl;
				; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}

			if ( ostrVector_){
				if (res = unaKApi_getOriginalHyoki(handle, targetStart,
					(morphBuffer+morphCurrent), &ohyouki,&ohyoukiLength)<0){
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
				originalString.clear();
				originalString.append(ohyouki, ohyoukiLength);
			}
			pos = (morphBuffer + morphCurrent)->hinshi;
			cost = (morphBuffer + morphCurrent)->cost;

		}
		formVector_.pushBack(tmp);
		posVector_.pushBack(pos);

		if (ostrVector_)
			ostrVector_->pushBack(originalString);

		if(costVector_)
			costVector_->pushBack(cost);

		if(uposVector_)
			uposVector_->pushBack(m_dicSet->getTypeCode(pos));

		// 形態素の辞書ベース名をdicNameVectorに保存
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + morphCurrent);
		ModUnicodeString utmp(dicName? dicName: "");
		ModUnaMiddleAnalyzer::dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// かかりうけ解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果のアプリケーション情報
// const ModUnicodeChar sep1_
//    セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//    セパレータ２（レコード区切り）
// const ModBoolean getNormalized_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlock(ModUnicodeString& result_,
		const ModUnicodeChar separator1_, const ModUnicodeChar separator2_,
		const ModBoolean getNormalized_)
{
	if (getNormalized_ == ModTrue) {
		return getBlockNormalized(result_, separator1_, separator2_);
	}
	return ModUnaMiddleAnalyzer::getBlock(result_, separator1_, separator2_);
}

//
// FUNCTION
// ModUnaAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 形態素解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    異表記正規化後の表記配列
// ModVector<int>& posVector_
//    品詞配列
// const ModBoolean getNormalized_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlock( ModVector<ModUnicodeString>& formVector_,
		ModVector<int>& posVector_, const ModBoolean getNormalized_)
{
	if (getNormalized_ == ModTrue) {
		return getBlockNormalized(formVector_, work4, posVector_,ModFalse);
	} else {
		if (doSub(mode)) {
			return getBlockCompoundDivOnly(formVector_, posVector_);
		} else {
			return ModUnaMiddleAnalyzer::getBlock(formVector_, posVector_);
		}
	}
}

//
// FUNCTION
// ModUnaAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 形態素解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    異表記正規化後の表記配列
// ModVector<ModUnicodeString>& ostrVector_
//    原表記配列
// ModVector<int>& posVector_
//    品詞配列
// const ModBoolean getNormalized_
//    正規化指示
// ModVector<int > *costVector_
//	  cost vector of analytical result
// ModVector<int> *uposVector_
//	  unified part of speech vector of analytical result
// ModBoolean ignore_
//	  ModTrue: the character string deleted by regularization
//	    	   is disregarded. Default setting.
//	  ModFalse:it doesn't disregard it.
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getBlock( ModVector<ModUnicodeString>& formVector_,
		ModVector<ModUnicodeString>& ostrVector_,
		ModVector<int>& posVector_,
		const ModBoolean getNormalized_,
		ModVector<int> *costVector_,
		ModVector<int> *uposVector_,
		ModBoolean ignore_)
{
	if (getNormalized_ == ModTrue) {
		return getBlockNormalized(formVector_, ostrVector_, posVector_, ModTrue, costVector_, uposVector_, ignore_);
	} else {
		if (doSub(mode)) {
			return getBlockCompoundDivOnly(formVector_, posVector_, &ostrVector_, costVector_, uposVector_);
		} else {
			return ModUnaMiddleAnalyzer::getBlock(formVector_, ostrVector_, posVector_, costVector_, uposVector_);
		}
	}
}

//
// FUNCTION
// ModUnaAnalyzer::getDicName -- 解析結果の辞書ベース名の取得
//
// NOTES
// 形態素解析したブロックごとの辞書ベース名を取得する
// 形態素と辞書ベース名の個数および位置は一対一対応している
//
// ARGUMENTS
// ModVector<ModUnicodeString>& dicNameVector_
//    辞書ベース名のベクター
//
// RETURN
// 解析結果があれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getDicName(ModVector<ModUnicodeString>& dicNameVector_)
{
	return ModUnaMiddleAnalyzer::getDicName(dicNameVector_);
}

//
// FUNCTION
// ModUnaAnalyzer::getMorph -- 形態素解析結果の取得
//
// NOTES
// 形態素解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// const ModBoolean getNormalized_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getMorph( ModUnicodeString& result_, const ModBoolean getNormalized_)
{
	if (getNormalized_ == ModTrue) {
		return getNormalized(result_,work3,ModFalse);
	}
	if (doSub(mode)) {
		//下位構造にばらす
		return getCheck(result_, work3, ModFalse);
	}
	// 下位構造にばらさない -- ModUnaMiddleAnalyzer の関数を呼び出す
	return get(result_, ModFalse);
}

//
// FUNCTION
// ModUnaAnalyzer::getMorph -- 形態素解析結果の原表記付き取得
//
// NOTES
// 形態素解析結果を取得する。
// 原表記はセットされたテキストへのポインタとしてかえる
// 異表記正規化の結果、ヌルになる単語があってもそのまま返す。
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeChar* &original_
//    原表記
// ModSize &len_
//    原表記の長さ
// int pos_
//    品詞
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getMorph( ModUnicodeString& result_,
	ModUnicodeChar* &original_, ModSize &len_, int& pos_)
{
	if (normalizer == 0) {
#ifdef DEBUG
		ModErrorMessage << "normalizer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}
	if (stemmer == 0) {
#ifdef DEBUG
		ModErrorMessage << "stemmer is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

 	ModUnicodeString& tmp = work1;
 	ModUnicodeString& tmp2 = work2;
 	tmp.clear();
	ModInt32 res;
	unaAppInfoT* appInfo;

	len_ = 0; // 初期化
retry:
	result_.clear();

	if (subMorphCurrent < subMorphNum) { // 下位構造がある場合
		tokenType = unaKApi_getAppInfo(handle,
				subMorphBuffer + subMorphCurrent, &appInfo);
		if (tokenType == UNA_ENG_TOKEN) { // 下位構造が英語トークン
			// 英語トークンは同じ表記であっても改行にまたがっているか
			// 否かによってステミング結果が変わるので、正規化までをキャッシュ
			tmp.append(
				una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				(appInfo->len)/sizeof(ModUnicodeChar));

			// cache を使って高速化
			long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
			if ( tmp == cache1[cnum]){ // キャッシュにヒット
				tmp2 = cache2[cnum];
			}
			else{ // キャッシュにヒットしなかった
				tmp2.clear();
				normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
				cache1[cnum] = tmp;
				cache2[cnum] = tmp2;
			}

			if (tmp2.getLength() == 0 || !doStem(mode)) { // この関数だけの特殊処理
				result_ = tmp2; // ヌル文字列 or 正規化結果を返す
			}
			else{
				try {
					stemmer->stem(tmp2, result_);
				} catch (ModException& e) {
					if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
						ModErrorMessage << e << ModEndl;
#endif
						ModRethrow(e);
					}
					ModErrorHandle::reset();
				}
			}
		} else { // 下位構造が英語トークン以外の辞書語の場合
			; ModAssert(tokenType == UNA_KNOWN_WORD);

			// アプリ辞書無し
			if (appInfo->len == 0 && !(resource->optionStatus && execQuick) ){
				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if ( unaKApi_getHyoki(handle,subMorphBuffer + subMorphCurrent,
						&hyouki, &hyoukiLength) <0){
#ifdef DEBUG
							ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
							ModThrow(ModModuleStandard,
								ModCommonErrorUndefined, ModErrorLevelError);
				}
				; ModAssert(hyoukiLength != 0);
				tmp.append(hyouki, hyoukiLength);
				normalizer->normalizeBuf(tmp, result_, 0, 0, ModNormalized);
				if ( result_.getLength() == 0){
					tmp.clear();
					++subMorphCurrent;
					goto retry;
				}
			}
			else{
				result_.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
					(appInfo->len)/sizeof(ModUnicodeChar));
				; ModAssert(result_.getLength() > 0);
			}

			// ステミングは不要
		}

		if (unaKApi_getOriginalHyoki(handle, targetStart,
			(subMorphBuffer + subMorphCurrent), &original_, &len_) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard, 
				 ModCommonErrorUndefined, ModErrorLevelError);
		}
		if ( subMorphCurrent==0 && morphCurrent==0 && targetStart !=original_){
			len_ += (int)(original_ - targetStart);
			original_ = (unsigned short*)targetStart;
		}
		; ModAssert(len_ != 0);
		pos_ = (subMorphBuffer+subMorphCurrent)->hinshi;

		++subMorphCurrent;
		return ModTrue;
	} // end of 下位構造がある場合

	// 下位構造が無い場合
	subMorphNum = subMorphCurrent = 0;

	if (morphCurrent == morphNum) { // 残り形態素が無い
		// 解析実行
		morphCurrent = 0;
		morphNum = 0;
		analyze(ModTrue);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	// 処理すべき形態素が残っている場合
	tokenType
		= unaKApi_getAppInfo(handle, morphBuffer + morphCurrent, &appInfo);
	if (tokenType == UNA_ENG_TOKEN) { // 残り形態素が英語トークン 
		; ModAssert(appInfo->len != 0);
		tmp.append(una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
		// cache を使って高速化
		long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
		if ( tmp == cache1[cnum]){ // キャッシュにヒットした
			tmp2 = cache2[cnum];
		}
		else{ // キャッシュにヒットしなかった
			tmp2.clear();
			normalizer->normalizeBuf(tmp, tmp2, 0, 0, ModNormalized);
			cache1[cnum] = tmp;
			cache2[cnum] = tmp2;
		}

		// そのまま結果を返すケース( ステミング無&下位構造無)
		if (tmp2.getLength() == 0 || (!doStem(mode) && !doSub(mode))){
			result_ = tmp2;
		}
		else {
			// 下位構造展開するケース(ステミング無&下位構造有)
			if ((!doStem(mode) || stemmer->look(tmp2)==ModFalse) && doSub(mode)) { // この関数だけの特殊処理
				; ModAssert(subMorphCurrent == 0);
				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
						  subMorphBuffer, &subMorphNum, unaLocalKeitaisoSize);
				; ModAssert (res == UNA_ENG_TOKEN);

#ifdef UNA_DEBUG
				ModDebugMessage << "substructure num: "
					<< subMorphNum << ModEndl;
#endif
				if (subMorphNum != 0) { // 下位構造があった 
					// とんだ先で下位構造を得る処理を実施する
					tmp.clear();
					tmp2.clear();
					++morphCurrent;
					goto retry;
				}
			}
			if ( doStem(mode)){
				// ステミング有(上位単語不成立で下位構造展開がある場合を除く)
				// ステミング結果をつくる
				try {
					stemmer->stem(tmp2, result_);
				} catch (ModException& e) {
					if (e.getErrorNumber() != ModCommonErrorBadArgument) {
#ifdef DEBUG
						ModErrorMessage << e << ModEndl;
#endif
						ModRethrow(e);
					}
					ModErrorHandle::reset();
				}
			}
			else{
				result_ = tmp2;
			}
		}
	} else {  // 残り形態素が英語トークンでない
		if (tokenType == UNA_KNOWN_WORD ) { // 辞書語 
			if ( doSub(mode)){
				// 下位構造があれば取り出す
				; ModAssert(subMorphCurrent == 0);
				res = unaKApi_getSubMorph(handle, morphBuffer + morphCurrent,
										  subMorphBuffer, &subMorphNum,
										  unaLocalKeitaisoSize);
				; ModAssert (res == UNA_KNOWN_WORD);
#ifdef UNA_DEBUG
				ModDebugMessage << "substructure num: " << subMorphNum << ModEndl;
#endif
				if (subMorphNum != 0) { // 下位構造があった 
					// とんだ先で下位構造を得る処理を実施する
					tmp.clear();
					tmp2.clear();
					++morphCurrent;
					goto retry;
				}
			}
		} else if (tokenType != UNA_UNKNOWN_WORD) { // エラーチェック
			// 英語トークンでなく辞書語/非辞書語でもない
#ifdef DEBUG
			; ModAssert(tokenType < 0 /*&& tokenType == UNA_NO_APP_DIC*/);
			ModErrorMessage << "unaKApi_getAppInfo error: "
				<< unaKApi_getErrMsg(tokenType) << ": " << target << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		// ここに来るのは英語トークンでなく下位構造がない場合

		// 辞書語のアプリ情報の長さは０もあり得る
		result_.clear();
		if ( appInfo->len > 0 ){ // appInf->len
			result_.append( una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
				   (appInfo->len)/sizeof(ModUnicodeChar));
		}
		// 辞書語でアプリ情報の長さ0 と未登録語で長さ0
		else {
			ModUnicodeChar* hyouki;
			ModSize hyoukiLength;
			if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
				&hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
				ModThrow(ModModuleStandard,
				ModCommonErrorUndefined, ModErrorLevelError);
			}
			; ModAssert(hyoukiLength != 0);

			tmp.append(hyouki, hyoukiLength);
			// cache を使って高速化
			long cnum = ((tmp[0] & 0x000f)<<2) + (tmp.getLength()&0x03);
			if ( tmp == cache1[cnum]){
				result_ = cache2[cnum];
			}
			else{
				normalizer->normalizeBuf(tmp, result_, 0, 0, ModNormalized);
				cache1[cnum] = tmp;
				cache2[cnum] = result_;
			}
		}
		// result_に関する長さチェックは不要
	}
	if (unaKApi_getOriginalHyoki(handle, targetStart,
		(morphBuffer + morphCurrent), &original_, &len_) < 0) {
#ifdef DEBUG
		ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
#endif
		ModThrow(ModModuleStandard, 
			ModCommonErrorUndefined, ModErrorLevelError);
	}
	if ( morphCurrent==0 && targetStart !=original_){
		len_ += (int)(original_ - targetStart);
		original_ = (unsigned short*)targetStart;
	}
	; ModAssert(len_ != 0);
	pos_ = (morphBuffer+morphCurrent)->hinshi;

	++morphCurrent;
	return ModTrue;
}

//
// FUNCTION
// ModUnaAnalyzer::getMorph -- 形態素解析結果の原表記付き取得
//
// NOTES
// 形態素解析結果を取得する。
// 原表記はセットされたテキストへのポインタとしてかえる
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果
// ModUnicodeString &ostr_
//    原表記
// const ModBoolean getNormalized_
//    異表記正規化の指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaAnalyzer::getMorph( ModUnicodeString& result_,
	ModUnicodeString& ostr_, const ModBoolean getNormalized_)
{
	if (getNormalized_ == ModTrue) {
		return getNormalized(result_, ostr_,ModTrue);
	}
	if (doSub(mode)){
		// 下位構造にばらす
		return getCheck(result_,ostr_,ModTrue);
	}
	// 下位構造にばらさない -- ModUnaMiddleAnalyzer の関数を呼び出す
	return get(result_, ostr_, ModFalse);
}

//
// Copyright (c) 2000-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
