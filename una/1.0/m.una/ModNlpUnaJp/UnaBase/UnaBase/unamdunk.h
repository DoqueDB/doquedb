//
// unamdunk.h -
//      未登録語処理モジュール
//		形態素解析処理の中で未登録語を処理するモジュール
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

#ifndef UNAMDUNK_H
#define UNAMDUNK_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"		/* UNAグローバルなヘッダファイル */
#include "unamorph.h"	/* 形態素解析メインモジュール */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* 未登録語カタカナの表記の最大長はデフォルトで15文字 */
#if !defined(UNAUNKKATAKANA) \
	|| UNAUNKKATAKANA < 15 || UNAUNKKATAKANA > UNA_HYOKI_LEN_MAX
	#define UNAUNKKATAKANA 15
#endif

/* 未登録語全体としての表記の最大長
				(カタカナ語の最大長が15文字以上の場合はその長さになる) */
// #define UNA_UNK_HYOKI_LIMIT UNAUNKKATAKANA
#define UNA_UNK_HYOKI_LIMIT UNA_HYOKI_LEN_MAX

/* 未登録語種類の数 */
#define UNA_UNK_MOR_KIND_CNT 13

//--------------------------------------------------------------------------
// TAG:	  unaMdUnkHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  未登録語処理モジュールをマルチスレッドセーフで実行するためのハンドラ
//
typedef struct unaMdUnkHandleT{	// 未登録語処理モジュール用ハンドル
	const char *unkMKTblImg;	/* 未登録語用文字種別テーブルのイメージ */
	const ushortT *uCTblImg;	/* 未登録語コスト推定テーブルのイメージ */
	int emulateBug;				/* switch for bug emulation */
	unaDicFuncT searchFunc; 	/* 未登録語検出関数 */
	unaDicFuncT searchFuncBug; 	/* 未登録語検出関数(バグエミュレート用) */
}unaMdUnkHandleT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaMdicUnknown_init(unaMdUnkHandleT *uh,const char *unkMKTblImg,
		const char *uCTblImg, int japaneaseMode);
/* 終了処理 */
int unaMdicUnknown_term(unaMdUnkHandleT *uh);
/* 未登録語検出関数 */
int unaMdicUnknown_searchMorph(unaMorphHandleT *mh,int txtPos,
		int dicNum,int *morCount,void *uHandle);

void unaMdicUnknown_setEmulateBug(unaMdUnkHandleT *uh, int emulateSwitch);

#endif /* end of UNAMDUNK_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
