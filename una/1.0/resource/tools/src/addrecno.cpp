//
// addrecno.cpp -
//      UNA 辞書ソースコンバートツール
//      UNA V3.0の辞書ソースをV3.2用にコンバートするツール
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
#include	"una.h"			/* UNAグローバルなヘッダファイル */
#include	"unaconst.h"

/* プロトタイプ宣言 */
static int AddStrLine(unaCharT *strBuf,int x);


//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	UCS2ファイルのレコードの先頭にレコード番号を振る
//
// FUNCTION:
//	  UCS2ファイルのレコードの先頭にレコード番号(行番号)をふる
//	  使い方は、コマンド行から次の通りに入力する。
//		addrecno.exe [オプション] 入力レコード 出力レコード
//
//	例:
//	addrecno unadic2.src unadic32.src
//
// RETURN:
//	  0			正常終了
//
// NOTE:
//	  但し、レコード番号(行番号)は 0 起算、9桁である(例 000011201)
//	  この関数は、UNA V3.0辞書ソースをUNA V3.2辞書ソースの形式に
//	  コンバートするためのものである
//
int main(
	 int argc,			/* コマンド行の引数の数(1以上) */
	 char *argv[]		/* コマンド行の引数 */
)
{
	int rv;
	char *inDicSrc;		/* 入力ファイル(V3.0 UNA 辞書ソース) */
	char *outDicSrc;	/* 出力ファイル(V3.2 UNA 辞書ソース) */
	FILE *fp;
	FILE *fo;
	unaCharT strBuf[MAX_LINE_LEN + 10];	/* 10 は行番号の分 */
	int i;

	/* コマンドライン引数の処理 */
	if (argc == 1 || strcmp(argv[1],"-h") == 0){
		fprintf(stdout,"addrecno (Ver1.02)\n");
		fprintf(stdout,"Add Line Number to Each Head of UCS2 String\n");
		fprintf(stdout,"Usage: addrecno V3.0_DIC_SRC V3.2_DIC_SRC\n");
		fprintf(stdout,"Ex   : addrecno unadic2.src unadic32.src\n");
		return 1;
	}

	if (argc != 3){
		fprintf(stdout,"Argument Err\n");
		return 2;
	}
	else{
		inDicSrc	= argv[1];
		outDicSrc	= argv[2];
	}


	if ((fp = fopen(inDicSrc, "rb")) == NULL) {
		fprintf(stdout,"Can't open %s\n", inDicSrc);
		return -10;
	}

	if ((fo = fopen(outDicSrc, "wb")) == NULL) {
		fprintf(stdout,"Can't open %s\n", outDicSrc);
		return -10;
	}

	strBuf[0] = 0xFEFF;	/* 予めBOMを書き込む */
	fwrite(&strBuf[0],sizeof(strBuf[0]),1,fo);

	for(i = 0;;i++){
		rv=una_fgetws(strBuf + 10,MAX_LINE_LEN,fp);
		if (rv < 0){
			break;
		}
		/* 読み込んだ1行の長さチェック */
		if (rv >= MAX_LINE_LEN - 1) { /* 行長がオーバーフローの可能性あり */
			fprintf(stdout,
					"Possibly the line buffer of %s overflow\n",inDicSrc);
			return -11;
		}
		/* 行番号を付加する */
		AddStrLine(strBuf,i);
		/* ファイルへ1行書き出す */
		fwrite(strBuf,(rv + 10) * 2,1,fo);
		strBuf[0] = 0x000A;	/* 改行コード */
		fwrite(&strBuf[0],sizeof(strBuf[0]),1,fo);
	}

	fclose(fp);
	fclose(fo);

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  AddStrLine
//
// ABSTRACT:	数値を文字に変換しバッファの先頭に付加する
//
// FUNCTION:
//	  x を9桁のUCS2文字列に変換し、strBufの先頭に付加する
//
// RETURN:
//	  0			正常終了
//
// NOTE:
//	  変換後の数値は、9桁のUCS2文字である。
//
static int AddStrLine(
	unaCharT *strBuf,
	int x
)
{
	int i;						/* ループ変数(tmpBuf の桁) */
	int j;						/* ループ変数(buf の桁) */
	int y;						/* x の 1/10 の数(切り捨て) */
	unaCharT tmpBuf[11];	/* 逆さになった文字列がセットされる */

	if (x > 999999999) {
		fprintf(stdout,"Line Overflow %d\n", x);
		return -10;
	}

	y = 1;
	for (i = 0; y != 0; i++){
		y = x / 10;
		tmpBuf[i] = (unaCharT)(0x0030 + (x - y * 10));
		x = y;
	}
	/* 上記 for ループを抜けた時点で、i には文字列長がセットされている */
	for (j = 0;j < 9 - i;j++) {	/* (9 - i)桁がつめるべき 0 の数 */
		strBuf[j] = 0x0030;	/* 先行0 */
	}
	for (j = 0;j < i;j++){
		strBuf[9 - i + j] = tmpBuf[(i - 1) - j];
										/* (i - 1)は、要素番号の最大値 */
	}
	strBuf[9] = 0x0020;	/* スペース */

	return 0;
}

//--------------------------------------------------------------------------
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
