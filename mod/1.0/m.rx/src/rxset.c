/*
 * rxset.c --- 整数セット処理(ordered)
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
#include "rxset.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */

#include <stdio.h>
#include <stdlib.h>

#define RXSET_ALLOC_SIZE 8

/*
 * rxSetAlloc - 集合用領域の確保/拡張
 */
static int
rxSetAlloc(set, size)
    rxSet* set;
    int size;
{
    int nalloc = set->nalloc;

    if (nalloc >= size)
	return RX_OK;

    while (nalloc < size)
	nalloc += RXSET_ALLOC_SIZE;

    if (set->elems) { /* 領域を拡大する */
	int* new = (int*)MEMRESIZE(set->elems, 
				   nalloc * sizeof(int), 
				   set->nalloc * sizeof(int));
	if (!new) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC);
	    return RX_ERR;
	}
	set->elems = new;
    } else { /* 新規に領域を確保する */
	set->elems = (int*)MEMALLOC(nalloc * sizeof(int));
	if (!set->elems) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC);
	    return RX_ERR;
	}
    }

    set->nalloc = nalloc;
    set->nelem = size;

    return RX_OK;
}

void
rxSetInit(set)
    rxSet* set;
{
    set->nalloc = 0;
    set->nelem = 0;
    set->elems = NULL;
}

void 
rxSetFree(set)
    rxSet* set;
{
    set->nelem = 0;
    if (set->elems)
	MEMFREE(set->elems, set->nalloc * sizeof(int));
    set->nalloc = 0;
    set->elems = NULL;
}

int 
rxSetSet(set, el)
    rxSet* set;
    int el;
{
    if (rxSetAlloc(set, 1) != RX_OK)
	return RX_ERR;

    set->elems[0] = el;
    return RX_OK;
}

int
rxSetCopy(dst, src)
    rxSet* dst;
    rxSet* src;
{
    int i;

    /* 必要ならばdstの領域をsrcに合わせて拡大する */
    if (rxSetAlloc(dst, src->nelem) != RX_OK)
	return RX_ERR;

    for (i = 0; i < src->nelem; ++i)
	dst->elems[i] = src->elems[i];

    return RX_OK;
}

int
rxSetEqual(p, q)
    rxSet* p;
    rxSet* q;
{
    int i;
  
    /* 要素数のチェック */
    if (p->nelem != q->nelem)
	return RX_FALSE;

    /* 
     * 各要素の比較。要素は昇順に格納されているので、
     * どこかで一致しなければ、集合全体は不一致。
     */
    for (i = 0; i < p->nelem; i++) {
	if (p->elems[i] != q->elems[i])
	    return RX_FALSE;
    }

    return RX_TRUE;
}

static void
rxSetMerge(p, q, r)
    rxSet* p;
    rxSet* q;
    rxSet* r;
{
    int i = 0, j = 0;
    int n = 0;

    /*
     * 集合のマージ。pとqの要素をマージしてrに入れる。
     * rの要素領域はあらかじめ十分なサイズを確保しておく。
     */

    while (i < p->nelem && j < q->nelem) {
	if (p->elems[i] < q->elems[j])
	    r->elems[n++] = p->elems[i++];
	else if (p->elems[i] > q->elems[j])
	    r->elems[n++] = q->elems[j++];
	else {
	    r->elems[n++] = p->elems[i++];
	    j++;
	}
    }
    while (i < p->nelem)
	r->elems[n++] = p->elems[i++];
    while (j < q->nelem)
	r->elems[n++] = q->elems[j++];

    r->nelem = n;
}

static int
rxSetCountUniqEntry(p, q)
    rxSet* p;
    rxSet* q;
{
    int i = 0, j = 0;
    int entries = 0;
  
    /*
     * 集合pとqでユニークな要素の数を数える
     */

    while (i < p->nelem && j < q->nelem) {
	entries++;
	if (p->elems[i] > q->elems[j])
	    j++;
	else if (p->elems[i] < q->elems[j])
	    i++;
	else {
	    i++;
	    j++;
	}
    }
    entries += p->nelem - i;
    entries += q->nelem - j;

    return entries;
}

int
rxSetUnion(p, q)
    rxSet* p;
    rxSet* q;
{
    int new_entries;
    rxSet merged;

    /* 集合pとqのユニオンをとってpに入れる */

    if (q->nelem == 0)
	return RX_OK;

    if (p->nelem == 0)
	return rxSetCopy(p, q);

    new_entries = rxSetCountUniqEntry(p, q);
    /* 要素数が変らない → 何もしないで帰る */
    if (new_entries == p->nelem)
	return RX_OK;

    rxSetInit(&merged);
    if (rxSetAlloc(&merged, new_entries) != RX_OK)
	return RX_ERR;

    rxSetMerge(p, q, &merged);
    rxSetFree(p);

    *p = merged;

    return RX_OK;
}

#ifdef DEBUG
void 
rxSetPrint(p)
    rxSet* p;
{
    int i;

    printf("{ ");

    for (i = 0; i < p->nelem; i++) {
	printf("%d ", p->elems[i]);
    }

    printf("}");
}
#endif /* DEBUG */

/*
 *	Copyright (c) 1996, 2000, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
