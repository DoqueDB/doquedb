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

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------

#include <string.h>			/* strcmp */
#include <assert.h>
#include "UnaBase/unamdtri.h"		/* 形態素辞書引きモジュール自身 */
#include "UnaBase/unamorph.h"		/* 形態素辞書引きモジュール自身 */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------
#define MODULE_NAME "UNAMDTRI"	/* モジュール名 */

/* モジュール内のメッセージ */
#define SUB_MORPH_BUF_OVER "Sub Morph Buffer Overflow"
#define ERR_VERSION_MORPH  "Unsupported Morph Dictionary Version"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define EOK 0x0000			/* 終了への遷移キーのコード */
#define NWL 0x000a			/* 改行(0x0a) */
#define RET 0x000d			/* 復帰(0x0d) */

/* モジュール内部で使用する関数のプロトタイプ宣言 */


/* ハンドラのリセット */
static void resetHandle(unaMdTriHandleT *th);

/* V125用関数 */
static int init125(unaMdTriHandleT *th, const char *morphDicImg,
	const char *dicName, const ucharT dicPrio);
static int search125(unaMorphHandleT *mh,int  txtPos,int dicNum,
	int *morCount, void *tHandle);
static int prio125(unaMorphHandleT	*mh,int st,int *ed,void *tHandle);
static int getSub125(const unaMorphT *morph, unaMorphT *morphBuf,
	int *morphNum, const int morphBufSize, unaMdTriHandleT *th);
static int BreakSubMor125(int start,int dicNum,uintT lstNo,unaBrnchT *brnch,
		int *brnchNum,const int brnchBufSize,const unaMdTriHandleT *th);

/* V124用関数 */
static int init124(unaMdTriHandleT *th, const char *morphDicImg,
	const char *dicName, const ucharT dicPrio);
static int search124(unaMorphHandleT *mh,int  txtPos,int dicNum,
	int *morCount, void *tHandle);
static int prio124(unaMorphHandleT	*mh,int st,int *ed,void *tHandle);
static int getSub124(const unaMorphT *morph, unaMorphT *morphBuf,
	int *morphNum, const int morphBufSize, unaMdTriHandleT *th);
static int BreakSubMor124(int start,int dicNum,uintT lstNo,unaBrnchT *brnch,
		int *brnchNum,const int brnchBufSize,const unaMdTriHandleT *th);

/* NULL辞書用関数 */
static int initNull(unaMdTriHandleT *th, const char *morphDicImg,
	const char *dicName, const ucharT dicPrio);
static int searchNull(unaMorphHandleT *mh,int  txtPos,int dicNum,
	int *morCount, void *tHandle);
static int prioNull(unaMorphHandleT	*mh,int st,int *ed,void *tHandle);
static int getSubNull(const unaMorphT *morph, unaMorphT *morphBuf,
	int *morphNum, const int morphBufSize, unaMdTriHandleT *th);

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_init
//
// ABSTRACT:    形態素辞書引きモジュールの初期化
//
// FUNCTION:
//	  形態素辞書引きモジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//    なし
//
int unaMdicTrie_init(
		unaMdTriHandleT *th,	/* ハンドラ */
		const char *morphDicImg,/* 形態素辞書のイメージ */
		const char *dicName,	/* 辞書ベース名 */
		const ucharT dicPrio	/* 辞書優先度 */
)
{
	/* カラ辞書の場合の初期化 */
	resetHandle(th);
	if ( morphDicImg == (const char*)NULL){
		return initNull(th, NULL, NULL, UNAMORPH_DEFAULT_PRIO);
	}

	 /* version 1.25の初期化を実施 */
	resetHandle(th);
	if ( init125(th, morphDicImg, dicName, dicPrio) == UNA_OK){
		return UNA_OK;
	}

	/* version 1.24の初期化を実施 */
	resetHandle(th);
	if ( init124(th, morphDicImg, dicName, dicPrio) == UNA_OK){
		return UNA_OK;
	}

	return UNA_ERR_VERSION_MORPH; /* 形態素辞書バージョンエラー */
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_searchMorph
//
// ABSTRACT:    指定したTRIE辞書を使った辞書引き
//
// FUNCTION:
//	  指定した文字位置(オフセット)から始まる形態素候補を辞書引きして
//	  格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//
int unaMdicTrie_searchMorph(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int  txtPos,			/* テキスト上で解析を行う開始位置(オフセット) */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 格納した形態素数 */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	unaMdTriHandleT *th;
	th = (unaMdTriHandleT*)tHandle;
	return ((*(th->searchFunc))(mh,txtPos,dicNum,morCount,tHandle));
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_getDicName
//
// ABSTRACT:	  辞書ベース名取得
//
// FUNCTION:
//	  初期化時に設定された辞書ベース名を返す
//
// RETURN:
//	  辞書ベース名
//
// NOTE:
//
char *unaMdicTrie_getDicName(
		unaMdTriHandleT *th		/* ハンドラ */
)
{
	return (char *)(th->dicName);
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_term
//
// ABSTRACT:	  形態素辞書引きモジュールの終了処理
//
// FUNCTION:
//	  形態素辞書引きモジュールの終了処理を行う
//
// RETURN:
//	  UNA_OK	 正常終了
//
// NOTE:
//
int unaMdicTrie_term(
		unaMdTriHandleT *th		/* ハンドラ */
)
{
	resetHandle(th);
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_setCR
//
// ABSTRACT:    改行指示のセット
//
// FUNCTION:
//	 単語辞書引きを行う場合に改行をまたぐ単語を検出するか否かをセットする
//
// RETURN:
//    なし
//
// NOTE:
//    なし
//
void unaMdicTrie_setCR(
		unaMdTriHandleT *th,	/* ハンドラ */
		int ignoreCrMode		/* 改行指示 UNA_TRUE で改行単語を検出 */
)
{
	th->ignoreCR = ignoreCrMode;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_getSubMorph
//
// ABSTRACT:    下位構造の取得(unaMorphT型)
//
// FUNCTION:
//	  辞書登録語の下位構造形態素を取得する
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
//
// NOTE:
//
int unaMdicTrie_getSubMorph(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,		/* バッファの大きさ(要素数) */
	unaMdTriHandleT *th			/* ハンドラ */
)
{
	int rv;
	rv =  (*(th->getSubFunc))(morph,morphBuf,morphNum,morphBufSize,th);

	/* 改行コードを含む表記となった場合は修正する */
	if (th->ignoreCR == UNA_TRUE){
		int offset;
		offset=0;
		for ( int i=0;i<(*morphNum);++i){
			assert(offset<morph[0].length);
			morphBuf[i].start += offset;
			for ( int j=0; j < morphBuf[i].length;++j){
				if ( morphBuf[i].start[j] == 0x0d ||
				     morphBuf[i].start[j] == 0x0a ){
					morphBuf[i].length ++;
					offset++;
					assert(morphBuf[i].length<morph[0].length);
				}
			}
		}
	}

	return rv;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicTrie_prioMorph
//
// ABSTRACT:    辞書登録語優先登録処理
//
// FUNCTION:
//	  辞書登録語の優先登録処理を行う。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
int unaMdicTrie_prioMorph(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int *ed,				/* 形態素の文字列上での末尾位置(オフセット) */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	return (*(((unaMdTriHandleT*)tHandle)->prioFunc))(mh,st,ed,tHandle);
}

//--------------------------------------------------------------------------
// MODULE:	  resetHandle
//
// ABSTRACT:	  ハンドラのリセット
//
// FUNCTION:
//	  ハンドラ内の各値をリセットする
//
// RETURN:
//	  無し
//
// NOTE:
//
void resetHandle(
		unaMdTriHandleT *th		/* ハンドラ */
)
{
	th->morphDicImg = (const char*)0;
	th->recCount = 0;
	th->lstCount = 0;
	th->daCount = 0;
	th->cnvTbl = (const unaCharT*)0;
	th->morData = (const unaMorDataT*)0;
	th->cMorData = (const unaCompactMorDataT*)0;
	th->subLst = (const uintT*)0;
	th->base = (const uintT*)0;
	th->label = (const unaCharT*)0;
	th->bl = (const baseAndLabelT*)0;
	th->ignoreCR = 0;
	th->dicName = (const char*)0;
	th->dicPrio = 0;
	th->hasVoid = UNA_FALSE;
}

//--------------------------------------------------------------------------
// MODULE:	  search125
//
// ABSTRACT:    指定したTRIE辞書を使った辞書引き(圧縮版)
//
// FUNCTION:
//	  指定した文字位置(オフセット)から始まる形態素候補を辞書引きして
//	  格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//
static int search125(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* テキスト上で解析を行う開始位置(オフセット) */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 格納した形態素数 */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	const unaMdTriHandleT *th;	/* ハンドラ(形態素辞書引き用) */
	int		i;					/* ループ変数 */
	uintT	j;					/* ループ変数 */
	int		rv;					/* 関数の返り値 */
	uintT	r;					/* ベース値 */
	uintT	morLen;				/* 形態素の長さ */
	uintT	appI;				/* アプリケーションインデックス情報 */
	uintT	id;					/* ID(最大24ビット=0xFFFFFF、
								   辞書作成ツールにもコメント済み) */
	int	prioFlg;				/* 優先登録フラグ */
	int localHin;				/* ローカル品詞番号 */
	int	returnValue;			/* この関数の返り値(最後に返す値を保持) */
	int nlLen;					/* 復帰/改行コードの連続数 */
	unaCharT inCode;
	th = (const unaMdTriHandleT *)tHandle;
	r = 0;
	*morCount	= 0;
	morLen		= 0;
	returnValue = UNA_OK;

	nlLen = 0;
	for(i = txtPos;i < mh->lat.txtLen && mh->lat.tbuf[i] != 0x0000;i++){
		
		/* 内部コードに変換 */
		inCode = th->cnvTbl[ mh->lat.tbuf[i]];

		/* 遷移可能かチェックを行う */
		if ((th->label)[r + inCode] != inCode ){

		  	/* 遷移不可能の場合、基本的に終了するが、以下のみ連続する */
			if ( th->ignoreCR == UNA_TRUE){

		   		/* 単語中に１回だけ出現した復帰/改行コードは読み飛ばす */
            	if ( inCode == RET && nlLen<1 && morLen>0){
              		if ( th->cnvTbl[mh->lat.tbuf[i+1]]==NWL){
                		morLen+=2;
                		nlLen++;
                		i++;
                		continue;
              		}
            	}
            	else if ( inCode == NWL && nlLen<1 && morLen>0){
              		morLen++;
              		nlLen++;
              		continue;
            	}
          	}
          	break;	/* 遷移不可能の時は抜ける */
		}

		r = th->base[r + inCode];	/* 遷移する */
        nlLen = 0;  /* 通常の遷移->改行コードの連続から抜けた->改行連続長0 */
		morLen++;					/* 遷移できたので長さをカウントアップ */

		if ( morLen> mh->mwLen){
          break;	/* 単語最大長によるチェック */
		}
		  
		if (th->label[r] == EOK){	/* ベースのラベルそのものの値がEOK */
			/* 部分文字列発見(辞書が引けた) */
			id = th->base[r];
			/* 連語の時(連語は同形語を持たないのでここでチェック) */
			prioFlg		= UNA_FALSE;	/* 優先登録フラグリセット */
			if ((th->morData)[id].morHinNo == UNA_HIN_RENGO_V125) {
				assert((th->morData)[id].doukeiNo == 1);
					/* 連語には同形語が無い(辞書作成ツールでチェック済み) */
				prioFlg = UNA_TRUE;	/* 優先登録語(連語)ならフラグ真 */
				returnValue = UNA_SYUSOKU;	/* 無条件に収束 */
			}
			for (j = 0 ; j < (th->morData)[th->base[r]].doukeiNo ; j++){
				/* ラティスに登録 */
				appI = (dicNum << 24) | id;
				localHin = (th->morData)[id].morHinNo;
				if (th->hasVoid == UNA_TRUE && localHin == UNA_HIN_VOID) {
					/* 無効語あり辞書のUNA_HIN_VOIDは無効語のため
					   ローカル無効語品詞に置き換えて登録 */
					localHin = LOCAL_HIN_VOID;
				}
				rv = unaMorph_latSet(mh,txtPos,morLen,
								localHin,
								(th->morData)[id].morCost,appI,
								(th->morData)[id].subLstNo,
								th->dicPrio,prioFlg);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
				id++;
			}
		}
	}

	/* 終了 */
	return returnValue;
}

//--------------------------------------------------------------------------
// MODULE:   init125
// 
// ABSTRACT:    Initialization of morpheme dictionary pulling module (compressed edition)
// 
// FUNCTION:
//	  形態素辞書引きモジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//    なし
// 
static int init125(
		unaMdTriHandleT *th,	/* ハンドラ */
		const char *morphDicImg,/* 形態素辞書のイメージ */
		const char *dicName,	/* 辞書ベース名 */
		const ucharT dicPrio    /* 辞書優先度 */
)
{
	const char *imgPtr;	/*  辞書を格納したメモリへのポインタ */

	/* 形態素辞書の情報を設定する */
	th->morphDicImg = morphDicImg;		/* 形態素辞書のイメージ */
	imgPtr = morphDicImg + UNA_COM_SIZE;

	if (strcmp(imgPtr, UNA_DIC_VER) == 0) {
		/* V1.25 無効語あり辞書 */
		th->hasVoid = UNA_TRUE;
	} else if (strcmp(imgPtr, UNA_DIC_VER_125) == 0) {
		/* V1.25 無効語なし辞書 */
		th->hasVoid = UNA_FALSE;
	} else {
		/* バージョンが違う */
		UNA_RETURN(ERR_VERSION_MORPH,NULL);
	}
	imgPtr += sizeof(UNA_DIC_VER);
	th->recCount = *(uintT *)imgPtr;	/* 登録されている形態素の数 */
	imgPtr += sizeof(uintT);
	th->lstCount = *(uintT *)imgPtr;	/* 下位構造リストのエントリ数 */
	imgPtr += sizeof(uintT);
	th->daCount = *(uintT *)imgPtr;		/* base、label 配列の数 */
	imgPtr += sizeof(uintT);
	th->cnvTbl = (unaCharT *)imgPtr;	/* unicode->内部コード変換テーブル */
	imgPtr += 0x10000*sizeof(unaCharT);

#if 0
	th->bl = (const baseAndLabelT *)imgPtr; /* baseとlabelの配列 */ 
	imgPtr += (th->daCount) * sizeof(baseAndLabelT);
#endif
	th->base = (const uintT *)imgPtr;		/* base配列の開始アドレス */
	imgPtr += (th->daCount) * sizeof(uintT);
	th->label = (const unaCharT *)imgPtr;	/* label配列の開始アドレス */
	imgPtr += (th->daCount) * sizeof(unaCharT);

	th->subLst = (const uintT *)imgPtr;
											/* 下位構造リストの開始アドレス */
	imgPtr += (th->lstCount) * sizeof(uintT);

	th->morData = (const unaMorDataT *)imgPtr;
											/* 形態素基本データの開始アドレス */
	imgPtr += (th->recCount) * sizeof(unaMorDataT);

	th->searchFunc = (unaDicFuncT)search125; /* 検索用関数の設定 */
	th->getSubFunc = (unaSubFuncT)getSub125; /* 下位構造取得関数の設定 */
	th->prioFunc   = (unaPrioFuncT)prio125;	/* 下位構造取得関数の設定 */

	th->dicName = dicName; /* 辞書ベース名 */
	th->dicPrio = dicPrio; /* 辞書優先度 */

	/* 正常終了 */
	return UNA_OK;
}


// MODULE:   prio125
// 
// ABSTRACT:    辞書登録語優先登録処理(圧縮)
//
// FUNCTION:
//	  辞書登録語の優先登録処理を行う。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  - 連語そのものをセットするのではなく、それを構成する下位形態素を
//		展開しセットする
//	  - 前接品詞との接続コストは、一番前の(下位)形態素との接続をチェック
//	  - 後続品詞との接続コストは、一番後の(下位)形態素との接続をチェック
//	  - latticeEndは、一番最後の(下位)形態素のもので更新する
//	  - 辞書引き及びコスト計算省略の関係上一番最後でない形態素は、
//		brnchIndexによってポイントされない
//		(brnchIndex に値をセットしない)
//	  - 辞書引き及びコスト計算省略の関係上一番最後でない形態素は、
//		他のObrotherによってポイントされない
//	  - 先頭の(下位)形態素の親はコスト最小法によって求め、先頭以外の
//		(下位)形態素の親は、無条件にその直前の(下位)形態素とする。
//	  - 最長の連語が末尾の形態素になっていったん収束して返る
//	  - 登録した(下位)形態素の開始位置は連続位置に置かれる(下記図参照)
//
//		前接形態素		今回登録した形態素
//			------		-----
//			  ----		     -----
//		   -------		          ---
//			    --		↑
//						先頭の形態素の親のみをコスト最小法で求める
// 
static int prio125(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int *ed,				/* 形態素の文字列上での末尾位置(オフセット) */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	const unaMdTriHandleT *th;	/* ハンドラ(形態素辞書引き用) */
	unaLatticeT *lat;			/* 形態素枝バッファ */
	int dicNum;					/* 辞書ナンバー */
	int brnchNum;				/* セットされた形態素の数 */
	int i;						/* ループ変数 */

	/* 初期設定 */
	th = (unaMdTriHandleT *)tHandle;	/* ハンドラの設定 */
	lat	= &(mh->lat);					/* ラティスの設定 */

	assert(lat->prBrnchPos != 0);

	dicNum	= (lat->morBrnch)[lat->prBrnchPos].appI >> 24;
									/* appIの上位8ビットが辞書番号 */
	/* stBrnchPos の位置から連語の下位形態素を展開する */
	(void)BreakSubMor125(st,dicNum,(lat->morBrnch)[lat->prBrnchPos].subI,
			&(lat->morBrnch)[lat->stBrnchPos],&brnchNum,
			UNAMORPH_LAT_BRNCH_MAX - lat->stBrnchPos + 1, th);
			/* オーバーフローが起った時はセットできるとこまでが
			   セットされ返る。少なくとも1個はセットされる */
	/* curBrnchPosのリセット */
	lat->curBrnchPos = (ucharT)(lat->stBrnchPos + brnchNum - 1);

	/* 親枝との接続(先頭の下位形態素のみ) */
	(void)unaMorph_linkWithParent(mh,st,lat->stBrnchPos,lat->stBrnchPos);

	*ed = st + (lat->morBrnch)[lat->stBrnchPos].ln;
	for (i = lat->stBrnchPos + 1;i <= lat->curBrnchPos;i++) {
		(lat->morBrnch)[i].parent	= (ucharT)(i - 1); /* 前の形態素が親 */
		(lat->morBrnch)[i].acumCost	= (lat->morBrnch)[i - 1].acumCost;
					/* 先頭以外の下位形態素のacumCostは、先頭と同じくする */
		*ed += (lat->morBrnch)[i].ln;
	}
	/* 下記はオーバーフロー処理と同じ理由で実行 */
	(void)unaMorph_resetLatEnd(lat,*ed);

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   getSub125
// 
// ABSTRACT:    下位構造の取得(unaMorphT型)(圧縮版)
//
// FUNCTION:
//	  辞書登録語の下位構造形態素を取得する
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
// 
// NOTE:
// 
static int getSub125(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,		/* バッファの大きさ(要素数) */
	unaMdTriHandleT *th			/* ハンドラ */
)
{
	int dicNum;					/* 辞書ナンバー */
	uintT	id;					/* ID(最大24ビット=0xFFFFFF、
								   辞書作成ツールにもコメント済み) */
	uintT lstNo;				/* 下位構造情報リストへのポインタ */
	unaBrnchT brnch[UNA_SUB_MORPH_CNT_MAX];
								/* 下位構造がセットされるバッファ */
	int rv;						/* 関数の返り値 */
	int i;						/* ループ変数 */

	dicNum	= morph->appI >> 24;		/* appIの上位8ビットが辞書番号 */
	id		= morph->appI & 0x00ffffff;	/* 下位3バイトがid(レコード番号) */
	lstNo	= (th->morData)[id].subLstNo;

	if (lstNo == 0xFFFFFF) {	/* 0xFFFFFFのとき */
		*morphNum = 0;			/* 下位構造はない */
		return UNA_OK;
	}

	rv = BreakSubMor125(0,dicNum,lstNo,&brnch[0],morphNum,morphBufSize,th);
	if (rv < 0) {
		return rv;
	}

	for (i = 0;i < *morphNum;i++) {
		morphBuf[i].start	= (unaCharT *)(morph->start) + brnch[i].st;
		morphBuf[i].length	= brnch[i].ln;
		morphBuf[i].hinshi	= brnch[i].hin;
		morphBuf[i].appI	= brnch[i].appI;
		morphBuf[i].subI	= brnch[i].subI;
		morphBuf[i].cost	= brnch[i].cost;
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   BreakSubMor125
// 
// ABSTRACT:    下位構造の取得(unaBrnchT型)
//
// FUNCTION:
//	  下位構造を取得する
//
// RETURN:
//	  UNA_OK					正常終了
//	  UNA_SUB_MORPH_BUF_OVER	下位形態素バッファオーバーフロー
//
// NOTE:
//	  バッファがオーバーフローを起こす時は、セットできるところまでを
//	  セットして返す。
//
//	(注1)
//		当該形態素の下位構造リストの先頭エントリには、下位構造の数がセット
//		されている。そして、続くエントリからその下位構造の数分のエントリに
//		下位形態素へのポインタが格納されている。(下記例参照)
//
//		+---------------------------+
//		|下位構造リストの先頭		|
//		+---------------------------+
//					・				  当該形態素の下位構造情報へのポインタ
//					・				 (下位構造リストの先頭からのオフセット。
//					・				  辞書登録語の場合、subI に入っている)
//		+---------------------------+				   |
//		|下位構造の数(例えば 2)		| ←---------------+ 
//		+---------------------------+
//		|下位形態素1へのポインタ	|
//		+---------------------------+
//		|下位形態素2へのポインタ	|
//		+---------------------------+
//
static int BreakSubMor125(
	int start,				  /* テキスト中でのオフセット */
	int dicNum,				  /* 辞書ナンバー */
	uintT lstNo,			  /* 下位構造情報リストへのポインタ(オフセット)*/
	unaBrnchT *brnch,		  /* 下位構造形態素がセットされるバッファ */
	int *brnchNum,			  /* セットされた下位構造形態素の数 */
	const int brnchBufSize,   /* バッファの大きさ(要素数) */
	const unaMdTriHandleT *th /* ハンドラ */
)
{
	uintT	i;				/* ループ変数 */
	uintT	id;				/* 下位構造の形態素のid */

	assert(lstNo != 0xFFFFFF); /* 上位モジュールでチェック済み */
	for (i = 1;i <= (th->subLst)[lstNo];i++) {	/* 注1 */
		if (i > (uintT)brnchBufSize) {
			*brnchNum = i - 1;	/* i - 1 個はセット済み */
			UNA_RETURN(SUB_MORPH_BUF_OVER,NULL);
		}
		id = (th->subLst)[lstNo + i];
		brnch[i - 1].st			= (sshortT)start;
		brnch[i - 1].hin		= (th->morData)[id].morHinNo;
		brnch[i - 1].ln			= (th->morData)[id].morLen;
		brnch[i - 1].cost		= (th->morData)[id].morCost;
		brnch[i - 1].appI		= (dicNum << 24) | id;
		brnch[i - 1].subI		= (th->morData)[id].subLstNo;
		brnch[i - 1].Obrother	= 0;					/* 連語用 */
		brnch[i - 1].acumCost	= UNA_MAX_ACUM_COST;	/* 連語用 */
		start += (th->morData)[id].morLen;
	}

	*brnchNum = i - 1;	/* 通常のfor抜けの時は、iは1大きい */

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   search124
// 
// ABSTRACT:    指定したTRIE辞書を使った辞書引き(圧縮版)
//
// FUNCTION:
//	  指定した文字位置(オフセット)から始まる形態素候補を辞書引きして
//	  格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//
static int search124(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* テキスト上で解析を行う開始位置(オフセット) */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 格納した形態素数 */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	const unaMdTriHandleT *th;	/* ハンドラ(形態素辞書引き用) */
	int		i;					/* ループ変数 */
	uintT	j;					/* ループ変数 */
	int		rv;					/* 関数の返り値 */
	uintT	r;					/* ベース値 */
	uintT	morLen;				/* 形態素の長さ */
	uintT	appI;				/* アプリケーションインデックス情報 */
	uintT	id;					/* ID(最大24ビット=0xFFFFFF、
								   辞書作成ツールにもコメント済み) */
	int	prioFlg;				/* 優先登録フラグ */
	int	returnValue;			/* この関数の返り値(最後に返す値を保持) */
	int nlLen;					/* 復帰/改行コードの連続数 */
	unaCharT inCode;
	th = (const unaMdTriHandleT *)tHandle;
	r = 0;
	*morCount	= 0;
	morLen		= 0;
	returnValue = UNA_OK;

	nlLen = 0;
	for(i = txtPos;i < mh->lat.txtLen && mh->lat.tbuf[i] != 0x0000;i++){
		
		/* 内部コードに変換 */
		inCode = th->cnvTbl[ mh->lat.tbuf[i]];

		/* 遷移可能かチェックを行う */
		if ((th->bl)[r + inCode].label != inCode ){

		  	/* 遷移不可能の場合、基本的に終了するが、以下のみ連続する */
			if ( th->ignoreCR == UNA_TRUE){

		   		/* 単語中に１回だけ出現した復帰/改行コードは読み飛ばす */
            	if ( inCode == RET && nlLen<1 && morLen>0){
              		if ( th->cnvTbl[mh->lat.tbuf[i+1]]==NWL){
                		morLen+=2;
                		nlLen++;
                		i++;
                		continue;
              		}
            	}
            	else if ( inCode == NWL && nlLen<1 && morLen>0){
              		morLen++;
              		nlLen++;
              		continue;
            	}
          	}
          	break;	/* 遷移不可能の時は抜ける */
		}

		r = th->bl[r + inCode].base;	/* 遷移する */
        nlLen = 0;  /* 通常の遷移->改行コードの連続から抜けた->改行連続長0 */
		morLen++;					/* 遷移できたので長さをカウントアップ */

		if ( morLen> mh->mwLen){
          break;	/* 単語最大長によるチェック */
		}
		  
		if (th->bl[r].label == EOK){	/* ベースのラベルそのものの値がEOK */
			/* 部分文字列発見(辞書が引けた) */
			id = th->bl[r].base;
			/* 連語の時(連語は同形語を持たないのでここでチェック) */
			prioFlg		= UNA_FALSE;	/* 優先登録フラグリセット */
			if ((th->cMorData)[id].mor.morHinNo == UNA_HIN_RENGO_V124) {
				assert((th->cMorData)[id].mor.doukeiNo == 1);
					/* 連語には同形語が無い(辞書作成ツールでチェック済み) */
				prioFlg = UNA_TRUE;	/* 優先登録語(連語)ならフラグ真 */
				returnValue = UNA_SYUSOKU;	/* 無条件に収束 */
			}
			for (j = 0 ; j < (th->cMorData)[th->bl[r].base].mor.doukeiNo ; j++){
				/* ラティスに登録 */
				appI = (dicNum << 24) | id;
				if ( (th->cMorData)[th->bl[r].base].mor.subFlg == 1){
					rv = unaMorph_latSet(mh,txtPos,morLen,
									(th->cMorData)[id].mor.morHinNo,
									(th->cMorData)[id].mor.morCost,appI,
									(th->cMorData)[id+1].sub.subLstNo,
									th->dicPrio,prioFlg);
									id++;
				}
				else{
					rv = unaMorph_latSet(mh,txtPos,morLen,
									(th->cMorData)[id].mor.morHinNo,
									(th->cMorData)[id].mor.morCost,appI,
									0xffffff,th->dicPrio,prioFlg);
				}
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
				id++;
			}
		}
	}

	/* 終了 */
	return returnValue;
}

//--------------------------------------------------------------------------
// MODULE:   init124
// 
// ABSTRACT:    形態素辞書引きモジュールの初期化(圧縮版)
//
// FUNCTION:
//	  形態素辞書引きモジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//    なし
// 
static int init124(
		unaMdTriHandleT *th,	/* ハンドラ */
		const char *morphDicImg,/* 形態素辞書のイメージ */
		const char *dicName,    /* 辞書ベース名 */
		const ucharT dicPrio    /* 辞書優先度 */
)
{
	const char *imgPtr;	/* 辞書を格納したメモリへのポインタ */

	/* 形態素辞書の情報を設定する */
	th->morphDicImg = morphDicImg;		/* 形態素辞書のイメージ */
	imgPtr = morphDicImg + UNA_COM_SIZE;

	if (strcmp(imgPtr,UNA_DIC_VER_124)!=0) { /* バージョンが違う */
		UNA_RETURN(ERR_VERSION_MORPH,NULL);
	}
	imgPtr += sizeof(UNA_DIC_VER);
	th->recCount = *(uintT *)imgPtr;	/* 登録されている形態素の数 */
	imgPtr += sizeof(uintT);
	th->lstCount = *(uintT *)imgPtr;	/* 下位構造リストのエントリ数 */
	imgPtr += sizeof(uintT);
	th->daCount = *(uintT *)imgPtr;		/* base、label 配列の数 */
	imgPtr += sizeof(uintT);
	th->cnvTbl = (unaCharT *)imgPtr;	/* unicode->内部コード変換テーブル */
	imgPtr += 0x10000*sizeof(unaCharT);

	th->bl = (const baseAndLabelT *)imgPtr; /* baseとlabelの配列 */ 
	imgPtr += (th->daCount) * sizeof(baseAndLabelT);

	th->subLst = (const uintT *)imgPtr;
										/* 下位構造リストの開始アドレス */
	imgPtr += (th->lstCount) * sizeof(uintT);

	th->cMorData = (const unaCompactMorDataT *)imgPtr;
										/* 形態素基本データの開始アドレス */
	imgPtr += (th->recCount) * sizeof(unaMorDataT);

	th->searchFunc = (unaDicFuncT)search124; /* 検索用関数の設定 */
	th->getSubFunc = (unaSubFuncT)getSub124; /* 下位構造取得関数の設定 */
	th->prioFunc   = (unaPrioFuncT)prio124;	/* 下位構造取得関数の設定 */

	th->dicName = dicName; /* 辞書ベース名 */
	th->dicPrio = dicPrio; /* 辞書優先度 */
	th->hasVoid = UNA_FALSE; /* 無効語あり辞書か? (V1.24では常にFALSE) */

	/* 正常終了 */
	return UNA_OK;
}


// MODULE:   prio124
// 
// ABSTRACT:    辞書登録語優先登録処理(圧縮)
//
// FUNCTION:
//	  辞書登録語の優先登録処理を行う。
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  - 連語そのものをセットするのではなく、それを構成する下位形態素を
//		展開しセットする
//	  - 前接品詞との接続コストは、一番前の(下位)形態素との接続をチェック
//	  - 後続品詞との接続コストは、一番後の(下位)形態素との接続をチェック
//	  - latticeEndは、一番最後の(下位)形態素のもので更新する
//	  - 辞書引き及びコスト計算省略の関係上一番最後でない形態素は、
//		brnchIndexによってポイントされない
//		(brnchIndex に値をセットしない)
//	  - 辞書引き及びコスト計算省略の関係上一番最後でない形態素は、
//		他のObrotherによってポイントされない
//	  - 先頭の(下位)形態素の親はコスト最小法によって求め、先頭以外の
//		(下位)形態素の親は、無条件にその直前の(下位)形態素とする。
//	  - 最長の連語が末尾の形態素になっていったん収束して返る
//	  - 登録した(下位)形態素の開始位置は連続位置に置かれる(下記図参照)
//
//		前接形態素		今回登録した形態素
//			------		-----
//			  ----		     -----
//		   -------		          ---
//			    --		↑
//						先頭の形態素の親のみをコスト最小法で求める
// 
static int prio124(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int *ed,				/* 形態素の文字列上での末尾位置(オフセット) */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	const unaMdTriHandleT *th;	/* ハンドラ(形態素辞書引き用) */
	unaLatticeT *lat;			/* 形態素枝バッファ */
	int dicNum;					/* 辞書ナンバー */
	int brnchNum;				/* セットされた形態素の数 */
	int i;						/* ループ変数 */

	/* 初期設定 */
	th = (unaMdTriHandleT *)tHandle;	/* ハンドラの設定 */
	lat	= &(mh->lat);					/* ラティスの設定 */

	assert(lat->prBrnchPos != 0);

	dicNum	= (lat->morBrnch)[lat->prBrnchPos].appI >> 24;
									/* appIの上位8ビットが辞書番号 */
	/* stBrnchPos の位置から連語の下位形態素を展開する */
	(void)BreakSubMor124(st,dicNum,(lat->morBrnch)[lat->prBrnchPos].subI,
			&(lat->morBrnch)[lat->stBrnchPos],&brnchNum,
			UNAMORPH_LAT_BRNCH_MAX - lat->stBrnchPos + 1, th);
			/* オーバーフローが起った時はセットできるとこまでが
			   セットされ返る。少なくとも1個はセットされる */
	/* curBrnchPosのリセット */
	lat->curBrnchPos = (ucharT)(lat->stBrnchPos + brnchNum - 1);

	/* 親枝との接続(先頭の下位形態素のみ) */
	(void)unaMorph_linkWithParent(mh,st,lat->stBrnchPos,lat->stBrnchPos);

	*ed = st + (lat->morBrnch)[lat->stBrnchPos].ln;
	for (i = lat->stBrnchPos + 1;i <= lat->curBrnchPos;i++) {
		(lat->morBrnch)[i].parent	= (ucharT)(i - 1); /* 前の形態素が親 */
		(lat->morBrnch)[i].acumCost	= (lat->morBrnch)[i - 1].acumCost;
					/* 先頭以外の下位形態素のacumCostは、先頭と同じくする */
		*ed += (lat->morBrnch)[i].ln;
	}
	/* 下記はオーバーフロー処理と同じ理由で実行 */
	(void)unaMorph_resetLatEnd(lat,*ed);

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   getSub124
// 
// ABSTRACT:    下位構造の取得(unaMorphT型)(圧縮版)
//
// FUNCTION:
//	  辞書登録語の下位構造形態素を取得する
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
// 
// NOTE:
// 
static int getSub124(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,		/* バッファの大きさ(要素数) */
	unaMdTriHandleT *th			/* ハンドラ */
)
{
	int dicNum;					/* 辞書ナンバー */
	uintT id;					/* ID(最大24ビット=0xFFFFFF、
								   辞書作成ツールにもコメント済み) */
	uintT lstNo;				/* 下位構造情報リストへのポインタ */
	unaBrnchT brnch[UNA_SUB_MORPH_CNT_MAX];
								/* 下位構造がセットされるバッファ */
	int rv;						/* 関数の返り値 */
	int i;						/* ループ変数 */

	dicNum	= morph->appI >> 24;		/* appIの上位8ビットが辞書番号 */
	id		= morph->appI & 0x00ffffff;	/* 下位3バイトがid(レコード番号) */

	if ((th->cMorData)[id].mor.subFlg == 0) {
		*morphNum = 0;			/* 下位構造はない */
		return UNA_OK;
	}

	lstNo	= (th->cMorData)[id+1].sub.subLstNo;

	rv = BreakSubMor124(0,dicNum,lstNo,&brnch[0],morphNum,morphBufSize,th);
	if (rv < 0) {
		return rv;
	}

	for (i = 0;i < *morphNum;i++) {
		morphBuf[i].start	= (unaCharT *)(morph->start) + brnch[i].st;
		morphBuf[i].length	= brnch[i].ln;
		morphBuf[i].hinshi	= brnch[i].hin;
		morphBuf[i].appI	= brnch[i].appI;
		morphBuf[i].subI	= brnch[i].subI;
		morphBuf[i].cost	= brnch[i].cost;
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   BreakSubMor124
// 
// ABSTRACT:    下位構造の取得(unaBrnchT型)
//
// FUNCTION:
//	  下位構造を取得する
//
// RETURN:
//	  UNA_OK					正常終了
//	  UNA_SUB_MORPH_BUF_OVER	下位形態素バッファオーバーフロー
//
// NOTE:
//	  バッファがオーバーフローを起こす時は、セットできるところまでを
//	  セットして返す。
//
//	(注1)
//		当該形態素の下位構造リストの先頭エントリには、下位構造の数がセット
//		されている。そして、続くエントリからその下位構造の数分のエントリに
//		下位形態素へのポインタが格納されている。(下記例参照)
//
//		+---------------------------+
//		|下位構造リストの先頭		|
//		+---------------------------+
//					・				  当該形態素の下位構造情報へのポインタ
//					・				 (下位構造リストの先頭からのオフセット。
//					・				  辞書登録語の場合、subI に入っている)
//		+---------------------------+				   |
//		|下位構造の数(例えば 2)		| ←---------------+ 
//		+---------------------------+
//		|下位形態素1へのポインタ	|
//		+---------------------------+
//		|下位形態素2へのポインタ	|
// 		+--------------------------------------+
//
static int BreakSubMor124(
	int start,				  /* テキスト中でのオフセット */
	int dicNum,				  /* 辞書ナンバー */
	uintT lstNo,			  /* 下位構造情報リストへのポインタ(オフセット)*/
	unaBrnchT *brnch,		  /* 下位構造形態素がセットされるバッファ */
	int *brnchNum,			  /* セットされた下位構造形態素の数 */
	const int brnchBufSize,   /* バッファの大きさ(要素数) */
	const unaMdTriHandleT *th /* ハンドラ */
)
{
	uintT	i;				/* ループ変数 */
	uintT	id;				/* 下位構造の形態素のid */

	assert(lstNo != 0xFFFFFF); /* 上位モジュールでチェック済み */
	for (i = 1;i <= (th->subLst)[lstNo];i++) {	/* 注1 */
		if (i > (uintT)brnchBufSize) {
			*brnchNum = i - 1;	/* i - 1 個はセット済み */
			UNA_RETURN(SUB_MORPH_BUF_OVER,NULL);
		}
		id = (th->subLst)[lstNo + i];
		brnch[i - 1].st			= (sshortT)start;
		brnch[i - 1].hin		= (th->cMorData)[id].mor.morHinNo;
		brnch[i - 1].cost		= (th->cMorData)[id].mor.morCost;
		brnch[i - 1].appI		= (dicNum << 24) | id;
		if ( (th->cMorData)[id].mor.subFlg==1){
			brnch[i - 1].subI		= (th->cMorData)[id+1].sub.subLstNo;
		}
		else{
			brnch[i - 1].subI		= 0xffffff;
		}
		brnch[i - 1].ln			= (th->cMorData)[id].mor.morLen;
		brnch[i - 1].Obrother	= 0;					/* 連語用 */
		brnch[i - 1].acumCost	= UNA_MAX_ACUM_COST;	/* 連語用 */
		start += (th->cMorData)[id].mor.morLen;
	}

	*brnchNum = i - 1;	/* 通常のfor抜けの時は、iは1大きい */

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:   initNull
// 
// ABSTRACT:    カラ辞書用形態素辞書引きモジュールの初期化
//
// FUNCTION:
//	  形態素辞書引きモジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//    なし
//
static int initNull(
		unaMdTriHandleT *th,	/* ハンドラ */
		const char *morphDicImg,/* 形態素辞書のイメージ */
		const char *dicName,    /* 辞書ベース名 */
		const ucharT dicPrio    /* 辞書優先度 */
)
{
	/* 形態素辞書の情報を設定する */
	th->morphDicImg = morphDicImg;		/* 形態素辞書のイメージ */
	if (dicName == NULL) {
		// 空辞書のベース名には空文字列を設定する
		dicName = "";
	}
	th->dicName = dicName;				/* 辞書ベース名 */
	th->dicPrio = dicPrio;				/* 辞書優先度 */

	th->searchFunc = (unaDicFuncT)searchNull; /* 検索用関数の設定 */
	th->getSubFunc = (unaSubFuncT)getSubNull; /* 下位構造取得関数の設定 */
	th->prioFunc   = (unaPrioFuncT)prioNull;  /* 下位構造取得関数の設定 */

	/* 正常終了 */
	return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE:   searchNull
// 
// ABSTRACT:    カラ辞書指定の場合のダミーの辞書引き
//
// FUNCTION:
//	  辞書引きを何もせずに返す
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
static int searchNull(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* テキスト上で解析を行う開始位置(オフセット) */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 格納した形態素数 */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	*morCount = 0;
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:   getSubNull
// 
// ABSTRACT:    ダミー用下位構造の取得(unaMorphT型)
//
// FUNCTION:
//	  呼ばれるはずはないので、呼ばれたらエラーを返す
//
// RETURN:
//    UNA_ERR_VERSION_MORPH
//
// NOTE:
//
static int getSubNull(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,	/* バッファの大きさ(要素数) */
	unaMdTriHandleT *th			/* ハンドラ */
)
{
	/* これが適切かどうかはよくわからないが・・・ */
	UNA_RETURN(ERR_VERSION_MORPH,NULL);
}
//--------------------------------------------------------------------------
// MODULE:   prioNull
// 
// ABSTRACT:    ダミー用辞書登録語優先登録処理
//
// FUNCTION:
//	  呼ばれるはずはないので、呼ばれたらエラーを返す
//
// RETURN:
//    UNA_ERR_VERSION_MORPH
//
// NOTE:
static int prioNull(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int *ed,				/* 形態素の文字列上での末尾位置(オフセット) */
	void *tHandle			/* 形態素辞書引き用ハンドラ(キャストして使用) */
)
{
	UNA_RETURN(ERR_VERSION_MORPH,NULL);
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
