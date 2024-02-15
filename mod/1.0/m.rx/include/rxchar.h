/*
 * rxchar.h --- 
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

#ifndef RXCHAR_H
#define RXCHAR_H

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */

#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


#include "rx.h"

/*
 * 文字(1byte)を以下の形式で表していた。
 * 
 * rxChar(int) を用いて、
 * rxChar を 0x12345678
 *
 * 上位1byte(↑の12)文字種(↓rxCharType)
 * 下位1byteを文字コード
 *
 */
#ifdef RX_UNICODE
typedef enum {
  rxMOJI = 0,
  rxBACKREF,
  rxCHARSET
} rxCharType;
#else	/* RX_UNICODE */
typedef enum {
  rxASCII = 0, 
  rxKANA,
  rxKANJI,
  rxXKANJI,
  rxBACKREF,
  rxCHARSET
} rxCharType;
#endif	/* RX_UNICODE */

typedef int rxChar;

#define RXCHARTYPE(c)	((c) >> 24)
#define RXCHARVAL(c)	((c) & ~0xff000000)
#define RXCHAR(t, c)	(((t) << 24) | (c))
#define RXCHAR_NULL	((rxChar)0)
#define RXCHAR_END	((rxChar)-1)
#define RXCHAR_ANY	RXCHAR(rxCHARSET, 0xffffff)

typedef struct _rxCharRange {
    rxChar	from;
    rxChar	to;
} rxCharRange;

typedef struct _rxCharSet {
    ModUnicodeChar	invert;
    int		nranges;
    rxCharRange* ranges;
} rxCharSet;

EXTERN_FUNC(rxChar, rxCharGet, (ModUnicodeChar** s));
#ifdef DEBUG
EXTERN_FUNC(void, rxCharPut, (rxChar ch));
#endif /* DEBUG */

#endif /* RXCHAR_H */

/*
 *	Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
