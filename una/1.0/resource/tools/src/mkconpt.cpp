//
// mkconpt.cpp -
//		接続表パッチソース作成モジュール
//		接続表パッチソースを作成するモジュール
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
#include <stdio.h>		/* fprintf */
#include <string.h>		/* strcmp */
#include <stdlib.h>		/* malloc,free */
#include <sys/stat.h>	/* stat */
#include "una.h"		/* una グローバルなヘッダファイル */
#include "unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define CONTINUE             1	/* 処理の継続(まだ未処理カテゴリあり)   */
#define END_OF_CATEGORY      0	/* 処理の終了(全てのカテゴリを処理した) */

//--------------------------------------------------------------------------
// TAG:	  hinLstIdxT
//
// ABSTRACT:	  UNA品詞リスト型
//
// NOTE:
//	  UNA品詞リストの1レコードの索引情報を表わす。但しレコードは当プログラム
//	  で使用するフィールドのみから構成される。レコードの実体は、別のデータ
//	  プールにある。
//
typedef struct hinLstIdxT{		// UNA品詞リスト型
	unaCharT *ptr;				/* 文字列(品詞名)へのポインタ */
	int	 len;				/* 文字列(品詞名)の長さ       */
}hinLstIdxT;

//--------------------------------------------------------------------------
// TAG:	  conDefIdxT
//
// ABSTRACT:	  接続定義ファイル型
//
// NOTE:
//	  接続定義ファイルの1レコードの索引情報表わす。レコードの実体は、別の
//	  データプールにある。なお、[0]のデータは、カテゴリヘッダのデータを
//	  表わす。
//
typedef struct conDefIdxT{		// 接続定義ファイル型
	unaCharT *ptr;				/* 文字列(品詞名)へのポインタ */
	int	 len;				/* 文字列(品詞名)の長さ       */
	unaCharT *ptr2;				/* シンボルとコストへのポインタ */
	int	 len2;				/* シンボルとコストの長さ       */
}conDefIdxT;

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int GetOptions(int argc,char *argv[],int *processedArgc);
static void OutputUsage(void);
static int ProcCategory(int hinLstCnt,FILE *fp1,FILE *fp2);
static int ReadHinLst(char *hinLst,int *hinLstCnt);
static int ReadConDef(FILE *fp1,int *conDefCnt,unaCharT *hinNamPtr,
	   int *hinNamLen,int *flg);
static int WriteFile(FILE *fp2,unaCharT *hin1ptr,int hin1Len,
	   unaCharT *hin2ptr,int hin2Len,unaCharT *otherPtr,int otherLen);
static int HinMeiComp(hinLstIdxT *w1,hinLstIdxT *w2);
static int IsBuMoji(unaCharT *s1,int s1Len,unaCharT *s2,int s2Len);

//--------------------------------------------------------------------------
// グローバル変数
//--------------------------------------------------------------------------
static hinLstIdxT	*unaHinLst;	  /* UNA品詞リスト */
static unaCharT		*unaHinPool;  /* UNA品詞リスト用文字列プール */
static conDefIdxT	*unaConDef;	  /* 接続定義ファイル */
static unaCharT		*unaConPool;  /* 接続定義ファイル用文字列プール */


//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	  実行接続表を作成する
//
// FUNCTION:
//	  接続定義ファイル及びパッチ済みUNA品詞リストから、接続表パッチソースを
//	  作成する。使い方は、コマンド行から次の通りに入力する。
//
//		mkconpt.exe 接続定義ファイル UNA品詞リスト 接続表パッチソース
//
//	  例:
//		mkconpt condef.txt unahin.lst conpt.src
//
// RETURN:
//	  UNA_OK		正常終了
//	  その他		エラー
//
// NOTE:
//	  (注1)
//		形態素品詞名順に接続表パッチソースが出力されるようにするため
//	  (注2)
//		メモリ上の初期作業領域。1カテゴリを読み込む事に1カテゴリ分の接続定義
//		ファイルのレコード(実レコードへの索引)がこの領域に格納される。
//		もし、領域が足りない場合には実際の読み込み時に realloc される。
//		なお、実際のレコードは、unaConPool に格納されるが最大でも、ファイル
//		の大きさなので、この大きさで十分である。
//
int main(
	 int argc,			/* コマンド行の引数の数(1以上) */
	 char *argv[]		/* コマンド行の引数 */
 )
{
	int rv;			/* 関数の返り値 */
	int processedArgc;	/* 処理した引数の数 */
	char *conDef;		/* 接続定義ファイル */
	char *hinLst;		/* UNA品詞リスト */
	char *conPt;		/* 接続表パッチソース */
	int hinLstCnt;		/* UNA品詞リストのレコード件数 */
	struct stat stbuf;	/* ファイルの情報 */
	FILE *fp1;			/* 接続定義ファイル */
	FILE *fp2;			/* 接続パッチソース */
	unaCharT uniChar;	/* UNICODE 1文字 */

	/* コマンドラインからオプションを得る */
	rv = GetOptions(argc,argv,&processedArgc);
	if (rv < 0){
  		return 1;
	}

	/* 残りの引数のチェック */
	if (argc - processedArgc != 3) {	/* 残りの引数の数がおかしい */
		fprintf(stdout,"Argument Err\n");
		return 2;
	}

	/* 引数の設定 */
	conDef	= argv[processedArgc];
	hinLst	= argv[processedArgc + 1];
	conPt	= argv[processedArgc + 2];

	/* UNA品詞リストをメモリに読み込み格納する */
	rv = ReadHinLst(hinLst,&hinLstCnt);
	if (rv < 0){
		return 3;
	}

	/* UNA品詞リストのマージソート(注1) */
	una_msort(unaHinLst,0,hinLstCnt - 1,sizeof(hinLstIdxT),
										(unaCompFuncT)HinMeiComp,NULL);

	/* 接続定義ファイルオープン */
	if ((fp1 = fopen(conDef,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",conDef);
		return 4;
	}

	/* 接続定義ファイルのサイズをチェック */
	if(stat(conDef,&stbuf)==-1){
		fprintf(stdout, "Can't get file info %s\n", conDef);
		return 5;
	}
	/* 接続定義ファイルの領域を確保する(注2) */
	unaConDef= (conDefIdxT *)malloc(LINES_BLOCK * sizeof(conDefIdxT));
	/* unaConPoolは一度に全部を確保(unaConPool < stbuf.st_size で足りる) */
	unaConPool = (unaCharT *)malloc(stbuf.st_size);

	/* 接続表パッチソースオープン */
	if ((fp2 = fopen(conPt,"wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",conPt);
		return 6;
	}

	/* 接続表パッチソースに予めBOMを書き込む */
	uniChar = 0xFEFF;
	fwrite(&uniChar,sizeof(uniChar),1,fp2);

	for(;;){	/* forever */
		rv = ProcCategory(hinLstCnt,fp1,fp2);
		if (rv != CONTINUE) {	/* エラー又はEND_OF_CATEGORY */
			break;
		}
	}

	/* パッチファイルのクローズ */
	fclose(fp2);

	/* 接続定義ファイルのクローズ */
	fclose(fp1);

	/* メモリの free */
	free(unaHinLst);
	free(unaHinPool);
	free(unaConDef);
	free(unaConPool);

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
	int *processedArgc	/* 処理した引数の数(後カウントアップ) */
)
{
	/* argc の数だけループ。第1引数にはプログラム名が入っているから1より */
	*processedArgc = 1;		/* 上記の理由により引数1個処理したとする */
	while (*processedArgc < argc) {
		if (*(argv[*processedArgc]) !='-') { /* オプションは '-' で始まる */
			break;						/* 引数が指定されたとしてブレーク */
		}
		switch(*(argv[*processedArgc] + 1)){ /* 引数の2バイト目をテスト */
		case 'h':	/* ヘルプの要求 */
			/* Usageを表示して終了 */
			OutputUsage();
			(*processedArgc)++;
			return -1;
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
	fprintf(stdout,"mkconpt (Ver1.02)\n");
	fprintf(stdout,"Usage: mkconpt [OPTION] CON_DEF_FIL UNA_HIN_LST");
	fprintf(stdout," CON_PT_SRC\n");
	fprintf(stdout,"  OPTION\n");
	fprintf(stdout,"    -h	           Help\n");
	fprintf(stdout,"Ex   : mkconpt.exe condef.txt unahin.lst conpt.src\n");
	fprintf(stdout,"Note...\n");
	fprintf(stdout,"  CON_DEF_FIL must be UCS2 text file. When a double");
	fprintf(stdout," definition of prefix exist\n");
	fprintf(stdout,"  in CON_DEF_FIL, the last definition is used.\n");
	return;
}


//--------------------------------------------------------------------------
// MODULE:	  ProcCategory
//
// ABSTRACT:	  カテゴリ処理
//
// FUNCTION:
//	  接続定義ファイルを1カテゴリ分だけ読み込み、
//	  それを元に1カテゴリ分の接続表パッチソースを作成する。
//
// RETURN:
//	  CONTINUE				処理の継続(まだ未処理カテゴリあり)
//	  END_OF_CATEGORY		処理の終了(全てのカテゴリを処理した)
//	  負の値				エラー番号
//
// NOTE:
//	  (注1)
//		読み込むレコードは、カテゴリヘッダレコードを除いた分である。
//	  (注2)
//		実際のところ conDefCnt == 0 の場合は、ReadConDef の返り値は
//		END_OF_CATEGORY である。なお、ReadConDef の返り値が END_OF_CATEGORY
//		であるからといって、必ず conDefCnt == 0 であるとは限らない
//	  (注3)
//		パッチソースレコードを出力するのは、接続定義ファイルに記述された
//		レコードのprefixがUNA品詞リストの品詞名の部分文字列になっている場合
//		だけであり、全ての品詞名との接続がパッチファイルに出力されるわけでは
//		無い。
//
static int ProcCategory(
	int hinLstCnt,	/* UNA品詞リストの件数 */
	FILE *fp1,		/* 接続定義ファイル */
	FILE *fp2		/* パッチソース */
)
{
	int rcRv;		/* ReadConDef関数の返り値 */
	int rv;		/* 関数の返り値 */
	int conDefCnt;	/* 接続定義ファイルの1カテゴリ内の件数 */
	unaCharT hinNamPtr[MAX_LINE_LEN];
					/* カテゴリの品詞名(R=やL=の次に書かれてる品詞名) */
	int hinNamLen;	/* カテゴリの品詞名の長さ */
	int flg;		/* 接続定義ファイルの1カテゴリ内のカテゴリ種別
						0: Left、1:Right */
	int i;			/* ループ変数 */
	int j;			/* ループ変数 */

	/* 接続定義ファイルの1カテゴリ分をメモリに読み込み格納する(注1) */
	rcRv = ReadConDef(fp1,&conDefCnt,hinNamPtr,&hinNamLen,&flg);
	if (rcRv < 0){	/* エラー */
		return rcRv;
	}

	/* 件数チェック(注2) */
	if (conDefCnt == 0) {
		return END_OF_CATEGORY;
	}

	/* 以下の for ループでは、1件ずつ処理を行う */
	for (i = 0;i < hinLstCnt;i++) {

		/* 以下の for ループでは、品詞リストのレコード1件について
			接続定義ファイルの全レコードと次々に比較を行う(逆順に) */
		for (j = conDefCnt - 1; j >= 0 ; j--) {
			rv = IsBuMoji(unaHinLst[i].ptr,unaHinLst[i].len,
							unaConDef[j].ptr,unaConDef[j].len);
			if (rv == 0) {	/* s2がs1の部分文字列になっている */
				/* 接続パッチソースの出力(注3) */
				if (flg == 0) {	/* Left、即ち左側が対象品詞の時 */
					WriteFile(fp2,hinNamPtr,hinNamLen,
								unaHinLst[i].ptr,unaHinLst[i].len,
								unaConDef[j].ptr2,unaConDef[j].len2);
				}
				else {
					WriteFile(fp2,unaHinLst[i].ptr,unaHinLst[i].len,
								hinNamPtr,hinNamLen,
								unaConDef[j].ptr2,unaConDef[j].len2);
				}
				break;
			}
		}
	}

	return rcRv;	/* ReadConDef関数の返り値をそのまま返す */
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
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//	  ・レコードは unaHinPool に格納され、そこへのポインタとレングスが 
//		unaHinLstに格納される。
//
static int ReadHinLst(
	char *hinLst,		/* UNA品詞リスト */
	int *hinLstCnt		/* UNA品詞リストのレコード件数 */
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

	/* 対象とするファイルのサイズをチェック */
	if(stat(hinLst,&stbuf)==-1){
		fprintf(stdout, "Can't get file info %s\n", hinLst);
		return -1;
	}

	/* まず最初分を確保 */
	unaHinLst= (hinLstIdxT *)malloc(LINES_BLOCK * sizeof(hinLstIdxT));
	/* unaHinPoolは一度に全部を確保(unaHinPool < stbuf.st_size で足りる) */
	unaHinPool = (unaCharT *)malloc(stbuf.st_size);

	/* 初期値の設定 */
	hinPoolEnd = 0;
	lineEnd    = 0;
	lineMax   = LINES_BLOCK;

	/* UNA品詞リストのオープン */
	if ((fp = fopen(hinLst,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",hinLst);
		return -2;
	}

	/* UNA品詞リストを直接 unaHinPool に読込む */
	while((rv=una_fgetws(unaHinPool+hinPoolEnd,MAX_LINE_LEN,fp)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		if (rv >= MAX_LINE_LEN - 1) { /* 行長がオーバーフローの可能性あり */
			fprintf(stdout,
				"Possibly the line buffer of UNA_HIN_LIST overflow\n");
			return -3;
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
			return -4;
		}

		/* 品詞名(第1トークン)のみを unaHinPool に残す */
		*(unaHinPool+hinPoolEnd+token[0].len) = 0x0000;	/* ターミネート */
		unaHinLst[lineEnd].ptr = unaHinPool+hinPoolEnd;	/* 品詞名の位置 */
		unaHinLst[lineEnd].len = token[0].len;			/* 品詞名の長さ */

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

	*hinLstCnt = lineEnd;
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadConDef
//
// ABSTRACT:    接続定義ファイルを読み込む
//
// FUNCTION:
//	  接続定義ファイルの1カテゴリ分を読み込みダイナミックメモリに格納する
//
// RETURN:
//	  CONTINUE				正常終了
//	  END_OF_CATEGORY		ファイル終了
//	  負の値				エラー
//
// NOTE:
//	  (注1)
//		1カテゴリの範囲は、カテゴリヘッダから、ブランク行が現れるまで
//		もしくはEOF(ファイルエンド)までとする
//	  (注2)
//		カテゴリヘッダレコードに第2以上のトークンが存在した場合には無視する
//	  (注3)
//		このチェックにより、*conDefCnt = 0 で返る場合の返り値は、
//		必ず END_OF_CATEGORY である
//
static int ReadConDef(
	FILE	 *fp1,		 /* 接続定義ファイル */
	int	 *conDefCnt, /* 接続定義ファイルのレコード数 */
	unaCharT *hinNamPtr, /* カテゴリの品詞名(R=やL=の次に書かれてる品詞名) */
	int 	 *hinNamLen, /* カテゴリの品詞名の長さ */
	int	 *flg		 /* 接続定義ファイルの1カテゴリ内のカテゴリ種別
							0: Left、1:Right */
)
{
	int		 rv;					/* 関数の返り値 */
	uintT		 conPoolEnd;			/* conHinPoolの書き込みオフセット */
	uintT		 lineEnd;				/* 書き込んだレコード数 */
	uintT		 lineMax;				/* 書き込み可能なレコード数 */
	int		 tokenCnt;				/* トークンの数 */
	unaStrIndexT token[MAX_TOKEN_CNT];	/* トークン */

	/* 初期値の設定 */
	conPoolEnd = 0;
	lineEnd    = 0;
	lineMax   = LINES_BLOCK;

	*hinNamLen = 0;	/* カテゴリ開始かどうかの判断にも使用 */

	/* 接続定義ファイルを直接 unaConPool に読込む(注1) */
	while((rv = una_fgetws(unaConPool+conPoolEnd,MAX_LINE_LEN,fp1)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		if (rv >= MAX_LINE_LEN - 1) { /* 行長がオーバーフローの可能性あり */
			fprintf(stdout,
				"Possibly the line buffer of CON_DEF_FIL overflow\n");
			return -1;
		}

		/* トークンに分割する */
		tokenCnt = una_getTokens(unaConPool+conPoolEnd,MAX_TOKEN_CNT,token);

		if (*hinNamLen == 0) {	/* カテゴリがまだ開始されてない */
			/* 空行は読み飛ばし */
			if (tokenCnt == 0) {
				continue;
			}
			/* コメント行は読み飛ばし */
			if (*(unaConPool+conPoolEnd+token[0].pos) == 0x0023){ /* '#' */
				continue;
			}
			/* カテゴリヘッダのレコード長のチェック */
			if (token[0].len < 3) { /* "L="又は"R="を除いて1文字は必要 */
				fprintf(stdout,"Category Headre Record Error.\n");
				return -2;
			}
			/* フラグ設定 */
			switch (*(unaConPool+conPoolEnd+token[0].pos)){
			case 0x004C:	/* 'L' */
				*flg = 0;
				break;
			case 0x0052:	/* 'R' */
				*flg = 1;
				break;
			default:
				fprintf(stdout,"Category Header String must start from");
				fprintf(stdout," \"L=\" or \"R=\".\n");
				return -3;
			}
			/* 2桁目が '=' かどうかチェック */
			if (*(unaConPool+conPoolEnd+token[0].pos+1) != 0x003D){ /* '=' */
				fprintf(stdout,"Category Header String must start from");
				fprintf(stdout," \"L=\" or \"R=\".\n");
				return -4;
			}
			/* カテゴリ名セット */
			memcpy(hinNamPtr,unaConPool+conPoolEnd+token[0].pos+2,
									(token[0].len - 2) * sizeof(unaCharT));
			*hinNamLen	= token[0].len - 2;	/* カテゴリ開始(注2) */
			continue;
		}

		/* 空行は、カテゴリの終わり(注1) */
		if (tokenCnt == 0) {
			*conDefCnt = lineEnd;
			return CONTINUE;
		}
		/* コメント行は読み飛ばし */
		if (*(unaConPool+conPoolEnd+token[0].pos) == 0x0023){ /* '#' */
			continue;
		}
		/* トークン数のチェック */
		if (tokenCnt < 3
		 || *(unaConPool+conPoolEnd+token[1].pos) == 0x0023
		 || *(unaConPool+conPoolEnd+token[2].pos) == 0x0023) {
			fprintf(stdout,"Insufficient token count of CON_DEF_FIL\n");
			return -5;
		}

		/* インデックスへ書き込む */
		unaConDef[lineEnd].ptr	= unaConPool+conPoolEnd+token[0].pos;
		unaConDef[lineEnd].len	= token[0].len;
		unaConDef[lineEnd].ptr2	= unaConPool+conPoolEnd+token[1].pos;
		unaConDef[lineEnd].len2	= token[2].pos+token[2].len-token[1].pos;

		/* 更新 */
		lineEnd++;
		conPoolEnd += token[2].pos + token[2].len;

		/* 行バッファをオーバーフローする場合はリアロック */
		if ( lineEnd + 1 >= lineMax){
			lineMax += LINES_BLOCK;
			unaConDef= (conDefIdxT *)realloc(unaConDef,
											lineMax * sizeof(conDefIdxT));
		}
	}

	/* プレフィックスレコードがちゃんとあるかチェック(注3) */
	if (lineEnd == 0 &&  *hinNamLen != 0) {	/* カテゴリヘッダのみ */
		fprintf(stdout, "No prefix record exist in the category.\n");
		return -6;
	}

	*conDefCnt = lineEnd;
	return END_OF_CATEGORY;	/* EOFの時 */
}


//--------------------------------------------------------------------------
// MODULE:	  WriteFile
//
// ABSTRACT:    接続表パッチソースの出力
//
// FUNCTION:
//	  接続表パッチソースを出力する
//
// RETURN:
//	  UNA_OK	正常
//	  それ以外	エラー
//
// NOTE:
//
static int WriteFile(
	FILE *fp2,				/* 接続表パッチソース */
	unaCharT *hin1ptr,		/* 品詞1のポインタ */
	int hin1Len,			/* 品詞1の長さ */
	unaCharT *hin2ptr,		/* 品詞2のポインタ */
	int hin2Len,			/* 品詞2の長さ */
	unaCharT *otherPtr,		/* シンボルとコストのポインタ */
	int otherLen			/* シンボルとコストの長さ */
)
{
	unaCharT rec[MAX_LINE_LEN];	/* レコード */
	unaCharT *recPtr;			/* レコードをポインタ表示したもの */
	unaCharT uniChar;			/* UNICODE 1文字 */

	/* 初期設定 */
	recPtr = rec;
	uniChar = 0x0020;			/* スペース */

	memcpy(recPtr,hin1ptr,hin1Len * sizeof(unaCharT));
	recPtr += hin1Len;
	memcpy(recPtr,&uniChar,sizeof(uniChar));
	recPtr += 1;
	memcpy(recPtr,hin2ptr,hin2Len * sizeof(unaCharT));
	recPtr += hin2Len;
	memcpy(recPtr,&uniChar,sizeof(uniChar));
	recPtr += 1;
	memcpy(recPtr,otherPtr,otherLen * sizeof(unaCharT));
	recPtr += otherLen;
	uniChar = 0x000A;	/* 改行コード */
	memcpy(recPtr,&uniChar,sizeof(uniChar));
	recPtr += 1;

	/* データの書き込み */
	fwrite(rec,recPtr - rec,2,fp2);

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  HinMeiComp
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
static int HinMeiComp(
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
// MODULE:	  IsBuMoji
//
// ABSTRACT:	  第2文字列が第1文字列の部分文字列になっているか検定する関数
//
// FUNCTION:
//	  第2文字列が第1文字列の先頭からの部分文字列になっているか検定する
//
// RETURN:
//	  0			部分文字列になっている
//	  1			部分文字列になっていない
//
// NOTE:
//	  (注1)
//		このツールの仕様として、単に部分文字列になっているということに加え
//		マッチしたところが丁度トークンの切れ目になっていなければならない。
//		そのチェックを行う。
//
static int IsBuMoji(
	unaCharT	*s1,		/* 第1文字列 */
	int		s1Len,
	unaCharT	*s2,		/* 第2文字列 */
	int		s2Len
)
{
	int i;			/* ループ変数 */

	if (s2Len == 1 && *s2 == 0xFF0A) {	/* '＊' */
		return 0;	/* 無条件に部分文字列になっているとする */
	}

	/* 比較する長さを決定する */
	if(s1Len < s2Len){
		return 1;
	}

	for (i = 0;i < s2Len ;i++) {
		if ( s1[i] != s2[i] ){
			return 1;
		}
	}

	/* ここまで来たということは通常で言う部分文字列になっている(注1) */
	if (s2Len == s1Len) {	/* 文字列完全一致の時 */
		return 0;
	}

	switch (s1[s2Len]) {	/* 一致した部分の次の文字 */
	case 0x002E:	/* '.' */
	case 0x002C:	/* ',' */
	case 0x003B:	/* ';' */
		return 0;
	}

	switch (s1[s2Len - 1]) {	/* 一致した部分の最後の文字 */
	case 0x002E:	/* '.' */
	case 0x002C:	/* ',' */
	case 0x003B:	/* ';' */
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------------
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
