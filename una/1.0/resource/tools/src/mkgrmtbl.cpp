//
// mkgrmtbl.cpp -
//    かかりうけ解析データ作成
//    かかりうけ解析データ作成ツール。
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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
#include <stdio.h>     /* for I/O functions */
#include <malloc.h>    /* for malloc */
#include <stdarg.h>    /* for vfprintf */
#include <assert.h>
#include <stdlib.h>
#include <math.h>      /* for log()  */
#include <string.h>    /* for memcpy */
#include <sys/stat.h>  /* for fstat */
#include "una.h"
#include "unabns.h"
#include "unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------


#define  KEY_TOKEN_CNT        1           /* キーとなるべきトークン数  */
#define  Q_COST_MAX         255           /* コストの量子化最大値      */
#define  KUB_MAX           1000           /* かかり属性とうけ属性を    */
                                          /*   ペアで格納するバッファの*/
                                          /*   最大格納数              */
#define  KAKARI_MAX         100           /* かかり属性最大値          */
#define  UKE_MAX            100           /* うけ属性最大値            */
#define  RULE_MAX           100           /* かかりうけ類型最大値      */
#define  LEN_MAX     UNA_LOCAL_BNS_SIZE   /* 文節間距離最大値          */

#define  MSG_SILENT           0           /* メッセージモードの値      */
#define  MSG_NORMAL           1           /* メッセージモードの値      */
#define  MSG_VERBOSE          2           /* メッセージモードの値      */

#define  ADD_NO               0           /* NameToNum()追加不能モード */
#define  ADD_YES              1           /* NameToNum()追加可能モード */

#define  COMMENT_SYM     0x0023           /* コメントのシンボル"#"     */

//--------------------------------------------------------------------------
// TAG: HinDataT
//
// ABSTRACT: 品詞リスト構造体
//
// NOTE: 品詞リストデータを格納するための構造体
//    
typedef struct HinDataT{     // 品詞リスト構造体 
  ushortT   morHin;               /* 形態素品詞番号   */
  ushortT   unaHin;               /* UNA品詞番号(本ツールでは未使用)   */
} HinDataT;

//--------------------------------------------------------------------------
// TAG: GramTableHeaderT
//
// ABSTRACT: ヘッダー構造体
//
// NOTE: 文法テーブルファイルのヘッダーの構造を示すファイル
//    
//
typedef struct GramTableHeaderT {  // ヘッダー構造体
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


//--------------------------------------------------------------------------
// TAG: K_U_B_tableT
//
// ABSTRACT: かかり/うけ/文節切れ目構造体
//
// NOTE: かかり属性、うけ属性、文節切れ目Y/N を格納する構造体
//    
//
typedef struct K_U_B_tableT{  // かかり/うけ/文節切れ目構造体 
  unsigned char k;      /* かかり属性 */
  unsigned char u;      /* うけ属性 */
  short         b;      /* 文節切れ目の有無 */
} K_U_B_tableT;

//--------------------------------------------------------------------------
// TAG: StrTableT
//
// ABSTRACT: 文字列構造体
//
// NOTE: 長さ情報付きで文字列を管理する構造体
//    
//
typedef struct StrTableT{      // 文字列構造体
  unaCharT str[MAX_NAME_LEN];  /* 文字列の本体 */
  int     len;                /* その長さ */
} StrTableT;


//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int InitData();
static int ReadHinList(char *hinListFile);
static int ReadCnctTbl(char *kuAttrLst);
static int ReadKuAttr(char *kuAttrLst);
static int ReadKuTbl(char *kuTblFile);
static int CompressKubTable();
static int MtxCompare( int x, int y);
static int NameToNum( unaCharT *name,int len, StrTableT *t,
          int tblSize,int addMode);
static int QuantizeCost();
static int SetLnCostTable();
static int WriteData(char *wFileName);
static int DumpData(GramTableHeaderT h);
static int Message(const char *fmt,...);
static int GetTokensWithoutComment(unaCharT *uStr, int maxTokenCnt,
          unaStrIndexT *token);

//--------------------------------------------------------------------------
// グローバル変数
//--------------------------------------------------------------------------


/*  ----------- 以下 定数 ------------ */

/* 「単語なし」のシンボル   "-" */
static unaCharT      NoWordSym[] ={0x002d,0x0000}; 
static int          NoWordSymLen = 1;

/* 「属性なし」のシンボル   "----" */
static unaCharT      NoGroupSym[] ={0x002d,0x002d,0x002d,0x002d,0x0000,0x0000};
static int          NoGroupSymLen = 4;

/* 接続属性のシンボル群  "xtqyrskbhPFYXQKOT" */ 
static unaCharT      CnctAttr[] ={0x0078,0x0074,0x0071,0x0079,0x0072,0x0073,
                                  0x006b,0x0062,0x0068,0x0050,0x0046,0x0059,
                                  0x0058,0x0051,0x004b,0x004f,0x0054,0x0000}; 


/* かかりうけ属性のシンボル群 "xfhHyYtTcCpk"  */
static unaCharT      KuAttr[] =  {0x0078,0x0066,0x0068,0x0048,0x0079,0x0059,
                                  0x0074,0x0054,0x0063,0x0043,0x0070,0x006b,
                                  0x0000,0x0000};


/*  ----------- 以下 変数 ------------ */

static char unaComment[UNA_COM_SIZE]; /* テーブルに付けるコメント */
static char unaDicVer[UNA_VER_SIZE] = UNA_GRM_VER; /* テーブルのバージョン*/

/* メッセージモードの指定 */
static int          MessageMode;


/* デフォルトの出力先 */
static FILE          *DefaultOut;

/* デフォルトのエラー出力先 */
static FILE          *DefaultErr;

/* 品詞データ */
static StrTableT     HinName[MAX_HINSHI];   /* 形態素品詞名 */
static HinDataT      HinData[MAX_HINSHI];   /* 形態素品詞番号<->ＵＮＡ品詞番号 */
static int          MorHinMax;          /* 品詞数 */

/* かかり属性データ */
static StrTableT     KName[KAKARI_MAX];  /* かかり属性名 */

/* うけ属性データ */
static StrTableT     UName[UKE_MAX];     /* うけ属性名 */

/* 品詞番号→圧縮品詞番号のテーブル */
static unsigned short MapToLocalHin[MAX_HINSHI]; /*形態素品詞番号→圧縮品詞番号 */
static int          LocalHinCount;          /* 圧縮品詞数 */

/* 圧縮品詞番号ｘ圧縮品詞番号→かかり/うけ/文節切れ目のテーブル */
static K_U_B_tableT  KubTable[MAX_HINSHI][MAX_HINSHI];

/* かかりうけ類型テーブル */
static unsigned char KuRuleTable[KAKARI_MAX][UKE_MAX];

/* かかりうけコストテーブル */
static double        KuCostTableD[KAKARI_MAX][UKE_MAX]; /* 倍精度浮動小数点 */
static unsigned char KuCostTable[KAKARI_MAX][UKE_MAX];  /* 整数値(0255) */

/* 距離コストテーブル */
static double        LnCostTableD[RULE_MAX][LEN_MAX]; /* 倍精度浮動小数点 */
static unsigned char LnCostTable[RULE_MAX][LEN_MAX]; /* 整数値(0255) */



//--------------------------------------------------------------------------
// MODULE: main
//
// ABSTRACT: メインモジュール
//
// FUNCTION: 
//     パッチ済みのＵＮＡ用かかりうけ表ソース及び接続表ソース、
//   品詞リストファイル、かかりうけ属性テーブル(パッチは無い)から、
//   実行用かかりうけテーブルを作成する。
//   使い方は、コマンド行から次の通りに入力する。
//   mkgrmtbl  [オプション] かかりうけ表ソース かかりうけ属性テーブル 
//               接続表ソース 品詞リストファイル 実行用かかりうけテーブル
//   例:
//      mkgrmtbl -a "For Ihyouki" -s kutbl.src kuattr.src unacon.src
//														unahin.lst gram.tbl
//
// RETURN:
//    ０       正常終了
//    それ以外 エラー
//
// NOTE:
//
int main(
  int argc,       /* コマンド行の引数の数(1以上) */
  char *argv[]    /* コマンド行の引数 */
)
{
  int   rv;      /* 戻り値格納用 */
  int   i;       /* 万能カウンタ */
  int   fileArgStart;  /* ソースファイル指定のアーギュメント中でのスタート */
  
  /* コマンドライン引数の処理 */
  if (argc == 1 || strcmp(argv[1],"-h") == 0){
    fprintf(stdout,"mkgrmtbl (Ver1.13)\n");
    fprintf(stdout,"Usage: mkgrmtbl [OPTION] KUTBL_SRC KUATTR_SRC CON_SRC");
    fprintf(stdout," HIN_LIST GRAM_TBL\n");
    fprintf(stdout,
    	"Ex   : mkgrmtbl -a \"For Ihyouki\" -s kutbl.src kuattr.src");
    fprintf(stdout," unacon.src unahin.lst gram.tbl\n");
    fprintf(stdout,"  OPTION\n");
    fprintf(stdout,"    -h                 Help\n");
	fprintf(stdout,"    -a ANNOTATION      The String to annotate");
	fprintf(stdout,"(Max48Byte)\n");
    fprintf(stdout,"    -s                 Silent(no message) MODE\n");
    fprintf(stdout,"    -v                 Verbose(full message) MODE\n");
    return 0;
  }
  
  MessageMode = MSG_NORMAL;
  DefaultOut = stdout;
  DefaultErr = stderr;
  for ( i = 1; argv[i][0] =='-'; ++i){   /* オプション指定の間 */
    if ( argv[i][1] =='s' ){    /* サイレントモードの指定なら */
      MessageMode = MSG_SILENT;
    }
    else if ( argv[i][1] =='v' ){    /* 冗長モードの指定なら */
      MessageMode = MSG_VERBOSE;
    }
    else if ( argv[i][1] =='a' ){    /* コメント指定 */
      strncpy(unaComment,argv[i + 1],UNA_COM_SIZE);
      ++i; /* -a オプションはさらに引数を1個取るのでiを1個進める*/
    }
    else{    /* 無視されるだけなのでカバレッジテストは行わない */
      fprintf(DefaultErr,"option %s is ignored\n", argv[i]);
    }
  }
  fileArgStart = i;
  
  
  /* 各データの初期化 */
  Message("...Initialize data matrix \n");
  rv = InitData();
  assert ( rv == UNA_OK);
  
     
  /* 品詞リストファイルを読み込む */
  Message("...Read hinshi-list file \n");
  rv = ReadHinList( argv[fileArgStart+3]);
  if ( rv < 0 ){
    fprintf(DefaultErr,"can't read hinshi list file %s \n",
          argv[fileArgStart+3]);
    return -1;
  }
  
  /* かかりうけ属性テーブルを読み込む */
  Message("...Read kakari-uke attribute file \n");
  rv = ReadKuAttr( argv[fileArgStart+1]);
  if ( rv < 0 ){
    fprintf(DefaultErr,"can't read kakari-uke attribute file %s \n",
        argv[fileArgStart+1]);
    return -1;
  }
  
  /* 接続表ソースを読み込む */
  Message("...Read connect table file \n");
  rv = ReadCnctTbl( argv[fileArgStart+2]);
  if ( rv < 0 ){
    fprintf(DefaultErr,"can't read connect matrix file %s \n",
        argv[fileArgStart+2]);
    return -1;
  }
  
  /* かかりうけ表を読み込む */
  Message("...Read kakari-uke table file \n");
  rv = ReadKuTbl( argv[fileArgStart+0]);
  if ( rv < 0 ){
    fprintf(DefaultErr,"can't read kakari-uke table file %s \n",
        argv[fileArgStart+0]);
    return -1;
  }
  
  /* 距離コストテーブルを設定する */
  Message("...Set length-cost table \n");
  rv = SetLnCostTable();
  assert ( rv == UNA_OK);

  /* Kubテーブルを圧縮する */
  Message("...Make compressed hinshi mapping table \n");
  rv = CompressKubTable();
  assert ( rv == UNA_OK);
  
  /* コストテーブルを量子化する */
  Message("...Quantize cost \n");
  rv = QuantizeCost();
  assert ( rv == UNA_OK);
  
  /* 格納 */
  Message("...Write data file \n");
  rv = WriteData( argv[fileArgStart+4]);
  if ( rv < 0 ){
    fprintf(DefaultErr,"can't write data file %s\n",argv[fileArgStart+4]);
    return 0;
  }
  
  /* 正常終了 */
  Message("...Finished\n");
  return 0;
  
}
//--------------------------------------------------------------------------
// MODULE: WriteData
//
// ABSTRACT:  データを書き込む
//
// FUNCTION:  
//    作成したデータを、実行用文法テーブルとしてバイナリファイルに書き込む。
//
// RETURN:
//    UNA_OK  正常終了
//    負の値  エラー番号
//
// NOTE:
//    なし
//
int WriteData(
  char *wFileName    /* 文法テーブルデータファイル名 */
)
{
  int             i;   /* 万能カウンタ１号 */
  int             j;   /* 万能カウンタ２号 */
  FILE             *fp; /* ファイル書き込み用 */
  GramTableHeaderT h;   /* 文法テーブルのヘッダデータ */
  unsigned char    d;   /* １バイトサイズのデータの計算用 */
  
  
  /* ファイルを開く */
  fp = fopen( wFileName, "wb");
  if (!fp){
    fprintf(DefaultErr,"can't open file %s with write mode\n", wFileName);
    return -1;
  }

  /* すぐにわかるヘッダの値は書き込んでおく */
  memset((char*)&h, 0, sizeof(GramTableHeaderT));
  h.mrHinMax = MorHinMax;
  h.kuHinMax = LocalHinCount;
  h.kLenMax  = LEN_MAX;
  for ( i = 0; KName[i].len !=0; ++i){
    ;
  }
  h.kGrpMax = i;

  for ( i = 0; UName[i].len !=0; ++i){
    ;
  }
  h.uGrpMax = i;

  /* コメント、バージョンの書き込み */
  fwrite(unaComment,sizeof(unaComment),1,fp); /* コメント */
  fwrite(unaDicVer,sizeof(unaDicVer),1,fp);   /* バージョン */

  /* とりあえずヘッダを書き込む */
  fwrite( &h, 1, sizeof(h), fp);
  
  /* 圧縮品詞 */
  h.kuHinStart = ftell(fp);
  for ( i = 0 ; i < MorHinMax; ++i){
    fwrite( &MapToLocalHin[i], 1, sizeof(MapToLocalHin[0]), fp);
  }
  
  /* かかり属性マッピングテーブル */
  h.kTblStart = ftell(fp);
  for ( i = 0; i < LocalHinCount; ++i){
    for ( j = 0; j < LocalHinCount; ++j){
      d = (ucharT)(KubTable[i][j].b<<7);
      d |= KubTable[i][j].k;
      fwrite( &d, 1, sizeof(d), fp);
    }
  }
  
  /* うけ属性マッピングテーブル */
  h.uTblStart = ftell(fp);
  for ( i = 0; i < LocalHinCount; ++i){
    for ( j = 0; j < LocalHinCount; ++j){
      d = (ucharT)(KubTable[i][j].b<<7);
      d |= KubTable[i][j].u;
      fwrite( &d, 1, sizeof(d), fp);
    }
  }
  
  /* かかりうけ類型テーブル */
  h.kuMapStart = ftell(fp);
  for ( i = 0; i < h.kGrpMax; ++i){
    for ( j = 0; j < h.uGrpMax; ++j){
      fwrite( &KuRuleTable[i][j], 1, sizeof(KuRuleTable[0][0]), fp);
    }
  }

  /* かかりうけコストテーブル */
  h.kuCostStart = ftell(fp);
  for ( i = 0; i < h.kGrpMax; ++i){
    for ( j = 0; j < h.uGrpMax; ++j){
      fwrite( &KuCostTable[i][j], 1, sizeof(KuCostTable[0][0]), fp);
    }
  }

  /* 距離コストテーブル */
  h.lnCostStart = ftell(fp);
  for ( i = 0; KuAttr[i] != 0x0000; ++i){
    for ( j = 0; j < LEN_MAX; ++j){
      fwrite( &LnCostTable[i][j], 1, sizeof(LnCostTable[0][0]), fp);
    }
  }

  /* ヘッダを再書き込み */
  fseek(fp,UNA_COM_SIZE + UNA_VER_SIZE,0); /* 0はSEEK_SET */
  fwrite(&h, 1, sizeof(h), fp);
  fclose(fp);
  
  if ( MessageMode == MSG_VERBOSE ){  /* 冗長モードなら */
    DumpData(h);
  }
  
  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: QuantizeCost
//
// ABSTRACT:  量子化
//
// FUNCTION:
//   コスト値の最大を調べ、これを用いてコストを量子化する
//
// RETURN:
//   UNA_OK  正常終了
//
// NOTE:
//
int QuantizeCost()
{
  int   i;       /* 万能カウンタ１号 */
  int   j;       /* 万能カウンタ２号 */
  double costMax; /* コストの最大値   */
  double cost;    /* 一時的にコスト値を格納する変数 */   
  
  /* 初期値 */
  costMax = 0.0;
  
  /* 類型コストテーブル内のコスト最大値をチェック */
  for ( i = 0; KName[i].len !=0; ++i){
    for ( j = 0; UName[j].len !=0; ++j){
      if ( costMax < KuCostTableD[i][j]){
        costMax = KuCostTableD[i][j];
      }
    }
  }
  
  /* 距離コストテーブル内のコスト最大値をチェック */
  for ( i = 0; KuAttr[i] != 0x0000; ++i){
    for ( j = 0; j < LEN_MAX; ++j){
      if ( costMax < LnCostTableD[i][j]){
        costMax = LnCostTableD[i][j];
      }
    }
  }
  Message(" cost value max = %lf\n", costMax);
      
  /* かかりうけコストを最大 Q_COST_MAX-1 になるように量子化 */
  for ( i = 0; KName[i].len !=0; ++i){
    for ( j = 0; UName[j].len !=0; ++j){
      if ( KuCostTableD[i][j] < 0.0){
        KuCostTable[i][j] = Q_COST_MAX;
      }
      else{
        cost = KuCostTableD[i][j]*(double)(Q_COST_MAX-1)/costMax;
        assert( cost < 255.0 && cost >= 0.0);   
        KuCostTable[i][j] = (unsigned char)cost;
      }
    }
  }
  
  /* 距離コストを最大 Q_COST_MAX-1 になるように量子化 */
  for ( i = 0; KuAttr[i]!=0x0000; ++i){
    for ( j = 0; j < LEN_MAX; ++j){
      if ( LnCostTableD[i][j]<0.0){
        LnCostTable[i][j] = Q_COST_MAX;
      }
      else{
        cost = LnCostTableD[i][j]*(double)(Q_COST_MAX-1)/costMax;
        assert( cost < 255.0 && cost >= 0.0);   
        LnCostTable[i][j] = (unsigned char)cost;
      }
    }
  }
  
  /* 正常終了 */
  return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE: SetLnCostTable
//
// ABSTRACT:  距離コスト設定
//
// FUNCTION:
//  あらかじめ用意された(プログラムで決定された)パターンに従って、
//  かかりうけルール-距離 → コストのテーブル(倍精度浮動小数点)を作成する
//
// RETURN:
//    UNA_OK  正常終了
//
// NOTE:
//
int SetLnCostTable()
{
  int i;     /* 万能カウンタ１号 */
  int j;     /* 万能カウンタ２号 */
  int hindo; /* 相対頻度値       */
  int sum;   /* 相対頻度の累計  */

  /* かかりうけ類型→距離コストのための傾きテーブル */
  /*   for  "xfhHyYtTcCpk"                          */
  int KuToA[]= { -1000, -1000, -500, -10, -100, +100,-100,-10,-1000, 
                  +100,    -10,  -10, 0};  
  
  
  /* 頻度1000からスタートして、傾きに従って相対頻度を割り振る */
  /* 各類型ごとに総計を求め、確率化してコストに変換する */
  for ( i = 0; KuAttr[i] !=0; ++i){
  
    /* 頻度を求め、総計を求める */
    hindo = 1000;
    sum   = 0;
    for ( j = 1; j < LEN_MAX; ++j){
      LnCostTableD[i][j] = (double)hindo;
      sum += hindo;
      hindo = hindo + KuToA[i];
      if ( hindo < 1 ){
        hindo = 0;
      }
    }
    
    /* 確率化してコスト変換 */
    for ( j = 1; j < LEN_MAX; ++j){
      LnCostTableD[i][j] = (double)LnCostTableD[i][j] / (double)sum;
      if ( LnCostTableD[i][j] ==0 ){
        LnCostTableD[i][j] =  -1.0;
      }
      else{    
        LnCostTableD[i][j] =  -1.0 * log(LnCostTableD[i][j]);
      }
    }
  }
  

  /* 正常終了 */  
  return UNA_OK;
  
}

//--------------------------------------------------------------------------
// MODULE: ReadKuTbl
//
// ABSTRACT:  かかりうけ表読み込み
//
// FUNCTION:
//    かかりうけ表を読み込み、KuRuleTable, KuCostTableD に格納する
//
// RETURN:
//    UNA_OK  正常終了
//    負値    エラー
//
// NOTE:
//
int ReadKuTbl(
  char *kuTblFile   /* かかりうけ表ファイル名 */
)
{
  int     i;           /* 万能カウンタ */ 
  int     j;           /* 万能カウンタ２号 */
  int     rv;          /* 戻り値格納用 */
  int     r;           /* かかりうけ類型番号 */
  int     tokenCnt;    /* トークン数 */
  FILE     *fp;         /* かかりうけ表用 */
  unaCharT lineBuf[MAX_LINE_LEN];    /* １行バッファ */
  unaStrIndexT token[MAX_TOKEN_CNT]; /* トークンバッファ */
  

  /* かかりうけ表を開く */
  fp = fopen((char *)kuTblFile,"rb");
  if (!fp) {
    fprintf(DefaultErr, "can't open kkari-uke table %s\n",kuTblFile);
    return -1;
  }
  
  /* ひたすら読んで格納 */
  for(;;){  /* 無限ループ */
  
    /* 読むべし */
    rv=una_fgetws(lineBuf, MAX_LINE_LEN, fp);
    if ( rv < 0 ){   /* これは正常な終了条件 */
      break;
    } 
    
    /* トークンに分解 */
    tokenCnt = GetTokensWithoutComment(lineBuf, MAX_TOKEN_CNT, token);
    if ( tokenCnt < 4){
      continue;
    }
    
    /* かかりうけ属性番号を取得する */
    i = NameToNum( (lineBuf+token[0].pos), token[0].len, KName,MAX_HINSHI,ADD_NO);
    j = NameToNum( (lineBuf+token[1].pos), token[1].len, UName,MAX_HINSHI,ADD_NO);

    /* かかりうけ類型を取得する */
    for ( r = 0; KuAttr[r]!=0; ++r){
      if ( lineBuf[token[2].pos] == KuAttr[r]){
        break;
      }
    }
    
    /* エラーチェック */
    if ( i < 0 || j < 0 || KuAttr[r] == 0 ){
      fprintf(DefaultErr, "kakari-uke -> rule mapping failed \n");
      fprintf(DefaultErr, "kakari = %d uke = %d rule = %d \n", i,j,r);
      return -1;
    }  
    
    /* 格納 */
    lineBuf[ token[3].pos+token[3].len] = 0x0000;
    assert( r >=0 && r <= 255);
    KuRuleTable[i][j] = (unsigned char)r;
    KuCostTableD[i][j] = una_utof( (lineBuf+token[3].pos) );
  }
  
  
  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: ReadKuAttr
//
// ABSTRACT:  かかりうけ属性表読み込み
//
// FUNCTION:
//    形態素品詞番号-形態素品詞番号 → かかりうけ属性表を読み込む
//
// RETURN:
//    UNA_OK  正常終了
//    負値    エラー
//
// NOTE:
//
int ReadKuAttr(
  char *kuAttrLst   /* かかりうけ属性テーブルファイル名 */
)
{
  int     i;           /* 万能カウンタ */ 
  int     j;           /* 万能カウンタ２号 */
  int     rv;          /* 戻り値格納用 */
  int     tokenCnt;    /* トークン数 */
  int     k;           /* かかり属性 */
  int     u;           /* うけ属性 */
  FILE     *fp;         /* かかりうけ属性ファイル用 */
  unaCharT lineBuf[MAX_LINE_LEN];      /* １行バッファ */
  unaStrIndexT token[MAX_TOKEN_CNT];   /* トークン用バッファ */
  

  /* かかりうけ属性表を開く */
  fp = fopen((char *)kuAttrLst,"rb");
  if (!fp) {
    fprintf(DefaultErr,"can't open kakari-uke attribute file %s\n",
                  kuAttrLst);
    return -1;
  }
  
  /* ひたすら読んで格納 */
  for(;;){  /* 無限ループ */
  
    /* 読むべし */
    rv=una_fgetws(lineBuf, MAX_LINE_LEN, fp);
    if ( rv < 0 ){   /* これは正常な終了条件 */
      break;
    } 
    
    /* トークンに分解 */
    tokenCnt = GetTokensWithoutComment(lineBuf, MAX_TOKEN_CNT, token);
    if ( tokenCnt < 4){
      continue;
    }
    
    /* 品詞番号を取得する */
    i = NameToNum( (lineBuf+token[0].pos),token[0].len,HinName,MAX_HINSHI,ADD_NO);
    j = NameToNum( (lineBuf+token[1].pos),token[1].len,HinName,MAX_HINSHI,ADD_NO);
    
    /* かかりうけ属性を取得する */
    k = NameToNum( (lineBuf+token[2].pos), token[2].len, 
                       KName, KAKARI_MAX, ADD_YES);
    u = NameToNum( (lineBuf+token[3].pos), token[3].len, 
                       UName, UKE_MAX, ADD_YES);
    
    /* エラーチェック */
    if ( i < 0 || j < 0 || k < 0 || u < 0){
      fprintf(DefaultErr, "kakari-uke attribute getfailed  \n");
      fprintf(DefaultErr, "mHin1 = %d mHin2 = %d k = %d u = %d\n", 
                    i,j,k,u);
      return -1;
    }  
    
    /* 格納 */
    assert( k<=255 && u<=255);
    KubTable[HinData[i].morHin][HinData[j].morHin].k = (unsigned char)k;
    KubTable[HinData[i].morHin][HinData[j].morHin].u = (unsigned char)u;
  }
  
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: ReadCnctTbl
//
// ABSTRACT:  接続表を読み込む
//
// FUNCTION:
//    UNA接続表ソースを読み込む
//
// RETURN:
//    UNA_OK  正常終了
//    負値    エラー
//
// NOTE:
//
int ReadCnctTbl(
  char *cnctLst      /* 接続表ファイル名 */
)
{
  int     i;           /* 万能カウンタ */ 
  int     j;           /* 万能カウンタ２号 */
  int     b;           /* 接続属性 */
  int     rv;          /* 戻り値格納用 */
  int     tokenCnt;    /* トークン数 */
  FILE     *fp;         /* 接続表用ファイルポインタ */
  unaCharT lineBuf[MAX_LINE_LEN];  /* １行バッファ */
  unaStrIndexT token[MAX_TOKEN_CNT];  /* トークンバッファ */
 
 
  /* 接続表を開く */
  fp = fopen((char *)cnctLst,"rb");
  if (!fp) {
    fprintf(DefaultErr, "can't open connect src %s\n",cnctLst);
    return -1;
  }
  
  /* ひたすら読んで格納 */
  for(;;){  /* 無限ループ */
  
    /* 読むべし */
    rv=una_fgetws(lineBuf, MAX_LINE_LEN, fp);
    if ( rv < 0 ){   /* これは正常な終了条件 */
      break;
    } 
    
    /* トークンに分解 */
    tokenCnt = GetTokensWithoutComment(lineBuf, MAX_TOKEN_CNT, token);
    if ( tokenCnt < 4){
      continue;
    }
    
    /* 品詞番号を取得する */
    i = NameToNum( (lineBuf+token[0].pos),token[0].len,HinName,MAX_HINSHI,ADD_NO);
    j = NameToNum( (lineBuf+token[1].pos),token[1].len,HinName,MAX_HINSHI,ADD_NO);
    
    /* 接続属性番号を取得する */
    for (b=0; CnctAttr[b] != 0; ++b){
      if ( lineBuf[token[2].pos] == CnctAttr[b]){
        break;
      }
    }
    
    
    /* エラーチェック */
    if ( i < 0 || j < 0 || CnctAttr[b] == 0){
      fprintf(DefaultErr, "connect source get failed \n");
      fprintf(DefaultErr, "mHin1 = %d mHin2 = %d cnctAttr = %d\n", i,j,b);
      return -1;
    }  
    
    /* 文節切れ目か否かだけに丸めて格納 */
    KubTable[HinData[i].morHin][HinData[j].morHin].b
                                                  = (short)(b < 11 ? 0 : 1 );
    
  }
  
  /* 残念ながら修正を行う。指定によらず、すべての句点末は
     文節切れ目になるようにした */
  for ( i= 1;i < MAX_HINSHI;++i){
    KubTable[UNA_HIN_KUTEN][i].b = 1;
  }
  
  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: ReadHinList
//
// ABSTRACT:  UNA品詞リストを読み込む
//
// FUNCTION:
//    UNA品詞リストの全レコードを読み込みメモリ(strPool)に格納する
//
// RETURN:
//    UNA_OK  正常終了
//    負の値  エラー番号
//
// NOTE:
//    なし
//
int ReadHinList(
  char *hinListFile   /* 品詞リストファイル名 */
)
{
  int         rv;          /* 戻り値格納用 */
  int         numInTable;  /* 品詞名リスト中の番号管理用 */
  int         tokenCnt;    /* トークン数 */
  FILE         *fp;         /* 品詞リストファイルポインタ */
  unaCharT     lineBuf[MAX_LINE_LEN]; /* １行バッファ */
  unaStrIndexT token[MAX_TOKEN_CNT];  /* トークンバッファ */


  /* UNA品詞リストを開く */
  fp = fopen((char *)hinListFile,"rb");
  if (!fp) {
    fprintf(DefaultErr, "can't open hinshi list file %s\n",hinListFile);
    return -1;
  }
  
  /* ひたすら読んで格納 */
  numInTable = 0;
  for(;;){  /* 無限ループ */
  
    /* 読むべし */
    rv=una_fgetws(lineBuf, MAX_LINE_LEN, fp);
    if ( rv < 0 ){   /* これは正常な終了条件 */
      break;
    } 
    
    /* トークンに分解 */
    tokenCnt = GetTokensWithoutComment(lineBuf, MAX_TOKEN_CNT, token);
    if ( tokenCnt < 3){
      continue;
    }
    
    /* 第１トークン(品詞名)をとって来て、格納 */
    rv = NameToNum( (lineBuf+token[0].pos), token[0].len,
                           HinName,MAX_HINSHI,ADD_YES);
    if ( rv < 0 ){ /* 品詞数が多すぎる場合エラーだがカバレッジテストはしない*/
      fprintf(DefaultErr,"hinshi list get failed at %d token 0\n",
        (numInTable+1));
      return rv;
    }
    numInTable = rv;
    
    /* 第２トークン(形態素品詞番号)をとって来て、格納 */
    lineBuf[token[1].pos+token[1].len] = 0x0000;
    rv = una_utol( (lineBuf+token[1].pos) );
    assert( rv>=0 && rv <= 0x0ffff);
    HinData[numInTable].morHin = (unsigned short)rv;
    
    /* 第３トークン(UNA品詞番号)をとって来て、格納 */
    lineBuf[token[2].pos+token[2].len] = 0x0000;
    rv = una_utol( (lineBuf+token[2].pos) );
    assert( rv>=0 && rv <= 0x0ffff);
    HinData[numInTable].unaHin = (unsigned short)rv;

    /* 形態素品詞番号の最大値を更新 */
    if ( HinData[numInTable].morHin > MorHinMax){
      MorHinMax = HinData[numInTable].morHin;
    }
  }
  
  /* 都合上、MorHinMax を＋１する */
  MorHinMax++;
  
  /* 後始末 */
  fclose(fp);


  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: CompressKubTable
//
// ABSTRACT:  Kubテーブルを圧縮する
//
// FUNCTION:
//    形態素品詞番号-形態素品詞番号→かかり/うけ/文節切れ目テーブルを
//    圧縮し、圧縮品詞リストを作成する。かなり正直な圧縮
//
// RETURN:
//    UNA_OK  正常終了
//
// NOTE:
//    なし
//
int CompressKubTable()
{
  int i;                 /* 万能カウンタ１号 */
  int j;                 /* 万能カウンタ２号 */
  int hinStack[MAX_HINSHI]; /* 異なり品詞を積み上げるスタック */
  int stkPos;            /* その管理用 */
  
  
  /* 各形態素品詞ごとにかかり/うけ/文節切れ目属性を比較し異なり品詞を積む */
  stkPos = 0;
  for(i=0;i<MorHinMax; ++i){
    for (j=0;j<stkPos; ++j ){
      if ( MtxCompare(i, hinStack[j]) == 0){
        break;
      }
    }
    assert(j>=0 && j <=0x0ffff);
    MapToLocalHin[i] = (unsigned short)j;
    if ( j == stkPos){
      hinStack[stkPos++] = i;
    }
  }
  
  /* 圧縮する */
  for ( i =0; i < stkPos; ++i){
    for ( j =0; j < stkPos; ++j){
      KubTable[i][j] = KubTable[hinStack[i]][hinStack[j]];
    }
  }
  
  LocalHinCount = stkPos;

  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: MtxCompare
//
// ABSTRACT:  Kubテーブル内で比較を行う
//
// FUNCTION:
//   かかり/うけ/文節切れ目の各性質について、それぞれのテーブルの
//   x行とy行、x列とy列を比較し、すべてが一致しているか否かを判定する
//
// RETURN:
//   0    xとyは、完全に一致する
//   1    xとyは異なる
//
// NOTE:
//
int MtxCompare( 
  int x,    /* 比較すべき第１の行/列の指定 */
  int y     /* 比較すべき第２の行/列の指定 */
)
{
  int i;    /* 万能カウンタ */

  /* 比較 */
  for ( i = 0; i < MorHinMax; ++i){
    if ( memcmp( &KubTable[i][x], &KubTable[i][y], sizeof(K_U_B_tableT))!=0 ||
         memcmp( &KubTable[x][i], &KubTable[y][i], sizeof(K_U_B_tableT))!=0 ){
      return 1;
    }
  }
  
  /* すべて一致した */
  return 0;
}


//--------------------------------------------------------------------------
// MODULE: InitData
//
// ABSTRACT: 初期化
//
// FUNCTION:
//    各種データを初期化する
//
// RETURN:
//    UNA_OK    正常終了
//
// NOTE:
//
int InitData()
{
  int i;  /* 万能カウンタ１号 */
  int j;  /* 万能カウンタ２号 */
  int rv; /* 戻り値格納用 */
  
  /* HinNameテーブル */
  HinName[0].len = 0;
  rv = NameToNum( NoWordSym, NoWordSymLen, HinName, MAX_HINSHI, ADD_YES);
  assert ( rv == 0 );
  
  /* KNameテーブル */
  KName[0].len = 0;
  rv = NameToNum( NoGroupSym, NoGroupSymLen, KName, KAKARI_MAX, ADD_YES);
  assert ( rv == 0 );
  
  /* UNameテーブル */
  UName[0].len = 0;
  rv = NameToNum( NoGroupSym, NoGroupSymLen, UName, UKE_MAX, ADD_YES);
  assert ( rv == 0 );

  /* HinDataテーブル */
  for ( i = 0; i < MAX_HINSHI; ++i){
    HinData[i].morHin = 0;   /* 品詞番号＝「無い」 */
    HinData[i].unaHin = 0;
  }
  
  /* Kubテーブル */
  for ( i = 0; i < MAX_HINSHI; ++i){
    for ( j = 0 ; j < MAX_HINSHI; ++j){
      KubTable[i][j].k = 0;
      KubTable[i][j].u = 0;
      KubTable[i][j].b = 0;
    }
  }
  
  /* 形態素品詞番号の最大値 */
  MorHinMax = -1;
  
  /* コストテーブル(最悪値で初期化) */
  for ( i = 0; i < KAKARI_MAX; ++i){
    for ( j = 0; j < UKE_MAX; ++j){
      KuCostTableD[i][j] = -1.0;
    }
  }
  for ( i = 0; i < RULE_MAX; ++i){
    for ( j = 0; j < LEN_MAX; ++j){
      LnCostTableD[i][j] = -1.0;
    }
  }
  
  /* 正常終了 */
  return UNA_OK;
  
}

//--------------------------------------------------------------------------
// MODULE: NameToNum
//
// ABSTRACT:  名前番号を取得
//
// FUNCTION:
//    名前文字列を受け、ユニークな番号を返す。
//    既格納であればその番号を、未格納であれば(追加可能な場合)格納し、
//    番号を与え、これを返す。
//
// RETURN:
//    正の値  名前番号
//    負の値  エラー番号
//
// NOTE:
//
int NameToNum( 
  unaCharT    *name,    /* さがすべき名前 */
  int        len,      /* その長さ */
  StrTableT   *t,       /* さがす名前が格納されているはずのテーブル */
  int        tblSize,  /* その最大サイズ */
  int        addMode   /* 追加可能/不可能のモード指定 */
  )
{
  int i;         /* 万能カウンタ */
  int numInTable;  /* テーブル中の番号 */
  
  assert(len > 0);
  
  /* リスト中にあるかどうか調べる */
  for ( i = 0; t[i].len != 0; ++i){
    if ((t[i].len==len)&&(memcmp(t[i].str,name,len*sizeof(unaCharT))==0)){
      break;
    }
  }
  
  /* とりあえず戻り値を格納 */
  numInTable = i;

  /* 必要なら新たに記憶 */
  if ( t[i].len == 0){    /* 新規の名前 */
    if ( i < tblSize && addMode == ADD_YES){ 
      memcpy( t[i].str, name, len*sizeof(unaCharT));
      t[i++].len = len;
      t[i].len = 0;
    }
    else{
      return -1;
    }
  }

  /* 正常終了 */  
  return numInTable;
  
}
//--------------------------------------------------------------------------
// MODULE: DumpData
//
// ABSTRACT: データダンプ 
//
// FUNCTION:
//   出力内容を画面表示する。
//
// RETURN:
//   UNA_OK  正常終了
//
// NOTE:
//
int DumpData(
  GramTableHeaderT h   /* 文法テーブルのヘッダデータ */
)
{
  int             i;   /* 万能カウンタ１号 */
  int             j;   /* 万能カウンタ２号 */
  
  /* ヘッダ */
  Message(" --- header infomation --- \n");
  Message("  mor hinshi max = %d \n", h.mrHinMax);
  Message("  compressed hinshi max = %d \n", h.kuHinMax);
  Message("  kakari attribute max = %d \n", h.kGrpMax);
  Message("  uke attribute max = %d \n", h.uGrpMax);
  Message("  kakari-uke length max = %d \n", h.kLenMax);
  Message("  compressed hin mapping table start offset = %d \n", 
                              h.kuHinStart);
  Message("  kakari attribute mapping table start offset = %d\n", 
                              h.kTblStart);
  Message("  uke attribute mapping table start offset = %d\n", 
                              h.uTblStart);
  Message("  kakari-uke -> rule table start offset = %d\n", 
                              h.kuMapStart);
  Message("  kakari-uke -> cost table start offset = %d\n", 
                              h.kuCostStart);
  Message("  rule-lene -> cost table start offset = %d\n", 
                              h.lnCostStart);

  /* 圧縮品詞 */
  Message("\n --- compressed hinshi table --- \n");
  for ( i = 0 ; i < MorHinMax; ++i){
    Message("%03d ", MapToLocalHin[i]);
    if ( i % 8 == 0){
      Message("\n");
    }
  }
    
  /* 文節切れ目情報 */
  Message("\n --- bunsetsu kireme table --- \n");
  for ( i = 0; i < LocalHinCount; ++i){
    Message("\n    -- compressed hin[%d]->compressed hin[x] \n", i);
    for ( j = 0; j < LocalHinCount; ++j){
      Message("%1d ", KubTable[i][j].b);
      if ( j%16 == 0){
        Message("\n");
      }
    }
  }
    
  /* かかり属性マッピングテーブル */
  Message("\n --- kakari attribute table --- \n");
  for ( i = 0; i < LocalHinCount; ++i){
    Message("\n    -- compressed hin[%d]->compressed hin[x] \n", i);
    for ( j = 0; j < LocalHinCount; ++j){
      Message("%03d ", KubTable[i][j].k);
      if ( j%16 == 0){
        Message("\n");
      }
    }
  }

  /* うけ属性マッピングテーブル */
  Message("\n --- uke attribute table --- \n");
  for ( i = 0; i < LocalHinCount; ++i){
    Message("\n    -- compressed hin[%d]->compressed hin[x] \n", i);
    for ( j = 0; j < LocalHinCount; ++j){
      Message("%03d ", KubTable[i][j].u);
      if ( j%16 == 0){
        Message("\n");
      }
    }
  }

  /* かかりうけルールテーブル */
  Message("\n --- kakari-uke rule table --- \n");
  for ( i = 0; i < h.kGrpMax; ++i){
    for ( j = 0; j < h.uGrpMax; ++j){
      Message("%03d ", KuRuleTable[i][j]);
    }
    Message("\n");
  }

  /* かかりうけコストテーブル */
  Message("\n --- kakari-uke cost table --- \n");
  for ( i = 0; i < h.kGrpMax; ++i){
    for ( j = 0; j < h.uGrpMax; ++j){
      Message("%03d ", KuCostTable[i][j]);
    }
    Message("\n");
  }

  /* 距離コストテーブル */
  Message("\n --- kuRule-length cost table --- \n");
  for ( i = 0; KuAttr[i] != 0x0000; ++i){
    for ( j = 0; j < LEN_MAX; ++j){
      Message("%03d ", LnCostTable[i][j]);
      if ( j%16 == 0){
        Message("\n");
      }
    }
    Message("\n");
  }
  
  /* 正常終了 */
  return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE: Message
//
// ABSTRACT: メッセージ出力 
//
// FUNCTION:
//   MessageModeがMSG_SILENT でない場合は、デフォルト出力先に
//   指定内容を出力する。
//
// RETURN:
//   UNA_OK  正常終了
//
// NOTE:
//
int Message(
  const char *fmt, /* フォーマット文字列 */
  ...              /* 可変長引き数 */
  )
{
  va_list args;
  
  /* サイレントモードならなにもしない */
  if ( MessageMode == MSG_SILENT ){
    return UNA_OK;
  }
  
  /* 単にvMessageを呼ぶ */
  va_start(args,fmt);
  vfprintf(DefaultOut, fmt,args);
  va_end(args);
 
  return UNA_OK;
}
  
//--------------------------------------------------------------------------
// MODULE: Message
//
// ABSTRACT: UNICODE文字列から全トークンを得る
//
// FUNCTION:
//   コメント記号(#)以降を除外して、
//   UNICODE文字列から全トークンを得る。pos には *uStr からのオフセットが
//   len には、当該トークンの長さ(文字数)がセットされる。
//
// RETURN:
//   得られたトークン数
//
// NOTE:
//   una_getTokens()の上に一枚、皮をかぶせただけの関数
//   ただし、ユニコード文字列の中身が書き換えられる。
//
int GetTokensWithoutComment(
  unaCharT     *uStr,       /* ヌルターミネート UNICODE 文字列 */
  int         maxTokenCnt, /* 最大トークン数(token配列の大きさ) */
  unaStrIndexT *token	    /* トークン配列 */
  )
{
  int i;
  int tokenCnt;
  
  /* あらかじめ # 記号を検出し、NULLターミネートしておく */
  for ( i = 0 ; uStr[i] !=0; ++i){
    if ( uStr[i] == COMMENT_SYM){
      uStr[i] = 0;
      break;
    } 
  }
    
  /* トークンに分解 */
  tokenCnt = una_getTokens(uStr, maxTokenCnt, token);

  return tokenCnt;
}
//--------------------------------------------------------------------------
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
