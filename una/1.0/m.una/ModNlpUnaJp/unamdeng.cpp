//
// unamdeng.cpp -
//      英語トークン検出モジュール
//		形態素解析処理の中で英語トークンを検出するモジュール
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

#include <stdio.h>				/* デバッグ用 */
#include <string.h>				/* memcpy */
#include <assert.h>				/* デバッグ用 */
#include "ModNlpUnaJp/unamdeng.h"		/* 英語トークン検出モジュール自身 */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------

#define MODULE_NAME "UNAMDENG"	/* モジュール名 */

/* モジュール内のメッセージ */
#define ERR_VERSION_EMK		"Unsupported English Moji Kind Table Version"
#define ENG_SUB_MOR_BUF_OVR	"Sub morph buffer of English Tokens overflow"
					/* 英語トークン用下位構造形態素バッファオーバーフロー */
//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// TAG:	  EngStatus
//
// ABSTRACT:    状態コードへの変換テーブル
//
// NOTE:
//    現在の状態コードと次の文字種番号を元に、次の状態コードを決定するための
//	  テーブル。状態コードは、以下の通りである。
//		 0		英語トークン対象外(初期状態決定に使用される)
//		 1		数字
//		 2		英字
//		 3		初期(1文字目)大文字
//		 4		初期(1文字目)小文字
//		 5		英連続大文字
//		 6		英連続小文字
//		 7		大文字頭文字
//		 8		小文字頭文字
//		 9		行末ハイフン後大文字
//		10		行末ハイフン後小文字
//		11		ピリオド
//		12		初期(1文字目)大文字後ピリオド
//		13		初期(1文字目)小文字後ピリオド
//		14		大文字頭文字後ピリオド
//		15		小文字頭文字後ピリオド
//		16		ハイフン
//		17		大文字後ハイフン
//		18		小文字後ハイフン
//		19		大文字1文字後ハイフン(使用していない)
//		20		空白
//		21		大文字後行末ハイフン後空白
//		22		小文字後行末ハイフン後空白
//		23		大文字1文字後行末ハイフン後空白(使用していない)
//		24		大文字後行末ハイフン後改行後空白
//		25		小文字後行末ハイフン後改行後空白
//		26		大文字1文字後行末ハイフン後改行後空白(使用していない)
//		27		復帰(CR)
//		28		大文字後行末ハイフン後復帰(CR)
//		29		小文字後行末ハイフン後復帰(CR)
//		30		大文字1文字後行末ハイフン後復帰(使用していない)
//		31		改行(LF)
//		32		大文字後行末ハイフン後改行(LF)
//		33		小文字後行末ハイフン後改行(LF)
//		34		大文字1文字後行末ハイフン後改行(使用していない)
//		35		音標
//		88		エラー
//		99		N/A(有得ないシーケンス)
//
//	  EngStatus[X][Y]のXは現在の状態コード、Yは文字種を表す。
//
//	  なお、文字種が「対象外」の時、次の状態が「対象外」に遷移する場合
//	  があるが、これは一時的なもの(次の状態を決定する場合)であり、
//	  その場合は、終了判別マトリックスより必ず処理終了となるので
//	  現在の状態が「対象外」になる事はない。
//
static const ucharT EngStatus[36][14] = { // 状態コードへの変換テーブル
/*                       対 数 大 大  Y 小 小  y 音 ピ ハ 空 復 改 */
/*                       象 字 文 文    文 文    標 リ イ 白 帰 行 */
/*                       外    字 字    字 字       オ フ          */
/*                             母 子    母 子       ド ン          */
/*                        0  1  2  3  4  5  6  7  8  9 10 11 12 13 */
/* 対象外(初期)    0 */ {99, 1, 3, 3, 3, 4, 4, 4,35,11,16,20,27,31},
/* 数字            1 */ { 0, 1, 3, 3, 3, 4, 4, 4,35,11,16,20,27,31},
/* 英字            2 */ { 0, 1, 2, 2, 2, 2, 2, 2, 2,11,17,20,27,31},
/* 初期大文字      3 */ { 0, 1, 5, 5, 5, 6, 6, 6, 3,12,17,20,27,31},
/* 初期小文字      4 */ { 0, 1, 2, 2, 2, 6, 6, 6, 4,13,18,20,27,31},
/* 大文字連続      5 */ { 0, 1, 5, 5, 5, 2, 2, 2, 5,11,17,20,27,31},
/* 小文字連続      6 */ { 0, 1, 2, 2, 2, 6, 6, 6, 6,11,18,20,27,31},
/* 頭大文字        7 */ { 0, 1, 2, 2, 2, 2, 2, 2, 7,14,16,20,27,31},
/* 頭小文字        8 */ { 0, 1, 2, 2, 2, 2, 2, 2, 8,15,16,20,27,31},
/* 末大文字        9 */ { 0, 1,10,10,10,10,10,10,10,11,16,20,27,31},
/* 末小文字       10 */ { 0, 1,10,10,10,10,10,10,10,11,16,20,27,31},
/* ピリオド       11 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,20,27,31},
/* 初頭大ピリオド 12 */ { 0, 1, 7, 7, 7, 8, 8, 8,35,11,16,20,27,31},
/* 初頭小ピリオド 13 */ { 0, 1, 7, 7, 7, 8, 8, 8,35,11,16,20,27,31},
/* 頭大ピリオド   14 */ { 0, 1, 7, 7, 7, 8, 8, 8,35,11,16,20,27,31},
/* 頭小ピリオド   15 */ { 0, 1, 7, 7, 7, 8, 8, 8,35,11,16,20,27,31},
/* ハイフン       16 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,20,27,31},
/* 大ハイフン     17 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,21,29,33},
/* 小ハイフン     18 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,22,29,33},
/* 末1ハイフン    19 */ {99,99,99,99,99,99,99,99,99,99,99,99,99,99},
/* 空白           20 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,20,27,31},
/* 大-空白        21 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,21,29,33},
/* 小-空白        22 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,22,29,33},
/* 末1空白        23 */ {99,99,99,99,99,99,99,99,99,99,99,99,99,99},
/* 末大頭空白     24 */ { 0, 1,10,10,10,10,10,10,35,11,16,24,27,31},
/* 末小頭空白     25 */ { 0, 1,10,10,10,10,10,10,35,11,16,25,27,31},
/* 末1頭空白      26 */ {99,99,99,99,99,99,99,99,99,99,99,99,99,99},
/* 復帰           27 */ {88,88,88,88,88,88,88,88,88,88,88,88,88,31},
/* 末大復帰       28 */ {88,88,88,88,88,88,88,88,88,88,88,88,88,32},
/* 末小復帰       29 */ {88,88,88,88,88,88,88,88,88,88,88,88,88,33},
/* 末1復帰        30 */ {99,99,99,99,99,99,99,99,99,99,99,99,99,99},
/* 改行           31 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,20,27,31},
/* 末大改行       32 */ { 0, 1,10,10,10,10,10,10,35,11,16,24,27,31},
/* 末小改行       33 */ { 0, 1,10,10,10,10,10,10,35,11,16,25,27,31},
/* 末1改行        34 */ {99,99,99,99,99,99,99,99,99,99,99,99,99,99},
/* 音標           35 */ { 0, 1, 2, 2, 2, 2, 2, 2,35,11,16,20,27,31}
};

//--------------------------------------------------------------------------
// TAG:	  TourokuMtx
//
// ABSTRACT:    英語トークン登録マトリックス
//
// NOTE:
//    現在の状態コードと次の文字種番号を元に、登録フラグの値を得るテーブル。
//	  ここで、登録フラグは英語トークンの候補出しをする場合のトークンの
//	  タイプを決定付ける元となるべきものであり、以下の値を取る。
//
//		0 の場合	候補出しをしない
//		1 の場合	通常の英語トークンとして候補出しをする
//		2 の場合	行末ハイフン処理された英語トークンとして候補出しをする
//		3 の場合	アラビア数字として候補出しをする
//		4 の場合	記号として候補出しをする
//		5 の場合	空白として候補出しをする
//		6 の場合	改行として候補出しをする
//		7 の場合	頭文字として候補出しをする
//		9 の場合	N/A(有得ないシーケンス)
//    TourokuMtx[X][Y]のXは現在の状態コード、Yは文字種を表す。
//
//	  (注1)
//		現在の状態が「対象外」になる事は有得ないから対象外の行は全て N/A
//		になっている。(次の状態が「対象外」になる事はある)
//
static const ucharT TourokuMtx[36][14] ={ // 英語トークン登録マトリックス
/*                       対 数 大 大  Y 小 小  y 音 ピ ハ 空 復 改 */
/*                       象 字 文 文    文 文    標 リ イ 白 帰 行 */
/*                       外    字 字    字 字       オ フ          */
/*                             母 子    母 子       ド ン          */
/*                        0  1  2  3  4  5  6  7  8  9 10 11 12 13 */
/* 対象外(注1)     0 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
/* 数字            1 */ { 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
/* 英字            2 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* 初期大文字      3 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* 初期小文字      4 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* 大文字連続      5 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* 小文字連続      6 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* 頭大文字        7 */ { 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7},
/* 頭小文字        8 */ { 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7},
/* 末大文字        9 */ { 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2},
/* 末小文字       10 */ { 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2},
/* ピリオド       11 */ { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
/* 初頭大ピリオド 12 */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
/* 初頭小ピリオド 13 */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
/* 頭大ピリオド   14 */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
/* 頭小ピリオド   15 */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7},
/* ハイフン       16 */ { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
/* 末大ハイフン   17 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末小ハイフン   18 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末1ハイフン    19 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 空白           20 */ { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5},
/* 末大空白       21 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末小空白       22 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末1空白        23 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末大頭空白     24 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末小頭空白     25 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末1頭空白      26 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 復帰           27 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末大復帰       28 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末小復帰       29 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末1復帰        30 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 改行           31 */ { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
/* 末大改行       32 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末小改行       33 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 末1改行        34 */ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 音標           35 */ { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}
};

//--------------------------------------------------------------------------
// TAG:	  OwariMtx
//
// ABSTRACT:    英語トークン終了判別マトリックス
//
// NOTE:
//    現在の状態コードと次の文字種番号を元に、英語トークン検出を終了するか
//	  を判定するテーブル。ここで
//		0 の場合	英語トークン検出を続ける
//		1 の場合	英語トークン検出を終了する
//		9 の場合	N/A(有得ないシーケンス)
//    OwariMtx[X][Y]のXは現在の状態コード、Yは文字種を表す。
//
//	  (注1)
//		現在の状態が「対象外」になる事は有得ないから対象外の行は全て N/A
//		になっている。(次の状態が「対象外」になる事はある)
//
static const ucharT OwariMtx[36][14] = { // 英語トークン終了判別マトリックス
/*                       対 数 大 大  Y 小 小  y 音 ピ ハ 空 復 改 */
/*                       象 字 文 文    文 文    標 リ イ 白 帰 行 */
/*                       外    字 字    字 字       オ フ          */
/*                             母 子    母 子       ド ン          */
/*                        0  1  2  3  4  5  6  7  8  9 10 11 12 13 */
/* 対象外(注1)     0 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
/* 数字            1 */ { 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* 英字            2 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
/* 初期大文字      3 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
/* 初期小文字      4 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
/* 大文字連続      5 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
/* 小文字連続      6 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
/* 頭大文字        7 */ { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
/* 頭小文字        8 */ { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
/* 末大文字        9 */ { 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
/* 末小文字       10 */ { 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
/* ピリオド       11 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* 初頭大ピリオド 12 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
/* 初頭小ピリオド 13 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
/* 頭大ピリオド   14 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
/* 頭小ピリオド   15 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
/* ハイフン       16 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* 末大ハイフン   17 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 末小ハイフン   18 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 末1ハイフン    19 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 空白           20 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
/* 末大空白       21 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 末小空白       22 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 末1空白        23 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
/* 末大頭空白     24 */ { 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1},
/* 末小頭空白     25 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1},
/* 末1頭空白      26 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1},
/* 復帰           27 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末大復帰       28 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末小復帰       29 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 末1復帰        30 */ { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0},
/* 改行           31 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/* 末大改行       32 */ { 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1},
/* 末小改行       33 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1},
/* 末1改行        34 */ { 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1},
/* 音標           35 */ { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

//--------------------------------------------------------------------------
// TAG:	  EngMorHinTable
//
// ABSTRACT:    英語トークン品詞推定テーブル
//
// NOTE:
//    英語トークンの種類(7種類)から、品詞を推定するための変換テーブル
//
static const ushortT EngMorHinTable[3] = { // 英語トークン品詞推定テーブル
	UNA_HIN_NOTHING,		/* N/A */
	UNA_HIN_USER_DEFINED_1,	/* 通常の英語トークン */
	UNA_HIN_USER_DEFINED_1,	/* 行末ハイフン処理された英語トークン */
};

//--------------------------------------------------------------------------
// TAG:	  EngCostTable
//
// ABSTRACT:    英語トークンコスト推定テーブル
//
// NOTE:
//    英語トークンの種類(7種類)から、コストを推定するための変換テーブル。
//	  頭文字を含む語がハイフン処理されていた場合、例えば
//
// JAPAN-[SP][LF]
// [SP]U.S.A
//		→ /JAPAN-[SP][LF][SP]U//.//S.A/  となってしまう可能性があるので
//		→ /JAPAN//-//[SP]//[LF]//[SP]//U.S.A/  となるように
// 又、
//
// U.S.A-[SP][LF]
// [SP]JAPAN
//		→ /U.S./A-[SP][LF][SP]JAPAN/  となってしまう可能性があるので
//		→ /U.S.A//-//[SP]//[LF]//[SP]//JAPAN/  となるように
//
//	- 普通英語トークンの可能性のある場合も長さによる重みは付けない。
//	  (空白も)
//	- デフォルトで行末ハイフン語のコストを頭文字語より高く設定
// 
static const ushortT DefaultEngCostTable[8] = { // 英語トークンコスト推定テーブル
	0,	/* N/A */
	4,	/* 通常の英語トークン */
	9,	/* 行末ハイフン処理された英語トークン */
	1,	/* アラビア数字 */
	1,	/* 記号 */
	1,	/* 空白 */
	1,	/* 改行 */
	1	/* 頭文字 */
};
static ushortT *EngCostTable = (ushortT *)DefaultEngCostTable;

/* マクロ定義 */
#define HYPHEN_TOKEN		1 /* 行末ハイフン処理されたトークンである  */
#define NOT_HYPHEN_TOKEN	0 /* 行末ハイフン処理されたトークンでない  */
#define ENG_BYPASS			1 /* 処理をバイパスする */
#define ENG_NOT_BYPASS		0 /* 処理はバイパスしない */
#define ENG_START			0 /* 初期値 */
#define ENG_C				1 /* 子音 */
#define ENG_V				2 /* 母音 */
#define ENG_VC				3 /* 母音+子音 */
#define ENG_CV				4 /* 子音+母音 */

/* バイパスチェック */
static int CheckBypass(const char *engMKTblImg,unaCharT *tbuf,
		int txtPos,uintT mojiKind);

/* 行末ハイフン処理トークンの検定及び下位構造をカウントする */
static int CheckAndCntHyphen(const unaMDEngHandleT *eh,const unaCharT *text,
		int txtPos,uintT morLen,int *subMorphCnt);

/* 英語トークンの形態素品詞及びコストの決定 */
static int EngGetHinAndCost(int tourokuFlg,int dicNum,
		ushortT *morHin,uintT *appI);

/* 行末ハイフン英語トークン用下位構造取得 */
static int GetSubMorphHyphen(const unaMorphT *morph,unaMorphT *morphBuf,
		int *morphNum,int appIHead,const unaMDEngHandleT *eh);

/* 形態素バッファに値をセットする */
static void SubMorphBufSet(unaMorphT *morphBuf,int	idx,unaCharT *start,
		sshortT  length,ushortT  hinshi,uintT   appI,uintT   subI);

//--------------------------------------------------------------------------
// MODULE:	  unaMDicEnglish_init
//
// ABSTRACT:    英語トークン処理モジュールの初期化
//
// FUNCTION:
//	  英語トークン処理モジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//	  なし
// 
int unaMDicEnglish_init(
		unaMDEngHandleT *eh,	/* ハンドラ */
		const char *engMKTblImg	/* 英語トークン用文字種別テーブルのイメージ*/
)
{
	const char *imgPtr;			/* テーブルを格納したメモリへのポインタ */

	imgPtr = engMKTblImg + UNA_COM_SIZE;
	if (strcmp(imgPtr, UNA_EMK_VER) == 0) {
		imgPtr += UNA_VER_SIZE;
		eh->engMKTblImg = imgPtr;
		eh->engCTblImg = (ushortT *)(imgPtr + 0x10000);
		EngCostTable = (ushortT *)(eh->engCTblImg);
		return UNA_OK;
	} else if (strcmp(imgPtr, UNA_EMK_VER_107) == 0) {
		imgPtr += UNA_VER_SIZE;
		eh->engMKTblImg = imgPtr;
		return UNA_OK;
	}
	
	/* バージョンが違う */
	UNA_RETURN(ERR_VERSION_EMK,NULL);
}

//--------------------------------------------------------------------------
// MODULE:	  unaMDicEnglish_term
//
// ABSTRACT:    英語トークン処理モジュールの終了処理
//
// FUNCTION:
//	  英語トークン処理モジュールの終了処理を行う
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//
int unaMDicEnglish_term(
		unaMDEngHandleT *eh		/* ハンドラ */
)
{
	/* 値のリセット */
	eh->engMKTblImg = (const char *)NULL;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMDicEnglish_searchMorph
//
// ABSTRACT:    英語トークンを検出
//
// FUNCTION:
//	  指定した文字位置から始まる英語トークンを格納して返す。
//
// RETURN:
//	  UNA_OK			正常終了
//	  UNA_SYUSOKU		収束した(オーバーフローしたので収束したとみなす)
//
// NOTE:
//
//	  (注1)
//		ここまで来たということは、英語トークン開始の可能性があるということ
//		なので、前回の状態が「対象外」だったとして、今回の(初期)状態コード
//		を求める。
//	  (注2)
//		ここには、英語トークン最大長(UNA_HYOKI_LEN_MAX)制限により強制終了する
//		場合の処理が書かれていたが、UNA_LOCAL_TEXT_SIZE=UNA_HYOKI_LEN_MAX
//		となった事により、その処理は不用になった。(テキストの最後の文字の
//		場合の処理に包含されるから)よってここには assert 文のみ残す。
//	  (注3)
//		トークン種別(tourokuFlgの値)から品詞とコストを推定する
//		appIもそのトークン種別のものに更新する。また、英語トークンと
//		頭文字トークンの subIは、下位構造の数をセットする
//
int unaMDicEnglish_searchMorph(
	unaMorphHandleT *mh,   /* 形態素解析メインモジュール用ハンドラ */
	int txtPos,			   /* テキスト上で解析を行う開始位置(オフセット) */
	int dicNum,			   /* 辞書ナンバー(appI格納のため) */
	int *morCount,		   /* 登録した形態素数 */
	void *eHandle		   /* 英語トークン検出用ハンドラ(キャストして使用) */
)
{
	unaMDEngHandleT *eh;	/* ハンドラ(英語トークン検出用) */
	uintT mojiKind;			/* 文字種 */
	uintT curStatus;		/* 現在の状態コード */
	uintT nextStatus;		/* 次の状態コード */
	uintT morLen;			/* 形態素(英語トークン)の文字長 */
	int subMorphCnt;		/* 下位構造の数 */
	int tourokuFlg;			/* 登録フラグ(1の場合登録) */
	int owariFlg;			/* 終了フラグ(1の場合終わり) */
	uintT appI;				/* アプリケーションインデックス情報 */
	ushortT morHin;			/* 形態素品詞番号 */
	int morCost;			/* 形態素(英語トークン)コスト */
	int i;					/* ループ変数 */
	int rv;					/* 関数の返り値 */

	/* 必要な変数の初期化 */
	*morCount = 0;
	morLen = 0;
	subMorphCnt = 0;
	tourokuFlg = 0;
	owariFlg = 0;
	eh = (unaMDEngHandleT *)eHandle;

	mojiKind = (eh->engMKTblImg)[mh->lat.tbuf[txtPos]];
	if (mojiKind == 0) {	/* 英語トークン対象外 */
		return UNA_OK;		/* バイパス */
	}

	rv = CheckBypass(eh->engMKTblImg,mh->lat.tbuf,txtPos,mojiKind);
	if (rv == ENG_BYPASS) {	/* バイパス */
		return UNA_OK;
	}

	/* 注1 */
	nextStatus = EngStatus[0][mojiKind];	/* 0 は、状態「対象外」 */

	/* １文字ずつ見ながら、英語トークン候補を形態素枝データに格納する */
	for(i = txtPos;;i++){	/* forever */

		/* 現在の英語トークン候補文字列の末尾文字の状態を更新する */
		curStatus = nextStatus;
		/* 文字列長をこれから登録しようとする英語トークンの長さに更新する */
		morLen++;

		if ( morLen > mh->mwLen){
			break;
		}

		/* 現在が文字列の最後の文字か判断(ここまでで文字列が終了するか) */
		if (i + 1 >= mh->lat.txtLen || mh->lat.tbuf[i + 1] == 0x0000) {
			/* 終わりの時は、次の文字が対象外だったと仮定して
												登録マトリックスを参照する */
			tourokuFlg	= TourokuMtx[curStatus][0];	/* 0 は対象外 */
			owariFlg	= 1;					/* 英語トークンは処理終了 */
		}
		else{	/* 終了しない場合 */
			/* 現在の英語トークン候補文字列の次の文字種を求める */
			mojiKind = (eh->engMKTblImg)[mh->lat.tbuf[i + 1]];
			/* 現在の状態と次の文字種から次の状態コードを得る */
			nextStatus = EngStatus[curStatus][mojiKind];
			if (nextStatus == 88) { /* エラー */
				break;	/* エラーコードは返さず、ここまでで終る */
			}
			/* 現在の状態と次の文字種から文字連接マトリクスを参照する */
			tourokuFlg	= TourokuMtx[curStatus][mojiKind];
			owariFlg	= OwariMtx[curStatus][mojiKind];
			assert (morLen < UNA_HYOKI_LEN_MAX); /* 注2 */
		}

		switch (tourokuFlg) {
		case 2:	/* 行末ハイフン処理された英語トークン */
			/* さらに音(イン)による厳密なチェックを行う */
			rv = CheckAndCntHyphen(eh,mh->lat.tbuf,txtPos,morLen,
															&subMorphCnt);
			if (rv != HYPHEN_TOKEN) {
				if (rv != NOT_HYPHEN_TOKEN) { /* エラー */
					return rv;
				}
				else {	  /* 行末ハイフン処理された英語トークンでない */
					tourokuFlg = 0;	/* 登録しない */
				}
			}
			break;
		default:
			subMorphCnt = 0;	 /* 上記以外の英語トークンの下位構造はない */
			break;
		}

		/* 登録、終了フラグによって候補格納／処理終了を行う */
		if (tourokuFlg != 0) {	/* tourokuFlgが0以外は、トークン種別 */
		
			if ( tourokuFlg >=3 ){ /* v3.2.5仕様変更に対応 */
				;
			}
			else{
				/* 注3 */
				morCost = EngGetHinAndCost(tourokuFlg,dicNum,&morHin,&appI);
				/* ラティスに登録 */
				rv = unaMorph_latSet(mh,txtPos,morLen,morHin,morCost,appI,
								(uintT)subMorphCnt,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
			}
		}
		/* 処理終了 */
		if (owariFlg == 1) {
			break;
		}
	}

	/* 終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  CheckBypass
//
// ABSTRACT:    バイパスチェック
//
// FUNCTION:
//		現在の1つ前の位置の文字種を得て、処理をバイパスするかチェックを行う
//
// RETURN:
//	  ENG_BYPASS		バイパスする
//	  ENG_NOT_BYPASS	バイパスしない
//
// NOTE:
//	  (注1)
//		1	数字
//		2	英大文字母音
//		3	英大文字子音
//		4	Y
//		5	英小文字母音
//		6	英小文字子音
//		7	y
//		8	音標(前文字種がアルファベットの時、音標もアルファベットとみなす)
//		11	空白
//
//	  (注2)
//		英語トークンの切れ目の時に、1文字ずつ伸ばしてトークン検出が
//		行われ、現在の文字もその一部として対象になっているはず
//		なのでバイパスする。。
//
static int CheckBypass(
	const char *engMKTblImg, /* 英語トークン用文字種別テーブルのイメージ*/
	unaCharT *tbuf,			 /* 入力文字列(テキスト全体)へのポインタ */
	int txtPos,				 /* テキスト上で解析を行う開始位置(オフセット) */
	uintT mojiKind			 /* 現在の位置(txtPos)の文字種 */
)
{
	uintT maeMojiKind;		 /* txtPosの1つ前の文字種 */
	
	if (txtPos != 0) {	/* txtPos==0 の時は前の文字が無いので対象にしない */
		maeMojiKind = engMKTblImg[tbuf[txtPos - 1]];
		if (mojiKind == 1 && maeMojiKind == 1) { /* 数字の途中(注1) */
			return ENG_BYPASS;	/* バイパス(注2) */
		}
		if ((mojiKind >= 2 && mojiKind <= 8)
		&& (maeMojiKind >= 2 && maeMojiKind <= 7)) { /* 英字の途中(注1) */
			return ENG_BYPASS;	/* バイパス(注2) */
		}
		if (mojiKind == 11 && maeMojiKind == 11) { /* 空白の途中(注1) */
			return ENG_BYPASS;	/* バイパス(注2) */
		}
	}

	return ENG_NOT_BYPASS;
}

//--------------------------------------------------------------------------
// MODULE:	  CheckAndCntHyphen
//
// ABSTRACT:    行末ハイフン英語トークンチェック
//
// FUNCTION:
//		行末ハイフン処理された英語トークンであるかを、音(イン)により
//		チェックすると同時に後続セグメントの数によるチェックも行う。
//		又、下位構造の数をカウントする。
//
// RETURN:
//	  HYPHEN_TOKEN			行末ハイフン処理された英語トークンである
//	  NOT_HYPHEN_TOKEN		行末ハイフン処理された英語トークンではない
//	  その他				エラー
//
// NOTE:
//	  判定対象となるトークンは、以下の形式でなければならない
//		X-[SP][CR](LF)[SP]YYY
//		X-[SP][CR](LF)[SP]yyy
//		XXX-[SP][CR](LF)[SP]YYY
//		xxx-[SP][CR](LF)[SP]yyy
//		Xxx-[SP][CR](LF)[SP]yyy
//		但し、
//			X は、1個の大文字アルファベット
//			XXX は、1個以上のアルファベット連続で、全て大文字
//			xxx は、1個以上のアルファベット連続で、全て小文字
//			YYY は、1個以上のアルファベット連続で、全て大文字
//			yyy は、1個以上のアルファベット連続で、全て小文字
//			[SP]は、任意個(0でも良い)の空白
//			[CR]は、1個または0個の復帰
//			(LF)は、1個の改行
//
//	  なおアルファベットは、音標文字とセットの場合もあり得る。
//
//	  (注3)
//		空白を下位形態素としてカウントするための処理。連続した空白は1個として
//		カウントされる。理由は以下の通り。
//		ハイフン直後に空白が存在した場合は subCntFlg の初期値より下位形態素
//		としてカウントされる。又復改後に空白が存在した場合にも復改の処理で
//		subCntFlg をリセットし直しているので下位形態素としてカウントされる。
//
static int CheckAndCntHyphen(
	const unaMDEngHandleT *eh,	/* ハンドラ(英語トークン検出用) */
	const unaCharT *text,		/* 文字列 */
	int txtPos,					/* 文字位置(オフセット) */
	uintT morLen,				/* 文字長(バイトではない) */
	int *subMorphCnt			/* 下位構造の数 */
)
{
	unsigned int	i;			/* ループ変数 */
	uintT	mojiKind;			/* 文字種 */
	int	subCntFlg;				/* 下位構造カウントフラグ
									(UNA_TRUE:カウントする UNA_FALSE:しない)*/

	/* 初期設定 */
	*subMorphCnt = 0;

	/* このループでは先行セグメントの音のチェックを行う */
	for (i = txtPos;(mojiKind = (eh->engMKTblImg)[text[i]]) != 10;i++) {
		;
	}
	subCntFlg	= UNA_TRUE;		/* 注3 */

	/* このループでは後続セグメントの音のチェックを行う */
	for (i++ ; i < txtPos + morLen ; i++) { /* iの初期値をカウントアップ */
		mojiKind = (eh->engMKTblImg)[text[i]];
		switch(mojiKind){
		case 11:	/* 空白 */
			if (subCntFlg == UNA_TRUE) { /* 注3 */
				(*subMorphCnt)++;		 /* 注3 */
				subCntFlg = UNA_FALSE;	 /* 注3 */
			}							 /* 注3 */
			break;
		case 12:	/* 復帰(CR) */
		case 13:	/* 改行(LF) */
			subCntFlg = UNA_TRUE;		 /* 注3 */
			break;
		case 2:		/* 英大文字母音 */
		case 3:		/* 英大文字子音 */
		case 4:		/* Y */
		case 5:		/* 英小文字母音 */
		case 6:		/* 英小文字子音 */
		case 7:		/* y */
			break;
		default:	/* 音標 */
			assert(mojiKind == 8);
		}
	}
	
	(*subMorphCnt) += 4; /* 先行セグメント、ハイフン、復改、後続セグメント
							の4つ分を足す */
	return HYPHEN_TOKEN;
}

//--------------------------------------------------------------------------
// MODULE:	  EngGetHinAndCost
//
// ABSTRACT:    英語トークンの形態素品詞及びコストの決定
//
// FUNCTION:
//	  登録フラグから形態素品詞番号とコストを得る
//
// RETURN:
//	  コスト値
//
// NOTE:
//	  辞書に登録された形態素のコストの最大値は、255 であるが、英語トークンの
//	  コストの最大値は、65535 までとる事ができる。
//	  ただし、現時点では辞書登録語より英語トークンによって検出された形態素を
//	  優先するのでコストはできるだけ小さくしてある。
//
static int EngGetHinAndCost(
	int tourokuFlg,			/* 登録フラグ */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	ushortT *morHin,		/* 形態素品詞番号 */
	uintT *appI				/* アプリケーションインデックス情報 */
)
{
	*morHin = EngMorHinTable[tourokuFlg];
	*appI = (dicNum << 24) | (1 << (tourokuFlg - 1));
	return EngCostTable[tourokuFlg]; /* 現在長さの重み0 */
}

//--------------------------------------------------------------------------
// MODULE:	  unaMDicEnglish_appInfoGet
//
// ABSTRACT:    英語トークン用アプリ情報取得
//
// FUNCTION:
//		英語トークンモジュールで辞書引きされた形態素のアプリ情報を
//		取得する。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  英語トークンは、本来未登録語の一種であるためアプリ情報は存在しないので
//	  表記を加工してアプリ情報として返す。なお、加工された表記の実体は
//	  ハンドラに存在する。
//
//	  (注1)
//		行末ハイフン処理された英語トークンは、以下の形式である
//			xxx-[SP][CR](LF)[SP]yyy
//		但し、
//			xxx は、先行セグメントで1個以上のアルファベット連続
//			- は、1個の行末ハイフン
//			yyy は、後続セグメントで1個以上のアルファベット連続
//			(SP)は、任意個(0でも良い)の空白
//			[CR]は、1個または0個の復帰
//			(LF)は、1個の改行
//		これを、xxxyyy のかたちにまとめ上げる
//		なおアルファベットは、音標文字とセットの場合もあり得る。
//
int unaMDicEnglish_appInfoGet(
	const unaMorphT *morph,	 	/* アプリ情報取得対象の形態素(単語) */
	unaAppInfoT **appInf,		/* アプリ情報 */
	unaMDEngHandleT *eh			/* ハンドラ(英語トークン検出用) */
)
{
	uintT	 tokenType;			/* トークン種別 */
	int	 pos;					/* ポジション */
	uintT	 mojiKind;			/* 文字種 */
	int	 alphaCnt;				/* まとめ上げた文字の文字数 */
	unaCharT *infoPtr;			/* アプケーション情報の内容を示すポインタ */

	*appInf = (unaAppInfoT *)&(eh->appInfo);
								/* アプリ情報のアドレスを予め設定 */

	tokenType = (morph->appI) & 0x00ffffff;
								/* appIの下位24ビットがトークン種別 */

	/* 行末ハイフン以外の英語トークンの場合 */
	if (tokenType != UNA_ENG_TOKEN_HYPHEN) { /* 行末ハイフン以外 */
		(*appInf)->len = morph->length * sizeof(unaCharT);
		memcpy((*appInf)->info,morph->start,(*appInf)->len);
		return UNA_OK;
	}

	/* 行末ハイフン英語トークンの場合まとめ上げを行う(注1) */
	alphaCnt = 0;
	infoPtr = (unaCharT *)((*appInf)->info);
	for (pos = 0;pos < morph->length;pos++){
		mojiKind = (eh->engMKTblImg)[(morph->start)[pos]];
		switch (mojiKind) {
		case 10:	/* ハイフン */
		case 11:	/* 空白     */
		case 12:	/* 復帰(CR) */
		case 13:	/* 改行(LF) */
			break;	/* 上記の場合は何もしない */
		default:
			*(infoPtr++) = *(morph->start + pos);
			alphaCnt++;
		}
	}
	/* まとめ上げ文字列の長さ(xxxyyy の長さ) */
	(*appInf)->len = alphaCnt * sizeof(unaCharT);

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMDicEnglish_getSubMorph
//
// ABSTRACT:    英語トークン用下位構造取得
//
// FUNCTION:
//		英語トークンの下位構造を取得する。
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
//
// NOTE:
//  (注1)
//		英語トークンの場合 subI には下位構造の数がセットされている
//
int unaMDicEnglish_getSubMorph(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,		/* バッファの大きさ(要素数) */
	const unaMDEngHandleT *eh	/* ハンドラ(英語トークン検出用) */
)
{
	int	appIHead;				/* appIヘッド */
	uintT	tokenType;			/* トークン種別 */
	int	rv;						/* 関数の返り値 */

	/* バッファオーバーフローチェック */
	if (morph->subI > (uintT)morphBufSize) {	/* 注1 */
		UNA_RETURN(ENG_SUB_MOR_BUF_OVR,NULL);
	}

	/* 上位の形態素のappIヘッドを得る */
	appIHead = morph->appI & 0xff000000;
								/* appIの上位8ビットがappIHead */

	/* 上位の形態素のトークン種別を得る */
	tokenType = morph->appI & 0x00ffffff;
								/* appIの下位24ビットがトークン種別 */

	/* トークン種別により処理を分ける */
	assert(tokenType>=0 && tokenType<3);
	switch (tokenType) {
	case UNA_ENG_TOKEN_HYPHEN:	/* 行末ハイフン */
		rv = GetSubMorphHyphen(morph,morphBuf,morphNum,appIHead,eh);
		assert (rv == UNA_OK);
		break;
	default:
		*morphNum = 0;	/* 下位構造はない */
	}

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  GetSubMorphHyphen
//
// ABSTRACT:    行末ハイフン英語トークン用下位構造取得
//
// FUNCTION:
//		行末ハイフン英語トークンの下位構造を取得する。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  この関数は行末ハイフン処理された英語トークンを対象とする
//	  即ちトークンは、以下の形式である
//		xxx-[SP][CR](LF)[SP]yyy
//	  但し、
//		xxx は、先行セグメントで1個以上のアルファベット連続
//		- は、1個の行末ハイフン
// 		yyy は、後続セグメントで1個以上のアルファベット連続
//		(SP)は、任意個(0でも良い)の空白
//		[CR]は、1個または0個の復帰
//		(LF)は、1個の改行
//
//	  なおアルファベットは、音標文字とセットの場合もあり得る。
//
static int GetSubMorphHyphen(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	int appIHead,				/* appIヘッド(appIの上位8ビット) */
	const unaMDEngHandleT *eh	/* ハンドラ(英語トークン検出用) */
)
{
	int	pos;				/* ポジション */
	uintT mojiKind;			/* 文字種 */
	int	alphaCnt;			/* 先行セグメント(アルファベット)の文字数 */
	int	spcCnt;				/* 空白の文字数 */
	int	kaiCnt;				/* 改行の文字数(1又は2) */
	int	offset;				/* 英語トークン先頭からのオフセット */
	int	idx;				/* 下位形態素配列の要素番号*/

	/* 初期値セット */
	offset = 0;
	idx = 0;

	/* 先行セグメント文字数カウント */
	alphaCnt = 0;
	for (pos = 0;;pos++){ /* forever */
		mojiKind = (eh->engMKTblImg)[(morph->start)[pos]];
		if(mojiKind == 10){	/* ハイフン */
			break;
		}
		alphaCnt++;
	}
	/* 先行セグメント */
	(void)SubMorphBufSet(morphBuf,idx,&morph->start[offset],(short)alphaCnt,
			UNA_HIN_USER_DEFINED_1,appIHead | UNA_ENG_TOKEN_NORMAL,0);
	/* オフセット、要素番号更新 */
	offset = alphaCnt;
	idx++;

	/* 行末ハイフン */
	(void)SubMorphBufSet(morphBuf,idx,&(morph->start)[offset],1,
		UNA_HIN_USER_DEFINED_1,appIHead | UNA_ENG_TOKEN_SYMBOL,0);
	/* オフセット、要素番号更新 */
	offset++;
	idx++;

	/* 行末ハイフンと復改の間の空白カウント */
	spcCnt = 0;
	for (pos++;;pos++){ /* forever */
		mojiKind = (eh->engMKTblImg)[(morph->start)[pos]];
		if(mojiKind == 12 || mojiKind == 13){	/* 復帰(CR) || 改行(LF) */
			break;
		}
		spcCnt ++;
	}

	if (spcCnt != 0) {
		/* 行末ハイフンと復改の間の空白 */
		(void)SubMorphBufSet(morphBuf,idx,&(morph->start)[offset],
					(short)spcCnt,UNA_HIN_USER_DEFINED_1,
					appIHead | UNA_ENG_TOKEN_SPACE,0);
		/* オフセット、要素番号更新 */
		offset += spcCnt;
		idx++;
	}

	/* 復改 */
	if (mojiKind == 12) {	/* 復帰(CR)付改行(LF) */
		kaiCnt = 2;
	}
	else {
		kaiCnt = 1;			/* 復帰(CR)無し改行(LF) */
	}
	(void)SubMorphBufSet(morphBuf,idx,&(morph->start)[offset],(short)kaiCnt,
			UNA_HIN_USER_DEFINED_1,appIHead | UNA_ENG_TOKEN_RETURN,0);
	/* オフセット、要素番号更新 */
	offset += kaiCnt;
	idx++;

	/* 復改と後続セグメントの間の空白カウント */
	spcCnt = 0;
	for (pos += kaiCnt;;pos++){ /* forever */
		mojiKind = (eh->engMKTblImg)[(morph->start)[pos]];
		if(mojiKind != 11){	/* 空白でなくなるまで */
			break;
		}
		spcCnt ++;
	}

	if (spcCnt != 0) {	/* 復改と後続セグメントとの間に空白あり */
		(void)SubMorphBufSet(morphBuf,idx,&(morph->start)[offset],
					   (short)spcCnt,UNA_HIN_USER_DEFINED_1,
					   appIHead | UNA_ENG_TOKEN_SPACE,0);
		/* オフセット、要素番号更新 */
		offset += spcCnt;
		idx++;
	}

	/* 後続セグメント */
	(void)SubMorphBufSet(morphBuf,idx,&morph->start[offset],
			(short)(morph->length - pos),UNA_HIN_USER_DEFINED_1,
			appIHead | UNA_ENG_TOKEN_NORMAL,0);
	idx++;

	/* 要素数 */
	*morphNum = idx;

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  SubMorphBufSet
//
// ABSTRACT:    形態素バッファに値をセットする
//
// FUNCTION:
//		idx で示された形態素バッファの1要素に値をセットする。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  idx は、上位関数にて既にチェック済みであるのでオーバーフローはしない
//
static void SubMorphBufSet(
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int	 idx,					/* 下位形態素配列の要素番号*/
	unaCharT *start,			/* 表記(unaCharT へのポインタ) */
	sshortT  length,			/* 表記の長さ(文字数) */
	ushortT  hinshi,			/* 形態素品詞番号 */
	uintT   appI,				/* アプリケーションインデックス情報 */
	uintT   subI				/* 下位構造情報 */
)
{
	morphBuf[idx].start		= start;
	morphBuf[idx].length	= length;
	morphBuf[idx].hinshi	= hinshi;
	morphBuf[idx].appI		= appI;
	morphBuf[idx].subI		= subI;

	return;
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
