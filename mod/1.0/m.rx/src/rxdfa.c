/*
 * rxdfa.c --- DFAの生成と照合処理
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
#include "rxtree.h"
#include "rxdfa.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define RXDFA_STATE_NOTRANS	(-1)
#define RXDFA_STATE_ACCEPT	(-2)

#define RXDFA_TRANS_SIZE	64
#define RXDFA_DSTATES_SIZE	64
#define RXDFA_TRACE_SIZE	64

#define BRBIT_BITMASK		0x0000ffff

#define RX_LSMASK		(1)

/*
 * dfa_init - DFA構造体の領域確保と初期化
 *
 * 【戻り値】
 * DFA構造体へのポインタ。エラーのときはNULL。
 *
 */
static rxDfa*
dfa_init()
{
    /* メモリ確保 */
    rxDfa* dfa = (rxDfa*)MEMALLOC(sizeof(rxDfa));

    if (dfa == NULL) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return NULL;
    }

    /* 遷移表サイズに0をセット */
    dfa->tlength = 0;

    /* 遷移表領域サイズにRXDFA_TRANS_SIZE(=64)をセット */
    dfa->talloc = RXDFA_TRANS_SIZE;

    /* 状態遷移表メモリ確保 */
    dfa->trans = (rxTrans*)MEMALLOC(dfa->talloc * sizeof(rxTrans));
    if (dfa->trans == NULL) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return NULL;
    }

    /* 文字セット表 に NULLをセット */
    dfa->ModUnicodeCharsets = (rxCharSet*)NULL;

    /* 文字セットサイズに0をセット */
    dfa->cslength = 0;

    /* 状態遷移インデックス表にNULLをセット */
    dfa->states = (rxState*)NULL; 

    /* 状態番号最大値 = 0 */
    dfa->state_max = 0;

    /* \(,\)の数 = 0 */
    dfa->parens = 0;

    return dfa;
}

/*
 * dfa_add_state - 状態遷移表に新しい状態遷移を追加する
 */
static int
dfa_add_state(dfa, state, ch, brbit, next)
    rxDfa* dfa;		/* DFA構造体へのポインタ */
    int state;		/* 状態番号 */
    rxChar ch;		/* 入力アルファベット */
    int brbit;		/* 副表現マークビット */
    int next;		/* 次状態番号 */
{
    int current = dfa->tlength++;

    /* 必要なら状態遷移表を拡大する */
    if (dfa->tlength >= dfa->talloc) {
	int newalloc = dfa->talloc * 2;
	rxTrans* new = (rxTrans*)MEMRESIZE(dfa->trans, 
					   newalloc * sizeof(rxTrans),
					   dfa->talloc * sizeof(rxTrans));
	if (new == NULL) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC);
	    return RX_ERR;
	}

	dfa->talloc = newalloc;
	dfa->trans = new;
    }

    dfa->trans[current].state = state;
    dfa->trans[current].value = ch;
    dfa->trans[current].next  = next;
    dfa->trans[current].brbit = brbit;

    return RX_OK;
}

/*
 * 状態番号→状態遷移表インデックス対応テーブルの作成
 */
static int
make_state_table(dfa)
    rxDfa* dfa;		/* DFA構造体へのポインタ */
{
    int i, state;

    /* テーブル領域の確保 */
    dfa->states = (rxState*)MEMALLOC(dfa->state_max * sizeof(rxState));
    if (dfa->states == NULL) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return RX_ERR;
    }

    /*
     * 状態遷移表は関数dfa_make()によって、
     * 状態番号順にソートされて作成される。
     * さらに、同一状態番号においてはvalueの値、
     * RXCHAR_END, 通常の文字、文字セット、前方参照
     * の順にソートされている。
     */

    state = -1;
    for (i = 0; i < dfa->tlength; i++) {
	if (state < dfa->trans[i].state) {
	    state = dfa->trans[i].state;
	    dfa->states[state].acceptable = RX_FALSE;
	    dfa->states[state].index = i;
	    dfa->states[state].lastindex = i;
	}

	dfa->states[state].lastindex++;

	/* 受理可能状態か否か */
	if (dfa->trans[i].next == RXDFA_STATE_ACCEPT)
	    dfa->states[state].acceptable = RX_TRUE;
    }

    return RX_OK;
}

typedef struct _rxAlpha {
    rxChar value;
    int    brbit;
} rxAlpha;

/*
 * dfa_make - 正規表現構文木からDFAを構成する
 *
 * 【戻り値】
 *   RX_OK  - 構成成功
 *   RX_ERR - 構成失敗
 *
 */
static int 
dfa_make(dfa, rxt)
    rxDfa* dfa;		/* DFA構造体へのポインタ */
    rxTree* rxt;	/* 正規表現構文木 */
{
    rxSet*	Dstates;
    int		Dlength;
    int		Dalloc;
    int		marked;
    int		i, j, k;
    int		err = RX_OK;

    /* 状態集合の集合Dstatesの初期化 */
    Dalloc = RXDFA_DSTATES_SIZE;
    Dlength = 0;
    Dstates = (rxSet*)MEMALLOC(Dalloc * sizeof(rxSet));
    if (Dstates == NULL) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return RX_ERR;
    }

    rxSetInit(&Dstates[Dlength]);
    err = rxSetCopy(&Dstates[Dlength++], &rxt->tree[rxt->root].firstpos);
    if (err != RX_OK) {
	MEMFREE(Dstates, Dalloc * sizeof(rxSet));
	return RX_ERR;
    }

    for (marked = 0; marked < Dlength; marked++) {
	rxSet  U;
	rxSet* T = &Dstates[marked];
	rxAlpha* alphabet;
	int nalloc;
	int nalpha;

	nalpha = 0;
	nalloc = T->nelem;
	alphabet = (rxAlpha*)MEMALLOC(nalloc * sizeof(rxAlpha));
	if (alphabet == NULL) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC);
	    err = RX_ERR;
	    break;
	}

	for (i = 0; i < T->nelem; i++) {
	    rxChar a = rxt->tree[T->elems[i]].value;
	    int brbit = rxt->tree[T->elems[i]].brbit;
	    for (j = 0; j < nalpha; j++)
		if (alphabet[j].value >= a)
		    break;

	    if (j >= nalpha) {
		alphabet[nalpha].value = a;
		alphabet[nalpha].brbit = brbit;
		nalpha++;
	    }
	    else if (alphabet[j].value != a) {
		for (k = nalpha; k > j; k--)
		    alphabet[k] = alphabet[k-1];
		alphabet[k].value = a;
		alphabet[k].brbit = brbit;
		nalpha++;
	    } else {
		alphabet[j].brbit |= brbit;
	    }
	}

	for (i = 0; i < nalpha; i++) {
	    rxChar ch = alphabet[i].value;
	    int brbit = alphabet[i].brbit;

	    if (ch == RXCHAR_END) {
		/* 受理状態を記録する */
		err = dfa_add_state(dfa, marked, RXCHAR_END, 
				       brbit, RXDFA_STATE_ACCEPT);
		if (err != RX_OK)
		    break;
	    }

	    rxSetInit(&U);
	    for (j = 0; j < T->nelem; j++)
		if (rxt->tree[T->elems[j]].value == ch) {
		    err = rxSetUnion(&U, &rxt->tree[T->elems[j]].followpos);
		    if (err != RX_OK)
			break;
		}
	    
	    if (err != RX_OK)
		break;

	    if (U.nelem > 0) {
		int pos = -1;
		
		/*
		 * Dstatesの中からUを探す。
		 * 見つかればDstatesの中での位置をposに、
		 * 見つからない場合、pos = -1
		 */
		for (k = 0; k < Dlength; k++) {
		    if (rxSetEqual(&Dstates[k], &U)) {
			pos = k;
			break;
		    }
		}

		if (pos < 0) {
		    pos = Dlength;
		    if (Dlength >= Dalloc) {
			int newalloc = Dalloc * 2;
			rxSet* newstates;

			newstates = (rxSet*)MEMRESIZE(Dstates,
						      newalloc * sizeof(rxSet),
						      Dalloc * sizeof(rxSet));
			if (newstates == NULL) {
			    RX_ERROR_SET(RX_ERR_MEMALLOC);
			    err = RX_ERR;
			    break;
			}

			Dalloc = newalloc;
			Dstates = newstates;
		    }

		    rxSetInit(&Dstates[Dlength]);
		    err = rxSetCopy(&Dstates[Dlength++], &U);

			T = &Dstates[marked];

		    if (err != RX_OK)
			break;
		}

		err = dfa_add_state(dfa, marked, ch, brbit, pos);
		if (err != RX_OK)
		    break;
	    }

	    rxSetFree(&U);
	} /* end of for (..; i < nalpha; ..) */

	rxSetFree(&U); /* エラーで脱出してきた時のため */

	MEMFREE(alphabet, nalloc * sizeof(rxAlpha));

	if (err != RX_OK)
	    break;

    } /* end of for (..; marked < Dlength; ..) */

    dfa->state_max = Dlength;

    /* Dstatesの領域の解放 */
    for (i = 0; i < Dlength; i++) {
	rxSetFree(&Dstates[i]);
    }
    MEMFREE(Dstates, Dalloced * sizeof(rxSet));

    return err;
}

/*
 * ModUnicodeCharset_inbound - 文字が文字セットに含まれるかどうかを調べる
 */
static int
ModUnicodeCharset_inbound(dfa, cset, ch)
    rxDfa* dfa;		/* DFA構造体へのポインタ */
    rxChar cset;	/* 文字セット */
    rxChar ch;		/* テスト文字 */
{
    rxCharSet* ModUnicodeCharset;
    int i;
    
    /*
     * 文字chが文字セットcsetに含まれるかどうかテスト
     */
    ModUnicodeCharset = &dfa->ModUnicodeCharsets[RXCHARVAL(cset)];
    for (i = 0; i < ModUnicodeCharset->nranges; i++) {
        if (ModUnicodeCharset->ranges[i].from <= ch && ch <= ModUnicodeCharset->ranges[i].to)
            return !ModUnicodeCharset->invert;
    }
    return ModUnicodeCharset->invert;
}

/*
 * redcord_brackets - 照合テキストでの副表現の位置を記録する
 */
static void
record_brackets(brbit, brackets, p, lastp)
    int brbit;			/* 副表現マークビット */
    rxBrackets* brackets;	/* 副表現位置テーブル */
    ModUnicodeChar* p;			/* 照合テキストポインタ */
    ModUnicodeChar* lastp;		/* 照合テキストポインタ前位置 */
{
    int bbit = brbit & BRBIT_BITMASK;
    int ebit = (brbit >> 16) & BRBIT_BITMASK;

    if (bbit) {
	int i;
	for (i = 0; bbit && i < 9; i++) {
	    if (bbit & 1)
		if (!brackets->braslist[i])
		    brackets->braslist[i] = lastp;
	    bbit >>= 1;
	}
    }

    if (ebit) {
	int i;
	for (i = 0; ebit && i < 9; i++) {
	    if (ebit & 1)
		brackets->braelist[i] = p;
	    ebit >>= 1;
	}
    }
}

/*
 * init_brackets - 副表現位置テーブルの初期化
 */
static void
init_brackets(brackets)
    rxBrackets* brackets;
{
    int i;

    for (i = 0; i < 9; i++) {
	brackets->braslist[i] = NULL;
	brackets->braelist[i] = NULL;
    }

}

#ifdef DEBUG
static void
print_brackets(brackets)
    rxBrackets* brackets;
{
    int i;

    for (i = 0; i < 9; i++) {
	ModUnicodeChar* bras = brackets->braslist[i];
	ModUnicodeChar* brae = brackets->braelist[i];
	if (bras && brae) {
	    printf("\\%d=(", i+1);
	    while (bras < brae) {
		putchar(*bras++);
	    }
	    printf(") ");
	}
    }
    printf("\n");
}
#endif /* DEBUG */

/*
 * next_state - 次の状態への遷移を試みる
 */
static int
next_state(dfa, index_r, lastindex, ch, brackets, lastp, p_r)
    rxDfa* dfa;			/* DFA構造体 */
    int* index_r;		/* 遷移表の開始インデックス */
    int lastindex;		/* 遷移表の終了インデックス */
    rxChar ch;			/* 入力文字 */
    rxBrackets* brackets;	/* 副表現位置テーブル */
    ModUnicodeChar* lastp;		/* 照合文字列の現在位置 */
    ModUnicodeChar** p_r;			/* 照合文字列の次の位置 */
{
	while (*index_r < lastindex) {
		rxTrans* trans = &dfa->trans[*index_r];
		rxChar val = trans->value;

		if (val == ch)
			return trans->next;

		if (RXCHARTYPE(val) == rxBACKREF) {
			int backref = RXCHARVAL(val);
			ModUnicodeChar* bras = brackets->braslist[backref - 1];
			ModUnicodeChar* brae = brackets->braelist[backref - 1];
			int length = (int)(brae - bras);
#ifdef RX_UNICODE
			if (bras && brae && length && 
				memcmp(lastp, bras, sizeof(ModUnicodeChar)*length) == 0) {
#else /* RX_UNICODE */
			if (bras && brae && length && strncmp(lastp, bras, length) == 0) {
#endif /* RX_UNICODE */
				*p_r = lastp + length;
				return trans->next;
			}
		}

		if (RXCHARTYPE(val) == rxCHARSET) {
			if (val == RXCHAR_ANY || ModUnicodeCharset_inbound(dfa, val, ch))
			return trans->next;
		}

		++*index_r;
	}

    return RXDFA_STATE_NOTRANS;
}

typedef struct _rxTrace {
    int state;
    int trans;
    ModUnicodeChar* ptr;
    rxBrackets brackets;
} rxTrace;

/*
 * match_short - 最短一致で照合する
 */
static int
match_short(dfa, text, eot, edloc_r, brackets)
    rxDfa* dfa;			/* DFA構造体 */
    ModUnicodeChar* text;			/* 照合対象文字列 */
    ModUnicodeChar* eot;			/* 照合対象文字列の終端位置 */
    ModUnicodeChar** edloc_r;		/* 照合末尾位置 */
    rxBrackets* brackets;	/* 副表現位置テーブル */
{


	ModUnicodeChar* p;
	ModUnicodeChar* lastp;
	rxChar ch;
	int state = 0;

	rxTrace* trace = NULL;
	int talloc = 0;
	int tracep = 0;
	int tindex = 0;

	if (dfa->states[0].acceptable) {
		if (*text != '\0' && text != eot)
		    /* ヌル文字にマッチした */
			return RX_TRUE;
	}

	talloc = RXDFA_TRACE_SIZE;
	trace = (rxTrace*)MEMALLOC(talloc * sizeof(rxTrace));
	if (trace == NULL) {
		RX_ERROR_SET(RX_ERR_MEMALLOC);
		return RX_ERR;
	}

	lastp = p = text;
	ch = rxCharGet(&p);
	while (ch != RXCHAR_NULL && (!eot || p <= eot)) {
		int newstate;
		rxState* rxstate = &dfa->states[state];
		int index = tindex ? tindex : rxstate->index;

		newstate = next_state(dfa, &index, rxstate->lastindex,
								      ch, brackets, lastp, &p);

		if (newstate == RXDFA_STATE_NOTRANS) {
		    if (tracep > 0) {
				--tracep;
				state = trace[tracep].state;
				p = trace[tracep].ptr;
				*brackets = trace[tracep].brackets;
				tindex = trace[tracep].trans;

				lastp = p;
				ch = rxCharGet(&p);
				continue;
	   		}
	    	break;
		}

		/* \(, \) の位置を記録しておく */
		if (dfa->trans[index].brbit != 0) {
			record_brackets(dfa->trans[index].brbit, brackets, p, lastp);

			if (tracep >= talloc) {
				int newalloc = talloc * 2;
				rxTrace* ntrace = (rxTrace*)MEMRESIZE(trace,
							      newalloc*sizeof(rxTrace),
							      talloc*sizeof(rxTrace));

				if (ntrace == NULL) {
				    RX_ERROR_SET(RX_ERR_MEMALLOC);
				    MEMFREE(trace, talloc*sizeof(rxTrace));
				    return RX_ERR;
				}
				trace = ntrace;
				talloc = newalloc;
		    }
	    	trace[tracep].state = state;
	    	trace[tracep].ptr = p;
	    	trace[tracep].brackets = *brackets;
	    	trace[tracep].trans = index + 1;
	    	++tracep;
		}

#ifdef DEBUG
		if (rxDebugMode > 1)
			print_brackets(brackets);
#endif /* DEBUG */	

		state = newstate;
		*edloc_r = p;

		if (dfa->states[state].acceptable) {
		    break;
		}

		tindex = 0;
		lastp = p;
		ch = rxCharGet(&p);
    }

	MEMFREE(trace, talloc*sizeof(rxTrace));
	return dfa->states[state].acceptable;
}

/*
 * match_long - 最長一致で照合する
 */
static int
match_long(dfa,  text, eot, edloc_r, brackets)
    rxDfa* dfa;			/* DFA構造体 */
    ModUnicodeChar* text;			/* 照合対象文字列 */
    ModUnicodeChar* eot;			/* 照合対象文字列の終端位置 */
    ModUnicodeChar** edloc_r;		/* 照合末尾位置 */
    rxBrackets* brackets;	/* 副表現位置テーブル */
{
	ModUnicodeChar* p;
	ModUnicodeChar* lastp;
	int accept = RX_FALSE;
	ModUnicodeChar* acceptp = NULL;
	rxBrackets abrackets;

	rxTrace* trace = NULL;
	int talloc = 0;
	int tracep = 0;
	int tindex = 0;

	rxChar ch;
	int state = 0;

	talloc = RXDFA_TRACE_SIZE;
	trace = (rxTrace*)MEMALLOC(talloc * sizeof(rxTrace));
	if (trace == NULL) {
		RX_ERROR_SET(RX_ERR_MEMALLOC);
		return RX_ERR;
	}

    lastp = p = text;
    ch = rxCharGet(&p);
    for (;;) {
	rxState* rxstate = &dfa->states[state];
	int newstate = RXDFA_STATE_NOTRANS;
	int index = tindex ? tindex : rxstate->index; 

	if (ch != RXCHAR_NULL && (!eot || p <= eot))
	    newstate = next_state(dfa, &index, rxstate->lastindex,
				  ch, brackets, lastp, &p);

	if (newstate == RXDFA_STATE_NOTRANS) {
	    if (tracep > 0) {
		--tracep;
		state = trace[tracep].state;
		tindex = trace[tracep].trans;
		p = trace[tracep].ptr;
		*brackets = trace[tracep].brackets;

		lastp = p;
		ch = rxCharGet(&p);
		continue;
	    }
	    break;
	}
		
	if (tracep >= talloc) {
	    int newalloc = talloc * 2;
	    rxTrace* expanded = (rxTrace*)MEMRESIZE(trace,
						    newalloc*sizeof(rxTrace),
						    talloc*sizeof(rxTrace));
	    if (expanded == NULL) {
		RX_ERROR_SET(RX_ERR_MEMALLOC);
		MEMFREE(trace, talloc*sizeof(rxTrace));
		return RX_ERR;
	    }
	    trace = expanded;
	    talloc = newalloc;
	}
	trace[tracep].state = state;
	trace[tracep].trans = index+1;
	trace[tracep].ptr   = lastp;
	trace[tracep].brackets = *brackets;
	++tracep;
	    
	/* \(, \) の位置を記録しておく */
	if (dfa->trans[index].brbit != 0)
	    record_brackets(dfa->trans[index].brbit, brackets, p, lastp);

#ifdef DEBUG
	if (rxDebugMode > 0)
	    print_brackets(brackets);
#endif /* DEBUG */	

	state = newstate;
	*edloc_r = p;

	if (dfa->states[state].acceptable) {
	    accept = RX_TRUE;
	    if (p > acceptp) {
		acceptp = p;
		abrackets = *brackets;
	    }
	}

	tindex = 0;
	lastp = p;
	ch = rxCharGet(&p);
    }

    MEMFREE(trace, talloc * sizeof(rxTrace));

    if (accept) {
	if (!dfa->states[state].acceptable) {
	    *edloc_r = acceptp;
	    *brackets = abrackets;
	}
	return RX_TRUE;
    }

    return dfa->states[state].acceptable;
}

/*
 * rxDfaCompile - 正規表現から決定性有限オートマトンを生成する。
 *
 * 【戻り値】
 * 生成に成功すると、DFA構造体へのポインタを返す。
 * 生成に失敗した時は、 NULLを返す。
 *
 */

rxDfa* 
rxDfaCompile(pattern)
    ModUnicodeChar* pattern;	/* 正規表現文字列 */
{
    rxTree tree;
    rxDfa* dfa;
  
    if (!pattern) {
	RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	return NULL;
    }

    /* DFA構造体の初期化 */
    if (!(dfa = dfa_init()))
	return NULL;

    if (rxTreeBuild(dfa, &tree, pattern) != RX_OK ||
	dfa_make(dfa, &tree) != RX_OK ||
	make_state_table(dfa) != RX_OK) {
	rxDfaFree(dfa);
	dfa = NULL;
    }

    rxTreeFree(&tree);

    return dfa;
}

/*
 * rxDfaFree - rxDfaCompileで確保したメモリ領域を解放する。
 *
 * 【戻り値】
 * なし
 */

void 
rxDfaFree(dfa)
    rxDfa* dfa;		/* DFA構造体へのポインタ */
{
    if (!dfa) return;

    MEMFREE(dfa->trans, dfa->talloc * sizeof(rxTrans));
    MEMFREE(dfa->states, dfa->state_max * sizeof(rxState));

    if (dfa->ModUnicodeCharsets) {
	int i;
	for (i = 0; i < dfa->cslength; i++)
	    MEMFREE(dfa->ModUnicodeCharsets[i].ranges, 
		    dfa->ModUnicodeCharsets[i].nranges * sizeof(rxCharRange));

	MEMFREE(dfa->ModUnicodeCharsets, dfa->cslength * sizeof(rxCharSet));
    }

    MEMFREE(dfa, sizeof(rxDfa));
}

/*
 * rxDfaMatch - 文字列がオートマトンによって受理されるかどうかを調べる。
 *
 * 【戻り値】
 * ・受理されればRX_TRUEを返し、
 *     照合範囲の末尾のつぎの位置をedlocに、
 *     照合範囲内の副表現に対応する文字列の位置情報をbracketsにセットする。
 * ・受理されない場合はRX_FALSEを返す。
 */
int 
rxDfaMatch(dfa, text, length, edloc_r, brackets, mode)
    rxDfa* dfa;			/* DFA構造体へのポインタ    */
    ModUnicodeChar* text;			/* 照合対象文字列           */
    int length;			/* 照合対象文字列の長さ     */
    ModUnicodeChar** edloc_r;		/* 照合範囲の次の位置を返す */
    rxBrackets* brackets;	/* 副表現の位置情報         */
    int mode;			/* 照合モード               */
{
    ModUnicodeChar* eot = NULL;

    ; assert(dfa != NULL);

    if (!text)
	return RX_FALSE;

    init_brackets(brackets);
    brackets->nbra = dfa->parens;
    *edloc_r = text;

#if 0
	/* 
	 * 文字数に変更済み！！！
	 *
	 * lengthには現在バイト数がセットされている Unicodeの場合
	 * eot = ext +length/2
	 * が正しい。
	 * (本当はlenghtに文字数をセットしたほうが良い？)
	 *
	 */
    if (length != RX_NULL_TERMINATE)
		eot = text + length/2;
#else 
    if (length != RX_NULL_TERMINATE)
	eot = text + length;
#endif

    if ((mode & RX_LSMASK) == RX_SHORTEST)
	return match_short(dfa, text, eot, edloc_r, brackets);
    else
	return match_long(dfa, text, eot, edloc_r, brackets);
}

/*
 * rxDfaExec - 文字列中にDFAによって受理される部分文字列があるか調べる。
 *
 * textにNULLを渡した場合、前回照合した次の文字から継続して照合を開始する。
 *
 * 【戻り値】
 * ・受理されればRX_TRUEを返し、
 *     照合範囲の開始位置、末尾のつぎの位置をそれぞれ*stloc_r, *edloc_rに、
 *     照合範囲内の副表現に対応する文字列の位置情報をbracketsにセットする。
 * ・受理されない場合はRX_FALSEを返す。
 */

int 
rxDfaExec(dfa, text, length, stloc_r, edloc_r, brackets, mode)
    rxDfa* dfa;			/* DFA構造体へのポインタ */
    ModUnicodeChar* text;			/* 照合文字列 */
    int length;			/* 照合対象文字列の長さ */
    ModUnicodeChar** stloc_r;		/* 照合開始位置 */
    ModUnicodeChar** edloc_r;		/* 照合終了位置の次の位置 */
    rxBrackets* brackets;	/* 副表現の位置情報 */
    int mode;			/* 照合モード */
{
    ModUnicodeChar* tptr = text;
    ModUnicodeChar* last = NULL;

    ; assert(dfa != NULL);

    if (!tptr) {
	tptr = *edloc_r;
	length = RX_NULL_TERMINATE;
    }

    if (!tptr) return RX_FALSE;

    if (length != RX_NULL_TERMINATE)
	last = tptr + length;

    do {
	*stloc_r = tptr;
	if (rxDfaMatch(dfa, tptr, length, edloc_r, brackets, mode)) {

	    /* 空文字にマッチしていて、スキップフラグがセットされている */
	    if (*stloc_r == *edloc_r && mode & RX_SKIPEMPTY) {
		/* 1文字すすめる */
		(void)rxCharGet(stloc_r);
		*edloc_r = *stloc_r;	/* *stloc_r == *edloc_r */

		/* ただし、文字列の終端の場合はノーマッチ */
		if (**stloc_r == '\0' || (last && *stloc_r >= last))
		    return RX_FALSE;
	    }
	    *edloc_r;

	    return RX_TRUE;
	}

	(void)rxCharGet(&tptr);
    } while (*tptr && (!last || tptr < last));
    
    return RX_FALSE;
}

/*
 *	Copyright (c) 1996, 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
