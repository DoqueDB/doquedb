/*
 * rxText.c --- テストプログラム
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2023 Ricoh Company, Ltd.
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
#include "rx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET "道内への帰省はピーク、電車・高速道路は混雑$、^道内・毣内・毣汭*稚内の百貨店(デパート;DEPATO?, department-store.\\+/)も混雑"
#define QUERY	"(一日|二日|三日|四日|五日|六日|七日|八日|九日|十日)&した。$"

#ifdef BUFSIZ
#undef BUFSIZ
#define BUFSIZ	65536
#endif

#define STEP	1
#define ADVANCE	2
#define WALK	4

extern char *optarg;
extern int optind, opterr, optopt;

void
print_res( re )
    rxMatchee	*re ;
{
    char *p ;
    int i, j ;

    for ( p = re->stloc ; p < re->edloc ; p ++ )
	putchar( *p ) ;
    putchar( '\n' ) ;
    printf( "+ %d ", re->expid ) ;
    if ( re->bras ) {
	for ( i = 0 ; i < re->nbras ; i ++ ) {
	    printf( "- %d(%d): ", i, re->bras[i].nbra ) ;
	    for ( j = 0 ; j < re->bras[i].nbra ; j ++ ) {
		for ( p = re->bras[i].braslist[j] ; p < re->bras[i].braelist[j] ; p ++ )
		    putchar( *p ) ;
		putchar( ' ' ) ;
	    }
	}
    }
    putchar( '\n' ) ;
}

main( argc,  argv )
    int		argc ;
    char	**argv ;
{
    rxHandle* rx = NULL ;
    rxMatchee re, *rr ;
    char *query, *target, *tmp ;
    int textlen ;
    int c, mode = 0, pmode = STEP, eee = 0 ;
    int i, j, num, n ;
    FILE *fp = stdin ;
    char	buf[BUFSIZ], buf2[BUFSIZ] ;

    while ( (c = getopt( argc, argv, "d:ei:m:M:" )) != -1 )
	switch( c ) {
#ifdef DEBUG
	case 'd':
	    rxDebugMode = atoi( optarg ) ;
	    break ;
	case 'e':
	    eee = 1 ;
	    break ;
	case 'i':
	    if ( (fp = fopen( optarg, "r" )) == NULL ) {
		perror( optarg ) ;
		exit( 1 ) ;
	    }
	    break ;
	case 'm':
	    mode = atoi( optarg ) ;
	    break ;
#endif
	case 'M':
	    pmode = atoi( optarg ) ;
	    break ;
	}

#ifdef RX_MSKANJI
    euc2sjis( buf2, TARGET ) ;
    target = buf2 ;
#else
    target = (argc<(optind+1)) ? TARGET : argv[optind] ;
#endif

    for ( ; fgets( buf, BUFSIZ, fp ) ; ) {
#ifdef DEBUG
	if ( buf[0] == '#' )
	    continue ;
	if ( buf[0] == '!' )
	    break ;
#endif
	buf[strlen(buf)-1] = 0 ;
	if ( tmp = strtok( buf, " " ) ) {
	    if ( (query = (char*)strdup( tmp )) == NULL ) {
#ifdef DEBUG
		fprintf( stderr, "!!! %s\n", tmp ) ;
		continue ;
#endif
	    }
	} else {	/* 空文字列の場合 */
	    if ( (query = (char*)strdup( "" )) == NULL ) {
#ifdef DEBUG
		fprintf( stderr, "!!! \n" ) ;
		continue ;
#endif
	    }
	}
	tmp = strtok( NULL, " " ) ;
	mode = tmp ? atoi(tmp) : 0 ;
	tmp = strtok( NULL, " " ) ;
	textlen = tmp ? atoi(tmp) : RX_NULL_TERMINATE ;

	rx = rxCompile( query ) ;
	if ( rx == NULL ) {
	    printf( "=== '%s' compile failed[%d]\n", query, rxErrorCode  ) ;
	    goto next ;
	} else {
	    printf( "=== '%s' ok\n", query ) ;
#ifdef DEBUG
	    if ( eee > 0 ) 
		goto next ;
#endif
	}
	
	if ( pmode&STEP ) {
	    switch ( rxStep( rx, mode, target, textlen, &re ) ) {
	    case RX_TRUE:
		printf( "--- Step %d [%d]\n", mode, textlen ) ;
		print_res( &re ) ;
		rxMatcheeFree( &re ) ;
		break ;
	    case RX_FALSE:
		printf( "--- Step %d [%d]\n", mode, textlen ) ;
		break ;
	    default:
		printf( "--- Step %d [%d] failed[%d]\n", mode,
		        textlen, rxErrorCode ) ;
		break ;
	    }
	}
	
	if ( pmode&ADVANCE ) {
	    switch ( rxAdvance( rx, mode, target, textlen, &re ) ) {
	    case RX_TRUE:
		printf( "--- Advance %d [%d]\n", mode, textlen ) ;
		print_res( &re ) ;
		rxMatcheeFree( &re ) ;
		break ;
	    case RX_FALSE:
		printf( "--- Advance %d [%d]\n", mode, textlen ) ;
		break ;
	    default:
		printf( "--- Advance %d [%d] failed[%d]\n", mode,
		        textlen, rxErrorCode ) ;
		break ;
	    }
	}
	
	if ( pmode&WALK ) {
	    num = rxWalk( rx, mode, target, textlen, &rr ) ;
	    if ( num > 0 ) {
		printf( "--- Walk %d [%d]\n", mode, textlen ) ;
		printf( "%s\n", target ) ;
		for ( n = 0 ; n < num ; n ++ ) {
		    printf( "%d:", n ) ;
		    print_res( &rr[n] ) ;
		    rxMatcheeFree( &rr[n] ) ;
		}
		free( rr ) ;
	    }
	    else if ( num == 0 )
		printf( "--- Walk %d [%d]\n", mode, textlen ) ;
	    else
		printf( "--- Walk %d [%d] failed[%d]\n", mode,
		        textlen, rxErrorCode ) ;
	}
    next:
	if ( rx ) {
	    rxFree( rx ) ;
	    rx = NULL ;
	}
	if ( query ) {
	    free( query ) ;
	    query = NULL ;
	}
    }/* for */
}

#ifdef RX_MSKANJI
euc2sjis( s1, s2 )
    unsigned char	*s1 ;
    unsigned char	*s2 ;
{
    unsigned char	*s, c1, c2 ;
    unsigned short	val ;

    for ( s = s2 ; c1 = *s ; s ++ ) {
	if ( c1 < 0x80 ) {
	    *s1++ = c1 ;
	    continue ;
	}

	c2 = *++s ;

	if ( c1 < 0xa1 )
	    val = 0x81 ;
	else if ( c1 < 0xdf )
	    val = 0x81 + (c1 - 0xa1)/2 ;
	else
	    val = 0xe0 + (c1 - 0xdf)/2 ;

	val <<= 8 ;

	if ( c1 & 0x01 ) {
	    val += 0x3f ;
	    if ( c2 < 0xa0 )
		;
	    else if ( c2 < 0xe0 )
		val += c2 - 0xa0 ;
	    else
		val += 1 + c2 - 0xa0 ;
	} else {
	    val += 0x9e ;
	    if ( c2 < 0xa0 )
		;
	    else
		val += c2 - 0xa0 ;
	}

	*s1++ = (val&0xff00)>>8 ;
	*s1++ = val&0xff ;
    }
    *s1 = 0 ;
}
#endif

/*
 *	Copyright (c) 1996, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
