//
// unabns.h -
//      かかりうけ解析
//    文節生成＆かかりうけ解析モジュール
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

#ifndef UNABNS_H
#define UNABNS_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"                /* UNAグローバルなヘッダファイル */
#include "unamorph.h"           /* 形態素を扱うので */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* ローカル処理サイズ */
#define UNA_LOCAL_BNS_SIZE 128       /* ローカルな文節生成数最大値 */

/* かかりうけ解析の関係値 */
#define UNA_REL_NOTHING   0   /* 無関係 */
#define UNA_REL_FUKUGO    1   /* 複合語関係 */
#define UNA_REL_HEIRETSU1 2   /* 強い並列関係 */
#define UNA_REL_HEIRETSU2 3   /* 弱い並列関係 */
#define UNA_REL_RENYOU1   4   /* 強い連用関係 */
#define UNA_REL_RENYOU2   5   /* 弱い連用関係 */
#define UNA_REL_RENTAI1   6   /* 強い連体関係 */
#define UNA_REL_RENTAI2   7   /* 弱い連体関係 */
#define UNA_REL_SETUZOKU1 8   /* 強い接続関係 */
#define UNA_REL_SETUZOKU2 9   /* 弱い接続関係 */
#define UNA_REL_KAKKO    10   /* 括弧関係 */
#define UNA_REL_KORITSU  11   /* 孤立関係 */

//--------------------------------------------------------------------------
// TAG:    unaBnsT
//
// ABSTRACT:    文節構造体
//
// NOTE:
//    文節を格納する構造体
//
typedef struct unaBnsT {// 文節構造体
  unaMorphT* start; 	 /* 開始位置(形態素バッファ中の) */
  int len;    			 /* その長さ(形態素数) */
  int target; 			 /* かかり先文節番号(文節バッファ中の) */
  int kuRel;  			 /* かかり先文節との関係 */
}unaBnsT;

//--------------------------------------------------------------------------
// TAG:    unaBnsHandleT
//
// ABSTRACT:   かかりうけモジュールのハンドラ
//
// NOTE:
//
typedef struct unaBnsHandleT {	// かかりうけモジュールのハンドラ
  int 			mrHinMax;	/* 形態素品詞番号最大値 */
  int 			hinMax;		/* 圧縮品詞番号最大値 */
  int 			kzMax;		/* かかり属性最大値 */
  int 			uzMax;		/* うけ属性最大値 */
  int 			lnMax;		/* 文節間距離最大値 */
  const short*	kuHin;		/* かかりうけ用の圧縮した品詞番号 */
  const ucharT* kTbl;		/* かかり属性用テーブル(兼文節切れ目フラグ) */
  const ucharT* uTbl;		/* うけ属性用テーブル */
  const ucharT* kuMap;		/* かかりうけ類型テーブル */
  const ucharT* kuCost;		/* かかりうけコストテーブル */
  const ucharT* lnCost;		/* 距離コストテーブル */
  int* 			pCost;		/* ワークバッファ１ */
  ucharT* 		pPtrn;		/* ワークバッファ２ */
  int* 			lStack;		/* ワークバッファ３ */
} unaBnsHandleT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaBns_init(unaBnsHandleT *bh, const char *gramTblImage);

/* 終了処理 */
int unaBns_term(unaBnsHandleT *bh);

/* 文節生成関数 */
int unaBns_gen( unaBnsHandleT *bh, unaBnsT *bbuf, int startPos,
  int bbufLen, unaMorphT *mbuf, int morphLen, int *processedMorphLen,
  int   *pCost, ucharT *pPtrn, int   *lStack);

/* かかりうけ解析関数 */
int unaBns_analyze( unaBnsHandleT *bh, unaBnsT *bbuf, int bbufLen,
  int   *pCost, ucharT *pPtrn, int   *lStack);

#endif /* end of UNABNS_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
