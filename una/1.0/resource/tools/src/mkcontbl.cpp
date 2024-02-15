//
// mkcontbl.cpp -
//      実行接続表作成モジュール
//		実行接続表を作成するモジュール
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
#include <stdio.h>		/* fprintf */
#include <string.h>		/* strcmp */
#include <stdlib.h>		/* malloc,free */
#include <sys/stat.h>	        /* stat */
#include "una.h"		/* una グローバルなヘッダファイル */
#include "unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define  KEY_TOKEN_CNT       1  /* キーとなるべきトークン数  */
#define  Q_COST_MAX        255  /* コストの量子化最大値      */

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
	ushortT	 morHin;			/* 形態素品詞番号     */
	unaHinT	 unaHin;			/* UNA品詞番号        */
}hinLstIdxT;

//--------------------------------------------------------------------------
// TAG:	  connectIdxT
//
// ABSTRACT:	  接続表ソース型
//
// NOTE:
//	  接続表ソースの1レコードを表わす。但し、品詞名は形態素品詞番号に変換され
//	  格納される
//
typedef struct connectIdxT{		// 接続表ソース型
	ushortT	 morHin1;			/* 形態素品詞番号1 */
	ushortT	 morHin2;			/* 形態素品詞番号2 */
	ucharT	 cost;				/* 接続コスト */
}connectIdxT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int GetOptions(int argc,char *argv[],char *comment,
			int *processedArgc);
static void OutputUsage(void);
static int OptionA(int argc,char *argv[],char *comment,
			int *processedArgc);
static int GetCostMax(double *costMax,char *costMaxFileName);
static int ReadHinLst(char *hinLst);
static int ReadConSrc(char *conSrc,int patchRecCnt);
static int WriteFile(char *contbl,int conSrcCnt,int hinLstCnt,int mkcount,int mucount);
static void ReplaceSymbols(unaCharT *pos, int len);
static int GetMorHin(unaCharT *iStr,unaStrIndexT *strIdx,int hinLstCnt);
static void PrintUcsToUtf8(unaCharT *ptr,uintT len);
#if 0
static void PrintUcsToHex(unaCharT *ptr,uintT len);
#endif
static int GetCost(unaCharT *iStr,unaStrIndexT *strIdx);
static int LineComp(hinLstIdxT *w1,hinLstIdxT *w2);
static int MorNoComp(hinLstIdxT *w1,hinLstIdxT *w2);
static int MorNoComp2(connectIdxT *w1,connectIdxT *w2);
static int SortGyoCost(int conSrcCnt,int hinLstCnt,int mkcount);
static int SortRetsuCost(int conSrcCnt,int hinLstCnt,int mkcount,int mucount);
void swap(ucharT *ptr1,ucharT *ptr2); 

//--------------------------------------------------------------------------
// グローバル変数
//--------------------------------------------------------------------------
static double		unaCostMax;		  /* コストの最大値(量子化に必要) */
static hinLstIdxT	*unaHinLst;		  /* UNA品詞リスト */
static unaCharT		*unaHinPool;	  /* UNA品詞リスト用文字列プール */
static connectIdxT	*unaConSrc;		  /* 接続表用データ */
static connectIdxT	*unaConSrcOrg;		  /* 接続表用データ */
static char unaComment[UNA_COM_SIZE];             /* テーブルに付けるコメント */
static char unaConVer[UNA_VER_SIZE] = UNA_CON_VER; /* テーブルのバージョン*/
static ushortT          *lineK;                   /* 行に関する元の行数*/
static ushortT          *lineU;                   /* 列に関する元の行数*/

//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	  実行接続表を作成する
//
// FUNCTION:
//	  パッチ済みUNA用接続表ソース及びパッチ済みUNA品詞リストから、
//    実行接続表を作成する。
//	  使い方は、コマンド行から次の通りに入力する。
//		mkcontbl.exe コストマックスファイル UNA品詞リスト UNA接続表ソース
//																実行接続表
//
//	  例:
//		mkcontbl -a "For Ihyouki" costmax.log unahin.lst unacon.src
//																connect.tbl
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
//
// NOTE:
//    なし
//
int main(
	 int argc,			/* コマンド行の引数の数(1以上) */
	 char *argv[]		        /* コマンド行の引数 */
 )
{
	int rv;			/* 関数の返り値 */
	int processedArgc;	        /* 処理した引数の数 */
	char *hinLst;		        /* UNA品詞リスト */
	char *conSrc;		        /* 接続表ソース */
	char *conTbl;		        /* 接続表 */
	int hinLstCnt;		        /* UNA品詞リストのレコード件数 */
	int conSrcCnt;		        /* 接続表ソースのレコード件数 */
#ifdef UNASORT_DEBUG
	int i;
#endif
        int  mk_count = 0;              /* 行方向でのマージ行数 */
        int  mu_count = 0;              /* 列方向でのマージ行数 */
        
	/* コマンドラインからオプションを得る */
	rv = GetOptions(argc,argv,unaComment,&processedArgc);
	if (rv < 0){
  		return 1;
	}

	/* 残りの引数のチェック */
	if (argc - processedArgc != 4) {	/* 残りの引数の数がおかしい */
		fprintf(stdout,"Argument Err\n");
		return 2;
	}

	rv = GetCostMax(&unaCostMax,argv[processedArgc]);
	if (rv < 0) {
		return 3;
	}
	hinLst		= argv[processedArgc + 1];
	conSrc		= argv[processedArgc + 2];
	conTbl		= argv[processedArgc + 3];

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

	/* 接続表ソースのメモリへの格納 */
	rv = ReadConSrc(conSrc,hinLstCnt);
	if (rv < 0){
		return 5;
	}
	conSrcCnt = rv;		/* 返り値は件数 */

	if(hinLstCnt*hinLstCnt != conSrcCnt){
		fprintf(stdout,
			"UNA HINSHI LIST count(%d)^2 not equal ConSrc count(%d)\n",			                                                                                hinLstCnt,conSrcCnt);
		return 6;
	}

	/* UNA品詞リストのマージソート(キー:形態素品詞番号) */
	una_msort(unaHinLst,0,hinLstCnt - 1,sizeof(hinLstIdxT),(unaCompFuncT)MorNoComp,NULL);

	/* 接続表ソースのマージソート(キー:形態素品詞番号1、形態素品詞番号2) */
	una_msort(unaConSrc,0,conSrcCnt - 1,sizeof(connectIdxT),(unaCompFuncT)MorNoComp2,NULL);
	
	memcpy(unaConSrcOrg, unaConSrc, sizeof(connectIdxT)*hinLstCnt*hinLstCnt);
        /* 接続表の圧縮 */
        /* 行方向にマージソート */
        rv = SortGyoCost(conSrcCnt,hinLstCnt,mk_count);
	if (rv < 0){
		return 7;
	}
	mk_count = rv;		/* 返り値は件数 */

        /* 列方向にマージソート */
        rv = SortRetsuCost(conSrcCnt,hinLstCnt,mk_count,mu_count);
	if (rv < 0){
		return 8;
	}
	mu_count = rv;		/* 返り値は件数 */

	int i,j;
	for (i = 0;i < hinLstCnt; i++){
		for (j = 0;j < hinLstCnt;j++){
			if ( unaConSrcOrg[i*hinLstCnt+j].cost !=
				unaConSrc[lineK[i]*hinLstCnt+lineU[j]].cost){
				fprintf(stdout,"something is wrong!!\n");
				exit(1);
			}
		}
	}
        
	/* ファイルの書き出し */
	rv = WriteFile(conTbl,conSrcCnt,hinLstCnt,mk_count,mu_count);
	if (rv < 0){
		return 9;
	}

	/* メモリの free */
	free(unaHinPool);
	free(unaHinLst);
	free(unaConSrc);
	free(unaConSrcOrg);
	free(lineK);
	free(lineU);
        
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
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{
	int rv;			/* 関数の返り値 */

	/* argc の数だけループ。第1引数にはプログラム名が入っているから1より */
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
	fprintf(stdout,"mkcontbl (Ver1.18)\n");
	fprintf(stdout,"Usage: mkcontbl [OPTION] COST_MAX_FIL UNA_HIN_LST");
	fprintf(stdout," CON_SRC CON_TBL\n");
	fprintf(stdout,"  OPTION\n");
	fprintf(stdout,"    -h	           Help\n");
	fprintf(stdout,
		"    -a ANNOTATION  The String to annotate(Max 48byte)\n");
	fprintf(stdout,"Ex   : mkcontbl -a \"For Ihyouki\"");
	fprintf(stdout," costmax.log unahin.lst unacon.src connect.tbl\n");
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
	char *hinLst
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
	unaCharT	 strUnaHin[12];			/* UNA品詞番号(文字表現) */
	unaCharT	 *p;

	/* 対象とするファイルのサイズをチェック */
	if(stat(hinLst,&stbuf)==-1){
		fprintf(stdout, "Can't get file info %s\n", hinLst);
		return -3;
	}

	/* まず最初分を確保 */
	unaHinLst= (hinLstIdxT *)malloc(LINES_BLOCK * sizeof(hinLstIdxT));
	/* unaHinPoolは一度に全部を確保(unaHinPool < stbuf.st_sizeで足りるはず)*/
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

		/* コメント行は読み飛ばし */
		for (p = unaHinPool+hinPoolEnd; *p == 0x0020 || *p == 0x0009; p++) {
			;
		}
		if (*p == 0x0023) { /* #(シャープ)で始まってる行 */
			continue;
		}

		/* トークンに分割する */
		tokenCnt = una_getTokens(unaHinPool+hinPoolEnd,MAX_TOKEN_CNT,token);

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

		/* 文字型UNA品詞番号(第3トークン)を数値に変換して格納 */
		memcpy(strUnaHin,unaHinPool+hinPoolEnd+token[2].pos,
						token[2].len * sizeof(unaCharT));
		strUnaHin[token[2].len] = 0x0000;	/* una_xtol のためターミネート */
		unaHinLst[lineEnd].unaHin = (unaHinT)una_xtol(strUnaHin);
		
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
			unaHinLst= (hinLstIdxT *)realloc(unaHinLst,
											lineMax * sizeof(hinLstIdxT));
		}
	}
	fclose(fp);

	return lineEnd;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadConSrc
//
// ABSTRACT:    UNA用接続表ソースを読み込む
//
// FUNCTION:
//	  UNA用接続表ソースの全レコードを読み込みダイナミックメモリに格納する
//
// RETURN:
//	  負の値	エラー
//	  それ以外	読み込んだレコード数
//
// NOTE:
//	  品詞名は、形態素品詞番号に変換され格納される
//
static int ReadConSrc(
	char			*conSrc,				/* 接続表ソース */
	int			hinLstCnt				/* UNA品詞リストのレコード数 */
)
{
	int			conSrcCnt;		/* 計算上の接続表ソースのレコード数 */
	int			i;						/* レコードの件数を数える */
	int			rv;						/* 関数の返り値 */
	FILE			*fi;					/* ファイルポインタ */
	unaCharT		iStr[MAX_LINE_LEN];		/* 読み込まれた文字列 */
	int			iStrLen;				/* 読み込まれた文字列数 */
	unaStrIndexT	token[MAX_TOKEN_CNT];	/* トークン */
	int			tokenCnt;				/* トークンの数 */

	/* 計算上の接続表ソースのレコード数を求める */
	conSrcCnt = hinLstCnt * hinLstCnt;

	/* メモリの確保 */
	unaConSrc = (connectIdxT *)malloc(sizeof(connectIdxT) * conSrcCnt);
	unaConSrcOrg = (connectIdxT *)malloc(sizeof(connectIdxT) * conSrcCnt);

	/* 接続表ソースのオープン */
	if ((fi = fopen(conSrc,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",conSrc);
		return -7;
	}

	for(i = 0;;i++){ /* forever */

		/* 接続表ソースを読み込む */
		iStrLen = una_fgetws(iStr,MAX_LINE_LEN,fi);
		if (iStrLen < 0){					/* ファイルエンド */
			break;
		}

		/* 読み込んだ1行の長さチェック */
		if (iStrLen >= MAX_LINE_LEN - 1) { /* 行長オーバーフローの可能性有*/
			fprintf(stdout,"Possibly the line buffer of ConSrc overflow\n");
			return -8;
		}

		/* 件数チェック */
		if (i > conSrcCnt) {
			fprintf(stdout,"The count of ConSrc is overflow\n");
			return -9;
		}
			
		/* トークンに分割する */
		tokenCnt = una_getTokens(iStr,MAX_TOKEN_CNT,token);
		if (tokenCnt < 4) {
			fprintf(stdout,"%d is insufficient token count for %s\n",
				tokenCnt,conSrc);
			return -10;
		}

		/* 品詞名の～, －を0xFF5E, 0xFF0Dから0x301C, 0x2212に置き換える */
		ReplaceSymbols(iStr + token[0].pos, token[0].len);
		ReplaceSymbols(iStr + token[1].pos, token[1].len);
		
		/* 品詞名1(第1トークン)を形態素品詞番号に変換 */
		rv = GetMorHin(iStr,&token[0],hinLstCnt);
		if (rv < 0){
			return rv;
		}
		unaConSrc[i].morHin1 = (ushortT)rv;		/* 返り値は morHinNo */

		/* 品詞名2(第2トークン)を形態素品詞番号に変換 */
		rv = GetMorHin(iStr,&token[1],hinLstCnt);
		if (rv < 0){
			return rv;
		}
		unaConSrc[i].morHin2 = (ushortT)rv;		/* 返り値は morHinNo */

		/* コスト(第4トークン)を量子化し取得 */
		rv = GetCost(iStr,&token[3]);
		if (rv < 0){
			return rv;
		}
		unaConSrc[i].cost = (ucharT)rv;
	}

	fclose(fi);

	return i;
}

//--------------------------------------------------------------------------
// MODULE:	  WriteFile
//
// ABSTRACT:    接続表の出力
//
// FUNCTION:
//	  接続表を出力する
//
// RETURN:
//	  UNA_OK	正常
//	  それ以外	エラー
//
// NOTE:
//	  ・接続コストテーブル部を接続表ソースの情報を圧縮した情報に対して、
//          係りコード受けコードを導入した形式で出力する。
//
static int WriteFile(
	char *conTbl,			/* 接続表 */
	int conSrcCnt,			/* 接続表ソースのレコード数 */
	int hinLstCnt,			/* UNA品詞リストのレコード数 */
	int mk_count,                  /* 行方向でのマージ行数 */
	int mu_count                   /* 列方向でのマージ行数 */
)
{
	int	       	i;	     /* ループ変数 */
	FILE       	*fo;	     /* ファイルポインタ */
	unaHinT    	unaHin;	     /* UNA品詞番号(=0)ダミーデータ用 */
	int         kCnt;        /* 係りコード数*/
	int         uCnt;        /* 受けコード数*/
	int		parity;	     /* ４バイトごとのデータ切れ目に調整用 */
	int		parityK;     /* ４バイトごとのデータ切れ目に調整用 */
	int		parityU;     /* ４バイトごとのデータ切れ目に調整用 */
	ushortT		code;        /* 係り受けコード(=9999)ダミーデータ用*/
	ucharT		cost;	     /* コスト(=0)ダミーデータ用 */
	int		pos;         /* 品詞名へのポインタをオフセットに変換 */
	unaCharT*	hinPoolEnd;  /* 品詞名プールの末尾 */
	unaCharT*	curPos;      /* 品詞名書き換え用 */

	/* 接続表オープン */
	if ((fo = fopen(conTbl, "wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",conTbl);
		return -11;
	}

	/* データの書き込み */
	fwrite(unaComment,sizeof(unaComment),1,fo);	 /* コメント */
	fwrite(unaConVer,sizeof(unaConVer),1,fo);	 /* バージョン */

        /* 品詞数の奇遇チェック */
	parity = (hinLstCnt + 1) % 2;        
        /* 係り方向の奇遇チェック */
        kCnt = hinLstCnt - mk_count;
        parityK = (kCnt) % 2;
        /* 受け方向の奇遇チェック */
        uCnt = hinLstCnt - mu_count;
        parityU = (uCnt) % 2;
        
	hinLstCnt++;		  /* 要素番号 0 のダミーデータの分をカウント */ 
	hinLstCnt += parity;
	fwrite(&hinLstCnt,sizeof(hinLstCnt),1,fo);	/* 品詞数の出力 */
	hinLstCnt -= parity;
	hinLstCnt--;

	/* UNA品詞への変換テーブル(形態素品詞番号でソート済み) */
	unaHin	= 0;
	fwrite(&unaHin,sizeof(ushortT),1,fo);		/* ダミーデータ */
	for (i = 0; i < hinLstCnt; i++){
		fwrite(&(unaHinLst[i].unaHin),sizeof(ushortT),1,fo);
	}
	if (parity){
		fwrite(&unaHin,sizeof(ushortT),1,fo);	      /* ダミーデータ */
	}
        /* 係りコード数*/
	kCnt += parityK;
        fwrite(&kCnt,sizeof(kCnt),1,fo);
	kCnt -= parityK;

        /* 受けコード数 */
	uCnt += parityU;
        fwrite(&uCnt,sizeof(uCnt),1,fo);
	uCnt -= parityU;
        
        /* 係りコードの出力 */
        code = 9999;
	fwrite(&code,sizeof(ushortT),1,fo);		      /* ダミーデータ */
        for(i = 0; i < hinLstCnt; i++){
            fwrite(&lineK[i],sizeof(ushortT),1,fo);
        }
	if (parity){
            fwrite(&code,sizeof(ushortT),1,fo);	              /* ダミーデータ */
	}
        
        /* 受けコードの出力 */
	fwrite(&code,sizeof(ushortT),1,fo);		      /* ダミーデータ */
        for(i = 0; i < hinLstCnt; i++){
            fwrite(&lineU[i],sizeof(ushortT),1,fo);
        }
	if (parity){
            fwrite(&code,sizeof(ushortT),1,fo);	              /* ダミーデータ */
	}
       
	/* 圧縮版接続コストテーブル */
	cost = 0;
	int kCode;
	int uCode;
	for(kCode = 0; kCode < kCnt; kCode++){
            for(uCode = 0; uCode < uCnt; uCode++){
                fwrite( &(unaConSrc[kCode * hinLstCnt + uCode].cost),
			  sizeof(ucharT),1,fo);
            }
            if(parityU){
                fwrite(&cost,sizeof(ucharT),1,fo);       /* ダミーデータ */
            }
        }
        if (parityK){
            for(uCode = 0; uCode < uCnt+parityU; uCode++){
                fwrite(&cost,sizeof(ucharT),1,fo);    /* ダミーデータ */
            } 
        }

	/* 品詞名プールの末尾を求める */
	hinPoolEnd = 0;
	for (i = 0; i < hinLstCnt; i++){
            if ( hinPoolEnd < unaHinLst[i].ptr+unaHinLst[i].len){
                hinPoolEnd = unaHinLst[i].ptr+unaHinLst[i].len;
            }
	}

	/* 品詞名リストの書き出し */
	pos = hinPoolEnd - unaHinPool; /* hinPoolEndには必ず 0 が入っている */
	fwrite(&pos,sizeof(pos),1,fo);		/* ダミーデータ */
	for(i = 0; i < hinLstCnt; i++){
            pos = unaHinLst[i].ptr - unaHinPool;
            fwrite(&pos,sizeof(pos),1,fo);
	}
	if(parity){
            pos = hinPoolEnd - unaHinPool;
            fwrite(&pos,sizeof(pos),1,fo);		/* ダミーデータ */
	}
	/* unicode変換に関する既知の問題対応(チルダがwave dashになってしまう)*/
	for(curPos = unaHinPool; curPos < hinPoolEnd; ++curPos){
            if(*curPos == 0x301c ){ /* wave dash */
                *curPos = 0xff5e;
            }
	}
	/* 品詞名プールの書き出し */
	fwrite(unaHinPool,sizeof(unaCharT)*(hinPoolEnd-unaHinPool+1),1,fo);

	fclose(fo);

	return UNA_OK;
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
//
static int GetMorHin(
	unaCharT *iStr,						/* 品詞名の入った文字列 */
	unaStrIndexT *strIdx,				/* オフセット、長さ */
	int hinLstCnt						/* UNA品詞リストの件数 */
)
{
	hinLstIdxT	keyIdx;					/* 探したいキーをセット */
	hinLstIdxT	*ansIdx;				/* マッチした場合の答えが返る */

	keyIdx.ptr = iStr + strIdx->pos;	/* オフセットを足してセット */
	keyIdx.len = strIdx->len;			/* キー長さをセット */
	keyIdx.morHin = 0;					/* 未使用 */
	keyIdx.unaHin = 0;					/* 未使用 */
	ansIdx = (hinLstIdxT *)bsearch(&keyIdx,unaHinLst,
				hinLstCnt,sizeof(hinLstIdxT),(unaCompFuncT)LineComp);

	if(ansIdx == NULL){
		fprintf(stdout,"No match data in UNA HINSHI LIST(Key=");
		PrintUcsToUtf8(keyIdx.ptr,keyIdx.len);
		fprintf(stdout,")\n");
		return -12;
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
// MODULE:	  GetCost
//
// ABSTRACT:	  コストの取得
//
// FUNCTION:
//	  コストを計算して返す
//
// RETURN:
//	  負の値	エラー
//	  それ以外	コスト(0-254,255)
//
// NOTE:
//	  255は特別な値で、接続不可を表わす。
//	  なお、接続値255のものはコスト計算時に65535にマッピングされる。
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
		fprintf(stdout,"The length of String ConCost is too int\n");
		return -10;
	}

	memcpy(uCost,iStr + strIdx->pos,strIdx->len * sizeof(unaCharT));
	uCost[strIdx->len]=0x0000;

	dCost = una_utof(uCost);

	/* 接続値を 0  Q_COST_MAX - 1 又は 255(接続不可)に量子化 */
	if (dCost < 0){
		cCost = 255; /* 接続不可。接続コスト計算時65535にマッピングされる */
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
// MODULE:	  MorNoComp
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
static int MorNoComp(
	hinLstIdxT *w1,	/* UNA品詞リスト */
	hinLstIdxT *w2	/* UNA品詞リスト */
	)
{

	if(w1->morHin > w2->morHin){
		return 1;
	}
	else if (w1->morHin < w2->morHin){
		return -1;
	}

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  MorNoComp2
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
static int MorNoComp2(
	connectIdxT *w1,	/* 接続表ソース */
	connectIdxT *w2		/* 接続表ソース */
	)
{

	if(w1->morHin1 > w2->morHin1){
		return 1;
	}
	else if (w1->morHin1 < w2->morHin1){
		return -1;
	}

	/* morHin1 が等しかった時、morHin2のチェック */
	if(w1->morHin2 > w2->morHin2){
		return 1;
	}
	else if (w1->morHin2 < w2->morHin2){
		return -1;
	}

	return 0;
}

//--------------------------------------------------------------------------
// MODULE:	  SortGyoCost
//
// ABSTRACT:	  比較関数
//
// FUNCTION:
//	  中間接続表のコストをソートするための比較関数
//
// RETURN:
//	  mk_count        行方向でのマージ行数
//
// NOTE:
//	  係り側形態素品詞番号k(k=0〜hinLstCnt-1)に対してlineK[k]で参照される、
//	  中間接続表の疑似2次元配列unaConSrcを、係り側形態素品詞番号(行方向)に
//	  ついてバブルソートし、隣り合う行が等価であれば行をマージして、後続の
//	  行をシフトして詰める。2次元配列を直接操作するのはコストがかかるため、
//	  実際の交換とシフトは間接参照配列lineIdx[]に対して行い、ソート終了後に
//	  元の2次元配列unaConSrcに結果を反映する。
//
//	  lineK[k]の内容はソートとマージを反映するよう更新され、lineK[k]経由で
//	  参照される受け側形態素品詞(0〜hinListCnt-1)との接続コストは、処理の
//	  前後で変化しない。
//
//	  unaConSrcの要素のうち実際にソートされるのはcostだけで、morHin1および
//	  morHin2は放置されるが、monHin1とmorHin2はSortGyoCostの前に実行される
//	  una_msort()で参照されるだけであり、これ以降は使われないため問題ない。
//
static int SortGyoCost(
        int conSrcCnt,		        /* 接続表ソースのレコード件数 */
        int hinLstCnt,		        /* UNA品詞リストの件数 */
        int  mk_count                   /* 行方向でのマージ行数 */
    )
{
    int        i;              /* ループ変数 */
    int        j;              /* ループ変数 */
    int        k;              /* ループ変数 */
    int        pass;           /* ループ変数 */
	int        i0;             /* lineIdx[i]の値 */
	int        i1;             /* lineIdx[i+1]の値 */
	ushortT    *lineIdx;       /* unaConSrc[]の行位置はlineIdx[]経由で参照する */
	connectIdxT	*unaConSrcTmp; /* 接続表用データコピー用作業領域 */
    
    /* 係りコードのメモリを確保 */
    lineK = ( ushortT *)malloc(sizeof(ushortT) * hinLstCnt);
	/* 行位置間接参照配列のメモリを確保 */
	lineIdx = ( ushortT *)malloc(sizeof(ushortT) * hinLstCnt);
    /* 元の行数を格納 */
    for(i= 0; i < hinLstCnt; i++){
        lineK[i] = i;
		lineIdx[i] = i;
    }

    /* 行に関してソート */
    for(pass = 0; pass < hinLstCnt - 1 - mk_count; pass++){
        for(i = 0; i < (hinLstCnt - 1 - mk_count - pass); i++){
			i0 = lineIdx[i];
			i1 = lineIdx[i+1];
            for(j = 0; j < hinLstCnt;){
                /* 次の行のコストの方が小さいとき */
                if(unaConSrc[i0 * hinLstCnt + j].cost > unaConSrc[i1 * hinLstCnt + j].cost){
                    /* lineK[] 中の行位置の値 i と i+1 を交換する */
                    for (k = 0; k < hinLstCnt; k++) {
                        if (lineK[k] == i) {
                            lineK[k] = i + 1;
                        } else if (lineK[k] == (i + 1)) {
                            lineK[k] = i;
                        }
                    }
					/* 行の参照先を交換する */
					int tmp = lineIdx[i];
					lineIdx[i] = lineIdx[i+1];
					lineIdx[i+1] = tmp;
                    j = hinLstCnt;      /* ループを抜ける */
                }
                /* 次の行のコストの方が大きいとき */
                else if(unaConSrc[i0 * hinLstCnt + j].cost < unaConSrc[i1 * hinLstCnt + j].cost){
                    j = hinLstCnt;        /* ループを抜ける */
                }    
                /* 次の行のコストと同じとき */
                else{
                    /* 同じコストをマージ */
                    if(j == hinLstCnt - 1){
                        /* lineK[] 中の行位置の値が i+1 以上なら1減らす */
                        for(k = 0; k < hinLstCnt; k++){
                            if(lineK[k] >= (i + 1)){
                                lineK[k]--;
                            }
                        }
						/* 行をマージし、それ以降の参照先をつめる */
                        for(k = i; k < hinLstCnt - 1 - mk_count; k++){
							lineIdx[k] = lineIdx[k+1];
						}
                        mk_count++;
                        i--;
                    }
                    j++;
                }
            }
        }
    }

	/* 間接参照配列のソート結果を元の中間接続表のコストに反映する */
	unaConSrcTmp = (connectIdxT *)malloc(sizeof(connectIdxT) * conSrcCnt);
	memcpy(unaConSrcTmp, unaConSrc, sizeof(connectIdxT) * conSrcCnt);
	for (i = 0; i < hinLstCnt - mk_count; i++) {
		i0 = lineIdx[i];
		for (j = 0; j < hinLstCnt; j++) {
			unaConSrc[i * hinLstCnt + j].cost = unaConSrcTmp[i0 * hinLstCnt + j].cost;
		}
	}
	free(unaConSrcTmp);
	free(lineIdx);

    return mk_count;
}

//--------------------------------------------------------------------------
// MODULE:	  SortRetsuCost
//
// ABSTRACT:	  比較関数
//
// FUNCTION:
//	  中間接続表のコストを列でソートするための比較関数
//
// RETURN:
//	  mu_count        列方向でのマージ行数
//
// NOTE:
//	  
//
static int SortRetsuCost(
        int ConSrcCnt,
        int hinLstCnt,
        int  mk_count,
        int  mu_count
)
{
    int        i;              /* ループ変数 */
    int        j;              /* ループ変数 */
    int        k;              /* ループ変数 */
    int        pass;           /* ループ変数 */
	int        i0;             /* lineIdx[i]の値 */
	int        i1;             /* lineIdx[i+1]の値 */
	ushortT    *lineIdx;       /* 列位置間接参照配列 */
	connectIdxT	*unaConSrcTmp; /* 接続表用データコピー用作業領域 */

    /* 受けコードのメモリを確保 */
    lineU = (ushortT *)malloc(sizeof(ushortT) * hinLstCnt);
	/* 列位置間接参照配列のメモリを確保 */
	lineIdx = ( ushortT *)malloc(sizeof(ushortT) * hinLstCnt);

    /* 元の行数を格納 */
    for(i= 0; i < hinLstCnt; i++){
        lineU[i] = i;
		lineIdx[i] = i;
    }

    /* 列に関してソート */
    for(pass = 0; pass < hinLstCnt - 1 - mu_count; pass++){
        for(i = 0; i < (hinLstCnt - 1 - mu_count - pass); i++){
			i0 = lineIdx[i];
			i1 = lineIdx[i+1];
            for(j = 0; j < hinLstCnt - mk_count ;){
                /* 次の列の方がコストが小さいとき */
                if(unaConSrc[j * hinLstCnt + i0].cost > unaConSrc[j * hinLstCnt + i1].cost){
                    /* lineU[] 中の行位置の値 i と i+1 を交換する */
                    for (k = 0; k < hinLstCnt; k++) {
                        if (lineU[k] == i) {
                            lineU[k] = i + 1;
                        } else if (lineU[k] == (i + 1)) {
                            lineU[k] = i;
                        }
                    }
					/* 列の参照先を交換する */
					int tmp = lineIdx[i];
					lineIdx[i] = lineIdx[i+1];
					lineIdx[i+1] = tmp;
                    j = hinLstCnt - mk_count;     /* ループを抜ける */
                }
                /* 次の列の方がコストが大きいとき */
                else if(unaConSrc[j * hinLstCnt + i0].cost < unaConSrc[j * hinLstCnt + i1].cost){
                    j = hinLstCnt - mk_count;     /* ループを抜ける */
                }
                /* コストが同じ場合 */
                else{
                    if(j == hinLstCnt - mk_count -1 ){
                        /* lineU[] 中の行位置の値が i+1 以上なら1減らす */
                        for (k = 0; k < hinLstCnt; k++) {
                            if (lineU[k] >= (i + 1)) {
                                lineU[k]--;
                            }
                        }
						/* 列をマージし、それ以降の参照先をつめる */
                        for(k = i; k < hinLstCnt - 1 - mu_count; k++){
							lineIdx[k] = lineIdx[k+1];
						}
                        mu_count++;
                        i--;
                    }
                    j++;
                }
            }
        }
    }

	/* 間接参照配列のソート結果を元の中間接続表のコストに反映する */
	unaConSrcTmp = (connectIdxT *)malloc(sizeof(connectIdxT) * ConSrcCnt);
	memcpy(unaConSrcTmp, unaConSrc, sizeof(connectIdxT) * ConSrcCnt);
	for (i = 0; i < hinLstCnt - mu_count; i++) {
		i0 = lineIdx[i];
		for (j = 0; j < hinLstCnt - mk_count; j++) {
			unaConSrc[j * hinLstCnt + i].cost = unaConSrcTmp[j * hinLstCnt + i0].cost;
		}
	}
	free(unaConSrcTmp);
	free(lineIdx);

    return mu_count;
}

//--------------------------------------------------------------------------
// MODULE:	  swap
//
// ABSTRACT:	  swap関数
//
// FUNCTION:
//	   型の値をswapする。
//
// RETURN:
//	  なし
//
// NOTE:
//	  
//
void swap(
        ucharT *ptr1,
        ucharT *ptr2
)
{
    ucharT temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

//--------------------------------------------------------------------------
// Copyright (c) 2000-2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
