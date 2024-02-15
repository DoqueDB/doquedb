//
// mkstdtbl.cpp -
//      UNA V3の文字変換テーブル作成モジュール
//      UNA V3の文字変換テーブルを作成するモジュール
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
#include	<string.h>
#include	<locale.h>		/* 評価用 */
#include	<sys/stat.h>	/* stat */
#include	<assert.h>		/* デバッグ用 */
#include	"una.h"			/* UNAグローバルなヘッダファイル */
#include	"unaconst.h"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define	LOCAL_AVE_TSTR_LEN	        4 /* 対象文字列の平均の桁数(バイトではない)  */

#define	LOCAL_MAX_REC_NO	   200000 /* 入力できる最大レコード数(最大0x1000000)
									 appIのidが24ビットという制約による */

#define	LOCAL_MAX_DA		   600000 /* RevuzのTrie用配列の大きさ         */

/* 変換後文字列全体の格納用プール */
#define	LOCAL_MAX_SSTRINGS_POOL 19000000 /* バイト数                          */

/* 変換前文字列全体の格納用プール */
#define	LOCAL_MAX_TSTRINGS_POOL (LOCAL_MAX_REC_NO * (LOCAL_AVE_TSTR_LEN + 1)) /* 文字数 */

#define STOP_CHARS_SIZE 0x10000 /* 変換処理終了用の文字テーブルサイズ(固定) */
#define NO					0 /* フラグ値					           */
#define YES					1 /* フラグ値					           */
#define EOK			   0x0000 /* End Of Key                            */
#define CLEAR_VALUE	   0xffff /* Trie配列初期化用の値                  */


//--------------------------------------------------------------------------
// TAG:   SstringT
//
// ABSTRACT:     変換後文字列型
//
// NOTE:
//		変換前文字列→変換後文字列において、変換後文字列を扱うための型。
//
typedef struct SstringT{             // 変換後文字列型
        int len;                                       /* 長さ(バイト数) */
        char body[1];                           /* 実際は可変長のデータ */
}SstringT;


//--------------------------------------------------------------------------
// TAG:	  StrPoolIdxT
//
// ABSTRACT:	  文字列プール型
//
// NOTE:
//	  変換前文字列が格納された文字列プールへのポインタと、長さの
//	  情報を管理する構造体である。
//
typedef struct StrPoolIdxT{	// 文字列プール型
	unaCharT *tStrPtr;			/* unastdTstrPool へのポインタ */
	ucharT	 tStrLen;			/* 文字数 */
} StrPoolIdxT;


//--------------------------------------------------------------------------
// 関数のプロトタイプ宣言
//--------------------------------------------------------------------------
static int GetOptions(int argc,char *argv[],char *comment,
	   int *verboseMode, int *processedArgc);
static void OutputUsage(void);
static int OptionA(int argc,char *argv[],char *comment,int *processedArgc);
static int ReadTblSrc(char *tblSrc);
static int ReadStopChrLst(char *stopChrLst);
static int ChkLineLen(int inStrLen);
static int ChkOverflowRecNo(int i);
static int ChkOverflowSstrPool(int tokenCnt,int inStrLen,
	   unaStrIndexT *token,char *sStrPtr);
static int TstringSet(unaCharT *tStrPtr,unaCharT *const tStrSrc,
	   int srcLen,int *dstLen);
static int SstringSet(char	*sStrDst,unaCharT *const sStrSrc,
	   int srcLen,int *dstLen);
static int GetPadding(int len);
static int Revuz(uintT *r,const uintT startRec,const uintT endRec,
	   const uintT targetPos);
static int FindBase(uintT *r,const uintT startRec,const uintT endRec,
	   const uintT targetPos);
static int WriteTable(char *tblExe);

//--------------------------------------------------------------------------
// グローバル変数
//--------------------------------------------------------------------------

/* ヘッダー用 */
static char unastdComment[UNA_COM_SIZE];	/* テーブルに付けるコメント */
static char unastdVersion[UNA_VER_SIZE] = UNA_STD_VER; /*テーブルのバージョン*/

static uintT unastdRecNo = 0;	  /* レコード番号。所謂 ID(0起算) */
static uintT unastdMaxBase = 0; /* 最大のbase値がセットされる(daCount計算用)*/
static uintT unastdMaxOccupy = 0;	/* 最大の遷移先番号がセットされる(同上) */

/* 配列 */
static char unastdStopTbl[STOP_CHARS_SIZE]; /* 変換処理終了のための文字リスト */
static uintT   unastdBase[LOCAL_MAX_DA];		   /* 索引部(RevuzのTrie) */
static unaCharT   unastdLabel[LOCAL_MAX_DA];	   /* 索引部(RevuzのTrie) */
static ucharT   unastdOccupy[LOCAL_MAX_DA];	   /* 索引部(RevuzのTrie) */
static uintT unastdSstrOffset[LOCAL_MAX_REC_NO]; /* 索引部(情報部のオフセット) */
static char	  unastdSstrPool[LOCAL_MAX_SSTRINGS_POOL]; /* 情報部(SstringT型のデータ) */

/* 表記が書かれてる形態素辞書作成用の配列。これを元にRevuzのTrieを作る */
static StrPoolIdxT unastdTstrIdx[LOCAL_MAX_REC_NO]; /* 表記へのポインタ、長さ */
static unaCharT	unastdTstrPool[LOCAL_MAX_TSTRINGS_POOL]; /* 全レコードの表記をプール */

/* スピードアップ用 */
static uintT	unastdStartBase = 0;	/* 空きベースを探し始める最初の位置 */

/* 評価用 */
static uintT unastdBaseCount = 0;	/* ベースの数 */
static uintT unastdOccupyCount = 0; /* Occupyの数(使用済Revuz配列要素数) */

/* 冗長情報モード用 */
static int unastdVerboseMode; /* 冗長情報モードで実行するかのフラグ */


//--------------------------------------------------------------------------
// MODULE:	  main
//
// ABSTRACT:	メインモジュール
//
// FUNCTION:
//	 UNA文字変換テーブルソース及び変換終了文字リストから、
//   実行用文字変換テーブルを作成する。
//
//	 使い方は、コマンド行から次の通りに入力する。
//	 mkstdtbl.exe [オプション] 文字変換テーブルソース 処理対象キー文字リスト
//											 実行用文字変換テーブル
//
// 例)
//	 mkstdtbl -a "For Ihyouki" -v stdtbl.src keychar.src unastd.tbl
//
// RETURN:
//	 0			正常
//	 1-255		エラー
//
// NOTE:
//	 文字変換テーブルソースは、表記をキーとして予めソートされていること
//
int main(
	int argc,			/* コマンド行の引数の数(1以上) */
	char *argv[]		/* コマンド行の引数 */
)
{
	int rv;			/* 関数の返り値 */
	int i;				/* ループ変数 */
	int processedArgc;	/* 処理した引数の数 */
	char *tblSrc;		/* 文字変換テーブルソース */
	char *stopChrLst;	/* 変換終了リスト */
	char *tblExe;		/* 実行用文字変換テーブル */
	uintT targetPos;	/* 処理対象とするレコードのカラム(要素番号、0起算) */
	uintT startRec;	/* 処理対象とする開始レコード(レコード番号、0起算) */
	uintT endRec;		/* 処理対象とする終了レコード(レコード番号、0起算) */
	uintT childR;		/* ベース */
	int   nullCheck;

	/* 初期設定 */
	for (i = 0; i < LOCAL_MAX_DA; i++){
		unastdLabel[i] = CLEAR_VALUE;
		unastdBase[i] = 0xFFFFFFFF;	/* 最上位1ビットがたつ値でクリア */
	}
	for (i = 0; i < STOP_CHARS_SIZE; i++){
		unastdStopTbl[i] = 0;
	}

	/* コマンドライン引数の処理 */
	unastdVerboseMode	= NO; 
	rv = GetOptions(argc,argv,unastdComment,&unastdVerboseMode, &processedArgc);
	if (rv < 0){
  		return 1;
	}

	if (argc - processedArgc != 3) { /* オプション以外に固定部3パラメータ必要*/
  		fprintf(stdout,"Argument Err\n");
		return 2;
	}


	/* I/Oファイル名の設定 */
	tblSrc      = argv[processedArgc + 0];
	stopChrLst  = argv[processedArgc + 1];
	tblExe    	= argv[processedArgc + 2];


	/* 辞書ソースのメモリへの格納 */
	rv = ReadTblSrc(tblSrc);
	if (rv < 0){
		return 5;
	}

	/* nullデータ対応 */
	nullCheck = 0;
	if ( rv == 0){
		nullCheck = 1;
	}

	if (unastdVerboseMode == YES) {
		fprintf(stdout,"REVUZ-001, Input %d lines\n",rv);
	}
	endRec = rv - 1;		/* 返り値は件数 */


	/* 変換終了リストのメモリへの格納 */
	rv = ReadStopChrLst(stopChrLst);
	if (rv < 0){
		return 5;
	}


	/* Revuz の Trie 辞書の作成(on memory) */
	if ( !nullCheck){
		startRec = 0;
		targetPos = 0;
		rv = Revuz(&childR,startRec,endRec,targetPos);
		if (rv < 0){
 			return 7;
		}
	}

	/* 辞書の出力 */
	rv = WriteTable(tblExe);
	if (rv < 0){
		return 8;
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
	int *verboseMode,	/* ヴァーボーズモードで実行するかのフラグ */
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
		case 'v':
			*verboseMode = YES;
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
	fprintf(stdout,"mkstdtbl (Ver1.02)\n");
	fprintf(stdout,"Usage: mkstdtbl [OPTION] ");
	fprintf(stdout,"TBL_SRC KEY_CHAR_LST TBL_EXE\n");
	fprintf(stdout,"Ex   : mkstdtbl -a \"For Ihyouki\" -v ");
	fprintf(stdout,"stdtbl.src keychar.src unastd.tbl\n");
	fprintf(stdout,"  OPTION\n");
	fprintf(stdout,"    -h	           Help\n");
	fprintf(stdout,"    -a ANNOTATION  The String to annotate(Max48byte)\n");
	fprintf(stdout,"    -v             Verbose MODE");
	fprintf(stdout,"(Display progress messages)\n");
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
// MODULE:	  ReadStopChrLst
//
// ABSTRACT:	  変換終了文字リストを読み込む
//
// FUNCTION:
//	  変換終了文字リストを読み込みグローバルメモリに格納する
//
// RETURN:
//	  負の値	エラー
//	  それ以外	読み込んだ文字数
//
// NOTE:
//	  処理速度及び安定性という点から格納先のメモリをグローバルメモリ
//	  (外部static変数)にしてある
//
static int ReadStopChrLst(
	char		 *stopChrLst		   /* 変換終了文字リスト */
)
{
	FILE *fi;						   /* ファイルポインタ */
	int i;							   /* レコードの件数を数える */
	unaCharT	 inStr[MAX_LINE_LEN];  /* 読み込まれた文字列 */
	unaCharT     tmpStr[5];            /* \u変換のための文字列 */
	unaCharT     stopChar;             /* 変換終了文字の1文字分 */
	int         inStrLen;             /* 読み込まれた文字列長 */

	
	/* 変換終了文字リストのオープン */
	if((fi = fopen(stopChrLst,"rb")) == NULL) {
		fprintf(stderr,"Can't open %s\n",stopChrLst);
		return -7;
	}

	i = 0; /* 入力文字数をカウントする */
	while( (inStrLen=una_fgetws(inStr,MAX_LINE_LEN,fi))>= 0) {

		/* 先頭の1文字分しか評価しない */
		if ((inStr[0] == 0x005C)				/* '\'(バックスラッシュ) */
				&& (inStr[1] == 0x0075)			/* '\'の後ろの文字が 'u' */
				&& (5 < inStrLen)					/* 変換の桁が十分 */
				&& (una_isxstr(&inStr[2],4) != 0)) {	/* 16進の文字 */
			memcpy(tmpStr,&inStr[2],4 * sizeof(unaCharT));
			tmpStr[4] = 0x0000;
			stopChar = (unaCharT)una_xtol(tmpStr);
			if (stopChar == 0x0000 || stopChar == 0xFEFF){
				fprintf(stdout,"\\uXXXX that means NULL or BOM exists\n");
				return -15;
			}
		}
		else {
			stopChar = *inStr;
		}

		unastdStopTbl[stopChar] = 1;

		/* 読み込んだ文字数をカウント */
		i++;
	}
	
	/* ヌルターミネートを取り扱うため、コード０については確実に変換終了 */
	unastdStopTbl[0] = 1;

	fclose(fi);


	return i;
}


//--------------------------------------------------------------------------
// MODULE:	  ReadTblSrc
//
// ABSTRACT:	  文字変換テーブルソースを読み込む
//
// FUNCTION:
//	  文字変換テーブルソースの全レコードを読み込みグローバルメモリに格納する
//
// RETURN:
//	  負の値	エラー
//	  それ以外	読み込んだレコード数
//
// NOTE:
//	  処理速度及び安定性という点から格納先のメモリをグローバルメモリ
//	  (外部static変数)にしてあるのでオーバーフローには十分注意する。
//
static int ReadTblSrc(
	char		 *tblSrc			   /* 辞書ソース */
)
{
	FILE *fi;						   /* ファイルポインタ */
	int i;							   /* レコードの件数を数える */
	int inStrLen;					   /* レコード長 */
	int rv;						   /* 関数の返り値 */
	unaCharT	 inStr[MAX_LINE_LEN];  /* 読み込まれた文字列 */
	unaStrIndexT token[MAX_TOKEN_CNT]; /* トークン */
	int		 tokenCnt;			   /* トークンの数 */
	unaCharT	 *tStrPtr;			   /* unaWordPoolを指すポインタ */
	int		 tStrLen;			   /* 表記の文字数(\u変換後の) */
	char		 *sStrPtr;			   /* 変換後文字列のポインタ情報を保持 */
	uintT		 sStrPoolOffset;	   /* unastdSstrPoolのオフセット */
	int		 padding;			   /* 処理系の違いのため必要 */

	/* 初期設定 */
	tStrPtr = unastdTstrPool;
	sStrPtr = unastdSstrPool;
	sStrPoolOffset = 0;
	
	/* 辞書ソースのオープン */
	if((fi = fopen(tblSrc,"rb")) == NULL) {
		fprintf(stderr,"Can't open %s\n",tblSrc);
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
		if (tokenCnt < 1) {
 			fprintf(stdout,"%d is insufficient token count for %s\n",
					tokenCnt,tblSrc);
			return -10;
		}

		/* コメントなら無視する */
		if ( inStr[ token[0].pos] == '#'){
			continue;
		}
		/* unastdTstrPoolオーバーフローチェック(ターミネート分も考慮してある) */
		if(tStrPtr + token[0].len > &unastdTstrPool[LOCAL_MAX_TSTRINGS_POOL - 1]){
			fprintf(stdout,"The array 'unastdTstrPool' is overflow.\n"
			               "Increase LOCAL_MAX_TSTRINGS_POOL\n");
			return -11;
		}

		/* 変換前文字列(第1トークン)を unastdTstrPool に書き込む */
		rv = TstringSet(tStrPtr,inStr + token[0].pos,token[0].len,&tStrLen);
		if (rv < 0){
			return rv;
		}

		/* 変換前文字列のインデックスを書き込む */
		unastdTstrIdx[i].tStrLen	= (ucharT)tStrLen;
		unastdTstrIdx[i].tStrPtr	= tStrPtr;
		tStrPtr += (tStrLen + 1);	/* ポインタの更新(EOKの分含む) */

		/* unastdSstrPool オーバーフローチェック(byteで)。なお padding は
				実データを書き込むわけではないのでここでは考えなくてよい */
		rv = ChkOverflowSstrPool(tokenCnt,inStrLen,token,sStrPtr);
		if (rv < 0){
			return rv;
		}

		/* アプリ情報(第2トークン以降)を unastdSstrPool に書き込む */
		if (tokenCnt < 2){	/* アプリ情報がない時 */
				((SstringT *)sStrPtr)->body[0]	= 0;
				((SstringT *)sStrPtr)->len		= 0;
		}
		else{
			rv = SstringSet(((SstringT *)sStrPtr)->body,
							inStr+token[1].pos,(inStrLen - token[1].pos),
							&((SstringT *)sStrPtr)->len);
			if (rv < 0){
				return rv;
			}
		}

		/* アプリ情報のオフセットを書き込む*/
		unastdSstrOffset[i] = sStrPoolOffset;

		/* 語境界をそろえる為(SPARCではintは4の倍数に配置される必要あり) */
		padding = GetPadding(((SstringT *)sStrPtr)->len);
		
		/* オフセットの更新sizeof(((SstringT *)sStrPtr)->len) = 4です*/
		sStrPoolOffset += (((SstringT *)sStrPtr)->len
					  + sizeof(((SstringT *)sStrPtr)->len)) + padding;
		sStrPtr = unastdSstrPool + sStrPoolOffset; /* 次の書き始め位置の更新 */

		/* 読み込んだレコード数をカウント */
		i++;
	}

	fclose(fi);


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
	if (inStrLen >= MAX_LINE_LEN - 1) { /* 行長オーバーフローの可能性有*/
		fprintf(stdout,
			"Possibly the line buffer of STD_TBL_SRC overflow\n");
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
	if (i >= LOCAL_MAX_REC_NO){
		fprintf(stdout,
			"Line count of table source exceeded LOCAL_MAX_REC_NO\n"
		    "Increase LOCAL_MAX_REC_NO\n");
		return -9;
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  ChkOverflowSstrPool
//
// ABSTRACT:	  変換後文字列プールオーバーフローチェック
//
// FUNCTION:
//	  変換後文字列プールのオーバーフローをチェックする
//
// RETURN:
//	  UNA_OK		正常終了
//	  それ以外		エラー
//
// NOTE:
//
static int ChkOverflowSstrPool(
	int		 tokenCnt,		/* トークンの数 */
	int		 inStrLen,		/* レコード長 */
	unaStrIndexT *token,		/* トークン */
	char		 *sStrPtr	/* 変換後文字列プールのポインタ情報を保持 */
)
{
	uintT		 tmpSstrByte; /* 変換後文字列のバイト数(\u、\x変換前の) */

	if (tokenCnt < 2){	/* アプリ情報がない時 */
		tmpSstrByte = sizeof(SstringT);
	}
	else {
		tmpSstrByte = (inStrLen - token[1].pos) * sizeof(unaCharT)
														+ sizeof(char *);
	}
	if(sStrPtr + tmpSstrByte - 1 > &unastdSstrPool[LOCAL_MAX_SSTRINGS_POOL - 1]){
		fprintf(stdout,"The array 'unastdSstrPool' is overflow.\n"
		               "Increase LOCAL_MAX_SSTRINGS_POOL\n");
		return -12;
	}
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  TstringSet
//
// ABSTRACT:	  変換前文字列のセット
//
// FUNCTION:
//	  変換前文字列をセットする
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  srcLen = 0 の場合は、*tStrDst に NULL だけが書き込まれ、
//	  *dstLen = 0 となる
//
static int TstringSet(
	unaCharT	*tStrDst,		/* コピー先(UNICODE) */
	unaCharT	*const tStrSrc, /* コピー元(UNICODE) */
	int		srcLen,		 	/* コピー元の長さ(文字数) */
	int		*dstLen	 		/* 実際にコピーした長さ(文字数) */
	)
{
	int i;						 /* ループ変数(処理した文字数を表わす) */
	unaCharT tmpStr[5];			 /* una_xtol用(ターミネートするため必要) */

	*dstLen = 0;
	for(i = 0;i < srcLen;){
		if ((tStrSrc[i] == 0x005C)				/* '\'(バックスラッシュ) */
		&& (tStrSrc[i + 1] == 0x0075)			/* '\'の後ろの文字が 'u' */
		&& (i + 5 < srcLen)					/* 変換の桁が十分 */
		&& (una_isxstr(&tStrSrc[i + 2],4) != 0)) {	/* 16進の文字 */
			memcpy(tmpStr,&tStrSrc[i + 2],4 * sizeof(unaCharT));
			tmpStr[4] = 0x0000;
			*tStrDst = (unaCharT)una_xtol(tmpStr);
			if (*tStrDst == 0x0000 || *tStrDst == 0xFEFF){
				fprintf(stdout,"\\uXXXX that means NULL or BOM exists\n");
				return -15;
			}
			i += 6;
			tStrDst ++;
			(*dstLen) ++;
		}
		else {
			*tStrDst = *(tStrSrc + i);
			i++;
			tStrDst++;
			(*dstLen)++;
		}
	}

	/* 表記長制限チェック */
	if (*dstLen > UNA_HYOKI_LEN_MAX) {
		fprintf(stdout,"Too long Hyoki(len=%d)\n",*dstLen);
		return -17;
	}
	
	*tStrDst = 0x0000; /* 最後はヌルでターミネート */

	return UNA_OK; 
}



//--------------------------------------------------------------------------
// MODULE:	  SstringSet
//
// ABSTRACT:	  変換後文字列のセット
//
// FUNCTION:
//	  変換後文字列をセットする
//
// RETURN:
//		UNA_OK					正常終了
//		その他					エラー
//
// NOTE:
//	  なし
//
static int SstringSet(
	char		*sStrDst, /* コピー先(BINARY) */
	unaCharT	*const sStrSrc,	 /* コピー元(UNICODE) */
	int		srcLen,		 /* コピー元の長さ(文字数) */
	int		*dstLen	 /* 実際にコピーした長さ(バイト数) */
)
{
	int i;						 /* ループ変数(処理した文字数を表わす) */
	unaCharT tmpStr[5];			 /* una_xtol用(ターミネートするため必要) */

	*dstLen = 0;
	for(i = 0;i < srcLen;){
		if (sStrSrc[i] == 0x005C) {	/* '\'(バックスラッシュ) */
			if ((sStrSrc[i + 1] == 0x0075)			/* 'u' */
			&& (i + 5 < srcLen)					/* 変換の桁が十分 */
			&& (una_isxstr(&sStrSrc[i + 2],4) != 0)) {	/* 16進の文字 */
				memcpy(tmpStr,&sStrSrc[i + 2], 4 * sizeof(unaCharT));
				tmpStr[4] = 0x0000;
				*(unaCharT *)sStrDst = (unaCharT)una_xtol(tmpStr);
				i += 6;
				sStrDst += 2;
				(*dstLen) += 2;
				continue;
			}
			else if ((sStrSrc[i + 1] == 0x0078)		/* 'x' */
			&& (i + 3 < srcLen)					/* 変換の桁が十分 */
			&& (una_isxstr(&sStrSrc[i + 2],2) != 0)) {	/* 16進の文字 */
				memcpy(tmpStr,&sStrSrc[i + 2], 2 * sizeof(unaCharT));
				tmpStr[2] = 0x0000;
				*sStrDst = (char)una_xtol(tmpStr);
				i += 4;
				sStrDst++;
				(*dstLen)++;
				continue;
			}
		}

		*((unaCharT *)sStrDst) = *(sStrSrc + i);
		i++;
		sStrDst += 2;
		(*dstLen) += 2;
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
//	  の配列にセットした後ベース値を返す
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
	oldUniChar = (unastdTstrIdx[startRec].tStrPtr)[targetPos];

	for(i = startRec;i <= endRec + 1;i++){ /* i の最大値は、endRec + 1 */

		/* 下位のRevuz関数に渡す為にedRecOfSamePrfxを決定する */
		if (i <= endRec){
			if (oldUniChar == (unastdTstrIdx[i].tStrPtr)[targetPos]){
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
			unastdBase[*r + oldUniChar] = childR;
		}
		else{			/* 語の最後の時(最後はEOKを処理することになる) */
			unastdLabel[*r] = 0;	/* ラベルは0(実はFindBaseでセット済み) */
			unastdBase[*r] = unastdRecNo;	/* ラベルが0の時のベースはRecNo */
			
			/* 以下同形語の間ループ(同形語がない場合は1ループして抜ける) */
			for (k = stRecOfSamePrfx; k <= edRecOfSamePrfx;k++){

				assert(unastdRecNo == k);
				if (unastdVerboseMode == YES){
					fprintf(stdout,"REVUZ-002, Process %u lines\n",k + 1);
				}

				unastdRecNo ++; /* 次回登録時のRecNoになる。最後はRec数を示す*/
			}
		}

		/* 次回ループのための前設定
		  (ブレーク処理とEOL処理を一緒にしたためここにif文が入ってしまった)*/
		if (i < endRec + 1){ /* 最終ループ(i == endRec + 1)でない時 */
			assert (i == edRecOfSamePrfx + 1);
			stRecOfSamePrfx = edRecOfSamePrfx + 1; /* stRecOfSamePrfx = i */
			oldUniChar = (unastdTstrIdx[i].tStrPtr)[targetPos];
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
//		例えば unastdOccupyが以下のようになっている時、既にラベルとして使用
//		されている場所も他のベースとして使用可能なはずなので unastdStartBase
//		は、最初のベース候補として[4]がセットされるが[4]は絶対にベースには
//		なり得ない。なぜなら0x00000xFFFFの全文字が遷移不可能だからである。
//		なお、 unastdOccupy配列は 3値を取り、その値は
//		0:全くの未使用 1:ベースとして既使用 2:ラベルとして既使用 である。
//		(-cオプション無しなら、unastdOccupy配列は0と1の二値である)
//		(例)
//		unastdOccupy  1 1 1 1 2 1 1 1 1 1 2・・・2 2 1 1 1 1 0 1 2 1
//		                   ↑                       ↑
//		                  [4]                    [4+0xFFFF]
//
//		さて、[4 + 0xFFFF + 3] = [7 + 0xFFFF] = 0 なので、unastdStartBaseは
//		最低でも [7 + 0xFFFF - 0xFFFF] = [7] 以上の個所である必要がある。
//		よって、unastdOccupy配列の値 = 0 となる最小の要素 i を求めて
//		unastdStartBase + 0xFFFF < i なら unastdStartBase をリセットするという
//		処理が必要になる。
//		ただし、unastdStartBase が示す個所が、ベース候補となり得るかは
//		別である。なお、この処理ない場合には、Pent-Ⅲ 450MHz 256Kb で
//		約175000レコードの処理に18分30秒かかり、これは処理がある場合の
//		約2.5倍である。(レコード件数が増えると級数的に遅くなる。
//		この処理がある時は1件あたりの処理スピードは安定している)
//
//  (注2)
//		-c オプション実行の場合
//		unique配列に、EOK(= 0)が含まれていた時、かつ、ベース候補が既に
//		ラベルとして使用されているところだった場合、ラベルが書き換わって
//		しまうのではないかかという恐れが考えられるが、ここでチェック
//		してあるので大丈夫。
//
//  (注3)
//		-c オプション実行の場合
//		unastdOccupy配列の値が 2:ラベルとして既使用の時、その場所が他のベースに
//		なりうる場合には、unastdOccupy配列の値は 1 に書きかえられる。
//		即ち unastdOccupy[*r] は、0 又は 2 であったが、ここで 1 なる。
//
//  (注4)
//		ラベルを書き込む場合に unique配列に EOK(= 0) が含まれていた
//		時は *r + unique[i] は(= *r で)ベース自身を差し示すことになる。
//		よって
//		- 丁度 unastdLabel[*r] = EOK(= 0) となりうまく行っている。
//		  (ただ、revez関数の中でEOK処理として再度0を書き込んでいるが)
//		- unastdOccupy[*r] には、既に 1 が立ててあるし、unastdOccupyCount
//		  の中にもカウント済みである。
//
static int FindBase(
	uintT			*r,			/* 見つかったベース値 */
	const uintT	startRec,	/* 開始レコード番号(0起算) */
	const uintT	endRec,		/* 終了レコード番号(0起算) */
	const uintT	targetPos	/* 処理対象とするカラム(0起算) */
)
{
	/* 受け取った文字列の1桁目を重複のない様に格納する配列 */
	static   unaCharT unique[0xFFFF + 1]; /* ユニーク配列(static) */
	uintT	 uniqueCount;	/* 上記配列にセットされたユニークだった文字数 */
	uintT	 i;				/* ループ変数 */
	uintT	 j;				/* ループ変数 */
	unaCharT oldUniChar;	/* 1つ前の行の文字(コントロールブレーク用) */
	uintT	 findFlg;		/* unastdStartBase セット用のためのフラグ
							   0:ベース候補が発見されてない 1:発見された */
	uintT	 noUsFlg;		/* unastdStartBase リセット用のためのフラグ
							   0:未使用ベースが発見されてない 1:発見された */

	/* 受け取った文字列の1桁目を unique 配列にセットする。既にセット済みの
	   文字は格納しない。なお、unique 配列はソートされるはずである */
	uniqueCount = 0;
	oldUniChar = (unastdTstrIdx[startRec].tStrPtr)[targetPos];
	for( i = startRec ;; i ++)	{	/* forever */
		if (i > endRec) {			/* 終了 */
			unique[uniqueCount] = oldUniChar;
			uniqueCount++;			/* これがないと数が合わない */
			break;
		}
		/* コントロールブレークする毎にセット */
		if (oldUniChar != (unastdTstrIdx[i].tStrPtr)[targetPos]){
			unique[uniqueCount] = oldUniChar;
			oldUniChar = (unastdTstrIdx[i].tStrPtr)[targetPos];
			uniqueCount++;
		}
	}

	/* 一つずつベース値をずらしながら、unique 配列にセットされた文字が遷移
	   できる最小のベース値を探す */
	findFlg = 0; /* ベース候補未発見とする */
	noUsFlg = 1; /* -c オプション時未使用ベース未発見とする */

	for (i = unastdStartBase;i < LOCAL_MAX_DA ;i++){ /* i がテストしているベース値 */

		/* ベース候補を見つける */
		if (findFlg == 0){ /* まだベース候補は見つかってない */
			if (unastdOccupy[i] == 1) { /* ベースとして使用済み */
				continue;			 /* 読み飛ばす */
			}
			else {					 /* 未使用発見 */
				unastdStartBase = i;	 /* 初めて見つかった未使用の値を保存 */
				findFlg = 1;		 /* unastdStartBaseセット後は else 通る様 */
			}
		}
		else {	/* 既に未使用が発見されている時はずっとこちらを通る */
			if (unastdOccupy[i] == 1) { /* ベースとして使用済み */
				continue;			 /* 読み飛ばす */
			}
		}

		/* -c オプション実行の場合(注1) */
		if (noUsFlg == 0) {
			if (unastdOccupy[i] == 0) {
				if (unastdStartBase + 0xFFFF < i){
					unastdStartBase = i - 0xFFFF;
				}
				noUsFlg = 1;
			}
		}
		
		/* 見つかった未使用ベースについて unique 配列の全文字が格納可能
		   であるかをテストする */
		for ( j = 0 ; j < uniqueCount ; j++ ){
			if(i + unique[j] >= LOCAL_MAX_DA){ /* Revuz配列オーバーフローチェック*/
				fprintf(stdout,
					"The arrays of 'Trie' is overflow.\n");
				fprintf(stdout,"Increase LOCAL_MAX_DA\n");
				return -24;
			}
			/* -c オプション実行の場合(注2) */
			if(unastdOccupy[i + unique[j]] != 0){ /* 格納できないものあり */
				break;	/* 既に遷移先がベースかラベルになっている */
			}
		}
		if (j == uniqueCount){ /* jが最後までループした時。即ちunique配列に
							      セットされた全文字が格納可能である時 */
			*r = i;			   /* ベース発見! */
			break;
		}

	}

	unastdOccupy[*r] = 1;	/* ベースの場所は無条件で使用済みとする */
	unastdOccupyCount++;

	/* ラベルの場所に使用済みフラグを立て、Label値を書き込む */
	for ( i = 0 ; i < uniqueCount; i++){

		/* ラベルを書き込む場合の考察(注4) */
		unastdLabel[*r + unique[i]] = unique[i];	/* Label値を書き込む */
		if(unique[i] != EOK){

			/* -c オプション実行の場合とそうでない場合の考察(注5) */
			unastdOccupy[*r + unique[i]] = 1; /* 使用済み */
			unastdOccupyCount++;
		}

		if (unastdMaxOccupy< *r + unique[i]){ /* ベース+ラベルの方がでかい時 */
			unastdMaxOccupy= *r+unique[i]; /*ベース+ラベルをunastdMaxOccupyに*/
		}
	}

	unastdBaseCount ++;		/* 見つかるたびにベース数をカウントアップ */
	if (unastdMaxBase < *r){	/* 見つかったベースの方がでかい時 */
		unastdMaxBase = *r;	/* 見つかったベースを unastdMaxBase に */
	}

	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  WriteTable
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
static int WriteTable(
	char *tblExe	   /* 文字変換モジュール用のテーブル名 */
)
{
	FILE *fp;		   /* ファイルポインタ */
	uintT sStrPoolsiz; /* アプリ辞書情報部(SstringT型データプール)のSIZE*/
	uintT daCount;	   /* 形態素辞書索引部(RevuzのTrie配列)の大きさ */


	/*
	 * データ出力
	 */
	if ((fp = fopen(tblExe,"wb")) == NULL) {
		fprintf(stderr,"Can't open %s\n\n",tblExe);
		return -26;
	}

	/* 変換前文字列索引部(RevuzのTrie配列)の大きさを以下のでかい方にする */
	if (unastdMaxBase + 0xffff + 1 > unastdMaxOccupy + 1){
		daCount = unastdMaxBase + 0xffff + 1;
	}
	else{
		daCount = unastdMaxOccupy + 1;
	}
	/* さらに偶数に整形 */
	daCount = (daCount+1)/2*2;

	/* 変換後文字列格納部(SstringT型データプール)のサイズを計算。
	   なおRevuz関数終了後 unastdRecNo は、全レコード数を示している(よって-1)*/
	sStrPoolsiz = unastdSstrOffset[unastdRecNo - 1] + sizeof(char *)
		+ ((SstringT *)(unastdSstrPool+unastdSstrOffset[unastdRecNo-1]))->len;


	/* ヘッダー出力 */
	fwrite(unastdComment,sizeof(unastdComment),1,fp);  /* コメント */
	fwrite(unastdVersion,sizeof(unastdVersion),1,fp);  /* バージョン */

	fwrite(&unastdRecNo,sizeof(unastdRecNo),1,fp);     /* 変換ルール数 */
	fwrite(&daCount,sizeof(daCount),1,fp);             /* Revuz配列のサイズ */

	/* 変換終了文字テーブルの格納 */
	fwrite(unastdStopTbl,sizeof(ucharT),STOP_CHARS_SIZE,fp);

	/* 変換前文字列の格納 */
	fwrite(unastdBase,sizeof(uintT),daCount,fp);			 /* 索引部 BASE */
	fwrite(unastdLabel,sizeof(unaCharT),daCount,fp);		 /* 索引部 LABEL */

	/* 変換後文字列の索引(直接unastdBaseに書けば不要かもしれない) */
	fwrite(unastdSstrOffset,sizeof(uintT),unastdRecNo,fp);	/* 索引部 */

	/* 変換後文字列の本体 */
	fwrite(unastdSstrPool,sStrPoolsiz,1,fp);					/* 情報部 */


	fclose(fp);

	return UNA_OK;
}


//--------------------------------------------------------------------------
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
