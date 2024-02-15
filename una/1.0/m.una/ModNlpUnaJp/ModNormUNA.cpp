// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormUNA.cpp -- Unicode対応異表記正規化の規則参照定義
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

#include "ModUnicodeString.h"
#include "ModNlpUnaJp/ModNormUNA.h"
#include "ModNlpUnaJp/unakapi.h"

#include "UnaReinterpretCast.h"

_UNA_USING
_UNA_UNAJP_USING

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
// なし
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


// FUNCTION public
// ModNormUNA::ModNormUNA -- コンストラクタ
//
// NOTES
//
// ARGUMENTS
// const char* const dic
//		UNA用単語バイナリファイル
// const char* const app
//		UNA用アプリバイナリファイル
// const char* const connect
//		UNA用接続表バイナリファイル
// const char* const unknownTable
//		UNA用未登録語表バイナリファイル
// const char* const unknownCost
//		UNA用未登録語コストバイナリファイル
// const char* const normalTable
//		UNA用文字列標準化表バイナリファイル
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//		UNAの初期化に失敗した
//
ModNormUNA::ModNormUNA(const char* const dic,
					   const char* const app,
					   const char* const connect,
					   const char* const unknownTable,
					   const char* const unknownCost,
					   const char* const normalTable)
{
	// 係り受けテーブル、英語トークン、未登録語テーブルは使用しない
    unaHandle = new unaKApiHandleT;
	unaKApiDicImgT dicImg;
	dicImg.dicName  = (char *)"una";
	dicImg.morphDic = (char *)dic;
	dicImg.appInfo  = (char *)app;
	dicImg.dicPrio  = 1;
    if (unaKApi_init(unaHandle, &dicImg, 1, connect, 0, 0,
					 unknownTable, unknownCost, 0,0) < 0) {
		delete unaHandle;
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
    }
}

// FUNCTION public
// ModNormUNA::~ModNormUNA -- デストラクタ 
//
// NOTES
//
// ARGUMENTS
// It is not
//
// RETURN
// It is not
//
// EXCEPTIONS
// It is not
//
ModNormUNA::~ModNormUNA()
{
    unaKApi_term(unaHandle);
    delete unaHandle;
}

// FUNCTION public
// ModNormUNA::getAppInfo -- バイナリファイルに参照する。
//
// NOTES
// アプリ情報取得する。
//
// ARGUMENTS
// ModUnicodeString& in_str  参照対象文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorUndefined
//		UNA関数がエラーを返した
//
void
ModNormUNA::getAppInfo(const ModUnicodeString& inString,
					   ModUnicodeString& outString)
{
// 一度に形態素解析可な最大サイズ(文字数)
#ifdef UNA_LOCAL_TEXT_SIZE
#define LOCAL_TXT_SIZE UNA_LOCAL_TEXT_SIZE
#else
#define LOCAL_TXT_SIZE 256
#endif
	const ModUnicodeChar* target = inString;
	ModInt32 targetLength(inString.getLength()), processedLength(0), morphNum;
	unaMorphT morphBuffer[LOCAL_TXT_SIZE + 1];
	unaAppInfoT* appInfo;
	int res;

	outString.clear();

	while (targetLength > 0) {
		res = unaKApi_moAna(unaHandle, target, targetLength,
							morphBuffer, &morphNum, LOCAL_TXT_SIZE + 1,
							&processedLength, ctrl_c_stop, UNA_TRUE,UNA_TRUE,UNA_FALSE);
		if (res < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_moAna error: "
				<< unaKApi_getErrMsg(res) << ": " << inString << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		for (ModInt32 n(0); n < morphNum; ++n) {
			res = unaKApi_getAppInfo(unaHandle, &morphBuffer[n], &appInfo);

			if (res == UNA_KNOWN_WORD) {
				// アプリ情報がとれた
#ifdef DEBUG0
				{
					ModUnicodeString tmp(
						una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						(appInfo->len)/sizeof(ModUnicodeChar));
					ModDebugMessage << "APPINF:" << tmp << ModEndl;
				}
#endif
				if (appInfo->len > 0) {
					// アプリ情報の長さが１以上の場合のみアペンドする
					outString.append(
						una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						(appInfo->len)/sizeof(ModUnicodeChar));
				}
			} else if (res == UNA_UNKNOWN_WORD) {
				// アプリ情報がとれない -- 表記を取り出す
				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if (unaKApi_getHyoki(unaHandle, &morphBuffer[n],
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
#ifdef DEBUG0
				{
					ModUnicodeString tmp(hyouki, hyoukiLength);
					ModDebugMessage << "HYOUKI:" << tmp << ModEndl;
				}
#endif
				outString.append(hyouki, hyoukiLength);
			} else  {
				// 上記以外は動作がおかしい
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(res) << ": " << inString << ModEndl;
				; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
		}

		; ModAssert(targetLength >= processedLength);
		targetLength -= processedLength;
		target += processedLength;
	}
}

// FUNCTION public
// ModNormUNA::getAppInfo -- バイナリファイルに参照する。
//
// NOTES
// アプリ情報取得する。
//
// ARGUMENTS
// ModUnicodeString& in_str  参照対象文字列
// ModUnicodeString& outString		application information
// ModBoolean& regUnaWord			ModTrue : registered word in the dictionary
//									ModFalse : unregistered words in the dictionary
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorUndefined
//		UNA関数がエラーを返した
//
void
ModNormUNA::getAppInfo(const ModUnicodeString& inString,
					   ModUnicodeString& outString, ModBoolean& regUnaWord)
{
// 一度に形態素解析可な最大サイズ(文字数)
#ifdef UNA_LOCAL_TEXT_SIZE
#define LOCAL_TXT_SIZE UNA_LOCAL_TEXT_SIZE
#else
#define LOCAL_TXT_SIZE 256
#endif
	const ModUnicodeChar* target = inString;
	ModInt32 targetLength(inString.getLength()), processedLength(0), morphNum;
	unaMorphT morphBuffer[LOCAL_TXT_SIZE + 1];
	unaAppInfoT* appInfo;
	int res;

	outString.clear();

	while (targetLength > 0) {
		res = unaKApi_moAna(unaHandle, target, targetLength,
							morphBuffer, &morphNum, LOCAL_TXT_SIZE + 1,
							&processedLength, ctrl_c_stop, UNA_TRUE,UNA_TRUE,UNA_FALSE);
		if (res < 0) {
#ifdef DEBUG
			ModErrorMessage << "unaKApi_moAna error: "
				<< unaKApi_getErrMsg(res) << ": " << inString << ModEndl;
#endif
			ModThrow(ModModuleStandard,
					 ModCommonErrorUndefined, ModErrorLevelError);
		}
		for (ModInt32 n(0); n < morphNum; ++n) {
			res = unaKApi_getAppInfo(unaHandle, &morphBuffer[n], &appInfo);

			if (res == UNA_KNOWN_WORD) {
				// アプリ情報がとれた
#ifdef DEBUG0
				{
					ModUnicodeString tmp(
						una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						(appInfo->len)/sizeof(ModUnicodeChar));
					ModDebugMessage << "APPINF:" << tmp << ModEndl;
				}
#endif
				if (appInfo->len > 0) {
					// アプリ情報の長さが１以上の場合のみアペンドする
					outString.append(
						una_reinterpret_cast<ModUnicodeChar*>(appInfo->info),
						(appInfo->len)/sizeof(ModUnicodeChar));
				}
			} else if (res == UNA_UNKNOWN_WORD) {
				// アプリ情報がとれない -- 表記を取り出す
				regUnaWord = ModFalse;
				ModUnicodeChar* hyouki;
				ModSize hyoukiLength;
				if (unaKApi_getHyoki(unaHandle, &morphBuffer[n],
									 &hyouki, &hyoukiLength) < 0) {
#ifdef DEBUG
					ModErrorMessage << "unaKApi_getHyoki error" << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorUndefined, ModErrorLevelError);
				}
#ifdef DEBUG0
				{
					ModUnicodeString tmp(hyouki, hyoukiLength);
					ModDebugMessage << "HYOUKI:" << tmp << ModEndl;
				}
#endif
				outString.append(hyouki, hyoukiLength);
			} else  {
				// 上記以外は動作がおかしい
#ifdef DEBUG
				ModErrorMessage << "unaKApi_getAppInfo error: "
					<< unaKApi_getErrMsg(res) << ": " << inString << ModEndl;
				; ModAssert(res < 0 /*&& res == UNA_NO_APP_DIC*/);
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorUndefined, ModErrorLevelError);
			}
		}

		; ModAssert(targetLength >= processedLength);
		targetLength -= processedLength;
		target += processedLength;
	}
}

//
// Copyright (c) 2000-2009, 2023 RICOH Company, Ltd.
// All rights reserved.
//
