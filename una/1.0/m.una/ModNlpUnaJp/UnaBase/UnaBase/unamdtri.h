//
// unamdtri.h -
//      大語彙形態素辞書引きモジュール
//		RevuzのTrieを使った大語彙形態素辞書引きモジュール
// 
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef UNAMDTRI_H
#define UNAMDTRI_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"		/* UNAグローバルなヘッダファイル */
#include "unamorph.h"	/* 形態素解析メインモジュール */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------
//
/* morph part of speech No.(fix) of collocation */
#define UNA_HIN_RENGO_V123	0x7FF	/* collocation, 16bit */
#define UNA_HIN_RENGO_V124	0x7FF	/* collocation, 11bit */
#define UNA_HIN_RENGO_V125	0xFFFF	/* collocation, 16bit */

//--------------------------------------------------------------------------
// TAG:   unaMorDataT
//
// ABSTRACT:      辞書内形態素データ型
//
// NOTE:
//    形態素辞書引きされた結果得られる形態素の基本データを表わす構造体
//    (Trie辞書の中の1形態素のデータ構造)。辞書作成ツールの中でも使われる。
//    メンバの並びは、語境界を意識してある
//
typedef struct unaMorDataT{ // 辞書内形態素データ型
	ucharT  doukeiNo;       /* 自分を含んだ同形語数
							   (同形語の始まりでなければ0がセット) */
	ucharT  morCost;        /* 形態素コスト */
	ushortT morHinNo;       /* 形態素品詞 */
	unsigned morLen   : 8;  /* 形態素の長さ(文字数、byteではない) */
	unsigned subLstNo : 24; /* 下位形態素リスト番号(インデックス即ちポインタ
							   に用いる) 0xFFFFFFの場合、下位形態素なし*/
}unaMorDataT;

//--------------------------------------------------------------------------
// TAG:	  unaCompactMorDataT
//
// ABSTRACT:	  圧縮辞書内形態素データ型
//
// NOTE:
//	  形態素辞書引きされた結果得られる形態素の基本データを表わす構造体
//	  (Trie辞書の中の1形態素のデータ構造)。辞書作成ツールの中でも使われる。
//	  メンバの並びは、語境界を意識してある
//
typedef union unaCompactMorDataT{	// 辞書内形態素データ型
	struct mor{
		unsigned morHinNo : 11;	/* 形態素品詞 */
		unsigned subFlg   :  1;	/* 下位構造フラグ */
		unsigned doukeiNo :  4;	/* 自分を含んだ同形語数
								   (同形語の始まりでなければ0がセット) */
		unsigned morLen   :  8;	/* 形態素の長さ(文字数、byteではない) */
		unsigned morCost  :  8;	/* 形態素コスト */
	} mor;
	struct sub{
		uintT subLstNo;		/* 下位形態素リスト番号(インデックス即ちポインタ
							   に用いる) 0xFFFFFFの場合、下位形態素なし*/
	} sub;
}unaCompactMorDataT;

//--------------------------------------------------------------------------
// TAG:	  baseAndLabelT
//
// ABSTRACT:	  ベースとラベルの値型
//
// NOTE:
//	  base配列とlabel配列を４バイト中に収めたデータ型。
//	  圧縮辞書内で用いる
//
typedef struct baseAndLabelT{	// 辞書内形態素データ型
		unsigned base  :  19;
		unsigned label :  13;
}baseAndLabelT;

//--------------------------------------------------------------------------
// TAG:	  unaMdTriHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  形態素辞書引きモジュールをマルチスレッドセーフで実行するためのハンドラ
//	  圧縮辞書内でのみ用いるものは(Compact)で、通常の辞書内でのみ用いるものは
//	  (Normal)で、両方で用いるものは(Both)で示す。
//
typedef struct unaMdTriHandleT{	// 形態素辞書引きモジュール用ハンドル
	const char *morphDicImg;	/* (B)実行形態素辞書のイメージ */
	uintT  recCount;			/* (B)形態素辞書(登録されている形態素の数) */
	uintT  lstCount;			/* (B)形態素辞書(下位構造リストのエントリ数) */
	uintT  daCount;				/* (B)形態素辞書(base、label配列の数) */
	const unaCharT *cnvTbl;		/* (C)unicode→内部コードへの変換テーブル */
	const unaMorDataT *morData;	/* (N)形態素辞書(同形語、コスト、品詞、
									  長さ、下位形態素リスト番号) */
	const unaCompactMorDataT 
					*cMorData;	/* (C)形態素辞書(同形語、コスト、品詞、
									  長さ、下位形態素リスト番号) */
	const uintT  *subLst;		/* (B)形態素辞書(下位構造リスト) */
	const uintT  *base;			/* (N)ベース配列 */
	const unaCharT  *label;		/* (N)ラベル配列 */
	const baseAndLabelT *bl;	/* (C)ベース配列とラベル配列 */
	int ignoreCR;				/* (B)改行をまたぐ単語を検出するか否かの指示 */
	unaDicFuncT searchFunc;		/* (B)検索関数 */
	unaSubFuncT getSubFunc;		/* (B)下位構造取得関数 */
	unaPrioFuncT prioFunc;		/* (B)優先登録関数 */
	const char *dicName;		/* (B)辞書ベース名 */
	ucharT dicPrio;				/* (B)辞書優先度 */
	int hasVoid;				/* (B)無効語あり辞書 */
}unaMdTriHandleT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaMdicTrie_init(unaMdTriHandleT *th,const char *morphDicImg,
		const char *dicName, const ucharT dicPrio);
/* 辞書ベース名取得 */
char *unaMdicTrie_getDicName(unaMdTriHandleT *th);
/* 終了処理 */
int unaMdicTrie_term(unaMdTriHandleT *th);
/* 改行指示のセット */
void unaMdicTrie_setCR(unaMdTriHandleT *th, int ignoreCrMode);
/* 辞書引き関数  */
int unaMdicTrie_searchMorph(unaMorphHandleT *mh,int txtPos,
		int dicNum,int *morCount,void *tHandle);
/* 優先登録処理関数  */
int unaMdicTrie_prioMorph(unaMorphHandleT *mh,int st,int *ed,
		void *tHandle);
/* 下位構造取得関数 */
int unaMdicTrie_getSubMorph(const unaMorphT *morph,unaMorphT *morphBuf,
		int *morphNum,const int morphBufSize,unaMdTriHandleT *th);

#endif /* end of UNAMDTRI_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
