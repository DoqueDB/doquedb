//
// unastd.cpp -
//		文字変換モジュール
//		形態素解析処理の前に入力文字列を標準的な文字列に
//		置き換えるためのモジュール
// 
// Copyright (c) 2001-2009, 2023 Ricoh Company, Ltd.
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
#include <string.h>		/* for strcmp */
#include "UnaBase/unastd.h"		/* 文字変換モジュール自身 */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------
#define MODULE_NAME "UNASTD"	/* モジュール名 */

/* モジュール内のメッセージ */
#define ERR_VERSION_STD			"Unsupported Replace Table Version"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define EOK 0x0000                      /* 終了への遷移キーのコード */
#define CHECK_NUMBER 511268				/* 初期化チェック用マジックナンバー */

//--------------------------------------------------------------------------
// MODULE:	unaStd_init
//
// ABSTRACT:	UNAの文字変換モジュールの初期化
//
// FUNCTION:
//	UNAの文字変換モジュールの初期化を行う。
//
// RETURN:
//	UNA_OK				正常終了
//	UNA_ERR_STD_TBL_VER	文字変換モジュール用データテーブルのバージョン不一致
//	その他				その他のエラー
//
//	NOTE:
//	なし
//
int unaStd_init(
	unaStdHandleT *h,		/* 文字変換モジュール用ハンドラ */
	const char *repTblImg	/* 文字変換モジュール用データテーブルのイメージ */
)
{
	const char *imgPtr; /* テーブルを格納したメモリへのポインタ */

	/* NULLで渡された場合は、各値をNULLにしてcheckNumberをセットして終了 */
	if ( repTblImg == (const char*)NULL){
		h->repRuleNum = 0;
		h->arraySize = 0;
		h->stopCharTbl = (const char*)NULL;
		h->base = (uintT*)NULL;
		h->label = (unaCharT*)NULL;
		h->repStrIdx = (uintT*)NULL;
		h->repStr = (char*)NULL;
		h->checkNumber = 0;
		return UNA_OK;
	}

	/* イメージの設定 */
	imgPtr = repTblImg + UNA_COM_SIZE;

	/* バージョンチェック */
	if (strcmp(imgPtr,UNA_STD_VER)!=0) { /* バージョンが違う */
		UNA_RETURN(ERR_VERSION_STD,NULL);
	}
	imgPtr += sizeof(UNA_STD_VER);

	/* 変換ルール数設定 */
	h->repRuleNum = *(int*)imgPtr;
	imgPtr += sizeof(int);

	/* 配列サイズ設定 */
	h->arraySize = *(int*)imgPtr;
	imgPtr += sizeof(int);

	/* 変換終了文字テーブル設定 */
	h->stopCharTbl = imgPtr;
	imgPtr += 0x10000;  /* 固定値 */

	/* ベース配列設定 */
	h->base = (uintT*)imgPtr;
	imgPtr += ((h->arraySize)*sizeof(uintT));

	/* ラベル配列設定 */
	h->label = (unaCharT*)imgPtr;
	imgPtr += ((h->arraySize)*sizeof(unaCharT));

	/* 変換後文字列インデックス設定 */
	h->repStrIdx = (uintT*)imgPtr;
	imgPtr += ((h->repRuleNum)*sizeof(uintT));

	/* 変換後文字列プール設定 */
	h->repStr = (char*)imgPtr;

	/* 初期化済みのcheckNumberをセット */
	h->checkNumber = CHECK_NUMBER;

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	unaStd_term
//
// ABSTRACT:	UNAの文字変換モジュールの終了処理
//
// FUNCTION:
//	UNAの文字変換モジュールの終了処理を行う。
//
// RETURN:
//	UNA_OK	正常終了
//	その他	その他のエラー
//
// NOTE:
//	なし
//
int unaStd_term(
	unaStdHandleT *h	/* 文字変換モジュール用ハンドラ */
)
{
    /* 値のリセット */
	h->repRuleNum = 0;
	h->arraySize  = 0;
	h->stopCharTbl = (const char*)NULL;
	h->base = (uintT*)NULL;
	h->label = (unaCharT*)NULL;
	h->repStrIdx = (uintT*)NULL;
	h->repStr = (char*)NULL;
	h->checkNumber = 0;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	unaStd_status
//
// ABSTRACT:	UNAの文字変換モジュールの状態取得
//
// FUNCTION:
//	NULLによる初期化がされているか、初期化がされていなけてば
//	機能していない旨を、そうでなければ機能している旨を返す
//
// RETURN:
//	UNA_TRUE	機能している
//	UNA_FALSE	機能していない
//
// NOTE:
//	初期化されていない場合は、マジックナンバーによるチェックなので
//	稀に誤る可能性がある
//
int unaStd_status(
	unaStdHandleT *h	/* 文字変換モジュール用ハンドラ */
)
{
	/* セットしてある(はずの)番号の整合性を調べる */
	if ( (h->checkNumber) != CHECK_NUMBER){
		return UNA_FALSE;
	}
	
	return UNA_TRUE;

}

//--------------------------------------------------------------------------
// MODULE:	unaStd_cnv
//
// ABSTRACT:	文字列の置き換え処理
//
// FUNCTION:
//	入力文字列をハンドラ内部に格納した文字置き換えテーブルデータを
//	用いて置きかえる。置き換え後文字列および入力文字列との対応テーブルを返す。
//	対応テーブルには、置き換え後文字位置をキーにして、入力文字列中の位置が
//	格納されている。
//
// RETURN:
//	正値	置き換え後文字列長
//	その他	その他のエラー
//
// NOTE:
//	処理した入力テキスト長を知りたいときには、置き換え後の文字列長を
//	キーにして対応テーブルを引けば良い
//
int unaStd_cnv(
	unaStdHandleT *sh,	/* 文字変換モジュール用ハンドラ */
	unaCharT *stdText,	/* 置き換え後文字列の格納用バッファ */
	int *textIndex,		/* 置き換え後文字列と入力文字列の対応テーブル */
	int stdTextMax,		/* 格納用バッファの最大格納数 */
	const unaCharT *inText,	/* 入力文字列を格納したバッファ */
	int inTextLen		/* 入力文字列長 */
)
{
	int tPos;	/* 入力文字列中の位置取り扱い用 */
	int i;		/* 入力文字列中の位置取り扱い用 */
	int j;		/* 変換後文字列中の位置取り扱い用 */
	int k;		/* 変換後文字列(テーブル)中の位置取り扱い用 */
	int tMax;	/* 入力文字列の処理長さ */
	int st;		/* 現在の状態 */
	int rId;	/* 変換ルールのID */
	int ePos;	/* 変換前文字列の末尾位置 */
	int sLn;	/* 変換後文字列の適用長さ */
	const unaCharT *sStr;	/* 変換後文字列の存在位置を示すポインタ */
	int lStdTextMax;	/* 内部的な格納バッファ最大値 */

	/* データ初期化 */
	j = 0;
	lStdTextMax = stdTextMax -1; /* あらかじめターミネータ用確保 */
	tMax = inTextLen;
	if ( inTextLen<0){
		tMax = UNA_LOCAL_TEXT_SIZE;
	}

	/* ここであらかじめ解析すべき長さを設定しておく*/
	for ( i = 0; i < tMax; ++i){
		if ( inText[i] == 0 ){
			i++;
			break;	/* 変換終了文字の直後までが解析長さ */
		}
	}
	tMax = i;

	/* 変換処理メイン */
	for ( tPos=0; tPos< tMax; ){ /* 入力文字すべてについて */
		
		/* 遷移のための初期化 */
		st = 0;
		rId = -1;
		i = tPos;

		/* いけるとこまで遷移！ */
		while ( (sh->label)[st+inText[i]] == inText[i] &&
				((sh->base)[st+inText[i]] &0x80000000)==0){ /* 注1 */

			/* あえてここで判断する */
			if ( inText[i] == 0){
				break;
			}
			st = sh->base[st+inText[i]];  /* 遷移する */

			/* 最長一致の候補を常に残す */
			if (sh->label[st] == EOK){   /* ベースのラベルそのものの値がEOK */
				rId  = sh->base[st];
				ePos = i;
			}
			i++;
		}

		if ( rId >= 0){
			sLn  = (*(int*)((sh->repStr)+(sh->repStrIdx)[rId]))
						/ sizeof(unaCharT); /* 先頭4バイト=長さ */
			sStr = (unaCharT*)((sh->repStr) + (sh->repStrIdx)[rId] 
						+ sizeof(int));
			/* 格納サイズチェック */
			if ( j+sLn >= lStdTextMax){       /* 格納可能サイズを越えたら中止 */
				break;           
			}

			/* 結果を適用 */
			for ( k=0; k<sLn; ++k){
				stdText[j] = sStr[k];
				textIndex[j] = tPos;
				j++;
			}

			/* 入力テキスト中の位置を更新 */
			tPos = ePos+1;
		}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		else if (inText[tPos] >= 0xd800 && inText[tPos] <= 0xdbff &&
				inText[tPos+1] >= 0xdc00 && inText[tPos+1] <= 0xdfff){
			/* サロゲートペアは2文字まとめて置き換え後文字列に格納する */

			/* 格納サイズチェック */
			if ( j+2 >= lStdTextMax){       /* 格納可能サイズを越えたら中止 */
				break;           
			}

			/* 結果を適用 */
			stdText[j] = inText[tPos];
			stdText[j+1] = inText[tPos+1];
			textIndex[j] = tPos;
			textIndex[j+1] = tPos+1;
			j += 2;

			/* 入力テキスト中の位置を更新 */
			tPos += 2;
		}
#endif
		else{
			/* 格納サイズチェック */
			if ( j+1 >= lStdTextMax){       /* 格納可能サイズを越えたら中止 */
				break;           
			}

			/* 結果を適用 */
			stdText[j] = inText[tPos];
			textIndex[j] = tPos;
			j++;

			/* 入力テキスト中の位置を更新 */
			tPos++;
		}
	}

	/* 置き換え後文字列長を返す */
	if ( j>0 && stdText[j-1] == 0){
		textIndex[j-1] = tPos-1; /* 0 格納は inText[tPos] = 0 を示す */
		return (j-1);
	}
	else{
		stdText[j] = 0;
		textIndex[j] = tPos;
		return j; 
    }
}

//--------------------------------------------------------------------------
// MODULE:	unaStd_check
//
// ABSTRACT:	処理対象の文字が入力文字列中に存在するかどうかの判断
//
// FUNCTION:
//		入力文字列の指定長さ分のデータに対して、チェック用テーブルを参照する。
//		参照結果が対象文字であったら、UNA_TRUEを返す
//
// RETURN:
//	UNA_TRUE 	はい！あります
//	UNA_FALSE	ないです
//
// NOTE:
//	正直にすべての文字列について変換すると時間がかかるので
//	この処理を設けた
//
int unaStd_check(
	unaStdHandleT *sh,		/* 文字変換モジュール用ハンドラ */
	const unaCharT *inText,	/* 入力文字列を格納したバッファ */
	int inTextLen			/* 入力文字列長 */
)
{
	int tMax;	/* チェック文字列の最大長 */
	int i;		/* カウンタ */
	
	tMax = inTextLen;
	if ( tMax >= UNA_LOCAL_TEXT_SIZE){
		tMax = UNA_LOCAL_TEXT_SIZE;
	}
	
	
	/* ここで調査 */
	for ( i = 0; i < tMax; ++i){
		if ( (sh->stopCharTbl)[ inText[i] ] ){	/* 処理対象文字なら */
			if ( inText[i] == 0){ /* ターミネータ */
				return UNA_FALSE;
			}
			return UNA_TRUE;
		}
	}
	
	return UNA_FALSE;
}

//--------------------------------------------------------------------------
// Copyright (c) 2001-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
