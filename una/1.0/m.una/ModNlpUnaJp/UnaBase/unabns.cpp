//
// unabns.cpp -
//    かかりうけ解析
//    かかりうけ解析処理のメインのモジュール。
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
// Reading the necessary header
//--------------------------------------------------------------------------
#include "UnaBase/una.h"          /* header of the whole una */
#include "UnaBase/unabns.h"       /* catching receiving analytical module itself */
#include "UnaBase/unamorph.h"     /* morpheme is handled, */
#include <string.h>       /* strcmp */
#include <assert.h>       /* for debugging */

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

/* 量子化最大値 */
#define CANT_REL             255  /* 文法テーブル上のかかりうけ付加の表現値 */
#define COST_OF_CANT_REL (256*16) /* かかりうけ不可に実際に振られるコスト */

/* マクロ関数 */
#define IsBsEnd(d) ((d)&0x80)    /* ２品詞間で文節切れ目有無の判定 */
#define KuCode(d)  ((d)&0x7f)    /* 連続した２品詞でかかりうけ属性を取得 */

//--------------------------------------------------------------------------
// TAG: GramTableHeaderT
//
// ABSTRACT: ヘッダー構造体
//
// NOTE: 文法テーブルファイルのヘッダーの構造を示すファイル
//       文法テーブル作成ツールは、これ一致するようにデータを
//       作成すること
//   
typedef struct GramTableHeaderT {  // 文法テーブルヘッダー構造体
  int mrHinMax;        /* 形態素解析用品詞の最大番号 */
  int kuHinMax;        /* 圧縮品詞の最大番号 */
  int kGrpMax;         /* かかり属性値の最大番号 */
  int uGrpMax;         /* うけ属性値の最大番号 */
  int kLenMax;         /* かかりうけ可能な距離の最大 */
  int kuHinStart;      /* 圧縮品詞マッピングテーブルの開始位置 */
  int kTblStart;       /* かかり属性マッピングテーブルの開始位置 */
  int uTblStart;       /* うけ属性マッピングテーブルの開始位置 */
  int kuMapStart;      /* かかりうけ類型テーブルの開始位置 */
  int kuCostStart;     /* かかりうけコストテーブルの開始位置 */
  int lnCostStart;     /* 距離コストテーブルの開始位置 */
} GramTableHeaderT;


/* 内部的な関数の宣言 */
static int KuHin( unaBnsHandleT *bh, ushortT morHin);
static int InitPCost( unaBnsHandleT *bh, unaBnsT *bbuf, int bnsLen);
static int SearchPath( unaBnsHandleT *bh, int bnsLen);
static int SetKuInfo( unaBnsHandleT *bh, unaBnsT *bbuf, int bnsLen);

//--------------------------------------------------------------------------
// MODULE:    unaBns_init
//
// ABSTRACT:  かかりうけ関連の初期化
//
// FUNCTION:  かかりうけ解析モジュールのハンドラを設定する
//
// RETURN:
//    UNA_OK   正常終了
//    負の値   エラー
//
// NOTE:
//
int unaBns_init(
  unaBnsHandleT *bh,    /* ハンドラ */
  const char *gramTblImage    /* 文法テーブルのイメージ */
  )
{
  GramTableHeaderT *gth;   /* イメージを受けて型変換するためのポインタ */
  const char *imgPtr;      /* テーブルを格納したメモリへのポインタ */

  if ( gramTblImage == (const char*)NULL){ /* NULLによる初期化 */
    bh->mrHinMax = 0;
    bh->hinMax   = 0;
    bh->kzMax    = 0;
    bh->uzMax    = 0;
    bh->lnMax    = 0;
    bh->kuHin    = (const short*)NULL;
    bh->kTbl     = (const ucharT*)NULL;
    bh->uTbl     = (const ucharT*)NULL;
    bh->kuMap    = (const ucharT*)NULL;
    bh->kuCost   = (const ucharT*)NULL;
    bh->lnCost   = (const ucharT*)NULL;
    return UNA_OK; /* NULLによる初期化はOKとする */
  }

  imgPtr = gramTblImage + UNA_COM_SIZE;
  if (strcmp(imgPtr,UNA_GRM_VER)!=0){  /* バージョンが違う */
    return UNA_ERR_VERSION_GRM;
  }
  imgPtr += UNA_VER_SIZE;

  /* 文法テーブル解釈用の型に変換 */
  gth = (GramTableHeaderT*)(imgPtr);
 
  /* 値を設定する */
  bh->mrHinMax = gth->mrHinMax;
  bh->hinMax   = gth->kuHinMax;
  bh->kzMax    = gth->kGrpMax;
  bh->uzMax    = gth->uGrpMax;
  bh->lnMax    = gth->kLenMax;
  bh->kuHin    = (const short*)(gramTblImage+(gth->kuHinStart));
  bh->kTbl     = (const ucharT*)(gramTblImage+(gth->kTblStart));
  bh->uTbl     = (const ucharT*)(gramTblImage+(gth->uTblStart));
  bh->kuMap    = (const ucharT*)(gramTblImage+(gth->kuMapStart));
  bh->kuCost   = (const ucharT*)(gramTblImage+(gth->kuCostStart));
  bh->lnCost   = (const ucharT*)(gramTblImage+(gth->lnCostStart));

  /* 正常終了 */
  return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:    unaBns_term
//
// ABSTRACT:  かかりうけ関連の終了処理
//
// FUNCTION:  現在は、やってもやらなくても同じ
//
// RETURN:
//    UNA_OK  正常終了
//
// NOTE:
//
int unaBns_term(
  unaBnsHandleT *bh   /* ハンドラ */
  )
{

  /* 値をリセットする */
  bh->mrHinMax = 0;
  bh->hinMax   = 0;
  bh->kzMax    = 0;
  bh->uzMax    = 0;
  bh->lnMax    = 0;
  bh->kuHin    = (const short*)NULL;
  bh->kTbl     = (const ucharT*)NULL;
  bh->uTbl     = (const ucharT*)NULL;
  bh->kuMap    = (const ucharT*)NULL;
  bh->kuCost   = (const ucharT*)NULL;
  bh->lnCost   = (const ucharT*)NULL;

  /* 正常終了 */
  return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:    unaBns_analyze
//
// ABSTRACT:    かかりうけ解析をする
//
// FUNCTION:
//    形態素解析結果をかかりうけ解析する。
//    文節切りは終わっているものとし、かかりうけ情報のみを付与する。
//
// RETURN:
//    UNA_OK      正常終了
//    負値        エラー番号(unaKApiでは発生しない)
//
// NOTE:
//
int unaBns_analyze(
  unaBnsHandleT *bh,            /* ハンドラ */
  unaBnsT       *bbuf,          /* 文節生成結果が格納された配列 */
  int          bnsLen,          /* 処理する要素数 */
  int          *work1,          /* ワークバッファ１ */
  ucharT        *work2,         /* ワークバッファ２ */
  int          *work3           /* ワークバッファ３ */
  )
{
  int rv;         /* 戻り値を受ける変数 */

  /* NULLデータによるかかりうけ解析 */
  if ( bh->mrHinMax == 0){
    int i;
    for (i=0; i < bnsLen-1; ++i){
      bbuf[i].kuRel  = UNA_REL_NOTHING;
      bbuf[i].target = i+1;
    }
    bbuf[i].kuRel  = UNA_REL_NOTHING;
    bbuf[i].target = i;
    return UNA_OK;
  }

  /* ワークバッファの設定 */
  bh->pCost   = work1;
  bh->pPtrn   = work2;
  bh->lStack  = work3;

  /* こうなっているはず */
  assert(bnsLen<=UNA_LOCAL_BNS_SIZE);

  /* テーブル初期値の設定 */
  rv = InitPCost( bh, bbuf, bnsLen);
  if ( rv < 0 ){   /* unaKApiでは負になることはない */
    return rv;
  }
  
  /* 動的計画法により最適なかかりうけの組み合わせを求める */
  rv = SearchPath(bh, bnsLen);
  assert( rv == UNA_OK);
  
  /* 展開して文節に格納する */
  rv = SetKuInfo(bh, bbuf, bnsLen);
  assert ( rv == UNA_OK);

  /* 正常終了 */
  return UNA_OK;

}
//--------------------------------------------------------------------------
// MODULE:    unaBns_gen
//
// ABSTRACT:  文節列を作る
//
// FUNCTION:
//    形態素解析結果をから文節列を生成する
//    以下の条件で終了し、結果(戻り値&processedMorphNum)を返す
//      (1) 句点まで文節生成した
//      (2) 生成結果の文節数がUNA_LOCAL_BNS_SIZEになった
//      (3) すべての入力形態素を文節生成した
//      (4) なんらかのエラーが生じた
//
// RETURN:
//    0 以上      文節配列中の末尾位置(全格納数)
//    負値        エラー番号
//
// NOTE: morphLen および startPos の0以下は0とみなす
//
int unaBns_gen(
  unaBnsHandleT *bh,            /* ハンドラ */
  unaBnsT   *bbuf,              /* 文節生成結果が格納される配列 */
  int      startPos,            /* 文節生成結果格納の開始位置 */
  int      bbufLen,             /* 文節生成結果格納配列の要素数 */
  unaMorphT *mbuf,              /* 入力形態素列が格納されたバッファ */
  int      morphLen,            /* 入力形態素列の長さ */
  int      *processedMorphNum,  /* 処理した形態素数 */
  int          *work1,          /* ワークバッファ１ */
  ucharT        *work2,         /* ワークバッファ２ */
  int          *work3           /* ワークバッファ３ */
  )
{
  int    readMorph;  /* データを見た形態素数 */
  int    writeBns;   /* 現在生成中の文節番号 */
  int    h1;         /* 文節境界判定時の前形態素の圧縮品詞番号 */  
  int    h2;         /* 同上、後形態素の圧縮品詞番号 */
  
  /* NULLデータによる文節生成 */
  if ( bh->mrHinMax == 0){
    int i;
    int epos;
    if ( startPos<=0){
      writeBns = 0;
    }
    else{
      writeBns = startPos;
    }
    epos = writeBns + morphLen;
    if ( epos >= UNA_LOCAL_BNS_SIZE){
      epos = UNA_LOCAL_BNS_SIZE;
    }
    if ( epos >= bbufLen){
      epos = bbufLen;
    }
    for ( i = 0; i<(epos-writeBns);++i){
      bbuf[writeBns+i].start = mbuf+i;
      bbuf[writeBns+i].len = 1;
    }
    *processedMorphNum = i;
    return epos;
  }

  /* ワークバッファの設定 */
  bh->pCost   = work1;
  bh->pPtrn   = work2;
  bh->lStack  = work3;

  /* 自明の戻り値 */
  if ( bbufLen <= startPos){
    *processedMorphNum = 0;
    return bbufLen;
  }
  else if ( morphLen <=0){ /* unaKapiではこの指定は無いのでCoverageは通らない*/
    *processedMorphNum = 0;
    return startPos;
  }
  
  /* 初期化 */
  if ( startPos <=0){         /* もっとも最初から */
    writeBns = 0;
    bbuf[writeBns].start = mbuf;
    h2 = UNA_HIN_NOTHING;
  }
  else{                       /* 途中から */
    writeBns = startPos -1;   /* 前回の文節生成の末尾に"そのまま"続ける */

    /* 句点の後には文節生成を続けない */
    if ( bbuf[writeBns].start[ bbuf[writeBns].len-1].hinshi == UNA_HIN_KUTEN){
      *processedMorphNum = 0;
      return startPos;
    }

    h2 = KuHin( bh,bbuf[writeBns].start[ bbuf[writeBns].len-1].hinshi);
    assert(h2>=0);  /* h2 はすでに１度チェック済みなので */
  }
  
  /* 文節切りを行う */  
  for (readMorph=0; readMorph<morphLen; ++readMorph){

    /* 品詞セット */
    h1 = h2;
    h2 = KuHin( bh, mbuf[readMorph].hinshi);
    if ( h2 < 0){    /* unaKApiではCoverageは通らないが他APIのために残す */
      return h2;
    }
    
    /* 句点に遭遇 */
    if ( mbuf[readMorph].hinshi == UNA_HIN_KUTEN){
      readMorph++;
      break;
    }
    
    /* 文節切れ目の処理 */
    else if ( IsBsEnd( (bh->kTbl)[ h1 * (bh->hinMax) + h2]) ){
      bbuf[writeBns].len = (int)((mbuf+readMorph) - bbuf[writeBns].start);

      if ( writeBns+1>=UNA_LOCAL_BNS_SIZE){ /* 2000/06の辞書では発生しない
                                              ため Coverageは通らない。
                                              発生条件は
                                              (1)１単語で１文節になる語がある
                                              (2)その語が結果として選択される
                                              (3)しかし収束する語ではない
                                              この(2)を満足するのは記号類だが
                                              記号類は(3)を満たさない      */
        break;
      }
      if ( writeBns+1>=bbufLen){ /*推奨に従えば発生しないのでCoverage通らない*/
        return UNA_ERR_OVERFLOW_BNS;
      }
      writeBns++;
      bbuf[writeBns].start = (mbuf+readMorph);
    }
  }
  
  /* 後始末 */
  bbuf[writeBns].len  = (int)((mbuf+readMorph) - bbuf[writeBns].start);
  writeBns++;
  *processedMorphNum = readMorph;
  

  /* 正常終了 */
  return writeBns;
}
//--------------------------------------------------------------------------
// MODULE:    InitPCost
//
// ABSTRACT:  かかりうけ用元テーブル作成
//
// FUNCTION:  かかりうけ解析に用いるpCostテーブルに、最初の値を設定する。
//
// RETURN:
//    UNA_OK    正常終了
//    負値      エラー番号
//
// NOTE:
//
static int InitPCost( 
  unaBnsHandleT *bh,   /* ハンドラ */
  unaBnsT *bbuf,       /* 文節バッファ */
  int    bnsLen        /* 文節数 */
  )
{
  int kBns;     /* かかり元の文節番号 */
  int uBns;     /* かかり先の文節番号 */
  int h1;       /* かかり元の文節の末尾から１個前の単語の圧縮品詞番号 */
  int h2;       /* かかり元の文節の末尾の単語の圧縮品詞番号 */
  int h3;       /* かかり先の文節の先頭の単語の圧縮品詞番号 */
  int h4;       /* かかり先の文節の先頭から１個後の単語の圧縮品詞番号 */
  int kGrp;     /* かかり元文節のかかり属性番号 */
  int uGrp;     /* かかり先文節のうけ属性番号 */
  int kuRel;    /* かかりうけルール番号 */
  int kc;       /* かかりうけコスト */
  int lc;       /* 距離コスト */
  
  /* ひたすらテーブルの値を設定する処理 */
  for ( kBns = 0; kBns< bnsLen; ++kBns){

    /* かかり側の品詞を設定 */
    h1 = UNA_HIN_NOTHING;
    if ( bbuf[kBns].len>1){
      h1 = KuHin( bh,bbuf[kBns].start[ bbuf[kBns].len-2].hinshi );
    }
    h2 = KuHin( bh,bbuf[kBns].start[ bbuf[kBns].len-1].hinshi );

    /* エラーチェック(unaKApiではCoverageは通らないが他APIのために残す) */
    if ( h1 < 0 || h2 < 0 ){
      return UNA_ERR_MORHINNUM_BNS;
    }

    /* かかり属性を求める */
    kGrp = KuCode( (bh->kTbl)[h1 * (bh->hinMax) + h2]);

    for ( uBns = kBns+1; uBns < bnsLen; ++uBns ){
      
      /* 受け側の品詞を設定 */
      h3 = KuHin( bh, bbuf[uBns].start[ 0].hinshi );
      h4 = UNA_HIN_NOTHING;
      if ( bbuf[uBns].len>1){
        h4 = KuHin( bh,bbuf[uBns].start[ 1].hinshi );
      }
      
      /* エラーチェック(unaKApiではCoverageは通らないが他APIのために残す) */
      if ( h3 < 0 || h4 < 0 ){
        return UNA_ERR_MORHINNUM_BNS;
      }
      
      /* うけ属性を求める */
      uGrp = KuCode( (bh->uTbl)[h3*(bh->hinMax)+h4]);
      
      /* かかりうけルール(類型)を求める */
      kuRel = (bh->kuMap)[kGrp*(bh->uzMax)+uGrp];
      
      /* コストを設定する */
      assert( (uBns-kBns)<UNA_LOCAL_BNS_SIZE);
      kc = (bh->kuCost)[kGrp*(bh->uzMax)+uGrp];
      lc = (bh->lnCost)[kuRel*(bh->lnMax)+(uBns-kBns)];
      if (kc==CANT_REL){
        kc = COST_OF_CANT_REL;
      }
      if (lc==CANT_REL){
        lc = COST_OF_CANT_REL;
      }
      
      (bh->pCost)[(kBns*UNA_LOCAL_BNS_SIZE + uBns)] = kc + lc;

      /* 分割パターンを設定する */
      (bh->pPtrn)[(kBns*UNA_LOCAL_BNS_SIZE + uBns)] = (ucharT)(uBns-kBns);
    }
  }
  
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE:  SearchPath
//
// ABSTRACT: かかりうけ解析
//
// FUNCTION: 最小コストなかかりうけの組み合わせを求める
//
// RETURN:
//    UNA_OK    正常終了
//
// NOTE:
//
static int SearchPath(
  unaBnsHandleT *bh,    /* ハンドラ */
  int bnsLen            /* 文節数 */
  )
{
  int bLen;    /* 文節間距離 */
  int sBns;    /* 内部分割パターン計算範囲の開始文節 */
  int eBns;    /* 内部分割パターン計算範囲の終了文節 */
  int mPtrn;   /* 最小コストを実現する内部分割パターン */
  int mCost;   /* 最小コスト */
  int tPtrn;   /* 内部分割パターンの一時値 */
  int tCost;   /* コストの一時値 */
  
  /* 探索 */
  for ( bLen = 1; bLen < bnsLen; ++bLen){
    for ( sBns = 0; sBns < bnsLen -bLen ; ++sBns ){
      
      eBns = sBns + bLen;
      
      /* sBns-eBns間の最小コストの内部分割パターンを求める */
      mPtrn = bLen; 
      mCost = (bh->pCost)[(sBns*UNA_LOCAL_BNS_SIZE + eBns)];
      for ( tPtrn = 1; tPtrn < bLen; ++ tPtrn){ /* すべての分割パターン */
        tCost = (bh->pCost)[(sBns*UNA_LOCAL_BNS_SIZE + (sBns+tPtrn))] +
                (bh->pCost)[((sBns+tPtrn)*UNA_LOCAL_BNS_SIZE +eBns)];
        if ( tCost < mCost ){
          mCost = tCost;
          mPtrn = tPtrn;
        }
      }
      (bh->pCost)[(sBns*UNA_LOCAL_BNS_SIZE+eBns)] = mCost;
      (bh->pPtrn)[(sBns*UNA_LOCAL_BNS_SIZE+eBns)] = (ucharT)mPtrn;

      /* 次のkakarLenの準備 */
      if ( sBns>0){
        (bh->pCost)[((sBns-1)*UNA_LOCAL_BNS_SIZE + eBns)] += mCost;
      }
    }
  }
  
  /* 正常終了 */ 
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE:  SetKuInfo
//
// ABSTRACT: かかりうけ情報の格納
//
// FUNCTION: 文節データにかかりうけ情報を付与する
//
// RETURN:
//    UNA_OK    正常終了
//
// NOTE:
//
static int SetKuInfo(
  unaBnsHandleT *bh,     /* ハンドラ */
  unaBnsT *bbuf,         /* 文節データ */ 
  int    bnsLen          /* 文節数 */
  )
{
  int kBns;     /* かかり元文節番号 */
  int uBns;     /* かかり先文節番号 */
  int kLen;     /* かかりうけ文節間距離 */
  int stackPos; /* 現在のスタック位置 */
  int h1;       /* かかり元文節の末尾から１個前の単語の圧縮品詞番号 */
  int h2;       /* かかり元文節の末尾単語の圧縮品詞番号 */
  int h3;       /* かかり先文節の先頭単語の圧縮品詞番号 */
  int h4;       /* かかり先文節の先頭から１個後の単語の圧縮品詞番号 */
  int kGrp;     /* かかり元文節のかかり属性 */
  int uGrp;     /* かかり先文節のうけ属性 */
  
  /* 展開して文節に格納する */
  kLen = bnsLen -1;
  (bh->lStack)[0] = 0;
  stackPos = 1;
  for ( kBns = 0; kBns < bnsLen-1; ++kBns){
    
    /* 注目文節のかかり先を設定 */
    uBns = kBns + (bh->pPtrn)[(kBns*UNA_LOCAL_BNS_SIZE +kBns+kLen)];
    bbuf[kBns].target = uBns;

    /* かかり側の品詞を設定 */
    h1 = UNA_HIN_NOTHING;
    if ( bbuf[kBns].len>1){
      h1 = KuHin( bh,bbuf[kBns].start[ bbuf[kBns].len-2].hinshi );
    }
    h2 = KuHin( bh,bbuf[kBns].start[ bbuf[kBns].len-1].hinshi );

    /* すでに１度チェック済みなので */
    assert( (h1>=0 && h2>=0));

    /* かかり属性を求める */
    kGrp = KuCode( (bh->kTbl)[h1 * (bh->hinMax) + h2]);

    /* 受け側の品詞を設定 */
    h3 = KuHin( bh, bbuf[uBns].start[ 0].hinshi );
    h4 = UNA_HIN_NOTHING;
    if ( bbuf[uBns].len>1){
      h4 = KuHin( bh,bbuf[uBns].start[ 1].hinshi );
    }

    /* すでに１度チェック済みなので */
    assert( (h3>=0 && h4>=0));

    /* うけ属性を求める */
    uGrp = KuCode( (bh->uTbl)[h3 * (bh->hinMax) + h4]);

    /* かかりうけ関係を設定 */
    bbuf[kBns].kuRel  = (bh->kuMap)[kGrp*(bh->uzMax)+uGrp];
    
    /* 残り距離をスタックに格納 */
    if ( kLen - (bh->pPtrn)[(kBns*UNA_LOCAL_BNS_SIZE + kBns+kLen)]>0){
      (bh->lStack)[stackPos++] = kLen - 
                              (bh->pPtrn)[(kBns*UNA_LOCAL_BNS_SIZE +kBns+kLen)];
    }
    
    /* 次の kLen を設定 */
    kLen = (bh->pPtrn)[(kBns*UNA_LOCAL_BNS_SIZE +kBns+kLen)] -1;
    if ( kLen <= 0 ){
      stackPos--;
      assert (stackPos>=0);   /* 文末以外のすべての文節には、必ず非交差で
                                 かかることのできるかかり先文節候補が存在 */
      kLen = (bh->lStack)[stackPos];
    }
  }

  bbuf[kBns].kuRel  = UNA_REL_NOTHING;
  bbuf[kBns].target = kBns;

  return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:    KuHin
//
// ABSTRACT:  品詞番号取得
//
// FUNCTION:  かかりうけ専用に圧縮した品詞番号を形態素品詞番号から取得する
//
// RETURN:
//    ０以上   かかりうけ用品詞番号
//    負値     エラー番号
//
// NOTE:
//
static int KuHin( 
  unaBnsHandleT *bh,     /* ハンドラ */
  ushortT       morHin   /* 形態素品詞番号 */
  )
{
  /* エラーチェック(unaKApiではCoverageは通らないが他APIのために残す) */
  if ( morHin>=(bh->mrHinMax)){
    return UNA_ERR_MORHINNUM_BNS;
  }
  
  /* 正常終了 */
  return (bh->kuHin)[morHin];
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
