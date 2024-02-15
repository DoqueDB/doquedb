/*
 * rxPmm.h --- 複数検索語の照合関数用ヘッダーファイル
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
#ifndef _rxPmm_h
#define _rxPmm_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


EXTERN_BEGIN

typedef struct _rxPmmHandle	rxPmmHandle ;
typedef struct _rxPmmLoc	rxPmmLoc ;

/* 照合位置情報 */
struct _rxPmmLoc {
    ModUnicodeChar	*st ;		/* 照合開始位置 */
    ModUnicodeChar	*ed ;		/* 照合末尾のつぎの位置 */
    int		id ;		/* 照合したパターンID */
    rxPmmLoc	*next ;		/* 次の照合位置情報へのポインタ */
} ;


EXTERN_FUNC( rxPmmHandle*,	rxPmmCompile,( ModUnicodeChar**, int )) ;
EXTERN_FUNC( void,		rxPmmSetText,( rxPmmHandle*, ModUnicodeChar*, int )) ;
EXTERN_FUNC( int,		rxPmmStep,( rxPmmHandle*, rxPmmLoc** )) ;
#ifdef RX_PMMADVANCE
EXTERN_FUNC( int,		rxPmmAdvance,( rxPmmHandle*, rxPmmLoc** )) ;
#endif
EXTERN_FUNC( void,		rxPmmFree,( rxPmmHandle* )) ;
EXTERN_FUNC( void,		rxPmmLocFree,( rxPmmLoc* )) ;
#ifdef RX_PMMLOCSORT
EXTERN_FUNC( void,		rxPmmLocSort,( rxPmmLoc** )) ;
#endif

EXTERN_END

#endif
/* _rxPmm_h */

/*
 *	Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
