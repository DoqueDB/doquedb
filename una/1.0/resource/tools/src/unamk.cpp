//
// unamk.cpp -
//  UNA V3で形態素解析を行なうプログラム
//  コンパクト言語解析系(UNA)V3の形態素解析／かかりうけ解析プログラム例
//  検索用ライブラリでの使用を前提としたAPIを使用している
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

#include <stdio.h>		/* FILENAME_MAX,FILE,fprintf,fopen,fread,fclose */
#include <string.h>		/* strlen,strcpy */
#include <stdlib.h>		/* atoi */
#include <time.h>		/* clock */
#include "unakapi.h"	/* UNAの検索用APIのヘッダファイル */
#include "unamorph.h"	/* UNAの形態素解析モジュールのヘッダファイル */
#ifndef RSC_DIR
  #define RSC_DIR "."	/* 辞書などのあるディレクトリのデフォルト */
#endif
#ifndef REP_TBL
  #define REP_TBL "" /* 文字置き換えテーブルのデフォルト */
#endif
#ifndef DIC_LIST
  #define DIC_LIST "diclist.dat" /* 辞書リストファイルのデフォルト */
#endif
#ifndef CNCT_TBL
  #define CNCT_TBL "connect.tbl" /* 接続表のデフォルト */
#endif
#ifndef GRAM_TBL
  #define GRAM_TBL ""			 /* 実行文法テーブルのデフォルト */
#endif
#ifndef EMK_TBL
  #define EMK_TBL "engmk.tbl" /* 英語トークン用文字種別テーブルのデフォルト */
#endif
#ifndef UMK_TBL
  #define UMK_TBL "unkmk.tbl" /* 未登録語用文字種別テーブルのデフォルト */
#endif
#ifndef UC_TBL
  #define UC_TBL "unkcost.tbl" /* 未登録語コスト推定テーブルのデフォルト */
#endif
#ifndef OUT_FILE
  #define OUT_FILE "unaout.txt"	 /* 出力ファイルのデフォルト */
#endif
#ifndef REP_TBL2
  #define REP_TBL2 "" 		/* 文字置き換えテーブル2のデフォルト */
#endif
#ifndef DIC_LIST2
  #define DIC_LIST2 ""		/* 辞書リストファイル2のデフォルト */
#endif
#ifndef CNCT_TBL2
  #define CNCT_TBL2 ""		/* 接続表2のデフォルト */
#endif
#ifndef GRAM_TBL2
  #define GRAM_TBL2 ""		/* 実行文法テーブル2のデフォルト */
#endif
#ifndef EMK_TBL2
  #define EMK_TBL2 ""	/* 英語トークン用文字種別テーブル2のデフォルト */
#endif
#ifndef UMK_TBL2
  #define UMK_TBL2 ""		/* 未登録語用文字種別テーブル2のデフォルト */
#endif
#ifndef UC_TBL2
  #define UC_TBL2 ""		/* 未登録語コスト推定テーブル2のデフォルト */
#endif
#ifndef STOP_TIME
  #define STOP_TIME -1.0		 /* プログラム終了までの時間のデフォルト */
#endif
#ifndef INTEXT_BUFSIZ
  #define INTEXT_BUFSIZ 1024	 /* 入力テキスト用バッファサイズ */
#endif
#ifndef OUTTEXT_BUFSIZ
  #define OUTTEXT_BUFSIZ 1024	 /* 出力テキスト用バッファサイズ */
#endif
#ifndef LOCAL_TEXT_SIZE
  #define LOCAL_TEXT_SIZE (UNA_LOCAL_TEXT_SIZE + 1)
						/* 一度に形態素解析可能な最大サイズ(文字数) + 1。
							+1 は強制収束時の仮想文節頭末分 */
#endif
#ifndef LOCAL_BNS_SIZE
  #define LOCAL_BNS_SIZE UNA_LOCAL_BNS_SIZE
						/* 一度に係り受け解析可な最大サイズ(形態素数) */
#endif
#define NO	0					/* フラグ値 */
#define YES	1					/* フラグ値 */
#define RESPONSE_TIME 0.1		/* プログラム中断指示が出てから、実際に */
								/* プログラムが終了するまでの希望時間(秒) */
#ifdef _WIN32
  #define PATH_DELIM "\\"		/* ファイルパスのデリミタ(WIN) */
#else
  #define PATH_DELIM "/"		/* ファイルパスのデリミタ(UNIX) */
#endif

/* ヒートランテスト用 */
#if defined(UNAHTTEST) && !defined(UNAMLTEST)
  #define LOOP_CNT 10000		/* ヒートランテスト用ループ回数 */
#endif

/* マルチスレッドテスト用 */
#if defined(UNAMLTEST) && !defined(UNAHTTEST)
  #if defined(_WIN32)
    #include <windows.h>			/* スレッドテスト用 */
    DWORD WINAPI Threadize(LPVOID arg);
    #if UNAMLTEST == 2
      #define THREAD_NUM 2			/* スレッド作成数 */
    #else
      #define THREAD_NUM 100		/* スレッド作成数 */
    #endif
  #else
    #include <unistd.h>
    #if defined(UNALINUX)
      #include <pthread.h>			/* linux */
    #else
      #include <thread.h>			/* solaris */
    #endif
    #include <errno.h>
    void *Threadize(void *arg);
    #if UNAMLTEST == 2
      #define THREAD_NUM 2			/* スレッド作成数 */
    #else
      #define THREAD_NUM 30			/* スレッド作成数(Unixの制限により30) */
    #endif
  #endif
  typedef struct TCB {		// スレッドコントロールブロック
    char *inFile;			/* 入力ファイル */
	unaKApiDicImgT *dicImgList; /* 形態素辞書・アプリ情報辞書のイメージ等 */
	int dicCount;			/* dicImgListの要素数 */
    char *repTblImg;		/* 文字置き換えテーブルのイメージ */
    char *cnctTblImg;		/* 接続表のイメージ */
    char *gramTblImg;		/* 文法テーブルのイメージ */
    char *eMKTblImg;		/* 英語トークン用文字種別テーブルのイメージ */
    char *uMKTblImg;		/* 未登録語用文字種別テーブルのイメージ */
    char *uCTblImg;			/* 未登録語コスト推定テーブルのイメージ */
    int outputHyoki;		/* 表記を出力するかのフラグ */
    int outputHyokiOnly;	/* 表記のみ出力するかのフラグ */
    int outputUnaHin;		/* UNA品詞を出力するかのフラグ */
    int outputSubMorph;	/* 下位形態素を出力するかのフラグ */
    int connectInfo;		/* 出力情報を繋げて出力するかのフラグ */
    int outputTargetTxt;	/* 解析対象テキストを表示するかのフラグ */
    int inputByLF;			/* 復改の単位で読み込むかのフラグ */
    int outputHinName;		/* 品詞名を出力するかどうかのフラグ */
    char outFile[FILENAME_MAX];	/* 出力ファイル */
  } TCB;
  static int EndThread = 0;		/* 終了しているスレッドの数を保持 */
  static void TstThread(char *inFile, unaKApiDicImgT **dicImgList, int *dicCount,
	char **repTblImg, char **cnctTblImg, char **gramTblImg,char **eMKTblImg,
	char **uMKTblImg, char **uCTblImg,char *outFile,int outputHyoki, int outputHyokiOnly,
	int outputUnaHin, int outputSubMorph,int connectInfo,
	int outputTargetTxt,int inputByLF, int thredNum);
#endif

/* モジュール内部で使用する関数のプロトタイプ宣言 */
static int GetRscNameFrmCmdLine(const int argc, const char *argv[], char *rscDir,
  char *dicList,  char *repTbl,  char *cnctTbl,  char *gramTbl,
  char *eMKTbl,   char *uMKTbl,  char *uCTbl,
  char *dicList2, char *repTbl2, char *cnctTbl2, char *gramTbl2,
  char *eMKTbl2,  char *uMKTbl2, char *uCTbl2,
  char *outFile, int *outputHyoki, int *outputHyokiOnly,
  int *outputUnaHin, int *outputSubMorph, int *connectInfo,
  int *outputTargetTxt, int *inputByLF, int *outputHinName, 
  int *processedArgc);
static void OutputUsage(void);
static int OptionR(const int argc,const char *argv[],char *rscDir,
  int *processedArgc);
static int OptionM(const int argc,const char *argv[],char *dicList,
  char *cnctTbl,int *processedArgc);
static int OptionN(const int argc,const char *argv[],char *dicList2,
  char *cnctTbl2,int *processedArgc);
static int OptionG(const int argc,const char *argv[],char *gramTbl,
  int *processedArgc);
static int OptionI(const int argc,const char *argv[],char *gramTbl2,
  int *processedArgc);
static int OptionE(const int argc,const char *argv[],char *eMKTbl,
  int *processedArgc);
static int OptionF(const int argc,const char *argv[],char *eMKTbl2,
  int *processedArgc);
static int OptionU(const int argc,const char *argv[],char *uMKTbl,
  char *uCTbl,int *processedArgc);
static int OptionV(const int argc,const char *argv[],char *uMKTbl2,
  char *uCTbl2,int *processedArgc);
static int OptionO(const int argc,const char *argv[],char *outFile,
  int *processedArgc);
static int OptionP(const int argc,const char *argv[],char *replTbl,
  int *processedArgc);
static int OptionQ(const int argc,const char *argv[],char *replTbl2,
  int *processedArgc);
static int OptionT(const int argc,const char *argv[],int *processedArgc);
static int SetFileRscName(char *outRscName,const char *inRscName);
static int ReadDic(char *rscDir,char *dicList,char *repTbl,
  char *cnctTbl,char *gramTbl,char *eMKTbl,char *uMKTbl,char *uCTbl,
  unaKApiDicImgT *dicImgList, int *dicCount, char **repTblImg, char **cnctTblImg,
  char **gramTblImg,char **eMKTblImg, char **uMKTblImg,char **uCTblImg);
static int FreeDic(unaKApiDicImgT *dicImgList, char *repTblImg, char *cnctTblImg,
  char *gramTblImg,char *eMKTblImg, char *uMKTblImg,char *uCTblImg);
static int ReadFileImg(const char *rscDir,const char *fileName,
  char **mapAddress);
static int DoKaiseki(FILE *fp,const unaKApiDicImgT *dicImgList, int dicCount,
  const char *repTblImg,const char *cnctTblImg,const char *gramTblImg,
  const char *eMKTblImg,const char *uMKTblImg,const char *uCTblImg,
  const int outputHyoki,const int outputHyokiOnly,
  const int outputUnaHin,const int outputSubMorph,
  const int connectInfo,const int outputTargetTxt,
  const int inputByLF,const int outputHinName, FILE *fo);
static int Getws(unaCharT *buf,int maxBufLen,FILE *fp);
static int Fgetws(unaCharT *buf,int maxBufLen,FILE *fp);
static int OutputMorphInfo(unaKApiHandleT *h,const unaMorphT *morphBuf,
  const int morphNum,ucharT **ansBuf,int ansBufSize,
  const int outputHyoki,const int outputHyokiOnly,
  const int outputUnaHin,const int connectInfo,const int outputSubMorph,
  const int outputHinName, int level,FILE *fo);
static int Ltou(int x,unaCharT *buf);
static int OutputBnsInfo(unaKApiHandleT *h,const unaBnsT *bnsBuf,
  const int bnsNum,const int outputHyoki,const int outputHyokiOnly,
  const int outputUnaHin,FILE *fo);
static int SetKakaUkeSymbol(int kuRel,unaCharT *ans);
static int GetAppInfo(unaKApiHandleT *h,const unaMorphT *morph,
  ucharT *appInfBuf,int appInfBufSiz,int *appInfByte);
static int EngSubMorphAppInf(unaKApiHandleT *h,const unaMorphT *morph,
  ucharT *appInfBuf,int appInfBufSiz,int *appInfByte);
static void GetTypeStr(unaCharT *typeStr,uintT tokenType);
static int StopByTime(void);

/* グローバル変数 */
static double StopTime;		/* この時間(秒)経過後プログラムを終了する */
static int AnadbgFlag = NO;	/* anadbg用の出力を標準出力に書き出す */


//--------------------------------------------------------------------------
// MODULE:		main
//
// ABSTRACT:	UNAを使って変換
//
// FUNCTION:
//	UNA V3を使って、テキスト変換(形態素解析)又は係り受け解析をする。
//	結果は出力ファイルに出力される。
//
// RETURN:
//	0			正常終了
//	1-255		エラー
//
// NOTE:
//	使い方は、コマンド行から次の通りに入力する。
//		una/unamk.exe [オプション] [ファイル...]
//
//	オプションは次の通り。
//		-h						 使い方を表示する。
//		-r RSCDIR				 辞書などがあるディレクトリを指定する。
//		-p REPTBL				 文字変換テーブルを指定する
//		-q REPTBL2				 文字変換テーブル2を指定する
//		-m DICLIST CNCTTBL		 形態素解析用辞書に関する情報を指定する
//		-n DICLIST2 CNCTTBL2	 形態素解析用辞書2に関する情報を指定する
//		-g GRAMTBL				 実行文法テーブルを指定する
//		-i GRAMTBL2				 実行文法テーブル2を指定する
//		-e ENGMKTBL				 英語トークン用文字種別テーブルを指定する
//		-f ENGMKTBL2			 英語トークン用文字種別テーブル2を指定する
//		-u UMKMKTBL UNKCOSTTBL	 未登録語検出用辞書に関する情報を指定する
//		-v UMKMKTBL2 UNKCOSTTBL2 未登録語検出用辞書2に関する情報を指定する
//		-o OUTFILE				 出力ファイルを指定する
//		-t STOPTIME				 中断までの時間を秒で指定する
//		-y 						 表記を出力する
//		-Y 						 表記のみ出力する
//		-c 						 UNA品詞を出力する
//		-s						 複合語の下位形態素も表示する
//		-k 						 形態素情報を改行しないで出力する
//		-d 						 対象テキストを表示した後、形態素を表示
//		-l						 改行コード(LF)で区切って読み込む
//		-N						 形態素品詞名を出力する
//		-a						 anadbgが使うラティス情報を標準出力に出力する
//
//	形態素解析用辞書に関する情報は以下の通り。
//		DICLIST			形態素辞書・アプリ情報辞書の辞書ベース名と辞書優先度を記述したリスト
//		CNCTTBL			接続表
//
//	未登録語検出用辞書に関する情報は以下の通り。
//		UNKMKTBL		未登録語用文字種別テーブル
//		UNKCOSTTBL		未登録語コスト推定テーブル
//
//	形態素解析用辞書2に関する情報は以下の通り。
//		DICLIST2		形態素辞書2・アプリ情報辞書2の辞書ベース名と辞書優先度を記述したリスト
//		CNCTTBL2		接続表2
//
//	未登録語検出用辞書2に関する情報は以下の通り。
//		UNKMKTBL2		未登録語用文字種別テーブル2
//		UNKCOSTTBL2		未登録語コスト推定テーブル2
//
//	デフォルト値は以下の通り。
//		-r .(カレントディレクトリ)
//		-p unastd.tbl
//		-m diclist.dat connect.tbl
//		-u unkmk.tbl unkcost.tbl
//		-o unaout.txt
//		-t -1.000000
//		その他のオプションは、デフォルトでは指定されてないものとみなされる
//
//	その他の注意
//
//	・-l オプションが無い場合のテキストの読み込みは、句点記号区切り
//	 (「。」「！」「？」「v等)又は、改行に続く1つの空行である。
//	・解析対象のファイルは複数個指定できる。
//	・辞書リスト、接続表、実行文法テーブル、英語トークン用文字種
//	  テーブル、未登録語文字種別テーブル、未登録語コスト推定テーブル、
//	  出力ファイルをディレクトリ付きで指定する場合には RSCDIR からの
//	  相対パスで指定する。
//	・形態素解析用辞書の情報を指定する場合は、必ず DICLIST、CNCTTBLの2つを
//	  一緒に指定する事。
//	・未登録語検出用辞書の情報を指定する場合は、必ず UMKTBL、UCTBLの2つを
//	  一緒に指定する事。
//	・英語トークン検出を行うか否かは、'-e' オプションを
//	  指定するかどうかによる。'-e' オプションがあれば英語トークン検出も行う。
//	・形態素解析をするか、係り受け解析をするかは、'-g' オプションを
//	  指定するかどうかによる。'-g' オプションがあれば係り受け解析をし、
//	  なければ形態素解析をする。
//	・STOPTIME < 0 の場合は、ノンストップ実行です。
//	・オプション '-n'、'-b'、'-i'、'-f'、'-v' は、マルチスレッドテストの
//	  うちの解析結果比較テストでのみ有効である。
//
//	例)
//	unamk -r \rsc -m diclist.dat connect.tbl -g gram.tbl
//		  -e engmk.tbl -u unkmk.tbl unkcost.tbl -o \\log\\unaout.txt
//		  -t 2.5 -y -c -s -k -d -l test1.txt test2.txt
//		  (全てのオプションを指定)
//
//	unamk -r kapi/rsc/bindata -e engmk.tbl
//								-o /log/kekka.log text/hyoka.txt
//		  (リソースディレクトリは kapi/rsc/bindata。
//		  英語トークン検出も行う。結果は、/log/kekka.log に出力。
//		  解析対象ファイルは、text/hyoka.txt で、句点記号までを1行とみなす)
//
int main(
	const int argc,			 /* コマンド行の引数の数(1以上) */
	const char *argv[]			 /* コマンド行の引数 */
	)
{
	char rscDir[FILENAME_MAX];		/* リソースのあるディレクトリ */
	char dicList[2][FILENAME_MAX];		/* 辞書リスト */
	char repTbl[2][FILENAME_MAX]; 	/* 文字置き換えテーブル */
	char cnctTbl[2][FILENAME_MAX];	/* 接続表 */
	char gramTbl[2][FILENAME_MAX];	/* 実行文法辞書 */
	char eMKTbl[2][FILENAME_MAX];	/* 英語トークン用文字種別テーブル */
	char uMKTbl[2][FILENAME_MAX];	/* 未登録語用文字種別テーブル */
	char uCTbl[2][FILENAME_MAX];	/* 未登録語コスト推定テーブル */
	char outFile[FILENAME_MAX];		/* 出力ファイル */
	int dicCount[2];				/* dicImgList[]の要素数 */
	int outputHyoki;			 /* 表記を出力するかのフラグ */
	int outputHyokiOnly;			 /* 表記のみ出力するかのフラグ */
	int outputUnaHin;			 /* UNA品詞を出力するかのフラグ */
	int outputSubMorph;		 /* 下位形態素を出力するかのフラグ */
	int connectInfo;			 /* 出力情報を繋げて出力するかのフラグ */
	int outputTargetTxt;		 /* 解析対象テキストを表示するかのフラグ*/
	int outputHinName;          /* 形態素品詞名を表示するかのフラグ*/
	int inputByLF;				 /* 復改の単位で読み込むかのフラグ */
	unaKApiDicImgT dicImgList[2][UNA_MORPH_DIC_MAX]; /* 実行用辞書のイメージ等 */
	char *repTblImg[2];			 /* 文字置き換えテーブルのイメージ */
	char *cnctTblImg[2];		 /* 接続表のイメージ */
	char *gramTblImg[2];		 /* 文法テーブルのイメージ */
	char *eMKTblImg[2];			 /* 英語トークン用文字種テーブルのイメージ */
	char *uMKTblImg[2];			 /* 未登録語用文字種テーブルのイメージ */
	char *uCTblImg[2];			 /* 未登録語コスト推定テーブルのイメージ */
	int processedArgc;			 /* 処理した引数の数 */
	int rv;					 /* 関数の返値 */
	int i;						 /* ループ変数 */
	int n;

/* ヒートランテスト用 */
#if defined(UNAHTTEST) && !defined(UNAMLTEST)
	int j;								/* ループ変数 */
	char fileHeadName[FILENAME_MAX];	/* ヒートランテスト用出力ファイル */
#endif

/* マルチスレッドテスト用 */
#if !defined(UNAMLTEST)
	FILE *fp;					 /* ファイル */
	FILE *fo;					 /* 出力ファイル */
	unsigned short uniChar;		 /* ユニコード1文字 */
	double rt;					 /* レスポンスタイム */
#endif

	/* 初期設定 */
	strcpy(rscDir,RSC_DIR);
	strcpy(dicList[0],DIC_LIST);
	strcpy(repTbl[0],REP_TBL);
	strcpy(cnctTbl[0],CNCT_TBL);
	strcpy(gramTbl[0],GRAM_TBL);
	strcpy(eMKTbl[0],EMK_TBL);
	strcpy(uMKTbl[0],UMK_TBL);
	strcpy(uCTbl[0],UC_TBL);
	strcpy(outFile,OUT_FILE);
	strcpy(dicList[1],DIC_LIST2);
	strcpy(repTbl[1],REP_TBL2);
	strcpy(cnctTbl[1],CNCT_TBL2);
	strcpy(gramTbl[1],GRAM_TBL2);
	strcpy(eMKTbl[1],EMK_TBL2);
	strcpy(uMKTbl[1],UMK_TBL2);
	strcpy(uCTbl[1],UC_TBL2);
	StopTime		= STOP_TIME;
	outputHyoki		= NO;
	outputHyokiOnly		= NO;
	outputUnaHin	= NO;
	outputSubMorph	= NO;
	connectInfo		= NO;
	outputTargetTxt	= NO;
	inputByLF		= NO;
	for (i = 0;i < 2;i++) {
		for (n = 0; n < UNA_MORPH_DIC_MAX; n++) {
			dicImgList[i][n].dicName	= (char *)NULL;
			dicImgList[i][n].morphDic	= (char *)NULL;
			dicImgList[i][n].appInfo	= (char *)NULL;
			dicImgList[i][n].dicPrio	= 0;
		}
		dicCount[i]			= 0;
		repTblImg[i]		= (char *)NULL;
		cnctTblImg[i]		= (char *)NULL;
		gramTblImg[i]		= (char *)NULL;
		eMKTblImg[i]		= (char *)NULL;
		uMKTblImg[i]		= (char *)NULL;
		uCTblImg[i]			= (char *)NULL;
	}

	/* 実行に必要なリソース（辞書名など）の取得 */
	rv = GetRscNameFrmCmdLine(argc,argv,
							rscDir,
							dicList[0],repTbl[0],cnctTbl[0],
							gramTbl[0],eMKTbl[0],uMKTbl[0],uCTbl[0],
							dicList[1],repTbl[1],cnctTbl[1],
							gramTbl[1],eMKTbl[1], uMKTbl[1],uCTbl[1],
							outFile,
							&outputHyoki,&outputHyokiOnly,
							&outputUnaHin,&outputSubMorph,
							&connectInfo,&outputTargetTxt,&inputByLF,
							&outputHinName, &processedArgc);
	if (rv != 0) {	/* エラー又はヘルプ表示の時 */
		return (int)rv;
	}

#if defined(UNAHTTEST) && !defined(UNAMLTEST)
strcpy(fileHeadName,outFile);
for (j = 0;j < LOOP_CNT; j++) {	/* 1万セットのヒートランのループ */
#endif

	/* 辞書類の読み込み */
	rv = ReadDic(rscDir,
				dicList[0],repTbl[0],cnctTbl[0],
				gramTbl[0],eMKTbl[0],uMKTbl[0],uCTbl[0],
				dicImgList[0],&dicCount[0],&repTblImg[0],&cnctTblImg[0],
				&gramTblImg[0],&eMKTblImg[0], &uMKTblImg[0],&uCTblImg[0]);
	if (rv != 0) {
		fprintf(stdout,"Error in reading First Dictionaries.\n");
		return 1;
	}

#if defined(UNAMLTEST) && !defined(UNAHTTEST)
/* スレッドの時 */
	#if UNAMLTEST == 2
		rv = ReadDic(rscDir,
				dicList[1],repTbl[1],cnctTbl[1],
				gramTbl[1],eMKTbl[1],uMKTbl[1],uCTbl[1],
				dicImgList[1],&dicCount[1],&repTblImg[1],&cnctTblImg[1],
				&gramTblImg[1],&eMKTblImg[1],&uMKTblImg[1],&uCTblImg[1]);
		if (rv != 0) {
			fprintf(stdout,"Error in reading Second Dictionaries.\n");
			return 2;
		}
	#endif
	TstThread((char *)argv[processedArgc],
			dicImgList,dicCount,repTblImg,cnctTblImg,
			gramTblImg,eMKTblImg,uMKTblImg,uCTblImg,
			outFile, outputHyoki, outputHyokiOnly, outputUnaHin,outputSubMorph,
			connectInfo, outputTargetTxt, inputByLF,THREAD_NUM);
#else
/* 以下スレッドじゃない時 */
	#if defined(UNAHTTEST) && !defined(UNAMLTEST)
		if (j % 100 == 0) {
			fprintf(stdout,"%d Loops Finished\n",j);
			sprintf(outFile,"%s%04d.txt",fileHeadName,j);
		}
	#endif

	/* 出力ファイルのオープン */
	if ((fo = fopen(outFile, "wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",outFile);
		for (n = 0; n < dicCount[0]; n++) {
			free(dicImgList[0][n].dicName);
			unaKApi_freeImg(dicImgList[0][n].morphDic);
			unaKApi_freeImg(dicImgList[0][n].appInfo);
		}
		unaKApi_freeImg(repTblImg[0]);
		unaKApi_freeImg(cnctTblImg[0]);
		unaKApi_freeImg(gramTblImg[0]);
		unaKApi_freeImg(eMKTblImg[0]);
		unaKApi_freeImg(uMKTblImg[0]);
		unaKApi_freeImg(uCTblImg[0]);
		return 5;
	}
	uniChar = 0xFEFF;	/* 予めBOMを書き込む */
	fwrite(&uniChar,sizeof(uniChar),1,fo);
	
	/* 対象ファイルの数だけループし解析を行う */
	for (i = processedArgc; i < argc; i++) {
		if ((fp = fopen(argv[i], "rb")) == NULL) {
			fprintf(stdout,"Can't open %s\n",argv[i]);
			for (n = 0; n < dicCount[0]; n++) {
				free(dicImgList[0][n].dicName);
				unaKApi_freeImg(dicImgList[0][n].morphDic);
				unaKApi_freeImg(dicImgList[0][n].appInfo);
			}
			unaKApi_freeImg(repTblImg[0]);
			unaKApi_freeImg(cnctTblImg[0]);
			unaKApi_freeImg(gramTblImg[0]);
			unaKApi_freeImg(eMKTblImg[0]);
			unaKApi_freeImg(uMKTblImg[0]);
			unaKApi_freeImg(uCTblImg[0]);
			return 6;
		}
		rv = DoKaiseki(fp,dicImgList[0],dicCount[0],repTblImg[0],cnctTblImg[0],
						gramTblImg[0],eMKTblImg[0],uMKTblImg[0],uCTblImg[0],
						outputHyoki,outputHyokiOnly,outputUnaHin,outputSubMorph,
						connectInfo,outputTargetTxt,inputByLF,outputHinName,fo);
		if (rv != 0) {
			if (rv == UNA_STOP) {	/* 終了時間の測定 */
				rt = (double)clock() / CLOCKS_PER_SEC - StopTime; 
				if (rt < RESPONSE_TIME) { 
					rv = 0;	/* 中断の時の返り値(時間どおりに止まった時) */
				}
				else {
					fprintf(stdout,"Stop Respons Time is %f sec.\n",rt);
					rv = 8; /* 中断の時の返り値(止まるのが遅かった時) */
				}
			}
			fclose(fo);
			fclose(fp);
			for (n = 0; n < dicCount[0]; n++) {
				free(dicImgList[0][n].dicName);
				unaKApi_freeImg(dicImgList[0][n].morphDic);
				unaKApi_freeImg(dicImgList[0][n].appInfo);
			}
			unaKApi_freeImg(repTblImg[0]);
			unaKApi_freeImg(cnctTblImg[0]);
			unaKApi_freeImg(gramTblImg[0]);
			unaKApi_freeImg(eMKTblImg[0]);
			unaKApi_freeImg(uMKTblImg[0]);
			unaKApi_freeImg(uCTblImg[0]);
			return (int)rv;
		}
		fclose(fp);
	}

	/* 出力ファイルのクローズ */
	fclose(fo);
#endif

	/* 形態素辞書・アプリ辞書イメージのfree */
	for (n = 0; n < dicCount[0]; n++) {
		free(dicImgList[0][n].dicName);
		unaKApi_freeImg(dicImgList[0][n].morphDic);
		unaKApi_freeImg(dicImgList[0][n].appInfo);
	}
	/* 文字置き換えテーブルイメージのfree */
	unaKApi_freeImg(repTblImg[0]);
	/* 接続表イメージのfree */
	unaKApi_freeImg(cnctTblImg[0]);
	/* 文法テーブルイメージのfree */
	unaKApi_freeImg(gramTblImg[0]);
	/* 英語トークン用文字種別テーブルイメージのfree */
	unaKApi_freeImg(eMKTblImg[0]);
	/* 未登録語用文字種別テーブルイメージのfree */
	unaKApi_freeImg(uMKTblImg[0]);
	/* 未登録語コスト推定テーブルテーブルイメージのfree */
	unaKApi_freeImg(uCTblImg[0]);

#if defined(UNAHTTEST) && !defined(UNAMLTEST)
}	/* ヒートランテストの時のループエンド */
#endif

	/* 終了 */
	return 0;
}


//--------------------------------------------------------------------------
// MODULE:		GetRscNameFrmCmdLine
//
// ABSTRACT:	リソースの取得
//
// FUNCTION:
//	  形態素解析に必要な辞書名などのリソースは、コマンドラインから入力される
//	  ので main 関数の引数として、プログラムに渡る。
//	  そのコマンドラインからの引数を受け取り、検査、整形して、各々の
//	  出力変数に格納し呼び出し元に返す。
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  引数に、"-h" が指定された時は、ヘルプを表示して戻る。
//	  コマンドラインに指定されなかった変数は、デフォルトのまま。
//
static int GetRscNameFrmCmdLine(
	const int argc,	  /* argv 配列の要素数 */
	const char *argv[],	  /* 配列(要素はアドレス)の先頭アドレス */
	char *rscDir,		  /* リソースのあるディレクトリ */
	char *dicList,		  /* 辞書リスト */
	char *repTbl,		  /* 入力文字置き換えテーブル */
	char *cnctTbl,		  /* 接続表 */
	char *gramTbl,		  /* 文法テーブル、不要ならNULLが返る */
	char *eMKTbl,	/* 英語トークン用文字種別テーブル、不要ならNULLが返る */
	char *uMKTbl,		  /* 未登録語用文字種別テーブル */
	char *uCTbl,		  /* 未登録語コスト推定テーブル */
	char *dicList2,		  /* 辞書リスト2 */
	char *repTbl2,		  /* 入力文字置き換えテーブル2 */
	char *cnctTbl2,		  /* 接続表2 */
	char *gramTbl2,		  /* 文法テーブル2、不要ならNULLが返る */
	char *eMKTbl2,	/* 英語トークン用文字種別テーブル2、不要ならNULLが返る */
	char *uMKTbl2,		  /* 未登録語用文字種別テーブル2 */
	char *uCTbl2,		  /* 未登録語コスト推定テーブル2 */
	char *outFile,		  /* 出力ファイル */
	int *outputHyoki,	  /* 表記を出力するかのフラグ */
	int *outputHyokiOnly,	  /* 表記のみ出力するかのフラグ */
	int *outputUnaHin,	  /* UNA品詞を出力するかのフラグ */
	int *outputSubMorph, /* 下位形態素を出力するかのフラグ */
	int *connectInfo,	   /* 出力情報を繋げて出力するかのフラグ */
	int *outputTargetTxt, /* 対象テキストを表示するかのフラグ */
	int *inputByLF,	   /* 復改の単位で読み込むかのフラグ */
	int *outputHinName,   /* 形態素品詞名を出力するかどうかのフラグ */
	int *processedArgc	   /* 処理した引数の数(後カウントアップ) */
)
{
	int rv;			   /* 関数の返値 */

	/* argc の数だけループ。第1引数にはプログラム名が入っているから1より */
	*processedArgc = 1;		/* 上記の理由により引数1個処理したとする */
	while (*processedArgc < argc) {
		if (*(argv[*processedArgc]) !='-') { /* オプションは '-' で始まる */
			break;				/* 解析ファイルが指定されたとしてブレーク */
		}
		switch(*(argv[*processedArgc] + 1)){ /* 引数の2バイト目をテスト */
		case 'h':	/* ヘルプの要求 */
			/* Usageを表示して終了 */
			OutputUsage();
			(*processedArgc)++;
			return 10;
		case 'r':
			rv = OptionR(argc,argv,rscDir,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'p':
			rv = OptionP(argc,argv,repTbl,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'q':
			rv = OptionQ(argc,argv,repTbl2,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'm':
			rv = OptionM(argc,argv,dicList,cnctTbl,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'n':
			rv = OptionN(argc,argv,dicList2,cnctTbl2,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'g':
			rv = OptionG(argc,argv,gramTbl,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'i':
			rv = OptionI(argc,argv,gramTbl2,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'e':
			rv = OptionE(argc,argv,eMKTbl,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'f':
			rv = OptionF(argc,argv,eMKTbl2,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'u':
			rv = OptionU(argc,argv,uMKTbl,uCTbl,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'v':
			rv = OptionV(argc,argv,uMKTbl2,uCTbl2,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'o':
			rv = OptionO(argc,argv,outFile,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 't':
			rv = OptionT(argc,argv,processedArgc);
			if (rv != 0){
				return rv;
			}
			break;
		case 'y':
			*outputHyoki = YES;
			(*processedArgc)++;
			break;
		case 'Y':
			*outputHyokiOnly = YES;
			(*processedArgc)++;
			break;
		case 'c':
			*outputUnaHin = YES;
			(*processedArgc)++;
			break;
		case 's':
			*outputSubMorph = YES;
			(*processedArgc)++;
			break;
		case 'k':
			*connectInfo = YES;
			(*processedArgc)++;
			break;
		case 'd':
			*outputTargetTxt = YES;
			(*processedArgc)++;
			break;
		case 'l':
			*inputByLF = YES;
			(*processedArgc)++;
			break;
		case 'N':
			*outputHinName = YES;
			(*processedArgc)++;
			break;
		case 'a':
			AnadbgFlag = YES;
			(*processedArgc)++;
			break;
		default:
			fprintf(stdout, "Bad option %s\n", argv[*processedArgc]);
			return 11;
		}
	}
	return 0;
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
  char dummy[1024];

  fprintf(stdout,"Usage: unamk [OPTIONS] [FILE...]\n");
  fprintf(stdout,"  OPTIONS\n");
  fprintf(stdout,"    -h                       Help\n");
  fprintf(stdout,"    -r RSCDIR                Resouce directory\n");
  fprintf(stdout,"    -p REPTBL                Char Replace Table\n");
  fprintf(stdout,"    -q REPTBL2               Char Replace Table2\n");
  fprintf(stdout,"    -m DICLIST CNCTTBL       Morph Dictionaries Info\n");
  fprintf(stdout,"    -n DICLIST2 CNCTTBL2     Morph Dictionaries2 Info\n");
  fprintf(stdout,"    -g GRAMTBL               Grammar Dictionary\n");
  fprintf(stdout,"    -i GRAMTBL2              Grammar Dictionary2\n");
  fprintf(stdout,"    -e ENGMKTBL              ENG Moji Kind Table\n");
  fprintf(stdout,"    -f ENGMKTBL2             ENG Moji Kind Table2\n");
  fprintf(stdout,"    -u UNKMKTBL UNKCOSTTBL   Unk Dictionaries Info\n");
  fprintf(stdout,"    -v UNKMKTBL2 UNKCOSTTBL2 Unk Dictionaries2 Info\n");
  fprintf(stdout,"    -o OUTFILE               Output File\n");
  fprintf(stdout,"    -t STOPTIME(Sec.)        Stop when time passed\n");
  fprintf(stdout,"    -y                       Output with Hyoki\n");
  fprintf(stdout,"    -Y                       Output Hyoki only\n");
  fprintf(stdout,"    -c                       Output with UNA Hin\n");
  fprintf(stdout,"    -s                       Output with Sub Morph\n");
  fprintf(stdout,"    -k                       Output connected Info\n");
  fprintf(stdout,"    -d                       Output original text,too\n");
  fprintf(stdout,"    -l                       Regard the string until");
  fprintf(stdout," LineFeed(LF) as One Line\n");
  fprintf(stdout,"    -N                       Output with part of speech name\n");
  fprintf(stdout,"    -a                       Generate output for anadbg to stdout\n");
  fprintf(stdout,"  Morph Dictionaries Info are\n");
  fprintf(stdout,"    DICLIST                  Dic list which contains Morph Dic and App Info\n");
  fprintf(stdout,"    CNCTTBL                  Connect table\n");
  fprintf(stdout,"  Unk Dictionaries Info are\n");
  fprintf(stdout,"    UNKMKTBL                 Unk Moji Kind Table\n");
  fprintf(stdout,"    UNKCOSTTBL               Unk Cost Table\n");
  fprintf(stdout,"  Morph Dictionaries2 Info are\n");
  fprintf(stdout,"    DICLIST2                 Dic list2 which contains Morph Dic and App Info\n");
  fprintf(stdout,"    CNCTTBL2                 Connect table2\n");
  fprintf(stdout,"  Unk Dictionaries2 Info are\n");
  fprintf(stdout,"    UNKMKTBL2                Unk Moji Kind Table2\n");
  fprintf(stdout,"    UNKCOSTTBL2              Unk Cost Table2\n");
  fprintf(stdout,"  Default values are...\n");
  fprintf(stdout,"    -r %s\n",RSC_DIR);
  fprintf(stdout,"    -p %s\n",REP_TBL);
  fprintf(stdout,"    -m %s %s\n",DIC_LIST,CNCT_TBL);
  fprintf(stdout,"    -u %s %s\n",UMK_TBL,UC_TBL);
  fprintf(stdout,"    -o %s\n",OUT_FILE);
  fprintf(stdout,"    -t %f\n",STOP_TIME);
  fprintf(stdout,"    Regard the other options are not specified\n");
  fprintf(stdout,"  Note...\n");
  fprintf(stdout,"    - Regard the string until KUTEN-KIGOU as One Line\n");
  fprintf(stdout,"     ,or until the (CR)LF just following (CR)LF");
  fprintf(stdout," when no '-l' option\n");
  fprintf(stdout,"    - You can specify *plural* Target FILEs\n");
  fprintf(stdout,"    - Use not absolute but *relative* path when specify DICLIST,CNCTTBL,\n");
  fprintf(stdout,"      GRAMTBL,ENGMKTBL,UNKMKTBL,UNKCOSTTBL and OUTFILE with directory\n");
  fprintf(stdout,"    - Morph Dictionaries Info must be specified");
  fprintf(stdout," by using pair of dics,\n      and not single dic\n");
  fprintf(stdout,"    - Unknown Dictionaries Info must be specified");
  fprintf(stdout," by using pair of dics,\n      and not single dic\n");
  fprintf(stdout,"    - English tokens are detected if WITH '-e' option\n");
  fprintf(stdout,"    - KAKARIUKE-KAISEKI if WITH '-g' option and\n");
  fprintf(stdout,"      KEITAISO-KAISEKI if WITHOUT '-g option\n");
  fprintf(stdout,"    - When STOPTIME < 0 then nonstop\n");
  fprintf(stdout,"    - Option '-n','-i','-f' and '-v' are valid");
  fprintf(stdout," in only \"Multi Tread Text Compare Test\"\n");
  fprintf(stdout,"Example...\n");
  fprintf(stdout,"  >unamk -l -r ./uwork -m diclist.dat connect.tbl");
  fprintf(stdout," -g gram.tbl -e engmk.tbl -u unkmk.tbl");
  fprintf(stdout," unkcost.tbl -o unaout.txt -t 2.5 -y -c -s -k -d");
  fprintf(stdout," test1.txt test2.txt\n");
  fprintf(stdout,"  (Example that all options are specified)\n\n");
  fprintf(stdout,"  >unamk -r ./uwork -e engmk.tbl");
  fprintf(stdout," -o log/kekka.log text/hyoka.txt\n");
  fprintf(stdout,"  (RSCDIR is ./uwork. Use APPINFO.");
  fprintf(stdout," English Tokens are detected.\n   Output to");
  fprintf(stdout," log/kekka.log. Target FILE is text/hyoka.txt and\n");
  fprintf(stdout,"   regard the string until KUTEN-KIGOU as One Line)\n");
  return;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionR
//
// ABSTRACT:   r オプションの処理
//
// FUNCTION:
//	  r オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionR(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*rscDir,			/* リソースのあるディレクトリ */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -r option.\n");
		return 20;
	}
	rv = SetFileRscName(rscDir,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long RSCDIR name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionM
//
// ABSTRACT:   m オプションの処理
//
// FUNCTION:
//	  m オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionM(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*dicList,			/* 辞書リスト */
	char	*cnctTbl,			/* 接続表 */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 3) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -m option.\n");
		return 30;
	}
	rv = SetFileRscName(dicList,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long DICLIST name.\n");
		return rv;
	}
	rv = SetFileRscName(cnctTbl,argv[*processedArgc + 2]);
	if (rv != 0){
		fprintf(stdout, "Too long CNCTTBL name.\n");
		return rv;
	}
	*processedArgc += 3;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionN
//
// ABSTRACT:   n オプションの処理
//
// FUNCTION:
//	  n オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionN(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*dicList2,			/* 辞書リスト2 */
	char	*cnctTbl2,			/* 接続表2 */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 3) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -n option.\n");
		return 30;
	}
	rv = SetFileRscName(dicList2,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long DICLIST2 name.\n");
		return rv;
	}
	rv = SetFileRscName(cnctTbl2,argv[*processedArgc + 2]);
	if (rv != 0){
		fprintf(stdout, "Too long CNCTTBL2 name.\n");
		return rv;
	}
	*processedArgc += 3;

	return 0;
}

//--------------------------------------------------------------------------
// MODULE:	  OptionQ
//
// ABSTRACT:   q オプションの処理
//
// FUNCTION:
//	  q オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionQ(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*replTbl2,			/* 置き換えテーブル2 */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -p option.\n");
		return 90;
	}
	rv = SetFileRscName(replTbl2,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long REPTBL2 name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}

//--------------------------------------------------------------------------
// MODULE:	  OptionP
//
// ABSTRACT:   p オプションの処理
//
// FUNCTION:
//	  p オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionP(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*replTbl,			/* 置き換えテーブル */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -p option.\n");
		return 90;
	}
	rv = SetFileRscName(replTbl,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long REPTBL name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionG
//
// ABSTRACT:   g オプションの処理
//
// FUNCTION:
//	  g オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionG(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*gramTbl,			/* 文法テーブル、不要ならNULLが返る */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -g option.\n");
		return 50;
	}
	rv = SetFileRscName(gramTbl,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long GRAMTBL name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionI
//
// ABSTRACT:   i オプションの処理
//
// FUNCTION:
//	  i オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionI(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*gramTbl2,			/* 文法テーブル2、不要ならNULLが返る */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -i option.\n");
		return 50;
	}
	rv = SetFileRscName(gramTbl2,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long GRAMTBL2 name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionE
//
// ABSTRACT:   e オプションの処理
//
// FUNCTION:
//	  e オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionE(
	const int argc,	/* argv 配列の要素数 */
	const char *argv[],	/* 配列(要素はアドレス)の先頭アドレス */
	char *eMKTbl, /* 英語トークン文字種別テーブル、不要ならNULLが返る */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -e option.\n");
		return 55;
	}
	rv = SetFileRscName(eMKTbl,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long ENGMKTBL name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionF
//
// ABSTRACT:   f オプションの処理
//
// FUNCTION:
//	  f オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionF(
	const int argc,	/* argv 配列の要素数 */
	const char *argv[],	/* 配列(要素はアドレス)の先頭アドレス */
	char *eMKTbl2, /* 英語トークン文字種別テーブル2、不要ならNULLが返る */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -f option.\n");
		return 55;
	}
	rv = SetFileRscName(eMKTbl2,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long ENGMKTBL2 name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionU
//
// ABSTRACT:   u オプションの処理
//
// FUNCTION:
//	  u オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionU(
	const int argc,	/* argv 配列の要素数 */
	const char *argv[],	/* 配列(要素はアドレス)の先頭アドレス */
	char *uMKTbl,		/* 未登録語文字種別テーブル */
	char *uCTbl,		/* 未登録語コスト推定テーブル */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 3) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -u option.\n");
		return 55;
	}
	rv = SetFileRscName(uMKTbl,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long UNKMKTBL name.\n");
		return rv;
	}
	rv = SetFileRscName(uCTbl,argv[*processedArgc + 2]);
	if (rv != 0){
		fprintf(stdout, "Too long UNKCOSTTBL name.\n");
		return rv;
	}
	*processedArgc += 3;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionV
//
// ABSTRACT:   v オプションの処理
//
// FUNCTION:
//	  v オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionV(
	const int argc,	/* argv 配列の要素数 */
	const char *argv[],	/* 配列(要素はアドレス)の先頭アドレス */
	char *uMKTbl2,		/* 未登録語文字種別テーブル2 */
	char *uCTbl2,		/* 未登録語コスト推定テーブル2 */
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 3) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -v option.\n");
		return 55;
	}
	rv = SetFileRscName(uMKTbl2,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long UNKMKTBL2 name.\n");
		return rv;
	}
	rv = SetFileRscName(uCTbl2,argv[*processedArgc + 2]);
	if (rv != 0){
		fprintf(stdout, "Too long UNKCOSTTBL2 name.\n");
		return rv;
	}
	*processedArgc += 3;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionO
//
// ABSTRACT:   o オプションの処理
//
// FUNCTION:
//	  o オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionO(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	char	*outFile,			/* 出力ファイル */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	int	rv;

	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -o option.\n");
		return 60;
	}
	rv = SetFileRscName(outFile,argv[*processedArgc + 1]);
	if (rv != 0){
		fprintf(stdout, "Too long OUTFILE name.\n");
		return rv;
	}
	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  OptionT
//
// ABSTRACT:   t オプションの処理
//
// FUNCTION:
//	  t オプションが指定された時の処理を行う
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int OptionT(
	const int	argc,			/* argv 配列の要素数 */
	const char	*argv[],		/* 配列(要素はアドレス)の先頭アドレス */
	int	*processedArgc		/* 処理した引数の数(後カウントアップ) */
)
{	
	if (argc - *processedArgc < 2) { /* 残りの引数の数が足りない時 */
		fprintf(stdout, "Insufficient -t option.\n");
		return 70;
	}

	StopTime = atof(argv[*processedArgc + 1]);

	*processedArgc += 2;

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  SetFileRscName
//
// ABSTRACT:   ファイル名のセット
//
// FUNCTION:
//	  ファイル名の長さを検査し出力変数にセットする
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//
static int SetFileRscName(
	char		*outRscName,		/* ファイル名 */
	const char	*inRscName			/* コマンドラインの引数 */
	)
{
	if (strlen(inRscName) > UNA_FNAME_MAX) { /* ファイル名の長さチェック */
		return 80;
	}
	strcpy(outRscName,inRscName);	/* 検査OKなら出力変数にセット */

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:		ReadDic
//
// ABSTRACT:	辞書類の読み込み
//
// FUNCTION:
//	  形態素解析に必要な辞書、テーブルなどのリソースを読み込む
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//
static int ReadDic(
	char *rscDir,	 			/* リソースのあるディレクトリ */
	char *dicList,				/* 辞書リスト */
	char *repTbl, 				/* 文字置き換えテーブル */
	char *cnctTbl,	 			/* 接続表 */
	char *gramTbl,	 			/* 実行文法辞書 */
	char *eMKTbl,	 			/* 英語トークン用文字種別テーブル */
	char *uMKTbl,				/* 未登録語用文字種別テーブル */
	char *uCTbl,	 			/* 未登録語コスト推定テーブル */
	unaKApiDicImgT *dicImgList,/* 形態素辞書・アプリ情報のイメージ等 */
	int *dicCount,				/* dicImgListの要素数 */
	char **repTblImg,			/* 文字置き換えテーブルのイメージ */
	char **cnctTblImg,			/* 接続表のイメージ */
	char **gramTblImg,			/* 文法テーブルのイメージ */
	char **eMKTblImg,			/* 英語トークン用文字種テーブルのイメージ */
	char **uMKTblImg,			/* 未登録語用文字種テーブルのイメージ */
	char **uCTblImg				/* 未登録語コスト推定テーブルのイメージ */
)
{
	int rv;					/* 関数の返値 */
	char rscPath[FILENAME_MAX * 2 + sizeof(PATH_DELIM)];
	char fileName[FILENAME_MAX * 2 + sizeof(PATH_DELIM)];
	char *p, buf[BUFSIZ], base[BUFSIZ];
	int lineno, count;
	int n, prio, prevPrio;
	FILE *fp;

	/* 辞書リストに基づく形態素辞書・アプリ情報辞書の読み込み */
	strcpy(rscPath, rscDir);
	if (strcmp(&rscDir[strlen(rscDir) - 1],PATH_DELIM) != 0) {
		strcat(rscPath, PATH_DELIM);	/* デリミタの付与 */
	}
	strcpy(fileName, rscPath);
	strcat(fileName, dicList);
	if ((fp = fopen(fileName, "r")) == NULL) {
		/* 辞書リストがないときは unawrd.dic/unaapp2.dic の単一辞書構成とみなす */

		/* 辞書ベース名の保存 */
		dicImgList[0].dicName = strdup("una");

		/* 形態素辞書の読み込み */
		rv = ReadFileImg(rscDir, "unawrd.dic", &(dicImgList[0].morphDic));
		if (rv < 0) {
			fprintf(stdout,"CODE=%d ", rv);
			fprintf(stdout,"MorphDic ReadError (%s)\n", fileName);
			FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
			return 3;
		}

		/* アプリ情報辞書の読み込み */
		rv = ReadFileImg(rscDir, "unaapp2.dic", &(dicImgList[0].appInfo));
		if (rv < 0) {
			fprintf(stdout,"CODE=%d ", rv);
			fprintf(stdout,"AppInfo ReadError (%s)\n", fileName);
			FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
			return 4;
		}

		/* 辞書優先度の保存 */
		dicImgList[0].dicPrio = 1;

		*dicCount = 1;
	} else {
		lineno = 0;
		count = 0;
		prevPrio = 1;
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			lineno++;

			/* 行頭のBOMや空白を読み飛ばす */
			p = buf;
			if (lineno == 1 && memcmp(p, "\xef\xbb\xbf", 3) == 0) {
				p += 3;
			}
			while (*p && (*p == ' ' || *p == '\t')) {
				p++;
			}

			/* 注釈行や空行はスキップ */
			if (*p == '\0' || *p == '#' || *p == '\r' || *p == '\n') {
				continue;
			}

			/* 優先度と辞書ファイルベース名を読む */
			n = sscanf(p, "%d , %[^# \t\r\n]", &prio, base);
			if (n != 2) {
				/* 構文エラー */
				fclose(fp);
				fprintf(stdout,"dicList: Syntax Error\n");
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 2;
			}
			if (prio < 1 || prio > 255) {
				/* 優先度が1〜255の範囲にない */
				fclose(fp);
				fprintf(stdout, "dicList: Priority Out Of Range\n");
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 2;
			}
			if (prio < prevPrio) {
				/* 優先度が逆順 */
				fclose(fp);
				fprintf(stdout, "dicList: Priority Out Of Order\n");
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 2;
			}
			if (count >= UNA_MORPH_DIC_MAX) {
				/* 辞書数がUNA_MORPH_DIC_MAXを超えた */
				fclose(fp);
				fprintf(stdout, "dicList: Too Many Dictionaries\n");
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 2;
			}

			/* 辞書ベース名の保存 */
			dicImgList[count].dicName = strdup(base);

			/* 形態素辞書の読み込み */
			strcpy(fileName, base);
			strcat(fileName, "wrd.dic");
			rv = ReadFileImg(rscDir, fileName, &(dicImgList[count].morphDic));
			if (rv < 0) {
				fprintf(stdout,"CODE=%d ", rv);
				fprintf(stdout,"MorphDic ReadError (%s)\n", fileName);
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 3;
			}

			/* アプリ情報辞書の読み込み */
			strcpy(fileName, base);
			strcat(fileName, "app2.dic");
			rv = ReadFileImg(rscDir, fileName, &(dicImgList[count].appInfo));
			if (rv < 0) {
				fprintf(stdout,"CODE=%d ", rv);
				fprintf(stdout,"AppInfo ReadError (%s)\n", fileName);
				FreeDic(dicImgList, NULL, NULL, NULL, NULL, NULL, NULL);
				return 4;
			}

			/* 辞書優先度の保存 */
			dicImgList[count].dicPrio = prio;

			prevPrio = prio;
			count++;
		}
		fclose(fp);
		if (count == 0) {
			fprintf(stdout,"dicList: No Dictionaries Found\n");
			return 2;
		} 
		*dicCount = count;
	}

	/* 文字置き換えテーブルの読み込み */
	rv = ReadFileImg(rscDir,repTbl,repTblImg);
	if (rv < 0&& rv != UNA_FILE_NAME_IS_NULL) { /* エラーの時(ファイル名がNULLも含む) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"Char Replace Table Read Error(or File Name is NULL)\n");
		FreeDic(dicImgList,NULL,NULL,NULL,NULL,NULL,NULL);
		return 5;
	}

	/* 接続表の読み込み */
	rv = ReadFileImg(rscDir,cnctTbl,cnctTblImg);
	if (rv < 0) { /* エラーの時(ファイル名がNULLも含む) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"CnctTbl Read Error(or File Name is NULL)\n");
		FreeDic(dicImgList,*repTblImg,NULL,NULL,NULL,NULL,NULL);
		return 6;
	}

	/* 実行文法テーブルの読み込み */
	rv = ReadFileImg(rscDir,gramTbl,gramTblImg);
	if (rv < 0 && rv != UNA_FILE_NAME_IS_NULL) {
		/* エラーの時(ファイル名がNULLは仕様上有り得る) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"GramTbl Read Error\n");
		FreeDic(dicImgList,*repTblImg,*cnctTblImg,NULL,NULL,NULL,NULL);
		return 7;
	}
  
	/* 英語トークン用文字種別テーブルの読み込み */
	rv = ReadFileImg(rscDir,eMKTbl,eMKTblImg);
	if (rv < 0 && rv != UNA_FILE_NAME_IS_NULL) {
		/* エラーの時(ファイル名がNULLは仕様上有り得る) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"EngMKTbl Read Error\n");
		FreeDic(dicImgList,*repTblImg,*cnctTblImg,*gramTblImg,NULL,NULL,NULL);
		return 8;
	}
  
	/* 未登録語用文字種別テーブルの読み込み */
	rv = ReadFileImg(rscDir,uMKTbl,uMKTblImg);
	if (rv < 0) {
		/* エラーの時(ファイル名がNULLも含む) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"UnkMKTbl Read Error\n");
		FreeDic(dicImgList,*repTblImg,*cnctTblImg,*gramTblImg,*eMKTblImg,NULL,NULL);
		return 9;
	}
  
	/* 未登録語コスト推定テーブルの読み込み */
	rv = ReadFileImg(rscDir,uCTbl,uCTblImg);
	if (rv < 0) {
		/* エラーの時(ファイル名がNULLも含む) */
		fprintf(stdout,"CODE=%d ",rv);
		fprintf(stdout,"UnkCostTbl Read Error\n");
		FreeDic(dicImgList,*repTblImg,*cnctTblImg,*gramTblImg,*eMKTblImg,*uMKTblImg,NULL);
		return 10;
	}

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:		FreeDic
//
// ABSTRACT:	辞書類の解放
//
// FUNCTION:
//	  読み込んだ辞書リソースを解放する
//
// RETURN:
//	  0				正常終了
//
// NOTE:
//
static int FreeDic(
	unaKApiDicImgT *dicImgList,/* 形態素辞書・アプリ情報のイメージ等 */
	char *repTblImg,			/* 文字置き換えテーブルのイメージ */
	char *cnctTblImg,			/* 接続表のイメージ */
	char *gramTblImg,			/* 文法テーブルのイメージ */
	char *eMKTblImg,			/* 英語トークン用文字種テーブルのイメージ */
	char *uMKTblImg,			/* 未登録語用文字種テーブルのイメージ */
	char *uCTblImg				/* 未登録語コスト推定テーブルのイメージ */
)
{
	int i;

	if (dicImgList != NULL) {
		for (i = 0; i < UNA_MORPH_DIC_MAX; i++) {
			if (dicImgList[i].dicName != NULL) {
				free(dicImgList[i].dicName);
			}
			if (dicImgList[i].morphDic != NULL) {
				unaKApi_freeImg(dicImgList[i].morphDic);
			}
			if (dicImgList[i].appInfo != NULL) {
				unaKApi_freeImg(dicImgList[i].appInfo);
			}
		}
	}
	if (repTblImg != NULL) {
		unaKApi_freeImg(repTblImg);
	}
	if (cnctTblImg != NULL) {
		unaKApi_freeImg(cnctTblImg);
	}
	if (gramTblImg != NULL) {
		unaKApi_freeImg(gramTblImg);
	}
	if (eMKTblImg != NULL) {
		unaKApi_freeImg(eMKTblImg);
	}
	if (uMKTblImg != NULL) {
		unaKApi_freeImg(uMKTblImg);
	}
	if (uCTblImg != NULL) {
		unaKApi_freeImg(uCTblImg);
	}
	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadFileImg
//
// ABSTRACT:	  辞書のメモリへの読み込み
//
// FUNCTION:
//	  ファイルサイズにあったメモリを確保し、そのメモリにファイルを
//	  読み込んでマップする(下位関数を呼ぶ事により実現している)
//
// RETURN:
//		UNA_OK					正常終了
//		UNA_FILE_NAME_IS_NULL	ファイル名がNULLである
//		その他					エラー
//
// NOTE:
//	  返り値は、下位関数のものである。
//	  ファイルサイズ分のメモリが確保される。
//	  ファイル名がNULLの場合、mapAddressにもNULLがセットされ返る
//
static int ReadFileImg(
	const char *rscDir,	  /* 辞書のあるディレクトリ */
	const char *fileName, /* 読み込んで欲しいファイル名(ディレクトリ無し) */
	char **mapAddress	  /* マップしたアドレス */
	)
{
	int rv;								/* 関数の返り値 */
	char wholeFileName[FILENAME_MAX * 2 + sizeof(PATH_DELIM)];
											/* <stdio.h>参照 */

	/* 入力引数チェック */
	if (*fileName == '\0') { /* ファイル名がNULLの時 */
		wholeFileName[0] = '\0';
	}
	else {
		/* ディレクトリ付きのファイル名を作る */
		strcpy(wholeFileName,rscDir);
		if (strcmp(&rscDir[strlen(rscDir) - 1],PATH_DELIM) != 0) {
			strcat(wholeFileName,PATH_DELIM);	/* デリミタの付与 */
		}
		strcat(wholeFileName,fileName);
	}

	rv = unaKApi_readFileImg(wholeFileName,mapAddress);

	/* 下位関数の返り値をそのまま返す */
	return rv;
}


#if defined(UNAMLTEST) && !defined(UNAHTTEST)
//--------------------------------------------------------------------------
// MODULE:		TstTread
//
// ABSTRACT:	マルチスレッドテスト
//
// FUNCTION:
//	  マルチスレッドのテストを行う
//
// RETURN:
//	  なし
//
// NOTE:
//	  - スレッドのテストでは、指定されたテキストのうち最初のものだけを
//		解析対象とする
//	  - スレッドのテストは、耐久テストと、解析結果比較テストの2つがある
//
static void TstThread(
	char *inFile,				 /* 入力ファイル */
	unaKApiDicImgT **dicImgList, /* 形態素辞書, アプリ情報辞書のイメージ等 */
	int *dicCount,				 /* dicImgListの要素数 */
	char **repTblImg,			 /* 文字置き換えテーブルのイメージ */
	char **cnctTblImg,			 /* 接続表のイメージ */
	char **gramTblImg,			 /* 文法テーブルのイメージ */
	char **eMKTblImg,			 /* 英語トークン用文字種テーブルのイメージ */
	char **uMKTblImg,			 /* 未登録語用文字種テーブルのイメージ */
	char **uCTblImg,			 /* 未登録語コスト推定テーブルのイメージ */
	char *outFile,				 /* 出力ファイル */
	int outputHyoki,			 /* 表記を出力するかのフラグ */
	int outputHyokiOnly,			 /* 表記のみ出力するかのフラグ */
	int outputUnaHin,			 /* UNA品詞を出力するかのフラグ */
	int outputSubMorph,		 /* 下位形態素を出力するかのフラグ */
	int connectInfo,			 /* 出力情報を繋げて出力するかのフラグ */
	int outputTargetTxt,		 /* 解析対象テキストを表示するかのフラグ*/
	int inputByLF,				 /* 復改の単位で読み込むかのフラグ */
	int threadNum				 /* 作成するスレッドの数 */
)
{
#ifdef _WIN32
	DWORD tid;					 /* スレッドID */
	HANDLE threadHandle;		 /* スレッド用ハンドラ */
#else
	#if defined(UNALINUX)
		pthread_t tid;			/* linux */
	#else
		thread_t tid;			/* solaris */
	#endif
	int threadHandle;
#endif
	TCB tcb[THREAD_NUM];		 /* スレッドコントロールブロック */
	int j;						 /* ループ変数 */

	/* スレッドによる実行 */
	for (j = 0;j < threadNum;j++){ /* thredNum の個数だけ作成される */
		tcb[j].inFile			= inFile;
		tcb[j].dicImgList		= *dicImgList;
		tcb[j].dicCount			= *dicCount;
		tcb[j].repTblImg		= *repTblImg;
		tcb[j].cnctTblImg		= *cnctTblImg;
		tcb[j].gramTblImg		= *gramTblImg;
		tcb[j].eMKTblImg		= *eMKTblImg;
		tcb[j].uMKTblImg		= *uMKTblImg;
		tcb[j].uCTblImg			= *uCTblImg;
		tcb[j].outputHyoki		= outputHyoki;
		tcb[j].outputHyokiOnly		= outputHyokiOnly;
		tcb[j].outputUnaHin		= outputUnaHin;
		tcb[j].outputSubMorph	= outputSubMorph;
		tcb[j].connectInfo		= connectInfo;
		tcb[j].outputTargetTxt	= outputTargetTxt;
		tcb[j].inputByLF		= inputByLF;
		tcb[j].outputHinName	= outputHinName;
		sprintf(tcb[j].outFile,"%s%02d00.txt",outFile,j);
		#ifdef _WIN32
			threadHandle = CreateThread(
				NULL,			 /* SECURITY_ATTRIBUTES構造体へのポインタ */
				10000,			 /* スタックサイズ(10K) */
				Threadize,		 /* スレッドとして実行する関数 */
				(LPVOID)&tcb[j], /* スレッドに渡す32ビット値 */
				0,		/* CREATE_SUSPENDEDだとサスペンド状態で作成される*/
				&tid	/* ここにスレッドidが格納される */
				);
			if (threadHandle == 0) { /* 失敗 */
				perror("thread");
				fprintf(stdout,"CreateThread failed(No=%d)\n",j);
			}
		#else
			#if defined(UNALINUX)
				threadHandle = pthread_create(&tid,NULL,Threadize,
											(void *)&tcb[j]);
			#else
				threadHandle = thr_create(NULL,NULL,Threadize,
											(void *)&tcb[j],0,&tid);
			#endif
			if (threadHandle != 0) {
				perror("thread");
				fprintf(stdout,"CreateThread failed(No=%d)\n",j);
			}
		#endif
		if (threadNum == 2) {
			++dicImgList;
			++dicCount;
			++repTblImg;
			++cnctTblImg;
			++gramTblImg;
			++eMKTblImg;
			++uMKTblImg;
			++uCTblImg;
		}
	}

	/* 全てのスレッドが終了するまで待つ */
	while (EndThread < threadNum) {
	#ifdef _WIN32
			Sleep(1000);	/* 1000milliseconds */
	#else
			sleep(1);		/* 1seconds */
	#endif
	}

	return;
}
#endif	/* end of function */


#if defined(UNAMLTEST) && !defined(UNAHTTEST)
//--------------------------------------------------------------------------
// MODULE:	  Threadize
//
// ABSTRACT:   スレッド化
//
// FUNCTION:
//	  この関数がスレッドとして実行される
//
// RETURN:
//	  NULL	(正常も異常も)
//
// NOTE:
//
#ifdef _WIN32
DWORD WINAPI Threadize(
	LPVOID arg
)
#else
void *Threadize(
	void *arg					/* スレッドの引数 */
	)
#endif

{
	TCB *tcb;				/* スレッドコントロールブロック */
	unsigned short uniChar;	/* ユニコード1文字 */
	int rv;				/* 関数の戻り値 */
	FILE *fp;				/* 入力ファイル */
	FILE *fo;				/* 出力ファイル */

	tcb = (TCB *)arg;

	/* 少し他のスレッドが動くのを待つ */
	#ifdef _WIN32
		Sleep(1000);	/* 1000milliseconds */
	#else
		sleep(1);		/* 1seconds */
	#endif
	/* 入力ファイルのオープン */
	if ((fp = fopen(tcb->inFile,"rb")) == NULL) {
		fprintf(stdout,"Can't read %s\n",tcb->inFile);
		EndThread++;
		return NULL;
	}

	/* 出力ファイルのオープン */
	if ((fo = fopen(tcb->outFile, "wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",tcb->outFile);
		EndThread++;
		return NULL;
	}

	uniChar = 0xFEFF;	/* 予めBOMを書き込む */
	fwrite(&uniChar,sizeof(uniChar),1,fo);

	rv = DoKaiseki(fp,tcb->dicImgList, tcb->dicCount,
				   tcb->repTblImg, tcb->cnctTblImg, tcb->gramTblImg,tcb->eMKTblImg,
				   tcb->uMKTblImg, tcb->uCTblImg,tcb->outputHyoki,
				   tcb->outputHyokiOnly,
				   tcb->outputUnaHin,tcb->outputSubMorph,tcb->connectInfo,
				   tcb->outputTargetTxt, tcb->inputByLF,tcb->outputHinName,fo);

	/* 入力ファイルのクローズ */
	fclose(fp);

	/* 出力ファイルのクローズ */
	fclose(fo);

	/* 終了した事をメインに伝える */
	EndThread++;	/* 途中終了の場合も EndThread は増やす */

	/* 終了(正常も異常も) */
	return NULL;
}
#endif	/* end fo function */


//--------------------------------------------------------------------------
// MODULE:	  DoKaiseki
//
// ABSTRACT:   解析の実行
//
// FUNCTION:
//	  解析対象ファイルを読み込み形態素解析又は係り受け解析を実行する。
//	  結果は出力ファイルに出力される。
//	  この機能は下位のAPIモジュールを呼び出すことにより、実現している。
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//
static int DoKaiseki(
	FILE *fp,				    /* 解析対象ファイル */
	const unaKApiDicImgT *dicImgList,	/* 形態素辞書,アプリ情報辞書のイメージ等 */
	const int dicCount,			/* dicImgListの要素数 */
	const char *repTblImg,    	/* 文字置き換えテーブルのイメージ */
	const char *cnctTblImg,	    /* 接続表のイメージ */
	const char *gramTblImg,	    /* 文法テーブルのイメージ */
	const char *eMKTblImg,	    /* 英語トークン用文字種別テーブルのイメージ*/
	const char *uMKTblImg,	    /* 未登録語用文字種別テーブルのイメージ*/
	const char *uCTblImg,	    /* 未登録語コスト推定テーブルのイメージ*/
	const int outputHyoki,	    /* 表記を出力するかのフラグ */
	const int outputHyokiOnly,  /* 表記のみ出力するかのフラグ */
	const int outputUnaHin,    /* UNA品詞を出力するかのフラグ */
	const int outputSubMorph,  /* 下位形態素を出力するかのフラグ */
	const int connectInfo,	    /* 出力情報を繋げて出力するかのフラグ */
	const int outputTargetTxt, /* 解析対象テキストを表示するかのフラグ */
	const int inputByLF,		/* 復改の単位で読み込むかのフラグ */
	const int outputHinName,	/* 品詞名を出力するかどうかのフラグ */
	FILE *fo				    /* 出力ファイル */
)
{
	unaKApiHandleT h;					/* ハンドラ */
	int rv;							/* UNA関数の戻り値 */
	unaCharT inTxtBuf[INTEXT_BUFSIZ];	/* 入力テキスト用バッファ */
	int n;								/* テキスト1行の文字数 */
	unaCharT *inTxtPtr;					/* 処理する文字列のアドレス */
	int inTxtLen;		 				/* テキスト長(文字数) */
	unaMorphT morphBuf[LOCAL_TEXT_SIZE];
									/* 形態素解析結果が書かれるバッファ*/
	int morphNum;					/* 書かれた形態素の数 */
	unaBnsT bnsBuf[LOCAL_BNS_SIZE];	/* かかりうけ解析結果が書かれるバッファ*/
	int bnsNum;					/* 書かれた文節の数 */
	int processedTxtLen;			/* 解析済のテキストの長さ(文字数) */
	unaCharT uniChar;				/* ユニコード1文字 */
	unaCharT uniChar2;				/* ユニコード1文字 */
	int (*readF)(unaCharT *buf,int maxBufLen,FILE *fp);
									/* テキスト読み込み関数 */
#ifndef UNASPTEST
	ucharT ansBuf[OUTTEXT_BUFSIZ];	/* 形態素情報出力用バッファ */
	ucharT *ansPtr;					/* 形態素情報出力用バッファのポインタ */
#endif

	/* 初期値設定 */
	uniChar = 0x000A;			/* LF */
	uniChar2 = 0x0022;			/* " */

	if (inputByLF == YES) {		/* LF区切り毎に読み込む指示あり */
		readF = Fgetws;			/* LF区切り毎に読み込む関数を使う */
	}
	else {
		readF = Getws;			/* 句点区切り毎に読み込む関数を使う */
	}

	/* 検索用APIの初期化 */
	rv = unaKApi_init(&h,dicImgList,dicCount,cnctTblImg,
					gramTblImg,eMKTblImg,uMKTblImg,uCTblImg,
					repTblImg,0);
	if (rv < 0) {
		fprintf(stdout,"CODE=%d API Initialize Error\n",rv);
		return 100;
	}

	/* 形態素解析モジュールの動作を設定する */
	/* AnadbgFlagがYESならanadbg用のラティス情報出力を行う */
	unaMorph_setDebugFlag(AnadbgFlag == YES? 2: 0);

	/* ループ前初期設定 */
#ifndef UNALNTEST
	inTxtLen = -1;						/* NULLターミネート文字として解析 */
#endif

	/* 検索用APIを用いて解析を実行 */
	while ((n = readF(inTxtBuf,sizeof(inTxtBuf)/sizeof(unaCharT),fp)) >= 0){

#ifdef UNALNTEST
		inTxtLen = n;					/* テキスト実長をセット */
#endif

		if (gramTblImg == (const char *)NULL && (outputTargetTxt == YES)) {
			/* 解析対象となる1行をまず表示 */
			fwrite(&uniChar2,sizeof(uniChar),1,fo);
			fwrite(&inTxtBuf,sizeof(unaCharT),n,fo);
			fwrite(&uniChar2,sizeof(uniChar),1,fo);
			fwrite(&uniChar,sizeof(uniChar),1,fo);
		}

		/* 読み込んだ一行に対して解析を行う */
		inTxtPtr = inTxtBuf;			/* ポインタの初期設定 */
		while(*inTxtPtr) {				/* NULL文字に突き当たるまで */
		/*
		 * 入力テキストは一度に全てが解析されるわけではなく、
		 * 途中迄解析され、戻る場合があるのでこのループが必要
		 */
			if (gramTblImg == (const char *)NULL) {
				/* 文法テーブル指定無しなら形態素解析 */
				rv = unaKApi_moAna(&h,inTxtPtr,inTxtLen,morphBuf,&morphNum,
						LOCAL_TEXT_SIZE,&processedTxtLen,StopByTime,UNA_TRUE,UNA_FALSE,UNA_FALSE);
				if (rv < 0) {
					fprintf(stdout,"CODE=%d KEITAISO-KAISEKI Error\n",rv);
					if (rv == UNA_STOP) {
						return rv;
					}
					else {
						return 101;
					}
				}
#ifdef UNAOPENSUBMORPH
				{
					int yy;
					int rvt;
					int subMorNum;
					unaMorphT subMorBuf[LOCAL_TEXT_SIZE];
					
					/* 下位構造を取得する(だけ) */
					for ( yy = 0; yy < rv; ++yy){

						rvt = unaKApi_getSubMorph(
								&h,&morphBuf[yy],subMorBuf,&subMorNum,
								UNA_LOCAL_TEXT_SIZE);
					}
				}
#endif
#ifndef UNASPTEST
				/* 形態素に分割されたとこまでの形態素情報を得る */
				ansPtr = ansBuf;
				rv = OutputMorphInfo(&h,morphBuf,morphNum,&ansPtr,
									sizeof(ansBuf),outputHyoki,
									outputHyokiOnly,outputUnaHin,
									connectInfo,outputSubMorph,
									outputHinName,0,fo);
				if (rv != 0) {
					return rv;
				}
#endif
			}
			else{
				/* 文法テーブル指定ありならかかりうけ解析 */
				rv = unaKApi_kuAna(&h,inTxtPtr,inTxtLen,morphBuf,
						&morphNum,LOCAL_TEXT_SIZE,bnsBuf, &bnsNum,
						LOCAL_BNS_SIZE, &processedTxtLen,StopByTime,UNA_FALSE,UNA_FALSE);
				if (rv < 0) {
					fprintf(stdout,"CODE=%d KAKARIUKE-KAISEKI Error\n",rv);
					if (rv == UNA_STOP) {
						return rv;
					}
					else {
						return 102;
					}
				}
				/* かかりうけ解析されたとこまでの形態素情報を得る */
				rv = OutputBnsInfo(&h,bnsBuf,bnsNum,outputHyoki,
									outputHyokiOnly,outputUnaHin,fo);
				if (rv != 0) {
					return rv;
				}
			}
			/* 次のループ(すぐ外の)での入力文字列の開始位置 */
			inTxtPtr = inTxtPtr + processedTxtLen;
		}
		if (gramTblImg == (const char *)NULL 
		&& (outputTargetTxt == YES || connectInfo == YES)) { /* 改行する */
			fwrite(&uniChar,sizeof(uniChar),1,fo);
		}
	}
	/* 検索用APIの終了 */
	unaKApi_term(&h);

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  Getws
//
// ABSTRACT:   ユニコードデータを読み込む
//
// FUNCTION:
//	  ユニコード形式のファイルからデータを読み込む。
//	  以下の特徴がある。
//	  ・ファイル fp から maxBufLen-1 文字まで、または、句点記号まで、または
//		ファイルエンドコード(0x001A)までの文字列を読み込む。
//		CR(0x000D)、LF(0x000A) も読み込む。
//		読み込んだ文字列の最後に、NULL(0x0000) を付加する。
//	  ・テキストデータの範疇を超えるが、NULL(0x0000) が存在した場合にも、
//	    そこまでのデータを読み込む。NULL も読み込む。
//	  ・BOM(0xFEFF)は、無視する。
//
// RETURN:
//	  負値			エラーまたはファイルエンド
//	  その他		読み込んだ文字数(ターミネータのNULLの数は含まない)
//
// NOTE:
//	  ・バイナリモードでオープンすること
//	  ・空行(リターンコードだけの行)を読み込んだ場合は、バッファ先頭には、
//		NULL(0x0000)が書き込まれ RETURNコードには、0 が返る。
//	  ・EOF条件に、RETURNコードが負値かどうかの判定を必要とするため、
//		RETURN コードを unsigned で受けてはならない。
//
static int Getws(
	unaCharT *buf,			/* UNICODE 文字列の読み込まれるバッファ */
	int	maxBufLen,		/* 文字列の最大長(NULLターミネータも含めて) */
	FILE	*fp				/* ファイル */
)
{
	int i;					/* ループ変数 */
	int n;					/* 読み込まれた個数 */
	int lfFlg;				/* LFフラグ、空行でないLFの時1、それ以外0 */

	/* 初期設定 */
	lfFlg = 0;

	for (i = 0 ; i < maxBufLen - 1 ;){
		n = fread(&(buf[i]),sizeof(unsigned short),1,fp);

		if(n < 1){		/* (n < 第3引数)は、EOF。fread の仕様より */
			if(i == 0){
				return -1;
			}
			else {
				break;
			}
		}

		switch(buf[i]){
		case 0x000A:	/* LF */
			if (lfFlg == 1) {	/* 空行の時 */
				buf[i] = 0x0000;	/* LF に NULL を上書きする */
				return i; 
			}
			else {
				lfFlg = 1;
				i++;
			}
			break;
		case 0x000D:	/* CR */
			if (lfFlg == 0) {	/* 空行ではない時 */
				i++;
			}
			break;
		case 0x3002:	/* 。(句点記号) */
		case 0xFF01:	/* ！(句点記号) */
		case 0xFF1F:	/* ？(句点記号) */
		case 0xFF61:	/* 半角句点記号) */
			buf[i + 1] = 0x0000;	/* ヌルターミネート */
			return i + 1;
		case 0xFEFF:	/* BOM */
			lfFlg = 0;
			break;				/* 読み飛ばし。次のデータが上書きされる */
		case 0x001A:	/* コードとしてのEOF */
			buf[i] = 0x0000;	/* LF に NULL を上書きする */
			return i; 
		case 0x0000:	/* NULL */
			return i;
		default:
			lfFlg = 0;
			i++;
		}
	}
	buf[i] =0x0000;		/* for条件又はEOF抜けの場合最後にNULLターミネート */
	return i;
}


//--------------------------------------------------------------------------
// MODULE:	  Fgetws
//
// ABSTRACT:   ユニコードデータを読み込む
//
// FUNCTION:
//	  ユニコード形式のファイルからデータを読み込む。
//	  C の標準関数の fgets に似ているが、以下の特徴がある。
//	  ・ファイル fp から maxBufLen-1 文字まで、または 復改文字までの文字列を
//	    読み込む。但し、LF(0x000A) は読み込まない。
//		読み込んだ文字列の最後に、NULL(0x0000) を付加する。
//	  ・ファイルエンドコード(0x001A)は、LF(0x000A) と同様の扱いをする。
//	  ・テキストデータの範疇を超えるが、NULL(0x0000) が存在した場合にも、
//	    そこまでのデータを読み込む。NULL も読み込む。
//	  ・BOM(0xFEFF) 及び CR(0x000D)は、無視する。
//
// RETURN:
//	  負値			エラーまたはファイルエンド
//	  その他		読み込んだ文字数(ターミネータのNULLの数は含まない)
//
// NOTE:
//	  ・バイナリモードでオープンすること
//	  ・空行(リターンコードだけの行)を読み込んだ場合は、バッファ先頭には、
//		NULL(0x0000)が書き込まれ RETURNコードには、0 が返る。
//	  ・EOF条件に、RETURNコードが負値かどうかの判定を必要とするため、
//		RETURN コードを unsigned で受けてはならない。
//
static int Fgetws(
	unaCharT *buf,			/* UNICODE 文字列の読み込まれるバッファ */
	int	maxBufLen,		/* 文字列の最大長(NULLターミネータも含めて) */
	FILE	*fp				/* ファイル */
		)
{
	int i;					/* ループ変数 */
	int n;					/* 読み込まれた個数 */

	for (i = 0 ; i < maxBufLen - 1 ;){
		n = fread(&(buf[i]),sizeof(unsigned short),1,fp);

		if(n < 1){		/* (n < 第3引数)は、EOF。fread の仕様より */
			if(i == 0){
				return -1;
			}
			else {
				break;
			}
		}

		switch(buf[i]){
		case 0x001A:	/* コードとしてのEOF。しかし、復改と同じ扱いをする */
		case 0x000A:	/* LF */
			buf[i] = 0x0000;	/* LF に NULL を上書きする */
			return i; 
		case 0xFEFF:	/* BOM */
		case 0x000D:	/* CR */
			continue;			/* 読み飛ばし。次のデータが上書きされる */
		case 0x0000:	/* NULL */
			return i;
		default:
			i++;
		}
	}
	buf[i] =0x0000;		/* for条件又はEOF抜けの場合最後にNULLターミネート */
	return i;
}


//--------------------------------------------------------------------------
// MODULE:	  OutputMorphInfo
//
// ABSTRACT:   形態素情報の出力
//
// FUNCTION:
//	  形態素情報を出力する
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  再帰関数である
//
static int OutputMorphInfo(
	unaKApiHandleT *h,			 /* ハンドラ */
	const unaMorphT *morphBuf,	 /* 形態素の書かれているバッファ */
	const int morphNum,		 /* 形態素の数 */
	ucharT	   **ansBuf,		 /* 形態素情報出力用バッファ */
	int	   ansBufSize,		 /* 形態素情報出力用バッファのサイズ */
	const int outputHyoki,		 /* 表記を出力するかのフラグ */
	const int outputHyokiOnly,	 /* 表記のみ出力するかのフラグ */
	const int outputUnaHin,	 /* UNA品詞を出力するかのフラグ */
	const int connectInfo,		 /* 出力情報を繋げて出力するかのフラグ */
	const int outputSubMorph,	 /* 下位形態素を出力するかのフラグ */
	const int outputHinName,	 /* 品詞名を出力するかのフラグ */
	int	   level,			 /* 再帰のレベル(0==最上位) */
	FILE *fo					 /* 出力ファイル */
	)
{
	int rv;					/* UNA関数の戻り値 */
	int i;						/* ループ変数 */
	unaCharT *hyokiPtr;			/* 表記のアドレス */
	uintT hyokiLen;			/* 表記の長さ(文字数) */
	int appInfByte;			/* アプリケーション情報の長さ(byte) */
	unaHinT unaHin;				/* UNA品詞 */
	ucharT *ansPtr;				/* 形態素情報出力用バッファのポインタ */
	unaCharT uniChar;			/* ユニコード1文字 */
	unaMorphT subMorBuf[UNA_LOCAL_TEXT_SIZE]; /* サイズは英語トークンを考慮*/
								/* 下位形態素の書かれているバッファ */
	int subMorNum;				/* 下位形態素の数 */

	if (level != 0) {
		ansPtr = *ansBuf;		/* 下位形態素用にポインタをリセット */
	}
	
	/* このループ1回で1形態素の情報が出力される */
	for (i = 0; i < morphNum; i++) {

		/* 再帰の最上位にいる時の処理 */
		if (level == 0) {
			ansPtr = *ansBuf;	/* 1形態素用にポインタをリセット */
			if (connectInfo == YES){	/* 繋げて出力する時 */  
				uniChar = 0x002F;			/* '/' */
				memcpy(ansPtr,&uniChar,sizeof(uniChar));
				ansPtr += sizeof(uniChar);
			}
		}
		else {	/* 下位形態素を処理中の時 */
			*(unaCharT *)ansPtr = 0x0025;	/* '%' */
			ansPtr += sizeof(unaCharT);
			rv = Ltou(level,(unaCharT *)ansPtr);
			ansPtr += (rv * sizeof(unaCharT));
			*(unaCharT *)ansPtr = 0x003E;	/* '>' */
			ansPtr += sizeof(unaCharT);
		}

		/* 出力用に加工されたアプリ情報の表示 */
		rv = GetAppInfo(h,&morphBuf[i],ansPtr,*ansBuf + ansBufSize - ansPtr,
																&appInfByte);
		if (rv != 0) {
			fprintf(stdout,"CODE=%d GetAppInfo Err\n",rv);
			return 110;
		}
		ansPtr += appInfByte;

		if (outputHyoki == YES){	/* 表記の出力指示あり */
			/* 表記の取得 */
			rv = unaKApi_getHyoki(h,&morphBuf[i],&hyokiPtr,&hyokiLen);
			if (rv != UNA_OK) {
				fprintf(stdout,"CODE=%d Get Hyoki Error\n",rv);
				return 120;
			}
			*(unaCharT *)ansPtr = 0x0040;		/* '@' で区切る */
			ansPtr += sizeof(unaCharT);
			memcpy(ansPtr,hyokiPtr,hyokiLen * sizeof(unaCharT));
			ansPtr += (hyokiLen * sizeof(unaCharT));
		}

		if (outputHyokiOnly == YES){	/* 表記のみの出力指示あり */
			/* 表記の取得 */
			rv = unaKApi_getHyoki(h,&morphBuf[i],&hyokiPtr,&hyokiLen);
			if (rv != UNA_OK) {
				fprintf(stdout,"CODE=%d Get Hyoki Error\n",rv);
				return 120;
			}
			ansPtr -= appInfByte; /* アプリ情報をキャンセル */
			memcpy(ansPtr,hyokiPtr,hyokiLen * sizeof(unaCharT));
			ansPtr += (hyokiLen * sizeof(unaCharT));
		}

		if (outputUnaHin == YES){	/* UNA品詞の出力指示あり */
			/* UNA品詞の取得 */
			rv = unaKApi_getUnaHin(h,&morphBuf[i],&unaHin);
			if (rv != UNA_OK) {
				fprintf(stdout,"CODE=%d Get UNA Hinshi Error\n",rv);
				return 130;
			}
			*(unaCharT *)ansPtr = 0x0028;		/* '(' */
			ansPtr += sizeof(unaCharT);
			rv = Ltou(unaHin,(unaCharT *)ansPtr);
			ansPtr += (rv * sizeof(unaCharT));
			*(unaCharT *)ansPtr = 0x0029;		/* ')' */
			ansPtr += sizeof(unaCharT);
		}

		if (outputHinName == YES){	/* 品詞名の出力指示あり */
			*(unaCharT *)ansPtr = 0x0028;		/* '(' */
			ansPtr += sizeof(unaCharT);
			const unaCharT *nn;
			nn = unaKApi_getHinName(h,&morphBuf[i]);
			for ( ; (*nn)!=0; ++nn){
				*(unaCharT *)ansPtr = *nn;
				ansPtr += sizeof(unaCharT);
			}
			*(unaCharT *)ansPtr = 0x0029;		/* ')' */
			ansPtr += sizeof(unaCharT);
		}

		if (outputSubMorph == YES) {	/* 下位形態素の出力指示あり */
			/* 下位構造を取得する */
			rv = unaKApi_getSubMorph(h,&morphBuf[i],subMorBuf,&subMorNum,
													UNA_LOCAL_TEXT_SIZE);
			if (rv == UNA_KNOWN_WORD) {
				if (subMorNum != 0) {
					/* 登録語で下位構造あり */
					rv = OutputMorphInfo(h,subMorBuf,subMorNum,&ansPtr,
							*ansBuf + ansBufSize - ansPtr,outputHyoki,
							outputHyokiOnly,
							outputUnaHin,connectInfo,outputSubMorph,
							outputHinName, level + 1,fo);
					if (rv != 0) {
						return rv;
					}
				}
			}
			else if (rv != UNA_ENG_TOKEN && rv != UNA_UNKNOWN_WORD) {
				fprintf(stdout,"CODE=%d getSubMorph Error\n",rv);
				return 121;
			}
		}

		/* 再帰の最上位にいる時の処理 */
		if (level == 0) {	/* 最上位 */
			if (connectInfo == YES) {	/* 繋げて出力する時 */  
				uniChar = 0x002F;			/* '/' */
				memcpy(ansPtr,&uniChar,sizeof(uniChar));
				ansPtr += sizeof(uniChar);
			}
			else {	/* 改行する時(繋げて出力しない時) */
				uniChar = 0x000A;		   /* LF */
				memcpy(ansPtr,&uniChar,sizeof(uniChar));
				ansPtr += sizeof(uniChar);
			}
			fwrite(*ansBuf,ansPtr - *ansBuf,1,fo);
										/* ansPtr - *ansBuf は、バイト長 */
		}
	}

	*ansBuf	= ansPtr;	/* 上位関数用にポインタをリセット */

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  Ltou
//
// ABSTRACT:   int型データのUNICODE文字列への変換
//
// FUNCTION:
//	  int型データをUNICODE文字列に変換する
//
// RETURN:
//	  変換後のUNICODE文字列の文字数
//
// NOTE:
//	  ・バッファの桁は十分に取ること(11桁以上必要)
//	  ・現在は、正数のみに対応している。(負数は変換できない)
//	  ・文字長を返すが、ヌルターミネートもされる
//
static int Ltou(
		int x,				/* 数値 */
		unaCharT *buf		/* 変換された UNICODE 文字列 */
)
{
	int i;					/* ループ変数(tmpBuf の桁) */
	int j;					/* ループ変数(buf の桁) */
	int y;					/* x の 1/10 の数(切り捨て) */
	unaCharT tmpBuf[11];	/* 逆さになった文字列がセットされる */

	y = 1;
	for (i = 0; y != 0; i++){
		y = x / 10;
		tmpBuf[i] = (unaCharT)(0x0030 + (x - y * 10));
		x = y;
	}
	/* 上記 for ループを抜けた時点で、i には文字列長がセットされている */
	for (j = 0;j < i;j++){
		buf[j] = tmpBuf[(i - 1) - j];	/* (i - 1)は、要素番号の最大値 */
	}
	buf[i] = 0x0000;					/* ヌルターミネート */

	return i;
}


//--------------------------------------------------------------------------
// MODULE:    OutputBnsInfo
//
// ABSTRACT:   かかりうけ情報の出力
//
// FUNCTION:
//    かかりうけ情報を出力する
//
// RETURN:
//    0        正常終了
//    その他    エラー
//
// NOTE:
//	  以下のような感じで出力される
//	  なお、かかりうけシンボルに先立つ、"└"、"│"、"─"、"　"、等の部分を
//	  便宜上、チャート部と呼ぶ
//
//	  無関：諸民族が
//	  └複合：連邦
//	  　└強体：中央に
//	  　　│無関：抱く
//	  　　└└無関：不信感も
//	  　　　　└強用：抜き
//	  　　　　　│無関：さしならぬ
//	  　　　　　│└無関：ものに
//	  　　　　　└─└無関：なった。
//
static int OutputBnsInfo(
	unaKApiHandleT *h,			/* ハンドラ */
	const unaBnsT *bnsBuf,		/* 文節の書かれているバッファ */
	const int bnsNum,			/* 文節の数 */
	const int outputHyoki,		/* 表記を出力するかのフラグ */
	const int outputHyokiOnly,	/* 表記のみ出力するかのフラグ */
	const int outputUnaHin,	/* UNA品詞を出力するかのフラグ */
	FILE *fo					/* 出力ファイル */
)
{
	int rv;						/* UNA関数の戻り値 */
	int i;							/* ループ変数(0起算、文節) */
	int j;							/* ループ変数(0起算、チャートの桁) */
	ucharT ansBuf[OUTTEXT_BUFSIZ];	/* 形態素情報出力用バッファ */
	ucharT *ansPtr;					/* 形態素情報出力用バッファのポインタ */
	unaCharT ans[OUTTEXT_BUFSIZ];	/* かかりうけシンボル迄の出力用 */
	int vLine[LOCAL_BNS_SIZE];		/* "│"又は"└" を出力するため */
	int hLineFlg;					/* "└"以降の桁で"─"を出力するため */

	for (i = 0; i < bnsNum; i++){	/* 文節の数だけループ */

		/*
		 * 以下、i 番目の文節について1行出力を行う
		 */

		/* かかりうけ状態を計算し、チャート部を出力する */
		vLine[i] = bnsBuf[i].target - i;	/* "│"又は"└" の出力残行数 */

		hLineFlg = NO;				/* ループ前初期設定 */
		for ( j = 0; j < i; j++) {	/* i = 0 の時はバイパス */
			vLine[j]--;				/* マイナスになっても問題はない */
			if (vLine[j] > 0){
				ans[j] = 0x2502;	/* "│" */
			}
			else if (vLine[j] == 0) {
				ans[j] = 0x2514;	/* "└" */
				hLineFlg = YES;	/* 以降の桁で"　"なら"─"と置き換える */
			}
			else {
				if (hLineFlg == NO){
					ans[j] = 0x3000;	/* "　" */
				}
				else {
					ans[j] = 0x2500;	/* "─" */
				}
			}
		}

		fwrite(ans,j * sizeof(unaCharT),1,fo);

		/* かかりうけシンボルをセットする */
		rv = SetKakaUkeSymbol(bnsBuf[i].kuRel,ans);
		if (rv != 0){
			return rv;
		}

		fwrite(ans,sizeof(unaCharT),3,fo);

		/* 形態素をつなげて出力(即ち文節の形で) */
		ansPtr = ansBuf;
		rv = OutputMorphInfo(h, bnsBuf[i].start, bnsBuf[i].len,&ansPtr,
					sizeof(ansBuf),outputHyoki,outputHyokiOnly,outputUnaHin,YES,NO,NO,0,fo);
		if (rv != 0){
			return rv;
		}

		/* for ループの最後で改行コードを出力(即ち ループ1回で1行出力) */
		ans[0] = 0x000A;
		fwrite(ans,sizeof(unaCharT),1,fo);

	}
	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  SetKakaUkeSymbol
//
// ABSTRACT:   かかりうけシンボルのセット
//
// FUNCTION:
//	  かかりうけシンボルをセットする
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  なし
//
static int SetKakaUkeSymbol(
	int kuRel,		/* かかり先文節との関係 */
	unaCharT *ans	/* シンボルがセットされる変数 */
)
{
	/* かかり先文節との関係によりかかりうけシンボルをセットする */
	switch( kuRel ){
	case 0:
		ans[0] = 0x7121;      /* 無関： */ 
		ans[1] = 0x95a2;   
		ans[2] = 0xff1a;   
		break;
	case 1:
		ans[0] = 0x8907;      /* 複合： */ 
		ans[1] = 0x5408;   
		ans[2] = 0xff1a;   
		break;
	case 2:
		ans[0] = 0x8fd1;      /* 近並： */ 
		ans[1] = 0x4e26;   
		ans[2] = 0xff1a;   
		break;
	case 3:
		ans[0] = 0x9060;      /* 遠並： */ 
		ans[1] = 0x4e26;   
		ans[2] = 0xff1a;   
		break;
	case 4:
		ans[0] = 0x8fd1;      /* 近用： */ 
		ans[1] = 0x7528;   
		ans[2] = 0xff1a;   
		break;
	case 5:
		ans[0] = 0x9060;      /* 遠用： */ 
		ans[1] = 0x7528;   
		ans[2] = 0xff1a;   
		break;
	case 6:
		ans[0] = 0x8fd1;      /* 近体： */ 
		ans[1] = 0x4f53;   
		ans[2] = 0xff1a;   
		break;
	case 7:
		ans[0] = 0x9060;      /* 遠体： */ 
		ans[1] = 0x4f53;   
		ans[2] = 0xff1a;   
		break;
	case 8:
		ans[0] = 0x8fd1;      /* 近接： */ 
		ans[1] = 0x6452;   
		ans[2] = 0xff1a;   
		break;
	case 9:
		ans[0] = 0x9060;      /* 遠接： */ 
		ans[1] = 0x6452;   
		ans[2] = 0xff1a;   
		break;
	case 10:
		ans[0] = 0x62ec;      /* 括弧： */ 
		ans[1] = 0x5f27;   
		ans[2] = 0xff1a;   
		break;
	case 11:
		ans[0] = 0x5b64;      /* 孤立： */ 
		ans[1] = 0x7acb;   
		ans[2] = 0xff1a;   
		break;
	default:
		ans[0] = 0xff1f;      /* ？？： */ 
		ans[1] = 0xff1f;   
		ans[2] = 0xff1a;   
		break;
	}

	/* 正常終了 */
	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  GetAppInfo
//
// ABSTRACT:   アプリケーション情報の取得
//
// FUNCTION:
//	  アプリケーション情報を取得する
//
// RETURN:
//	  0				正常終了
//	  その他		エラー
//
// NOTE:
//	  アプリケーション情報は、出力用に加工されて返される。
//
static int GetAppInfo(
	unaKApiHandleT  *h,				/* ハンドラ */
	const unaMorphT	*morph,			/* 形態素(単語) */
	ucharT			*appInfBuf,		/* アプリ情報バッファ */
	int				appInfBufSiz,	/* アプリ情報バッファのサイズ(byte) */
	int				*appInfByte		/* アプリ情報の長さ(byte) */
)
{
	int rv;						/* 関数の返り値 */
	unaAppInfoT *appInf;			/* アプリケーション情報型へのポインタ */
	int subAppInfByte;				/* 下位構造のアプリ情報の長さ(byte) */
	int tokenType;					/* 英語トークンのトークン種別 */

	/* アプリケーション情報の取得 */
	rv = unaKApi_getAppInfo(h,morph,&appInf);
	switch (rv) {
	case UNA_KNOWN_WORD:	/* 登録語 */
		if (appInf->len > appInfBufSiz) { /* バッファオーバーフロー */
			fprintf(stdout,"Apprication buffer overflow\n");
			return 200;
		}
		memcpy(appInfBuf,appInf->info,appInf->len);
		*appInfByte = appInf->len;
		break;
	case UNA_ENG_TOKEN:		/* 英語トークン */
		tokenType = unaKApi_getTokenType(h,morph);
		if (tokenType < 0) { /* エラー */
			return 201;
		}
		if (tokenType == UNA_ENG_TOKEN_RETURN) { /* 復改 */
			if (10 > appInfBufSiz) { /* 10 は、(' ' + トークン種別)の大きさ*/
				fprintf(stdout,"Apprication buffer overflow\n");
				return 202;
			}
			*(unaCharT *)appInfBuf = 0x0020; /* 表示は ' ' に置き換える */
			*appInfByte = 2;
		}
		else {
			if (appInf->len + 8 > appInfBufSiz) { /* 8 はトークン種別 */
				fprintf(stdout,"Apprication buffer overflow\n");
				return 203;
			}
			memcpy(appInfBuf,appInf->info,appInf->len);
			*appInfByte = appInf->len;
		}
		/* トークン種別も表示 */
		GetTypeStr((unaCharT *)&appInfBuf[*appInfByte],tokenType);
		*appInfByte += 8;
		/* 下位構造(あれば)のアプリ情報も表示 */
		rv = EngSubMorphAppInf(h,morph,&appInfBuf[*appInfByte],
							appInfBufSiz - *appInfByte,&subAppInfByte);
		if (rv != 0) {
			return rv;
		}
		*appInfByte += subAppInfByte;
		break;
	case UNA_NO_APP_DIC:	/* 登録語(アプリ辞書無し) */
	case UNA_UNKNOWN_WORD:	/* 未登録語 */
		*appInfByte = morph->length * sizeof(unaCharT);
		if (*appInfByte > appInfBufSiz) { /* バッファオーバーフロー */
			fprintf(stdout,"Apprication buffer overflow\n");
			return 204;
		}
		memcpy(appInfBuf,morph->start,*appInfByte);
		break;
	default:
		fprintf(stdout,"CODE=%d Get App Info Error\n",rv);
		return 205;
	}

	/* 正常終了 */
	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  EngSubMorphAppInf
//
// ABSTRACT:   英語トークン下位構造のアプリ情報取得
//
// FUNCTION:
//	  英語トークン下位構造のアプリ情報を取得する
//
// RETURN:
//	  0				正常終了
//	  それ以外		エラー
//
// NOTE:
//	  下位構造のアプリケーション情報は、一塊になり出力用に加工されて返される
//
static int EngSubMorphAppInf(
	unaKApiHandleT  *h,			/* ハンドラ */
	const unaMorphT	*morph,		/* 形態素(単語) */
	ucharT		 *appInfBuf,	/* アプリ情報バッファ */
	int		 appInfBufSiz,	/* アプリ情報バッファのサイズ(byte) */
	int		 *appInfByte	/* アプリ情報の長さ(byte) */
)
{
	unaMorphT morphBuf[UNA_LOCAL_TEXT_SIZE]; /* 1文字1形態素の場合を考慮 */
	int morphNum;				/* 書かれた下位形態素の数 */
	int rv;					/* 関数の返り値 */
	int i;						/* ループ変数 */
	unaAppInfoT *appInf;		/* アプリケーション情報型へのポインタ */
	int tokenType;				/* 英語トークンのトークン種別 */

	/* 初期設定 */
	*appInfByte = 0;

	/* 下位情報を取得する */
	rv = unaKApi_getSubMorph(h,morph,morphBuf,&morphNum,UNA_LOCAL_TEXT_SIZE);
	if (rv != UNA_ENG_TOKEN) {
		fprintf(stdout,"Not english token in spite of enough check\n");
		return 206;
	}

	if (morphNum == 0) {	/* 下位構造なし */
		return 0;
	}

	/* 下位情報のアプリ情報を取得する */
	for (i = 0;i < morphNum;i++) {

		rv = unaKApi_getAppInfo(h,&morphBuf[i],&appInf);
		if (rv != UNA_ENG_TOKEN) {
			fprintf(stdout,"Not english token in spite of enough check\n");
			return 207;
		}

		if (2 > appInfBufSiz - *appInfByte) { /* 2 は、'\' の分 */
			fprintf(stdout,"Apprication buffer overflow\n");
			return 208;
		}
		*(unaCharT *)&appInfBuf[*appInfByte] = 0x005C; /* '\' で区切る */
		*appInfByte += 2;

		tokenType = unaKApi_getTokenType(h,&morphBuf[i]);
		if (tokenType < 0) { /* エラー */
			return 209;
		}
		if (tokenType == UNA_ENG_TOKEN_RETURN) {
			if (10 > appInfBufSiz - *appInfByte) {
				/* 10 は、' ' と トークン種別 */
				fprintf(stdout,"Apprication buffer overflow\n");
				return 210;
			}
			*(unaCharT *)&appInfBuf[*appInfByte] = 0x0020;
							  /* 表示は ' ' に置き換える */
			*appInfByte += 2; /* 復帰(CR)改行(LF)でも1文字(2byte)で数える */
		}
		else {
			if (appInf->len + 8 > appInfBufSiz - *appInfByte) {
				/* 10 は、トークン種別 */
				fprintf(stdout,"Apprication buffer overflow\n");
				return 211;
			}
			memcpy(&appInfBuf[*appInfByte],appInf->info,appInf->len);
			*appInfByte += appInf->len;
		}

		/* トークンタイプ(文字列)のセット */
		GetTypeStr((unaCharT *)&appInfBuf[*appInfByte],tokenType);
		*appInfByte += 8;
	}

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  GetTypeStr
//
// ABSTRACT:   英語トークンタイプ(文字列)取得
//
// FUNCTION:
//	  文字型の英語トークンタイプを取得する
//
// RETURN:
//	  0				正常終了
//
// NOTE:
//	  typeStr の大きさは、必ず UNICODE 4文字(8 byte) 以上であること
//
void GetTypeStr(
	unaCharT *typeStr,		/* 英語トークンタイプ(文字列) */
	uintT	tokenType		/* 英語トークンタイプ(数値) */
)
{
	switch (tokenType) {
	case UNA_ENG_TOKEN_NORMAL:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x666E;	/* 普 */
		typeStr[2] = 0x901A;	/* 通 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	case UNA_ENG_TOKEN_HYPHEN:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x884C;	/* 行 */
		typeStr[2] = 0x672B;	/* 末 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	case UNA_ENG_TOKEN_ARABIC:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x6570;	/* 数 */
		typeStr[2] = 0x5B57;	/* 字 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	case UNA_ENG_TOKEN_SYMBOL:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x8A18;	/* 記 */
		typeStr[2] = 0x53F7;	/* 号 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	case UNA_ENG_TOKEN_SPACE:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x7A7A;	/* 空 */
		typeStr[2] = 0x767D;	/* 白 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	case UNA_ENG_TOKEN_RETURN:
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x6539;	/* 改 */
		typeStr[2] = 0x884C;	/* 行 */
		typeStr[3] = 0x005D;	/* ] */
		break;
	default:	/* UNA_ENG_TOKEN_INITIAL */
		typeStr[0] = 0x005B;	/* [ */
		typeStr[1] = 0x982D;	/* 頭 */
		typeStr[2] = 0x5B57;	/* 字 */
		typeStr[3] = 0x005D;	/* ] */
	}
	return;
}


//--------------------------------------------------------------------------
// MODULE:	  StopByTime
//
// ABSTRACT:	  中断関数
//
// FUNCTION:
//	  イベントが起きたときに、中断することをUNAに知らせるための関数。
//	  この関数では、StopTime 秒過ぎたときに、真となる。
//
// RETURN:
//	  1		StopTime 秒経過した
//	  0		StopTime 秒経過していない
//
// NOTE:
//	  StopTime が負の場合は、この関数は常に0を返す。
//
static int StopByTime(void)
{
	clock_t lapTime;	/* プログラム開始からの経過時間 */

	if (StopTime < 0) {	/* StopTime が負の時 */
		return 0;
	}

	lapTime = clock();	/* プログラム開始からの経過時間を取得 */
	if (lapTime >= (clock_t)(StopTime * CLOCKS_PER_SEC)) {
		return 1;
	}

	return 0;
}

//--------------------------------------------------------------------------
// Copyright (c) 2000-2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
