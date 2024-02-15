/*
 * rxtree.h --- 正規式パース & 構文木作成処理
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

#ifndef RXTREE_H
#define RXTREE_H

#include "rx.h"
#include "rxchar.h"
#include "rxset.h"
#include "rxdfa.h"

/*
 * ModUnicodeChar.hをinclude出来ないため、これで代用(中身は同じ)
 */
#ifdef RX_UNICODE
#include "rxModUnicodeChar.h"
#endif /* RX_UNICODE */


#define RXTREE_INITIAL_SIZE 64

typedef enum {
  rxERR = -2,
  rxEND = -1,
  rxEMPTY = 0,
  rxCHAR,
  rxLPAREN, rxRPAREN,
  rxLBRA, rxRBRA,
  rxSTAR, rxPLUS, rxQUESTION, 
  rxOR, rxCAT,
  rxBEGLINE, rxENDLINE,
  rxBMARK, rxEMARK
} rxToken;

typedef enum {
  opLEAF, opOR, opCAT, opSTAR, opPLUS, opOPT, opMARK
} opType;

typedef struct _rxTnode {
  opType	op;		/* ノードタイプ		*/
  rxSet		followpos;	/* ノード位置集合	*/
  rxSet		firstpos;	/* ノード位置集合	*/
  rxSet		lastpos;	/* ノード位置集合	*/
  rxChar	value;		/* アルファベット	*/
  int		brbit;		/* 副表現位置ビット	*/
  int		left;		/* 子ノード番号		*/
  int		right;		/* 子ノード番号		*/
} rxTnode;

/* 正規表現構文木 */
typedef struct _rxTree {
  int		root;		/* ルートノード番号	*/
  rxTnode*	tree;		/* ノード配列		*/
  int		tindex;		/* ノード配列サイズ	*/
  int		talloc;		/* ノード配列領域サイズ	*/
} rxTree;

EXTERN_FUNC(int, rxTreeBuild, (rxDfa*, rxTree*, ModUnicodeChar* pattern));
EXTERN_FUNC(void, rxTreeFree, (rxTree*));

#endif /* RXTREE_H */

/*
 *	Copyright (c) 1996, 2000, 2023 Ricoh Company, Ltd.
 *	All rights reserved.
 */
