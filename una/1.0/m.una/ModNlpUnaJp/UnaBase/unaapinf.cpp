//
// unaapinf.cpp -
//      アプリケーション辞書情報取得モジュール
//		形態素解析処理の中でアプリ情報を取得するためのモジュール
// 
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
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

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include <stdio.h>			/* NULL */
#include <string.h>			/* strcmp */
#include "UnaBase/unaapinf.h"		/* アプリ情報取得モジュール自身 */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------
#define MODULE_NAME "UNAAPINF"	/* モジュール名 */

/* モジュール内のメッセージ */
#define ERR_VERSION_APP			"Unsupported Appri Dictionary Version"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// MODULE:	  unaAppInfo_init
//
// ABSTRACT:    アプリ情報取得モジュールの初期化
//
// FUNCTION:
//	  アプリ情報取得モジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//    なし
//
int unaAppInfo_init(
		unaApInfHandleT *ah,	/* ハンドラ */
		const char *appInfoImg	/* アプリ辞書のイメージ */
)
{
	const char *imgPtr;			/* 辞書を格納したメモリへのポインタ */

	/* アプリ辞書の情報を設定する */
	ah->appInfoImg = appInfoImg;				/* アプリ辞書のイメージ */
	if (appInfoImg != (const char *)NULL){
		imgPtr = appInfoImg + UNA_COM_SIZE;
		if (strcmp(imgPtr,UNA_APP_VER)!=0) { /* バージョンが違う */
			UNA_RETURN(ERR_VERSION_APP,NULL);
		}
		imgPtr += sizeof(UNA_APP_VER);
		ah->recCount = *(uintT *)imgPtr;		/* アプリ情報の件数 */
		imgPtr += sizeof(uintT);
		ah->appInfoOffset	= (const uintT *)imgPtr; /* オフセット情報 */
		imgPtr += (ah->recCount) * sizeof(uintT);
		ah->appInfoPool		= imgPtr;			/* アプリ情報のプール */
	}
	else{
		ah->recCount = 0;
		ah->appInfoOffset	= (const uintT *)NULL;
		ah->appInfoPool		= (const char *)NULL;
	}
	
	/* 正常終了 */
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  unaAppInfo_term
//
// ABSTRACT:	  アプリ情報取得モジュールの終了処理
//
// FUNCTION:
//	  アプリ情報取得モジュールの終了処理を行う
//
// RETURN:
//	  UNA_OK	 正常終了
//
// NOTE:
//
int unaAppInfo_term(
		unaApInfHandleT *ah		/* ハンドラ */
)
{
	/* 値のリセット */
	ah->appInfoImg		= (const char *)NULL;
	ah->recCount		= 0;
	ah->appInfoOffset	= (const uintT *)NULL;
	ah->appInfoPool		= (const char *)NULL;
	
	/* 正常終了 */
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  unaAppInfo_get
//
// ABSTRACT:    アプリケーション情報を取得する
//
// FUNCTION:
//	  指定した形態素のアプリケーション情報をアプリケーション情報辞書より
//	  取得する。
//
// RETURN:
//	  UNA_OK			正常終了
//	  UNA_NO_APP_DIC	アプリケーション辞書の指定がない
//
// NOTE:
//	  なし
//
int unaAppInfo_get(
	const unaMorphT *morph,		/* アプリ情報取得対象の形態素 */
	unaAppInfoT **appInf,		/* アプリ情報 */
	const unaApInfHandleT *ah	/* ハンドラ */
)
{
	uintT	id;				 /* Trie の ID */
	const uintT *appInfoOffset;
							 /* アプリ情報のプールへのポインタ(オフセット) */
	const char *appInfoPool; /* アプリ情報のプール */

	appInfoOffset	= ah->appInfoOffset;
	appInfoPool		= ah->appInfoPool;

	if (ah->appInfoImg == (const char *)NULL) {
		/* アプリケーション辞書指定無し */
		return UNA_NO_APP_DIC;
	}

	/* ID を取り出す */
	id = morph->appI & 0x00ffffff;	/* appIの下位24ビットが ID */

	appInfoOffset += id;
	*appInf = (unaAppInfoT *)(appInfoPool + *appInfoOffset);

	/* 終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
