//
// unakapi.h -
//      UNA V3の形態素解析用API
//      コンパクト言語解析系(UNA)V3の、検索ライブラリでの使用を
//      前提とした、形態素解析及び係り受け解析用API。
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

#ifndef	UNAKAPI_H
#define	UNAKAPI_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "../UnaBase/UnaBase/una.h"			/* una グローバルなヘッダファイル */
#include "../UnaBase/UnaBase/unamorph.h"	/* 形態素解析メインモジュールのヘッダファイル */
#include "../UnaBase/UnaBase/unamdtri.h"	/* 形態素辞書引きモジュール用のヘッダファイル */
#include "../UnaBase/UnaBase/unamdunk.h"	/* 未登録語処理モジュール用のヘッダファイル */
#include "../UnaBase/UnaBase/unaapinf.h"	/* アプリ情報取得モジュール用のヘッダファイル */
#include "../UnaBase/UnaBase/unabns.h"		/* かかりうけ解析モジュール用のヘッダファイル */
#include "../UnaBase/UnaBase/unastd.h"		/* 文字変換モジュール用のヘッダファイル */
#include "unamdeng.h"						/* 英語トークン検出モジュール用のヘッダファイル */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* 形態素解析辞書数上限 */
#define	UNA_MORPH_DIC_MAX	16

/* 形態素辞書検索関数クロージャのテーブル数(形態素辞書,英語トークン,未登録語,null終端) */
#define UNA_DIC_FUNC_MAX	(UNA_MORPH_DIC_MAX + 3)

/* アプリ情報取得関数の型(キャスト用) */
typedef int (*unaAppFuncT)(const unaMorphT *morph,unaAppInfoT **appInf,
		void *arg);

//--------------------------------------------------------------------------
// TAG:	  unaKApiHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  API層のプログラムをマルチスレッドセーフで実行するためのハンドラ
//
typedef struct unaKApiHandleT{	// ハンドラ型
	unaMorphHandleT mh;			/* 形態素解析メインモジュール用ハンドル */
	unaMdTriHandleT th[UNA_MORPH_DIC_MAX];
								/* 形態素辞書引きモジュール用ハンドル配列 */
	unaMDEngHandleT eh;			/* 英語トークン検出モジュール用ハンドル */
	unaMdUnkHandleT uh;			/* 未登録語処理モジュール用ハンドル */
	unaApInfHandleT ah[UNA_MORPH_DIC_MAX];
								/* アプリ情報取得モジュール用ハンドル */
	unaBnsHandleT 	bh;			/* かかりうけ解析モジュール用ハンドル */
	unaStdHandleT 	sh;			/* 文字列標準化モジュール用ハンドル */
	int known_dic_count;		/* 形態素辞書数(thとknown_dic_numの使用中要素数 */
 	int known_dic_num[UNA_MORPH_DIC_MAX];
								/* 大語彙形態素辞書引きモジュールの辞書番号*/
 	int eng_dic_num;			/* 英語トークン検出モジュールの辞書番号*/
 	int unknown_dic_num;		/* 未登録語辞書引きモジュールの辞書番号*/
	unaFuncClT morFuncMtx[UNA_DIC_FUNC_MAX];
							/* 形態素辞書検索関数クロージャのテーブル */
	unaFuncClT appFuncMtx[UNA_DIC_FUNC_MAX];
							/* アプリ情報取得関数クロージャのテーブル */
	unaFuncClT subFuncMtx[UNA_DIC_FUNC_MAX];
							/* 下位構造取得関数クロージャのテーブル */

	unaCharT stdUnaText[UNA_LOCAL_TEXT_SIZE+1]; /* 標準化後の文字列 */
	int textIndex[UNA_LOCAL_TEXT_SIZE+1];		/* (標)文字位置→入力文字位置 */
}unaKApiHandleT;

//--------------------------------------------------------------------------
// TAG:	  unaKApiDicImgT
//
// ABSTRACT:	  辞書イメージ配列
//
// NOTE:
//	  形態素解析辞書イメージをもつ配列
//
typedef struct unaKApiDicImgT {
	char*  dicName;			// 辞書名(〜wrd/app2.dicの〜部分)
	char*  morphDic;		// 形態素辞書のイメージ
	char*  appInfo;			// アプリ情報のイメージ
	ucharT dicPrio;			// 辞書優先度(1〜255)
} unaKApiDicImgT;

/* 関数のモジュールごとの返り値の定義(UNAKAPI) */
#define UNA_FILE_NAME_IS_NULL -1001	/* ファイル名がヌルである */
#define UNA_TOO_LONG_FILENAME -1002	/* ファイル名が長い */
#define UNA_CANT_GET_FILEINFO -1003	/* ファイルの情報を得られない */
#define UNA_CANT_OPEN_FILE	  -1004	/* ファイルがオープンできない */
#define UNA_ERR_MALLOC		  -1005	/* malloc関数の失敗 */
#define UNA_ERR_DIC_REC_CNT	  -1006	/* 形態素辞書とアプリ辞書の件数が不一致*/
#define UNA_ERR_OVERFLOWAPBUF -1007	/* アプリ情報バッファオーバーフロー */
#define UNA_ERR_HINCONV	  	  -1008	/* 品詞番号がテーブルの範囲外 */
#define UNA_CANT_GET_TOKENTYP -1009	/* トークンタイプを得られない */
#define UNA_KNOWN_WORD		  1001	/* 登録語である */
#define UNA_ENG_TOKEN		  1002	/* 英語トークンである */
#define UNA_UNKNOWN_WORD	  1003	/* 未登録語である */

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {	// C関数としての宣言
#endif

/* 辞書のメモリへの読み込み */
int unaKApi_readFileImg(const void *fileName, char **mapAddress);

/* メモリの開放 */
int unaKApi_freeImg(const char *memAddress);

/* API for the search of UNA initialization */
int unaKApi_init(unaKApiHandleT *h,
		const unaKApiDicImgT *dicImgList,
		int dicImgCount,
		const char *cnctTblImg,
		const char *gramTblImg,
		const char *eMKTblImg,
		const char *uMKTblImg,
		const char *uCTblImg,
		const char *stdTblImg,
		unsigned int maxWordLen);

/* UNA検索用APIの終了 */
int unaKApi_term(unaKApiHandleT *h);

/* 形態素解析 */
int unaKApi_moAna(unaKApiHandleT *h,const unaCharT *inTxtPtr,
		const int inTxtLen,unaMorphT *morphBuf,int *morphNum,
		const int morphBufSize,int *processedTxtLen,unaStopFuncT stopFunc,const int execStd, const int ignoreCR, const int emulateBug);

/* かかりうけ解析 */
int unaKApi_kuAna(unaKApiHandleT *h, const unaCharT *inTxtPtr,
		const int inTxtLen, unaMorphT *morphBuf, int *morphNum,
		int morphBufSize, unaBnsT *bnsBuf, int *bnsNum, const int bnsBufSize,
		int *processedTxtLen, unaStopFuncT stopFunc, const int ignoreCR, const int emulateBug);

/* 表記の取得 */
int unaKApi_getHyoki(unaKApiHandleT *h, const unaMorphT *morph,
		unaCharT **hyokiPtr, unsigned int *hyokiLen);

/* 元表記の取得 */
int unaKApi_getOriginalHyoki(unaKApiHandleT *h,
		const unaCharT *txtPtr, const unaMorphT *morph,
		unaCharT **hyokiPtr, unsigned int *hyokiLen);

/* アプリケーション情報の取得 */
int unaKApi_getAppInfo(const unaKApiHandleT *h,
		const unaMorphT *morph,unaAppInfoT **appInf);

/* UNA品詞の取得 */
const unaCharT* unaKApi_getHinName(const unaKApiHandleT *h,const unaMorphT *morph);

/* UNA品詞の取得 */
int unaKApi_getUnaHin(const unaKApiHandleT *h,const unaMorphT *morph, unaHinT *unaHin);

/* トークン種別の取得 */
int unaKApi_getTokenType(const unaKApiHandleT *h, const unaMorphT *morph);

/* 辞書ベース名の取得 */
const char* unaKApi_getDicName(const unaKApiHandleT *h,const unaMorphT *morph);

/* 下位構造の取得 */
int unaKApi_getSubMorph(const unaKApiHandleT *h,
		const unaMorphT *morph,unaMorphT *morphBuf,int *morphNum,
		const int morphBufSize);

/* 下位構造の取得 */
const char *unaKApi_getErrMsg(const int errNo);

#ifdef __cplusplus
}			// C関数としての宣言終わり
#endif

#endif /* end of UNAKAPI_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
