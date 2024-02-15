//
// unakapi.cpp -
//      UNA V3の形態素解析用API
//      コンパクト言語解析系(UNA)V3の、検索ライブラリでの使用を
//      前提とした、形態素解析及び係り受け解析用API。
// 
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
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

#include <stdio.h>				/* FILENAME_MAX,FILE,fopen,fread,fclose */
#include <string.h>				/* strcpy,strlen,strcat,strcmp */
#include <malloc.h>				/* malloc,free */
#include <assert.h>				/* デバッグ用 */
#include "ModNlpUnaJp/unakapi.h"	/* 本モジュール用ヘッダ */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(OS_RHLINUX6_0)
#include <sys/uio.h>
#include <unistd.h>
#else
#include <io.h>
#include <stdlib.h>
#endif

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------

#define MODULE_NAME "UNAKAPI"

/* モジュール内部のメッセージ */
#define FILE_NAME_IS_NULL	"File name is null"
#define TOO_LONG_FILENAME	"Too long file name(%s)"
#define CANT_GET_FILEINFO	"Can't get file info(%s)"
#define CANT_OPEN_FILE		"Can't open file(%s)"
#define ERR_MALLOC			"Failed malloc function"
#define ERR_DIC_REC_CNT		"Diff MorDic rec count and AppDic rec count"
#define ERR_OVERFLOWAPBUF	"Aprication infomation buffer overflow"
#define ERR_HINCONV			"Out of the max number of HINSHI of CNCT_TBL"
#define CANT_GET_TOKENTYP	"Can't Token Type because of Not English Token"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

/* dummy appInfo(コンパイラの制約上、型変換して用いる) */
static const char dInf[8] = {0,0,0,0,0,0,0,0};

/* モジュール内部で使用する関数のプロトタイプ宣言 */
static int DmyPrioMorph(unaMorphHandleT *mh,int st,int *ed,void *arg);
static int DmyPrioMorph(unaMorphHandleT *mh,int st,int *ed,void *arg);
static int DmyAppInfo_get(const unaMorphT *morph,unaAppInfoT **appInf,
		void *arg);
static int DmyGetSubMorph(const unaMorphT *morph,unaMorphT *morphBuf,
		int *morphNum,const int morphBufSize,void *arg);



/* エラー番号→メッセージへの変換文字列 */
static struct {
  int NO;
  const char* message;
} errMessage[]={
/* 関数のモジュールごとの返り値の定義(UNAMORPH) */
{UNA_ERR_VERSION_CON      ,"unaMorph: connect.tbl - version error"},
{UNA_ERR_BRNCH_SIZE       ,"unaMorph: lattice buffer - overflow error"},
{UNA_ERR_PATH             ,"unaMorph: morpho buffer - overflow error"},

/* 関数のモジュールごとの返り値の定義(UNAMDTRI) */
{UNA_SUB_MORPH_BUF_OVER   ,"unaMdTri: submorph buffer - overflow error"},
{UNA_ERR_VERSION_MORPH    ,"unaMdTri: unawrd.dic - version error"},

/* 関数のモジュールごとの返り値の定義(UNAMDUNK) */
{UNA_ERR_VERSION_UMK      ,"unaMdUnk: unkmk.tbl - version error"},
{UNA_ERR_VERSION_UC       ,"unaMdUnk: unkcost.tbl - version error"},

/* 関数のモジュールごとの返り値の定義(UNAAPINF) */
{UNA_NO_APP_DIC           ,"unaApInf: unaapp.dic - no appdic error"},
{UNA_ERR_VERSION_APP      ,"unaApInf: unaapp.dic - version error"},

/* 関数のモジュールごとの返り値の定義(UNABNS) */
{UNA_ERR_VERSION_GRM      ,"unaBns: gram.tbl - version error"},
{UNA_ERR_MORHINNUM_BNS    ,"unaBns: hinshi number - range error"},
{UNA_ERR_OVERFLOW_BNS     ,"unaBns: bns buffer - overflow error"},

/* 関数のモジュールごとの返り値の定義(UNAMDUSR) */
{UNA_ERR_INIT             ,"unaMdUsr: initialize error"},
{UNA_ERR_TOO_LONG_INFO    ,"unaMdUsr: appinfo buffer - overflow error1"},
{UNA_ERR_OVERFLOW_INFO    ,"undMdUsr: appinfo buffer - overflow error2"},
{UNA_ERR_OVERFLOW_MORPH   ,"undMdUsr: wordinfo buffer - overflow error"},
{UNA_ERR_WRONG_ID         ,"undMdUsr word id - invalid word id error"},
{UNA_ERR_WRONG_POS        ,"undMdUsr: word pos - invalid word pos error"},
{UNA_ERR_FORMAT_COST      ,"undMdUsr: word cost - invalid cost error"},
{UNA_ERR_FORMAT_HYOKI     ,"undMdUsr: word hyouki - invalid hyouki error"},
{UNA_ERR_OVERFLOW_TXTBUF  ,"undMdUsr: dic format - invalid format error\n"},

/* 関数のモジュールごとの返り値の定義(UNAMDENG) */
{UNA_ERR_VERSION_EMK      ,"unaMdEng: engmk.tbl - version error"},
{UNA_ENG_SUB_MOR_BUF_OVR  ,"unaMdEng: submorph buffer - overflow error"},

/* 関数のモジュールごとの返り値の定義(UNAWDGEN) */
{UNA_ERR_VERSION_WDGEN    ,"unaWdGen: idgram.tbl - version error"},
{UNA_ERR_WDGEN_ACPT       ,"unaWdGen: wdgen rule - too complex"},
{UNA_ERR_WDGEN_IDSET      ,"unaWdGen: wdgen rule - too complex 2"},
{UNA_ERR_WDGEN_STATE      ,"unaWdGen: wdgen rule - too complex 3"},
{UNA_ERR_WDGEN_BRNCH      ,"unaWdGen: morph string - too complex"},
{UNA_ERR_WDGEN_SUB        ,"unaWdGen: submorph buffer - overflow error"},

/* 関数のモジュールごとの返り値の定義(UNASTD) */
{UNA_ERR_VERSION_STD      ,"unaStd: unastd.tbl - version error"},
{UNA_ERR_UNSUPPORTED_CHAR ,"unaStd: code - unsupported unicode char"},

/* 関数のモジュールごとの返り値の定義(UNAKAPI) */
{UNA_FILE_NAME_IS_NULL    ,"unaKApi: file name - null error"},
{UNA_TOO_LONG_FILENAME    ,"unaKApi: file name - too long error"},
{UNA_CANT_GET_FILEINFO    ,"unaKApi: file - can't get file info"},
{UNA_CANT_OPEN_FILE       ,"unaKApi: file - open error"},
{UNA_ERR_MALLOC           ,"unaKApi: malloc - allocate error"},
{UNA_ERR_DIC_REC_CNT      ,"unaKApi: dictionaries - word dictionary and application dictionary unmacth"},

{UNA_ERR_OVERFLOWAPBUF    ,"unaKApi: application buffer - overflow error"},
{UNA_ERR_HINCONV          ,"unaKApi: hinshi number - range error"},
{UNA_CANT_GET_TOKENTYP    ,"unaKApi: word info - tokentype get error"},

{0,"unknown module: unknown error"}
};

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getErrMsg
//
// ABSTRACT:	  エラーメッセージの取得
//
// FUNCTION:
//	  エラーメッセージを取得する
//
// RETURN:
//	  エラーメッセージへのポインタ
//
// NOTE:
//
const char *unaKApi_getErrMsg(
	const int	errNo)			/* エラー番号 */
{
  int i;

  for ( i = 0; errMessage[i].NO!=0; ++i){
	if ( errMessage[i].NO == errNo){
	  return errMessage[i].message;
    }
  }
  return errMessage[i].message; // デフォルトのメッセージ
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_readFileImg
//
// ABSTRACT:	  辞書のメモリへの読み込み
//
// FUNCTION:
//	  ファイルサイズにあったメモリを確保し、そのメモリにファイルを
//	  読み込んでマップする
//
// RETURN:
//		UNA_OK					正常終了
//		UNA_FILE_NAME_IS_NULL	ファイル名がNULLである
//		UNA_TOO_LONG_FILE_NAME	ファイル名が長すぎる
//		UNA_CANT_GET_FILEINFO	ファイルの情報が得られない
//		UNA_CANT_OPEN_FILE		ファイルがオープンできない
//		UNA_ERR_MALLOC			メモリが確保できなかった
//
// NOTE:
//	  ファイルサイズ分のメモリが確保される。
//	  ファイル名がNULLの場合、mapAddressにもNULLがセットされ返る
//
int unaKApi_readFileImg(
	const void *fileNameV,	/* 読み込んで欲しいファイル名(ucs2) */
	char **mapAddress	  		/* マップしたアドレス */
	)
{
	int fd;
	struct stat stbuf;						/* ファイルの情報 */
	const char* fileName;
	fileName = (const char*)fileNameV;

	*mapAddress = (char *)NULL;  /* NULLはデフォルトのエラー時戻りアドレス */

	/* 入力引数チェック */
	if (fileName[0] == 0) { /* ファイル名がNULLの時 */
		return UNA_FILE_NAME_IS_NULL;
	}

	/* ファイル名の長さのチェック */
	if (strlen(fileName) > UNA_FNAME_MAX) {
		UNA_RETURN(TOO_LONG_FILENAME,fileName);
	}
	/* 対象とするファイルのサイズをチェック */
	if (stat(fileName, &stbuf) == -1) { /* ファイルの情報が得られない */
		UNA_RETURN(CANT_GET_FILEINFO,fileName);
	}
	/* ファイルをオープンする */
	if ((fd = open(fileName, O_RDONLY)) <0 ) {
		UNA_RETURN(CANT_OPEN_FILE,fileName);
	}
	/* 領域をアロケートする */
	*mapAddress = (char *)malloc(stbuf.st_size);
	if (*mapAddress == (char *)NULL) {
		UNA_RETURN(ERR_MALLOC,NULL);
				/* システム関係のエラーにつきテストせず(カバレッジ対象外) */
	}
	/* ファイルから領域に書き込む */
	read(fd, *mapAddress, stbuf.st_size);
	close(fd);

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_freeImg
//
// ABSTRACT:   メモリの開放
//
// FUNCTION:
//	  unaKApi_readFileImgで確保したメモリ領域が開放される
//
// RETURN:
//	  UNA_OK				正常終了
//	  UNA_FILE_NAME_IS_NULL	ファイル名がヌルである
//
// NOTE:
//
int unaKApi_freeImg(
	const char *memAddress	/* 開放したいメモリ領域 */
	)
{
	if (memAddress == (const char *)NULL) {
		/* アドレスがヌル即ちファイルが指定されてなかった時 */
		return UNA_FILE_NAME_IS_NULL;
	}

	free ((void *)memAddress);

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  UNAkApi_init
//
// ABSTRACT:	  UNAの検索用APIの初期化
//
// FUNCTION:
//	  UNAの検索用のAPIの初期化を行う。
//	  スレッドごとに必要。
//
// RETURN:
//	  UNA_OK				正常終了
//	  UNA_ERR_DIC_REC_CNT	形態素辞書とアプリ辞書が不整合(件数不一致)
//	  その他				その他のエラー
//
// NOTE:
//	  英語トークン用文字種別テーブルが指定されているかいないかにより
//	  辞書引き用関数の設定が異なる。
//	  ・指定されている時、以下の順に辞書引きを行う
//			(1)大語彙形態素辞書引きモジュール
//			(2)英語トークン検出モジュール
//			(3)未登録語検出モジュール
//	  ・指定されていない時、以下の順に辞書引きを行う
//			(1)大語彙形態素辞書引きモジュール
//			(2)未登録語検出モジュール
//
int unaKApi_init(
	unaKApiHandleT *h,			 /* ハンドラ */
	const unaKApiDicImgT *dicImgList, /* 辞書イメージ配列 */
	int dicImgCount,			 /* 辞書イメージ要素数 */
	const char *cnctTblImg,		 /* 接続表のイメージ */
	const char *gramTblImg,		 /* 文法テーブルのイメージ */
	const char *eMKTblImg,		 /* 英語トークン用文字種別テーブル */
	const char *uMKTblImg,		 /* 未登録語用文字種別テーブル */
	const char *uCTblImg,		 /* 未登録語コスト推定テーブル */
	const char *repTblImg,		 /* 文字列標準化テーブル */
	unsigned int maxWordLen 	 /* 単語の最大長指定(0なら未指定) */
)
{
	int rv;		/* UNA関数の返値 */
	int i;			/* 辞書番号 */


	/* 入力文字列標準化モジュールの初期化(NULLでも可能) */
	rv = unaStd_init(&(h->sh),repTblImg);
	if (rv < 0) {
		return rv;
	}
	/* 形態素解析メインモジュールの初期化 */
	rv = unaMorph_init(&(h->mh),cnctTblImg,maxWordLen);
	if (rv < 0) {
		return rv;
	}

	/* 形態素辞書引きモジュール, アプリ情報取得モジュールの初期化 */
	if (dicImgList == NULL) {
		// 空辞書の場合
		h->known_dic_count = 1;
		rv = unaMdicTrie_init(&(h->th[0]), NULL, NULL, UNAMORPH_DEFAULT_PRIO);
		if (rv < 0) {
			return rv;
		}
		rv = unaAppInfo_init(&(h->ah[0]), NULL);
		if (rv < 0) {
			return rv;
		}
	} else {
		h->known_dic_count = dicImgCount;
		for (i = 0; i < h->known_dic_count; i++) {
			const unaKApiDicImgT* p = &dicImgList[i];
			rv = unaMdicTrie_init(&(h->th[i]), p->morphDic, p->dicName, p->dicPrio);
			if (rv < 0) {
				return rv;
			}
			rv = unaAppInfo_init(&(h->ah[i]), p->appInfo);
			if (rv < 0) {
				return rv;
			}
		}
	}

	/* 英語トークン検出モジュールの初期化 */
	if (eMKTblImg != (const char *)NULL) { /* 文字種別テーブル指定あり */
		rv = unaMDicEnglish_init(&(h->eh),eMKTblImg);
		if (rv < 0) {
			return rv;
		}
	}

	/* 未登録語検出モジュールの初期化 */
	if (dicImgList == NULL) { /* 非日本語 */
		rv = unaMdicUnknown_init(&(h->uh),uMKTblImg,uCTblImg,0);
	}
	else{ /* 日本語 */
		rv = unaMdicUnknown_init(&(h->uh),uMKTblImg,uCTblImg,1);
	}
	if (rv < 0) {
		return rv;
	}

	/* 形態素辞書、アプリ辞書件数チェック */
	for (int n = 0; n < dicImgCount; n++) {
		if (h->th[n].recCount != h->ah[n].recCount) {
			UNA_RETURN(ERR_DIC_REC_CNT,NULL);
		}
	}

	/* かかりうけモジュールの初期化 */
	rv = unaBns_init(&(h->bh),gramTblImg);
	if (rv < 0) {
		return rv;
	}

	/* 呼び出し関数の設定 */
	/*
	 * 登録語
	 */
	for (i = 0; i < h->known_dic_count; i++) {
		h->known_dic_num[i] = i;
		/* 形態素検出 */
		h->morFuncMtx[i].func	= (unaFuncT)unaMdicTrie_searchMorph;
		h->morFuncMtx[i].func2	= (unaFuncT)unaMdicTrie_prioMorph;
		h->morFuncMtx[i].arg	= &(h->th[i]); /* 形態素辞書引き用ハンドル */
		/* アプリケーション情報取得 */
		if ( (h->ah[i]).recCount != 0){
		  h->appFuncMtx[i].func	= (unaFuncT)unaAppInfo_get;
		}
		else{
		  h->appFuncMtx[i].func	= (unaFuncT)DmyAppInfo_get;
		}
		h->appFuncMtx[i].func2	= (unaFuncT)NULL;
		h->appFuncMtx[i].arg	= &(h->ah[i]);	/* アプリ情報用ハンドル */
		/* 下位構造取得 */
		h->subFuncMtx[i].func	= (unaFuncT)unaMdicTrie_getSubMorph;
		h->subFuncMtx[i].func2	= (unaFuncT)NULL;
		h->subFuncMtx[i].arg	= &(h->th[i]); /* 形態素辞書引き用ハンドル */
	}
	i = h->known_dic_count - 1;
	if (eMKTblImg != (const char *)NULL) { /* 文字種テーブル指定あり */
		/*
		 * 英語トークン
		 */
		h->eng_dic_num = ++i;
		/* 形態素検出 */
		h->morFuncMtx[i].func	= (unaFuncT)unaMDicEnglish_searchMorph;
		h->morFuncMtx[i].func2	= (unaFuncT)DmyPrioMorph;
		h->morFuncMtx[i].arg	= &(h->eh); /* 英語トークン検出用ハンドル */
		/* アプリケーション情報取得 */
		h->appFuncMtx[i].func	= (unaFuncT)unaMDicEnglish_appInfoGet;
		h->appFuncMtx[i].func2	= (unaFuncT)NULL;
		h->appFuncMtx[i].arg	= &(h->eh); /* 英語トークン検出用ハンドル */
		/* 下位構造取得 */
		h->subFuncMtx[i].func	= (unaFuncT)unaMDicEnglish_getSubMorph;
		h->subFuncMtx[i].func2	= (unaFuncT)NULL;
		h->subFuncMtx[i].arg	= &(h->eh); /* 英語トークン検出用ハンドル */
	}
	else {
		h->eng_dic_num = -1;
	}
	/*
	 * 未登録語
	 */
	h->unknown_dic_num = ++i;
	/* 形態素検出 */
	h->morFuncMtx[i].func	= (unaFuncT)unaMdicUnknown_searchMorph;
	h->morFuncMtx[i].func2	= (unaFuncT)DmyPrioMorph;
	h->morFuncMtx[i].arg	= &(h->uh); /* 未登録語検出用ハンドル */
	/* アプリケーション情報取得 */
	h->appFuncMtx[i].func	= (unaFuncT)DmyAppInfo_get;
	h->appFuncMtx[i].func2	= (unaFuncT)NULL;
	h->appFuncMtx[i].arg	= NULL;		/* 引数なし */
	/* 下位構造取得 */
	h->subFuncMtx[i].func	= (unaFuncT)DmyGetSubMorph;
	h->subFuncMtx[i].func2	= (unaFuncT)NULL;
	h->subFuncMtx[i].arg	= NULL;		/* 引数なし */

	/*
	 * ポインタなのでヌルを入れてターミネート
	 */
	h->morFuncMtx[++i].func	= (unaFuncT)NULL;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_term
//
// ABSTRACT:	  UNA検索用API終了
//
// FUNCTION:
//	  UNAの検索用APIを終了する。
//
// RETURN:
//	  UNA_OK			正常終了
//
// NOTE:
//	  ハンドラ中の形態素辞書検索関数クロージャのテーブルがクリアされる。
//	  以後、当ハンドラによる辞書引きはできなくなる。
//
int unaKApi_term(
	unaKApiHandleT *h			/* ハンドラ */
	)
{
	int rv;					/* 関数の返値 */
	
	/* 各モジュールの終了処理を行う */
	for (int i = 0; i < h->known_dic_count; i++) {
		rv = unaMdicTrie_term(&(h->th[i]));
		assert(rv == UNA_OK);
		rv = unaAppInfo_term(&(h->ah[i]));
		assert(rv == UNA_OK);
	}
	rv = unaBns_term(&(h->bh));
	assert(rv == UNA_OK);
	rv = unaMDicEnglish_term(&(h->eh));
	assert(rv == UNA_OK);
	rv = unaMdicUnknown_term(&(h->uh));
	assert(rv == UNA_OK);
	rv = unaMorph_term(&(h->mh));
	assert(rv == UNA_OK);
	rv = unaStd_term(&(h->sh));
	assert(rv == UNA_OK);
	
	/* ポインタなのでヌルを入れてターミネート */
	h->morFuncMtx[0].func = (unaFuncT)NULL;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_moAna
//
// ABSTRACT:	  形態素解析
//
// FUNCTION:
//	  入力テキストを形態素解析する。
//	  以下のどれかの条件が達成されると、そこまでの形態素解析結果を返す。
//	  ・入力テキストが終了する
//	  ・解析された入力テキストの長さがUNA_LOCAL_TEXT_SIZE(255文字)近くに
//	    達する。なお、UNA_LOCAL_TEXT_SIZEをまたぐ単語があるとそこで切られる
//	  ・ラティスが収束する(句読点などの区切り文字が見つかるという条件も含む)
//	  ・中断関数の返り値が 0 以外になる
//	  なお、バッファの長さはUNA_LOCAL_TEXT_SIZE + 1 あればあふれることは無い
//	  ここで、+ 1 は、強制収束時の仮想文節頭末の分である。
//	  また、textLenに負の数が指定されている場合には、ヌルターミネータまでの
//	  文字数が使われる。
//	  小さい形態素バッファを指定したなどの理由で、バッファがオーバーフロー
//	  した場合には、エラーを返す。
//
// RETURN:
//	  UNA_OK			正常終了
//	  UNA_STOP			中断(下位関数の戻り値)
//	  その他			エラー
//
// NOTE:
//
int unaKApi_moAna(
	unaKApiHandleT *h,			/* ハンドラ */
	const unaCharT *inTxtPtr,	/* 入力テキスト(UCS-2) */
	const int inTxtLen,		/* 入力テキストの長さ */
	unaMorphT *morphBuf,		/* 形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた形態素の数 */
	const int morphBufSize,	/* バッファの大きさ(要素数) */
	int *processedTxtLen,		/* 形態素解析されたテキストの長さ(文字数) */
	unaStopFuncT stopFunc,		/* 中断関数 */
	const int execStd,			/* 文字列標準化を実施するか否かのスイッチ */
	const int emulateBug,		/* max-word-bug emulate switch */
	const int ignoreCR			/* 改行をまたぐ単語を検出するか否かのスイッチ */
)
{
	int repTextLen; 			/* 置き換え後のテキスト長さ */
	int tMorNum;				/* 形態素列の生成数(一時的) */
	int ttMorNum;				/* 形態素列の生成数(一時的) */
	int pStdTxtLen;			/* 置き換え文字列の処理済み長さ */
	int tpStdTxtLen;			/* 置き換え文字列の処理済み長さ */
	int headHin;				/* 解析時の頭品詞 */
	int rv;					/* 戻り値用 */
	
	/* まず頭品詞を取得しておく */
	headHin = unaMorph_getSentenceTail( &(h->mh));

	/* 改行の扱いを設定 */
	for (int i = 0; i < h->known_dic_count; i++) {
		unaMdicTrie_setCR(&(h->th[i]),ignoreCR);
	}

	/* set bug emulate switch */
	unaMdicUnknown_setEmulateBug(&(h->uh),emulateBug);
	
	/* 文字列から形態素列を生成 */
	tMorNum = unaMorph_gen(&(h->mh),morphBuf,morphBufSize,
			(unaCharT*)inTxtPtr,inTxtLen,&(h->morFuncMtx[0]),&pStdTxtLen,
			stopFunc);
	if (tMorNum < 0) {	/* エラー */
		return tMorNum;
	}

	/* 文字変換モジュールの動作によって分岐 */
	if ( execStd != UNA_TRUE || unaStd_status( &(h->sh))!=UNA_TRUE ||
		 unaStd_check( &(h->sh), inTxtPtr, 
		   (pStdTxtLen+8))!=UNA_TRUE ){ /* 解析範囲後８文字まで見る */
		*morphNum = tMorNum; 
		*processedTxtLen = pStdTxtLen;
		h->stdUnaText[0] = 0;
	}
	else{	/* 文字変換モジュールが動作し、かつ処理対象文字が存在 */
		
		/* 再解析のために頭品詞を再セット */
		rv = unaMorph_setSentenceTail( &(h->mh), headHin);
		assert( rv == headHin);
		
		/* 入力文字列を標準化 */
		repTextLen = unaStd_cnv(&(h->sh), h->stdUnaText, h->textIndex, 
			UNA_LOCAL_TEXT_SIZE+1, inTxtPtr, inTxtLen);
		if (repTextLen < 0){ /* エラー */
			return repTextLen;
		}
		
		/* 文字列から形態素列を生成 */
		tMorNum = 0;
		pStdTxtLen = 0;
		while (pStdTxtLen<repTextLen){
			/* 頭品詞の取得 */
			headHin = unaMorph_getSentenceTail( &(h->mh));

			/* 形態素解析 */
			ttMorNum = unaMorph_gen(&(h->mh),
				(morphBuf+tMorNum),(morphBufSize-tMorNum),
				((h->stdUnaText)+pStdTxtLen),(repTextLen-pStdTxtLen),
				&(h->morFuncMtx[0]),&tpStdTxtLen, stopFunc);

			/* 終了チェック */
			if (ttMorNum < 0) {	/* エラー */
				if ( ttMorNum == UNA_ERR_PATH){ /* 形態素数が多すぎ */
					rv = unaMorph_setSentenceTail( &(h->mh), headHin);
					break;
				}
				else{ 
					return ttMorNum;
				}
			}
			else if (pStdTxtLen+tpStdTxtLen>=repTextLen&&pStdTxtLen>0){
				// ここまでとする(末尾部分の結果は捨てる)
				rv = unaMorph_setSentenceTail( &(h->mh), headHin);
				break;
			}

			/* 解析位置更新 */
			tMorNum += ttMorNum;
			pStdTxtLen += tpStdTxtLen;
		}

		*morphNum = tMorNum; 
		*processedTxtLen = (h->textIndex)[pStdTxtLen];
	}
		
	/* 文末を明示的に指定 */
	if ( *processedTxtLen == inTxtLen || inTxtPtr[(*processedTxtLen)]==0){
		unaMorph_termSentence( &(h->mh));
	}
	
	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_kuAna
//
// ABSTRACT:	  係り受け解析
//
// FUNCTION:
//	  入力テキストを係り受け解析する。
//	  以下のどれかの条件が達成されると、そこまでの係り受け解析結果を返す。
//	  ・入力テキストが終了した時。
//	  ・形態素バッファがオーバーフローを起しそうになった時
//	  ・文節の数がUNA_LOCAL_BNS_SIZE(デフォルト128)になった時。
//	  ・句点が見つかったとき。
//	  ・中断関数の返り値が 0 以外になる
//	  よって、バッファの長さはUNA_LOCAL_BNS_SIZEより大きくても意味が無い。
//
// RETURN:
//	  UNA_OK			正常終了
//	  UNA_STOP			中断(下位関数の戻り値)
//	  その他			エラー
//
// NOTE:
//	  (注1)
//		inTxtLenが負の値として指定されて来た場合でも、unaMorph_genの引数の
//		inTxtLen - *processedTxtLen は、ずっと負になるので問題はない。
int unaKApi_kuAna(
	unaKApiHandleT *h,			/* ハンドラ */
	const unaCharT *inTxtPtr,	/* 入力テキスト(UCS-2) */
	const int inTxtLen,		/* 入力テキストの長さ */
	unaMorphT *morphBuf,		/* 形態素解析結果が書かれるバッファ */
	int *morphNum,				/* 書かれた形態素の数 */
	int morphBufSize,			/* 形態素バッファの大きさ(要素数) */
	unaBnsT *bnsBuf,			/* かかりうけ解析結果が書かれるバッファ */
	int *bnsNum,				/* 書かれた文節の数 */
	const int bnsBufSize,		/* 文節バッファの大きさ(要素数) */
	int *processedTxtLen,		/* 形態素解析されたテキストの長さ(文字数) */
	unaStopFuncT stopFunc,		/* 中断関数 */
	const int emulateBug,			/* switch for max-word-bug emulation */
	const int ignoreCR			/* 改行記号を無視するか否か */
)
{
	int rv;					/* 関数の返り値 */
  	int tmpProcessedTxtLen;	/* 形態素解析されたテキストの長さ(tmp) */
  	int processedMorphNum;		/* 処理した形態素数 */
  	int nextStartPos;			/* 文節生成結果格納の開始位置 */
  	int tmpMorphNum;			/* 書かれた形態素(tmp) */
	int *grm_work1;	/* かかりうけ用ワークバッファ1 */
	ucharT *grm_work2;	/* かかりうけ用ワークバッファ2 */
	int *grm_work3;	/* かかりうけ用ワークバッファ3 */

  	/* 初期化 */
  	nextStartPos = 0;
	*processedTxtLen = 0;
	*morphNum = 0;
	*bnsNum = 0;
	grm_work1 = (int*)malloc(UNA_LOCAL_BNS_SIZE*
			UNA_LOCAL_BNS_SIZE*sizeof(int));
	if ( !grm_work1 ){
		return UNA_ERR_MALLOC;
	}
	grm_work2 = (ucharT*)malloc(UNA_LOCAL_BNS_SIZE*
			UNA_LOCAL_BNS_SIZE*sizeof(ucharT));
	if ( !grm_work2 ){
		free(grm_work1);
		return UNA_ERR_MALLOC;
	}
	grm_work3 = (int*)malloc(UNA_LOCAL_BNS_SIZE*sizeof(int));
	if ( !grm_work3 ){
		free(grm_work1);
		free(grm_work2);
		return UNA_ERR_MALLOC;
	}
  	/* 改行の扱いを設定 */
	for (int i = 0; i < h->known_dic_count; i++) {
		unaMdicTrie_setCR(&(h->th[i]),ignoreCR);
	}

	/* set bug emulate switch */
	unaMdicUnknown_setEmulateBug(&(h->uh),emulateBug);

  	for(;;){		/* forever */
		/* 入力テキストは一度に全てが形態素解析されるわけではなく、
		   途中迄解析され、戻る場合があるのでこのループが必要 */

		/* テキストから形態素列を得る(注1) */
		rv = unaMorph_gen(&h->mh,morphBuf + *morphNum,
				morphBufSize - *morphNum,
				(unaCharT *)(inTxtPtr + *processedTxtLen),
				inTxtLen - *processedTxtLen,&(h->morFuncMtx[0]),
				&tmpProcessedTxtLen, stopFunc);
		if (rv < 0) {	/* エラー */
			if (rv == UNA_ERR_PATH) {	/* 形態素バッファオーバーフロー */
				break;
			}
			else {
				free( grm_work1);
				free( grm_work2);
				free( grm_work3);
				return rv;
			}
		}
		tmpMorphNum = rv;/* 生成された形態素数 */

		if (tmpMorphNum == 0) { /* テキスト終わり */
			assert(tmpProcessedTxtLen ==0);
			break;
		}

		/* 形態素列から文節列を生成 */
		rv = unaBns_gen(&h->bh,bnsBuf,nextStartPos,bnsBufSize,
					(morphBuf + *morphNum),tmpMorphNum,&processedMorphNum,
					grm_work1, grm_work2,grm_work3);
		if (rv < 0 ) {	/* エラー */
			free( grm_work1);
			free( grm_work2);
			free( grm_work3);
			return rv;
		}
		/* 全ての形態素を使うことなく文節ができた(形態素が余った) */
		if (processedMorphNum < tmpMorphNum) {
			*morphNum += processedMorphNum;
			*processedTxtLen = (int)(
				morphBuf[*morphNum - 1].start +
				morphBuf[*morphNum - 1].length -
				inTxtPtr);
			*bnsNum = rv;	/* 形態素は後ろにくっ付く。全文節数が返り値 */
			break;
		}
		/* 全ての形態素を使った場合(まだ形態素が文節に続く可能性あり) */
		*processedTxtLen += tmpProcessedTxtLen;
		*morphNum += tmpMorphNum;
		*bnsNum = rv;	/* 全文節数が返り値なので、"+=" ではない */
		nextStartPos = rv;
	}

	/* *bnsNum == 0 でも下位関数は正常に通る */
	rv = unaBns_analyze(&h->bh,bnsBuf,*bnsNum,grm_work1,grm_work2,grm_work3);
	assert (rv == UNA_OK);

	free( grm_work1);
	free( grm_work2);
	free( grm_work3);

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getOriginalHyoki
//
// ABSTRACT:	  元表記の取得
//
// FUNCTION:
//	  unaMorphT から元表記を取得する。
//
// RETURN:
//	  UNA_OK			正常終了
//
// NOTE:
//
int unaKApi_getOriginalHyoki(
	unaKApiHandleT *h,			/* ハンドラ */
	const unaCharT *origStr,	/* 元文字列 */
	const unaMorphT *morph,		/* 形態素(単語) */
	unaCharT **hyokiPtr,		/* 表記のアドレス */
	unsigned int *hyokiLen		/* 表記の長さ(文字数) */
)
{
	/* 文字列標準化が行われているか否かチェック */
	if ( h->stdUnaText[0] == 0){
		*hyokiPtr = (morph->start);
		*hyokiLen = (morph->length);
	}
	else{ /* 行われている場合は、元表記を計算して返す */
		*hyokiPtr = (unaCharT*)(origStr + 
			(h->textIndex)[((morph->start)-(h->stdUnaText))]);
		*hyokiLen = 
			(h->textIndex)[(morph->start)+(morph->length)-(h->stdUnaText)]
			-(h->textIndex)[((morph->start)-(h->stdUnaText))];
	}

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getHyoki
//
// ABSTRACT:	  表記の取得
//
// FUNCTION:
//	  unaMorphT から表記を取得する。
//
// RETURN:
//	  UNA_OK			正常終了
//
// NOTE:
//	  morphのstartとlengthを返すだけなので、この関数の情報を使用する場合には
//	  実体(テキスト)をクリアしてはならない。
//
int unaKApi_getHyoki(
	unaKApiHandleT *h,			/* ハンドラ */
	const unaMorphT *morph,		/* 形態素(単語) */
	unaCharT **hyokiPtr,		/* 表記のアドレス */
	unsigned int *hyokiLen		/* 表記の長さ(文字数) */
)
{
	int dicNum;
	int rv;
	unaAppInfoT *a;

	dicNum = morph->appI >> 24;
	if ( dicNum == h->eng_dic_num){
		rv = ((unaAppFuncT)(h->appFuncMtx[dicNum].func))
			(morph,&a,h->appFuncMtx[dicNum].arg);
		if ( rv < 0 ){
			return rv;
		}
		*hyokiPtr = (unaCharT*)(a->info);
		*hyokiLen = (a->len)/sizeof(unaCharT);
	}
	else{
		*hyokiPtr = morph->start;
		*hyokiLen = morph->length;
	}
	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getAppInfo
//
// ABSTRACT:	  アプリケーション情報の取得
//
// FUNCTION:
//	  辞書ナンバーに適したアプリケーション情報を取得する。
//
// RETURN:
//	  UNA_KNOWN_WORD	登録語である
//	  UNA_NO_APP_DIC	登録語であるがアプリ辞書の指定がない
//						(下位関数の返り値)
//	  UNA_ENG_TOKEN		英語トークンである
//	  UNA_UNKNOWN_WORD  未登録語である
//
// NOTE:
//	  (注1)
//		morphは非公開なので書き換えられる事はないが一応assert
//	  (注2)
//		英語トークンモジュール使用の場合は問題ないが、未使用の
//		場合でもこのロジックでOK。なぜなら、その時 dicNum == 0 又は 1 で
//		h->eng_dic_num == -1 だから未登録語の場合は UNA_UNKNOWN_WORD が返る
//
int unaKApi_getAppInfo(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph,		/* 形態素(単語) */
	unaAppInfoT **appInf		/* アプリケーション情報 */
)
{
	int dicNum;			/* 辞書ナンバー */
	int rv;				/* 関数の返り値 */

	dicNum = morph->appI >> 24;	/* appIの上位8ビットが辞書ナンバー */
	assert(dicNum >= 0 && dicNum <= h->known_dic_count + 2); /* 注1 */

	rv = ((unaAppFuncT)(h->appFuncMtx[dicNum].func))(morph,appInf,
											h->appFuncMtx[dicNum].arg);
	if (rv < 0) {
		return rv;	/* UNA_NO_APP_DIC のみ */
	}

	if (dicNum < h->known_dic_count) {
		return UNA_KNOWN_WORD;
	}
	if (dicNum == h->eng_dic_num) {	/* 注2 */
		return UNA_ENG_TOKEN;
	}
	else {
		return UNA_UNKNOWN_WORD;
	}
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getHinName
//
// ABSTRACT:	  形態素品詞名の取得
//
// FUNCTION:
//	  unaMorphT から品詞名を取得する。
//
// RETURN:
//	  ヌル文字			不正な品詞番号を持つ形態素
//	  それ以外			品詞名
//
// NOTE:
//
const unaCharT* unaKApi_getHinName(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph 		/* 形態素(単語) */
)
{
	/* エラーチェック */
	if (morph->hinshi >= h->mh.morpHinNumMax) { /* 配列の範囲内か */
		return (const unaCharT*)0;
	}
	
	/* 下位関数に任せる */
	return unaMorph_getHinName(&(h->mh),morph->hinshi);
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getUnaHin
//
// ABSTRACT:	  UNA品詞の取得
//
// FUNCTION:
//	  unaMorphT からUNA品詞を取得する。
//
// RETURN:
//	  UNA_OK			正常終了
//	  ERR_HINCONV		不正な形態素のため変換できなかった
//						(品詞番号がテーブルの範囲外)
//
// NOTE:
//
int unaKApi_getUnaHin(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph,		/* 形態素(単語) */
	unaHinT *unaHin				/* UNA品詞 */
)
{
	int rv;					/* 関数の返り値 */
	
	/* エラーチェック */
	if (morph->hinshi >= h->mh.morpHinNumMax) { /* 配列の範囲内か */
		UNA_RETURN(ERR_HINCONV,NULL);
	}
	
	/* 下位関数に任せる */
	rv = unaMorph_getUnaHin(&(h->mh),morph->hinshi,unaHin);
	assert(rv == UNA_OK);

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getTokenType
//
// ABSTRACT:	  トークン種別の取得
//
// FUNCTION:
//	  unaMorphT からトークン種別を取得する。
//
// RETURN:
//	  UNA_CANT_GET_TOKENTYP		トークン種別を得る事ができない
//	  それ以外					トークン種別
//
// NOTE:
//
int unaKApi_getTokenType(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph		/* 形態素(単語) */
)
{
	/* エラーチェック */
	if (h->eng_dic_num == -1
	||  (uintT)h->eng_dic_num != (morph->appI >> 24)) {
		UNA_RETURN(CANT_GET_TOKENTYP,NULL);
	}

	/* appI の下位3バイトを取り出す */
	return morph->appI & 0x00ffffff;
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getSubMorph
//
// ABSTRACT:	  下位構造の取得
//
// FUNCTION:
//	  unaMorphT の下位構造を取得する。
//
// RETURN:
//	  UNA_KNOWN_WORD	登録語である
//	  UNA_ENG_TOKEN		英語トークンである
//	  UNA_UNKNOWN_WORD  未登録語である(下位構造はない)
//	  その他			エラー
//
// NOTE:
//	  下位構造が無い場合は、バッファには何も格納されず morphNum=0 として返る
//
//	  (注1)
//		morphは非公開なので書き換えられる事はないが一応assert
//	  (注2)
//		英語トークンモジュール使用の場合は問題ないが、未使用の
//		場合でもこのロジックでOK。なぜなら、その時 dicNum == 0 又は 1 で
//		h->eng_dic_num == -1 だから未登録語の場合は UNA_UNKNOWN_WORD が返る
//
int unaKApi_getSubMorph(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize		/* バッファの大きさ(要素数) */
)
{
	int dicNum;				/* 辞書ナンバー */
	int rv;					/* 関数の返り値 */

	dicNum = morph->appI >> 24;	/* appIの上位8ビットが辞書ナンバー */
	assert(dicNum >= 0 && dicNum <= h->known_dic_count + 2); /* 注1 */
	rv = ((unaSubFuncT)(h->subFuncMtx[dicNum].func))(morph,morphBuf,
						morphNum,morphBufSize,h->subFuncMtx[dicNum].arg);
	if (rv < 0) {
		return rv;
	}

	if (dicNum < h->known_dic_count) {
		return UNA_KNOWN_WORD;
	}
	if (dicNum == h->eng_dic_num) {	/* 注2 */
		return UNA_ENG_TOKEN;
	}
	else {
		return UNA_UNKNOWN_WORD;
	}
}

//--------------------------------------------------------------------------
// MODULE:	  unaKApi_getDicName
//
// ABSTRACT:	  辞書ベース名の取得
//
// FUNCTION:
//	  unaMorphT から辞書ベース名を取得する。
//
// RETURN:
//	  登録語のとき			辞書ベース名
//	  それ以外のとき		NULL
//
const char* unaKApi_getDicName(
	const unaKApiHandleT *h,	/* ハンドラ */
	const unaMorphT *morph		/* 形態素(単語) */
)
{
	int dicNum;			/* 辞書ナンバー */
	int rv;				/* 関数の返り値 */

	dicNum = morph->appI >> 24;	/* appIの上位8ビットが辞書ナンバー */
	assert(dicNum >= 0 && dicNum <= h->known_dic_count + 2); /* 注1 */

	if (dicNum < h->known_dic_count) {
		return unaMdicTrie_getDicName((unaMdTriHandleT *)&(h->th[dicNum]));
	}
	return NULL;
}

//--------------------------------------------------------------------------
// MODULE:	  DmyPrioMorph
//
// ABSTRACT:	  優先登録処理(ダミー関数)
//
// FUNCTION:
//	  優先登録処理を行う。
//
// RETURN:
//	  UNA_OK  正常終了
//
// NOTE:
//	  現在は、ed(=latticeEnd) の値を返すだけのダミー関数になってる。
//
static int DmyPrioMorph(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,				/* 文字列中のはじまり位置(オフセット) */
	int *ed,				/* 形態素の文字列上での末尾位置(オフセット) */
	void *arg				/* 引数 */
)
{
	if (mh->lat.prBrnchPos == 0) {	/* 優先登録語はない */
		return UNA_OK;
	}
	*ed = mh->lat.latticeEnd;
	return UNA_OK;
}


//--------------------------------------------------------------------------
// MODULE:	  DmyAppInfo_get
//
// ABSTRACT:	  アプリケーション情報の取得(ダミー関数)
//
// FUNCTION:
//	  アプリケーション情報を取得する。
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//	  ダミー関数である。
//
static int DmyAppInfo_get(
	const unaMorphT *morph,		/* アプリ情報を取得対象の形態素 */
	unaAppInfoT **appInf,		/* アプリ情報 */
	void *arg					/* 引数 */
)
{
	*appInf = (unaAppInfoT*)dInf;
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  DmyGetSubMorph
//
// ABSTRACT:	  下位構造の取得(ダミー関数)
//
// FUNCTION:
//	  下位構造を取得する。
//
// RETURN:
//	  UNA_OK  正常終了
//
// NOTE:
//	  ダミー関数である。
//
static int DmyGetSubMorph(
	const unaMorphT *morph,		/* 下位構造を得たい形態素(単語) */
	unaMorphT *morphBuf,		/* 下位形態素結果が書かれるバッファ */
	int *morphNum,				/* 書かれた下位形態素の数 */
	const int morphBufSize,	/* バッファの大きさ(要素数) */
	void *arg					/* 引数 */
)
{
	*morphNum = 0;				/* 下位構造はない */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------

