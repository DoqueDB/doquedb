// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModUNA.cpp -- UNA ラッパークラスの実装
// 
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
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

#include "UNA_UNIFY_TAG.h"
#include "ModMessage.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModParameter.h"
#include "ModException.h"
#include "LibUna/DicSet.h"
#include "ModNlpUnaJp/ModUnaMiddle.h"
#include "ModNlpUnaJp/unakapi.h"

#include <stdio.h>
_UNA_USING
_UNA_UNAJP_USING

#if 0
#include <stdio.h>
#endif

// CONST
// doSubChk -- 下位構造取得モードの判定配列
//
// NOTES
// 下位構造取得モードの判定配列
//
static const ModBoolean doSubChk[]={
	ModFalse,ModTrue,ModTrue,ModFalse,
	ModFalse,ModTrue,ModTrue,ModFalse};

// CONST
// doStemChk -- ステミングモードの判定配列
//
// NOTES
// ステミングモードの判定配列
//
static const ModBoolean doStemChk[]={
	ModFalse,ModTrue,ModFalse,ModTrue,
	ModFalse,ModTrue,ModFalse,ModTrue};

// CONST
// doCRChk -- 改行精密処理モードの判定配列
//
// NOTES
// 改行精密処理モードの判定配列
//
static const ModBoolean doCRChk[]={
	ModFalse,ModFalse,ModFalse,ModFalse,
	ModTrue,ModTrue,ModTrue,ModTrue};
	

// CONST
// ModUnaMiddleAnalyzer::unaLocalKeitaisoSize -- 形態素解析可能な最大サイズ
//
// NOTES
// UNA が一度に形態素解析可能な最大サイズ(文字数)
//
/*static*/ const ModSize
#ifdef UNA_LOCAL_TEXT_SIZE
ModUnaMiddleAnalyzer::unaLocalKeitaisoSize(UNA_LOCAL_TEXT_SIZE + 1);
#else
ModUnaMiddleAnalyzer::unaLocalKeitaisoSize(256);
#endif

// CONST
// ModUnaMiddleAnalyzer::unaLocalBunsetsuSize -- 係り受け解析可能な最大サイズ
//
// NOTES
// UNA が一度に係り受け解析可能な最大サイズ(文字数)
//
/*static*/ const ModSize
#ifdef UNA_LOCAL_BNS_SIZE
ModUnaMiddleAnalyzer::unaLocalBunsetsuSize(UNA_LOCAL_BNS_SIZE + 1);
#else
ModUnaMiddleAnalyzer::unaLocalBunsetsuSize(129);		// 128 + 1 にすべきか？
#endif

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
static int ctrl_c_stop()
{
  return 0;
}
};
#else
static int ctrl_c_stop()
{
  return 0;
}
#endif

//
// FUNCTION
// ModUnaMiddleAnalyzer::getMorph -- 形態素解析結果の取得
//
// NOTES
// 形態素解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString & data_
// 		解析結果
// const ModBoolean normalize_
// 		正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// Exception from subordinate position is returned that way
//
ModBoolean
ModUnaMiddleAnalyzer::getMorph(ModUnicodeString& data_,
	const ModBoolean normalize_)
{
	if (normalize_ == ModTrue) {
#ifdef DEBUG
		ModErrorMessage << "normalize mode is not supported" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotSupported, ModErrorLevelError);
	}
	return get(data_, ModFalse);
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::getMorph -- 形態素解析結果の取得
//
// NOTES
// 形態素解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString & data_
// 		解析結果
// ModUnicodeString & original_
// 		原表記
// const ModBoolean normalize_
// 		正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// Exception from subordinate position is returned that way
//
ModBoolean
ModUnaMiddleAnalyzer::getMorph(
	ModUnicodeString& data_,
	ModUnicodeString& original_,
	const ModBoolean normalize_)
{
	if (normalize_ == ModTrue) {
#ifdef DEBUG
		ModErrorMessage << "normalize mode is not supported" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotSupported, ModErrorLevelError);
	}
	return get(data_, original_, ModFalse);
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::ModUnaMiddleAnalyzer -- コンストラクタ
//
// NOTES
// 解析器のコンストラクタ
//
// ARGUMENTS
// ModUnaResource* const resource_
//    リソース
// const ModBoolean useNormTbl_
//    文字正規化表を使用するかの指示
// ModSize maxWordLen_
//    最大単語長の指示(0は指示なし)
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//		リソースが初期化されていない
//
ModUnaMiddleAnalyzer::ModUnaMiddleAnalyzer(
	const ModUnaResource* resource_,
	DicSet	*dicSet_,
	const ModBoolean useNormTbl_,
	const ModBoolean useJpn_,
	ModSize maxWordLen_)
	:
	resource(resource_),
	m_dicSet(dicSet_),
	target(0),
	targetLength(0),
	morphBuffer(0), morphNum(0), morphCurrent(0),
	subMorphBuffer(0), subMorphNum(0), subMorphCurrent(0),
	bunsetsuBuffer(0), bunsetsuNum(0), bunsetsuCurrent(0),
	maxWordLen(maxWordLen_),
	emulateMwOld(ModFalse)
{
	if (resource == 0) {
#ifdef DEBUG
		ModErrorMessage << "resource is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	ModInt32 ret;

	handle = new unaKApiHandleT;
	ret = unaKApi_init(handle,
				((useJpn_ == ModTrue) ? resource->dicList:0 ),
				((useJpn_ == ModTrue) ? resource->dicCount:0 ),
				resource->connectTbl,
				((useJpn_ == ModTrue) ? resource->kakariTbl:0 ),
				resource->egtokenTbl, resource->unknownTbl,
				resource->unknownCost,
				((useNormTbl_ == ModTrue)
					? resource->normalTbl:0 ),
				maxWordLen_);

	if ( ret < 0 ){
#ifdef DEBUG
		ModErrorMessage << "ModUnaMiddleAnalyzer: "
			<< unaKApi_getErrMsg(ret) << ModEndl;
#endif
		delete handle;
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
    }

	try {
		morphBuffer = new unaMorphT[unaLocalKeitaisoSize];
		subMorphBuffer = new unaMorphT[unaLocalKeitaisoSize];
		bunsetsuBuffer = new unaBnsT[unaLocalBunsetsuSize];
		dicNameVector.clear();

	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "ModUnaMiddleAnalyzer: " << e << ModEndl;
#endif
		// ハンドルも破棄する
		clear(ModTrue);
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::addDictionary -- 辞書追加
//
// NOTES
// 必要ならリソースに辞書を追加し、kapiのハンドラを再構築する
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//		リソースが初期化されていない
// ModCommonErrorUndefined
//		新しいリソースでのハンドラ作成ができない
//
void
ModUnaMiddleAnalyzer::addDictionary()
{
	if (resource == 0) {
#ifdef DEBUG
		ModErrorMessage << "resource is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
			 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	unaKApiHandleT bk = (*handle);
	try {
		morphNum = 0;
		morphCurrent = 0;
		subMorphNum = 0;
		subMorphCurrent = 0;
		bunsetsuNum = 0;
		bunsetsuCurrent = 0;
		ModInt32 ret;

		ret = unaKApi_init(handle, resource->dicList, resource->dicCount,
				 resource->connectTbl, resource->kakariTbl, resource->egtokenTbl,
				 resource->unknownTbl, resource->unknownCost,
				 resource->normalTbl ,maxWordLen);
		if ( ret < 0 ){
#ifdef DEBUG
			ModErrorMessage << unaKApi_getErrMsg(ret) << ModEndl;
#endif
			ModThrow(ModModuleStandard,
				ModCommonErrorUndefined, ModErrorLevelError);
		}
	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "ModUnaMiddleAnalyzer: " << e << ModEndl;
#endif
		// 元のハンドルに戻す
		(*handle) = bk;
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::delDictionary -- 辞書削除
//
// NOTES
// 日本語辞書を利用しないkapiのハンドラを再構築する
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//		リソースが初期化されていない
// ModCommonErrorUndefined
//		新しいリソースでのハンドラ作成ができない
//
void
ModUnaMiddleAnalyzer::delDictionary()
{
	if (resource == 0) {
#ifdef DEBUG
		ModErrorMessage << "resource is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
			 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	unaKApiHandleT bk = (*handle);
	try {
		morphNum = 0;
		morphCurrent = 0;
		subMorphNum = 0;
		subMorphCurrent = 0;
		bunsetsuNum = 0;
		bunsetsuCurrent = 0;
		ModInt32 ret;
		ret = unaKApi_init(handle, 0, 0, resource->connectTbl,
				 resource->kakariTbl, resource->egtokenTbl,
				 resource->unknownTbl, resource->unknownCost,
				 resource->normalTbl ,maxWordLen);
		if ( ret < 0 ){
#ifdef DEBUG
			ModErrorMessage << unaKApi_getErrMsg(ret) << ModEndl;
#endif
			ModThrow(ModModuleStandard,
				ModCommonErrorNotInitialized, ModErrorLevelError);
		}
	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "ModUnaMiddleAnalyzer: " << e << ModEndl;
#endif
		// 元のハンドルに戻す
		(*handle) = bk;
		ModRethrow(e);
	}
}

//
// FUNCTIOデストラクタ
//
// NOTES
// 解析器のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//
ModUnaMiddleAnalyzer::~ModUnaMiddleAnalyzer()
{
	// ハンドルも破棄する
	clear(ModTrue);
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::clear -- クリア
//
// NOTES
// クリアし、初期状態にする。
//
// ARGUMENTS
// const ModBoolean clearHandle_
//    ハンドルのクリア指示
//
// RETURN
// なし
//
// EXCEPTIONS
//
void
ModUnaMiddleAnalyzer::clear(const ModBoolean clearHandle_)
{
	if (clearHandle_ == ModTrue && handle != 0) {
		unaKApi_term(handle);
		delete handle;
		handle = 0;
	}

	delete [] morphBuffer;
	morphBuffer = 0;
	morphNum = morphCurrent = 0;

	delete [] subMorphBuffer;
	subMorphBuffer = 0;
	subMorphNum = subMorphCurrent = 0;

	delete [] bunsetsuBuffer;
	bunsetsuBuffer = 0;
	bunsetsuNum = bunsetsuCurrent = 0;

	dicNameVector.clear();
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::set -- 解析対象テキストの設定
//
// NOTES
// 解析対象テキストを設定する
//
// ARGUMENTS
// const ModUnicodeString& target_
//    解析対象テキスト
// const ModSize mode_
//    解析モード
//
// RETURN
// なし
//
// EXCEPTIONS
//
void
ModUnaMiddleAnalyzer::set(const ModUnicodeString& target_, const ModSize mode_)
{
	target = target_;
	targetLength = target_.getLength();
	mode = mode_;
	morphNum = morphCurrent = 0;
	subMorphNum = subMorphCurrent = 0;
	bunsetsuNum = bunsetsuCurrent = 0;

	// targetが 0 から始まる場合の特殊処理
	ModSize pLen=0;
	while ( (ModSize)targetLength > pLen && target[pLen]==0) pLen++;
	targetLength -= pLen;
	target +=pLen;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::get -- 解析結果の取得
//
// NOTES
// 解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& data_
//    解析結果
// ModUnicodeString& original_
//    元表記
// const ModBoolean execStdMode_
//    前処理を実施するか否かの指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::get( ModUnicodeString& data_,
	ModUnicodeString& original_, const ModBoolean execStdMode_)
{
	if (morphCurrent == morphNum) {
		morphCurrent = 0;
		morphNum = 0;
		analyze(execStdMode_);
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

	data_.clear();
	data_.append(hyouki, hyoukiLength);
#if 0
#ifdef UNA_DEBUG
	ModDebugMessage << data_ << ' ' << int((morphBuffer + morphCurrent)->hinshi)
			   << ModEndl;
#endif
#endif
	if (unaKApi_getOriginalHyoki(handle, targetStart,
		morphBuffer + morphCurrent, &hyouki, &hyoukiLength) < 0) {
		ModErrorMessage << "unaKApi_getOriginalHyoki error" << ModEndl;
	}

	original_.clear();
	original_.append(hyouki, hyoukiLength);
#if 0
#ifdef UNA_DEBUG
	ModDebugMessage << original_ << ' ' << ModEndl;
#endif
#endif

	++morphCurrent;

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::get -- 解析結果の取得
//
// NOTES
// 解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& data_
//    解析結果
// const ModBoolean execstdMode__
//    前処理を実施するか否かの指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::get(ModUnicodeString& data_,const ModBoolean execStdMode_)
{
	if (morphCurrent == morphNum) {
		morphCurrent = 0;
		morphNum = 0;
		analyze(execStdMode_);
		if (targetLength == 0 && morphNum == 0) {
			return ModFalse;
		}
	}

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
						 &hyouki, &hyoukiLength) < 0) {
		ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
	}

	data_.clear();
	data_.append(hyouki, hyoukiLength);
#if 0
#ifdef UNA_DEBUG
	ModDebugMessage << data_ << ' ' << int((morphBuffer + morphCurrent)->hinshi)
			   << ModEndl;
#endif
#endif

	++morphCurrent;

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// かかり受け解析したブロックごとの解析結果を取得する
//
// ARGUMENTS
// ModUnicodeString& data_
//		解析結果のアプリケーション情報
// const ModUnicodeChar sep1_
//		セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//		セパレータ２（レコード区切り）
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::getBlock(ModUnicodeString& data_,
	 const ModUnicodeChar sep1_, const ModUnicodeChar sep2_)
{
	data_.clear();

	if (targetLength == 0) {
		// 解析対象が残っていない
		return ModFalse;
	}

	analyze(ModTrue,ModTrue);

	ModCharString ctmp, cdata;
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

		// 形態素番号（何番目の形態素か; 1 origin）
		ctmp.format("%d", morphCurrent + 1);
		{
			ModUnicodeString utmp(ctmp);
			data_.append(utmp);
		}
		data_.append(sep1_);

		// 形態素の表記
		if (unaKApi_getHyoki(handle,morphBuffer + morphCurrent,
							 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		data_.append(hyouki, hyoukiLength);
		data_.append(sep1_);

		// 形態素の品詞
		{
			ModUnicodeString
				utmp(unaKApi_getHinName(handle, (morphBuffer+morphCurrent)),0);
			data_.append(utmp);
		}
		data_.append(sep1_);

		// 文節番号（何番目の文節か; 1 origin）
		ctmp.format("%d", (b) ? 0 : bunsetsuCurrent);
		{
			ModUnicodeString utmp(ctmp);
			data_.append(utmp);
		}
		data_.append(sep1_);

		// 係り先の文節番号（何番目の文節か; 1 origin）
		ctmp.format("%d",
					(b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].target + 1);
		{
			ModUnicodeString utmp(ctmp);
			data_.append(utmp);
		}
		data_.append(sep1_);

		// 係り受け関係の番号
		ctmp.format("%d",
					(b) ? 0 : bunsetsuBuffer[bunsetsuCurrent - 1].kuRel + 1);
		{
			ModUnicodeString utmp(ctmp);
			data_.append(utmp);
		}
		data_.append(sep2_);
	}

	// 形態素の辞書ベース名をdicNameVectorに保存
	dicNameVector.clear();
	dicNameVector.reserve(morphNum);
	for (int n = 0; n < morphNum; n++) {
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + n);
		ModUnicodeString utmp(dicName? dicName: "");
		dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 形態素解析したブロックごとの解析結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    異表記正規化済みの表記ベクター
// ModVector<int>& posVector_
//    品詞ベクター
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::getBlock( ModVector<ModUnicodeString>& formVector_,
	ModVector<int>& posVector_)
{
	if (targetLength == 0) {
		// 解析対象が残っていない
		return ModFalse;
	}
	analyze(ModTrue);

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);

	// 形態素数だけループする
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
		ModUnicodeString form(hyouki,hyoukiLength);
		formVector_.pushBack(form);
		posVector_.pushBack((morphBuffer+morphCurrent)->hinshi);
	}

	// 形態素の辞書ベース名をdicNameVectorに保存
	dicNameVector.clear();
	dicNameVector.reserve(morphNum);
	for (int n = 0; n < morphNum; n++) {
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + n);
		ModUnicodeString utmp(dicName? dicName: "");
		dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 形態素解析したブロックごとの解析結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    異表記正規化済みの表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    原表記ベクター
// ModVector<int>& posVector_
//    品詞ベクター
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::getBlock( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	ModVector<int> *costVector_ = 0,
	ModVector<int> *uposVector_ = 0)
{
	if (targetLength == 0) {
		// 解析対象が残っていない
		return ModFalse;
	}
	analyze(ModTrue);

	ModUnicodeChar* hyouki;
	ModSize hyoukiLength;
	ModUnicodeChar* ohyouki;
	ModSize ohyoukiLength;

	formVector_.clear();
	formVector_.reserve(morphNum);
	posVector_.clear();
	posVector_.reserve(morphNum);
	ostrVector_.clear();
	ostrVector_.reserve(morphNum);
	if(costVector_){
		costVector_->clear();				/* temp */
		costVector_->reserve(morphNum);		/* temp */
	}
	if(uposVector_){
		uposVector_->clear();
		uposVector_->reserve(morphNum);		/* temp */
	}

	// 形態素数だけループする
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

		// 形態素の表記
		if (unaKApi_getOriginalHyoki(handle, targetStart,
							morphBuffer + morphCurrent,
							 &ohyouki, &ohyoukiLength) < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_getOrigialHyoki error" << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		ModUnicodeString form(hyouki,hyoukiLength);
		formVector_.pushBack(form);
		ModUnicodeString ostr(ohyouki,ohyoukiLength);
		ostrVector_.pushBack(ostr);
		int hinshi;
		posVector_.pushBack(hinshi = (morphBuffer+morphCurrent)->hinshi);
		if(costVector_)
		{
			costVector_->pushBack((morphBuffer+morphCurrent)->cost);
#if 0
			printf("ModUnaMiddleAnalyzer::getBlock cost=%d\n",(morphBuffer+morphCurrent)->cost);
#endif
		}

		if(uposVector_)
			uposVector_->pushBack(m_dicSet->getTypeCode(hinshi));
	}

	// 形態素の辞書ベース名をdicNameVectorに保存
	dicNameVector.clear();
	dicNameVector.reserve(morphNum);
	for (int n = 0; n < morphNum; n++) {
		const char* dicName = unaKApi_getDicName(handle, morphBuffer + n);
		ModUnicodeString utmp(dicName? dicName: "");
		dicNameVector.pushBack(utmp);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::getDicName -- 解析結果の辞書ベース名取得
//
// NOTES
// 形態素解析したブロックごとの辞書ベース名を取得する
// 形態素と辞書ベース名の個数および位置は一対一対応している
//
// ARGUMENTS
// ModVector<ModUnicodeString>& dicNameVector_
//    辞書ベース名のベクター(辞書ベース名がないときは空文字列)
//
// RETURN
// 解析結果があれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::getDicName(ModVector<ModUnicodeString>& dicNameVector_)
{
	if (dicNameVector.isEmpty()) {
		// 解析結果がない
		return ModFalse;
	}

	dicNameVector_.clear();
	dicNameVector_.reserve(dicNameVector.getSize());

	// 最後のgetBlock()で記憶した辞書ベース名をコピー
	ModVector<ModUnicodeString>::Iterator name;
	for (name = dicNameVector.begin(); name != dicNameVector.end(); name++) {
		dicNameVector_.pushBack(*name);
	}

	return ModTrue;
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::analyze -- 解析
//
// NOTES
// UNAを用いて解析する。解析結果は morphBuffer にためられ、get() で取得する。
//
// ARGUMENTS
// const ModBoolean execStdMode_
//    前処理実施の指示
// const ModBoolean execKuAna_
//    かかりうけ解析の指示
//
// RETURN
// なし
//
// EXCEPTIONS
//
void
ModUnaMiddleAnalyzer::analyze(const ModBoolean execStdMode_,
	const ModBoolean execKuAna_)
{
	ModInt32 res;
	ModInt32 processedLength;
	int emulateMaxWordBug = UNA_FALSE;
	if (emulateMwOld == ModTrue){
		emulateMaxWordBug = UNA_TRUE;
	}

	int ignoreCR = UNA_FALSE;
	if (doCR(mode)){
		ignoreCR = UNA_TRUE;
	}

	int execStd = UNA_FALSE;
	if (execStdMode_ == ModTrue){
		execStd = UNA_TRUE;
	}
	if (targetLength > 0) {
		if ( execKuAna_ == ModFalse){
			res = unaKApi_moAna(handle, target, targetLength,
				morphBuffer, &morphNum, unaLocalKeitaisoSize,
				&processedLength, ctrl_c_stop,execStd, emulateMaxWordBug, ignoreCR);
		}
		else{
			res = unaKApi_kuAna(handle, target, targetLength,
				morphBuffer, &morphNum, unaLocalKeitaisoSize,
				bunsetsuBuffer, &bunsetsuNum, unaLocalBunsetsuSize,
				&processedLength, ctrl_c_stop, emulateMaxWordBug, ignoreCR);
		}
		if (res != UNA_OK) {
#ifdef DEBUG
			ModErrorMessage << "ModUnaMiddle::analyze error: "
				<< unaKApi_getErrMsg(res) << ": " << target << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}

		; ModAssert(targetLength >= processedLength);
		// 文字列中に 0 が入っている場合の特殊処理
		while (targetLength > processedLength && target[processedLength]==0){
			processedLength++;
		}
		targetLength -= processedLength;
		targetStart = target;
		target += processedLength;
	}
}

//
// FUNCTION
// ModUnaResource::ModUnaResource -- コンストラクタ
//
// NOTES
// 解析用リソースのコンストラクタ。
// リソースディレクトリへのパスを指定する
//
// ARGUMENTS
// const ModUnicodeString& path_
//    リソースディレクトリへのパス
//
// RETURN
// なし
//
// EXCEPTIONS
//
ModUnaResource::ModUnaResource(const ModUnicodeString& path_,
							   ModBoolean memSwitch_)
	: connectTbl(0), kakariTbl(0),
	  egtokenTbl(0), unknownTbl(0), unknownCost(0), normalTbl(0)
{
	ModUnicodeString listPath(path_),
					 conPath(path_), kkrPath(path_), egtPath(path_),
					 untPath(path_), uncPath(path_), nrmPath(path_);
	ModVector<ModUnicodeString> baseVector, wrdPathVector, app2PathVector;
	ModVector<int> dicPrioVector;
	baseVector.clear();
	wrdPathVector.clear();
	app2PathVector.clear();
	dicPrioVector.clear();

	listPath += "diclist.dat";
	conPath += "connect.tbl";
	kkrPath += "gram.tbl";
	egtPath += "engmk.tbl";
	untPath += "unkmk.tbl";
	uncPath += "unkcost.tbl";
	nrmPath += "unastd.tbl";

	// diclist.datから優先度と辞書名ベースを読み、辞書リストを組み立てる
	char buf[BUFSIZ];
	FILE* fp = fopen(listPath.getString(), "r");
	if (fp == NULL) {
		// diclist.datがなければunawrd/app2.dicの単一辞書構成を想定
		baseVector.pushBack("una");
		wrdPathVector.pushBack(path_ + "unawrd.dic");
		app2PathVector.pushBack(path_ + "unaapp2.dic");
		dicPrioVector.pushBack(1);
	} else {
		int lineno = 0;
		int count = 0;
		int prevPrio = 1;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			lineno++;

			// 行頭のBOMや空白を読み飛ばす
			char* p = buf;
			if (lineno == 1 && memcmp(p, "\xef\xbb\xbf", 3) == 0) {
				p += 3;
			}
			while (*p && (*p == ' ' || *p == '\t')) {
				p++;
			}

			// 注釈行や空行はスキップ
			if (*p == '\0' || *p == '#' || *p == '\r' || *p == '\n') {
				continue;
			}

			// 優先度と辞書ファイルベース名を読む
			int n, prio;
			char base[BUFSIZ];
			n = sscanf(p, "%d , %[^# \t\r\n]", &prio, base);
			if (n != 2) {
				// 構文エラー
				fclose(fp);
				ModErrorMessage << "ModUnaResource: diclist.dat: syntax error" << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
			}
			if (prio < 1 || prio > 255) {
				// 優先度が1〜255の範囲にない
				fclose(fp);
				ModErrorMessage << "ModUnaResource: diclist.dat: priority out of range" << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorOutOfRange, ModErrorLevelError);
			}
			if (prio < prevPrio) {
				// 優先度が逆順
				fclose(fp);
				ModErrorMessage << "ModUnaResource: diclist.dat: priority out of order" << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
			}
			if (++count > UNA_MORPH_DIC_MAX) {
				// 辞書数がUNA_MORPH_DIC_MAXを超えた
				fclose(fp);
				ModErrorMessage << "ModUnaResource: diclist.dat: too many dictionaries" << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorOutOfRange, ModErrorLevelError);
			}
			baseVector.pushBack(base);
			wrdPathVector.pushBack(path_ + base + "wrd.dic");
			app2PathVector.pushBack(path_ + base + "app2.dic");
			dicPrioVector.pushBack(prio);
			prevPrio = prio;
		}
		fclose(fp);
		if (count == 0) {
			// 辞書がひとつもない
			ModErrorMessage << "ModUnaResource: diclist.dat: no dictionaries found" << ModEndl;
			ModThrow(ModModuleStandard, ModCommonErrorUndefined, ModErrorLevelError);
		}
	}

	// switch for reducing memory
	if ( memSwitch_ == ModTrue){
		kkrPath.clear();
	}

	load(baseVector, wrdPathVector, app2PathVector, dicPrioVector,
			conPath, kkrPath, egtPath, untPath, uncPath, nrmPath);
}

//
// FUNCTION
// ModUnaResource::~ModUnaResource -- デストラクタ
//
// NOTES
// 解析用リソースのデストラクタ。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
ModUnaResource::~ModUnaResource()
{
	free();
}

//
// FUNCTION
// ModUnaResource::load -- ロード
//
// NOTES
// 解析用リソースをロードする。
// パスが空文字列のリソースはロードしない（この場合は例外にならない）。
//
// ARGUMENTS
// const ModVector<ModUnicodeString>& baseVector_
//    辞書ベース名のベクター
// const ModVector<ModUnicodeString>& wrdPathVector_
//    形態素辞書パスのベクター
// const ModVector<ModUnicodeString>& app2PathVector_
//    アプリ情報辞書パスのベクター
// const ModVector<ModUnicodeString>& dicPrioVector_
//    辞書優先度のベクター
// const ModUnicodeString& conPath_
//    接続表へのパス
// const ModUnicodeString& kkrPath_
//    かかり受け表へのパス
// const ModUnicodeString& egtPath_
//    英語トークン情報へのパス
// const ModUnicodeString& untPath_
//    未登録語表へのパス
// const ModUnicodeString& uncPath_
//    未登録語コストへのパス
// const ModUnicodeString& nrmPath_
//    文字列標準化表へのパス
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		不正なパス名
//
void
ModUnaResource::load(ModVector<ModUnicodeString>& baseVector_,
					 ModVector<ModUnicodeString>& wrdPathVector_,
					 ModVector<ModUnicodeString>& app2PathVector_,
					 ModVector<int>& dicPrioVector_,
					 ModUnicodeString& cntPath_,
					 ModUnicodeString& kkrPath_,
					 ModUnicodeString& egtPath_,
					 ModUnicodeString& untPath_,
					 ModUnicodeString& uncPath_,
					 ModUnicodeString& nrmPath_)
{
	int ret;
	char* morphDic;
	char* appInfo;

#ifdef UNA_DEBUG
	dicCount = baseVector_.getSize();
	fprintf(stdout, "ModUnaResource::load\n");
	for (int i = 0; i < dicCount; i++) {
		fprintf(stdout, "  [%d] prio=%d, name=%s, wrd=%s, app2=%s\n",
				i, dicPrioVector_[i],
				baseVector_[i].getString(),
				wrdPathVector_[i].getString(),
				app2PathVector_[i].getString());
	}
	fflush(stdout);
#endif

	// 辞書イメージ表の配列を確保する
	dicCount = baseVector_.getSize();
	dicList = new unaKApiDicImgT[dicCount];
	memset((char *)dicList, 0x00, sizeof(unaKApiDicImgT) * dicCount);

	// 複数辞書対応に伴い、V1.6ではアプリ辞書なし(パスが空)の動作には対応しない
	optionStatus = 1; // オプション辞書は常にありとする
	try {
		for (int n = 0; n < dicCount; n++) {
			// 辞書ベース名
			dicList[n].dicName = baseVector_[n].getNewString();

			// 形態素辞書イメージ
			ModUnicodeString& a = wrdPathVector_[n];
			ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()), &morphDic);
			if (ret <0){
#ifdef DEBUG
				ModErrorMessage << "wrdPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << wrdPathVector_[n] << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			dicList[n].morphDic = morphDic;

			// アプリ情報イメージ
			a = app2PathVector_[n];
			ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()), &appInfo);
			if (ret <0){
#ifdef DEBUG
				ModErrorMessage << "app2Path invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << app2PathVector_[n] << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			dicList[n].appInfo = appInfo;

			// 辞書優先度
			dicList[n].dicPrio = dicPrioVector_[n];
		}

		if (cntPath_.getLength() != 0) {
			ModUnicodeString& a = cntPath_;
			ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &connectTbl);
			if (ret <0){
#ifdef DEBUG
				ModErrorMessage << "cntPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << cntPath_ << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
		}
		if (kkrPath_.getLength() != 0) {
			ModUnicodeString& a = kkrPath_;
			ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &kakariTbl);
			// かかりうけ情報は読み込めなくても可
		}
		if (egtPath_.getLength() != 0) {
			ModUnicodeString& a = egtPath_;
			ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &egtokenTbl);
			if (ret < 0){
#ifdef DEBUG
				ModErrorMessage << "egtPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << egtPath_ << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
		}
		if (untPath_.getLength() != 0) {
			ModUnicodeString& a = untPath_;
			ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &unknownTbl);
			if (ret < 0){
#ifdef DEBUG
				ModErrorMessage << "untPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << untPath_ << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
		}
		if (uncPath_.getLength() != 0) {
			ModUnicodeString& a = uncPath_;
			ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &unknownCost);
			if (ret < 0){
#ifdef DEBUG
				ModErrorMessage << "uncPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << uncPath_ << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
		}
		if (nrmPath_.getLength() != 0) {
			ModParameter param;
			ModBoolean skip(ModFalse);
			if (param.getBoolean(skip, "UnaSkipCharNormalize") == ModFalse
				|| skip == ModFalse) {
				ModUnicodeString& a = nrmPath_;
				ret = unaKApi_readFileImg( a.getString(ModOs::Process::getEncodingType()), &normalTbl);
				if ( ret < 0){
#ifdef DEBUG
					ModErrorMessage << "nrmPath invalid: "
						<< unaKApi_getErrMsg(ret) << ": " << nrmPath_ << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorBadArgument, ModErrorLevelError);
				}
			} else {
				ModDebugMessage << "nrmTbl load skipped" << ModEndl;
			}
		}

	} catch (ModException& e) {
		free();
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModUnaResource::free -- フリー
//
// NOTES
// 解析用リソースをフリーする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//
void
ModUnaResource::free()
{
	for (int n = 0; n < dicCount; n++) {
		if (dicList[n].dicName != 0) {
			ModUnicodeString::freeNewString(dicList[n].dicName);
		}
		if (dicList[n].morphDic != 0) {
			unaKApi_freeImg(dicList[n].morphDic);
		}
		if (dicList[n].appInfo != 0) {
			unaKApi_freeImg(dicList[n].appInfo);
		}
	}
	delete [] dicList;
	dicList = 0;
	if (connectTbl != 0) {
		unaKApi_freeImg(connectTbl);
		connectTbl = 0;
	}
	if (kakariTbl != 0) {
		unaKApi_freeImg(kakariTbl);
		kakariTbl = 0;
	}
	if (egtokenTbl != 0) {
		unaKApi_freeImg(egtokenTbl);
		egtokenTbl = 0;
	}
	if (unknownTbl != 0) {
		unaKApi_freeImg(unknownTbl);
		unknownTbl = 0;
	}
	if (unknownCost != 0) {
		unaKApi_freeImg(unknownCost);
		unknownCost = 0;
	}
	if (normalTbl != 0) {
		unaKApi_freeImg(normalTbl);
		normalTbl = 0;
	}
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::doStem -- ステミングモード判定
//
// NOTES
// 与えられたモードがステミングを実施するモードか否かを返す
//
// ARGUMENTS
// const ModSize mode_
//
// RETURN
// ステミングするモードならModTrue、そうでなければModFalse 
//
// EXCEPTIONS
//
const ModBoolean
ModUnaMiddleAnalyzer::doStem(const ModSize mode_)
{
	if( mode_ < 0 || mode_ > 7) return ModFalse;
	return doStemChk[mode_];
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::doSub -- 下位構造取得モード判定
//
// NOTES
// 与えられたモードが下位構造取得を実施するモードか否かを返す
//
// ARGUMENTS
// const ModSize mode_
//
// RETURN
// 下位構造取得するモードならModTrue、そうでなければModFalse 
//
// EXCEPTIONS
//
const ModBoolean
ModUnaMiddleAnalyzer::doSub(const ModSize mode_)
{
	if( mode_ < 0 || mode_ > 7) return ModFalse;
	return doSubChk[mode_];
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::doCR -- 改行精密処理モード判定
//
// NOTES
// 与えられたモードが改行をまたがる単語の検出を実施するモードか否かを返す
//
// ARGUMENTS
// const ModSize mode_
//
// RETURN
// 改行精密処理モードならModTrue、そうでなければModFalse 
//
// EXCEPTIONS
//
const ModBoolean
ModUnaMiddleAnalyzer::doCR(const ModSize mode_)
{
	if( mode_ < 0 || mode_ > 7) return ModFalse;
	return doCRChk[mode_];
}

//
// FUNCTION
// ModUnaMiddleAnalyzer::checkNextChar -- check next charactor
//
// NOTES
// This function will return first charactor of 
// the text buffer which is not analyzed.
// Statuses such as morphBuffer isn't change with this function
//
// ARGUMENTS
//
// RETURN
// The first charctor of the remained text
//
// EXCEPTIONS
//
const ModUnicodeChar 
ModUnaMiddleAnalyzer::checkNextChar()
{
	// check submorph buffer and return first charactr of first sub morpho
	if ( doSub(mode) && subMorphNum > 0 && subMorphCurrent<subMorphNum){
		return subMorphBuffer[subMorphCurrent].start[0];
	}
	
	// no morpho in morphBuffer
	if (morphCurrent == morphNum) {
		if (targetLength > 0) {
			return target[0];
		}
		else {
			return 0;
		}
	}

	// return first charctor of first morpho in morphBuffer
	return morphBuffer[morphCurrent].start[0];
}

void
ModUnaMiddleAnalyzer::emulateMaxWordOld(
	const ModBoolean s)
{
	emulateMwOld = s;
}

// FUNCTION
// ModUnaMiddleAnalyzer::getRemainLength -- get the length of target not processed yet
//
// NOTES
//	 get the length of target not processed yet
//
// ARGUMENTS
//	 ModSize* pulRemainLength_
//		the length of target not processed yet
//
// RETURN
//	 ModTrue:  some morphs got at previous analyze() are remained.
//	 ModFalse: all morphs got at previous analyze() have been dealed.
//
// EXCEPTIONS
//
ModBoolean
ModUnaMiddleAnalyzer::getRemainLength(ModSize* pulRemainLength_)
{
	*pulRemainLength_ = targetLength;
	return morphCurrent != morphNum ? ModTrue : ModFalse;
}

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
