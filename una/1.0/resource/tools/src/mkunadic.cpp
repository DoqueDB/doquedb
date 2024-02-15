//
// mkunadic.cpp -
//      UNA V3の実行辞書作成モジュール
//      UNA V3の実行辞書を作成するモジュール
// 
// Copyright (c) 2000-2002, 2023 Ricoh Company, Ltd.
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
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<locale.h>		/* 評価用 */
#include	<sys/stat.h>	/* stat */
#include	<assert.h>		/* デバッグ用 */
#include	"una.h"			/* UNAグローバルなヘッダファイル */
#include	"unamorph.h"	/* 形態素解析メインモジュール */
#include	"unamdtri.h"	/* 辞書引きモジュール */
#include	"unaapinf.h"	/* アプリ情報取得モジュール */
#include	"unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define	LOCAL_MAX_WORDS_POOL (MAX_REC_NO * (AVE_WORD_LEN + 1)) /* 文字数 */
#define NO					0 /* フラグ値					           */
#define YES					1 /* フラグ値					           */
#define EOK			   0x0000 /* End Of Key                            */
#define MAX_ELEM_CNT UNA_SUB_MORPH_CNT_MAX + 1
							  /* 1つの形態素あたりの最大下位構造数 + 1 */
#define Q_COST_MAX		  255 /* コストの量子化最大値                  */

/* 辞書形式 (dicVersionの値) */
#define	V124              124 /* WRD V1.24+の辞書形式 */
#define	V125              125 /* WRD V1.25+の辞書形式 */
//--------------------------------------------------------------------------
// TAG:	  hinLstIdxT
//
// ABSTRACT:	  UNA品詞リスト型
//
// NOTE:
//	  UNA品詞リストの1レコードを表わす。但し品詞名(文字列)は別の文字列プール
//	  に格納され、そこへのポインタと、長さという形で管理している。
//
typedef struct hinLstIdxT{		// UNA品詞リスト型
	unaCharT *ptr;				/* 文字列(品詞名)へのポインタ */
	uintT	 len;				/* 文字列(品詞名)の長さ       */
	ushortT	 morHin;			/* 形態素品詞番号   */
	unaHinT	 unaHin;			/* UNA品詞番号      */
}hinLstIdxT;

//--------------------------------------------------------------------------
// TAG:	  wrdsPoolIdxT
//
// ABSTRACT:	  文字列プール型
//
// NOTE:
//	  UNA辞書ソースの表記が格納された文字列プールへのポインタと、長さの
//	  情報を管理する構造体である。
//
typedef struct wrdsPoolIdxT{	// 文字列プール型
	unaCharT *wordPtr;			/* unaWrdsPool へのポインタ */
	ucharT	 wordLen;			/* 文字数 */
} wrdsPoolIdxT;

//--------------------------------------------------------------------------
// TAG:	  cvtMorIDTblT
//
// ABSTRACT:	  形態素IDレコード番号対応型
//
// NOTE:
//	  形態素IDとレコード番号(行番号-1)の対応づけをする構造体である。
//
typedef struct cvtMorIDTblT{	// 形態素IDレコード番号対応型
	int morphID;				/* 形態素ID */
	uintT recNo;				/* レコード番号 */
} cvtMorIDTblT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int GetOptions(int argc,char *argv[],char *comment,
	   int *compactMode,int *verboseMode,int *tellMode, int *processedArgc);
static void OutputUsage(void);
static int OptionA(int argc,char *argv[],char *comment,
	   int *processedArgc);
static int GetCostMax(double *costMax,char *costMaxFileName);
static int ReadHinLst(char *hinLst);
static int ReadDicSrc(char *dicSrc,int hinLstCnt);
static int ChkLineLen(int inStrLen);
static int ChkOverflowRecNo(int i);
static int ChkOverflowAppPool(int tokenCnt,int inStrLen,
	   unaStrIndexT *token,char *appInfoPtr);
static int ProcMorID(unaCharT *iStr,unaStrIndexT *strIdx,int recNo);
static int SeprMorID(unaCharT *morIDStr,int morIDStrLen,int maxElemCnt,
	   int *morID);
static int HyokiSet(unaCharT *wordPtr,unaCharT *const hyokiPtr,
	   int hyokiLen,int *realHyokiLen);
static void ReplaceSymbols(unaCharT *pos, int len);
static int GetMorHin(unaCharT *iStr,unaStrIndexT *strIdx,int hinLstCnt,
	   int recNo);
static void PrintUcsToUtf8(unaCharT *ptr,uintT len);
#if 0
static void PrintUcsToHex(unaCharT *ptr,uintT len);
#endif
static int GetCost(unaCharT *iStr,unaStrIndexT *strIdx);
static int AppInfoSet(char	*appInfoInfoPtr,unaCharT *const infoPtr,
	   int infoLen,int *realInfoLen);
static int GetPadding(int len);
static int ProcSubMor(int recCnt);
static int Revuz(uintT *r,const uintT startRec,const uintT endRec,
	   const uintT targetPos);
static int FindBase(uintT *r,const uintT startRec,const uintT endRec,
	   const uintT targetPos);
static int WriteDict(char *morDic,char *appDic);
static int WriteDictNull(char *morDic,char *appDic);
static int LineComp(hinLstIdxT *w1,hinLstIdxT *w2);
static int CodeComp(int *c1,int *c2);
static int MorIDComp(cvtMorIDTblT *w1,cvtMorIDTblT *w2);
static int TestRevuz(const uintT startR);

//--------------------------------------------------------------------------
// グローバル変数
//--------------------------------------------------------------------------

/* ヘッダー用 */
static char unaComment[UNA_COM_SIZE];		/* 辞書に付けるコメント */
static char unaDicVer[UNA_VER_SIZE] = UNA_DIC_VER; /* 辞書のバージョン */
static char unaAppVer[UNA_VER_SIZE] = UNA_APP_VER; /* アプリのバージョン */
static uintT unaRecNo = 0;	  /* レコード番号。所謂 ID(0起算) */
static uintT unaLstNo = 0;	  /* 下位構造リスト番号。0起算 */
static uintT unaMaxBase = 0; /* 最大のbase値がセットされる(daCount計算用) */
static uintT unaMaxOccupy = 0;	/* 最大の遷移先番号がセットされる(同上) */

/* 形態素辞書用配列 */
static unaCompactMorDataT *unaCMorData = NULL;
static unaCompactMorDataT *unaCMorData2 = NULL;
				/* V1.24情報部(同形語数 品詞 長さ コスト 下位形態素リスト番号) */
static unaMorDataT *unaMorData = NULL;
				/* V1.25情報部(同形語数 品詞 長さ コスト 下位形態素リスト番号) */
static uintT	   unaSubLst[MAX_LST_NO];
				/* 下位構造リスト部。いくつかのブロックに分かれていると
				   考えた方がわかりやすい。1つのブロックが1つの形態素の
				   下位構造を表わし、下位構造をもつ形態素からブロック先頭
				   にポインタがはられている。ブロック先頭の要素は下位構造
				   の数を示し、その数分の形態素へのポインタ情報
				   (情報部のレコード番号)が次の要素より書かれている */
static uintT	   unaBase[MAX_DA];		   /* 索引部(RevuzのTrie) */
static unaCharT	   unaLabel[MAX_DA];	   /* 索引部(RevuzのTrie) */
static ucharT	   unaOccupy[MAX_DA];	   /* 索引部(RevuzのTrie) */

/* ラベル/baseの処理値のデフォルト */
const unaCharT MaxValueOfCode = 0xffff;
const uintT   ClearValueOfBase  = 0xffffffff;

/* ラベル/baseの辞書中のデフォルト(main()で明示的に値を設定する) */
unaCharT MaxValueOfLabelInDic = 0xffff;		/* 16ビット */
uintT   MaxValueOfBaseInDic  = 0xffffffff;	/* 32ビット */

/* アプリ辞書用配列 */
static uintT unaAppOffset[MAX_REC_NO]; /* 索引部(情報部のオフセット) */
static char	  unaAppPool[MAX_APP_POOL]; /* 情報部(unaAppInfoT型のデータ) */

/* 表記が書かれてる形態素辞書作成用の配列。これを元にRevuzのTrieを作る */
static wrdsPoolIdxT unaWrdsPoolIdx[MAX_REC_NO]; /* 表記へのポインタ、長さ */
static unaCharT	unaWrdsPool[LOCAL_MAX_WORDS_POOL]; /* 全レコードの表記をプール */

/* 形態素IDとレコード番号を関係づけるテーブル */
static cvtMorIDTblT unaCvtTbl[MAX_REC_NO];

/* UNA品詞リスト */
static hinLstIdxT	*unaHinLst;		/* UNA品詞リスト */
static unaCharT		*unaHinPool;	/* UNA品詞リスト用文字列プール */

/* スピードアップ用 */
static uintT	unaStartBase = 0;	/* 空きベースを探し始める最初の位置 */

/* コストの最大値(量子化に必要) */
static double	unaCostMax;

/* 評価用 */
static uintT unaBaseCount = 0;	/* ベースの数 */
static uintT unaOccupyCnt = 0; /* Occupyの数(使用済Revuz配列要素数) */

/* プログラムスイッチ(スピード優先か、サイズ優先かが切り替わる) */
static uintT unaFlagVal1; /* FindBase関数のnoUsFlgのループ前初期値に使う */
static ucharT unaFlagVal2; /* FindBase関数で unaOccupy のセット値に使う */

/* 冗長情報モード用 */
static int unaVerboseMode; /* 冗長情報モードで実行するかのフラグ */

/* 連語の品詞名(UCS2、連語の形態素品詞番号を求めるのに使用) */
static const unaCharT unaRengo[2] =  {0x9023,0x8A9E};	/* '連語' */

/* unicode文字コード→内部文字コード変換用配列 */
static int codeCnvTable[0x10000];
static int codeMax; // 内部コードの最大値

/* レコード番号と圧縮レコード番号の対応表 */
static int recToCrec[MAX_REC_NO];

/* 連語用擬似品詞番号(main()で明示的に値を設定する) */
static int unaHinRengo = UNA_HIN_RENGO_V125;

/* 作成する形態素解析辞書の辞書形式(GetOptions()で明示的に値を設定する) */
static int dicVersion = V125;

//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	メインモジュール
//
// FUNCTION:
//	 パッチ済みUNA用辞書ソース及びパッチ済みUNA品詞リストから、
//   実行形態素辞書、実行アプリケーション辞書を作成する。
//
//	 使い方は、コマンド行から次の通りに入力する。
//	 mkunadic.exe [オプション] コストマックスファイル 品詞リスト
//											 辞書ソース 形態素辞書 アプリ辞書
//
// 例)
//	 mkunadic -a "For Ihyouki" -c -v -t costmax.log unahin.lst unadic.src
//														unawrd.dic unaapp.dic
//
// RETURN:
//	 0			正常
//	 1-255		エラー
//
// NOTE:
//	 UNA辞書ソースは、表記をキーとして予めソートされていること
//
int main(
	int argc,			/* コマンド行の引数の数(1以上) */
	char *argv[]		/* コマンド行の引数 */
)
{
	int rv;			/* 関数の返り値 */
	int i;				/* ループ変数 */
	int compactMode;	/* コンパクトモードで実行するかのフラグ */
	int tellMode;		/* 作成後、辞書情報の表示を行うかのフラグ */
	int processedArgc;	/* 処理した引数の数 */
	char *hinLst;		/* UNA品詞リスト */
	char *dicSrc;		/* UNA用辞書ソース */
	char *morDic;		/* 実行形態素辞書 */
	char *appDic;		/* 実行アプリ辞書 */
	int hinLstCnt;		/* UNA品詞リストのレコード件数 */
	uintT targetPos;	/* 処理対象とするレコードのカラム(要素番号、0起算) */
	uintT startRec;	/* 処理対象とする開始レコード(レコード番号、0起算) */
	uintT endRec;		/* 処理対象とする終了レコード(レコード番号、0起算) */
	uintT childR;		/* ベース */
	char* nullDicBase = (char*)"nullDic.";	/* 空辞書のベース名 */

	/* 初期設定 */
	for (i = 0; i < MAX_DA; i++){
		unaLabel[i] = MaxValueOfCode;
		unaBase[i]  = ClearValueOfBase;
	}
	for (i=0;i<LOCAL_MAX_WORDS_POOL;++i){
		unaWrdsPool[i] = MaxValueOfCode; /* 初期化 */
	}

	/* コマンドライン引数の処理 */
	compactMode		= NO;
	unaVerboseMode	= NO; 
	tellMode		= NO;  
	rv = GetOptions(argc,argv,unaComment,&compactMode,&unaVerboseMode,
									&tellMode,&processedArgc);
	if (rv < 0){
  		return 1;
	}

	if (argc - processedArgc != 5) {
  		fprintf(stdout,"Argument Err\n");
		return 2;
	}

	if (dicVersion == V125) {
		/* V1.25形式の形態素辞書を作る */
		/* バージョン文字列をV1.25形式のものにする */
		memcpy(unaDicVer, UNA_DIC_VER, UNA_VER_SIZE);

		/* ベース配列の要素上限値 */
		MaxValueOfLabelInDic = 0xffff;		/* 16ビット */
		MaxValueOfBaseInDic  = 0xffffffff;	/* 32ビット */

		/* 情報部領域を確保する */
		unaMorData = (unaMorDataT *)malloc(MAX_REC_NO * sizeof(unaMorDataT));

		/* 連語用擬似品詞番号(16ビット) */
		unaHinRengo = UNA_HIN_RENGO_V125;
	} else {
		/* V1.24形式の形態素辞書を作る */
		/* バージョン文字列をV1.24形式のものにする */
		memcpy(unaDicVer, UNA_DIC_VER_124, UNA_VER_SIZE);

		/* ベース配列の要素上限値 */
		MaxValueOfLabelInDic = 0x1fff;		/* 13ビット */
		MaxValueOfBaseInDic  = 0x7ffff;		/* 19ビット */

		/* 情報部領域を確保する */
		unaCMorData = (unaCompactMorDataT *)malloc(MAX_REC_NO * sizeof(unaCompactMorDataT));
		unaCMorData2 = (unaCompactMorDataT *)malloc(MAX_REC_NO * sizeof(unaCompactMorDataT));

		/* 連語用擬似品詞番号(11ビット) */
		unaHinRengo = UNA_HIN_RENGO_V124;
	}

	if (compactMode == YES){		/* compact but slow */
		unaFlagVal1 = 0; /* unaStartBaseリセット処理ちゃんと行うため */
		unaFlagVal2 = 2; /* ラベル値が書かれた所を他のベース候補とするため */
	}
	else {							/* big but very fast */
		unaFlagVal1 = 1; /* unaStartBaseリセット処理はしょるため */
		unaFlagVal2 = 1; /* ラベル値が書かれた所を他のベース候補としないため */
	}

	rv = GetCostMax(&unaCostMax,argv[processedArgc]);
	if (rv < 0) {
		return 3;
	}
	hinLst		= argv[processedArgc + 1];
	dicSrc      = argv[processedArgc + 2];
	morDic    	= argv[processedArgc + 3];
	appDic    	= argv[processedArgc + 4];

	/* unaCostMax 範囲チェック */
	if (unaCostMax <= 0) {
		fprintf(stdout,"COST_MAX must be plus.(Now %f)",unaCostMax);
		return 3;
	}

	/* UNA品詞リストをメモリに読み込み格納する */
	rv = ReadHinLst(hinLst);
	if (rv < 0){
		return 4;
	}
	hinLstCnt = rv;		/* 返り値は件数 */

	/* 辞書ソースのメモリへの格納 */
	rv = ReadDicSrc(dicSrc,hinLstCnt);
	if (rv < 0){
		return 5;
	}

	if (rv == 0){ /* 一件も読み込まなかった時 */
		fprintf(stdout,"Dic source has no data: %s\n", dicSrc);
		WriteDictNull(morDic, appDic);
 		if(strstr(dicSrc,nullDicBase) != NULL){
			/* normデータ作成のために空辞書が渡された場合 */
			return 0;
 		}else{
			return 6;
 		}
	}

	if (unaVerboseMode == YES) {
		fprintf(stdout,"REVUZ-001, Input %d lines\n",rv);
	}

	endRec = rv - 1;		/* 返り値は件数 */

	/* メモリの free */
	free(unaHinPool);
	free(unaHinLst);

	/* 文字コードの変換 */ 
	for (i=0;i<0x10000;++i){
		codeCnvTable[i] = MaxValueOfCode;
	}
	for ( i = 0; i < LOCAL_MAX_WORDS_POOL;++i){
		if ( unaWrdsPool[i] != MaxValueOfCode){ /* NULLではない */
			codeCnvTable[ unaWrdsPool[i]] = 1;
		}
	}
	for (i=0;i<0x20;++i){
		codeCnvTable[i] = 1; /* 制御コードのみはデフォルト */
	}
	int sum;
	sum = 0;
	for(i=0;i<0x10000;++i){
		if ( codeCnvTable[i] != MaxValueOfCode){
			codeCnvTable[i] = sum;
			sum++;
		}
	}
	codeMax = sum+1;
	for ( i = 0; i < LOCAL_MAX_WORDS_POOL;++i){
		if (  codeCnvTable[ unaWrdsPool[i] ] == MaxValueOfCode){
			assert( unaWrdsPool[i] == MaxValueOfCode);
		}
		unaWrdsPool[i] = codeCnvTable[ unaWrdsPool[i] ];
	}

	/* Revuz の Trie 辞書の作成(on memory) */
	startRec = 0;
	targetPos = 0;

	rv = Revuz(&childR,startRec,endRec,targetPos);
	if (rv < 0){
 		return 7;
	}

	/* 辞書の出力 */
	rv = WriteDict(morDic,appDic);
	if (rv < 0){
		return 8;
	}

	if (tellMode == YES) {
		rv = TestRevuz(childR); /* 評価用 */
		if (rv < 0){
			return 9;
		}
	}

	/* 情報部領域を解放する */
	if (dicVersion == V125) {
		free(unaMorData);
	} else {
		free(unaCMorData);
		free(unaCMorData2);
	}

  return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  GetOptions
//
// ABSTRACT:	  オプションの取得
//
// FUNCTION:
//	  コマンドラインよりオプションを取得する。
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  特になし
//
static int GetOptions(
	int argc,			/* コマンド行の引数の数(1以上) */
	char *argv[],		/* コマンド行の引数 */
	char *comment,		/* 辞書に付けるコメント */
	int *compactMode,	/* コンパクトモードで実行するかのフラグ */
	int *verboseMode,	/* 冗長情報モードで実行するかのフラグ */
	int *tellMode,		/* 作成後、辞書情報の表示を行うかのフラグ */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{
	int rv;			/* 関数の返り値 */

	/* argc の数だけループ。第1引数にはプログラム名が入っているから1より */
	dicVersion = V125;
	*processedArgc = 1;		/* 上記の理由により引数1個処理したとする */
	while (*processedArgc < argc) {
		if (*(argv[*processedArgc]) !='-') { /* オプションは '-' で始まる */
			break;				/* 辞書ファイルが指定されたとしてブレーク */
		}
		switch(*(argv[*processedArgc] + 1)){ /* 引数の2バイト目をテスト */
		case 'h':	/* ヘルプの要求 */
			/* Usageを表示して終了 */
			OutputUsage();
			(*processedArgc)++;
			return -1;
		case 'a':
			rv = OptionA(argc,argv,comment,processedArgc);
			if (rv < 0){
				return rv;
			}
			break;
		case 'c':
			*compactMode = YES;
			(*processedArgc)++;
			break;
		case 'v':
			*verboseMode = YES;
			(*processedArgc)++;
			break;
		case 't':
			*tellMode = YES;
			(*processedArgc)++;
			break;
		case 'b':
			dicVersion = V124;
			(*processedArgc)++;
			break;
		default:
			fprintf(stdout, "Bad option %s\n", argv[*processedArgc]);
			return -2;
		}
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  OutputUsage
//
// ABSTRACT:   使用法の表示
//
// FUNCTION:
//	  使用法を表示する
//
// RETURN:
//	  なし
//
// NOTE:
//	  Usage を表示する。
//
static void OutputUsage(void)
{
	fprintf(stdout,"mkunadic (Ver1.26)\n");
	fprintf(stdout,"Usage: mkunadic [OPTION] ");
	fprintf(stdout,
		"COST_MAX_FIL HIN_LST SRC_FILE MOR_DIC APP_DIC\n");
	fprintf(stdout,"Ex   : mkunadic -a \"For Ihyouki\" -c -v -t ");
	fprintf(stdout,
		"costmax.log unahin.lst unadic.src unawrd.dic unaapp.dic\n");
	fprintf(stdout,"  OPTION\n");
	fprintf(stdout,"    -h	           Help\n");
	fprintf(stdout,"    -a ANNOTATION  The String to annotate(Max48byte)\n");
	fprintf(stdout,"    -c             Compact but Very Slow MODE\n");
	fprintf(stdout,"    -v             Verbose MODE");
	fprintf(stdout,"(Display progress messages)\n");
	fprintf(stdout,"    -t             tell the informations of the");
	fprintf(stdout," dictionaries\n");
	fprintf(stdout,"    -b             backward compatible MODE\n");
	fprintf(stdout,"                   (make V1.24 dic instead of V1.25)\n");
	fprintf(stdout,"  Default values are...\n");
	fprintf(stdout,"    MAX_REC_NO     %8d records\n",MAX_REC_NO);
	fprintf(stdout,"    MAX_LST_NO     %8d entry\n",MAX_LST_NO);
	fprintf(stdout,"    MAX_DA         %8d double array size\n",MAX_DA);
	fprintf(stdout,"    MAX_APP_POOL   %8d byte\n",MAX_APP_POOL);
	fprintf(stdout,"    LOCAL_MAX_WORDS_POOL %8d MOJI\n",LOCAL_MAX_WORDS_POOL);
	fprintf(stdout,"  Note...\n");
	fprintf(stdout,"    On default\n");
	fprintf(stdout,
		"    LOCAL_MAX_WORDS_POOL is MAX_REC_NO * (AVE_WORD_LEN + 1)\n");
	fprintf(stdout,"    AVE_WORD_LEN   is %d\n",AVE_WORD_LEN);
	return;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionA
//
// ABSTRACT:   a オプションの処理
//
// FUNCTION:
//	  a オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionA(
	int argc,			/* argv 配列の要素数 */
	char *argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char *comment,		/* 辞書に付けるコメント */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{	
	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -a option.\n");
		return -3;
	}

	strncpy(comment,argv[*processedArgc + 1],UNA_COM_SIZE);
									/* " を除いてコピー */
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  GetCostMax
//
// ABSTRACT:	  コストの最大値を得る
//
// FUNCTION:
//	  コストマックスが書かれたファイルを読み込み、double 型データに変換して返す
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー番号
//
// NOTE:
//	  ・1行だけのファイルである
//	  ・コードは ASCII である
//	  ・形式は、printf("%f")で出力される形式である
//	  例)
//			14.1754345
static int GetCostMax(
	double *costMax,			/* コストの最大値 */
	char *costMaxFileName		/* コストマックスファイル名 */
)
{
	FILE *fp;					/* ファイル */
	char inBuf[MAX_LINE_LEN];	/* 入力バッファ */	

	/* コストマックスファイルのオープン */
	if ((fp = fopen(costMaxFileName,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",costMaxFileName);
		return -4;
	}

	fgets(inBuf,MAX_LINE_LEN,fp);
	*costMax =  atof(inBuf);	/* 数値でないものが現れたら変換終わり */

	/* ファイルのクローズ */
	fclose(fp);

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadHinLst
//
// ABSTRACT:	  UNA品詞リストを読み込む
//
// FUNCTION:
//	  UNA品詞リストの全レコードを読み込みメモリに格納する
//
// RETURN:
//	  正の値	読み込んだレコード数
//	  負の値	エラー番号
//
// NOTE:
//	  ・UNA品詞リストは、品詞名(文字列)によりソートされている
//	  ・レコードは品詞名を除きunaHinLstに格納される。品詞名は別途unaHinPool
//		に格納され、そこへのポインタとレングスが unaHinLstに格納される。
//
static int ReadHinLst(
	char *hinLst						/* UNA品詞リスト */
)
{
	struct stat stbuf;					/* ファイルの情報 */
	FILE		 *fp;					/* ファイルポインタ */
	int		 rv;					/* 関数の返り値 */
	uintT		 hinPoolEnd;			/* unaHinPoolの書き込みオフセット */
	uintT		 lineEnd;				/* 書き込んだレコード数 */
	uintT		 lineMax;				/* 書き込み可能なレコード数 */
	int		 tokenCnt;				/* トークンの数 */
	unaStrIndexT token[MAX_TOKEN_CNT];	/* トークン */
	unaCharT	 strMorHin[12];			/* 形態素品詞番号(文字表現) */

	/* 対象とするファイルのサイズをチェック */
	if(stat(hinLst,&stbuf)==-1){
		fprintf(stdout, "Can't get file info %s\n", hinLst);
		return -3;
	}

	/* まず最初分を確保 */
	unaHinLst = (hinLstIdxT *)malloc(LINES_BLOCK * sizeof(hinLstIdxT));
	/* unaHinPoolは一度に全部を確保(unaHinPool < stbuf.st_sizeで足りるはず) */
	unaHinPool = (unaCharT *)malloc(stbuf.st_size);

	/* 初期値の設定 */
	hinPoolEnd = 0;
	lineEnd    = 0;
	lineMax   = LINES_BLOCK;

	/* UNA品詞リストのオープン */
	if ((fp = fopen(hinLst,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",hinLst);
		return -4;
	}

	/* UNA品詞リストを直接 unaHinPool に読込む */
	while((rv=una_fgetws(unaHinPool+hinPoolEnd,MAX_LINE_LEN,fp)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		if (rv >= MAX_LINE_LEN - 1) { /* 行長がオーバーフローの可能性あり */
			fprintf(stdout,
				"Possibly the line buffer of UNA_HIN_LIST overflow\n");
			return -5;
		}

		/* トークンに分割する */
		tokenCnt = una_getTokens(unaHinPool+hinPoolEnd,MAX_TOKEN_CNT,token);

		/* コメント行は読み飛ばし */
		if (*(unaHinPool+hinPoolEnd+token[0].pos) == 0x0023){ /* #(シャープ)で始まってる行 */
			continue;
		}

		if (tokenCnt < 3) {
			fprintf(stdout,"%d is insufficient token count for %s\n",
					tokenCnt,hinLst);
			return -6;
		}

		/* 文字型形態素品詞番号(第2トークン)を数値に変換して格納 */
		memcpy(strMorHin,unaHinPool+hinPoolEnd+token[1].pos,
						token[1].len * sizeof(unaCharT));
		strMorHin[token[1].len] = 0x0000;	/* una_utol のためターミネート */
		unaHinLst[lineEnd].morHin = (ushortT)una_utol(strMorHin);

		/* UNA品詞番号は使用しないので 0 に */
		unaHinLst[lineEnd].unaHin = 0;

		/* 品詞名(第1トークン)のみを unaHinPool に残す */
		*(unaHinPool+hinPoolEnd+token[0].len) = 0x0000;	/* ターミネート */
		unaHinLst[lineEnd].ptr = unaHinPool+hinPoolEnd;	/* 品詞名の位置 */
		unaHinLst[lineEnd].len = token[0].len;			/* 品詞名の長さ */

		/* 品詞名の～, －を0xFF5E, 0xFF0Dから0x301C, 0x2212に置き換える */
		ReplaceSymbols(unaHinLst[lineEnd].ptr, unaHinLst[lineEnd].len);

		/* 更新 */
		lineEnd++;
		hinPoolEnd += (token[0].len + 1);	/* 1はターミネータの分 */

		/* 行バッファをオーバーフローする場合はリアロック */
		if ( lineEnd + 1 >= lineMax){
			lineMax += LINES_BLOCK;
			unaHinLst = (hinLstIdxT *)realloc(unaHinLst,
											lineMax * sizeof(hinLstIdxT));
		}
	}
	fclose(fp);

	return lineEnd;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadDicSrc
//
// ABSTRACT:	  UNA用辞書ソースを読み込む
//
// FUNCTION:
//	  UNA用辞書ソースの全レコードを読み込みグローバルメモリに格納する
//
// RETURN:
//	  負の値	エラー
//	  それ以外	読み込んだレコード数
//
// NOTE:
//	  処理速度及び安定性という点から格納先のメモリをグローバルメモリ
//	  (外部static変数)にしてあるのでオーバーフローには十分注意する。
//	  品詞名は、形態素品詞番号に変換され格納される。
//
static int ReadDicSrc(
	char		 *dicSrc,			   /* 辞書ソース */
	int		 hinLstCnt			   /* UNA品詞リストのレコード数 */
)
{
	FILE *fi;						   /* ファイルポインタ */
	int i;							   /* レコードの件数を数える */
	int inStrLen;					   /* レコード長 */
	int rv;						   /* 関数の返り値 */
	unaCharT	 inStr[MAX_LINE_LEN];  /* 読み込まれた文字列 */
	unaStrIndexT token[MAX_TOKEN_CNT]; /* トークン */
	int		 tokenCnt;			   /* トークンの数 */
	unaCharT	 *wordPtr;			   /* unaWordPoolを指すポインタ */
	int		 wordLen;			   /* 表記の文字数(\u変換後の) */
	char		 *appInfoPtr;	/* アプリ情報情報部のポインタ情報を保持 */
	uintT		 appInfoOffset;		   /* unaAppPoolのオフセット */
	int		 padding;			   /* 処理系の違いのため必要 */

	/* 初期設定 */
	wordPtr = unaWrdsPool;
	appInfoPtr = unaAppPool;
	appInfoOffset = 0;
	
	/* 辞書ソースのオープン */
	if((fi = fopen(dicSrc,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",dicSrc);
		return -7;
	}

	i = 0; /* 入力レコードをカウントする */
	while((inStrLen = una_fgetws(inStr,MAX_LINE_LEN,fi)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		rv = ChkLineLen(inStrLen);
		if (rv < 0) {
			return rv;
		}
		
		/* 入力レコード件数オーバーフローチェック */
		rv = ChkOverflowRecNo(i);
		if (rv < 0) {
			return rv;
		}

		/* トークンに分割する */
		tokenCnt = una_getTokens(inStr,MAX_TOKEN_CNT,token);
		if (tokenCnt < 4) {
 			fprintf(stdout,"%d is insufficient token count for %s\n",
					tokenCnt,dicSrc);
			return -10;
		}

		/* 品詞名(第3トークン)の～, －を0x301C, 0x2212に置き換える */
		ReplaceSymbols(inStr + token[2].pos, token[2].len);

		/* 形態素ID(第1トークン)の処理を行う */
		rv = ProcMorID(inStr,&token[0],i);
		if (rv < 0){
			fprintf(stderr,"line No = %d\n",i);
			return rv;
		}

		/* unaWrdsPoolオーバーフローチェック(ターミネート分も考慮してある) */
		if(wordPtr + token[1].len > &unaWrdsPool[LOCAL_MAX_WORDS_POOL - 1]){
			fprintf(stdout,"The array 'unaWrdsPool' is overflow.\n"
			               "Increase LOCAL_MAX_WORDS_POOL\n");
			return -11;
		}

		/* 表記(第2トークン)を unaWrdsPool に書き込む */
		rv = HyokiSet(wordPtr,inStr + token[1].pos,token[1].len,&wordLen);
		if (rv < 0){
			return rv;
		}

		/* 表記の管理情報(インデックス)をunaWrdsPoolIdxに書き込む */
		unaWrdsPoolIdx[i].wordLen		= (ucharT)wordLen;
		unaWrdsPoolIdx[i].wordPtr		= wordPtr;
		wordPtr += (wordLen + 1);	/* ポインタの更新(EOKの分含む) */

		if (dicVersion == V125) {
			/* 表記の長さ(文字数)を書き込む */
			assert(wordLen<=0xff);
			unaMorData[i].morLen = (ucharT)wordLen;

			/* 品詞名(第3トークン)の処理を行う(チェック&形態素品詞番号に変換) */
			rv = GetMorHin(inStr,&token[2],hinLstCnt,i);
			if (rv < 0){
				return rv;
			}
			assert( rv <= 0xffff);
			unaMorData[i].morHinNo = (ushortT)rv;

			/* コスト(第4トークン)を量子化し取得 */
			rv = GetCost(inStr,&token[3]);
			if (rv < 0){
				return rv;
			}
			assert( rv <= 0xff);
			unaMorData[i].morCost = (ucharT)rv;
		} else {
			/* 表記の長さ(文字数)を書き込む */
			assert(wordLen<=0xff);
			unaCMorData[i].mor.morLen = (ucharT)wordLen;

			/* 品詞名(第3トークン)の処理を行う(チェック&形態素品詞番号に変換) */
			rv = GetMorHin(inStr,&token[2],hinLstCnt,i);
			if (rv < 0){
				return rv;
			}
			assert( rv <= 0x7ff);
			unaCMorData[i].mor.morHinNo = (ushortT)rv;

			/* コスト(第4トークン)を量子化し取得 */
			rv = GetCost(inStr,&token[3]);
			if (rv < 0){
				return rv;
			}
			assert( rv <= 0xff);
			unaCMorData[i].mor.morCost = (ucharT)rv;
		}

		/* unaAppPool オーバーフローチェック(byteで)。なお padding は
				実データを書き込むわけではないのでここでは考えなくてよい */
		rv = ChkOverflowAppPool(tokenCnt,inStrLen,token,appInfoPtr);
		if (rv < 0){
			return rv;
		}

		/* アプリ情報(第5トークン以降)を unaAppPool に書き込む */
		if (tokenCnt < 5){	/* アプリ情報がない時 */
				((unaAppInfoT *)appInfoPtr)->info[0]	= 0;
				((unaAppInfoT *)appInfoPtr)->len		= 0;
		}
		else{
			rv = AppInfoSet(((unaAppInfoT *)appInfoPtr)->info,
							inStr+token[4].pos,(inStrLen - token[4].pos),
							&((unaAppInfoT *)appInfoPtr)->len);
			if (rv < 0){
				return rv;
			}
		}
		/* アプリ情報の管理情報(オフセット)を unaAppOffset に書き込む */
		unaAppOffset[i] = appInfoOffset;

		/* 語境界をそろえる為(SPARCではintは4の倍数に配置される必要あり) */
		padding = GetPadding(((unaAppInfoT *)appInfoPtr)->len);
		
		/* オフセットの更新sizeof(((unaAppInfoT *)appInfoPtr)->len) = 4です */
		appInfoOffset += (((unaAppInfoT *)appInfoPtr)->len
					  + sizeof(((unaAppInfoT *)appInfoPtr)->len)) + padding;
		appInfoPtr = unaAppPool + appInfoOffset; /* 次の書き始め位置の更新 */

		/* 読み込んだレコード数をカウント */
		i++;
	}

	fclose(fi);

	/* 下位構造の処理 */
	rv = ProcSubMor(i);
	if (rv < 0){
		return rv;
	}

	return i;
}


//--------------------------------------------------------------------------
// MODULE:	  ChkLineLen
//
// ABSTRACT:	  読み込んだ1行の長さチェック
//
// FUNCTION:
//	  読み込んだ1行の長さをチェックする
//
// RETURN:
//	  UNA_OK		正常終了
//	  それ以外		エラー
//
// NOTE:
//
static int ChkLineLen(
	int inStrLen					   /* 行長 */
)
{
	if (inStrLen >= MAX_LINE_LEN - 1) { /* 行長オーバーフローの可能性有 */
		fprintf(stdout,
			"Possibly the line buffer of UNA_DIC_SRC overflow\n");
		return -8;
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ChkOverflowRecNo
//
// ABSTRACT:	  入力レコード件数オーバーフローチェック
//
// FUNCTION:
//	  入力レコード件数のオーバーフローをチェックする
//
// RETURN:
//	  UNA_OK		正常終了
//	  それ以外		エラー
//
// NOTE:
//
static int ChkOverflowRecNo(
	int i							   /* レコードの件数 */
)
{
	if (i >= MAX_REC_NO){
		fprintf(stdout,
			"Line count of dictionary source exceeded MAX_REC_NO\n"
		    "Increase MAX_REC_NO\n");
		return -9;
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ChkOverflowAppPool
//
// ABSTRACT:	  入力レコード件数オーバーフローチェック
//
// FUNCTION:
//	  入力レコード件数のオーバーフローをチェックする
//
// RETURN:
//	  UNA_OK		正常終了
//	  それ以外		エラー
//
// NOTE:
//
static int ChkOverflowAppPool(
	int		 tokenCnt,		/* トークンの数 */
	int		 inStrLen,		/* レコード長 */
	unaStrIndexT *token,		/* トークン */
	char		 *appInfoPtr	/* アプリ情報情報部のポインタ情報を保持 */
)
{
	uintT		 tmpAppInfByte; /* アプリ情報のバイト数(\u、\x変換前の) */

	if (tokenCnt < 5){	/* アプリ情報がない時 */
		tmpAppInfByte = sizeof(unaAppInfoT);
	}
	else {
		tmpAppInfByte = (inStrLen - token[4].pos) * sizeof(unaCharT)
														+ sizeof(char *);
	}
	if(appInfoPtr + tmpAppInfByte - 1 > &unaAppPool[MAX_APP_POOL - 1]){
		fprintf(stdout,"The array 'unaAppPool' is overflow.\n"
		               "Increase MAX_APP_POOL\n");
		return -12;
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ProcMorID
//
// ABSTRACT:	  形態素IDの処理を行う
//
// FUNCTION:
//	  形態素ごとにユニークに振られた形態素IDの処理を行う。
//
// RETURN:
//		UNA_OK				正常終了
//		その他				エラー
//
// NOTE:
//	  この関数では以下を行う。
//	  - 形態素IDの文字列をセパレータ(:)によって分割する
//	  - 先頭の形態素ID(morID[0])は、当該形態素の形態素IDであると判断し、
//		形態素IDとレコード番号を関連付けるテーブルに書き込む
//	  - 2番目以降の形態素IDがあれば、下位構造の形態素IDであると判断し、
//		下位構造リストへセットする。又、そこへのポインタ情報を、
//		unaCompactMorDataTの下位構造リストへのポインタ欄にセットする
//
static int ProcMorID(
	unaCharT *iStr,						/* 形態素IDの入った文字列 */
	unaStrIndexT *strIdx,				/* オフセット、長さ */
	int recNo							/* レコード番号 */
)
{
	int morIDCnt;						/* 形態素ID数 */
	int morID[MAX_ELEM_CNT];			/* 形態素ID */
	int i;								/* ループ変数 */

	morIDCnt = SeprMorID(iStr + strIdx->pos,strIdx->len,MAX_ELEM_CNT,morID);
	if (morIDCnt <= 0) { /* エラー */
	fprintf(stderr,"%c%c%c%c%c%c%c\n", 
	iStr[0], iStr[1], iStr[2], iStr[3], iStr[4], iStr[5], iStr[6]);
		return morIDCnt;
	}

	/* 形態素IDとレコード番号を関連付けるテーブルへのセット */
	unaCvtTbl[recNo].morphID	= morID[0];
	unaCvtTbl[recNo].recNo		= recNo;

	/* 下位構造のセット */
	if (morIDCnt > 1) {	/* 下位構造がある時 */
		if (dicVersion == V125) {
			unaMorData[recNo].subLstNo = unaLstNo; /* 下位構造リストへのポインタ */
		} else {
			unaCMorData2[recNo].sub.subLstNo = unaLstNo; /* 下位構造リストへのポインタ */
		}
		unaSubLst[unaLstNo]		= morIDCnt - 1;
		assert(unaSubLst[unaLstNo] < 0x80000000);
		unaSubLst[unaLstNo] |= 0x80000000;
									/* リストの先頭は下位構造形態素の数 */
		unaLstNo++;
		for (i = 1;i < morIDCnt;i++) {
			assert(morID[i]<0x80000000);
			unaSubLst[unaLstNo++]	= morID[i]; /* 下位形態素の形態素ID */
		}
	}
	else {
		if (dicVersion == V125) {
			unaMorData[recNo].subLstNo = 0xffffff; /* 下位構造がない時(仕様) */
		} else {
			unaCMorData2[recNo].sub.subLstNo = 0xffffff; /* 下位構造がない時(仕様) */
		}
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  SeprMorID
//
// ABSTRACT:	  形態素IDを分割する
//
// FUNCTION:
//	  形態素IDをセパレータで分割する
//
// RETURN:
//		正の値				形態素IDの数
//		その他				エラー
//
// NOTE:
//	  形態素IDの文字列は以下のような形式をしている。
//	  - コロン(:)をセパレータとしていくつかの数字列に区切られる
//		例)000001800:000001700:000012000
//	  - セパレータで区切られないものも存在する
//		例)000001501
//	  - 一つの数字列の固まりは、9桁である
//
static int SeprMorID(
	unaCharT *morIDStr,		/* 形態素IDの文字列の開始アドレス */
	int morIDStrLen,		/* 形態素IDの文字列の長さ */
	int maxElemCnt,		/* 最大トークン数(morID配列の大きさ) */
	int *morID				/* 形態素ID */
)
{
	int i;					/* ループ変数 */
	int j;					/* ループ変数 */
	int k;					/* ループ変数 */
	unaCharT uniMorID[10];	/* 切り出した単一の形態素ID */
	int morIDIdx;			/* 切り出した形態素IDに付ける番号(0起算) */

	/* 初期設定 */
	assert(morIDStrLen > 0);
	morIDIdx = 0;

	/* ループ前初期設定 */
	j = 0;
	for (i = 0;i <= morIDStrLen;i++) {

		if (morIDStr[i] != 0x003A	/* ':' でなく */
		 && j < 9					/* かつ9桁以下 */
		 && i < morIDStrLen) {		/* かつ文字列の長さは超えてない */
			uniMorID[j++] = morIDStr[i];
			continue;
		}

		if (j != 9) {	/* 9桁ではない */
			fprintf(stdout,"The length of Morph ID is not 9\n");
			fprintf(stdout," ...%c%c%c%c%c%c%c%c\n",
			morIDStr[0], morIDStr[1], morIDStr[2], morIDStr[3], 
			morIDStr[4], morIDStr[5], morIDStr[6], morIDStr[7]);
			return -10;
		}

		uniMorID[j] = 0x0000;

		if (morIDIdx >= maxElemCnt) { /* 切り出した形態素IDの数が多い */
			fprintf(stdout,"The SubMorph count is too many\n");
			return -10;
		}
			
		/* 文字種チェック(UNICODEの '0'  '9' か) */
		k = 0;
		while(uniMorID[k] != '\0') {
			if (uniMorID[k] < 0x0030 || 0x0039 < uniMorID[k]){ /* 数字以外 */
				fprintf(stdout,"Illegal character of Morph ID\n");
				return -10;
			}
			k++;
		}

		morID[morIDIdx++] = una_utol(uniMorID);	/* 数値に変換されたmorID */

		j = 0;
	}

	return morIDIdx;
}


//--------------------------------------------------------------------------
// MODULE:	  HyokiSet
//
// ABSTRACT:	  表記のセット
//
// FUNCTION:
//	  表記をセットする
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  hyokiLen = 0 の場合は、*wordPtr に NULL だけが書き込まれ、
//	  *realHyokiLen = 0 となる
//
static int HyokiSet(
	unaCharT	*wordPtr,		 /* コピー先(UNICODE) */
	unaCharT	*const hyokiPtr, /* コピー元(UNICODE) */
	int		hyokiLen,		 /* コピー元の長さ(文字数) */
	int		*realHyokiLen	 /* 実際にコピーした長さ(文字数) */
	)
{
	int i;						 /* ループ変数(処理した文字数を表わす) */
	unaCharT tmpStr[5];			 /* una_xtol用(ターミネートするため必要) */

	*realHyokiLen = 0;
	for(i = 0;i < hyokiLen;){
		if ((hyokiPtr[i] == 0x005C)				/* '\'(バックスラッシュ) */
		&& (hyokiPtr[i + 1] == 0x0075)			/* '\'の後ろの文字が 'u' */
		&& (i + 5 < hyokiLen)					/* 変換の桁が十分 */
		&& (una_isxstr(&hyokiPtr[i + 2],4) != 0)) {	/* 16進の文字 */
			memcpy(tmpStr,&hyokiPtr[i + 2],4 * sizeof(unaCharT));
			tmpStr[4] = 0x0000;
			*wordPtr = (unaCharT)una_xtol(tmpStr);
			if (*wordPtr == 0x0000 || *wordPtr == 0xFEFF){
				fprintf(stdout,"\\uXXXX that means NULL or BOM exists\n");
				return -15;
			}
			i += 6;
			wordPtr ++;
			(*realHyokiLen) ++;
		}
		else {
			*wordPtr = *(hyokiPtr + i);
			i++;
			wordPtr++;
			(*realHyokiLen)++;
		}
	}

	/* 表記長制限チェック */
	if (*realHyokiLen > UNA_HYOKI_LEN_MAX) {
		fprintf(stdout,"Too int Hyoki(len=%d)\n",*realHyokiLen);
		return -17;
	}
	
	*wordPtr = 0x0000; /* 最後はヌルでターミネート */

	return UNA_OK; 
}


//--------------------------------------------------------------------------
// MODULE:	  ReplaceSymbols
//
// ABSTRACT:	  記号の置き換え
//
// FUNCTION:
//	  ～, －のコードポイントを以下のように置き換える
//	    ～：0xFF5E → 0x301C
//	    －：0xFF0D → 0x2212
//    ～と－は品詞リスト内で文字コードが0xFF5E, 0xFF0Dであるものの、
//    ソート順で0x301C, 0x2212の位置に置かれているため、
//    バイナリサーチでこれらの文字を探せるよう文字コードを変更する
//
// RETURN:
//     なし
//
static void ReplaceSymbols(
	unaCharT *pos,		/* 置き換え対象文字列の開始位置 */
	int len				/* 置き換え対象文字列の長さ */
)
{
	while (len-- > 0) {
		if (*pos == 0xff5e) {
			*pos = 0x301c;
		} else if (*pos == 0xff0d) {
			*pos = 0x2212;
		}
		pos++;
	}
}


//--------------------------------------------------------------------------
// MODULE:	  GetMorHin
//
// ABSTRACT:	  形態素品詞番号の取得
//
// FUNCTION:
//	  バイナリサーチにより形態素品詞番号を取得する
//
// RETURN:
//	  負の値	エラー
//	  それ以外	形態素品詞番号
//
// NOTE:
//	  メモリ上に格納したUNA品詞リストからからバイナリサーチにより
//	  形態素品詞番号を取得する。マッチングキーは品詞名である。
//	  但し、連語の場合は、
//	  - バイナリサーチは行われず unaHinRengo が返される。
//	  - 下位構造があるかどうかも同時にチェックする
//
static int GetMorHin(
	unaCharT *iStr,						/* 品詞名の入った文字列 */
	unaStrIndexT *strIdx,				/* オフセット、長さ */
	int hinLstCnt,						/* UNA品詞リストの件数 */
	int recNo							/* レコードの番号 */
)
{
	unaCharT	*hinAdr;				/* 品詞名の開始アドレス */
	hinLstIdxT	keyIdx;					/* 探したいキーをセット */
	hinLstIdxT	*ansIdx;				/* マッチした場合の答えが返る */

	hinAdr = iStr + strIdx->pos;		/* オフセットを足してセット */
	/* 連語の場合の処理 */
	if (memcmp(hinAdr,unaRengo,sizeof(unaRengo)) == 0) { /* 連語 */
		if (dicVersion == V125 && unaMorData[recNo].subLstNo == 0xffffff ||
			dicVersion == V124 && unaCMorData2[recNo].sub.subLstNo == 0xffffff) {
			/* 下位構造無し */
			fprintf(stdout,"RENGO doesn't have Sub Morph(RecNo=%d)\n",recNo);
			return -18;
		}
		return unaHinRengo;
	}

	keyIdx.ptr = hinAdr;				/* キーの開始アドレス */
	keyIdx.len = strIdx->len;			/* キー長さをセット */
	keyIdx.morHin = 0;					/* 未使用 */
	keyIdx.unaHin = 0;					/* 未使用 */
	ansIdx = (hinLstIdxT *)bsearch(&keyIdx,unaHinLst,
				hinLstCnt,sizeof(hinLstIdxT),(unaCompFuncT)LineComp);

	if(ansIdx == NULL){
		fprintf(stdout,"No match data in UNA HINSHI LIST(Key=");
		PrintUcsToUtf8(keyIdx.ptr,keyIdx.len);
		fprintf(stdout,")\n");
		return -18;
	}

	return ansIdx->morHin;				/* 形態素品詞番号を返す */
}


//--------------------------------------------------------------------------
// MODULE:	  PrintUcsToUtf8
//
// ABSTRACT:	  ユニコード文字列のUTF-8表示
//
// FUNCTION:
//	  ユニコード文字列をUTF-8に変換して表示する
//
static void PrintUcsToUtf8(
	unaCharT *ptr,				/* 文字列(品詞名)へのポインタ */
	uintT	 len				/* 文字列(品詞名)の長さ       */
)
{
	uintT i;					/* ループ変数 */
	
	ucharT str[4];
	for (i = 0;i < len; i++) {
		if (ptr[i] < 0x0080) {
			str[1] = 0;
			str[0] = (ucharT)(ptr[i] & 0x7f);
		} else if (ptr[i] < 0x0800) {
			str[0] = (ucharT)((ptr[i] >> 12) & 0x3f | 0xc0);
			str[1] = (ucharT)((ptr[i] >>  6) & 0x3f | 0x80);
			str[2] = 0;
		} else {
			str[0] = (ucharT)((ptr[i] >> 12) & 0x3f | 0xe0);
			str[1] = (ucharT)((ptr[i] >>  6) & 0x3f | 0x80);
			str[2] = (ucharT)((ptr[i] >>  0) & 0x3f | 0x80);
			str[3] = 0;
		}
		fprintf(stdout, "%s", str);
	}
	return;
}


#if 0
//--------------------------------------------------------------------------
// MODULE:	  PrintUcsToHex
//
// ABSTRACT:	  ユニコード文字列の16進数表示
//
// FUNCTION:
//	  ユニコード文字列を16進数表示する
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  なし
//
static void PrintUcsToHex(
	unaCharT *ptr,				/* 文字列(品詞名)へのポインタ */
	uintT	 len				/* 文字列(品詞名)の長さ       */
)
{
	uintT i;					/* ループ変数 */
	
	for (i = 0;i < len; i++) {
		fprintf(stdout,"\\u%04hX",*(ptr + i));
	}
	return;
}
#endif


//--------------------------------------------------------------------------
// MODULE:	  GetCost
//
// ABSTRACT:	  コストの取得
//
// FUNCTION:
//	  コストを計算して返す
//
// RETURN:
//	  負の値	エラー
//	  それ以外	コスト(0-254)
//
// NOTE:
//	  255は未使用。但し、接続コストでは特別な値として使用される。
//
static int GetCost(
	unaCharT *iStr,				/* コスト(文字型)の入った文字列 */
	unaStrIndexT *strIdx		/* オフセット、長さ */
)
{
	unaCharT uCost[18];			/* コスト(文字列) */
	double dCost;				/* 文字列コストを double に変換したもの */
	ucharT cCost;				/* 量子化されたコスト(これを返す) */

	if (18 <= strIdx->len) {
		fprintf(stdout,"The length of String MorCost is too long\n");
		return -10;
	}

	memcpy(uCost,iStr + strIdx->pos,strIdx->len * sizeof(unaCharT));
	uCost[strIdx->len]=0x0000;

	dCost = una_utof(uCost);

	/* 接続値を 0  Q_COST_MAX -1 に量子化 */
	if (dCost < 0){
		fprintf(stdout,"The cost of morph is minus(cost=%f)\n",dCost);
		return -19;
	}
	else{
		if (dCost > unaCostMax) {
			dCost = 1;
		}
		else {
			dCost /= unaCostMax; /* この時点でdCostは1以下 */
		}
		cCost = (ucharT)(dCost * (Q_COST_MAX - 1) + 0.5);
								 /* 四捨五入。最大で Q_COST_MAX - 1 になる */
	}

	return cCost;
}


//--------------------------------------------------------------------------
// MODULE:	  AppInfoSet
//
// ABSTRACT:	  アプリケーション情報のセット
//
// FUNCTION:
//	  アプリケーション情報をセットする
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  なし
//
static int AppInfoSet(
	char		*appInfoInfoPtr, /* コピー先(BINARY) */
	unaCharT	*const infoPtr,	 /* コピー元(UNICODE) */
	int		infoLen,		 /* コピー元の長さ(文字数) */
	int		*realInfoLen	 /* 実際にコピーした長さ(バイト数) */
)
{
	int i;						 /* ループ変数(処理した文字数を表わす) */
	unaCharT tmpStr[5];			 /* una_xtol用(ターミネートするため必要) */

	*realInfoLen = 0;
	for(i = 0;i < infoLen;){
		if (infoPtr[i] == 0x005C) {	/* '\'(バックスラッシュ) */
			if ((infoPtr[i + 1] == 0x0075)			/* 'u' */
			&& (i + 5 < infoLen)					/* 変換の桁が十分 */
			&& (una_isxstr(&infoPtr[i + 2],4) != 0)) {	/* 16進の文字 */
				memcpy(tmpStr,&infoPtr[i + 2], 4 * sizeof(unaCharT));
				tmpStr[4] = 0x0000;
				*(unaCharT *)appInfoInfoPtr = (unaCharT)una_xtol(tmpStr);
				i += 6;
				appInfoInfoPtr += 2;
				(*realInfoLen) += 2;
				continue;
			}
			else if ((infoPtr[i + 1] == 0x0078)		/* 'x' */
			&& (i + 3 < infoLen)					/* 変換の桁が十分 */
			&& (una_isxstr(&infoPtr[i + 2],2) != 0)) {	/* 16進の文字 */
				memcpy(tmpStr,&infoPtr[i + 2], 2 * sizeof(unaCharT));
				tmpStr[2] = 0x0000;
				*appInfoInfoPtr = (char)una_xtol(tmpStr);
				i += 4;
				appInfoInfoPtr++;
				(*realInfoLen)++;
				continue;
			}
		}

		*((unaCharT *)appInfoInfoPtr) = *(infoPtr + i);
		i++;
		appInfoInfoPtr += 2;
		(*realInfoLen) += 2;
	}

	return UNA_OK; 
}


//--------------------------------------------------------------------------
// MODULE:	  GetPadding
//
// ABSTRACT:	  パディングを求める
//
// FUNCTION:
//	  語境界をそろえる為のパディングを求める
//
// RETURN:
//		パディングの値
//
// NOTE:
//	  SPARCではintは4の倍数に配置される必要あり
//
static int GetPadding(
	int	len				/* アプリケーション情報の長さ */
)
{
	int	padding;		/* パディング値 */

	padding = len % UNA_ALIGNMENT;
	if (padding != 0){
		padding = UNA_ALIGNMENT - padding;	/* SUN's SPARC */
	}

	return	padding;

}


//--------------------------------------------------------------------------
// MODULE:	  ProcSubMor
//
// ABSTRACT:	  下位形態素IDの処理を行う
//
// FUNCTION:
//	  下位構造形態素IDの処理を行う。
//
// RETURN:
//		UNA_OK				正常終了
//		その他				エラー
//
// NOTE:
//	  この関数では以下を行う。
//	  - 形態素IDとレコード番号を関連付けるテーブルを形態素IDによってソート
//		する
//	  - 下位構造リストの内容をbsearchにより 形態素ID→レコード番号に変換する
//	  - 下位構造に連語が含まれていればエラーとする
//
static int ProcSubMor(
	int recCnt							/* レコード件数 */
)
{
	int i;								/* ループ変数 */
	int j;								/* 処理した下位形態素の数 */
	int subMorCnt;						/* 下位形態素の数 */
	cvtMorIDTblT keyIdx;				/* 探したいキーをセット */
	cvtMorIDTblT *ansIdx;				/* マッチした場合の答えが返る */

	/* 形態素IDとレコード番号を関連付けるテーブルのソート */
	una_msort(unaCvtTbl,0,recCnt - 1,sizeof(cvtMorIDTblT),
										(unaCompFuncT)MorIDComp,NULL);


	/*
	 * 下位構造リストの内容を形態素ID→レコード番号に変換する
	 */

	/* ループ前初期設定 */
	j			= 0;	/* 初回のエントリが下位形態素の数を表わす様 */
	subMorCnt	= 0;	/* 同上 */
	for (i = 0;(unsigned)i <= unaLstNo; i++) {
		if (j >= subMorCnt) {	/* エントリが下位形態素の数を表わす時 */
			subMorCnt = (unaSubLst[i]&0x7fffffff);	/* 下位形態素の数 */
			j = 0;
			continue;
		}

		/* 変換 */
		assert( unaSubLst[i]<0x80000000);
		keyIdx.morphID	= unaSubLst[i];	/* この時点では形態素IDが入ってる */
		keyIdx.recNo	= 0;	/* 未使用 */
		ansIdx = (cvtMorIDTblT *)bsearch(&keyIdx,unaCvtTbl,
					recCnt,sizeof(cvtMorIDTblT),(unaCompFuncT)MorIDComp);

		assert(ansIdx != NULL); /* 存在するものをテーブル
									に格納したのでアンマッチは有得ない */

		/* 下位形態素が連語かチェック */
		if (dicVersion == V125 && unaMorData[ansIdx->recNo].morHinNo == unaHinRengo ||
			dicVersion == V124 && unaCMorData[ansIdx->recNo].mor.morHinNo == unaHinRengo) {
			fprintf(stdout,"RENGO exist in subMorph\n");
			return -10;
		}
		
		unaSubLst[i] = ansIdx->recNo;	/* レコード番号に変換 */
		assert( unaSubLst[i]<0x80000000);
		j++;
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  Revuz
//
// ABSTRACT:	  Revuzのtrie作成
//
// FUNCTION:
//	  Revuzのtrieを作成する。
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  共通接頭辞を持つ文字列群(共通接頭辞は除いてある)を受け取りそれをRevuz
//	  のダブル配列にセットした後ベース値を返す
//	  受け取った文字列に共通接頭辞があれば同様の処理を繰り返し行なう(再帰)
//
//  (注1)
//		下位のRevuz関数を実行し現在処理中のカラムより後の文字を先に
//		セットする。
//
//		あいうえお
//		いいうう
//		いいえいうあ
//
//		上で、23行目の2カラム目を処理中の時、「うう」「えいうあ」を
//		Revuz配列にセットして、その時の2カラム目の次(3カラム目)の
//		ベース値(例では「う」「え」に遷移可能なベース値)を返してもらう
//		即ち、ここで返ってきたベース値は、次のカラムの文字へ遷移
//		するためのものである
//
static int Revuz(
	uintT		 *r,		  /* ベース値 */
	const uintT startRec,	  /* 開始レコード番号(0起算) */
	const uintT endRec,	  /* 終了レコード番号(0起算) */
	const uintT targetPos	  /* 処理対象とするカラム(0起算) */
)
{
	int	 rv;			  /* 関数の返り値 */
	uintT	 i;				  /* ループ変数 */
	uintT	 k;				  /* ループ変数 */
	uintT	 childR;		  /* 下位Revuz関数が返すベース値 */
	uintT	 stRecOfSamePrfx; /* 共通接頭辞を持つレコードの開始RecNo
								(0起算、下位のRevuz関数用) */
	uintT	 edRecOfSamePrfx; /* 共通接頭辞を持つレコードの終了RecNo
								(0起算、下位のRevuz関数用) */
	unaCharT oldUniChar;	  /* 現在の行(i行)の1つ前の行の同カラムの文字 */

	/* 現在のカラムの全文字が格納可能な最小のベース値 r を求める */
	rv = FindBase(r,startRec,endRec,targetPos);
	if (rv < 0) {
		return rv;
	}

	/* ループ前初期設定 */
	stRecOfSamePrfx = startRec;
	oldUniChar = (unaWrdsPoolIdx[startRec].wordPtr)[targetPos];

	for(i = startRec;i <= endRec + 1;i++){ /* i の最大値は、endRec + 1 */

		/* 下位のRevuz関数に渡す為にedRecOfSamePrfxを決定する */
		if (i <= endRec){
			if (oldUniChar == (unaWrdsPoolIdx[i].wordPtr)[targetPos]){
				continue;	/* 同じ文字が続いている時 */
			}
			else{			/* 文字がコントロールブレークした */
				edRecOfSamePrfx = i - 1; /* ブレークした1つ前の行をセット */
			}
		}
		else{	/* i > endRec の時(即ち i == endRec + 1 の時、最終ループ) */
			edRecOfSamePrfx = endRec;	 /* 最終行をセット */
		}

		/*
		 * 以下はコントロールブレーク(最終行に達した事を含む)処理
		 */
		if (oldUniChar != EOK){	/* 語の最後ではない時(注1) */
			rv = Revuz(&childR, stRecOfSamePrfx,
						edRecOfSamePrfx,(targetPos + 1));
			if (rv < 0) {
				return rv;
			}
			/* 次のカラムの文字へ遷移するためのベース値を書き込む */
			unaBase[*r + oldUniChar] = childR;
		}
		else{				/* 語の最後の時(最後はEOKを処理することになる) */
			unaLabel[*r] = 0;		/* ラベルは0(実はFindBaseでセット済み) */
			unaBase[*r] = unaRecNo;	/* ラベルが0の時のベースはRecNo */

			/* 以下同形語の間ループ(同形語がない場合は1ループして抜ける) */
			for (k = stRecOfSamePrfx; k <= edRecOfSamePrfx;k++){

				assert(unaRecNo == k);
				if ((unaVerboseMode == YES)&&((k + 1) % 1000 == 0)){
					fprintf(stdout,"REVUZ-002, Process %u lines\n",k + 1);
				}

				if (dicVersion == V125) {
					/* 連語が同形語をもつかチェック */
					if (unaMorData[unaRecNo].morHinNo == unaHinRengo
					 && stRecOfSamePrfx != edRecOfSamePrfx) {
						fprintf(stdout,"RENGO has the same form word\n");
						return -10;
					}

					if (k == stRecOfSamePrfx) {
						/* 先頭の同形語の同形数は自分を含んだ同形語の数をセット(仕様) */
						assert((ucharT)(edRecOfSamePrfx - stRecOfSamePrfx + 1)<=0x0f); 
						unaMorData[unaRecNo].doukeiNo
							= (ucharT)(edRecOfSamePrfx - stRecOfSamePrfx + 1); 
					}
					else {
						/* 先頭以外の同形語の同形数は0をセット(仕様) */
						unaMorData[unaRecNo].doukeiNo = 0;
					}
				} else {
					/* 連語が同形語をもつかチェック */
					if (unaCMorData[unaRecNo].mor.morHinNo == unaHinRengo
					 && stRecOfSamePrfx != edRecOfSamePrfx) {
						fprintf(stdout,"RENGO has the same form word\n");
						return -10;
					}

					if (k == stRecOfSamePrfx) {
						/* 先頭の同形語の同形数は自分を含んだ同形語の数をセット(仕様) */
						assert((ucharT)(edRecOfSamePrfx - stRecOfSamePrfx + 1)<=0x0f); 
						unaCMorData[unaRecNo].mor.doukeiNo
							= (ucharT)(edRecOfSamePrfx - stRecOfSamePrfx + 1); 
					}
					else {
						/* 先頭以外の同形語の同形数は0をセット(仕様) */
						unaCMorData[unaRecNo].mor.doukeiNo = 0;
					}
				}
				unaRecNo ++; /* 次回登録時のRecNoになる。最後はRec数を示す */
			}
		}

		/* 次回ループのための前設定
		  (ブレーク処理とEOL処理を一緒にしたためここにif文が入ってしまった) */
		if (i < endRec + 1){ /* 最終ループ(i == endRec + 1)でない時 */
			assert (i == edRecOfSamePrfx + 1);
			stRecOfSamePrfx = edRecOfSamePrfx + 1; /* stRecOfSamePrfx = i */
			oldUniChar = (unaWrdsPoolIdx[i].wordPtr)[targetPos];
		}
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  FindBase
//
// ABSTRACT:	  ベースを探す
//
// FUNCTION:
//	  最小のベース値を探す
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  EOKも文字列の一部として処理を行う
//
//  (注1)
//		-c オプション実行の場合
//		(-cオプション無しならnoUsFlg==unaFlagVal1==1でループ前初期化される
//		ので関係なし)
//		例えば unaOccupyが以下のようになっている時、既にラベルとして使用
//		されている場所も他のベースとして使用可能なはずなので unaStartBase
//		は、最初のベース候補として[4]がセットされるが[4]は絶対にベースには
//		なり得ない。なぜなら0〜codeMaxの全文字が遷移不可能だからである。
//		なお、 unaOccupy配列は 3値を取り、その値は
//		0:全くの未使用 1:ベースとして既使用 2:ラベルとして既使用 である。
//		(-cオプション無しなら、unaOccupy配列は0と1の二値である)
//		(例)
//		unaOccupy  1 1 1 1 2 1 1 1 1 1 2・・・2 2 1 1 1 1 0 1 2 1
//		                   ↑                       ↑
//		                  [4]                    [4+codeMax]
//
//		さて、[4 + codeMax + 3] = [7 + codeMax] = 0 なので、unaStartBaseは
//		最低でも [7 + codeMax - codeMax] = [7] 以上の個所である必要がある。
//		よって、unaOccupy配列の値 = 0 となる最小の要素 i を求めて
//		unaStartBase + codeMax < i なら unaStartBase をリセットするという
//		処理が必要になる。ただし、unaStartBase が示す個所が、ベース候補と
//		なり得るかは別である。
//
//  (注2)
//		-c オプション実行の場合
//		nique配列に、EOK(= 0)が含まれていた時、かつ、ベース候補が既に
//		ラベルとして使用されているところだった場合、ラベルが書き換わって
//		しまうのではないかかという恐れが考えられるが、ここでチェック
//		してあるので大丈夫。
//
//  (注3)
//		-c オプション実行の場合
//		unaOccupy配列の値が 2:ラベルとして既使用の時、その場所が他のベースに
//		なりうる場合には、unaOccupy配列の値は 1 に書きかえられる。
//		即ち unaOccupy[*r] は、0 又は 2 であったが、ここで 1 なる。
//
//  (注4)
//		ラベルを書き込む場合に unique配列に EOK(= 0) が含まれていた
//		時は *r + unique[i] は(= *r で)ベース自身を差し示すことになる。
//		よって
//		- 丁度 unaLabel[*r] = EOK(= 0) となりうまく行っている。
//		  (ただ、revez関数の中でEOK処理として再度0を書き込んでいるが)
//		- unaOccupy[*r] には、既に 1 が立ててあるし、unaOccupyCnt
//		  の中にもカウント済みである。
//
//  (注5)
//		-cオプションがない場合には、unaFlagVal2 = 1 なので
//		ベース候補にはなり得ない。
//		-cオプション実行の場合には、unaFlagVal2 = 2 なので
//		unaOccupy配列に 2 が書き込まれ、他のベース候補となり得る。
//		なお、この if のおかげでベースの所で立てた
//		unaOccupy[*r] = 1 が 2 になってしまう事はない。
//
static int FindBase(
	uintT			*r,			/* 見つかったベース値 */
	const uintT	startRec,	/* 開始レコード番号(0起算) */
	const uintT	endRec,		/* 終了レコード番号(0起算) */
	const uintT	targetPos	/* 処理対象とするカラム(0起算) */
)
{
	/* 受け取った文字列の1桁目を重複のない様に格納する配列 */
	static   unaCharT unique[MaxValueOfCode + 1]; /* ユニーク配列(static) */
	uintT	 uniqueCount;	/* 上記配列にセットされたユニークだった文字数 */
	uintT	 i;				/* ループ変数 */
	uintT	 j;				/* ループ変数 */
	unaCharT oldUniChar;	/* 1つ前の行の文字(コントロールブレーク用) */
	uintT	 findFlg;		/* unaStartBase セット用のためのフラグ
							   0:ベース候補が発見されてない 1:発見された */
	uintT	 noUsFlg;		/* unaStartBase リセット用のためのフラグ
							   0:未使用ベースが発見されてない 1:発見された */

	/* 受け取った文字列の1桁目を unique 配列にセットする。既にセット済みの
	   文字は格納しない。なお、unique 配列はソートされるはずである */
	uniqueCount = 0;
	oldUniChar = (unaWrdsPoolIdx[startRec].wordPtr)[targetPos];
	for( i = startRec ;; i ++)	{	/* forever */
		if (i > endRec) {			/* 終了 */
			unique[uniqueCount] = oldUniChar;
			uniqueCount++;			/* これがないと数が合わない */
			break;
		}
		/* コントロールブレークする毎にセット */
		if (oldUniChar != (unaWrdsPoolIdx[i].wordPtr)[targetPos]){
			unique[uniqueCount] = oldUniChar;
			oldUniChar = (unaWrdsPoolIdx[i].wordPtr)[targetPos];
			uniqueCount++;
		}
	}

	/* 一つずつベース値をずらしながら、unique 配列にセットされた文字が遷移
	   できる最小のベース値を探す */
	findFlg = 0; /* ベース候補未発見とする */
	noUsFlg = unaFlagVal1; /* -c オプション時未使用ベース未発見とする */

	for (i = unaStartBase;i < MAX_DA ;i++){ /* i がテストしているベース値 */

		/* ベース候補を見つける */
		if (findFlg == 0){ /* まだベース候補は見つかってない */
			if (unaOccupy[i] == 1) { /* ベースとして使用済み */
				continue;			 /* 読み飛ばす */
			}
			else {					 /* 未使用発見 */
				unaStartBase = i;	 /* 初めて見つかった未使用の値を保存 */
				findFlg = 1;		 /* unaStartBaseセット後は else 通る様 */
			}
		}
		else {	/* 既に未使用が発見されている時はずっとこちらを通る */
			if (unaOccupy[i] == 1) { /* ベースとして使用済み */
				continue;			 /* 読み飛ばす */
			}
		}

		/* -c オプション実行の場合(注1) */
		if (noUsFlg == 0) {
			if (unaOccupy[i] == 0) {
				if (unaStartBase + codeMax < i){
					unaStartBase = i - codeMax;
				}
				noUsFlg = 1;
			}
		}
		
		/* 見つかった未使用ベースについて unique 配列の全文字が格納可能
		   であるかをテストする */
		for ( j = 0 ; j < uniqueCount ; j++ ){
			if(i + unique[j] >= MAX_DA){ /* Revuz配列オーバーフローチェック */
				fprintf(stdout,
					"The arrays of 'Trie' is overflow.\n");
				fprintf(stdout,"Increase MAX_DA\n");
				return -24;
			}
			/* -c オプション実行の場合(注2) */
			if(unaOccupy[i + unique[j]] != 0){ /* 格納できないものあり */
				break;	/* 既に遷移先がベースかラベルになっている */
			}
		}
		if (j == uniqueCount){ /* jが最後までループした時。即ちunique配列に
							      セットされた全文字が格納可能である時 */
			*r = i;			   /* ベース発見! */
			break;
		}

	}

	/* -c オプション実行の場合(注3) */
	unaOccupy[*r] = 1;	/* ベースの場所は無条件で使用済みとする */
	unaOccupyCnt++;
	/* ラベルの場所に使用済みフラグを立て、Label値を書き込む */
	for ( i = 0 ; i < uniqueCount; i++){
		/* ラベルを書き込む場合の考察(注4) */
		unaLabel[*r + unique[i]] = unique[i];	/* Label値を書き込む */
		if(unique[i] != EOK){
			/* -c オプション実行の場合とそうでない場合の考察(注5) */
			unaOccupy[*r + unique[i]] = unaFlagVal2; /* 使用済み */
			unaOccupyCnt++;
		}

		if (unaMaxOccupy < *r + unique[i]){	/* ベース+ラベルの方がでかい時 */
			unaMaxOccupy = *r + unique[i]; /* ベース+ラベルをunaMaxOccupyに */
		}
	}

	unaBaseCount ++;		/* 見つかるたびにベース数をカウントアップ */
	if (unaMaxBase < *r){	/* 見つかったベースの方がでかい時 */
		unaMaxBase = *r;	/* 見つかったベースを unaMaxBase に */
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  WriteDict
//
// ABSTRACT:	  実行辞書の出力
//
// FUNCTION:
//	  実行辞書を出力する。
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  メモリ上にあるデータを使い実行形態素辞書、実行アプリ辞書として出力する
//
static int WriteDict(
	char *morDic,	   /* 形態素辞書 */
	char *appDic	   /* アプリ辞書 */
)
{
	FILE *fp;		   /* ファイルポインタ */
	uintT i;		   /* ループ変数 */
	uintT appPoolsiz; /* アプリ辞書情報部(unaAppInfoT型データプール)のSIZE */
	uintT daCount;	   /* 形態素辞書索引部(RevuzのTrie配列)の大きさ */

	/*
	 * データ圧縮のための前処理
	 */
	/* 見かけ上のレコード件数およびレコード番号からの対応表を計算 */ 
	/* 下位構造を扱うため、１レコードは2件あるいは1件のサイズとなる */
	uintT recCnt=0;
	for(i = 0;i < unaRecNo;i++){
		recToCrec[i] = recCnt;
		if (dicVersion == V125) {
			/* V1.25では下位構造の有無によらず1レコードは常に1件 */
			/* (したがってrecToCrec[n]は常にn) */
			recCnt++;
		} else {
			/* V1.24では下位構造があれば1レコードは2件、さもなければ1件 */
			if ( unaCMorData2[i].sub.subLstNo == 0xffffff) {
				recCnt++;
			} else {
				unaCMorData[i].mor.subFlg=1;
				recCnt++;
				recCnt++;
			}
		}
	}
	/*
	 * アプリケーション辞書出力
	 */
	if ((fp = fopen(appDic,"wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",appDic);
		return -25;
	}

	/* アプリ辞書情報部(unaAppInfoT型データプール)のサイズを計算。
	   なおRevuz関数終了後 unaRecNo は、全レコード数を示している(よって-1) */
	appPoolsiz = unaAppOffset[unaRecNo - 1] + sizeof(char *)
		+ ((unaAppInfoT *)(unaAppPool + unaAppOffset[unaRecNo - 1]))->len;

	/* 出力 */
	fwrite(unaComment,sizeof(unaComment),1,fp);			/* ヘッダー */
	fwrite(unaAppVer,sizeof(unaAppVer),1,fp);			/* ヘッダー */
	fwrite(&recCnt,sizeof(recCnt),1,fp);			/* ヘッダー */
	uintT nullpos=appPoolsiz-1;
	for(i=0;i< unaRecNo;++i){
		if (dicVersion == V124 && unaCMorData[i].mor.subFlg == 1) {
			fwrite(&unaAppOffset[i],sizeof(uintT),1,fp);	/* 索引部 */
			fwrite(&nullpos,sizeof(uintT),1,fp);	/* 索引部 */
		}
		else{
			fwrite(&unaAppOffset[i],sizeof(uintT),1,fp);	/* 索引部 */
		}
	}
	fwrite(unaAppPool,appPoolsiz,1,fp);					/* 情報部 */

	fclose(fp);

	/*
	 * 形態素辞書出力
	 */
	if ((fp = fopen(morDic,"wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n\n",morDic);
		return -26;
	}

	/* 形態素辞書索引部(RevuzのTrie配列)の大きさを以下のでかい方にする */
	assert(codeMax < MaxValueOfLabelInDic);
	if (unaMaxBase + codeMax + 1 > unaMaxOccupy + 1){
		daCount = unaMaxBase + codeMax + 1;
	}
	else{
		daCount = unaMaxOccupy + 1;
	}

	/* 出力 */
	fwrite(unaComment,sizeof(unaComment),1,fp);			 /* ヘッダー */
	fwrite(unaDicVer,sizeof(unaDicVer),1,fp);			 /* ヘッダー */
	fwrite(&recCnt,sizeof(recCnt),1,fp);			 /* ヘッダー */
	fwrite(&unaLstNo,sizeof(unaLstNo),1,fp);			 /* ヘッダー */
	fwrite(&daCount,sizeof(daCount),1,fp);				 /* ヘッダー */
	for(i = 0;i < 0x10000;i++){
		ushortT x;
		if ( codeCnvTable[i] == MaxValueOfCode){
			x = codeMax+1; // labelとしてはありえない値
		}
		else{
			x = codeCnvTable[i];
		}
		fwrite(&x,sizeof(ushortT),1,fp); /* 文字変換テーブル */
	}

	for(i=0;i<daCount;++i){
		if ( unaLabel[i] == EOK){
			assert(unaBase[i] <= MaxValueOfBaseInDic);
			unaBase[i] = recToCrec[ unaBase[i] ];

		}
		if ( unaBase[i] == ClearValueOfBase){
			unaBase[i] = MaxValueOfBaseInDic;
		}
		if ( unaLabel[i] == MaxValueOfCode){
			unaLabel[i] = codeMax;
		}

		// assert(unaBase[i] <= MaxValueOfBaseInDic);
		// assert(unaLabel[i] < 0x2000);
		if (unaBase[i] > MaxValueOfBaseInDic) {
			fprintf(stdout, "Value of unaBase element exceeds limit.\n");
			fclose(fp);
			return -27;
		}
		if (unaLabel[i] >= 0x2000) {
			fprintf(stdout, "Value of unaLabel element exceeds limit.\n");
			fclose(fp);
			return -28;
		}
	}

	if (dicVersion == V125) {
		/* V1.25のダブル配列(ベース32ビット, ラベル16ビット) */
		fwrite(unaBase,sizeof(uintT),daCount,fp);			 /* 索引部 BASE */
		fwrite(unaLabel,sizeof(unaCharT),daCount,fp);		 /* 索引部 LABEL */
	} else {
		/* V1.24のダブル配列(ベース19ビット, ラベル13ビット) */
		baseAndLabelT wdata;
		for(i=0;i<daCount;++i){
			wdata.label = unaLabel[i];
			wdata.base  = unaBase[i];
			fwrite(&wdata,sizeof(baseAndLabelT),1,fp);		 /* 索引部 LABEL&BASE */
		}
	}

	/* 下位構造リストを、レコード番号のずれを補正して格納　*/
	for(i = 0;i < unaLstNo;i++){
		if ( unaSubLst[i]>=0x80000000){ /* 格納数 */
			unaSubLst[i] &= 0x7fffffff;
		}
		else{
			assert(unaSubLst[i] != 0xffffff);
			unaSubLst[i] = recToCrec[ unaSubLst[i] ];
		}
		fwrite(&unaSubLst[i],sizeof(uintT),1,fp);	/* 下位構造リスト部 */
	}

	/* 情報部のレコード配列を格納 */
	if (dicVersion == V125) {
		for (i = 0; i < unaRecNo; i++) {
			fwrite(&unaMorData[i],sizeof(unaMorDataT),1,fp); /* 情報部 */
		}
	} else {
		// ここでBASEに格納されたレコード番号と、実際のレコード配列の対応がずれる*/
		for(i = 0;i < unaRecNo;i++){
			if ( unaCMorData2[i].sub.subLstNo == 0xffffff){
				fwrite(&unaCMorData[i],sizeof(unaCompactMorDataT),1,fp); /* 情報部 */
			}
			else{
				fwrite(&unaCMorData[i],sizeof(unaCompactMorDataT),1,fp); /* 情報部 */
				fwrite(&unaCMorData2[i],sizeof(unaCompactMorDataT),1,fp); /* 情報部 */
			}
		}
	}
	fclose(fp);

	return UNA_OK;
}
//--------------------------------------------------------------------------
// MODULE:        WriteDictNull
//
// ABSTRACT:      Nullな実行辞書の出力
//
// FUNCTION:
//        実行辞書(カラ)を出力する。
//
// RETURN:
//              UNA_OK                                  正常終了
//              その他                                  エラー
//
// NOTE:
//
static int WriteDictNull(
	char *morDic,      /* 形態素辞書 */
	char *appDic      /* アプリ辞書 */
)
{
	FILE *fp;                  /* ファイルポインタ */
	uintT i;                  /* ループ変数 */
	uintT daCount;    /* 形態素辞書索引部(RevuzのTrie配列)の大きさ */
	uintT recCnt=0;
    codeMax = 0;
	unaMaxBase = 0;

	/*
	 * アプリケーション辞書出力
	 */
	if ((fp = fopen(appDic,"wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",appDic);
		return -25;
	}

	/* 出力 */
	recCnt = 0;
	fwrite(unaComment,sizeof(unaComment),1,fp);         /* ヘッダー */
	fwrite(unaAppVer,sizeof(unaAppVer),1,fp);           /* ヘッダー */
	fwrite(&recCnt,sizeof(recCnt),1,fp);            /* ヘッダー */
	fclose(fp);

	/*
	 * 形態素辞書出力
	 */
	if ((fp = fopen(morDic,"wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n\n",morDic);
		return -26;
	}

	/* 形態素辞書索引部(RevuzのTrie配列)の大きさを以下のでかい方にする */
	assert(codeMax < MaxValueOfLabelInDic);
	daCount = unaMaxBase + codeMax + 2; // daCount = 1;

	/* 出力 */
	recCnt = 0;
	unaLstNo = 0;
	fwrite(unaComment,sizeof(unaComment),1,fp);          /* ヘッダー */
	fwrite(unaDicVer,sizeof(unaDicVer),1,fp);            /* ヘッダー */
	fwrite(&recCnt,sizeof(recCnt),1,fp);             /* ヘッダー */
	fwrite(&unaLstNo,sizeof(unaLstNo),1,fp);             /* ヘッダー */
	fwrite(&daCount,sizeof(daCount),1,fp);               /* ヘッダー */
	for(i = 0;i < 0x10000;i++){
		ushortT x;
		x = codeMax +1; // labelとしてはありえない値
		fwrite(&x,sizeof(ushortT),1,fp); /* 文字変換テーブル */
	}
	if (dicVersion == V125) {
		/* V1.25のダブル配列(ベース32ビット, ラベル16ビット) */
		for(i=0;i<daCount;++i){
			unaBase[i] = MaxValueOfBaseInDic;
			unaLabel[i] = codeMax;
		}
		fwrite(unaBase,sizeof(uintT),daCount,fp);			 /* 索引部 BASE */
		fwrite(unaLabel,sizeof(unaCharT),daCount,fp);		 /* 索引部 LABEL */
	} else {
		/* V1.24のダブル配列(ベース19ビット, ラベル13ビット) */
		for(i=0;i<daCount;++i){
			baseAndLabelT wdata;
			unaBase[i] = MaxValueOfBaseInDic;
			unaLabel[i] = codeMax;
			wdata.label = unaLabel[i];
			wdata.base  = unaBase[i];
			fwrite(&wdata,sizeof(baseAndLabelT),1,fp);       /* 索引部 LABEL&BASE */
		}
	}

	fclose(fp);

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  CodeComp
//
// ABSTRACT:	  比較関数
//
// FUNCTION:
//	  文字コード並び替えのための比較関数
//
// RETURN:
//	  負値		第1引数が第2引数より小さい
//	  0		第1引数が第2引数と等しい
//	  正値		第1引数が第2引数より大きい
//
// NOTE:
//	  qsort の比較関数に準じる
//
static int CodeComp(
	int *c1,	/* UNA品詞リスト */
	int *c2	/* UNA品詞リスト */
	)
{
	return (c2[1] - c1[1]);
}
//--------------------------------------------------------------------------
// MODULE:	  LineComp
//
// ABSTRACT:	  比較関数
//
// FUNCTION:
//	  bsearch のための比較関数
//
// RETURN:
//	  負値		第1引数が第2引数より小さい
//	  0			第1引数が第2引数と等しい
//	  正値		第1引数が第2引数より大きい
//
// NOTE:
//	  qsort の比較関数に準じる
//
static int LineComp(
	hinLstIdxT *w1,	/* UNA品詞リスト */
	hinLstIdxT *w2	/* UNA品詞リスト */
	)
{
	int i;			/* ループ変数 */
	int n;			/* 比較する長さ */
	unaCharT *s1;	/* UNICODE文字列 */
	unaCharT *s2;	/* UNICODE文字列 */

	s1 = w1->ptr;
	s2 = w2->ptr;
	
	/* 比較する長さを決定する */
	if(w1->len > w2->len){
		n = w2->len;
	}
	else{
		n = w1->len;
	}

	/* unsigned short を順番に比較 */
	for ( i = 0;i < n;i++ ){
		if ( s1[i] < s2[i] ){
			return -1;
		}
		else if ( s1[i] > s2[i] ){
			return 1;
		}
	}

	/* 比較して等しかった場合には長さで判断 */
	if(w1->len > w2->len){
		return 1;
	}
	else if (w1->len < w2->len){
		return -1;
	}

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  MorIDComp
//
// ABSTRACT:	  比較関数
//
// FUNCTION:
//	  una_msort のための比較関数
//
// RETURN:
//	  負値		第1引数が第2引数より小さい
//	  0			第1引数が第2引数と等しい
//	  正値		第1引数が第2引数より大きい
//
// NOTE:
//	  qsort の比較関数に準じる
//
static int MorIDComp(
	cvtMorIDTblT *w1,	/* 形態素IDとレコード番号を関連付けるテーブル */
	cvtMorIDTblT *w2	/* 形態素IDとレコード番号を関連付けるテーブル */
)
{

	if(w1->morphID > w2->morphID){
		return 1;
	}
	else if (w1->morphID < w2->morphID){
		return -1;
	}

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  TestRevuz
//
// ABSTRACT:	  評価関数
//
// FUNCTION:
//	  作成した辞書情報を標準出力に表示する
//	  又、辞書がちゃんと登録できたかテストする
//
// RETURN:
//	  UNA_OK	正常
//	  それ以外	エラー
//
// NOTE:
//	  標準入力から文字列を受け取り、それに対応する品詞番号を表示する
//	  文字列に部分文字列があれば(但し1桁目開始の)その部分文字列についても
//	  表示する
//
static int TestRevuz(
	const uintT startR			    /* 最初のベース値 */
	)
{
	double occupyRitu;	/* Revuz配列での配列の使用率(小さいとスパース) */
	fprintf(stdout,"Processed records count is %u\n",unaRecNo);
	fprintf(stdout,"LOCAL_MAX_WORDS_POOL must be more than %lu MOJIs\n",
	  ((uintptr_t)(unaWrdsPoolIdx[unaRecNo - 1].wordPtr) +
	  (unaWrdsPoolIdx[unaRecNo - 1].wordLen + 1) * sizeof(unaCharT) -
	  (uintptr_t)(unaWrdsPoolIdx[0].wordPtr)) / sizeof(unaCharT));
	fprintf(stdout,
	  "The used element of Revuz' arrays(unaOccupyCnt) is %u\n",unaOccupyCnt);
	fprintf(stdout,"BASE count(unaBaseCount) is %u\n",unaBaseCount);
	fprintf(stdout,"LABEL count(unaOccupyCnt - unaBaseCount) is %u\n",
		unaOccupyCnt - unaBaseCount);
	fprintf(stdout,
	  "The value of unaMaxBase is [%u].The expected largest SEN-I is [%u]\n",
	  unaMaxBase,unaMaxBase + 0xffff);
	fprintf(stdout,"The value of unaMaxOccupy is [%u]\n",unaMaxOccupy);
	fprintf(stdout,"so MAX_DA must be more than %u or %u.(big one!)\n",
	  unaMaxBase + 0xffff + 1,unaMaxOccupy + 1);
	occupyRitu = unaOccupyCnt/(double)(unaMaxBase + 0xffff + 1)*100;
	fprintf(stdout,
	  "Occupy array(Big dictionary) is %3.2f%% used\n",occupyRitu);
	occupyRitu = unaOccupyCnt/(double)(unaMaxOccupy + 1)*100;
	fprintf(stdout,
	  "Occupy array(Small dictionary)is %3.2f%% used\n",occupyRitu);
	fprintf(stdout,"Start value of BASE is %u\n",startR);

	return UNA_OK;
}



//--------------------------------------------------------------------------
// Copyright (c) 2000-2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
