//
// unamorph.h -
//      形態素解析メインモジュール
//		形態素解析処理のメインのモジュール。
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

#ifndef UNAMORPH_H
#define UNAMORPH_H

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include "una.h"		/* UNAグローバルなヘッダファイル */

//--------------------------------------------------------------------------
// データ型の定義とマクロ定数
//--------------------------------------------------------------------------

/* 固定の形態素品詞番号(unahin.hよりペースト) */
#define UNA_HIN_NOTHING	0	/* 単語無し(どの品詞でもない) */
#define UNA_HIN_UNKNOWN_KATAKANA	1	/* ？(3.2.8)/未登録語.カタカナ(3.3.0) */
#define UNA_HIN_KUTEN	2	/* 記号.句点 */
#define UNA_HIN_TOUTEN	3	/* 記号.読点 */
#define UNA_HIN_SUUSHI	4	/* 数詞 */
#define UNA_HIN_MEISHI_IPPAN	5	/* 名詞.一般 */
#define UNA_HIN_MEISHI_KOYUU	6	/* 名詞.固有 */
#define UNA_HIN_MEISHI_DAI	7	/* 名詞.指示 */
#define UNA_HIN_MEISHI_SAHEN	8	/* 名詞.サ変 */
#define UNA_HIN_JOSUU	9	/* 助数詞 */
#define UNA_HIN_KEIDOU_KANGO	10	/* 形容動詞.一般 */
#define UNA_HIN_RENTAI	11	/* 連体詞 */
#define UNA_HIN_FUKUSHI	12	/* 副詞 */
#define UNA_HIN_SETSUZOKU	13	/* 接続詞 */
#define UNA_HIN_KANDOU	14	/* 感動詞 */
#define UNA_HIN_SETTOUTSUJI_IPPAN	15	/* 接頭辞.一般 */
#define UNA_HIN_SETSUBIJI_IPPAN	16	/* 接尾辞 */
#define UNA_HIN_KIGOU_IPPAN	17	/* 記号.一般 */
#define UNA_HIN_DOUSHI_5_SHUU_KA	18	/* 動詞.終止連体,カ行五段 */
#define UNA_HIN_DOUSHI_5_I_KA	19	/* 動詞.音便,カ行五段 */
#define UNA_HIN_DOUSHI_5_MI_KA	20	/* 動詞.未然,カ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_KA	21	/* 動詞.連用,カ行五段 */
#define UNA_HIN_DOUSHI_5_KARI_KA	22	/* 動詞.仮定命令,カ行五段 */
#define UNA_HIN_DOUSHI_5_U_KA	23	/* 動詞.ウ接続,カ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_GA	24	/* 動詞.終止連体,ガ行五段 */
#define UNA_HIN_DOUSHI_5_I_GA	25	/* 動詞.音便,ガ行五段 */
#define UNA_HIN_DOUSHI_5_MI_GA	26	/* 動詞.未然,ガ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_GA	27	/* 動詞.連用,ガ行五段 */
#define UNA_HIN_DOUSHI_5_KA_GA	28	/* 動詞.仮定命令,ガ行五段 */
#define UNA_HIN_DOUSHI_5_U_GA	29	/* 動詞.ウ接続,ガ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_SA	30	/* 動詞.終止連体,サ行五段 */
#define UNA_HIN_DOUSHI_5_MI_SA	31	/* 動詞.未然,サ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_SA	32	/* 動詞.連用,サ行五段 */
#define UNA_HIN_DOUSHI_5_KA_SA	33	/* 動詞.仮定命令,サ行五段 */
#define UNA_HIN_DOUSHI_5_U_SA	34	/* 動詞.ウ接続,サ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_TA	35	/* 動詞.終止連体,タ行五段 */
#define UNA_HIN_DOUSHI_5_TSU_TA	36	/* 動詞.音便,タ行五段 */
#define UNA_HIN_DOUSHI_5_MI_TA	37	/* 動詞.未然,タ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_TA	38	/* 動詞.連用,タ行五段 */
#define UNA_HIN_DOUSHI_5_KA_TA	39	/* 動詞.仮定命令,タ行五段 */
#define UNA_HIN_DOUSHI_5_U_TA	40	/* 動詞.ウ接続,タ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_NA	41	/* 動詞.終止連体,ナ行五段 */
#define UNA_HIN_DOUSHI_5_MI_NA	42	/* 動詞.未然,ナ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_NA	43	/* 動詞.連用,ナ行五段 */
#define UNA_HIN_DOUSHI_5_KA_NA	44	/* 動詞.仮定命令,ナ行五段 */
#define UNA_HIN_DOUSHI_5_U_NA	45	/* 動詞.ウ接続,ナ行五段 */
#define UNA_HIN_DOUSHI_5_N_NA	46	/* 動詞.音便,ナ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_BA	47	/* 動詞.終止連体,バ行五段 */
#define UNA_HIN_DOUSHI_5_MI_BA	48	/* 動詞.未然,バ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_BA	49	/* 動詞.連用,バ行五段 */
#define UNA_HIN_DOUSHI_5_KA_BA	50	/* 動詞.仮定命令,バ行五段 */
#define UNA_HIN_DOUSHI_5_U_BA	51	/* 動詞.ウ接続,バ行五段 */
#define UNA_HIN_DOUSHI_5_N_BA	52	/* 動詞.音便,バ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_MA	53	/* 動詞.終止連体,マ行五段 */
#define UNA_HIN_DOUSHI_5_MI_MA	54	/* 動詞.未然,マ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_MA	55	/* 動詞.連用,マ行五段 */
#define UNA_HIN_DOUSHI_5_KA_MA	56	/* 動詞.仮定命令,マ行五段 */
#define UNA_HIN_DOUSHI_5_U_MA	57	/* 動詞.ウ接続,マ行五段 */
#define UNA_HIN_DOUSHI_5_N_MA	58	/* 動詞.音便,マ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_RA	59	/* 動詞.終止連体,ラ行五段 */
#define UNA_HIN_DOUSHI_5_TSU_RA	60	/* 動詞.音便,ラ行五段 */
#define UNA_HIN_DOUSHI_5_MI_RA	61	/* 動詞.未然,ラ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_RA	62	/* 動詞.連用,ラ行五段 */
#define UNA_HIN_DOUSHI_5_KA_RA	63	/* 動詞.仮定命令,ラ行五段 */
#define UNA_HIN_DOUSHI_5_U_RA	64	/* 動詞.ウ接続,ラ行五段 */
#define UNA_HIN_DOUSHI_5_SHUU_WA	65	/* 動詞.終止連体,ワ行五段 */
#define UNA_HIN_DOUSHI_5_TSU_WA	66	/* 動詞.音便,ワ行五段 */
#define UNA_HIN_DOUSHI_5_MI_WA	67	/* 動詞.未然,ワ行五段 */
#define UNA_HIN_DOUSHI_5_YOU_WA	68	/* 動詞.連用,ワ行五段 */
#define UNA_HIN_DOUSHI_5_KA_WA	69	/* 動詞.仮定命令,ワ行五段 */
#define UNA_HIN_DOUSHI_5_U_WA	70	/* 動詞.ウ接続,ワ行五段 */
#define UNA_HIN_DOUSHI_1_SHUU	71	/* 動詞.終止連体,一段 */
#define UNA_HIN_DOUSHI_1_YOU	72	/* 動詞.連用,一段 */
#define UNA_HIN_DOUSHI_1_KA	73	/* 動詞.仮定,一段 */
#define UNA_HIN_DOUSHI_1_MEI	74	/* 動詞.命令,一段.ロ */
#define UNA_HIN_DOUSHI_1_MEIYO	75	/* 動詞.命令,一段.ヨ */
#define UNA_HIN_KEIYOU_SHUU	76	/* 形容詞.終止連体 */
#define UNA_HIN_KEIYOU_GOKAN	77	/* 形容詞.語幹 */
#define UNA_HIN_KEIYOU_TSU	78	/* 形容詞.音便 */
#define UNA_HIN_KEIYOU_MI	79	/* 形容詞.ズ接続 */
#define UNA_HIN_KEIYOU_YOU	80	/* 形容詞.未然連用 */
#define UNA_HIN_KEIYOU_KA	81	/* 形容詞.仮定 */
#define UNA_HIN_KEIYOU_U	82	/* 形容詞.ウ接続 */
#define UNA_HIN_KEIYOU_KI	83	/* 形容詞.連体,文語.キ */
#define UNA_HIN_KEIYOU_MEI	84	/* 形容詞.命令,文語 */
#define UNA_HIN_UNKNOWN_MEISHI_SAHEN	85	/* 未登録語.名詞,サ変 */
#define UNA_HIN_UNKNOWN_MEISHI_KOYUU	86	/* 未登録語.名詞,固有 */
#define UNA_HIN_UNKNOWN_KIGOU	87	/* 未登録語.記号 */
#define UNA_HIN_UNKNOWN_KIGOU_ALPHABET	88	/* 未登録語.記号,アルファベット */
#define UNA_HIN_UNKNOWN_KATSUYOUGO	89	/* 未登録語.活用語 */
#define UNA_HIN_UNKNOWN_IPPAN	90	/* 未登録語.一般 */
#define UNA_HIN_USER_DEFINED_1	91	/* ユーザ定義1 */
#define UNA_HIN_USER_DEFINED_2	92	/* ユーザ定義2 */
#define UNA_HIN_USER_DEFINED_3	93	/* ユーザ定義3 */
#define UNA_HIN_USER_DEFINED_4	94	/* ユーザ定義4 */
#define UNA_HIN_USER_DEFINED_5	95	/* ユーザ定義5 */
#define UNA_HIN_USER_DEFINED_6	96	/* ユーザ定義6 */
#define UNA_HIN_USER_DEFINED_7	97	/* ユーザ定義7 */
#define UNA_HIN_USER_DEFINED_8	98	/* ユーザ定義8 */
#define	UNA_HIN_VOID	99			/* 無効語 */
									/* ラティス上には載るが、最適パス生成時に*/
									/* 除外され、出力には含まれない */

/* 無効語をもたない辞書ではUNA_HIN_VOIDに相当する品詞番号が通常品詞となる
   ラティス上の品詞を品詞番号だけで無効語と判別するために、
   すべての辞書で共通の、無効語を表す品詞番号をローカルに定義し、
   unaMorph_latSet()で登録する語が無効語のとき(search125()のみ)
   そのローカル無効語の品詞番号に置き換えて登録する */
#define	LOCAL_HIN_VOID	65535		/* ローカル無効語 */

/* 累積コストの最大値 */
#define UNA_MAX_ACUM_COST UINT_MAX

/* ラティスのノード数の最大値
	Obrother のデータタイプの表わせる最大個数 - 1 以下でなければならない
	現在は Obrother は ucharT なので 0xFF 以下と言う制限になる */
#define UNAMORPH_LAT_BRNCH_MAX 255	/* 256個 - 1個 */

/* 標準の辞書優先度値(英語トークン, 未登録語, タグの登録時に指定する) */
#define UNAMORPH_DEFAULT_PRIO 1		/* 優先度高, 常に登録される */

//--------------------------------------------------------------------------
// TAG:	  unaMorphT
//
// ABSTRACT:	  形態素データ型
//
// NOTE:
//	  ひとつの形態素を表わす型。形態素解析によってこのデータが
//	  生成されていく。
//	  表記は実際には、形態素解析のために入力されたテキスト上の
//	  その形態素の位置を指している。
//	  appI(アプリケーションインデックス情報)は、次の情報からなる。
//		  上位 8ビット(1バイト): 辞書番号
//        下位24ビット(3バイト): 単語ID
//
typedef struct unaMorphT {	// 形態素データ型
	unaCharT *start;		/* 表記(unaCharT へのポインタ) */
	sshortT  length;		/* 表記の長さ(文字数) */
	ushortT  hinshi;		/* 形態素品詞番号 */
	uintT    appI;			/* アプリケーションインデックス情報 */
	uintT    subI;			/* 下位構造情報 */
	ushortT  cost;			/* cost */
} unaMorphT;

//--------------------------------------------------------------------------
// TAG:	  unaBrnchT
//
// ABSTRACT:	  形態素枝構造体
//
// NOTE:
//    形態素を格納する形態素枝(ノード)の構造体
//
typedef struct unaBrnchT{ // 形態素枝構造体
	sshortT st;		 /* 入力文字列中の開始位置 */
	ushortT hin;	 /* 形態素品詞番号(無効語の情報も含む) */
	unaHinT unaHin;  /* UNA品詞番号(同品詞の判定に使う; 無効語は0) */
	uintT acumCost;  /* 累積コスト */
	ucharT ln;		 /* 長さ */
	ucharT dummy[3]; /* パディング用(for SPARK) */
	ushortT cost;	 /* その形態素のコスト(最大255、但し未登録語は別) */
	ucharT parent;	 /* 前接形態素枝のうち最小コストのもの */
	ucharT Obrother; /* 同じ終了位置で終わる形態素枝 */
	uintT appI;	     /* アプリケーションインデックス情報 */
	uintT subI;	     /* 下位構造情報 */
	ucharT dicPrio;  /* 辞書優先度 */
} unaBrnchT;

//--------------------------------------------------------------------------
// TAG:	  unaLatticeT
//
// ABSTRACT:	  形態素ラティス構造体
//
// NOTE:
//    形態素ラティスを格納する構造体
//
typedef struct unaLatticeT { // 形態素ラティス構造体
	unaBrnchT morBrnch[UNAMORPH_LAT_BRNCH_MAX + 1];
						 /* 形態素枝(ノード)、+1 は根の分(添字で言うと[0]) */
	unaCharT *tbuf;		 /* 入力文字列(テキスト全体)へのポインタ。
							この文字列を形態素解析する */
	int txtLen;			 /* 入力文字列(テキスト全体)の長さ。
							但し unaMorph_gen で元々指定されてきた文字列長が
							UNA_LOCAL_TEXT_SIZE以上か、負の場合には
							UNA_LOCAL_TEXT_SIZE がセットされる。
							これ以前にNULLがある場合にはそこまでが対象 */
	int latticeEnd;		 /* 解析中には、ラティスに登録された形態素のうち
							テキスト上で最も末端位置に達しているその
							最後尾位置がセットされる。そして収束直後には
							テキストのここまで解析したという意味になる。
							1起算。5なら5文字目まで解析完了ということ */
	ucharT brnchIndex[UNA_LOCAL_TEXT_SIZE + 2];
						 /* その位置を終わりとする形態素へのポインタ。
							例えば、6文字目で丁度終わる形態素を探したい時
							[6] を見ればよく、これは morBrnch の添字である。
							[1]から[256]を使用する。[256]は強制収束時の
							仮想文節頭末分のための予備 */
	ucharT morChk[UNA_HYOKI_LEN_MAX + 1];
						 /* 既登録語と同じ位置、長さの未登録語を登録しない様
							チェックする配列。チェックには[1]から[15]を使用
							するが、フラグたては表記の長さの場所にするので
							このサイズになってる。文字長7の形態素が登録
							されているかどうかは[7]を見る */
	ucharT curBrnchPos;	 /* ここまでは使用してあるという morBrnch の添字。
							1起算。8 なら、[8]迄使用済みということ */
	ucharT stBrnchPos;	 /* 同位置より検出された形態素が登録される最初の
							morBrnch の添字。1起算。
							今この値が8でtxtPos=2で検出された形態素を登録中
							とすると[8]から登録が始まったという事を表わす。
							txtPos移動ごとに更新される */
	ucharT prBrnchPos;	 /* 連語が存在した場合の morBrnch の添字。1起算。
						    8ならmorBrnch[8]に連語があるという事。
						    0は、連語がないという事 */
} unaLatticeT;

//--------------------------------------------------------------------------
// TAG:	  unaMorphHandleT
//
// ABSTRACT:	  ハンドラ型
//
// NOTE:
//	  形態素解析メインモジュールをマルチスレッドセーフで実行するための
//	  ハンドラ
//
typedef struct unaMorphHandleT{	// 形態素解析メインモジュールハンドル構造体
	const char *cnctTblImg;		/* 接続表のイメージ */
	const int *hinNamePos;		/* 品詞名の位置配列 */
	const unaCharT *hinNamePool;/* 品詞名の文字列プール */
	uintT morpHinNumMax;		/* 品詞数。形態素品詞番号がそのままテーブル
								   の要素番号になるように、実際に存在する
								   品詞数よりも1つ大きい。 */
	uintT kakariNumMax;			/* かかりコード数 */
	uintT ukeNumMax;			/* うけコード数 */
	const unaHinT *kakari;		/* 品詞番号→かかりコード変換テーブル */
	const unaHinT *uke;			/* 品詞番号→うけコード変換テーブル */
	const unaHinT *unaHinNumTable;
								/* 形態素品詞番号からUNA品詞番号への変換
								   テーブル。[0]はダミーデータである。 */
	const ucharT  *connectCostTable;
								/* 形態素品詞1、形態素品詞2をインデックスに
								   したコストテーブル。但し1次元的に表現して
								   る。[0][n]、[n][0]はダミーデータである */
	unaLatticeT lat;			/* 形態素ラティス */
	int hinBunEnd;				/* 仮想文末品詞の形態素品詞番号 */
	int maeHin;					/* 前接形態素の形態素品詞番号 */
	uintT mwLen;				/* 単語最大長 */
}unaMorphHandleT;

/* 形態素検出関数の型(キャスト用) */
typedef int (*unaDicFuncT)(unaMorphHandleT *mh,int txtPos,
							int dicNum,int *morCount,void *arg);

/* 下位構造取得関数の型(キャスト用) */
typedef int (*unaSubFuncT)(const unaMorphT *morph,unaMorphT *morphBuf,
				int *morphNum,const int morphBufSize,void *arg);

/* 優先登録処理関数の型(キャスト用) */
typedef int (*unaPrioFuncT)(unaMorphHandleT *mh,int st,int *ed,
							 void *arg);

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言とマクロ関数
//--------------------------------------------------------------------------

/* 初期化処理 */
int unaMorph_init(unaMorphHandleT *mh,const char *cnctTblImg,uintT maxWordLen);
/* 終了処理 */
int unaMorph_term(unaMorphHandleT *mh);
/* デバッグフラグの設定 */
int unaMorph_setDebugFlag(int flag);
/* 形態素解析メイン関数 */
int unaMorph_gen(unaMorphHandleT *mh, unaMorphT *wbuf, int wbufLen,
	unaCharT *tbuf, int txtLen,unaFuncClT *dicFunc,int *processedTxtLen,
	unaStopFuncT stopFunc);
/* 形態素ラティスに形態素を追加する */
int unaMorph_latSet(unaMorphHandleT *mh,int st,int ln,int hin,
	int cost,uintT appI,uintT subI,ucharT dicPrio,int prioFlg);
/* 親形態素とリンクする */
int unaMorph_linkWithParent(unaMorphHandleT *mh,int st,int stPos,
	int endPos);
/* latticeEndの整合を取る */
int unaMorph_resetLatEnd(unaLatticeT *lat,int ed);
/* 形態素品詞番号からUNA品詞番号を取得する */
int unaMorph_getUnaHin(const unaMorphHandleT *mh,ushortT morHin,
	unaHinT *unaHin);
/* 形態素品詞番号から形態素品詞名を取得する */
const unaCharT* unaMorph_getHinName(const unaMorphHandleT *mh,ushortT morHin);
/* 形態素解析モジュールに明示的に、現在、文末であることを知らせる */
void unaMorph_termSentence(unaMorphHandleT *mh);
/* 形態素解析モジュールから、現在の文末を取得する */
int unaMorph_getSentenceTail(unaMorphHandleT *mh);
/* 形態素解析モジュールに、現在の文末をセットする(次回解析用) */
int unaMorph_setSentenceTail(unaMorphHandleT *mh, int hin);
#endif /* end of UNAMORPH_H */

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
