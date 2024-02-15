//
// una.h -
//      UNA V3のグローバル定義集
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

#ifndef	UNA_H
#define	UNA_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------
#include <stdio.h>		/* FILE */
#include <limits.h>		/* UCHAR_MAX */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* Version of the UNA dictionaries (being SPACE including largest ASCII 15 letter) */
#define UNA_DIC_VER	"WRD V1.25V-    "	/* UNAWRD.DIC V1.25 無効語あり */
#define UNA_DIC_VER_125	"WRD V1.25-     "	/* UNAWRD.DIC V1.25 無効語なし */
#define UNA_DIC_VER_124	"WRD V1.24-     "	/* UNAWRD.DIC V1.24 */
#define UNA_DIC_VER_123	"WRD APP V1.23- "	/* UNAWRD.DIC UNAAPP.DIC */
#define UNA_APP_VER	"WRD APP V1.23- "	/* UNAAPP.DIC */
#define UNA_CON_VER	"CON V1.16-     "	/* CONNECT.TBL           */
#define UNA_GRM_VER	"GRM V1.11-     "	/* GRAM.TBL              */
#define UNA_EMK_VER	"EMK V1.08-     "	/* ENGMK.TBL             */
#define UNA_UMK_VER	"UMK V1.01-     "	/* UNKMK.TBL             */
#define UNA_UC_VER	"UC V1.02-      "	/* UNKCOST.TBL           */
#define UNA_STD_VER	"STD V1.01-     "	/* UNASTD.TBL            */
#define UNA_CON_VER_115	"CON V1.15-     "	/* CONNECT.TBL           */
#define UNA_EMK_VER_107	"EMK V1.07-     "	/* ENGMK.TBL             */

/* 辞書類のコメント、バージョンのサイズ */
#define UNA_COM_SIZE	48	/* byte */
#define UNA_VER_SIZE	16	/* byte */

/* バッファサイズなどの定義 */
#ifndef UNA_LOCAL_TEXT_SIZE
  #define UNA_LOCAL_TEXT_SIZE 255 /*一度に形態素解析できる最大サイズ(文字) */
#endif

/* UNAで扱える表記の最大長(文字) */
#define UNA_HYOKI_LEN_MAX UCHAR_MAX	/* ucharTで表わせる最大数=255文字 */

/* 辞書語下位構造形態素の最大個数 */
#define UNA_SUB_MORPH_CNT_MAX	100

/* UNAで扱うファイル名の最大長 */
#define UNA_FNAME_MAX			200	  /* ターミネータ含まず */

/* エラーメッセージ表示 */#if defined(UNA_DEBUG)
  #define UNA_RETURN(msg,para) una_errMsg(0,MODULE_NAME,UNA_##msg,msg,para);\
								return(UNA_##msg)
#else
  #define UNA_RETURN(msg,para) return(UNA_##msg)
#endif

/* 語境界 */
#ifndef UNA_ALIGNMENT
#define UNA_ALIGNMENT 1	/* Intel */
#endif

/* フラグ、スイッチの値の定義 */
#define UNA_TRUE				1	   /* 真 */
#define UNA_FALSE				0	   /* 偽 */

/* 関数の標準の返り値の定義 */
#define UNA_OK					1	   /* 正常終了 */
#define UNA_STOP				-1	   /* 中断 */

/* 複数のモジュールにより使用される返り値 */
#define UNA_SYUSOKU				-101 /* ラティスが収束した */

/* 関数のモジュールごとの返り値の定義(UNAMORPH) */
#define UNA_ERR_VERSION_CON		-11001 /* 接続表バージョンエラー */
#define UNA_ERR_BRNCH_SIZE		-11002 /* 形態素枝バッファのオーバーフロー */
#define UNA_ERR_PATH	  		-11003 /* 形態素バッファのオーバーフロー */

/* 関数のモジュールごとの返り値の定義(UNAMDTRI) */
#define UNA_SUB_MORPH_BUF_OVER	-12001 /* 下位構造バッファオーバーフロー */
#define UNA_ERR_VERSION_MORPH	-12002 /* 形態素辞書バージョンエラー */

/* 関数のモジュールごとの返り値の定義(UNAMDUNK) */
#define UNA_ERR_VERSION_UMK		-13001 /* 未登録語文字種別テーブル
													バージョンエラー */
#define UNA_ERR_VERSION_UC		-13002 /* 未登録語コスト推定テーブル
													バージョンエラー */

/* 関数のモジュールごとの返り値の定義(UNAAPINF) */
#define UNA_NO_APP_DIC			-14001 /* アプリケーション辞書の指定がない */
#define UNA_ERR_VERSION_APP		-14002 /* アプリ辞書バージョンエラー */

/* 関数のモジュールごとの返り値の定義(UNABNS) */
#define UNA_ERR_VERSION_GRM		-15001 /* かかりうけテーブル
													バージョンエラー */
#define UNA_ERR_MORHINNUM_BNS	-15002 /* 形態素品詞番号範囲エラー */
#define UNA_ERR_OVERFLOW_BNS	-15003 /* 文節バッファオーバーフロー */

/* 関数のモジュールごとの返り値の定義(UNAMDUSR) */
#define UNA_ERR_INIT			-16001 /* 初期化失敗 */
#define UNA_ERR_TOO_LONG_INFO	-16002 /* アプリ情報が長すぎる
														(65535を超えた) */
#define UNA_ERR_OVERFLOW_INFO	-16003 /* アプリ情報バッファオーバーフロー */
#define UNA_ERR_OVERFLOW_MORPH	-16004 /* 形態素情報バッファオーバーフロー */
#define UNA_ERR_WRONG_ID		-16005 /* 単語IDが不適切 */
#define UNA_ERR_WRONG_POS		-16006 /* 単語位置が不適切 */
#define UNA_ERR_FORMAT_COST		-16007 /* テキスト形式のコストに誤りがある */
#define UNA_ERR_FORMAT_HYOKI	-16008 /* テキスト形式の表記を辞書形式に
															変換できない */
#define UNA_ERR_OVERFLOW_TXTBUF	-16009 /* ユーザ辞書形式をテキスト形式に
															変換できない */

/* 関数のモジュールごとの返り値の定義(UNAMDENG) */
#define UNA_ERR_VERSION_EMK		-17001 /* 英語トークン文字種別テーブル
													バージョンエラー */
#define UNA_ENG_SUB_MOR_BUF_OVR -17002 /* 英語トークン用下位構造形態素
													バッファオーバーフロー */

/* 関数のモジュールごとの返り値の定義(UNAWDGEN) */
#define UNA_ERR_VERSION_WDGEN   -18001 /* 単語生成データバージョンエラー */
#define UNA_ERR_WDGEN_ACPT      -18002 /* 元ルールの正規表現が複雑 */
#define UNA_ERR_WDGEN_IDSET     -18003 /* 元ルールの正規表現が複雑2 */
#define UNA_ERR_WDGEN_STATE     -18004 /* 元ルールの正規表現が複雑3 */
#define UNA_ERR_WDGEN_BRNCH     -18005 /* 複雑な解釈の考えられる形態素列 */
#define UNA_ERR_WDGEN_SUB       -18006 /* 下位構造格納バッファが足りない */

/* 関数のモジュールごとの返り値の定義(UNASTD) */
#define UNA_ERR_VERSION_STD     -19001 /* 文字変換データバージョンエラー */
#define UNA_ERR_UNSUPPORTED_CHAR	-19002 /* サポートされていないユニコード文字 */

/* 関数のモジュールごとの返り値の定義(UNATAG) */
#define UNA_ERR_TAG_NAME        -20001 /* タグ要素名エラー */

/* サインド・アンサインドデータ型 */
typedef unsigned char	 	ucharT;
typedef unsigned short	 	ushortT;
typedef unsigned int	 	uintT;
typedef signed char	 		scharT;
typedef signed short	 	sshortT;
typedef signed int	 		sintT;

/* 関数へのポインタ型 */
typedef void (*unaFuncT)();
/* 停止関数の型宣言 */
typedef int (*unaStopFuncT)(void);
/* 比較関数の型(キャスト用、sort、bsearchで使用) */
typedef int (*unaCompFuncT)(const void *,const void *); /* Unix */

//--------------------------------------------------------------------------
// TAG:	  unaFuncClT
//
// ABSTRACT:	  関数クロージャー型
//
// NOTE:
//	  実行すべき複数の関数を下位の関数に渡すための構造体
//
typedef struct unaFuncClT {			// 関数クロージャ型
	unaFuncT func;					/* 関数へのポインタ */
	unaFuncT func2;					/* 関数へのポインタ2 */
	void	 *arg;					/* 引数(void*型のみ) */
} unaFuncClT;

//--------------------------------------------------------------------------
// TAG:	  unaCharT
//
// ABSTRACT:	  内部コード型
//
// NOTE:
// 	  UNAがテキストを扱うときの内部コードを使うためのもの。
// 	  (unaCharT *)型によってテキストを表わし、これはユニコードである。
//
typedef ushortT unaCharT;

//--------------------------------------------------------------------------
// TAG:	  unaHinT
//
// ABSTRACT:	  UNA品詞型
//
// NOTE:
//	  UNAの品詞の型
//
typedef ushortT unaHinT;

//--------------------------------------------------------------------------
// TAG:	  unaStrIndexT
//
// ABSTRACT:	  ソートデータ用索引型
//
// NOTE:
//	  ソート用の索引を表わす構造体
//
typedef struct unaStrIndexT{	// ソートデータ用索引型
 	uintT pos;    			/* 文字列へのポインタ(要素番号) */
 	uintT len;    			/* 文字列の長さ     */
}unaStrIndexT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------
#if defined(UNA_DEBUG)
	extern int una_errMsg(const int handleNum,const char *moduleName,
							const int errNo,const char *errMsg,...);
	extern char *una_unitombs(const unaCharT *uStr,const uintT uStrLen);
	extern unaCharT *una_mbstouni(const char *sStr,const uintT sStrLen);
#endif
extern int una_fgetws(unaCharT *buf,int maxBufLen,FILE *fp);
extern int una_getToken(const unaCharT *uStr,int *tokenPos);
extern int una_getTokens(const unaCharT *uStr,int maxTokenCnt,
							unaStrIndexT *token);
extern int una_isxstr(unaCharT *uStr,int len);
extern int una_isdstr(unaCharT *uStr,int len);
extern int una_uc0stoas(ucharT *aStr,const unaCharT *uStr,int n);
extern int una_astouc0s(unaCharT *uStr,const ucharT *aStr,int n);
extern int una_utol(unaCharT *str);
extern uintT una_xtol(unaCharT *uStr);
extern double una_utof(unaCharT *uStr);
extern int una_ltou(int x,unaCharT *buf);
extern int una_wstrlen(const unaCharT *uStr);
extern int una_wstrcmp(const unaCharT *uStr1,const unaCharT *uStr2);
extern int una_wstrncmp(const unaCharT *uStr1,const unaCharT *uStr2,int n);
// この関数は実行データ作成時に使用される
extern void una_msort(void *base,int first,int last,int size,
					int (*cmp)(const void *,const void *),void *wrk);

#endif /* end of UNA_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
