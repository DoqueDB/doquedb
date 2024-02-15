/*
 * rxParse.h --- 拡張正規表現パーザ用ヘッダーファイル
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
#ifndef _rxParse_h
#define _rxParse_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


/* パターンのタイプ */
#define RX_REGX		(1)		/* 正規表現 */
#define RX_HEAD		(2)		/* 先頭条件 */
#define RX_TAIL		(4)		/* 末尾条件 */

/* ノードタイプ */
#define RX_OP_OR	(1)		/* OR */
#define RX_OP_AND	(2)		/* AND */
#define RX_OP_ANDNOT	(3)		/* ANDNOT */
#define RX_OP_PID	(4)		/* 末端ノード */
#define RX_OP_BMP	(5)		/* OR を縮退させたノード */


EXTERN_BEGIN

EXTERN_FUNC( int,	rxParse,	( ModUnicodeChar*, ModUnicodeChar***, int**, 
					  rxNode** ) ) ;
EXTERN_FUNC( void,	rxNodeFree,	( rxNode* ) ) ;
#ifdef DEBUG
EXTERN_FUNC( void,	rxNodePrint,	( rxNode*, FILE* ) ) ;
#endif

EXTERN_END

#endif
/* _rxParse_h */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
