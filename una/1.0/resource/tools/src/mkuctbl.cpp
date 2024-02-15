//
// mkuctbl.cpp -
//      未登録語コスト推定テーブル作成ツール
//      未登録語のコストを推定するテーブルを作成するツール
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
#include	<stdio.h>
#include	<string.h>
#include	<assert.h>		/* デバッグ用 */
#include	"una.h"			/* UNAグローバルなヘッダファイル */
#include	"unamdunk.h"	/* UNA_UNK_MOR_KIND_CNT */
#include	"unaconst.h"

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

/* マクロ定義 */
#define LOCAL_MAX_TOKEN_CNT (MAX_LINE_LEN / 2 + 1) /* 1行あたりの最大トークン数 */
#define UCTBL_SIZE	  (UNA_HYOKI_LEN_MAX) * (UNA_UNK_MOR_KIND_CNT)
											 /* コストテーブルのサイズ    */

/* グローバル変数 */
static char unaComment[UNA_COM_SIZE]; /* テーブルに付けるコメント */
static char unaDicVer[UNA_VER_SIZE] = UNA_UC_VER; /* テーブルのバージョン */

//--------------------------------------------------------------------------
// TAG:	  UnkCostTbl
//
// ABSTRACT:    未登録語コスト格納用配列
//
// NOTE:
//    ソースファイルから読み込んだコスト値を、格納する配列
//
static ushortT UnkCostTbl[UCTBL_SIZE];

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int GetOptions(int argc,char *argv[],char *comment,
			int *processedArgc);
static void OutputUsage(void);
static int OptionA(int argc,char *argv[],char *comment,
			int *processedArgc);
static int ReadSrc(char *unkCostSrc);
static int WriteTable(char *unkCostTbl,int tblSize);


//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	メインモジュール
//
// FUNCTION:
//	 未登録語コスト推定テーブルを作成する。
//
//	 使い方は、コマンド行から次の通りに入力する。
//	 mkuctbl.exe [オプション] ソースファイル 未登録語コスト推定テーブル
//
// 例)
//	 mkuctbl.exe -a "For Ihyouki" unkcost.src unkcost.tbl
//
// RETURN:
//	 0			正常
//	 1-255		エラー
//
// NOTE:
//	 ソースファイルは、unicode(ucs2)テキストファイルである
//
int main(
	int argc,			 /* コマンド行の引数の数(1以上) */
	char *argv[]		 /* コマンド行の引数 */
)
{
	int rv;			 /* 関数の返り値 */
	int processedArgc;	 /* 処理した引数の数 */
	char *unkCostSrc;	 /* 未登録語コストソース */
	char *unkCostTbl;	 /* 未登録語コスト推定テーブル */

	/* コマンドラインからオプションを得る */
	rv = GetOptions(argc,argv,unaComment,&processedArgc);
	if (rv < 0){
  		return 1;
	}

	/* 残りの引数のチェック */
	if (argc - processedArgc != 2) {	/* 残りの引数の数がおかしい */
		fprintf(stdout,"Argument Err\n");
		return 2;
	}

	/* ファイル名セット */
	unkCostSrc		= argv[processedArgc];
	unkCostTbl		= argv[processedArgc + 1];
	
	rv = ReadSrc(unkCostSrc);
	if (rv < 0) {
		return 3;
	}

	rv = WriteTable(unkCostTbl,rv * sizeof(ushortT));
	if (rv < 0){
  		return 4;
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
	fprintf(stdout,"mkuctbl (Ver1.06)\n");
	fprintf(stdout,"Usage: mkuctbl SOURCE_FILE UNK_COST_TABLE\n");
	fprintf(stdout,"Ex   : mkuctbl -a \"For Ihyouki\" unkcost.src");
	fprintf(stdout," unkcost.tbl\n");
	fprintf(stdout,"  OPTION\n");
	fprintf(stdout,"    -h             Help\n");
	fprintf(stdout,"    -a ANNOTATION  The String to annotate");
	fprintf(stdout,"(Max48Byte)\n");
	fprintf(stdout,"  Note...\n");
	fprintf(stdout,"    SOURCE_FILE is the unicode(ucs2) text file.\n");
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
// MODULE:	  ReadSrc
//
// ABSTRACT:	  未登録語コストソースの入力
//
// FUNCTION:
//	  未登録語コストソースを読み込み、数値に変換して static 配列に
//	  格納する。
//
// RETURN:
//		負の値			エラー
//		正の値			読み込んだコストの数
//
// NOTE:
//	  コストは10進数で記述されている
//
static int ReadSrc(
	char	*unkCostSrc				   /* 未登録語コストソースファイル */
)
{
	FILE		 *fp;				   /* ファイルポインタ */
	int		 rv;				   /* 関数の返り値 */
	unaCharT	 inStr[MAX_LINE_LEN];  /* 読み込まれた文字列 */
	unaStrIndexT token[LOCAL_MAX_TOKEN_CNT]; /* トークン */
	int		 tokenCnt;			   /* トークンの数 */
	int		 i;					   /* ループ変数 */
	int		 cost;				   /* 数値化したコスト */
	int		 idx;				   /* static 配列の要素番号 */
	unaCharT	 tmpStr[5];			   /* una_utol用
										  (ターミネートするため必要) */

	/* 初期設定 */
	idx = 0;

	/* 未登録語コストソースファイルのオープン */
	if ((fp = fopen(unkCostSrc,"rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",unkCostSrc);
		return -1;
	}

	/* 未登録語コストソースファイルの読込み */
	while((rv=una_fgetws(inStr,MAX_LINE_LEN,fp)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		if (rv >= MAX_LINE_LEN - 1) { /* 行長がオーバーフローの可能性あり */
			fprintf(stdout,"Possibly the line buffer of %s overflow\n",
															unkCostSrc);
			return -2;
		}

		/* トークンに分割する */
		tokenCnt = una_getTokens(inStr,LOCAL_MAX_TOKEN_CNT,token);

		/* MAX_LINE_LEN = 1024 の時、記述できる最大トークン数は 512 で
		   現在、LOCAL_MAX_TOKEN_CNT = 512 なので必ず、tokenCnt < LOCAL_MAX_TOKEN_CNT */
		assert (tokenCnt < LOCAL_MAX_TOKEN_CNT);

		/* 空行は読み飛ばし */
		if (tokenCnt == 0) {
			continue;
		}

		for (i = 0;i < tokenCnt; i++) {

			/* コメント以降は処理しない */
			if (*(inStr+token[i].pos) == 0x0023){ /* #(シャープ)以降 */
				break;
			}

			/* トークンの桁数チェック */
			if (token[i].len > 5) { /* 最高でも 65535 の5 桁 */
				fprintf(stdout,"Too long length token\n");
				return -4;
			}

			/* 10進文字列かチェックする */
			rv = una_isdstr(inStr+token[i].pos,token[i].len);
			if (rv == 0) {
				fprintf(stdout,"Not decimal string\n");
				return -5;
			}

			memcpy(tmpStr,inStr+token[i].pos,token[i].len * sizeof(unaCharT));
			tmpStr[token[i].len] = 0x0000;
			cost = una_utol(tmpStr);
			if (cost < 0 || 65535 < cost) {
				/* ushortT で表わせる範囲以外ならエラー */
				fprintf(stdout,"The cost(%d) is out of scope\n",cost);
				return -7;
			}

			UnkCostTbl[idx++] = (ushortT)cost;
		}
	}

	fclose(fp);

	return idx;
}


//--------------------------------------------------------------------------
// MODULE:	  WriteTable
//
// ABSTRACT:    未登録語コスト推定テーブルの出力
//
// FUNCTION:
//	  未登録語コスト推定テーブルを出力する
//
// RETURN:
//	  UNA_OK	正常
//	  それ以外	エラー
//
// NOTE:
//	  メモリから外部記憶装置への書き出しである
//
static int WriteTable(
	char *unkCostTbl,		/* 未登録語用文字種別テーブル */
	int tblSize			/* テーブルの大きさ */
)
{
	FILE			*fp;	/* ファイルポインタ */

	/* 未登録語用文字種別テーブルオープン */
	if ((fp = fopen(unkCostTbl, "wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n",unkCostTbl);
		return -1;
	}

	/* データの書き込み */
	fwrite(unaComment,sizeof(unaComment),1,fp);		 /* コメント */
	fwrite(unaDicVer,sizeof(unaDicVer),1,fp);		 /* バージョン */
	fwrite(UnkCostTbl,tblSize,1,fp);

	fclose(fp);

	return UNA_OK;
}

//--------------------------------------------------------------------------
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
