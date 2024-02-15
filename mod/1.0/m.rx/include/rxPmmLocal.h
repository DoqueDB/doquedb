/*
 * rxPmmLocal.h --- 複数検索語の照合関数用ヘッダーファイル
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2000, 2003, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _rxPmmLocal_h
#define _rxPmmLocal_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


#define RX_ALPHABET_SIZE	256	/* アルファベット数 */
					/* バイト単位に照合するので */
					/* 256 となる */

typedef unsigned short	(rxStateArray[RX_ALPHABET_SIZE]) ;
					/* 状態は short とする */

#define RX_MAX_STATE	65533		/* 状態の最大数 */
					/* 状態を short で表現している */
					/* が FAIL、２／３バイト文字の */
					/* ための中間状態があるので、 */
					/* 65536 - 3 となる */

#define RX_FAIL_STATE	((unsigned short)(-1))	/* 失敗状態 */
#define RX_INIT_STATE	((unsigned short)0)	/* 初期状態 */

#ifdef RX_UNICODE
/* Unicodeの場合は中間状態は1つ */
#define RX_EXTRA_STATE	(2)		/* 初期状態 + 中間状態 */

#else // RX_UNICODE

#ifdef RX_MSKANJI
#define RX_EXTRA_STATE	(2)		/* 初期状態 + 中間状態 */
#else
#define RX_EXTRA_STATE	(3)		/* 初期状態 + 中間状態（２個）*/
#endif

#endif // RX_UNICODE

typedef struct _rxOutNode rxOutNode ;
typedef struct _rxPmmQueue rxPmmQueue ;

struct _rxPmmHandle {
    int				patNum ;		/* パターンの個数 */
    int*			patLens ;		/* 各パターンの長さ */
    int				stateNum ;		/* 状態数 */
    rxStateArray*	gotoTable ;		/* goto 表 */
    rxOutNode**		outTable ;		/* out 表 */
    int				lastState ;		/* 遷移表の状態 */
    ModUnicodeChar*	text ;			/* テキスト（照合位置） */
    int				len ;			/* テキスト長（残りの長さ) */
    int				charlen ;		/* 変換後のテキスト長（残りの長さ) */
} ;

/* out表 */
struct _rxOutNode {
    int		id ;			/* パターンID */
    rxOutNode*	next ;			/* リンク */
} ;

/* queue */
struct _rxPmmQueue {
    int		sz ;			/* サイズ */
    int		*buf ;			/* データ格納場所 */
    int		nm ;			/* データ数 */
    int		st ;			/* データ開始位置 */
    int		ed ;			/* データ終了位置 */
} ;

#endif 
/* _rxPmmLocal_h */

/*
 *	Copyright (c) 1996, 2000, 2003, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
