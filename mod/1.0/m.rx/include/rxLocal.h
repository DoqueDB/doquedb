/*
 * rxLocal.h --- ローカルな構造体定義
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
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
#ifndef _rxlocal_h
#define _rxlocal_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */
#include "rxPmm.h"
#include "rxdfa.h"

EXTERN_BEGIN

typedef struct _rxNode	rxNode ;

struct _rxHandle {
    int		pnum ;		/* パターン数 */
    rxNode	*rnode ;	/* 拡張論理演算木の根ノード */
    int		hnum ;		/* 先頭条件数 */
    int		rnum ;		/* 正規表現数 */
    int		*rids ;		/* 正規表現のパターンID */
    rxDfa	**dfa ;		/* 正規表現用オートマトンの配列 */
    rxPmmHandle	*pmm ;		/* 非正規表現用オートマトン */
    unsigned	*ed ;		/* 末尾条件のビットマップ */
	unsigned	*bitmap1;	/* rxStep, rxWalkで使用するビットマップ */
	unsigned	*bitmap2;	/* rxStep, rxWalkで使用するビットマップ */
	unsigned	*bitmap3;	/* rxStep, rxWalkで使用するビットマップ */
	unsigned	*bitmap4;	/* rxStep, rxWalkで使用するビットマップ */
	int		error_code ;		/* エラーコード */
} ;

struct _rxNode {
    int	 	optype ;	/* 演算子の種類 */
    union {
	struct {
	    rxNode*	left ;
	    rxNode*	right ;
	}       child ;		/* RX_OP_AND, OR, NOT */
	ModUnicodeChar*   expr ;		/* RX_OP_EXPR */
	void*   dfa ;		/* RX_OP_DFA */
	int	pid ;		/* RX_OP_PID */
	struct {
	    unsigned	*buf ;
	    int		num ;
        }	bitmap ;	/* RX_OP_BMP */
    }		data ;
} ;

EXTERN_END

#endif
/* _rxlocal_h */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
