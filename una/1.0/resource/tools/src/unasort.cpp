//
// unasort.cpp -
//      UNA 辞書ソースデータ用ソートプログラム
//      UNA V3の辞書ソースデータをマージソートするプログラム(UNICODE版)
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "una.h"
#include "unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
unaStrIndexT	*strIndex;		 /* 文字列プールの文字列へのインデックス */
unaCharT		*strPool;		 /* 文字列プール                         */

//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int LineComp(unaStrIndexT *w1,unaStrIndexT *w2);
static int HyokiSet(unaCharT *wordPtr,unaCharT *const hyokiPtr,
					int hyokiLen,int *realHyokiLen);


//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	メインモジュール
//
// FUNCTION:
//	 ソート前UNA用辞書ソースを読み込み、表記をソートキーとしてソートし、
//   ソート済みUNA用辞書ソースとして出力する。
//
//	 使い方は、コマンド行から次の通りに入力する。
//	 unasort.exe [オプション] ソート前辞書ソース ソート済み辞書ソース
//
// 例)
//	 unasort.exe unadic.src unadic.srt
//
// RETURN:
//	 0			正常
//	 1-255		エラー
//
// NOTE:
//
int main(
	 int argc,			/* コマンド行の引数の数(1以上) */
	 char *argv[]		/* コマンド行の引数 */
)
{
	FILE *fp;			/* ファイルポインタ */
	uintT  i;			/* ループ変数 */
	uintT  strPoolEnd;	/* 文字列プールの末尾 */
	uintT  strPoolMax;	/* 文字列プールの限界値 */
	uintT  lineEnd;	/* 行数 */
	uintT  lineMax;	/* 行数の限界値 */
	int  tmpStrLen;	/* 読み込んだ1行の長さ */
	unaCharT uniChar;	/* ユニコード1文字 */

	/* -h オプションが指定されてた時、または、何も引数がなかった時 */
	if (argc == 1 || strcmp(argv[1],"-h") == 0){
		fprintf(stdout,"unasort (Ver1.10)\n");
		fprintf(stdout,"Usage: unasort INPUT_FILE OUTPUT_FILE\n");
		fprintf(stdout,"Ex   : unasort unadic.src unadic2.src\n");
		fprintf(stdout,"Note\n");
		fprintf(stdout,"  UNICODE Sort Program for Una Dic Source.\n");
		fprintf(stdout,"  Used Merge Sort Algorithm.\n");
		return 0;
	}

	if (argc != 3){	/* 引数は、3 以外はエラー */
		printf("Argument Err\n");
		return 1;
	}

	/* まず最初分を確保 */
	strPool = (unaCharT *)malloc(STR_POOL_BLOCK);
	strIndex = (unaStrIndexT *)malloc(LINES_BLOCK * sizeof(unaStrIndexT));

	/* 初期値の設定 */
	strPoolEnd = 0;
	lineEnd    = 0;
	strPoolMax = STR_POOL_BLOCK;
	lineMax   = LINES_BLOCK;

	/*
	 * 入力ファイルの全レコードをバッファに読込む
	 */
	if ((fp = fopen(argv[1],"rb")) == NULL) {
		printf("Can't open%s\n",argv[1]);
		return 2;
	}
	while((tmpStrLen=una_fgetws(strPool+strPoolEnd,MAX_LINE_LEN,fp)) >= 0) {

		/* 読み込んだ1行の長さチェック */
		if (tmpStrLen >= MAX_LINE_LEN - 1) { /* オーバーフローの可能性あり */
			fprintf(stdout,"Maybe the line buffer of dic source overflow\n");
			fprintf(stdout,"Max size of line buffer is MAX_LINE_LEN\n");
			return 3;
		}

		(strIndex[lineEnd].pos) = strPoolEnd; /* 上記で読込んだ位置を記憶 */
		(strIndex[lineEnd].len) = tmpStrLen;  /* 上記で読込んだ長さを記憶 */

		/* 更新(改行だけの行はNULLだけが書き込まれる) */
		lineEnd++;
		strPoolEnd += (tmpStrLen + 1);	/* 1はターミネータの分 */

		/* 文字列バッファをオーバーフローする場合はリアロック */
		if ((strPoolEnd + MAX_LINE_LEN) * 2 >= strPoolMax){	/* バイトで計算*/
			strPoolMax += STR_POOL_BLOCK;
			strPool = (unaCharT *)realloc(strPool,strPoolMax);
		}
		/* 行バッファをオーバーフローする場合はリアロック */
		if ( lineEnd + 1 >= lineMax){
			lineMax += LINES_BLOCK;
			strIndex = (unaStrIndexT *)realloc(strIndex,
											lineMax * sizeof(unaStrIndexT));
		}
	}
	fclose(fp);

	/*
	 * マージソートする
	 */
	una_msort(strIndex,0,lineEnd - 1,sizeof(unaStrIndexT),
				(unaCompFuncT)LineComp,NULL);

	/*
	 * ファイルへ書き出す
	 */
	if ((fp = fopen(argv[2], "wb")) == NULL) {
		printf("Can't open%s\n",argv[2]);
		return 4;
	}
	uniChar = 0xFEFF;	/* 予めBOMを書き込む */
	fwrite(&uniChar,sizeof(uniChar),1,fp);
	uniChar = 0x000A;	/* 改行コード */
	for(i=0;i<lineEnd;i++){
		fwrite(strPool+(strIndex[i].pos),(strIndex[i].len) * 2,1,fp);
		fwrite(&uniChar,sizeof(uniChar),1,fp);
	}
	fclose(fp);

	/* メモリの free */
	free(strPool);
	free(strIndex);

	return 0;
}


//--------------------------------------------------------------------------
// MODULE:	  LineComp
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
int LineComp(
	unaStrIndexT *w1,					/* レコード1へのインデックス */
	unaStrIndexT *w2					/* レコード2へのインデックス */
)
{
	unaCharT	 *s1;					/* レコード(行)1 */
	unaCharT	 *s2;					/* レコード(行)2 */
	unaStrIndexT token[MAX_TOKEN_CNT];	/* トークン */
	int		 i;						/* ループ変数 */
	int		 rv;					/* 関数の返り値 */
	unaCharT	 keyStr1[MAX_LINE_LEN];	/* 比較すべきキー1 */
	unaCharT	 keyStr2[MAX_LINE_LEN];	/* 比較すべきキー2 */
	int		 tokenCnt;				/* トークン数 */
	int		 realHyokiLen;			/* 表記の実長(文字数) */

	/* 初期設定 */
	s1 = strPool + w1->pos;
	s2 = strPool + w2->pos;

	/*
	 * 比較すべきキー1を作る
	 */

	/* 1行をトークンに分解 */
	tokenCnt = una_getTokens(s1,MAX_TOKEN_CNT,token);
	if (tokenCnt < 2) {
		exit (10);	/* エラーの時返り値を戻せないので exit(正値) */
	}

	/* 表記を整形しつつキーにセット */
	rv = HyokiSet(keyStr1,s1+token[1].pos,token[1].len,&realHyokiLen);
	if (rv < 0) {
		exit (-rv);	/* エラーの時返り値を戻せないので exit(正値) */
	}

	*(keyStr1+realHyokiLen) = 0x0000; /* HyokiSetでされるが一応ターミネート*/
	/* トークンがないと *KeyStr1 は、NULL となる */

	/*
	 * 比較すべきキー2を作る
	 */

	tokenCnt = una_getTokens(s2,MAX_TOKEN_CNT,token);
	if (tokenCnt < 2) {
		exit (11);	/* エラーの時返り値を戻せないので exit(正値) */
	}

	/* 表記を整形しつつキーにセット */
	rv = HyokiSet(keyStr2,s2+token[1].pos,token[1].len,&realHyokiLen);
	if (rv < 0) {
		exit (-rv);	/* エラーの時返り値を戻せないので exit(正値) */
	}

	*(keyStr2+realHyokiLen) = 0x0000; /* HyokiSetでされるが一応ターミネート*/
	/* トークンがないと *KeyStr2 は、NULL となる */

	/* unaCharT を順番に比較 */
	for ( i = 0;; i++ ){
		if ( keyStr1[i] < keyStr2[i] ){
			return -1;
		}
		else if ( keyStr1[i] > keyStr2[i] ){
			return 1;
		}
		else if ( keyStr2[i] == 0 ){
			return 0;
		}
	}
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
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
