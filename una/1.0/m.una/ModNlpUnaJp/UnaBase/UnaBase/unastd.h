//
// unastd.h -
//		文字変換モジュール
//		形態素解析処理の前に入力文字列を標準的な文字列に
//		置き換えるためのモジュール
// 
// Copyright (c) 2001-2009, 2023 Ricoh Company, Ltd.
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

#ifndef UNASTD_H
#define UNASTD_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"		/* UNAグローバルなヘッダファイル */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// TAG:	  unaStdHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  文字変換モジュールをマルチスレッドセーフで実行するためのハンドラ
//
typedef struct unaStdHandleT{	// 文字変換モジュール用ハンドル */
	int   checkNumber;			/* 初期化の有無をチェックするための番号 */
	int   repRuleNum;			/* 変換ルール数 */
	int   arraySize;			/* 変換用の配列サイズ */
	const char *stopCharTbl;	/* 変換終了文字テーブル */
	const uintT *base;			/* 文字変換のためのベース配列 */
	const unaCharT *label;		/* 文字変換のためのラベル配列 */
	const uintT *repStrIdx;		/* 変換後文字列を格納したバッファ位置 */
	const char *repStr;			/* 変換後文字列 */
}unaStdHandleT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaStd_init(unaStdHandleT *sh,const char *repTblImg);
/* 終了処理 */
int unaStd_term(unaStdHandleT *sh);
/* 文字列変換関数 */
int unaStd_cnv(unaStdHandleT *sh, unaCharT *stdText, int *textIndex, int stdTextMax, const unaCharT *inText, int inTextLen);
/* unaStdの状態取得関数 */
int unaStd_status(unaStdHandleT *sh);
/* unaStdの処理対象となる文字列のチェック */
int unaStd_check(unaStdHandleT *sh, const unaCharT *inText, int inTextLen);
#endif /* end of UNASTD_H */

//--------------------------------------------------------------------------
// Copyright (c) 2001-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
