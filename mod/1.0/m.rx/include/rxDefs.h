/*
 * rxDefs.h --- ローカルなマクロ定義
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
#ifndef _rxdefs_h
#define _rxdefs_h

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */
#include "rxLocal.h"


#define RX_ADVANCE	(0x10)
#define RX_DFA_MODEMASK	(0x0f)


/* 文字セット */
#define C0_BG		0x00
#define C0_ED		0x1f
#define GL_BG		0x20	/* スペースが 0x20 である */
#define GL_ED		0x7e

#ifdef RX_MSKANJI

#define HK_BG		0xa0	/* 半角カナ */
#define HK_ED		0xdf
#define Z1A_BG		0x81	/* 全角文字・１バイト目 */
#define Z1A_ED		0x9f
#define Z1B_BG		0xe0
#define Z1B_ED		0xef
#define Z1C_BG		0xfa	/* IBM 拡張文字コード・１バイト目 */
#define Z1C_ED		0xfc
#define Z2A_BG		0x40	/* 全角文字・２バイト目 */
#define Z2A_ED		0x7e
#define Z2B_BG		0x80
#define Z2B_ED		0xfc

#else /* RX_MSKANJI */

#define GR_BG		0xa1
#define GR_ED		0xfe
#define SS2		0x8e	/* 半角カナへのシフトコード */
#define SS3		0x8f	/* 補助漢字へのシフトコード */

#endif


/* ビット列操作用マクロ */
#define BIT_ON( bitmap, pos )	((bitmap)[(pos)/32]|=(1<<((pos)%32)))
#define BIT_OFF( bitmap, pos )	((bitmap)[(pos)/32]&=(~(1<<((pos)%32))))
#define BIT_CHECK( bitmap, pos )	((bitmap)?((bitmap)[(pos)/32])&(1<<((pos)%32)):0)


/* エラーコード設定用マクロ */
#if 0
#define RX_ERROR_SET(code)	(rxErrorCode=__LINE__)
#else
#define RX_ERROR_SET(code)	(rxErrorCode=(code))
#endif

/* メモリー操作用マクロ */
#ifndef MEMALLOC
#define MEMALLOC(size)	malloc((size))
#endif

#ifndef MEMRESIZE
#define MEMRESIZE(mem,newsize,oldsize)	realloc((void*)(mem),(newsize))
#endif

#ifndef MEMFREE
#define MEMFREE(mem,size)	free((void*)(mem))
#endif

#endif
/* _rxdefs_h */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
