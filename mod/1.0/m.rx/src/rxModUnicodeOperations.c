/*
 * rxModUnicodeOperations.c --- ModUnicodoCharの処理
 *  拡張正規表現ライブラリ  Ver. 1.0
 * 
 * Copyright (c) 1999, 2000, 2003, 2023 Ricoh Company, Ltd.
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
#include <stdlib.h>
#include <assert.h>
#include "rx.h"
#include "rxModUnicodeChar.h"
#include "rxDefs.h"

/*****************************************************************************
 *
 * FUNC : size_t ModUnicodeCharLength(ModUnicodeChar* p)
 *		引きすに渡された、ModUnicdeChar文字列(ModUnicodeChar*)の長さを返す。
 *  	正確には0x00までの文字数を返す。
 *
 *	size_t strlen(cha *)に相当。
 *
 * ARGS :	char * 	[文字列]
 *
 * RETURN :	size_t	[文字列の長さ]
 *
 *
 *****************************************************************************/
size_t ModUnicodeCharLength(ModUnicodeChar* p) {
	size_t count = 0;

	if(p == NULL) {
#ifdef DEBUG
		printf("ModUnicodeCharLength given NULL pointer\n");
#endif /* DEBUG */
		return 0; 
	}
	while(*(p) != 0x00) {
		count++;
		p++;
	}
	return count;
}

/*
 *  Copyright (c) 1999, 2000, 2003, 2023 Ricoh Company, Ltd.
 *  All rights reserved.
 */
