/*
 * rxtree.c --- 正規式パース & 構文木作成処理
 *  拡張正規表現ライブラリ	Ver. 1.0
 * 
 * Copyright (c) 1996, 2009, 2023 Ricoh Company, Ltd.
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
#include <string.h>
#include <assert.h>

#include "rx.h"
#include "rxDefs.h"
#include "rxtree.h"
#include "rxdfa.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif

#define RANGES_ARRAY_MAX 128
#define RXCHARSET_ARRAY_SIZE 256

/* tree情報の一時格納領域 */
typedef struct {
    ModUnicodeChar*	patptr;		/* テキストポインタ 	*/
    rxChar	chvalue;	/* 文字値		*/
    rxToken	token;		/* トークン		*/
    rxTree*	rxtree;		/* 正規表現構文木	*/
    int		markcount;	/* 副表現\(\)の出現数	*/
    ModUnicodeChar	availmark[9];	/* 副表現完了フラグ	*/
} rxTreeInfo;

/*----------------------------------------------------------------*/

/*
 * tree_add_node - 正規表現構文木にノードを追加する
 *
 * 【戻り値】
 * 構文木のノード番号。エラーのときはRX_ERR。
 */
static int
tree_add_node(rxt, ch, op, left, right)
    rxTree* rxt;	/* 正規表現構文木 */
    rxChar ch;		/* アルファベット(リーフの場合) */
    opType op;		/* ノードタイプ */
    int left, right;	/* 子ノードへのポインタ */
{
    int index;
    rxTnode* node;

    if (left < 0 || right < 0)
	return RX_ERR;

    index = rxt->tindex++;
    if (index >= rxt->talloc) {
	rxTnode* tree;
	int alloc = rxt->talloc;
	while (index >= alloc)
	    alloc *= 2;
	tree = (rxTnode*)MEMRESIZE(rxt->tree,
				  alloc * sizeof(rxTnode), rxt->talloc);
	if (!tree) {
	    RX_ERROR_SET(RX_ERR_MEMALLOC);
	    return RX_ERR;
	}

	rxt->tree = tree;
	rxt->talloc = alloc;
    }

    node = &rxt->tree[index];
    node->op = op;
    node->left = left;
    node->right = right;
    node->value = ch;
    node->brbit = 0;
    rxSetInit(&node->firstpos);
    rxSetInit(&node->lastpos);
    rxSetInit(&node->followpos);

    return index;
}

#define TRY(action) \
{\
    int err = (action); \
    if (err == RX_ERR) return err; \
}

/*
 * tree_calc_pos - 構文木の各ノードの先頭、末尾、後続位置の計算
 */
static int 
tree_calc_pos(rxt, nn)
    rxTree* rxt;	/* 正規表現構文木 */
    int nn;		/* 構文木上の位置 */
{
    int s1, s2;
    int n1, n2;
    int nullable = RX_TRUE;
    int i;

    ; assert(rxt != NULL);
    ; assert(nn >= 0 && nn < rxt->tindex);

    rxSetInit(&rxt->tree[nn].firstpos);
    rxSetInit(&rxt->tree[nn].lastpos);
    rxSetInit(&rxt->tree[nn].followpos);

    switch (rxt->tree[nn].op) {
    case opLEAF:
	if (rxt->tree[nn].value == RXCHAR_NULL) {
	    nullable = RX_TRUE;
	} else {
	    TRY(rxSetSet(&rxt->tree[nn].firstpos, nn));
	    TRY(rxSetSet(&rxt->tree[nn].lastpos, nn));
	    nullable = RX_FALSE;
	}
	break;

    case opOR:
	s1 = rxt->tree[nn].left;
	s2 = rxt->tree[nn].right;
	TRY(n1 = tree_calc_pos(rxt, s1));
	TRY(n2 = tree_calc_pos(rxt, s2));
	nullable = n1 || n2;

	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	TRY(rxSetUnion(&rxt->tree[nn].firstpos, &rxt->tree[s2].firstpos));
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));
	TRY(rxSetUnion(&rxt->tree[nn].lastpos, &rxt->tree[s2].lastpos));
	break;

    case opCAT:
	s1 = rxt->tree[nn].left;
	s2 = rxt->tree[nn].right;
	TRY(n1 = tree_calc_pos(rxt, s1));
	TRY(n2 =  tree_calc_pos(rxt, s2));
	nullable = n1 && n2;
    
	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	if (n1) {
	    TRY(rxSetUnion(&rxt->tree[nn].firstpos, &rxt->tree[s2].firstpos));
	}
    
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s2].lastpos));
	if (n2) {
	    TRY(rxSetUnion(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));
	}

	for (i = 0; i < rxt->tree[s1].lastpos.nelem; i++) {
	    int t = rxt->tree[s1].lastpos.elems[i];
	    TRY(rxSetUnion(&rxt->tree[t].followpos, &rxt->tree[s2].firstpos));
	}

	break;

    case opSTAR:
	s1 = rxt->tree[nn].left;
	TRY(tree_calc_pos(rxt, s1));
	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));
	nullable = RX_TRUE;

	for (i = 0; i < rxt->tree[nn].lastpos.nelem; i++) {
	    int t = rxt->tree[nn].lastpos.elems[i];
	    TRY(rxSetUnion(&rxt->tree[t].followpos, &rxt->tree[nn].firstpos));
	}

	break;

    case opPLUS:
	s1 = rxt->tree[nn].left;
	TRY(tree_calc_pos(rxt, s1));
	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));
	nullable = RX_FALSE;

	for (i = 0; i < rxt->tree[s1].lastpos.nelem; i++) {
	    int t = rxt->tree[s1].lastpos.elems[i];
	    TRY(rxSetUnion(&rxt->tree[t].followpos, &rxt->tree[nn].firstpos));
	}

	break;

    case opOPT:
	s1 = rxt->tree[nn].left;
	TRY(tree_calc_pos(rxt, s1));
	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));
	nullable = RX_TRUE;
	break;

    case opMARK:
	s1 = rxt->tree[nn].left;
	TRY(nullable = tree_calc_pos(rxt, s1));
	TRY(rxSetCopy(&rxt->tree[nn].firstpos, &rxt->tree[s1].firstpos));
	TRY(rxSetCopy(&rxt->tree[nn].lastpos, &rxt->tree[s1].lastpos));

	/* 副表現\(の先頭に来る全てのノードの副表現位置ビットをセットする */
	for (i = 0; i < rxt->tree[nn].firstpos.nelem; i++) {
	    int t = rxt->tree[nn].firstpos.elems[i];
	    rxt->tree[t].brbit |= 1 << (rxt->tree[nn].value - 1);
	}

/* 副表現\)の末尾に来る全てのノードの副表現位置ビットをセットする */
	for (i = 0; i < rxt->tree[nn].lastpos.nelem; i++) {
	    int t = rxt->tree[nn].lastpos.elems[i];
	    rxt->tree[t].brbit |= 1 << (rxt->tree[nn].value - 1 + 16);
	}
	break;

    default:
	/* 全ての場合を尽しているので、これはない */
	; assert(0);
	break;
    }

    return nullable;
}

#ifdef DEBUG
/*
 * tree_print -正規表現構文木の出力 (デバッグ用)
 */
static void 
tree_print(rxt, pos, level)
    rxTree* rxt;
    int pos;
    int level;
{
    int i;

    for (i = 0; i < level; i++)
	printf("   ");
    printf("+-- ");

    switch (rxt->tree[pos].op) {
    case opLEAF:
	rxCharPut(rxt->tree[pos].value);
	printf(" (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf(" --- ");
	rxSetPrint(&rxt->tree[pos].followpos);
	printf(" %08x", rxt->tree[pos].brbit);
	printf("\n");
	break;

    case opSTAR:
	printf("* (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	break;

    case opPLUS:
	printf("+ (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	break;

    case opOPT:
	printf("? (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	break;

    case opOR:
	printf("| (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	tree_print(rxt, rxt->tree[pos].right, level+1);
	break;

    case opCAT:
	printf(". (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	tree_print(rxt, rxt->tree[pos].right, level+1);
	break;

    case opMARK:
	printf("@ (%d) ", pos);
	rxSetPrint(&rxt->tree[pos].firstpos);
	rxSetPrint(&rxt->tree[pos].lastpos);
	printf("\n");
	tree_print(rxt, rxt->tree[pos].left, level+1);
	break;
    }
}
#endif /* DEBUG */

/*----------------------------------------------------------------*/

/*
 * register_ModUnicodeCharsets - 文字セットをDFA構造体へ登録する
 *
 * 【戻り値】
 *   RX_OK。エラーのときはRX_ERR。
 */
static int
register_ModUnicodeCharsets(dfa)
    rxDfa* dfa;	/* DFA構造体へのポインタ */
{
    int i;

    rxCharSet* charsets
	= (rxCharSet*)MEMALLOC(dfa->cslength * sizeof(rxCharSet));
    if (!charsets) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return RX_ERR;
    }

    for (i = 0; i < dfa->cslength; i++)
	charsets[i] = dfa->ModUnicodeCharsets[i];

    dfa->ModUnicodeCharsets = charsets;

    return RX_OK;
}

/*
 * ModUnicodeCharset_equal -文字セットと文字範囲配列の比較
 */
static int
ModUnicodeCharset_equal(cset, ranges, nranges, invert)
    rxCharSet* cset;		/* 文字セット */
    rxCharRange* ranges;	/* 文字範囲配列 */
    int nranges;		/* 文字範囲配列のサイズ */
    int invert;			/* 反転フラグ */
{
    int i;

    if (cset->invert != invert || cset->nranges != nranges)
	return RX_FALSE;
    
    for (i = 0; i < nranges; i++)
	if (cset->ranges[i].from != ranges[i].from ||
	    cset->ranges[i].to   != ranges[i].to)
	    return RX_FALSE;

    return RX_TRUE;
}

/*
 * ModUnicodeCharset_index - 文字セット表でのインデックスを求める
 *
 * 【説明】
 *  与えられた文字セットの文字セット表でのインデックス値を求める。
 *  文字セット表に無い場合は新たに登録する。
 *
 * 【戻り値】
 * 文字セット表でのインデックス値。エラーのときはRX_ERR。
 *
 */
static int
ModUnicodeCharset_index(dfa, ranges, nranges, invert)
    rxDfa* dfa;
    rxCharRange* ranges;	/* 文字範囲配列 */
    int nranges;		/* 文字範囲配列のサイズ */
    int invert;			/* 反転フラグ */
{
    int i;

    /* 文字セット表に既に登録されているか調べる */
    for (i = 0; i < dfa->cslength; i++)
	if (ModUnicodeCharset_equal(&dfa->ModUnicodeCharsets[i],
				    ranges, nranges, invert))
	    return i;
    
    /*
     * 新しい文字セットの出現
     */

    /* 文字セットの数がModUnicodeCharsets配列のサイズを超えた場合 */
    if (dfa->cslength >= RXCHARSET_ARRAY_SIZE) {
	/* エラーにしてしまう */
	RX_ERROR_SET(RX_ERR_COMPLEXEXPR);
	return RX_ERR;		
    }

    dfa->ModUnicodeCharsets[i].invert = invert;
    dfa->ModUnicodeCharsets[i].nranges = nranges;
    dfa->ModUnicodeCharsets[i].ranges
	= (rxCharRange*)MEMALLOC(nranges * sizeof(rxCharRange));
    if (!dfa->ModUnicodeCharsets[i].ranges) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return RX_ERR;
    }

    memcpy(dfa->ModUnicodeCharsets[i].ranges,
	   ranges, nranges * sizeof(rxCharRange));

    return dfa->cslength++;
}


#define RXGETCHAR(info, c)		\
  {				\
    (c) = rxCharGet(&info->patptr);	\
    if ((c) == RXCHAR_NULL)	\
       return RXCHAR_END;	\
  }

/*
 * read_ModUnicodeCharset - []で囲まれた文字セットをパースする
 */
static rxChar
read_ModUnicodeCharset(dfa, info)
     rxDfa *dfa;
     rxTreeInfo *info;
{
    int index;
    rxChar ch;
    rxChar c1, c2;

    int invert = 0;
    int nranges = 0;
    rxCharRange ranges[RANGES_ARRAY_MAX];

    RXGETCHAR(info, ch);

	/*
	 * 本当はRXCHARVAL(ch)としたほうが良い？
	 * 現在はRXGETCHAR(info, ch)→rxGetChar()はrxModji(=0)の文字種しか
	 * 返さないので問題なし。
	 */

	if (ch == RX_HAT) {		/* [^..]の場合 */
		RXGETCHAR(info, ch);
		invert = 1;
	}

	do {
		if (ch == RX_ESCAPE) {
		    RXGETCHAR(info, ch);
		}

		RXGETCHAR(info, c1);

		if (c1 == RX_MINUS) {	/* 文字範囲なのでもう一文字 */
	    	RXGETCHAR(info, c2);
	    	if (c2 == RX_CKET) {
				--info->patptr;	/* '-'は無かったことにする */
				c2 = ch;
	    	} else {
				if (c2 == RX_ESCAPE) {
		    		RXGETCHAR(info, c2);
				}
				RXGETCHAR(info, c1);
	    	}
		} else {
	    	c2 = ch;		/* 単独の文字の場合。[a-a]として処理 */
		}

		/*
		 * 文字範囲テーブルに範囲[ch〜c2]を追加する。
		 * ch > c2の場合はエラー。
		 */
		if (ch > c2) {
			RX_ERROR_SET(RX_ERR_INVALIDEXPR);
			return RX_ERR;
		} else {
			int findex, tindex, i;
	    
			for (findex = 0; findex < nranges; findex++)
				if (ch <= ranges[findex].to) 
					break;

			for (tindex = findex; tindex < nranges - 1; tindex++)
				if (c2 < ranges[tindex + 1].from)
					break;
	    
			if (findex == tindex) {
				if (findex >= nranges) {
					ranges[findex].from = ch;
					ranges[findex].to = c2;
					nranges++;
				}
				else {
					if (c2 < ranges[findex].from) {
						for (i = nranges; i > findex; --i)
							ranges[i] = ranges[i - 1];
							ranges[findex].from = ch;
							ranges[findex].to   = c2;
							nranges++;
					}
					else {
						if (ch < ranges[findex].from)
							ranges[findex].from = ch;
						if (c2 > ranges[findex].to)
							ranges[findex].to = c2;
					}
				}

				/* 文字範囲テーブルのレンジチェック */
				if (nranges >= RANGES_ARRAY_MAX) {
					RX_ERROR_SET(RX_ERR_COMPLEXEXPR);
					return RX_ERR;
				}
			}
			else if (findex < tindex) {
				if (ch < ranges[findex].from)
					ranges[findex].from = ch;
				if (ch > ranges[tindex].to)
					ranges[tindex].to = c2;
				ranges[findex].to = ranges[tindex].to;
				for (i = 1; tindex + i < nranges; i++) {
					ranges[findex + i] = ranges[tindex + i];
				}
				nranges = findex + i;
			}
		}
    } while ((ch = c1) != RX_CKET);


	/* 
	 * 文字範囲テーブルの正規化
	 * 連続したものがあれば一つにまとめる
	 */

	{
		int i, j;

		j = 0;

		for (i = 1; i < nranges; i++) {
			if (ranges[j].to + 1 == ranges[i].from) {
				ranges[j].to = ranges[i].to;
			} else {
				j++;
				if (j != i) {
					ranges[j] = ranges[i];
				}
			}
		}
		nranges = j + 1;
	}

#ifdef DEBUG
	if (rxDebugMode > 0) {
		int d;
		printf("ModUnicodeCharacter sets:\n");
		for (d = 0; d < nranges; d++) {
			printf("[");
			rxCharPut(ranges[d].from);
			printf("-");
			rxCharPut(ranges[d].to);
			printf("]\n");
		}
	}
#endif /* DEBUG */

	/*
	 * 1文字だけの文字セットだった場合、文字として返す。
	 */
	if (!invert && nranges == 1 && ranges[0].from == ranges[0].to)
		return ranges[0].from;

	/* 文字セット表でのインデックスを求める */
	index = ModUnicodeCharset_index(dfa, ranges, nranges, invert);
	if (index == RX_ERR)
		return RX_ERR;

    return RXCHAR(rxCHARSET, index);
}

/*
 * read_token - パターンバッファからトークンを1つ取り出す
 */
static void
read_token(dfa, info)
     rxDfa *dfa;
     rxTreeInfo *info;
{
	rxChar ch;
	ch = rxCharGet(&info->patptr);
    
	if (ch == RXCHAR_NULL) {
		info->chvalue = RXCHAR_END;
		info->token = rxEND;
		return;
	}

#ifdef RX_UNICODE

	/*
	 * chは文字種付き文字(rxCharTypeが先頭に付加されている)：
	 */
	
	switch (RXCHARVAL(ch)) {

		case RX_STAR:
	    	info->token = rxSTAR;
	    	return;

		case RX_PLUS:
			info->token = rxPLUS;
			return;

		case RX_QUESTION:
			info->token = rxQUESTION;
			return;

		case RX_OR:
		 	info->token = rxOR;
			return;

		case RX_LPAREN:
			info->token = rxLPAREN;
			return;
		case RX_RPAREN:
			info->token = rxRPAREN;
			return;
#ifdef NOTDEF
		case RX_HAT:
			info->token = rxBEGLINE;
			return;

		case RX_DOLLAR:
			info->token = rxENDLINE;
			return;

#endif /* NOTDEF */

		case RX_PERIOD:
			info->chvalue = RXCHAR_ANY;
			info->token = rxCHAR;
			return;

		case RX_BRA:
			info->chvalue = read_ModUnicodeCharset(dfa, info);
			if (info->chvalue == RX_ERR) {
				info->token = rxERR;
			} else {
				info->token = rxCHAR;
			}
			return;

		case RX_ESCAPE:
			ch = rxCharGet(&info->patptr);
			if(RXCHARVAL(ch) >= RX_DIG_ZERO && RXCHARVAL(ch) <=RX_DIG_NINE) {
				/* chが'1'〜'9'の場合 */
				int refnum = ch - RX_DIG_ZERO;
				if (!info->availmark[refnum - 1]) {
					RX_ERROR_SET(RX_ERR_INVALIDEXPR);
					info->token = rxERR;
					return;
				}
				info->chvalue = RXCHAR(rxBACKREF, refnum);
				info->token = rxCHAR;
				return;

			} else if(RXCHARVAL(ch) == RX_LPAREN) {
				/* '(' の場合 */
				info->token = rxBMARK;
				return;

			} else if(RXCHARVAL(ch) == RX_RPAREN) {
				/* ')' の場合 */
				info->token = rxEMARK;
				return;
			}
		default:
			break;
	}	
    info->chvalue = ch;
    info->token = rxCHAR;
    return;



#else	/* RX_UNICODE */
    if (RXCHARTYPE(ch) == rxASCII) {
	switch (RXCHARVAL(ch)) {
	case RX_STAR:
	    info->token = rxSTAR;
	    return;

	case RX_PLUS:
	    info->token = rxPLUS;
	    return;

	case RX_QUESTION:
	    info->token = rxQUESTION;
	    return;

	case RX_OR:
	    info->token = rxOR;
	    return;

	case RX_LPAREN:
	    info->token = rxLPAREN;
	    return;

	case RX_RPAREN:
	    info->token = rxRPAREN;
	    return;

#ifdef NOTDEF
	case RX_HAT:
	    info->token = rxBEGLINE;
	    return;

	case RX_DOLLAR:
	    info->token = rxENDLINE;
	    return;
#endif /* NOTDEF */
      
	case '.':
	    info->chvalue = RXCHAR_ANY;
	    info->token = rxCHAR;
	    return;

	case RX_BRA:
	    info->chvalue = read_ModUnicodeCharset(dfa, info);
	    if (info->chvalue == RX_ERR)
		info->token = rxERR;
	    else
		info->token = rxCHAR;
	    return;

	case RX_ESCAPE:
	    ch = rxCharGet(&info->patptr);
	    if (RXCHARTYPE(ch) == rxASCII) {
		if ((ModUnicodeChar)RXCHARVAL(ch) >= '1' && (ModUnicodeChar)RXCHARVAL(ch) <= '9') {
		    int refnum = RXCHARVAL(ch) - '0';
		    if (!info->availmark[refnum - 1]) {
			RX_ERROR_SET(RX_ERR_INVALIDEXPR);
			info->token = rxERR;
			return;
		    }
		    info->chvalue = RXCHAR(rxBACKREF, refnum);
		    info->token = rxCHAR;
		    return;
		}
		else if (RXCHARVAL(ch) == RX_LPAREN) {
		    info->token = rxBMARK;
		    return;
		}
		else if (RXCHARVAL(ch) == RX_RPAREN) {
		    info->token = rxEMARK;
		    return;
		}
	    }
	    break;

	default:
	    break;
	}
    }

    info->chvalue = ch;
    info->token = rxCHAR;
    return;
#endif	/* RX_UNICODE */
}

/*
 * addleaf - 正規表現構文木にリーフノードを追加する
 */
static int 
addleaf(info, val)
     rxTreeInfo *info;
    rxChar val;	/* リーフノードの文字値 */
{
    return tree_add_node(info->rxtree, val, opLEAF, 0, 0);
}

/*
 * addop1 - 正規表現構文木に単項ノードを追加する
 */
static int 
addop1(info, op, pos)
     rxTreeInfo *info;
    opType op;	/* ノードタイプ */
    int pos;	/* 子ノードへのポインタ */
{
    return tree_add_node(info->rxtree, 0, op, pos, 0);
}

/*
 * addop2 - 正規表現構文木に2項ノードを追加する
 */
static int 
addop2(info, op, left, right)
     rxTreeInfo *info;
    opType op;		/* ノードタイプ */
    int left, right;	/* 子ノードへのポインタ */
{
    return tree_add_node(info->rxtree, 0, op, left, right);
}

/*
 * addopmark - 正規表現構文木に副表現マークノードを追加する
 */
static int 
addopmark(info, brbit, pos)
     rxTreeInfo *info;
    int brbit;		/* 副表現位置ビット */
    int pos;		/* 子ノードへのポインタ */
{
    return tree_add_node(info->rxtree, brbit, opMARK, pos, 0);
}

static int reg(rxDfa* dfa, rxTreeInfo* info);

/*
 * rxExp - 正規表現のパース(文字およびカッコに囲まれた式)
 */
static int 
rxExp(dfa, info)
     rxDfa* dfa;
     rxTreeInfo *info;
{
    int p, q, m;

    switch (info->token) {
    case rxCHAR:
	p = addleaf(info, info->chvalue);
	read_token(dfa, info);
	break;

    case rxLPAREN:
	read_token(dfa, info);
	p = reg(dfa, info);
	if (info->token != rxRPAREN) {
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	    return RX_ERR;
	}
	read_token(dfa, info);
	break;

    case rxBMARK:
	if (info->markcount >= 9) {
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	    return RX_ERR;
	}
	read_token(dfa, info);
	m = ++info->markcount;
	q = reg(dfa, info);
	if (info->token != rxEMARK) {
	    RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	    return RX_ERR;
	}
	info->availmark[m - 1] = 1;
	p = addopmark(info, m, q);
	read_token(dfa, info);
	break;

    default:
	RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	return RX_ERR;
    }
    
    if (info->token == rxERR) {
	/* 先読みしたトークンがエラー */
	RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	return RX_ERR;
    }
    
    return p;
}

/*
 * factor - 正規表現のパース(*, +, ?)
 */
static int 
factor(dfa, info)
     rxDfa* dfa;
     rxTreeInfo *info;
{
    int p = rxExp(dfa, info);

    if (p == RX_ERR) 
	return RX_ERR;

    switch (info->token) {
    case rxSTAR:
	p = addop1(info, opSTAR, p);
	read_token(dfa, info);
	break;

    case rxPLUS:
	p = addop1(info, opPLUS, p);
	read_token(dfa, info);
	break;

    case rxQUESTION:
	p = addop1(info, opOPT, p);
	read_token(dfa, info);
	break;

    default:
	break;
    }

    return p;
}

/*
 * term - 正規表現のパース(連接)
 */
static int 
term(dfa, info)
     rxDfa *dfa;
     rxTreeInfo *info;
{
    int p, q;

    if (info->token == rxERR || info->token == rxEND || info->token == rxOR || 
	info->token == rxRPAREN || info->token == rxEMARK) {
	RX_ERROR_SET(RX_ERR_INVALIDEXPR);
	return RX_ERR;
    }

    p = factor(dfa, info);
    if (p == RX_ERR) 
	return RX_ERR;

    while (info->token != rxERR && info->token != rxEND && info->token != rxOR && 
	   info->token != rxRPAREN && info->token != rxEMARK) {
	q = factor(dfa, info);
	if (q == RX_ERR) 
	    return RX_ERR;
	p = addop2(info, opCAT, p, q);
    }

    return p;
}

/*
 * reg - 正規表現のパース(|)
 */
static int 
reg(dfa, info)
     rxDfa* dfa;
     rxTreeInfo *info;
{
    int p, q;

    p = term(dfa, info);
    if (p == RX_ERR) 
	return RX_ERR;

    while (info->token == rxOR) {
	read_token(dfa, info);
	q = term(dfa, info);
	if (q == RX_ERR) 
	    return RX_ERR;
	p = addop2(info, opOR, p, q);
    }

    return p;
}

/*
 * tree_parse - 文字列を正規表現としてパースし構文木を作る
 *
 * 【説明】
 * 再帰下降法によって正規表現をパースする。
 *
 * 【戻り値】
 * RX_OK。エラーのときはRX_ERR。
 */
static int 
tree_parse(dfa, info, rxt, s)
     rxDfa* dfa;
     rxTreeInfo* info;
     rxTree* rxt;	/* 正規表現構文木 */
     ModUnicodeChar* s;		/* 正規表現文字列 */
{
    int endp;

    info->patptr = s;
    info->rxtree = rxt;

    rxt->root = -1;
    rxt->tindex = 0;
    rxt->talloc = RXTREE_INITIAL_SIZE;
    rxt->tree = (rxTnode*)MEMALLOC(rxt->talloc * sizeof(rxTnode));
    if (!rxt->tree) {
	RX_ERROR_SET(RX_ERR_MEMALLOC);
	return RX_ERR;
    }

    read_token(dfa, info);
    info->rxtree->root = reg(dfa, info);
    if (info->rxtree->root < 0 || info->token != rxEND)
	return RX_ERR;

    endp = addleaf(info, RXCHAR_END);
    info->rxtree->root = addop2(info, opCAT, info->rxtree->root, endp);
    return (info->rxtree->root != RX_ERR) ? RX_OK : RX_ERR;
}

/*
 * rxTreeBuild - 正規表現構文木を作成する
 *
 * 【戻り値】
 * RX_OK。エラーのときはRX_ERR。
 */
int 
rxTreeBuild(dfa, tree, pattern)
    rxDfa* dfa;
    rxTree* tree;
    ModUnicodeChar* pattern;
{
    int i;

    rxCharSet ModUnicodeCharsets[RXCHARSET_ARRAY_SIZE]; /* 文字セットテーブル */

    rxTreeInfo cinfo;
    rxTreeInfo *info = &cinfo;

    info->markcount = 0;		/* 副表現\(\)の出現数   */

     /* 副表現完了フラグavaimark[]の初期化 */
    for (i = 0; i < 9; i++)
	info->availmark[i] = 0;

    dfa->ModUnicodeCharsets = ModUnicodeCharsets;
    dfa->cslength = 0;	/* 文字セット数 */

    /* tree_parse - 文字列を正規表現としてパースし構文木を作る */
    if (tree_parse(dfa, info, tree, pattern) == RX_ERR) {
	/** added 02/2002/01/29 **/
	for(i = 0; i < dfa->cslength; i++) {
	    MEMFREE(dfa->ModUnicodeCharsets[i].ranges, 
		    dfa->ModUnicodeCharsets[i].nranges * sizeof(rxCharRange));
	}
	/**********************************/
	dfa->ModUnicodeCharsets = NULL;
	dfa->cslength = 0;
	return RX_ERR;
    }

    dfa->parens = info->markcount;
    if (register_ModUnicodeCharsets(dfa) == RX_ERR) {
	for(i = 0; i < dfa->cslength; i++) {
	    MEMFREE(dfa->ModUnicodeCharsets[i].ranges, 
		    dfa->ModUnicodeCharsets[i].nranges * sizeof(rxCharRange));
	}
	dfa->ModUnicodeCharsets = NULL;
	dfa->cslength = 0;
	return RX_ERR;
    }

    if (tree_calc_pos(tree, tree->root) == RX_ERR)
	return RX_ERR;

#ifdef DEBUG
    if (rxDebugMode > 0)
		tree_print(tree, tree->root, 0);
#endif /* DEBUG */

    return RX_OK;
}

/*
 * rxTreeFree - 正規表現構文木用に確保した領域の解放
 */
void
rxTreeFree(rxt)
    rxTree* rxt;	/* 正規表現構文木 */
{
    int i;

    for (i = 0; i < rxt->tindex; i++) {
	rxSetFree(&rxt->tree[i].followpos);
	rxSetFree(&rxt->tree[i].firstpos);
	rxSetFree(&rxt->tree[i].lastpos);
    }
    MEMFREE(rxt->tree, rxt->talloc * sizeof(rxTnode));
}

/*
 *	Copyright (c) 1996, 2009, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
