/*
 * rxParse.c --- 拡張正規表現パーザ
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
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "rx.h"
#include "rxLocal.h"
#include "rxDefs.h"
#include "rxParse.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


#define RX_DONE			(-2)
#define RX_TOKEN_AND		(-(RX_AND))
#define RX_TOKEN_OR		(-(RX_OR))
#define RX_TOKEN_ANDNOT		(-(RX_ANDNOT))
#define RX_TOKEN_LPAREN		(-(RX_LPAREN))
#define RX_TOKEN_RPAREN		(-(RX_RPAREN))


#define RX_PARSE_BUF_SIZE	4096	/* 一つの正規表現パターンの最 */
					/* 大長 */
#define RX_PARSE_BUF_SIZE2	4094
#define RX_PARSE_ARRAY_INC	16	/* 部分パターン領域の増分 */


typedef struct {
	ModUnicodeChar	*patbuf ;	/* 検索条件 */
	int	rxtoken ;		/* 次に処理すべきトークン */

	ModUnicodeChar	**rxbufs ;	/* 部分パターンのための領域 */
	int	rxbufs_num ;		/* 部分パターン数 */
	int	rxbufs_size ;		/* 部分パターン領域の大きさ */
	int	*rx_ptype ;		/* 部分パターンタイプのための領域 */
} rxParseInfo;

#ifdef DEBUG
static int rxbufs_num;
static ModUnicodeChar **rxbufs;
#endif


STATIC_FUNC( rxNode*,	rxNodeNewLeaf,		(int) ) ;
STATIC_FUNC( rxNode*,	rxNodeNewBranch,	(int,rxNode*,rxNode*) ) ;

STATIC_FUNC( int,	rxParseLex,		(rxParseInfo*) ) ;
STATIC_FUNC( rxNode*,	rxParseExpr,		(rxParseInfo*) ) ;
STATIC_FUNC( rxNode*,	rxParseTerm,		(rxParseInfo*) ) ;
STATIC_FUNC( rxNode*,	rxParseFactor,		(rxParseInfo*) ) ;
STATIC_FUNC( int,	rxParseMatch,		(rxParseInfo*,int) ) ;

STATIC_FUNC( void,	rxPatternInit,		(rxParseInfo*) ) ;
STATIC_FUNC( int,	rxPatternAppend,	(rxParseInfo*,ModUnicodeChar*,int,int) ) ;
STATIC_FUNC( void,	rxPatternFree,		(rxParseInfo*) ) ;


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * 検索条件の解析
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*======================================================================*
 * rxParse		検索条件の解析し、解析木を作成する
 *----------------------------------------------------------------------*
 * 【戻り値】
 * n(>0):		部分パターン数
 * RX_ERR:		処理失敗
 *----------------------------------------------------------------------*
 * 【注意】
 *  ・pats, ptype, rn の解放は呼び出し側で行なう
 *  ・ここから呼び出される最急降下パーザではエラーコードは設定していない
 *======================================================================*/
int
rxParse( pattern, pats, ptype, rn )
    ModUnicodeChar	*pattern ;	/* 拡張正規表現 */
    ModUnicodeChar	***pats ;	/* 部分パターン配列 */
    int		**ptype ;	/* 部分パターンタイプ配列 */
    rxNode	**rn ;		/* 解析木 */
{
    rxParseInfo _info, *info;

    info = &_info;
	
    ; assert( pattern && rn && ptype && pats ) ;

    *rn = NULL ;
    info->patbuf = pattern ;
    rxPatternInit( info ) ;		/* 部分パターン領域の初期化 */
    info->rxtoken = rxParseLex( info ) ;/* 最初の１トークンだけ取り出す */
					/* 以降はrxParseExpr()の中から */

    if ( info->rxtoken == RX_ERR )
	    goto error ;
    else if ( info->rxtoken == RX_DONE ) {	/* 検索語が空文字列 */
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
	    goto error ;
    }

    *rn = rxParseExpr( info ) ;
    if ( *rn == NULL )
	    goto error ;

    if ( info->rxtoken != RX_DONE ) {
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;	/* '(' がないのに ')'  */
	    goto error ;			/* で閉じようとした */
    }

    *ptype = info->rx_ptype ;
    *pats = info->rxbufs ;			/* free は上位が責任を持つ */

#ifdef DEBUG
    rxbufs_num = info->rxbufs_num;
    rxbufs = info->rxbufs;
#endif

    return info->rxbufs_num ;			/* パターン数を返す */

 error:
    if ( *rn )
	rxNodeFree( *rn ) ;
    rxPatternFree( info ) ;
    return RX_ERR ;
}


/*======================================================================*
 * rxNodeFree		解析木のメモリ解放
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *----------------------------------------------------------------------*
 * 【説明】
 *  ・解析木のメモリを解放する
 *======================================================================*/
void
rxNodeFree( top )
    rxNode	*top ;		/* ノード */
{
    if ( top ) {
	switch( top->optype ) {
	case RX_OP_PID:
	    break ;

	case RX_OP_BMP:
	    if ( top->data.bitmap.buf ) {
		MEMFREE( top->data.bitmap.buf,
			 top->data.bitmap.num*sizeof(unsigned) ) ;
	    }
	    break ;

	case RX_OP_OR:
	case RX_OP_AND:
	case RX_OP_ANDNOT:
	    if ( top->data.child.left ) {
		rxNodeFree( top->data.child.left ) ;
	    }
	    if ( top->data.child.right ) {
		rxNodeFree( top->data.child.right ) ;
	    }
	    break ;
#ifndef NDEBUG
	default:
	    ; assert( 0 ) ;
	    break ;
#endif
	}
	MEMFREE( top, sizeof(rxNode) ) ;
    }
}

#ifdef DEBUG
/*
 * rxNodePrint		解析木を標準出力にテキスト形式で出力する
 */
void
rxNodePrint( top, fp )
    rxNode	*top ;		/* ノード */
    FILE	*fp ;		/* 出力ストリーム */
{
    ; assert( top ) ;
    switch( top->optype ) {
    case RX_OP_PID:
	{
	    ModUnicodeChar*	tmp ;
	    ; assert( 0 <= top->data.pid && top->data.pid < rxbufs_num ) ;
	    tmp = rxbufs[top->data.pid] ;
	    if ( tmp ) 
		fprintf( fp, "'%s'@%d", tmp, top->data.pid ) ;
	    else
		fprintf( fp, "(null)@%d", top->data.pid ) ;
	}
	break ;

    case RX_OP_BMP:
	{
	    int b ;
	    fprintf( fp, "(" ) ;
	    for ( b = 0 ; b < top->data.bitmap.num ; b ++ ) {
		if ( b )
		    fprintf( fp, "," ) ;
		fprintf( fp, "%08x", top->data.bitmap.buf[b] ) ;
	    }
	    fprintf( fp, ")" ) ;
	}
	break ;

    case RX_OP_OR:
	fprintf( fp, "(+ " ) ;
	rxNodePrint( top->data.child.left, fp ) ;
	fprintf( fp, " " ) ;
	rxNodePrint( top->data.child.right, fp ) ;
	fprintf( fp, ")" ) ;
	break ;

    case RX_OP_AND:
	fprintf( fp, "(* " ) ;
	rxNodePrint( top->data.child.left, fp ) ;
	fprintf( fp, " " ) ;
	rxNodePrint( top->data.child.right, fp ) ;
	fprintf( fp, ")" ) ;
	break ;

    case RX_OP_ANDNOT:
	fprintf( fp, "(- " ) ;
	rxNodePrint( top->data.child.left, fp ) ;
	fprintf( fp, " " ) ;
	rxNodePrint( top->data.child.right, fp ) ;
	fprintf( fp, ")" ) ;
	break ;

    default:
	break ;
    }
    return ;
}
#endif


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * 解析木のノード操作関数
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * 末端（リーフ）ノードを作成する
 */
static rxNode*
rxNodeNewLeaf( data )
    int		data ;		/* パターンID */
{
    rxNode*	ret ;

    ; assert( data >= 0 ) ;

    ret = (rxNode*)MEMALLOC( sizeof( rxNode ) ) ;
    if ( ret == NULL ) {
	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	return NULL ;
    }

    ret->optype = RX_OP_PID ;
    ret->data.pid = data ;

    return ret ;
}


/*
 * 中間ノードを作成する
 */
static rxNode*
rxNodeNewBranch( op, lchild, rchild )
    int		op ;
    rxNode	*lchild ;
    rxNode	*rchild ;
{
    rxNode*	ret ;

    ; assert( (op == RX_OP_OR)||(op == RX_OP_AND)||(op == RX_OP_ANDNOT) ) ;

    ret = (rxNode*)MEMALLOC( sizeof( rxNode ) ) ;
    if ( ret == NULL ) {
	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	return NULL ;
    }

    ret->optype = op ;
    ret->data.child.left = lchild ;
    ret->data.child.right = rchild ;

    return ret ;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * 検索条件のための最急降下パーザ
 * 【参考】
 *  ・Aho, A. V. and Sethi, R. and Ullman, J. D.,
 *	"Compilers: Principles, Techniques, and Tools",
 *	Addison-Wesley,1986
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * トークンを探す
 *	全トークンを処理し終えた場合	RX_DONE
 *	論理演算子・括弧		トークンに応じた負の値
 *	部分パターン			トークンに応じた正の値
 */
static int
rxParseLex( info )
    rxParseInfo *info ;
{
    ModUnicodeChar	c, *pos ;
    int		len, is_escape, is_subexp, bracketpos, tailpos ;
    int		ptype ;

    c = *info->patbuf++ ;
	switch ( c ) {
		case '\0':
			return RX_DONE ;

		case RX_AND:
		case RX_OR:
		case RX_ANDNOT:
		case RX_LPAREN:
		case RX_RPAREN:
			return (-((int)c)) ;
		default:
			is_escape = 0 ;		/* エスケープ文字があったか否か */
			is_subexp = 0 ;		/* \(...\) の処理中か否か */
			bracketpos = 0 ;	/* '[' の位置 */
			tailpos = 0 ;		/* '$' の位置 */
			ptype = 0 ;		/* 正規表現か否か */
			pos = &info->patbuf[-1] ;	/* 部分パターンの開始位置 */
			len = 0 ;		/* 部分パターンの長さ */

			for ( ; ; c = *info->patbuf++ ) {
			    if ( is_escape ) {
					if ( c == '\0' ) {
						RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
						return RX_ERR ;
					}
					if ( c == RX_LPAREN ) {		/* '\(' の場合 */
		    			ptype |= RX_REGX ;
		    			is_subexp ++ ;		/* ネストに備える */ 
					}
					if ( c == RX_RPAREN ) {		/* '\)' の場合 */
		    			if ( is_subexp == 0 ) {
							RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
							return RX_ERR ;		/* '\(' がなかった */
		    			}
		    			is_subexp -- ;
					}
					len += 2 ;
					is_escape = 0 ;
					continue ;
	    		}

	    		switch ( c ) {
	    			case '\0':
						info->patbuf -- ;		/* 次回のために一つ戻す */
						goto fini ;
	    			case RX_ESCAPE:
						if ( bracketpos ) {	/* '[...]' では例外 */
		    				len++ ;
		    				continue ;
						}
						is_escape = 1 ;
						continue ;
	    			case RX_PERIOD:
						ptype |= RX_REGX ;
						break ;
	    			case RX_BRA:
						if ( bracketpos == 0 ) {
		    				bracketpos = len + 1 ; /* '[]abc]' に対処するため */
		    				ptype |= RX_REGX ;	/* 位置を記録する */
						}
						break ;
	    			case RX_CKET:
						if ( bracketpos == 0 ) {
		    				RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
		    				return RX_ERR ;		/* '[' がなかった */
						} else if ( bracketpos != len )
		    				bracketpos = 0 ;
							break ;
	    			case RX_AND:
	    			case RX_OR:
	    			case RX_ANDNOT:
	    			case RX_LPAREN:
	    			case RX_RPAREN:
						if ( is_subexp || bracketpos ) {
		    				len ++ ;
		    				continue ;
						}
						info->patbuf -- ;		/* 次回のために一つ戻す */
						goto fini ;
	    			case RX_STAR:
	    			case RX_PLUS:
	    			case RX_QUESTION:
						ptype |= RX_REGX ;
						break ;
	    			case RX_HAT:
						if ( len == 0 && !(ptype&RX_HEAD) ) {
							/* '^' は先頭でのみ特殊文字 */
		    				pos ++ ;		/* '^' を削除する */
		    				ptype |= RX_HEAD ;
		    				continue ;
						}
						break ;
	    			case RX_DOLLAR:
						tailpos = len + 1 ;	/* 末尾条件の可能性あり */
						break ;			/* 最終判断は fini で行なう */
	    			default:
						break ;
	    		}
	    		len ++ ;
			}
   	}

 fini:
	if ( len > 0 && tailpos == len ) {	/* $ が末尾にあった */
		; assert( len > 0 ) ;
		len -- ;			/* '$' を削除する */
		ptype |= RX_TAIL ;
    }

	if ( len == 0 && !(ptype&RX_HEAD && ptype&RX_TAIL) ) {
					/* '^', '$' など */
		RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
		return RX_ERR ;
	}

	if ( is_subexp || bracketpos ) {	/* '[', '\(' が閉じてない */
		RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
		return RX_ERR ;
    }

#ifdef DEBUG
    if ( rxDebugMode > 1 )
	fprintf( stderr, "rxParseLex:%s(%d)[%d]:leaf\n", pos, len, ptype ) ;
#endif

    return rxPatternAppend( info, pos, len, ptype ) ;
}


/*
 * OR の処理
 */
static rxNode*
rxParseExpr( info )
    rxParseInfo *info ;
{
    rxNode *lc, *rc, *tmpc ;

    lc = rxParseTerm( info ) ;
    if ( lc == NULL )
	return NULL ;

    for ( ; ; ) {
	switch ( info->rxtoken ) {
	case RX_TOKEN_OR:
	    if ( rxParseMatch( info, info->rxtoken ) == RX_ERR )
		goto error ;
	    rc = rxParseTerm( info ) ;
	    if ( rc == NULL )		/* orの右側の項がない */
		goto error ;
	    tmpc = rxNodeNewBranch( RX_OP_OR, lc, rc ) ;
	    if ( tmpc == NULL ) {
		rxNodeFree( rc ) ;
		goto error ;
	    }
	    lc = tmpc ;
	    break ;

	default:
	    return lc ;
	}
    }

 error:
    rxNodeFree( lc ) ;
    return NULL ;
}


/*
 * AND, ANDNOT の処理
 */
static rxNode*
rxParseTerm( info )
    rxParseInfo *info ;
{
    rxNode *lc, *rc, *tmpc ;

    lc = rxParseFactor( info ) ;
    if ( lc == NULL )
	return NULL ;

    for( ; ; ) {
	switch( info->rxtoken ) {
	case RX_TOKEN_AND:
	    if ( rxParseMatch( info, info->rxtoken ) == RX_ERR )
		goto error ;
	    rc = rxParseFactor( info ) ;
	    if ( rc == NULL )		/* and の右側の項がない */
		goto error ;
	    tmpc = rxNodeNewBranch( RX_OP_AND, lc, rc ) ;
	    if ( tmpc == NULL ) {
		rxNodeFree( rc ) ;
		goto error ;
	    }
	    lc = tmpc ;
	    break ;

	case RX_TOKEN_ANDNOT:
	    if ( rxParseMatch( info, info->rxtoken ) == RX_ERR )
		goto error ;
	    rc = rxParseFactor( info ) ;
	    if ( rc == NULL )		/* notの右側の項がない */
		goto error ;
	    tmpc = rxNodeNewBranch( RX_OP_ANDNOT, lc, rc ) ;
	    if ( tmpc == NULL ) {
		rxNodeFree( rc ) ;
		goto error ;
	    }
	    lc = tmpc ;
	    break ;

	case RX_TOKEN_LPAREN:
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
	    goto error ;		/* 論理演算子なしに、論理演算 */
					/* 子以外のトークンの直後に  */
					/* '(' が来た */

	default:
	    return lc ;
	}
    }

 error:
    rxNodeFree( lc ) ;
    return NULL ;
}


/*
 * 括弧、部分パターンの処理
 */
static rxNode*
rxParseFactor( info )
    rxParseInfo *info ;
{
    rxNode *lc ;

    switch( info->rxtoken ) {
    case RX_TOKEN_LPAREN:
	if ( rxParseMatch( info, info->rxtoken ) == RX_ERR )
	    return NULL ;
	lc = rxParseExpr( info ) ;
	if ( lc == NULL )
	    return NULL ;
	if ( rxParseMatch( info, RX_TOKEN_RPAREN ) == RX_ERR )
	    goto error ;		/* '('が閉じられずに末尾に来た */
	return lc ;

    default:
	if ( info->rxtoken < 0 ) {		/* 正規表現が来るべきところ */
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
	    return NULL ;
	}
	lc = rxNodeNewLeaf( info->rxtoken ) ;
	if ( lc == NULL )
	    return NULL ;
	if ( rxParseMatch( info, info->rxtoken ) == RX_ERR )
	    goto error ;
	return lc ;
    }

 error:
    rxNodeFree( lc ) ;
    return NULL ;
}


/*
 * つぎのトークンを得る
 */
static int
rxParseMatch( info, t )
    rxParseInfo *info ;
    int		t ;		/* 現在のあるべきトークン */
{
    if ( info->rxtoken != t ) {
	RX_ERROR_SET(RX_ERR_INVALIDEXPR) ;
	return RX_ERR ;		/* '(' の後に正規表現あるが、閉じられ */
				/* ていない場合など	*/
    }
    info->rxtoken = rxParseLex( info ) ;
    if ( info->rxtoken == RX_ERR )
	return RX_ERR ;
    return RX_OK ;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * 部分パターンを保持する関数群
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * 領域の初期化
 */
static void
rxPatternInit( info )
    rxParseInfo *info ;
{
    info->rxbufs = NULL ;		/* free は上位が行なうこと */
    info->rxbufs_num = 0 ;
    info->rxbufs_size = 0 ;
    info->rx_ptype = NULL ;		/* free は上位が行なうこと */
}


/*
 * パターンの追加
 *	失敗時のメモリ解放は上位関数で行なうこと
 */
static int
rxPatternAppend( info, expr, len, ptype )
    rxParseInfo *info ;
    ModUnicodeChar	*expr ;		/* 部分パターンの先頭 */
    int		len ;		/* 部分パターンの長さ */
    int		ptype ;		/* 部分パターンのタイプ */
{

	if ( info->rxbufs_size == 0 ) {
		; assert( info->rxbufs == NULL && info->rx_ptype == NULL && info->rxbufs_num == 0 ) ;
		info->rxbufs = (ModUnicodeChar**)MEMALLOC(
					sizeof(ModUnicodeChar*)*RX_PARSE_ARRAY_INC ) ;
		info->rx_ptype = (int*)MEMALLOC(sizeof(ModUnicodeChar*)*RX_PARSE_ARRAY_INC ) ;
		if ( info->rxbufs == NULL || info->rx_ptype == NULL ) {
			RX_ERROR_SET(RX_ERR_MEMALLOC) ;
			return RX_ERR ;
		}
		info->rxbufs_size = RX_PARSE_ARRAY_INC ;

    } else if ( info->rxbufs_size <= info->rxbufs_num ) {
		ModUnicodeChar	**bufs ;
		int	*ptmp ;
		bufs = (ModUnicodeChar**)MEMRESIZE( info->rxbufs,
				sizeof(ModUnicodeChar*)*(info->rxbufs_size + RX_PARSE_ARRAY_INC),
				sizeof(ModUnicodeChar*)*info->rxbufs_size ) ;

		ptmp = (int*)MEMRESIZE( info->rx_ptype,
				sizeof(ModUnicodeChar*)*(info->rxbufs_size + RX_PARSE_ARRAY_INC),
				sizeof(ModUnicodeChar*)*info->rxbufs_size ) ;
		if ( bufs == NULL || ptmp == NULL ) {
	    	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	    	return RX_ERR ;
		}
		info->rxbufs = bufs ;
		info->rx_ptype = ptmp ;
		info->rxbufs_size += RX_PARSE_ARRAY_INC ;
    }

#ifdef RX_UNICODE
	info->rxbufs[info->rxbufs_num] = (ModUnicodeChar*)MEMALLOC(sizeof(ModUnicodeChar) * (len+1)) ;
#else /* RX_UNICODE */
	info->rxbufs[info->rxbufs_num] = MEMALLOC( len + 1 ) ;
#endif /* RX_UNICODE */

	if ( info->rxbufs[info->rxbufs_num] == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		return RX_ERR ;
    }

#ifdef RX_UNICODE
	(void)memcpy( info->rxbufs[info->rxbufs_num], expr, sizeof(ModUnicodeChar)*len ) ;
	/* (void)memcpy( info->rxbufs[info->rxbufs_num], expr, len ) ; */
#else /* RX_UNICODE */
	(void)strncpy( info->rxbufs[info->rxbufs_num], expr, len ) ;
#endif /* RX_UNICODE */

#if 0
for(xx=0; xx < len; xx++) {
	printf("info->rxbufs[%d][%d]=\'%02x\'\n", info->rxbufs_num, xx,info->rxbufs[info->rxbufs_num][xx]);
}
#endif // 0


	info->rxbufs[info->rxbufs_num][len] = '\0' ;
	info->rx_ptype[info->rxbufs_num] = ptype ;

    return info->rxbufs_num ++ ;
}


/*
 * 領域の解放
 */
static void
rxPatternFree( info )
    rxParseInfo *info ;
{
    if ( info->rxbufs ) {
	int n ;
	for ( n = 0 ; n < info->rxbufs_num ; n ++ ) {
	    if ( info->rxbufs[n] ) {
#ifdef RX_UNICODE
		MEMFREE( info->rxbufs[n], ModUnicodeCharLength(info->rxbufs[n]) + 1 ) ;
#else /* RX_UNICODE */
		MEMFREE( info->rxbufs[n], strlen(info->rxbufs[n]) + 1 ) ;
#endif /* RX_UNICODE */
	    }
	}
	MEMFREE( info->rxbufs, info->rxbufs_num*sizeof(ModUnicodeChar*) ) ;
	MEMFREE( info->rx_ptype, info->rxbufs_num*sizeof(int) ) ;
	info->rxbufs_num = 0 ;
	info->rxbufs = 0 ;
	info->rx_ptype = 0 ;
    }
}

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
