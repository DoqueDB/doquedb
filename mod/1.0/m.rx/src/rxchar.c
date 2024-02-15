/*
 * rxchar.c --- 
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

#include "rx.h"
#include "rxDefs.h"
#include "rxchar.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */



/*
 * rxCharGet - テキストバッファから1文字とり出す
 */
rxChar 
rxCharGet(s)
    ModUnicodeChar** s;	/* テキストバッファへのポインタ */
{
    /* unsigned ModUnicodeChar** p = (unsigned ModUnicodeChar**)s; */
    ModUnicodeChar** p = (ModUnicodeChar**)s;
    unsigned int c;

#ifdef RX_UNICODE
	/* 
	 *
	 * UNICODE の場合は文字種 
	 * rxCharType(rxASCII/rxKANA/rxKANJI/rxXKANJI/rxBACKREF/rxCHARSET)は不要と
	 * 思われる。
	 * rxCharGet(s)は単純にsから一文字返す。
	 * 文字種のセットは行わない。
	 * 
	 * マクロ
	 *	RXCHARTYPE(c)   削除？
	 * 	RXCHARVAL(c)    削除？
	 *	RXCHAR(t, c)    削除？
	 * は変更する必要がある。
	 * 
	 */

    if (**p == '\0') {
		return RXCHAR_NULL;
	}

    c = *(*p)++;
	/* 
	 *
	 * rxChar(文字種付き文字)に変換して返す。 
	 * とりあえずrxMOJIとして返す。
	 *
	 * 本当はrxASCIIは0x00であるため何もしてない。
	 * そのまま返してもよい。
	 *
	 */
	return RXCHAR(rxMOJI, c);
	

#else	/* RX_UNICODE */
    if (**p == '\0')
	return RXCHAR_NULL;

    c = *(*p)++;

    if (isprint(c))
	return RXCHAR(rxASCII, c);

#ifndef RX_MSKANJI
    if (c >= GR_BG && c <= GR_ED) {
	/* 全角文字の1バイトめ */
	if (**p >= GR_BG && **p <= GR_ED) {
	    c = (c << 8) | *(*p)++;
	    return RXCHAR(rxKANJI, c);
	}
    }
    else if (c == SS2) {
	/* 半角カナの1バイトめ */
	if (**p >= GR_BG && **p <= GR_ED) {
	    c = *(*p)++;
	    return RXCHAR(rxKANA, c);
	}
    }
    else if (c == SS3) {
	/* 外字の1バイトめ */
	if (((*p)[0] >= GR_BG && (*p)[0] <= GR_ED) && 
	    ((*p)[1] >= GR_BG && (*p)[1] <= GR_ED)) {
	    c = ((*p)[0] << 8) | (*p)[1];
	    (*p) += 2;
	    return RXCHAR(rxXKANJI, c);
	}
    }
#else /* RX_MSKANJI */
    if ((c >= Z1A_BG && c <= Z1A_ED) || (c >= Z1B_BG && c <= Z1B_ED)) {
	/* 全角文字の1バイトめ */
	if ((**p >= Z2A_BG && **p <= Z2A_ED) ||
	    (**p >= Z2B_BG && **p <= Z2B_ED)) {
	    c = (c << 8) | *(*p)++;
	    return RXCHAR(rxKANJI, c);
	}
    }
    else if (c >= 0xa0 && c <= 0xdf) {
	/* 半角カナ */
	return RXCHAR(rxKANA, c);
    }
    else if (c >= Z1C_BG && c <= Z1C_ED) {
	/* IBM拡張文字 */
	if ((**p >= Z2A_BG && **p <= Z2A_ED) ||
	    (**p >= Z2B_BG && **p <= Z2B_ED)) {
	    c = (c << 8) | *(*p)++;
	    return RXCHAR(rxXKANJI, c);
	}
    }
#endif /* RX_MSKANJI */

    /* とりあえずASCII文字ということにしておく */
    return RXCHAR(rxASCII, c);
#endif	/* RX_UNICODE */

}

#ifdef DEBUG
/*
 * rxCharPut - 1文字出力(デバッグ用)
 */
void 
rxCharPut(ch)
    rxChar ch;	/* 出力する文字 */
{
#ifdef RX_UNICODE

	switch (RXCHARTYPE(ch)) {
		case rxMOJI:
			printf("%04x ", RXCHARVAL(ch));
	
    	case rxCHARSET:
			if (ch == RXCHAR_ANY)
				printf("<.>");
			else
				printf("[%04x] ", (int)RXCHARVAL(ch));

			break;

		case rxBACKREF:
			printf("\\0x4", (int)RXCHARVAL(ch));
			break;

		default:
			break;
	}

	if (ch == RXCHAR_END) {
		printf("#");
	}


#else	/* RX_UNICODE */
    switch (RXCHARTYPE(ch)) {
    case rxASCII:
	if (RXCHARVAL(ch) >= GL_BG)
	    putchar(RXCHARVAL(ch));
	else
	    printf("<%02x>", (int)RXCHARVAL(ch));
	break;

    case rxKANJI:
	putchar(RXCHARVAL(ch) >> 8);
	putchar(RXCHARVAL(ch) & 0xff);
	break;

    case rxCHARSET:
	if (ch == RXCHAR_ANY)
	    printf("<.>");
	else
	    printf("[%d]", (int)RXCHARVAL(ch));
	break;

    case rxBACKREF:
	printf("\\%d", (int)RXCHARVAL(ch));
	break;

    default:
	break;
    }

    if (ch == RXCHAR_END) {
	printf("#");
    }
#endif	/* RX_UNICODE */
}
#endif /* DEBUG */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
