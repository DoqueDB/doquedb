/*
 * rx.c --- API 関数の定義
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rx.h"
#include "rxDefs.h"
#include "rxPmm.h"
#include "rxdfa.h"
#include "rxParse.h"
#include "rxModUnicodeOperations.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#include "rxModUnicodeChar.h"

/* rxWalk のための照合位置情報の一時格納領域 */
typedef struct {
	rxMatchee	*rx_matchee ;		/* 照合位置情報の領域 */
	int		rx_matchee_num ;	/* 照合数 */
	int		rx_matchee_size ;	/* 領域サイズ */
} rxWalkInfo;


STATIC_FUNC( unsigned*,	rxBitmapAlloc,	( int,unsigned )) ;
STATIC_FUNC( unsigned*, rxBitmapSet,  ( unsigned*,int,unsigned )) ;
STATIC_FUNC( int, rxDfaAdvance, ( rxDfa*,int,ModUnicodeChar*,int,ModUnicodeChar**,rxBrackets*,int )) ;
STATIC_FUNC( int, rxDfaStep, ( rxDfa*,int,ModUnicodeChar*,int,ModUnicodeChar**,ModUnicodeChar**,rxBrackets*,int )) ;
STATIC_FUNC( void,	rxMatcheeInit,	( rxWalkInfo* )) ;
STATIC_FUNC( int,	rxMatcheeAppend,( rxWalkInfo*,int,ModUnicodeChar*,ModUnicodeChar*,int,rxBrackets* ));

STATIC_FUNC( int,	rxNodeMod,	( rxNode*,int )) ;
STATIC_FUNC( int,	rxNodeCheck,	( rxNode*,unsigned*,int )) ;
STATIC_FUNC( void,	rxBitmapNotOn,	( unsigned*,int,rxNode*,int )) ;

#define RX_MATCHEE_ARRAY_INC	16

int			rxErrorCode = 0 ;	/* エラーコード */
#ifdef DEBUG
int			rxDebugMode = 0 ;	/* デバッグモード */
#endif


#if 0	/* 正式版では不要 */
	/* テストではバイト数を与えられるため */
int BYTES_2_NUMBER_OF_UNICODE_CHARACTOR(int num) 
{
	if(num < 0)
		return num;
	else
		return num/sizeof(ModUnicodeChar);
}

int NUMBER_OF_UNICODE_CHARACTOR_2_BYTES(int num)
{
	if(num < 0)
		return num;
	else
		return num * sizeof(ModUnicodeChar);
}
#endif // TSET_SASA

/*======================================================================*
 * rxCompile		拡張正規表現のコンパイル
 *----------------------------------------------------------------------*
 * 【戻り値】
 * ハンドル:		処理成功
 * NULL:		処理失敗
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・パターンは NULL ターミネイトしていること
 *  ・ハンドルは rxFree() で解放すること
 *======================================================================*/
rxHandle*
rxCompile( pattern )
    _CONST_ ModUnicodeChar	*pattern ;	/* 拡張正規表現 */
{
    rxHandle	*ret = NULL ;
    rxNode	*rnode = NULL ;
    int		*ptype = NULL ;
    int		pnum, bnum, rnum, rtmp = 0, n ;
    ModUnicodeChar	**pats = NULL ;
    unsigned	*ed = NULL;

    pnum = rxParse( pattern, &pats, &ptype, &rnode ) ;
    if ( pnum == RX_ERR )
	return NULL ;
#ifdef DEBUG
    if ( rxDebugMode > 0 ) {
	fprintf( stderr, "* " ) ;
	rxNodePrint( rnode, stderr ) ;
	fprintf( stderr, "\n* " ) ;
	for ( n = 0 ; n < pnum ; n ++ )
	    fprintf( stderr, "%s,%d@%d ", pats[n], ptype[n], n ) ;
	fprintf( stderr, "\n" ) ;
    }
#endif
    /* 正規表現および先頭条件である非正規表現の個数を数える */
    for ( n = 0, rnum = 0 ; n < pnum ; n ++ ) {
	if ( ptype[n]&(RX_REGX|RX_HEAD) )
	    rnum ++ ;
    }
    /* OR 条件をまとめる */
    bnum = pnum/32 + 1 ;
    if ( rxNodeMod( rnode, bnum ) == RX_ERR )
	goto error1 ;

    ret = (rxHandle*)MEMALLOC( sizeof(rxHandle) ) ;
    if ( ret == NULL ) {
	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	goto error1 ;
    }
    ret->rnode = rnode ;
    ret->pmm = NULL ;
    ret->dfa = (rxDfa**)MEMALLOC( rnum*sizeof(rxDfa*) ) ;
    ret->rids = (int*)MEMALLOC( rnum*sizeof(int) ) ;
    if ( ret->dfa == NULL || ret->rids == NULL ) {
	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	goto error ;
    }
    memset( ret->dfa, 0, rnum*sizeof(rxDfa*) ) ;
    ret->pnum = pnum ;
    ret->rnum = rnum ;
    ret->ed = NULL ;
    ret->bitmap1 = (unsigned*)MEMALLOC( bnum*sizeof(unsigned) ) ;
    ret->bitmap2 = (unsigned*)MEMALLOC( bnum*sizeof(unsigned) ) ;
    ret->bitmap3 = (unsigned*)MEMALLOC( bnum*sizeof(unsigned) ) ;
    ret->bitmap4 = (unsigned*)MEMALLOC( bnum*sizeof(unsigned) ) ;

    /* 先頭条件パターンのコンパイル、末尾条件の記録 */
    for ( n = 0 ; n < pnum ; n ++ ) {
	/* 先頭条件パターンのコンパイル */
	if ( ptype[n]&RX_HEAD ) {
	    ; assert( rtmp < rnum ) ;

#ifdef RX_UNICODE
	    if ( ModUnicodeCharLength( pats[n] ) == 0 ) {
#else /* RX_UNICODE */
	    if ( strlen( pats[n] ) == 0 ) {
#endif /* RX_UNICODE */
		/* 先頭条件で空文字列は末尾条件でなければならない */
		; assert( ptype[n]&RX_TAIL ) ;
		ret->dfa[rtmp] = NULL ;
	    } else {
		ret->dfa[rtmp] = rxDfaCompile( pats[n] ) ;
		if ( ret->dfa[rtmp] == NULL )
		    goto error ;
	    }
	    ret->rids[rtmp] = n ;
	    rtmp ++ ;

#ifdef RX_UNICODE
	    MEMFREE( pats[n], ModUnicodeCharLength(pats[n]) + 1 ) ;
#else /* RX_UNICODE */
	    MEMFREE( pats[n], strlen(pats[n]) + 1 ) ;
#endif /* RX_UNICODE */
	    pats[n] = NULL ;		/* 非正規表現用オートマトンで */
					/* は不要なのでクリアする */
	}
	/* 末尾条件の検査 */
	if ( ptype[n]&RX_TAIL ) {
	    if ( ed == NULL ) {
		ret->ed = ed = rxBitmapAlloc( bnum, 0 ) ;
		if ( ed == NULL )
		    goto error ;
	    }
	    ; assert( ed ) ;
	    BIT_ON( ed, n ) ;
	}
    }
    ret->hnum = rtmp ;			/* 先頭条件数の記録 */

    /* 先頭条件でない正規表現のコンパイル */
    for ( n = 0 ; n < pnum ; n ++ ) {
	if ( !(ptype[n]&RX_HEAD) && ptype[n]&RX_REGX ) {
	    ; assert( rtmp < rnum ) ;
	    ret->dfa[rtmp] = rxDfaCompile( pats[n] ) ;
	    if ( ret->dfa[rtmp] == NULL )
		goto error ;
	    ret->rids[rtmp] = n ;
	    rtmp ++ ;

#ifdef RX_UNICODE
	    MEMFREE( pats[n], ModUnicodeCharLength(pats[n]) + 1 ) ;
#else /* RX_UNICODE */
	    MEMFREE( pats[n], strlen(pats[n]) + 1 ) ;
#endif /* RX_UNICODE */
	    pats[n] = NULL ;		/* 同上 */
	}
    }
    ; assert( rtmp == rnum ) ;

    /* 非正規表現（単純)文字列を全て照合するオートマトンの作成 */
    if ( rnum < pnum ) {
	ret->pmm = rxPmmCompile( pats, pnum ) ;
	if ( ret->pmm == NULL )
	    goto error ;
    }

    /* パターンを破棄する */
    for ( n = 0 ; n < pnum ; n ++ ) {
	if ( pats[n] ) {
#ifdef RX_UNICODE
	    MEMFREE( pats[n], ModUnicodeCharLength(pats[n]) + 1 ) ;
#else /* RX_UNICODE */
	    MEMFREE( pats[n], strlen(pats[n]) + 1 ) ;
#endif /* RX_UNICODE */
	}
    }
    MEMFREE( pats, pnum*sizeof(ModUnicodeChar*) ) ;
    MEMFREE( ptype, pnum*sizeof(int) ) ;

    return ret ;

 error1:
    if ( rnode )
	rxNodeFree( rnode ) ;
 error:
    for ( n = 0 ; n < pnum ; n ++ ) {
	if ( pats[n] ) {
#ifdef RX_UNICODE
	    MEMFREE( pats[n], ModUnicodeCharLength(pats[n]) + 1 ) ;
#else /* RX_UNICODE */
	    MEMFREE( pats[n], strlen(pats[n]) + 1 ) ;
#endif /* RX_UNICODE */
	}
    }
    MEMFREE( pats, pnum*sizeof(ModUnicodeChar*) ) ;
    MEMFREE( ptype, pnum*sizeof(int) ) ;
    rxFree( ret ) ;
    return NULL ;

}


/*
 * bnum 個の要素からなるビットパターン用領域を確保する
 */
static unsigned*
rxBitmapAlloc( bnum, ival )
    int		bnum ;		/* 要素数 */
    unsigned	ival ;		/* 初期値 */
{
    unsigned	*ret ;

    ret = (unsigned*)MEMALLOC( bnum*sizeof(unsigned) ) ;
    if ( ret == NULL ) {
	RX_ERROR_SET( RX_ERR_MEMALLOC ) ;
	return NULL ;
    }
    return rxBitmapSet( ret, bnum, ival );
}


/*
 * bnum 個の要素からなるビットパターン用領域をクリアする
 */
static unsigned*
rxBitmapSet( bitmap, bnum, ival )
    unsigned	*bitmap ;	/* ビットマップ */
    int		bnum ;		/* 要素数 */
    unsigned	ival ;		/* 初期値 */
{
    int		b ;

    for ( b = 0 ; b < bnum ; b ++ )
	bitmap[b] = ival ;
    return bitmap ;
}


/*======================================================================*
 * rxStep		拡張正規表現による照合
 *----------------------------------------------------------------------*
 * 【戻り値】
 * RX_TRUE:		照合成功
 * RX_FALSE:		照合失敗
 * RX_ERR:		エラー
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・照合対象の、拡張正規表現と照合する部分を見つける
 *  ・mode には以下の値（の OR）を渡す
 *	RX_LONGEST:	正規表現を最長一致で照合（デフォルト）
 *	RX_SHORTEST:	正規表現を最短一致で照合
 *	RX_SKIPEMPTY:	正規表現の照合で、同一空文字列と照合しなくする
 *	RX_ADVANCE:	照合対象の先頭から照合する（内部使用のみ）
 *  ・textlen が 0 の場合、照合対象が NULL ターミネイトするまで処理
 *  ・照合位置情報は rxMatcheeFree() で解放すること
 *======================================================================*/
int
rxStep( rx, mode, text, textlen, matchee )
    rxHandle	*rx ;		/* ハンドル */
    int		mode ;		/* モード */
    _CONST_ ModUnicodeChar *text ;	/* 照合対象 */
    int		textlen ;	/* 照合対象長さ */
    rxMatchee	*matchee ;	/* 照合位置情報 */
{
    rxPmmHandle*	pmm ;
    rxDfa**	dfa ;
    rxNode	*rnode ;
    int		ret = RX_ERR ;
    int		n, id, x, b ;
    int		pnum, bnum, rnum, hnum ;
    unsigned	*bitmap ;	/* 出現したパターンIDを記録 */
    unsigned	*bitmap2 ;	/* 出現する可能性のあるIDを記録: */
				/* はじめに全てのビットをONし、出現  */
				/* していないことが分ったらOFFする */
    unsigned	*to_ed ;
    ModUnicodeChar	*st, *ed, *stloc = NULL, *edloc ;
#ifdef DEBUG
    ModUnicodeChar	*str;
#endif
    rxBrackets	*bras = NULL ;

	/*
	 * 照合対象の長さとしてバイト数が与えられるがUnicodeでは文字数に変換して
	 * 使用する。
	 */
	int UniTextlen;
#if 0
	/* テストではバイト数を与えられるため */
	/* 正式版は文字数を受け取る ↓(UniTextlen = textlen;) をとおる*/
	UniTextlen = BYTES_2_NUMBER_OF_UNICODE_CHARACTOR(textlen);
#else
	UniTextlen = textlen;
#endif
	


    if ( rx == NULL ) {
	RX_ERROR_SET(RX_ERR_INVALIDHANDLE) ;
	return RX_ERR ;
    }

    dfa = rx->dfa ;
    rnode = rx->rnode ;
    rnum = rx->rnum ;
    pnum = rx->pnum ;
    hnum = rx->hnum ;
    bnum = pnum/32 + 1 ;
    to_ed = rx->ed ;
    edloc = text ;

    bitmap = rxBitmapSet( rx->bitmap1, bnum, 0 ) ;
    bitmap2 = rxBitmapSet( rx->bitmap2, bnum, ~0 ) ;
    rxBitmapNotOn( bitmap, bnum, rnode, 0 ) ;
    for ( b = 0 ; b < bnum ; b ++ )
	bitmap2[b] = ~(bitmap[b]) ;

    bras = (rxBrackets*)MEMALLOC( pnum*sizeof(rxBrackets) ) ;
    if ( bras == NULL ) {
	RX_ERROR_SET( RX_ERR_MEMALLOC ) ;
	goto error ;
    }
    (void)memset( bras, 0, pnum*sizeof(rxBrackets) ) ;

    /* 先頭条件をパターンごとに照合する */
    for ( n = 0 ; n < hnum ; n ++ ) {
	id = rx->rids[n] ;
	switch ( rxDfaAdvance( dfa[n], BIT_CHECK(to_ed,id), text, UniTextlen,
			       &ed, &bras[id], mode&RX_DFA_MODEMASK ) ) {
	case RX_TRUE:
	    BIT_ON( bitmap, id ) ;
	    if ( BIT_CHECK( bitmap2, id ) ) {	/* あってはいけない場 */
						/* 合は除く */
		if ( stloc == NULL )
		    stloc = text ;
		if ( edloc < ed )
		    edloc = ed ;
	    }
	    BIT_ON( bitmap2, id ) ;
	    break ;
	case RX_FALSE:
	    BIT_OFF( bitmap, id ) ;
	    BIT_OFF( bitmap2, id ) ;
	    break ;
#ifndef NDEBUG
	default:
	    ; assert( 0 ) ;
#endif
	}
	if ( rxNodeCheck( rnode, bitmap, bnum ) )
	    goto match ;
	if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
	    goto fail ;	/* 残りのパターン全てがあっても照合しない */
    } /* for 部分パターン */

    /* 非正規表現の処理: 複数を同時に照合する */
    /* ここで照合するパターンは先頭条件ではない */
    if ( pmm = rx->pmm ) {
	rxPmmLoc	*head = NULL, *tmp ;
	unsigned	*btmp ;		/* ここで照合すべきで照合しな */
					/* いものを見つけるため */
	unsigned	*btmp2 ;	/* bitmap2 の保存 */

	btmp = rxBitmapSet( rx->bitmap3, bnum, 0 ) ;
	btmp2 = rxBitmapSet( rx->bitmap4, bnum, 0 ) ;
	for ( n = 0 ; n < rnum ; n ++ )		/* ここで照合しなくて  */
	    BIT_ON( btmp, rx->rids[n] ) ;	/* もよいものをONする */
	for ( b = 0 ; b < bnum ; b ++ )
	    btmp2[b] = bitmap2[b] ;

	rxPmmSetText( pmm, text, UniTextlen ) ;
	/* 継続して条件を満たすようになるか調べる */
	for ( x = rxPmmStep(pmm,&head) ; x > 0 ; x = rxPmmStep(pmm,&head) ) {
	    for ( tmp = head ; tmp ; tmp = tmp->next )  {
		id = tmp->id ;
		st = tmp->st ;
		ed = tmp->ed ;
#ifdef DEBUG
		if ( rxDebugMode > 0 ) {
		    fprintf( stderr, "FOUND[%d]:%d,", n, id ) ;
		    for ( str = st ; str < ed ; str ++ )
			putc( *str, stderr ) ;
		    putc( '\n', stderr ) ;
		}
#endif

#ifdef RX_UNICODE
		if ( BIT_CHECK( to_ed, id ) && 
		     *ed != '\0' && (ed-text)!=UniTextlen)
		    continue ;		/* 末尾条件であるが、末尾では */
					/* ない場合、つぎの照合を探す */
#else /* RX_UNICODE */
		/*
		 * これではed-textは文字数であり、textlenはバイト数であるため、
		 * unmatchになってしまう。
		 *
		 * とりあえず、ed-textをバイト数に変換して対応。
		 *	→要検討
		 *	textlen(Compileより渡される)を文字数にしたほうが自然？
		 */
		if ( BIT_CHECK( to_ed, id ) && 
		     *ed != '\0' && (ed-text)!=textlen )
		    continue ;		/* 末尾条件であるが、末尾では */
					/* ない場合、つぎの照合を探す */
#endif /* RX_UNICODE */

		BIT_ON( btmp, id ) ;
		BIT_ON( bitmap, id ) ;
		if ( BIT_CHECK( btmp2, id ) ) {
		    if ( stloc == NULL || stloc > st )
			stloc = st ;
		    if ( edloc < ed )
			edloc = ed ;
		}
		BIT_ON( bitmap2, id ) ;
		if ( rxNodeCheck( rnode, bitmap, bnum ) )
		    if ( !(mode&RX_ADVANCE) || stloc == text ) {
			; assert( head && btmp && btmp2 ) ;
			rxPmmLocFree( head ) ;
			btmp = btmp2 = NULL ;
			goto match ;
		    }
		if ( !rxNodeCheck( rnode, bitmap2, bnum ) ) {
		    ; assert( head && btmp && btmp2 ) ;
		    rxPmmLocFree( head ) ;
		    btmp = btmp2 = NULL ;
		    goto fail ;
		}
	    } /* for 一回の照合で同時に見つかったものに関する処理 */
	    ; assert( head ) ;
	    rxPmmLocFree( head ) ;
	    head = NULL ;
	}
	if ( x == RX_ERR ) {
	    btmp = btmp2 = NULL ;
	    goto error ;
	}
	for ( n = 0 ; n < bnum ; n ++ ) {
	    bitmap[n] &= btmp[n] ;	/* 照合すべきだが、しなかった */
	    bitmap2[n] &= btmp[n] ;	/* ものをOFFする */
	}
	btmp = btmp2 = NULL ;
	; assert( head == NULL ) ;
	if ( rxNodeCheck( rnode, bitmap, bnum ) )
	    if ( !(mode&RX_ADVANCE) || stloc == text )
		goto match ;
	if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
	    goto fail ;	/* 残りのパターン全てがあっても照合しない */
    }

    /* 正規表現の処理: 正規表現ごとに照合する */
    /* ここで照合するパターンは先頭条件ではない */
    for ( n = hnum ; n < rnum ; n ++ ) {
	id = rx->rids[n] ;
	switch ( rxDfaStep( dfa[n], BIT_CHECK(to_ed,id), text, UniTextlen, 
		            &st, &ed, &bras[id], mode&RX_DFA_MODEMASK ) ) {
	case RX_TRUE:
	    BIT_ON( bitmap, id ) ;
	    if ( BIT_CHECK( bitmap2, id ) ) {
		if ( stloc == NULL || stloc > st )
		    stloc = st ;
		if ( edloc < ed )
		    edloc = ed ;
	    }
	    BIT_ON( bitmap2, id ) ;
	    break ;
	case RX_FALSE:
	    BIT_OFF( bitmap, id ) ;
	    BIT_OFF( bitmap2, id ) ;
	    break ;
#ifndef NDEBUG
	default:
	    ; assert( 0 ) ;
#endif
	}
	if ( rxNodeCheck( rnode, bitmap, bnum ) )
	    if ( !(mode&RX_ADVANCE) || stloc == text )
		goto match ;
	if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
	    goto fail ;	/* 残りのパターン全てがあっても照合しない */
    } /* for 部分パターン */

 fail:
    ret = RX_FALSE ;

 error:
    if ( bras ) {
	MEMFREE( bras, pnum*sizeof(rxBrackets) ) ;
    }
    return ret ;

 match:
    matchee->stloc = stloc ;
    matchee->edloc = edloc ;
    matchee->expid = 0 ;		/* 仕様上、rxStep では値は不定 */
    matchee->nbras = pnum ;
    matchee->bras = bras ;

    return RX_TRUE ;

}






/*======================================================================*
 * rxAdvance		拡張正規表現による照合対象の先頭からの照合
 *----------------------------------------------------------------------*
 * 【戻り値】
 * RX_TRUE:		照合成功
 * RX_FALSE:		照合失敗
 * RX_ERR:		エラー
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・照合対象の、拡張正規表現と照合する部分を見つける
 *  ・mode には以下の値を渡す
 *	RX_LONGEST:	正規表現を最長一致で照合（デフォルト）
 *	RX_SHORTEST:	正規表現を最短一致で照合
 *  ・textlen が 0 の場合、照合対象が NULL ターミネイトするまで処理
 *  ・照合位置情報は rxMatcheeFree() で解放すること
 *======================================================================*/
int
rxAdvance( rx, mode, text, textlen, matchee )
    rxHandle	*rx ;		/* ハンドル */
    int		mode ;		/* モード */
    _CONST_ ModUnicodeChar *text ;	/* 照合対象 */
    int		textlen ;	/* 照合対象長さ */
    rxMatchee	*matchee ;	/* 照合位置情報 */
{
	/*
	 * rxStepを呼んでいるだけなので
	 * textlen → UniTextlenの変換は行わない
	 *
	 */

    return rxStep( rx, mode|RX_ADVANCE, text, textlen, matchee ) ;
}


/*======================================================================*
 * rxWalk		拡張正規表現による照合
 *----------------------------------------------------------------------*
 * 【戻り値】
 * n(>0):		照合個数
 * RX_FALSE:		照合失敗
 * RX_ERR:		エラー
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・照合対象全体が拡張正規表現と照合するか判断し、各部分パターンの
 *    出現位置を全て調べる
 *  ・mode には以下の値を渡す
 *	RX_LONGEST:	正規表現を最長一致で照合（デフォルト）
 *	RX_SHORTEST:	正規表現を最短一致で照合
 *  ・textlen が 0 の場合、照合対象が NULL ターミネイトするまで処理
 *  ・照合位置情報は rxMatcheeFree() で解放すること
 *======================================================================*/
int
rxWalk( rx, mode, text, textlen, matchee )
    rxHandle	*rx ;		/* ハンドル */
    int		mode ;		/* モード */
    _CONST_ ModUnicodeChar *text ;	/* 照合対象 */
    int		textlen ;	/* 照合対象長さ */
    rxMatchee	**matchee ;	/* 照合位置情報 */
{
    rxPmmHandle*	pmm ;
    rxDfa**	dfa ;
    rxNode	*rnode ;
    int		ret = RX_ERR ;
    int		n, id, x, b ;
    int		pnum, bnum, rnum, hnum ;
    unsigned	*bitmap ;	/* 出現したパターンIDを記録 */
    unsigned	*bitmap2 ;	/* 出現する可能性のあるIDを記録: */
				/* はじめに全てのビットをONし、出現  */
				/* していないことが分ったらOFFする */
    unsigned	*btmp2 ;
    unsigned	*to_ed ;
    ModUnicodeChar	*st, *ed ;
#ifdef DEBUG
    ModUnicodeChar	*str;
#endif
    rxBrackets	bra ;
    rxWalkInfo	_info, *info;

    int UniTextlen;
#if 0
    /* テストではバイト数を与えられるため */
    /* 正式版は文字数を受け取る ↓(UniTextlen = textlen;) をとおる*/
    UniTextlen = BYTES_2_NUMBER_OF_UNICODE_CHARACTOR(textlen);
#else
    UniTextlen = textlen;
#endif

    info = &_info;

    if ( rx == NULL ) {
	RX_ERROR_SET(RX_ERR_INVALIDHANDLE) ;
	return RX_ERR ;
    }

    dfa = rx->dfa ;
    rnode = rx->rnode ;
    rnum = rx->rnum ;
    pnum = rx->pnum ;
    hnum = rx->hnum ;
    bnum = pnum/32 + 1 ;
    to_ed = rx->ed ;

    bitmap = rxBitmapSet( rx->bitmap1, bnum, 0 ) ;
    bitmap2 = rxBitmapSet( rx->bitmap2, bnum, ~0 ) ;
    btmp2 = rxBitmapSet( rx->bitmap3, bnum, ~0 ) ;
    rxBitmapNotOn( bitmap, bnum, rnode, 0 ) ;
    for ( b = 0 ; b < bnum ; b ++ )
	btmp2[b] = bitmap2[b] = ~(bitmap[b]) ;

    rxMatcheeInit(info) ;		/* 照合位置記憶のための前準備 */

    /* 先頭条件をパターンごとに照合する */
    for ( n = 0 ; n < hnum ; n ++ ) {
	id = rx->rids[n] ;
	switch ( rxDfaAdvance( dfa[n], BIT_CHECK(to_ed,id), text, UniTextlen,
			       &ed, &bra, mode&RX_DFA_MODEMASK ) ) {
	case RX_TRUE:
	    BIT_ON( bitmap, id ) ;
	    if ( BIT_CHECK( bitmap2, id ) ) {	/* あってはいけない場 */
						/* 合は除く */
		if ( rxMatcheeAppend( info, id, text, ed, 1, &bra ) == RX_ERR )
		    goto error ;
	    }
	    BIT_ON( bitmap2, id ) ;
	    break ;
	case RX_FALSE:
	    BIT_OFF( bitmap, id ) ;
	    BIT_OFF( bitmap2, id ) ;
	    break ;
#ifndef NDEBUG
	default:
	    ; assert( 0 ) ;
#endif
	}
	if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
	    goto fail ;	/* 残りのパターン全てがあっても照合しない */
    } /* for 部分パターン */

    /* 非正規表現の処理: 複数を同時に照合する */
    /* ここで照合するパターンは先頭条件ではない */
    if ( pmm = rx->pmm ) {
	rxPmmLoc	*head = NULL, *tmp ;
	unsigned	*btmp ;		/* ここで照合すべきで照合しな */
					/* いものを見つけるため */

	btmp = rxBitmapSet( rx->bitmap4, bnum, 0 ) ;
	if ( btmp == NULL )
	    goto error ;
	for ( n = 0 ; n < rnum ; n ++ )		/* ここで照合しなくて */
	    BIT_ON( btmp, rx->rids[n] ) ;	/* もよいものをONする */

	rxPmmSetText( pmm, text, UniTextlen ) ;
	/* 継続して条件を満たすようになるか調べる */
	for ( x = rxPmmStep(pmm,&head) ; x > 0 ; x = rxPmmStep(pmm,&head) ) {
	    for ( tmp = head ; tmp ; tmp = tmp->next )  {
		id = tmp->id ;
		st = tmp->st ;
		ed = tmp->ed ;
#ifdef DEBUG
		if ( rxDebugMode > 0 ) {
		    fprintf( stderr, "FOUND[%d]:%d,", n, id ) ;
		    for ( str = st ; str < ed ; str ++ )
			putc( *str, stderr ) ;
		    putc( '\n', stderr ) ;
		}
#endif
		if ( BIT_CHECK( to_ed, id ) && 
		     *ed != '\0' && (ed-text)!=UniTextlen )
		    continue ;		/* 末尾条件であるが、末尾では */
					/* ない場合、つぎの照合を探す */

		BIT_ON( btmp, id ) ;
		BIT_ON( bitmap, id ) ;
		if ( BIT_CHECK( btmp2, id ) )
		    if ( rxMatcheeAppend( info, id, st, ed, 0, NULL )
			 == RX_ERR ) {
			; assert( head && btmp ) ;
			rxPmmLocFree( head ) ;
			goto error ;
		    }
		BIT_ON( bitmap2, id ) ;
		if ( !rxNodeCheck( rnode, bitmap2, bnum ) ) {
		    ; assert( head && btmp ) ;
		    rxPmmLocFree( head ) ;
		    goto fail ;
		}
	    } /* for 一回の照合で同時に見つかったものに関する処理 */
	    ; assert( head ) ;
	    rxPmmLocFree( head ) ;
	    head = NULL ;
	}
	if ( x == RX_ERR ) {
	    goto error ;
	}
	for ( n = 0 ; n < bnum ; n ++ ) {
	    bitmap[n] &= btmp[n] ;	/* 照合すべきだが、しなかった */
	    bitmap2[n] &= btmp[n] ;	/* ものをOFFする */
	}
	; assert( head == NULL ) ;
	if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
	    goto fail ;	/* 残りのパターン全てがあっても照合しない */
    }

    /* 正規表現の処理: 正規表現ごとに照合する */
    /* ここで照合するパターンは先頭条件ではない */
    for ( n = hnum ; n < rnum ; n ++ ) {
	id = rx->rids[n] ;
	x = rxDfaStep( dfa[n], BIT_CHECK(to_ed,id), text, UniTextlen,
		       &st, &ed, &bra, mode&RX_DFA_MODEMASK ) ;
	for ( ; x == RX_TRUE ; ) {
	    BIT_ON( bitmap, id ) ;
	    if ( BIT_CHECK( btmp2, id ) )
		if ( rxMatcheeAppend( info, id, st, ed, 1, &bra ) == RX_ERR )
		    goto error ;
	    BIT_ON( bitmap2, id ) ;
	    if ( UniTextlen == RX_NULL_TERMINATE )
		x = rxDfaStep( dfa[n], BIT_CHECK(to_ed,id), ed,
			       RX_NULL_TERMINATE, &st, &ed, &bra,
			       mode&RX_DFA_MODEMASK|RX_SKIPEMPTY ) ;
	    else
		x = rxDfaStep( dfa[n], BIT_CHECK(to_ed,id), ed,
			       UniTextlen - (ed - text), &st, &ed, &bra,
			       mode&RX_DFA_MODEMASK|RX_SKIPEMPTY ) ;
	}
	; assert( x != RX_ERR ) ;
	if ( !BIT_CHECK( bitmap, id ) || !BIT_CHECK( bitmap2, id ) ) {
	    /* bitmap, bitmap2 ともに TRUE でなければ、このパターンは */
	    /* 出現していなかったことになる */
	    BIT_OFF( bitmap, id ) ;
	    BIT_OFF( bitmap2, id ) ;
	    if ( !rxNodeCheck( rnode, bitmap2, bnum ) )
		goto fail ;	/* 残りのパターン全てがあっても照合しない */
	}
    } /* for 部分パターン */

    if ( rxNodeCheck( rnode, bitmap, bnum ) )
	goto match ;
 fail:
    ret = RX_FALSE ;

 error:
    for ( n = 0 ; n < info->rx_matchee_num ; n ++ ) {
	if ( info->rx_matchee[n].bras ) {
	    MEMFREE( info->rx_matchee[n].bras, sizeof(rxBrackets) ) ;
	}
    }
    if ( info->rx_matchee ) {
	MEMFREE( info->rx_matchee, sizeof(rxMatchee)*info->rx_matchee_size ) ;
    }
    return ret ;

 match:
    if ( info->rx_matchee_num < info->rx_matchee_size ) {
	/* 未使用部分を削除する */
	; assert( info->rx_matchee_num > 0 && info->rx_matchee ) ;
	*matchee = (rxMatchee*)MEMRESIZE( info->rx_matchee,
			      sizeof(rxMatchee)*info->rx_matchee_num,
			      sizeof(rxMatchee)*info->rx_matchee_size ) ;
	if ( *matchee == NULL ) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	    goto error ;
	}
    } else {
	; assert( info->rx_matchee_num == info->rx_matchee_size ) ;
	*matchee = info->rx_matchee ;
    }

    return info->rx_matchee_num ;

}




/*======================================================================*
 * rxFree		コンパイルされた拡張正規表現の解放
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *======================================================================*/
void
rxFree( rx )
    rxHandle	*rx ;		/* ハンドル */
{
    if ( rx ) {
	int bnum = (rx->pnum)/32 + 1 ;
	if ( rx->dfa ) {
	    int n ;
	    for ( n = 0 ; n < rx->rnum ; n ++ ) {
		if ( rx->dfa[n] ) {
		    rxDfaFree( rx->dfa[n] ) ;
		}
	    }
	    MEMFREE( rx->dfa, rx->rnum*sizeof(rxDfa*) ) ;
	}
	if ( rx->rids ) {
	    MEMFREE( rx->rids, rx->rnum*sizeof(int) ) ;
	}
	if ( rx->pmm ) {
	    rxPmmFree( rx->pmm ) ;
	}
	if ( rx->rnode ) {
	    rxNodeFree( rx->rnode ) ;
	}
	if ( rx->ed ) {
	    MEMFREE( rx->ed, bnum*sizeof(unsigned) ) ;
	}
	if ( rx->bitmap1 ) {
	    MEMFREE( rx->bitmap1, bnum*sizeof(unsigned) );
	}
	if ( rx->bitmap2 ) {
	    MEMFREE( rx->bitmap2, bnum*sizeof(unsigned) );
	}
	if ( rx->bitmap3 ) {
	    MEMFREE( rx->bitmap3, bnum*sizeof(unsigned) );
	}
	if ( rx->bitmap4 ) {
	    MEMFREE( rx->bitmap4, bnum*sizeof(unsigned) );
	}
	MEMFREE( rx, sizeof(rxHandle) ) ;
    }
}


/*======================================================================*
 * rxMatcheeFree	照合位置情報の解放
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・照合位置情報自身の解放は行なわないので、呼び出し側が行なうこと
 *======================================================================*/
void
rxMatcheeFree( rm )
    rxMatchee	*rm ;		/* 照合位置情報 */
{
    if ( rm ) {
	if ( rm->bras ) {
	    MEMFREE( rm->bras, rm->nbras*sizeof(rxBrackets) ) ;
	    rm->bras = 0;
	    rm->nbras = 0;
	}
    }
}



/*
 * 正規表現と先頭から一致する文字列を発見する
 */
static int
rxDfaAdvance( dfa, is_tail, text, textlen, ed, bra, mode )
    rxDfa	*dfa ;		/* 正規表現照合オートマトン */
    int		is_tail ;	/* 末尾条件か否か */
    ModUnicodeChar	*text ;		/* 照合対象 */
    int		textlen ;	/* 照合対象長さ */
    ModUnicodeChar	**ed ;		/* 照合終了位置 */
    rxBrackets	*bra ;		/* 照合副表現 */
    int		mode ;		/* モード */
{
    int	x = RX_FALSE, is_first ;

    if ( dfa ) {
	/* 先頭からの照合を調べるので rxDfaMatch を用いる */
	x = rxDfaMatch( dfa, text, textlen, ed, bra, mode ) ;
	is_first = mode&RX_SHORTEST ;	/* 最長一致モードでは再検査の */
					/* 必要なし */
    } else {	/* '^$' の場合 */
	if ( text[0] == '\0' || textlen == 0 ) {
	    *ed = text ;
	    x = RX_TRUE ;
	    is_first = RX_FALSE ;	/* retry の必要なし */
	}
	bra->nbra = 0 ;			/* 副表現なし */
    }
 retry:
    if ( x == RX_TRUE ) {		/* 見つかった */
#ifdef DEBUG
	if ( rxDebugMode > 0 ) {
	    fprintf( stderr, "FOUND:" ) ;
	    {
		ModUnicodeChar *p ;
		int i = 0;
		for ( p = text ; p < *ed ; p ++ )
		    putc( *p, stderr ) ;
		putc( '\n', stderr ) ;
		fprintf( stderr, "- %d(%d):", i, bra->nbra ) ;
		for ( i = 0 ; i < bra->nbra ; i ++ ) {
		    fprintf( stderr, "(%x,%x)",
			    bra->braslist[i],
			    bra->braelist[i] ) ;
		    for ( p = bra->braslist[i] ; p < bra->braelist[i] ; p ++ )
			putc( *p, stderr ) ;
		    putc( ' ', stderr ) ;
		}
		putc( '\n', stderr ) ;
	    }
	}
#endif

#ifdef RX_UNICODE
	if ( is_tail && **ed != '\0' && 
					(*ed-text)!=textlen){
#else /* RX_UNICODE */
	if ( is_tail && **ed != '\0' && (*ed-text)!=textlen ) {
#endif /* RX_UNICODE */


	    /* 末尾条件であるが、末尾ではない場合 */
	    if ( is_first ) {
		/* 最短一致モードでは、末尾条件に失敗しても最長一 */
		/* 致で照合し直して、照合に成功することがある */
		x = rxDfaMatch( dfa, text, textlen, ed, bra, RX_LONGEST ) ;
		is_first = RX_FALSE ;
		goto retry ;
	    }
	    return RX_FALSE ;
	}
	return RX_TRUE ;
    }
    ; assert( x != RX_ERR ) ;
    return RX_FALSE ;
}


/*
 * 正規表現と一致する文字列を発見する
 */
static int
rxDfaStep( dfa, is_tail, text, textlen, st, ed, bra, mode )
    rxDfa	*dfa ;		/* 正規表現照合オートマトン */
    int		is_tail ;	/* 末尾条件か否か */
    ModUnicodeChar	*text ;		/* 照合対象 */
    int		textlen ;	/* 照合対象長さ */
    ModUnicodeChar	**st ;		/* 照合開始位置 */
    ModUnicodeChar	**ed ;		/* 照合終了位置 */
    rxBrackets	*bra ;		/* 照合副表現 */
    int		mode ;		/* モード */
{
    int	x, is_first = mode&RX_SHORTEST ;
    ModUnicodeChar	*text1 = text ;
    int		textlen1 = textlen ;

    x = rxDfaExec( dfa, text1, textlen1, st, ed, bra, mode ) ;
    for ( ; x == RX_TRUE ; ) {
#ifdef DEBUG
	if ( rxDebugMode > 0 ) {
	    fprintf( stderr, "FOUND:" ) ;
	    {
		ModUnicodeChar *p ;
		int i = 0;
		for ( p = *st ; p < *ed ; p ++ )
		    putc( *p, stderr ) ;
		putc( '\n', stderr ) ;
		fprintf( stderr, "- %d(%d):", i, bra->nbra ) ;
		for ( i = 0 ; i < bra->nbra ; i ++ ) {
		    fprintf( stderr, "(%x,%x)",
			    bra->braslist[i],
			    bra->braelist[i] ) ;
		    for ( p = bra->braslist[i] ; p < bra->braelist[i] ; p ++ )
			putc( *p, stderr ) ;
		    putc( ' ', stderr ) ;
		}
		putc( '\n', stderr ) ;
	    }
	}
#endif
	if ( is_tail && **ed != '\0' && (*ed-text)!=textlen ) {
	    /* 末尾条件であるが、末尾ではない場合 */
	    if ( is_first ) {
		/* 最短一致モードでは、末尾条件に失敗しても最長一 */
		/* 致で照合し直して、照合に成功することがある */
		x = rxDfaExec( dfa, text1, textlen1, st, ed,
			       bra, mode&~RX_SHORTEST ) ;
		is_first = RX_FALSE ;
	    } else {
		text1 = NULL ;
		textlen1 = 0 ;
		mode |= RX_SKIPEMPTY ;
		x = rxDfaExec( dfa, text1, textlen1, st, ed,
			       bra, mode ) ;
		is_first = mode&RX_SHORTEST ;
	    }
	} else {
	    return RX_TRUE ;
	}
    }
    ; assert( x != RX_ERR ) ;
    return RX_FALSE ;
}


/*
 * 照合位置の記録のための初期化
 *	rxWalk() のみで使用される 
 */
static void
rxMatcheeInit( info )
    rxWalkInfo	*info ;		/* rxWalkのための一時領域 */
{
    info->rx_matchee = NULL ;		/* free は上位が行なうこと */
    info->rx_matchee_size = 0 ;
    info->rx_matchee_num = 0  ;
}


/*
 * 照合位置の記録
 *	rxWalk() のみで使用される 
 */
static int
rxMatcheeAppend( info, pid, st, ed, nbra, bra )
    rxWalkInfo	*info ;		/* rxWalkのための一時領域 */
    int		pid ;		/* パターンID */
    ModUnicodeChar	*st ;		/* 照合開始位置 */
    ModUnicodeChar	*ed ;		/* 照合終了位置 */
    int		nbra ;		/* rxBrackets の個数（0 or 1） */
    rxBrackets	*bra ;		/* rxBrackets（あれば） */
{
    if ( info->rx_matchee_size == 0 ) {
	; assert( info->rx_matchee == NULL && info->rx_matchee_num == 0 ) ;
	info->rx_matchee =
	    (rxMatchee*)MEMALLOC( sizeof(rxMatchee)*RX_MATCHEE_ARRAY_INC ) ;
	if ( info->rx_matchee == NULL ) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	    return RX_ERR ;
	}
	info->rx_matchee_size = RX_MATCHEE_ARRAY_INC ;
    }
    else if ( info->rx_matchee_size <= info->rx_matchee_num ) {
	info->rx_matchee
		= (rxMatchee*)MEMRESIZE(
			info->rx_matchee,
			sizeof(rxMatchee)
			*(info->rx_matchee_size+RX_MATCHEE_ARRAY_INC),
			sizeof(rxMatchee)*info->rx_matchee_size ) ;
	if ( info->rx_matchee == NULL ) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	    return RX_ERR ;
	}
	info->rx_matchee_size += RX_MATCHEE_ARRAY_INC ;
    }
    info->rx_matchee[info->rx_matchee_num].expid = pid ;/* 照合したパターンID */
    info->rx_matchee[info->rx_matchee_num].stloc = st ;
    info->rx_matchee[info->rx_matchee_num].edloc = ed ;
    info->rx_matchee[info->rx_matchee_num].nbras = nbra ;

    if ( bra ) {
	int		m ;
	; assert( nbra == 1 ) ;
	info->rx_matchee[info->rx_matchee_num].bras =
	    (rxBrackets*)MEMALLOC( sizeof(rxBrackets) ) ;
	if ( info->rx_matchee[info->rx_matchee_num].bras == NULL ) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	    return RX_ERR ;
	}
	info->rx_matchee[info->rx_matchee_num].bras->nbra = bra->nbra ;
	for ( m = 0 ; m < bra->nbra ; m ++ ) {
	    info->rx_matchee[info->rx_matchee_num].bras->braslist[m]
		    = bra->braslist[m] ;
	    info->rx_matchee[info->rx_matchee_num].bras->braelist[m]
		    = bra->braelist[m] ;
	}
    } else {
	; assert( nbra == 0 ) ;
	info->rx_matchee[info->rx_matchee_num].bras = NULL ;
    }

    return ++info->rx_matchee_num ;		/* 照合を数える */
}

    
/*
 * 解析木において、自分の配下の OR ノードのパターンIDをビットパターン
 * で表現し、自ノードに記録する（再帰的処理）
 */
static int
rxNodeMod( rn,  bnum )
    rxNode	*rn ;		/* 解析木ノード */
    int		bnum ;		/* ビットマップの配列数 */
{
    int ret = RX_FALSE, b ;
    unsigned *bitmap, *bitmap2 ;

    switch ( rn->optype ) {
    case RX_OP_OR:	/* OR の場合、自分の縮退を試みる */
	if ( rn->data.child.left->optype == RX_OP_PID ) {
	    if ( rn->data.child.right->optype == RX_OP_PID ) {
		/* 左右の子ノードともにリーフなので、子ノードのIDを */
		/* 自ノードでビットマップ表現する */
		bitmap = rxBitmapAlloc( bnum, 0 ) ;
		if ( bitmap == NULL )
		    return RX_ERR ;
		BIT_ON( bitmap, rn->data.child.left->data.pid ) ;
		BIT_ON( bitmap, rn->data.child.right->data.pid ) ;
		rxNodeFree( rn->data.child.left ) ;	/* 子は不要 */
		rxNodeFree( rn->data.child.right ) ;
		rn->optype = RX_OP_BMP ;
		rn->data.bitmap.buf = bitmap ;
		rn->data.bitmap.num = bnum ;
		ret = RX_TRUE ;
	    }
	    else switch ( rxNodeMod( rn->data.child.right, bnum ) ) {
	    case RX_TRUE:
		/* 右子ノードは縮退できるので、そのビットマップに左子 */
		/* ノードのIDをマージし、自ノードのビットマップとする */
		bitmap = rn->data.child.right->data.bitmap.buf ;
		BIT_ON( bitmap, rn->data.child.left->data.pid ) ;
		rxNodeFree( rn->data.child.left ) ;
		rn->data.child.right->data.bitmap.buf = NULL ;
				/* 子ノードをfreeする際、ビットマップ */
				/* がfreeされないようにする */
		rn->data.child.right->data.bitmap.num = 0 ;
		rxNodeFree( rn->data.child.right ) ;
		rn->optype = RX_OP_BMP ;
		rn->data.bitmap.buf = bitmap ;
		rn->data.bitmap.num = bnum ;
		ret = RX_TRUE ;
		break ;
	    case RX_FALSE:
		/* 右子ノードは縮退できないので、そのまま */
		break ;
	    default:
		ret = RX_ERR ;
	    }
	}
	else switch ( rxNodeMod( rn->data.child.left, bnum ) ) {
	case RX_TRUE:
	    bitmap = rn->data.child.left->data.bitmap.buf ;
	    if ( rn->data.child.right->optype == RX_OP_PID ) {
		/* 左子ノードは縮退できるので、そのビットマップに右子 */
		/* ノードのIDをマージし、自ノードのビットマップとする */
		BIT_ON( bitmap, rn->data.child.right->data.pid ) ;
		rn->data.child.left->data.bitmap.buf = NULL ;
		rn->data.child.left->data.bitmap.num = 0 ;
		rxNodeFree( rn->data.child.left ) ;
		rxNodeFree( rn->data.child.right ) ;
		rn->optype = RX_OP_BMP ;
		rn->data.bitmap.buf = bitmap ;
		rn->data.bitmap.num = bnum ;
		ret = RX_TRUE ;
	    }
	    else switch ( rxNodeMod( rn->data.child.right, bnum ) ) {
	    case RX_TRUE:
		/* 左右の子ノードともに縮退できるので、左子ノードのビッ */
		/* トマップに右子ノードのビットマップをマージし、自ノー */
		/* ドのビットマップとする */
		bitmap2 = rn->data.child.right->data.bitmap.buf ;
		for ( b = 0 ; b < bnum ; b ++ )
		    bitmap[b] |= bitmap2[b] ;
		rn->data.child.left->data.bitmap.buf = NULL ;
		rn->data.child.left->data.bitmap.num = 0 ;
		rxNodeFree( rn->data.child.left ) ;
		rxNodeFree( rn->data.child.right ) ;
		rn->optype = RX_OP_BMP ;
		rn->data.bitmap.buf = bitmap ;
		rn->data.bitmap.num = bnum ;
		ret = RX_TRUE ;
		break ;
	    case RX_FALSE:
		/* 右子ノードは縮退できないので、そのまま */
		break ;
	    default:
		ret = RX_ERR ;
	    }
	    break ;
	case RX_FALSE:
	    /* 左子ノードは縮退できないので、そのまま */
	    break ;
	default:
	    ret = RX_ERR ;
	}
	break ;
    case RX_OP_AND:	/* OR 以外では、子ノードの縮退を試みるが、自分 */
    case RX_OP_ANDNOT:	/* は縮退しない */
	if ( rxNodeMod( rn->data.child.left, bnum ) == RX_ERR )
	    return RX_ERR ;
	if ( rxNodeMod( rn->data.child.right, bnum ) == RX_ERR )
	    return RX_ERR ;
	break ;
    case RX_OP_PID:	/* リーフでは縮退しない */
	break ;
#ifndef NDEBUG
    default:
	; assert( 0 ) ;
#endif
    }
    return ret ;
}


/*
 * 解析木が、パターンIDをビットパターンを満たすか否かを再帰的に判断する
 */
static int
rxNodeCheck( rn, bitmap, bnum )
    rxNode	*rn ;		/* 解析木ノード */
    unsigned	*bitmap ;	/* ビットマップ */
    int		bnum ;		/* ビットマップの配列数 */
{
    switch ( rn->optype ) {
    case RX_OP_OR:
	return rxNodeCheck( rn->data.child.left, bitmap, bnum ) ||
	       rxNodeCheck( rn->data.child.right, bitmap, bnum ) ;
    case RX_OP_AND:
	return rxNodeCheck( rn->data.child.left, bitmap, bnum ) &&
	       rxNodeCheck( rn->data.child.right, bitmap, bnum ) ;
    case RX_OP_ANDNOT:
	return rxNodeCheck( rn->data.child.left, bitmap, bnum ) &&
	       !rxNodeCheck( rn->data.child.right, bitmap, bnum ) ;
    case RX_OP_PID:
	return BIT_CHECK( bitmap, rn->data.pid ) ;
    case RX_OP_BMP:
	{
	    int b ;
	    for ( b = 0 ; b < bnum ; b ++ )
		if ( bitmap[b]&(rn->data.bitmap.buf[b]) )
		    return RX_TRUE ;	/* ビットマップはORなので、ど */
					/* れかがマッチすれば良い */ 
	}
	break ;
#ifndef NDEBUG
    default:
	; assert( 0 ) ;
	return RX_ERR ;
#endif
    }
    return RX_FALSE ;
}


/*
 * bitmap において、否定されているパターンIDに対応するビットをONする
 */
static void
rxBitmapNotOn( bitmap, bnum, rn, is_on )
    unsigned	*bitmap ;	/* ビットマップ */
    int		bnum ;		/* ビットマップの配列数 */
    rxNode	*rn ;		/* 解析木ノード */
    int		is_on ;		/* ビットを ON するか否か（1/0の場合）*/
{
    switch ( rn->optype ) {
    case RX_OP_OR:
    case RX_OP_AND:
	rxBitmapNotOn( bitmap, bnum, rn->data.child.left, is_on ) ;
	rxBitmapNotOn( bitmap, bnum, rn->data.child.right, is_on ) ;
	break ;
    case RX_OP_ANDNOT:
	rxBitmapNotOn( bitmap, bnum, rn->data.child.left, is_on ) ;
	rxBitmapNotOn( bitmap, bnum, rn->data.child.right, !is_on ) ;
	break ;
    case RX_OP_PID:
	if ( is_on )
	    BIT_ON( bitmap, rn->data.pid ) ;
	break ;
    case RX_OP_BMP:
	if ( is_on ) {
	    int b ;
	    for ( b = 0 ; b < bnum ; b ++ )
		bitmap[b] |= rn->data.bitmap.buf[b] ;
	}
	break ;
#ifndef NDEBUG
    default:
	; assert( 0 ) ;
#endif
    }
}

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
