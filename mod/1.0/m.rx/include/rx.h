/*
 * rx.c --- アプリケーション用ヘッダーファイル
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
#ifndef _rx_h
#define _rx_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#include "rxModUnicodeChar.h" 


#if __cplusplus
/* C++ */
#define EXTERN_BEGIN	extern "C" {
#define	EXTERN_END	}
#define EXTERN_FUNC( rtn, name, args ) extern rtn name args
#define STATIC_FUNC( rtn, name, args ) static rtn name args
#ifndef _DDD_
#define _DDD_		...
#endif
#define _VOID_		void
#define _CONST_		const
#elif defined(__STDC__)
/* ANSI C */
#define EXTERN_BEGIN
#define	EXTERN_END
#define EXTERN_FUNC( rtn, name, args ) extern rtn name args
#define STATIC_FUNC( rtn, name, args ) static rtn name args
#ifndef _DDD_
#define _DDD_		...
#endif
#define _VOID_		void
#define _CONST_
#else
/* K&R C */
#define EXTERN_BEGIN
#define	EXTERN_END
#define EXTERN_FUNC( rtn, name, args ) extern rtn name()
#define STATIC_FUNC( rtn, name, args ) static rtn name()
#ifndef _DDD_
#define _DDD_
#endif
#define	_VOID_
#define _CONST_
#endif

#define RX_OK		(0)
#define RX_ERR		(-1)

#define RX_TRUE		(1)
#define RX_FALSE	(!(RX_TRUE))

#ifdef RX_UNICODE

#define RX_AND		0x0026	/* '&'	 論理積（AND） */
#define RX_OR		0x007c	/* '|'	 論理和（OR） */
#define RX_ANDNOT	0x002d	/* '-'	 論理差（ANDNOT） */
#define RX_LPAREN	0x0028	/* '('	 優先順位指定の開始 */
#define RX_RPAREN	0x0029	/* ')'	 優先順位指定の終了 */
#define RX_ESCAPE	0x005c	/* '\\'	 エスケープ文字 */
#define RX_PERIOD	0x002e	/* '.'	 任意文字 */
#define RX_HAT		0x005e	/* '^'	 先頭条件 */
#define RX_DOLLAR	0x0024	/* '$'	 末尾条件 */
#define RX_STAR		0x002a	/* '*'	 0 回以上の繰り返し */
#define RX_PLUS		0x002b	/* '+'	 1 回以上の繰り返し */
#define RX_QUESTION	0x003f	/* '?'	 0 または 1 回の繰り返し */
#define RX_BRA		0x005b	/* '['	 文字範囲指定の開始 */
#define RX_CKET		0x005d	/* ']'	 文字範囲指定の終了 */

#define RX_DIG_ZERO	0x0030	/* '0'	数字の0 */
#define RX_DIG_ONE	0x0031	/* '1'	数字の1 */
#define RX_DIG_NINE	0x0039	/* '9'	数字の9 */
#define RX_MINUS	0x002d	/*
				 * RX_ANDNOTと同じであるが、
				 * 意味が違うので作っておく 
				 */

#else  /* RX_UNICODE */

#define RX_AND		'&'	/* 論理積（AND） */
#define RX_OR		'|'	/* 論理和（OR） */
#define RX_ANDNOT	'-'	/* 論理差（ANDNOT） */
#define RX_LPAREN	'('	/* 優先順位指定の開始 */
#define RX_RPAREN	')'	/* 優先順位指定の終了 */
#define RX_ESCAPE	'\\'	/* エスケープ文字 */
#define RX_PERIOD	'.'	/* 任意文字 */
#define RX_HAT		'^'	/* 先頭条件 */
#define RX_DOLLAR	'$'	/* 末尾条件 */
#define RX_STAR		'*'	/* 0 回以上の繰り返し */
#define RX_PLUS		'+'	/* 1 回以上の繰り返し */
#define RX_QUESTION	'?'	/* 0 または 1 回の繰り返し */
#define RX_BRA		'['	/* 文字範囲指定の開始 */
#define RX_CKET		']'	/* 文字範囲指定の終了 */


#endif  /* RX_UNICODE */


#define RX_NULL_TERMINATE	(-1)	/* NULL ターミネイトまで照合 */

#define RX_LONGEST	(0)	/* 正規表現を最長一致で照合するモード */
#define RX_SHORTEST	(1)	/* 正規表現を最短一致で照合するモード */
#define RX_SKIPEMPTY	(2)	/* 前回に空文字列と照合した場合、同一  */
				/* 空文字列との照合を回避するモード */

#define RX_ERR_MEMALLOC	(1)		/* メモリー確保エラー */
#define RX_ERR_INVALIDEXPR	(2)	/* 不正な拡張正義表現 */
#define RX_ERR_COMPLEXEXPR	(3)	/* 複雑すぎる拡張正義表現 */
#define RX_ERR_INVALIDHANDLE	(4)	/* 不正なハンドル */


EXTERN_BEGIN

typedef struct _rxHandle rxHandle ;

/* 副表現とは \(...\) で指定される表現のこと */
/* 部分パターンとは論理演算の対象となる正規表現のこと */

/* 副表現情報（正規表現用） */
typedef struct _rxBrackets {
    int		nbra;		/* 副表現の個数 */
    ModUnicodeChar*	braslist[9];	/* 副表現の開始位置 */
    ModUnicodeChar*	braelist[9];	/* 副表現の末尾のつぎの位置 */
} rxBrackets;

/* 照合位置情報 */
typedef struct _rxMatchee {
    ModUnicodeChar*	stloc ;		/* 照合開始位置 */
    ModUnicodeChar*	edloc ;		/* 照合末尾のつぎの位置 */
    int		expid ;		/* 照合した部分パターンID: rxWalk */
				/* rxStep,rxAdvance では不定 */
    int		nbras ;		/* bras に配置された要素数 */
    rxBrackets*	bras ;		/* 副表現の配列: rxStep,rxAdvance */
				/* 照合した部分パターンにおける副表現: rxWalk */
} rxMatchee ;

extern int		rxErrorCode ;	/* エラーコード(MT-Unsafe) */
#ifdef DEBUG
extern int		rxDebugMode ;	/* デバッグモード */
#endif

EXTERN_FUNC( rxHandle*,	rxCompile,	( _CONST_ ModUnicodeChar* pattern ) );
EXTERN_FUNC( int,	rxStep,		( rxHandle*, int, _CONST_ ModUnicodeChar*, 
					  int, rxMatchee* ) ) ;
EXTERN_FUNC( int,	rxAdvance,	( rxHandle*, int, _CONST_ ModUnicodeChar*,
					  int, rxMatchee* ) ) ;
EXTERN_FUNC( int,	rxWalk,		( rxHandle*, int, _CONST_ ModUnicodeChar*,
					  int, rxMatchee** ) ) ;
EXTERN_FUNC( void,	rxFree,		( rxHandle* ) ) ;
EXTERN_FUNC( void,	rxMatcheeFree,	( rxMatchee* ) ) ;

EXTERN_END

#endif
/* _rx_h */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
