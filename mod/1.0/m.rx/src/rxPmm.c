/* -*-Mode: C; tab-width: 4;-*-
 *
 * rxPmm.c --- 複数検索語の照合
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
#include <assert.h>
#include <stdlib.h>

#include "rx.h"
#include "rxPmm.h"
#include "rxPmmLocal.h"
#include "rxDefs.h"
#include "rxModUnicodeOperations.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#include "rxModUnicodeChar.h"
#include "rxModUnicodeOperations.h"

STATIC_FUNC( int,	rxPmmPath1,( rxStateArray*,rxOutNode**,int*,ModUnicodeChar**,int, char* )) ;
STATIC_FUNC( int,	rxPmmPath2,( rxStateArray*,int*,rxOutNode**,int, char* )) ;

STATIC_FUNC( int,	rxPmmPath3,( rxStateArray*,int*,int )) ;

STATIC_FUNC( int,	rxPmmEnter,( rxStateArray*,rxOutNode**,int*,ModUnicodeChar*,int,int , char*)) ;

STATIC_FUNC( rxPmmLoc*,	rxPmmLocNew,( ModUnicodeChar*,ModUnicodeChar*,int )) ;

STATIC_FUNC( rxOutNode**,	rxPmmOutInit,( int )) ;
STATIC_FUNC( rxOutNode**,	rxPmmOutRealloc,( rxOutNode**, int, int )) ;
STATIC_FUNC( void,	rxPmmOutFree,( rxOutNode**, int )) ;
STATIC_FUNC( int,	rxPmmOutInsert,( rxOutNode**, int, int )) ;
STATIC_FUNC( int,	rxPmmOutMerge,( rxOutNode**, int, int )) ;
STATIC_FUNC( int,	rxPmmOutExistp,( rxOutNode**, int, int )) ;
#ifdef DEBUG
STATIC_FUNC( void,	rxPmmOutDump,( rxOutNode**, int, FILE* )) ;
#endif

STATIC_FUNC( rxPmmQueue*,	rxPmmQueueAlloc,( int )) ;
STATIC_FUNC( void,	rxPmmQueueFree,( rxPmmQueue* )) ;
STATIC_FUNC( void,	rxPmmQueuePush,( rxPmmQueue*, int )) ;
STATIC_FUNC( int,	rxPmmQueuePop,( rxPmmQueue* )) ;

/*======================================================================*
 * rxPmmCompile		文字列照合オートマトンの作成
 *----------------------------------------------------------------------*
 * 【戻り値】
 * ハンドルポインタ:	処理成功
 * NULL:		処理失敗
 *----------------------------------------------------------------------*
 * 【説明】
 *  ・文字列照合オートマトン（ＰＭＭ）を作成する
 *----------------------------------------------------------------------*
 * 【参考】
 *  ・有川節夫, 篠原武: 文字列パターン照合アルゴリズム,
 *	コンピュータソフトウェア, Vol.4, No.2(1987), pp.2-23
 *  ・バイト単位で照合し、２／３バイト文字境界の処理のために中間ノード
 *	を用意する
 *======================================================================*/
rxPmmHandle*
rxPmmCompile( pat, patnum )
	ModUnicodeChar	**pat ;		/* 照合文字列の配列 */
	int		patnum ;	/* 照合文字列数 */
{
	rxPmmHandle*	ret ;
	int		*failTable = NULL ;	/* fail 表 */
	int		p, patlen = 0, i, j ;
	int		statenum ;

	char	*UpperLowerTable = NULL;

	/* ハンドラのメモリ確保 */
	ret = (rxPmmHandle*)MEMALLOC( sizeof( rxPmmHandle ) ) ;
	if ( ret == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		return NULL ;
	}

	ret->charlen = 0;

	/* PmmHandle->patLen = 各パターンの長さ */
	/* バターン数分確保 */
	ret->patLens = (int*)MEMALLOC( sizeof(int)*patnum ) ;
	if ( ret->patLens == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		goto error ;
	}

	/* 各パターンの長さの合計 → patlen */
	for ( p = 0 ; p < patnum ; p ++ )
	patlen += (pat[p]) ? (int)ModUnicodeCharLength( pat[p] ) : 0 ;

	/* PmmHandleにパターン数セット */
	ret->patNum = patnum ;

	/* 状態数 = 各パターンの長さの合計 + RX_EXTRA_STATE */
	/* Unicodeでは中間状態は必要ないので RX_EXTRA_STATE=1(初期状態の分)? */

	/* バイト単位で評価するので、状態数は(各パターンの長さの合計*2)+中間状態 */ 
	/* 但し中間状態は一つ */

	/* Unicodeの場合はpatlenには状態数 をセットする */

	/* RX_EXTRASTATE = 1であること → 確認 */
	patlen = patlen*sizeof(ModUnicodeChar);
	statenum = patlen + RX_EXTRA_STATE ;

	/* UpperLowerTable の初期化 */ 
	UpperLowerTable = (char *)MEMALLOC(sizeof(char)*statenum);
	if ( UpperLowerTable == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		goto error ;
	}

	for ( i = 0 ; i < statenum ; i ++ ) {
		*(UpperLowerTable+i) = -1;
	}
	/* 初期状態(INIT_STATE) は1(Upper) */
	*UpperLowerTable = 1;

	/* goto 表の初期化 */
	/*
	 * gotoTableに
	 * 状態数(statenum)×sizeof(rxStateArray)分のメモリを確保する
	 */
	ret->gotoTable = (rxStateArray*)MEMALLOC( statenum*sizeof(rxStateArray) ) ;
	if ( ret->gotoTable == NULL ) {
	RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	goto error ;
	}

	/* gotoTableをRX_FAIL_STATEで初期化 */
	for ( i = 0 ; i < statenum ; i ++ ) {
		for ( j = 0 ; j < RX_ALPHABET_SIZE ; j ++ ) {
			ret->gotoTable[i][j] = RX_FAIL_STATE ;
		}
	}

	/* out 表の初期化 */
	ret->outTable = rxPmmOutInit( statenum ) ;
	if ( ret->outTable == NULL )
		goto error ;

	/* パス１：goto 表の作成 */
	statenum = rxPmmPath1( ret->gotoTable, ret->outTable, ret->patLens,
			   pat, patnum,  UpperLowerTable) ;

	if ( statenum == RX_ERR )
		goto error ;
	else if ( statenum < (patlen + RX_EXTRA_STATE) ) {
		/* 接頭部分が共通のパターンがあり、内部状態数が少なくてすむ場合 */
		ret->gotoTable = (rxStateArray*)MEMRESIZE( ret->gotoTable,
						statenum*sizeof(rxStateArray),
						(patlen + RX_EXTRA_STATE)*sizeof(rxStateArray) ) ;
		if ( ret->gotoTable == NULL ) {
			RX_ERROR_SET(RX_ERR_MEMALLOC) ;
			goto error ;
		}
		ret->outTable = rxPmmOutRealloc( ret->outTable, statenum,
										 patlen + RX_EXTRA_STATE ) ;
		if ( ret->outTable == NULL ) {
			RX_ERROR_SET(RX_ERR_MEMALLOC) ;
			goto error ;
		}
	}

	/* fail 表の初期化 */
	/* 状態数分だけ確保 */
	failTable = (int*)MEMALLOC( statenum*sizeof(int)) ;
	if ( failTable == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		goto error ;
	}

	/* RX_FAIL_STATEで初期化 */
	for ( i = 0 ; i < statenum ; i ++ )
	failTable[i] = RX_FAIL_STATE ;

	/* パス２：fail 表の作成 */
	/* RX_INIT_STATEに戻るもののみ作成 ？*/
	if ( rxPmmPath2( ret->gotoTable, failTable, ret->outTable,
		  statenum, UpperLowerTable ) == RX_ERR )
		goto error ;

	/* パス３：最小化（DFA化） */
	if ( rxPmmPath3( ret->gotoTable, failTable, statenum ) == RX_ERR )
		goto error ;

	/* 最小化すれば fail 表は不要 */
	MEMFREE( failTable, statenum*sizeof(int) ) ;

	MEMFREE(UpperLowerTable, (patlen + RX_EXTRA_STATE)*(sizeof(char)));

	ret->stateNum = statenum ;
	ret->lastState = RX_INIT_STATE ;

	return ret ;

 error:
	if ( failTable ) {
		MEMFREE( failTable, statenum*sizeof(int) ) ;
	}
	rxPmmFree( ret ) ;

	if(UpperLowerTable) {
		MEMFREE(UpperLowerTable, (patlen + RX_EXTRA_STATE)*(sizeof(char)));
	}

	return NULL ;
}


/*======================================================================*
 * rxPmmSetText		照合対象文字列のセット
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *======================================================================*/
void
rxPmmSetText( pmm, text, textlen )
	rxPmmHandle	*pmm ;		/* ハンドル */
	ModUnicodeChar	*text ;		/* 照合対象 */
	int		textlen ;	/* 照合対象長 */
{
	; assert( pmm ) ;

	pmm->text = text ;

	/* Unicodeの場合はバイト数ではなく文字数をセットしている */
	pmm->len = textlen ;		/* NULL ターミネイトまで照合す */
					/* る場合、(-1) が来るが、その */
					/* ままでよい */
	pmm->lastState = RX_INIT_STATE ;	/* 初期状態 */

	pmm->charlen = (int)ModUnicodeCharLength(pmm->text) * 2;
}


/*======================================================================*
 * pmmStep		文字列照合
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *----------------------------------------------------------------------*
 * 【説明】
 *  ・テキストで、はじめに一致する（複数の）パターンを発見する
 *  ・loc に照合位置情報がセットされる。
 *	loc が NULL の場合、新たな照合結果のみが返される
 *	loc が NULL でない場合、これまでの照合結果に新たな照合結果が
 * 	連結、返される
 *======================================================================*/
int
rxPmmStep( pmm, loc )
	rxPmmHandle	*pmm ;		/* 文字照合ハンドル */
	rxPmmLoc	**loc ;		/* 文字照合位置 */
{
	int  count;
	unsigned char	*p, c ;
	rxStateArray	*gtable ;
	rxOutNode	**otable, *tmp ;
	int		state, num = 0, remlen ;
	rxPmmLoc	*head = NULL, *tail = NULL ;
#ifndef NDEBUG
	int		statenum ;
#endif

	; assert( pmm && pmm->text ) ;	/* 引数の検査 */

	/* 
	 * 文字列照合の部分は１バイト単位で照合しているため 
	 */
	if(pmm->len > 0) {
		remlen = pmm->len * sizeof(ModUnicodeChar);
	} else {
		remlen = pmm->len;
	}
	if ( remlen == 0 )			/* すでに末尾に達している */
		return RX_FALSE ;

	gtable = pmm->gotoTable ;
	otable = pmm->outTable ;
	state = pmm->lastState ;
#ifndef NDEBUG
	statenum = pmm->stateNum ;
#endif
	; assert( 0 <= state && state < statenum ) ;

	count = 0;
	for (p = (unsigned char*)pmm->text; count < pmm->charlen ; p++, count++) {
		c = *p;

		state = gtable[state][c] ;

		; assert( 0 <= state && state < statenum ) ;
		if ( otable[state] ) {
			/* 照合文字列を発見した */
#ifdef DEBUG
			if ( rxDebugMode > 1 )
				fprintf( stderr, "FOUND[%d]:", state ) ;
#endif
			for ( tmp = otable[state] ; tmp ; tmp = tmp->next ) {
				/* 照合文字列の記録（複数あり) */
#ifdef DEBUG
				if ( rxDebugMode > 1 )
					fprintf( stderr, " %d", tmp->id ) ;
#endif
				; assert( 0 <= tmp->id && tmp->id < pmm->patNum ) ;

				if ( head == NULL ) {
					/* 照合位置の記録: １個目の出現 */

					/* ここでwarrning */
					head = rxPmmLocNew(
						((pmm->text)+((count+1)/2))-((pmm->patLens[tmp->id])/2),
						((pmm->text)+((count+1)/2)), tmp->id );

					if ( head == NULL )
						goto error ;
					tail = head ;
				} else {
					/* 照合位置の記録: ２個目以降の出現 */

					/* ここでwarrning */
					tail->next = rxPmmLocNew(
						((pmm->text)+((count+1)/2))-((pmm->patLens[tmp->id])/2),
						((pmm->text)+((count+1)/2)), tmp->id );

					if ( tail->next == NULL )
						goto error ;
					tail = tail->next ;
				}
				num ++ ;
			}
#ifdef DEBUG
			if ( rxDebugMode > 1 )
				fprintf( stderr, "\n" ) ;
#endif
			--remlen ;
			break ;
		}
		if ( --remlen == 0 )	/* 末尾に達した */
			break ;

	} /* for */

	if ( loc )
		*loc = head ;
	else
		rxPmmLocFree( head ) ;	/* loc==NULL の場合、位置情報は返さない */

	pmm->lastState = state ;	/* 最終状態を記録する */
	if (count > 0) {
		pmm->text = (pmm->text)+((count+1)/2);
		; assert(pmm->charlen >= count);
		pmm->charlen = pmm->charlen - count - 1;
	}

	if(remlen > 0) {
		/* 
		 * rxPmmStep()内部では１バイト単位で照合するが、
		 * pmmは文字数情報を保持しているため。
		 * 文字数へ変換
		 */ 
		pmm->len = remlen / sizeof(ModUnicodeChar);
	} else {
		pmm->len = remlen ;
	}
	return num ;

 error:
	pmm->lastState = 0 ;
	pmm->text = NULL ;
	pmm->len = 0 ;
	pmm->charlen = 0;
	rxPmmLocFree( head ) ;
	return RX_ERR ;
}


#ifdef RX_PMMADVANCE
/*======================================================================*
 * pmmAdvance		文字列照合
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *----------------------------------------------------------------------*
 * 【説明】
 *  ・テキストで、文頭から一致する（複数の）パターンを発見する
 *  ・loc に照合位置情報がセットされる。
 *	loc が NULL の場合、新たな照合結果のみが返される
 *	loc が NULL でない場合、これまでの照合結果に新たな照合結果が
 * 	連結、返される
 *======================================================================*/
int
rxPmmAdvance( pmm, loc )
	rxPmmHandle	*pmm ;		/* 文字照合ハンドル */
	rxPmmLoc	**loc ;		/* 文字照合位置 */
{
	unsigned ModUnicodeChar	*p, c ;
	rxStateArray	*gtable ;
	rxOutNode	**otable, *tmp ;
	int		state, num = 0, remlen ;
	rxPmmLoc	*head = NULL, *tail = NULL ;
#ifndef NDEBUG
	int		statenum ;
#endif

	; assert( pmm && pmm->text ) ;	/* 引数の検査 */

	if(pmm->len > 0) {
		remlen = pmm->len * sizeof(ModUnicodeChar);
	} else {
		remlen = pmm->len;
	}

	if ( remlen == 0 )			/* すでに末尾に達している */
		return RX_FALSE ;

	gtable = pmm->gotoTable ;
	otable = pmm->outTable ;
	state = pmm->lastState ;
#ifndef NDEBUG
	statenum = pmm->stateNum ;
#endif
	; assert( 0 <= state && state < statenum ) ;

	for ( p = (unsinged ModUnicodeChar*)(pmm->text) ; c = *p++ ; ) {
		state = gtable[state][c] ;
		; assert( 0 <= state && state < statenum ) ;
		if ( otable[state] ) {
			/* 照合文字列を発見した */
#ifdef DEBUG
		if ( rxDebugMode > 1 )
			fprintf( stderr, "FOUND[%d]:", state ) ;
#endif
		for ( tmp = otable[state] ; tmp ; tmp = tmp->next ) {
			/* 照合文字列の記録（複数あり) */
#ifdef DEBUG
			if ( rxDebugMode > 1 )
				fprintf( stderr, " %d", tmp->id ) ;
#endif
			; assert( 0 <= tmp->id && tmp->id < pmm->patNum ) ;

			if ( (p - pmm->patLens[tmp->id]) != pmm->text )
				continue ;	/* 先頭に一致しないので照合ではない */

			if ( head == NULL ) {
				/* 照合位置の記録: １個目の出現 */
				head = rxPmmLocNew( pmm->text, p, tmp->id ) ;
				if ( head == NULL )
					goto error ;
				tail = head ;
			} else {
				/* 照合位置の記録: ２個目以降の出現 */
				tail->next = rxPmmLocNew( pmm->text, p, tmp->id ) ;
				if ( tail->next == NULL )
					goto error ;

				tail = tail->next ;
			}
			num ++ ;
		}
#ifdef DEBUG
		if ( rxDebugMode > 1 )
			fprintf( stderr, "\n" ) ;
#endif
		--remlen ;
		break ;

	} else if ( !state ) {
		break ;		/* Advance では、失敗したらすぐ fail */
	}

	if ( --remlen == 0 )	/* 末尾に達した */
		break ;
	}

 fini:
	if ( loc )
		*loc = head ;
	else
		rxPmmLocFree( head ) ;	/* loc==NULL の場合、位置情報は返さない */

	pmm->lastState = state ;	/* 最終状態を記録する */
	pmm->text = (ModUnicodeChar*)p ;	/* つぎの照合位置を記録する */

	pmm->len = remlen ;

	if(remlen > 0) {
		pmm->len = remlen / sizeof(ModUnicodeChar);
	} else {
		pmm->len = remlen;
	}
	return num ;

 error:
	pmm->lastState = 0 ;
	pmm->text = NULL ;
	pmm->len = 0 ;
	pmm->charlen = 0;
	rxPmmLocFree( head ) ;
	return RX_ERR ;
}
#endif


/*======================================================================*
 * rxPmmFree		文字列照合オートマトンの解放
 *----------------------------------------------------------------------*
 * 【戻り値】
 * なし
 *----------------------------------------------------------------------*
 * 【説明】
 *  ・文字列照合オートマトン（ＰＭＭ）のメモリを解放する
 *======================================================================*/
void
rxPmmFree( pmm )
	rxPmmHandle	*pmm ;		/* 文字照合ハンドル */
{
	if ( pmm ) {
		if ( pmm->patLens ) {
			MEMFREE( pmm->patLens, pmm->stateNum*sizeof(int) ) ;
		}
		if ( pmm->gotoTable ) {
			MEMFREE( pmm->gotoTable, pmm->stateNum*sizeof(rxStateArray) ) ;
		}
		if ( pmm->outTable ) {
			rxPmmOutFree( pmm->outTable, pmm->stateNum ) ;
		}
		MEMFREE( pmm, sizeof(rxPmmHandle) ) ;
	}
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * 照合結果を操作する関数群
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * 照合位置記録ノードの生成
 */
static rxPmmLoc*
rxPmmLocNew( st, ed, id )
	ModUnicodeChar	*st ;		/* 開始位置 */
	ModUnicodeChar	*ed ;		/* 終了位置 */
	int		id ;		/* パターン番号 */
{
	rxPmmLoc*	tmp ;

	tmp = (rxPmmLoc*)MEMALLOC( sizeof(rxPmmLoc) ) ;
	if ( tmp == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		return NULL ;
	}

	tmp->st = st ;
	tmp->ed = ed ;
	tmp->id = id ;
	tmp->next = NULL ;

	return tmp ;
}


#ifdef RX_PMMLOCSORT
/*
 * rxPmmLocSort		照合位置記録ノードリストの出現位置によるソート
 */
void
rxPmmLocSort( top )
	rxPmmLoc	**top ;		/* 文字照合位置 */
{
	rxPmmLoc*	next ;

	; assert( top && *top ) ;

	next = (*top)->next ;
	if ( next ) {
	rxPmmLocSort( &next ) ;

	while ( ((*top)->st > next->st) ||
			(((*top)->st == next->st)&&((*top)->ed > next->ed)) ) {
		rxPmmLoc* tmp = *top ;

		tmp->next = next->next ;
		next->next = tmp ;
		*top = next ;
		next = tmp ;

		rxPmmLocSort( &next ) ;
	}
	(*top)->next = next ;
	}
}		
#endif


/*
 * 照合位置記録ノードの解放
 */
void
rxPmmLocFree( loc )
	rxPmmLoc	*loc ;		/* 文字照合位置 */
{
	rxPmmLoc*	tmp ;

	for ( ; loc ; loc = tmp ) {
		tmp = loc->next ;
		MEMFREE( loc, sizeof(rxPmmLoc) ) ;
	}
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * goto 表関係
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * goto 表の作成（有川節夫:文字列パターン照合アルゴリズム:Algorithm 7）
 */
static int
rxPmmPath1( gtable, otable, patlens, pats, patnum, upperLower )
	rxStateArray	*gtable ;	/* goto 表 */
	rxOutNode	**otable ;	/* out 表 */
	int		*patlens ;	/* パターン長 */
	ModUnicodeChar	**pats ;	/* パターン */
	int		patnum ;	/* パターン数 */
	char*	upperLower;
{
	int		p, a, newstate = RX_INIT_STATE + 1 ;

	for ( p = 0 ; p < patnum ; p++ ) {
	if ( pats[p] == NULL )
		continue ;
	newstate = rxPmmEnter( gtable, otable, &patlens[p],
				   pats[p], p, newstate, upperLower ) ;

#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "p=%d, newstate=%d\n", p, newstate ) ;
#endif
	if ( newstate == RX_ERR )
		return RX_ERR ;
	}
#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "newstate=%d\n", newstate ) ;
#endif

	/* 状態数のMAX=RX_ALPHABET_SIZE ？ */
	for ( a = 0x00 ; a < RX_ALPHABET_SIZE ; a++ ) {
		/*
		 *
		 *		   集合I以外
		 *			   ←		   許可される入力集合=I
		 *  newsstate		   INIT_STATE   →   その他状態
		 *			   →
		 *		  すべての入力
		 *
		 *
		 */ 

		if ( gtable[RX_INIT_STATE][a] == RX_FAIL_STATE ) {
			gtable[RX_INIT_STATE][a] = newstate ;
		}

		if ( gtable[newstate][a] == RX_FAIL_STATE ) {
			gtable[newstate][a] = RX_INIT_STATE ;
		}

	}

	return ++newstate ;
}


/*
 * pat に関する goto 表の更新
 */
static int
rxPmmEnter( gtable, otable, patlen, pat, patid, newstate, upperLower )
	rxStateArray	*gtable ;	/* goto 表 */
	rxOutNode	**otable ; 	/* out 表 */
	int		*patlen ;	/* パターン長 */
	ModUnicodeChar	*pat ;		/* パターン */
	int		patid ;		/* パターン番号 */
	int		newstate ;	/* ステータス */
	char*	upperLower;
{
	/* 状態遷移はバイト単位で行うためUnicodeの文字列をchar文字列へ変換 */
	int charPatLen;
	unsigned char	*p, c ;
	int		state = RX_INIT_STATE ;
	int		is_escape = 0, len = 0 ;
	int count = 0;

	*patlen = 0 ;

	/* ここで pat=UnicodeChar[]→char[] を行う */
	/* 変換用関数を作成 */
	/* Unicodeの文字列をchar文字列へ変換 */
	charPatLen = (int)ModUnicodeCharLength(pat) * 2;


	/* 登録済みの対象文字列と接頭部が一致する部分の処理 */
	count = 0;
	for ( p = (unsigned char*)pat ;count < charPatLen; count++) {
		c = *p ; 
		p++;
#ifdef MCH_INTEL
		/* バイトオーダがリトルエンディアンの場合 */
		/* Unicode(0x0000)→char(RX_ESCAPE)+char(0x00) のため */
		if (*p == 0x00) {
			if(*(upperLower+state) == 1 && !is_escape && c == RX_ESCAPE ) {
				/* エスケープ文字 */
				is_escape = 1;
				p++;
				count++;
				continue;
			} 
		}
#else
		/* バイトオーダがビッグエンディアンの場合 */
		/* Unicode(0x0000)→char(0x00)+char(RX_ESCAPE) のため */
		if (c == 0x00) {
			if( *(upperLower+state) == 1 && !is_escape && *p == RX_ESCAPE ) {
				/* エスケープ文字 */
				is_escape = 1;
				p++;
				count++;
				continue;
			} 
		}
#endif

		if ( gtable[state][c] == RX_FAIL_STATE ) {
			p--;
			/* count--; */
			break ;		/* 一致しない部分が見つかった */
		}

		state = gtable[state][c] ;
		len ++ ;
		is_escape = 0 ;		/* '\\' が検索語の場合にも正しく処理す */
							/* るために、is_escape のクリアは上の  */
							/* break より後になければならない */
	}

	/* 一致する接頭部以降の処理 */
	/* 新しい状態(newstate)を生成する  */

	/*
	 * 現在の状態(state)に遷移可能な入力(*p)があった場合newstateに遷移する
	 * gtable[state][c] = newstate ;
	 */

	for (;count < charPatLen; count++) {
		c = *p ;
		p++; 
#ifdef MCH_INTEL
		/* バイトオーダがリトルエンディアンの場合 */
		/* Unicode(0x0000)→char(RX_ESCAPE)+char(0x00) のため */
		if (*p == 0x00) {
			if( *(upperLower+state) == 1 && !is_escape && c == RX_ESCAPE ) {
				/* エスケープ文字 */
				is_escape = 1;
				p++;
				count++;
				continue;
			} 
		}
#else
		/* バイトオーダがビッグエンディアンの場合 */
		/* Unicode(0x0000)→char(0x00)+char(RX_ESCAPE) のため */
		if (c == 0x00) {
			if( *(upperLower+state) == 1 && !is_escape && *p == RX_ESCAPE ) {
				/* エスケープ文字 */
				is_escape = 1;
				p++;
				count++;
				continue;
			}
		}
#endif
		if(*(upperLower+state) == 1) {
			/*
			 * stateはUpper(Unicodeの上位バイト)である
			 * 次のnewstateはLower(Unicodeの下位バイト)になる
			 */
			*(upperLower+newstate) = 0;
		} else {
			/*
			 * stateはUpper(Unicodeの下位バイト)である
			 * 次のnewstateはLower(Unicodeの上位バイト)になる
			 */
			*(upperLower+newstate) = 1;
		}
		gtable[state][c] = newstate ;

		state = newstate ++ ;
		len ++ ;

		if ( newstate == RX_MAX_STATE ) {	/* short のレンジを超えた */
			RX_ERROR_SET(RX_ERR_COMPLEXEXPR) ;
			return RX_ERR ;
		}
		is_escape = 0 ;
	}

	/* 照合状態（状態IDとパターンIDの対応）の記録 */
	/* out表への追加 */
	
	if ( rxPmmOutInsert( otable, state, patid ) == RX_ERR ) {
		return RX_ERR ;
	}

	/* patlenにはとりあえず状態数をセットする。後で確認する！！ */
	*patlen = len ;

	return newstate ;
}


/*
 * fail 表の作成（有川節夫:文字列パターン照合アルゴリズム:Algorithm 8）
 */

static int
rxPmmPath2( gtable, ftable, otable, statenum, upperLower )
	rxStateArray	*gtable ;	/* goto 表 */
	int		*ftable ;	/* fail 表 */
	rxOutNode	**otable ;	/* out 表 */
	int		statenum ;	/* 状態数 */
	char*	upperLower;
{
	rxPmmQueue*	queue ;
	int		a, s, r, state ;

	int		midstate = statenum - RX_EXTRA_STATE + 1 ;

	queue = rxPmmQueueAlloc( statenum ) ; /* 状態数分のキューを用意 */
	if ( queue == NULL ) {
		return RX_ERR ;
	}

	/* 初期状態から遷移可能な状態を集め、その失敗時のイプシロン遷移先 */
	/* を設定する。ただし、遷移先がINIT_STATE/midstate場合は行わない。 */

	/* 状態数のMAX=RX_ALPHABET_SIZE-1 ？ */
	for ( a = 0 ; a < RX_ALPHABET_SIZE ; a++) {
		/* RX_INIT_STATEへの入力a */
		/* gotoの先がINIT_STATEの場合は何もしない */
		if (((s = gtable[RX_INIT_STATE][a]) == RX_INIT_STATE )
					|| ((s = gtable[RX_INIT_STATE][a]) == midstate)) {
			continue ;
		}

		/* INIT_STATE以外の場合 */
		/* キューにプッシュ */
		rxPmmQueuePush( queue, s );

		/* fail表のs(INIT_STATEからの遷移先)の値をRX_INIT_STATEにする */

		/*
		 *
		 * 状態によりfail表の行き先が変わる。
		 * 	sが奇数状態の場合 = Unicodeの1バイト目
		 *		2バイト目を読み飛ばす必要あり。midstateへ
		 * 	sが偶数状態の場合 = Unicodeの2バイト目
		 *		INIT_STATEへ
		 *
		 */
		if(*(upperLower+s) == 0) {
		 	/* 	Unicodeの1バイト目 */
			ftable[s] = midstate ;
		} else {
		 	/* 	Unicodeの1バイト目 */
			ftable[s] = RX_INIT_STATE;
		}
	}

	/* 初期状態以外から遷移可能な状態を順次処理する */
	for ( ; (r = rxPmmQueuePop( queue )) != RX_ERR ; ) {
		/* NULL は特別なので 1 からはじめる */
		/* Unicodeの場合は0でよい */

		for ( a = 0 ; a < RX_ALPHABET_SIZE ; a ++ ) {
			if ((s = gtable[r][a]) == RX_FAIL_STATE )  {
				continue ;
			}

			rxPmmQueuePush( queue, s ) ;
			state = ftable[r] ;
			; assert( state != RX_FAIL_STATE ) ;
			while ( gtable[state][a] == RX_FAIL_STATE ) {
				state = ftable[state] ;
				; assert( state != RX_FAIL_STATE ) ;
			}
#ifdef DEBUG
			if ( rxDebugMode > 2 )
			fprintf( stderr, "f[%d] <= %d\n", s, gtable[state][a] ) ;
#endif
			ftable[s] = gtable[state][a] ;
			if ( rxPmmOutMerge( otable, s, ftable[s] ) == RX_ERR ) {
				rxPmmQueueFree( queue ) ;
				return RX_ERR ;
			}
		}
	}

	rxPmmQueueFree( queue ) ;

	return RX_OK ;
}


/*
 * DFA 化（有川節夫:文字列パターン照合アルゴリズム:Algorithm 9）
 */
static int
rxPmmPath3( gtable, ftable, statenum )
	rxStateArray	*gtable ;	/* goto 表 */
	int		*ftable ;	/* fail 表 */
	int		statenum ;	/* 状態数 */
{
	rxPmmQueue*	queue ;
	int		a, s, r ;

	int		midstate = statenum - RX_EXTRA_STATE + 1 ;

	queue = rxPmmQueueAlloc( statenum ) ;
	if ( queue == NULL )
	return RX_ERR ;

	/* 初期状態から遷移可能な状態を集める */
	/* ASCII(C0,space,GL)、ただし NULL は特別なので 1 からはじめる */
	/* UnicodeではNULLも特別扱いしない */

	for ( a = 0 ; a <= 0xff ; a ++ ) {
		if ((( s = gtable[RX_INIT_STATE][a]) == RX_INIT_STATE )
				|| ((s = gtable[RX_INIT_STATE][a]) == midstate )) {
			continue ;
		}

		/* INIT_STATE以外に遷移するものをキューに入れる */
		rxPmmQueuePush( queue, s ) ;
	}

	/* 初期状態以外から遷移可能な状態を順次処理する */
	/* キューから状態(INIT_STATEより遷移可能な)を取出す */
	for ( ; (r = rxPmmQueuePop( queue )) != RX_ERR ; ) {
		/* NULL は特別なので 1 からはじめる */
		/* UnicodeではNULLも特別扱いする必要はない */

		for ( a = 0 ; a <= 0xff ; a ++ ) {
			if ( (s = gtable[r][a]) == RX_FAIL_STATE ) {
			/* 行き先が決まっていないものの遷移先を決める */
#ifdef DEBUG
			if ( rxDebugMode > 2 )
				fprintf( stderr, "GOTO(%d,%d)=%d <= GOTO(%d,%d)=%d\n",
					 r, a, s, ftable[r], a, gtable[ftable[r]][a] ) ;
#endif
				gtable[r][a] = gtable[ftable[r]][a] ;

			} else if ( s > 0 ) {
#ifdef DEBUG
				if ( rxDebugMode > 2 )
					fprintf( stderr, "GOTO(%d,%x)=%d <= queue\n",
				 		r,	a, s ) ;
#endif
				rxPmmQueuePush( queue, s ) ;
			}
		}
	}

	rxPmmQueueFree( queue ) ;

	return RX_OK ;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * out 表関係
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * out 表の初期化
 */
static rxOutNode**
rxPmmOutInit( statenum )
	int		statenum ;	/* 状態数 */
{
	rxOutNode	**ret ;

	ret = (rxOutNode**)MEMALLOC( sizeof(rxOutNode*)*statenum ) ;
	if ( ret == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		return NULL ;
	}
	(void)memset( ret, 0, sizeof(rxOutNode*)*statenum ) ;

	return ret ;
}


/*
 * out 表のリサイズ
 */
static rxOutNode**
rxPmmOutRealloc( otable, newnum, oldnum )
	rxOutNode	**otable ;	/* out 表 */
	int		newnum ;	/* 修正後の状態数 */
	int		oldnum ;	/* 修正前の状態数 */

{
	rxOutNode	**ret ;

	; assert( otable ) ;
	ret = (rxOutNode**)MEMRESIZE( otable, newnum*sizeof(rxOutNode*),
								  oldnum*sizeof(rxOutNode*) ) ;
	if ( ret == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
	}
	return ret ;
}


/*
 * out 表の開放
 */
static void
rxPmmOutFree( otable, onum )
	rxOutNode	**otable ;	/* out 表 */
	int		onum ;		/* 状態数 */
{
	if ( otable ) {
		int 	n ;
		rxOutNode	*tmp, *tmp1 ;

		for ( n = 0 ; n < onum ; n ++ ) {
			for ( tmp = otable[n] ; tmp ; tmp = tmp1 ) {
				tmp1 = tmp->next ;
				MEMFREE( tmp, sizeof(rxOutNode) ) ;
			}
		}
		MEMFREE( otable, onum*sizeof(rxOutNode*) ) ;
	}
}


/*
 * out 表へのデータ（状態IDとパターンIDの対応）追加
 */
static int
rxPmmOutInsert( otable, state, patid )
	rxOutNode	**otable ;	/* out 表 */
	int		state ;		/* 状態 */
	int		patid ;		/* パターン番号 */
{
	rxOutNode	*tmp ;

#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "o[%d] <= %d\n", state, patid ) ;
#endif

	; assert( otable && state >= 0 ) ;
	tmp = otable[state] ;
	if ( tmp && rxPmmOutExistp( otable, state, patid ) == RX_TRUE ) {
		/* すでに登録されているので、何もしない */
		return RX_OK ;
	}

	otable[state] = (rxOutNode*)MEMALLOC( sizeof(rxOutNode) ) ;
	if ( otable[state] == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		return RX_ERR ;
	}
	otable[state]->id = patid ;
	otable[state]->next = tmp ;

	return RX_OK ;
}


/*
 * out 表で、state1 に state2 をマージする
 */
static int
rxPmmOutMerge( otable, state1, state2 )
	rxOutNode	**otable ;	/* out 表 */
	int		state1 ;	/* 状態 */
	int		state2 ;	/* 状態 */
{
	rxOutNode	*tmp ;

#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "omerge[%d,%d]:", state1, state2 ) ;
#endif

	; assert( otable && state2 >= 0 ) ;
	for ( tmp = otable[state2] ; tmp ; tmp = tmp->next ) {
	if ( rxPmmOutInsert( otable, state1, tmp->id ) == RX_ERR )
		return RX_ERR ;
	}
	return RX_OK ;
}


/*
 * out 表で、state に patid が記録されているか調べる
 */
static int
rxPmmOutExistp( otable, state, patid )
	rxOutNode	**otable ;	/* out 表 */
	int		state ;		/* 状態 */
	int		patid ;		/* パターン番号 */
{
	rxOutNode	*tmp ;

	; assert( otable && state >= 0 ) ;
	for ( tmp = otable[state] ; tmp ; tmp = tmp->next ) {
	if ( patid == tmp->id )
		return RX_TRUE ;
	}
	return RX_FALSE ;
}


#ifdef DEBUG	
static void
rxPmmOutDump( otable, onum, fp )
	rxOutNode	**otable ;
	int		onum ;
	FILE	*fp ;
{
	int 	n ;
	rxOutNode*	tmp ;

	for ( n = 0 ; n < onum ; n ++ ) {
		fprintf( fp, "%d:", n ) ;
		for ( tmp = otable[n] ; tmp ; tmp = tmp->next ) {
			fprintf( fp, " %d", tmp->id ) ;
		}
		fprintf( fp, "\n" ) ;
	}
}
#endif


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 * queue 関係
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*
 * 初期化
 */
static rxPmmQueue*
rxPmmQueueAlloc( size )
	int		size ;	/* キューの大きさ */
{
	rxPmmQueue*	ret ;

	ret = (rxPmmQueue*)MEMALLOC( sizeof(rxPmmQueue) ) ;
	if ( ret == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
 		return NULL ;
	}
	ret->buf = (int*)MEMALLOC( size*sizeof(int) ) ;
	if ( ret->buf == NULL ) {
		RX_ERROR_SET(RX_ERR_MEMALLOC) ;
		MEMFREE( ret, sizeof(rxPmmQueue) ) ;
		return NULL ;
	}
	ret->sz = size ;
	ret->nm = 0 ;
	ret->st = 0 ;
	ret->ed = 0 ;

	return ret ;
}


/*
 * 開放
 */
static void
rxPmmQueueFree( qh )
	rxPmmQueue	*qh ;	/* キュー */
{
	if ( qh ) {
		if ( qh->buf ) {
			MEMFREE( qh->buf, qh->sz*sizeof(int) ) ;
		}
		MEMFREE( qh, sizeof(rxPmmQueue)  ) ;
	}
}


/*
 * データの追加（末尾にデータを連結する）
 */
static void
rxPmmQueuePush( qh, s )
	rxPmmQueue	*qh ;	/* キュー */
	int		s ;	/* データ（状態） */
{
	; assert( qh ) ;
#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "push() <= %d[%d]\n", s, qh->st ) ;
#endif

	; assert( 0 <= qh->st && qh->st < qh->sz ) ;
	qh->buf[qh->st] = s ;
	qh->st ++ ;
	qh->nm ++ ;

	return ;
}


/*
 * データの取出し
 */
static int
rxPmmQueuePop( qh )
	rxPmmQueue	*qh ;	/* キュー */
{
	int		ret ;

	; assert( qh ) ;
	if ( qh->nm == 0 ) {
		/* データがなくなった時。正常パス */
#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "rxPmmQueuePop(): no data\n" ) ;
#endif
		return RX_ERR ;
	}

	; assert( 0 <= qh->ed && qh->ed < qh->sz ) ;
	ret =  qh->buf[qh->ed] ;
#ifdef DEBUG
	if ( rxDebugMode > 2 )
		fprintf( stderr, "pop() => %d[%d]\n", ret, qh->ed ) ;
#endif

	qh->ed ++ ;
	qh->nm -- ;
	return ret ;
}
	
/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
