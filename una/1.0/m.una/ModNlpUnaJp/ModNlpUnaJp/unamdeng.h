//
// unamdeng.h -
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

#ifndef UNAMDENG_H
#define UNAMDENG_H

//--------------------------------------------------------------------------
// include required header file
//--------------------------------------------------------------------------

#include "../UnaBase/UnaBase/una.h"			/* UNAグローバルなヘッダファイル */
#include "../UnaBase/UnaBase/unamorph.h"	/* 形態素解析メインモジュール用ヘッダファイル */
#include "../UnaBase/UnaBase/unaapinf.h"	/* アプリ情報取得モジュール用ヘッダファイル */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* 英語トークンタイプ(andが取れる様、ユニークなbitを立てた値になっている) */
#define UNA_ENG_TOKEN_NORMAL (1)      /* 通常の英語トークン */
#define UNA_ENG_TOKEN_HYPHEN (1 << 1) /* 行末ハイフン処理された英語トークン*/
#define UNA_ENG_TOKEN_ARABIC (1 << 2) /* アラビア数字 */
#define UNA_ENG_TOKEN_SYMBOL (1 << 3) /* 記号 */
#define UNA_ENG_TOKEN_SPACE  (1 << 4) /* 空白 */
#define UNA_ENG_TOKEN_RETURN (1 << 5) /* 改行 */
#define UNA_ENG_TOKEN_INITIAL (1 << 6) /* 頭文字 */

//--------------------------------------------------------------------------
// TAG:	  unaMDEngHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  英語トークン検出モジュールをマルチスレッドセーフで実行するための
//	  ハンドラ
//
//	  なお、英語トークン用アプリケーション情報は行末ハイフントークンの場合
//	  にのみ使用され、まとめあげられた形態素が入る。まとめあげられた形態素は
//	  元々の形態素の最大長 UNA_HYOKI_LEN_MAX 以下であるのでこの大きさで十分。
//	  但し、SPARCのためのパディング用に2バイト多めにとってある。
//
typedef struct unaMDEngHandleT{	// 英語トークン検出モジュール用ハンドル
	const char *engMKTblImg;	/* 英語トークン用文字種別テーブルのイメージ*/
	const ushortT *engCTblImg;	/* English token cost table on engmk.tbl */
	char appInfo[sizeof(int) + (UNA_HYOKI_LEN_MAX) * sizeof(unaCharT) + 2];
								/* 英語トークン用アプリケーション情報 */
}unaMDEngHandleT;

//--------------------------------------------------------------------------
// Prototype declaration of a function, and a macro function
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaMDicEnglish_init(unaMDEngHandleT *eh,const char *engMKTblImg);
/* 終了処理 */
int unaMDicEnglish_term(unaMDEngHandleT *eh);
/* 英語トークン検出関数 */
int unaMDicEnglish_searchMorph(unaMorphHandleT *mh,int txtPos,
		int dicNum,int *morCount,void *eHandle);
/* 英語トークン用アプリ情報取得 */
int unaMDicEnglish_appInfoGet(const unaMorphT *morph,unaAppInfoT **appInf,
		unaMDEngHandleT *eh);
/* 英語トークン用下位構造取得 */
int unaMDicEnglish_getSubMorph(const unaMorphT *morph,unaMorphT *morphBuf,
		int *morphNum,const int morphBufSize,const unaMDEngHandleT *eh);

#endif /* end of UNAMDENG_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
