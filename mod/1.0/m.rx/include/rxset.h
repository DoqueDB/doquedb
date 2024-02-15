/*
 * rxset.h --- 整数セット処理
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

#ifndef RXSET_H
#define RXSET_H

#include "rx.h"

typedef struct _rxSet {
  int	nalloc;		/* メモリ確保した要素数 */
  int	nelem;		/* 実際に使用している要素数 */
  int*	elems;		/* 要素配列 */
} rxSet;

EXTERN_FUNC(void, rxSetInit, (rxSet* set));
EXTERN_FUNC(void, rxSetFree, (rxSet* set));
EXTERN_FUNC(int,  rxSetSet,  (rxSet* set, int el));
EXTERN_FUNC(int,  rxSetCopy, (rxSet* dst, rxSet* src));
EXTERN_FUNC(int,  rxSetUnion, (rxSet* s, rxSet* t));
EXTERN_FUNC(int,  rxSetEqual, (rxSet* s, rxSet* t));

#ifdef DEBUG
EXTERN_FUNC(void, rxSetPrint, (rxSet* set));
#endif /* DEBUG */

#endif /* RXSET_H */

/*
 *	Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
