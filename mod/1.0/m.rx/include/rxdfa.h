/*
 * rxdfa.h --- DFAの生成処理
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef RXDFA_H
#define RXDFA_H

#include "rx.h"
#include "rxchar.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */

/*
 * 状態遷移表の欄
 */
typedef struct _rxTrans {
    int		state;		/* 状態番号			*/
    rxChar	value;		/* 入力アルファベット		*/
    int		next;		/* 次の状態			*/
    int		brbit;		/* \(\)グルーピングマークビット */
} rxTrans;

/*
 * 状態遷移インデックステーブルの欄
 */ 
typedef struct _rxState {
    int		index;		/* 遷移表インデックス値		*/
    int		lastindex;	/* 次状態インデックス値		*/
    int		acceptable;	/* 受理可能状態フラグ		*/
} rxState;

/*
 * DFA構造体
 */
typedef struct _rxDfa {
    rxTrans*	trans;		/* 状態遷移表			*/
    int		tlength;	/* 遷移表サイズ 		*/
    int		talloc;		/* 遷移表領域サイズ		*/

    rxCharSet*	ModUnicodeCharsets;	/* 文字セット表			*/
    int		cslength;	/* 文字セットサイズ		*/

    rxState*	states;		/* 状態遷移インデックス表	*/
    int		state_max;	/* 状態番号最大値		*/

    int		parens;		/* \(,\)の数			*/
} rxDfa;

EXTERN_FUNC(rxDfa*, rxDfaCompile, (ModUnicodeChar* pattern));
EXTERN_FUNC(void, rxDfaFree, (rxDfa* dfa));
EXTERN_FUNC(int, rxDfaMatch, (rxDfa* dfa, ModUnicodeChar* text, int length,
			      ModUnicodeChar** edloc, rxBrackets* brackets, int mode));
EXTERN_FUNC(int, rxDfaExec, (rxDfa* dfa, ModUnicodeChar* text, int length,
			     ModUnicodeChar** stloc, ModUnicodeChar** edloc,
			     rxBrackets* brackets, int mode));

#endif /* RXDFA_H */

/*
 *	Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
