//
// unamdunk.cpp -
//      未登録語検出モジュール
//		形態素解析処理の中で未登録語を検出するモジュール
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

#include <stdio.h>				/* デバッグ用 */
#include <assert.h>				/* デバッグ用 */
#include <string.h>				/* strcmp */
#include "UnaBase/unamdunk.h"			/* 未登録語処理モジュール自身 */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------
#define MODULE_NAME "UNAMDUNK"	/* モジュール名 */

/* モジュール内のメッセージ */
#define ERR_VERSION_UMK		"Unsupported UNK Moji Kind Table Version"
#define ERR_VERSION_UC		"Unsupported UNK Cost Table Version"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------

#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
/*
 * 未登録語用文字種別テーブル中のサロゲートペア要素の番号
 * 文字種別による表引きを行う前に記号(SIGN)または漢字(CHINESE_CHARACTER)に変換される
 *   HIGH_SURROGATE + LOW_SURROGATE → 記号(SIGN)
 *   HIGH_SURROGATE_IDEOGRAPH + LOW_SURROGATE → 漢字(CHINESE_CHRACTER)
 *   HIGH_SURROGATE単体 または LOW_SURROGATE単体 → 記号(SIGN)
 */
#define	HIGH_SURROGATE 95						// サロゲートペア前半(非漢字)
#define	HIGH_SURROGATE_IDEOGRAPH 96				// サロゲートペア前半(漢字)
#define	LOW_SURROGATE 97						// サロゲートペア後半
#endif

/* 未登録語用文字種別テーブル中の可変文字種の番号 */
#define KAHEN 98

/* 未登録語登録限界コスト(これ以下のコスト値の未登録語は必ず登録される) */
#define PUT_COST 25

/* 未登録語用文字種別テーブル中の文字種の番号 */
#define CHINESE_CHARACTER 1
#define NUMBER 2
#define ENGLISH_CAPITAL_LETTER 3
#define KATAKANA 5
#define SIGN 6
#define HIRAGANA_7 7
#define HIRAGANA_TSU 32
#define HIRAGANA_39 39
#define SMALL_LETTER_KATAKANA 40
#define CHINESE_NUMERALS 41
#define DOUNOJITEN 42

/* 未登録語種別 */
/* Kind of unregistered word */
#define CHINESE_CHARACTER_STRINGS 0				// 漢字で始まり漢字で終わる(漢字列)
#define CHINESE_CHARACTER_HIRAGANA_STRINGS 1	// 漢字で始まりひらがなで終わる(漢字ひらがな列)
#define HIRAGANA_STRINGS 2						// ひらがなで始まりひらがなで終わる(ひらがな列)
#define NUMERIC_STRINGS 3						// 数字で始まり数字で終わる(数字列)
#define CHINESE_NUMERALS_STRINGS 4				// 漢数字で始まり漢数字で終わる(漢数字列)
#define KATAKANA_NUMERIC_STRINGS 5				// カタカナ列
#define SMALL_LETTER_KATAKANA_STRINGS 6			// カタカナ小字で始まるカタカナ列
#define SIGN_STRINGS 7							// 記号
#define DOUNOJITEN_CHINESE_CHARACTER_STRINGS 8	// "々"で始まり漢字で終わる(々漢字列)
#define DOUNOJITEN_HIRAGANA_STRINGS 9			// "々"で始まりひらがなで終わる(々漢字ひらがな列)
#define ALPHABET_1_LETTER 10					// 英1文字
#define ENGLISH_CAPITAL_LETTER_STRINGS 11		// 英大文字で始まる英字列(英大文字始まり)
#define ENGLISH_SMALL_LETTER_STRINGS 12			// 英小文字で始まる英字列(英小文字始まり)

/* 可変文字種決定関数 */
static int UnkGetKind(unaMdUnkHandleT *uh,unaCharT *txt, int pos, int preK);

/* 未登録語文字列の未登録語種類取得関数 */
static int UnkGetType(int startKind, int endKind, int morLen);

/* 未登録語検出関数(日本語) */
static int SearchMorphJa( unaMorphHandleT *mh, int txtPos,
	int dicNum, int *morCount, void *uHandle);

/* 未登録語検出関数(非日本語) */
static int SearchMorphNotJa( unaMorphHandleT *mh, int txtPos,
	int dicNum, int *morCount, void *uHandle);

/* 未登録語検出関数(日本語) */
static int SearchMorphJaBug( unaMorphHandleT *mh, int txtPos,
	int dicNum, int *morCount, void *uHandle);

/* 未登録語検出関数(非日本語) */
static int SearchMorphNotJaBug( unaMorphHandleT *mh, int txtPos,
	int dicNum, int *morCount, void *uHandle);

//--------------------------------------------------------------------------
// TAG:   UnkMorHinNo
//
// ABSTRACT:    未登録語種類→形態素品詞番号対応表
//
// NOTE:
//	  未登録語8番、9番は次回改良項目
//
static const ucharT UnkMorHinNo[UNA_UNK_MOR_KIND_CNT] = {
								  // 未登録語種類→形態素品詞番号対応表
	UNA_HIN_UNKNOWN_MEISHI_SAHEN, /* 未登録語0番(漢字列)			→ サ変名詞 */
	UNA_HIN_UNKNOWN_KATSUYOUGO,	  /* 未登録語1番(漢字ひらがな列)	→ 活用語 */
	UNA_HIN_UNKNOWN_IPPAN,		  /* 未登録語2番(ひらがな列)		→ 一般 */
	UNA_HIN_SUUSHI,				  /* 未登録語3番(数字列)			→ 数詞 */
	UNA_HIN_SUUSHI,				  /* 未登録語4番(漢数字列)			→ 数詞 */
	UNA_HIN_UNKNOWN_KATAKANA, 	  /* 未登録語5番(カタカナ列)		→ 未登録語.カタカナ */
	UNA_HIN_UNKNOWN_IPPAN,		  /* 未登録語6番(カタカナ小列)		→ 一般 */
	UNA_HIN_UNKNOWN_KIGOU,		  /* 未登録語7番(記号)				→ 記号 */
	UNA_HIN_UNKNOWN_IPPAN,		  /* 未登録語8番(々漢字列)			→ 一般 */
	UNA_HIN_UNKNOWN_IPPAN,		  /* 未登録語9番(々漢字ひらがな列)	→ 一般 */
	UNA_HIN_UNKNOWN_KIGOU_ALPHABET,	/* 未登録語10番(英1文字)		→ アルファ */
	UNA_HIN_UNKNOWN_MEISHI_KOYUU, /* 未登録語11番(英大文字始まり)	→ 固有  */
	UNA_HIN_UNKNOWN_IPPAN		  /* 未登録語12番(英小文字始まり)	→ 一般   */
};

//--------------------------------------------------------------------------
// TAG:   HyokiPointInit
//
// ABSTRACT:    表記の長さの重みの初期値表
//
// NOTE:
//	  未登録語最大長制限判断に用いる表記の長さの重みの初期値を示した表。
//	  なお、マクロ定義より 15 <= UNA_UNK_HYOKI_LIMIT <= 255 である。
//	  (unaMdicUnknown_searchMorph 関数の NOTE 参照)
//
static const ucharT HyokiPointInit[43] = { // 表記の長さの重みの初期値表
	UNA_UNK_HYOKI_LIMIT - 15,	/* 漢字偶 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* 漢字奇 */
	UNA_UNK_HYOKI_LIMIT - 15,	/*  数字  */
	UNA_UNK_HYOKI_LIMIT - 15,	/* 英字大 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* 英字小 */
	0,							/*  カナ  */
	UNA_UNK_HYOKI_LIMIT - 15,	/*  記号  */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら01 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら02 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら03 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら04 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら05 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら06 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら07 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら08 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら09 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら10 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら11 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら12 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら13 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら14 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら15 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら16 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら17 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら18 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら19 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら20 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら21 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら22 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら23 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら24 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら25 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら26 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら27 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら28 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら29 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら30 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら31 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら32 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* ひら33 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* カナ小 */
	UNA_UNK_HYOKI_LIMIT - 15,	/* 漢数字 */
	UNA_UNK_HYOKI_LIMIT - 15	/* 「々」 */
};

//--------------------------------------------------------------------------
// TAG:	  TourokuMtx
//
// ABSTRACT:    未登録語登録マトリックス
//
// NOTE:
//    未登録語登録マトリックス
//    ここで1の場合登録する
//    TourokuMtx[X][Y]のX,Yはそれぞれ文字種を表す。
//    なお、「々」の行及び列の値は、漢字奇と同一である。
//
static const ucharT TourokuMtx[43][43] ={ // 未登録語登録マトリックス
/*           漢漢数英英カ記ひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひカ漢   */
/*           字字  字字    らららららららららららららららららららららららららららららららららナ数々 */
/*           偶奇字大小ナ号010203040506070809101112131415161718192021222324252627282930313233小字   */
/* 漢字偶 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* 漢字奇 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,1},
/*  数字  */ {1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* 英字大 */ {1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* 英字小 */ {1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/*  カナ  */ {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
/*  記号  */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* ひら01 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,1,1},
/* ひら02 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら03 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら04 */ {1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,1,1,1},
/* ひら05 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,1,1,1,1,1},
/* ひら06 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,1,0,0,1,0,1,1,1},
/* ひら07 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1},
/* ひら08 */ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら09 */ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら10 */ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら11 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら12 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,0,0,0,1,0,1,1,0,0,0,1,0,1,1,1},
/* ひら13 */ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら14 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら15 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら16 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,0,0,0,1,0,0,0,1,1,1,0,0,1,1,1,0,1,1,0,1,1,1},
/* ひら17 */ {1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,0,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら18 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら19 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら20 */ {1,1,1,1,1,1,1,0,0,1,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,0,1,1,1,0,0,1,0,1,1,1},
/* ひら21 */ {1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,0,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら22 */ {1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら23 */ {1,1,1,1,1,1,1,0,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,1,1},
/* ひら24 */ {1,1,1,1,1,1,1,0,1,1,1,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら25 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,1,1,1,0,0,1,0,1,1,1},
/* ひら26 */ {1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1},
/* ひら27 */ {1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,1,1,1},
/* ひら28 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
/* ひら29 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,1,1},
/* ひら30 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら31 */ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら32 */ {1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら33 */ {1,1,1,1,1,1,1,0,0,1,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,0,0,1,1,0,0,1,1,1,1,1},
/* カナ小 */ {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
/* 漢数字 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
/* 「々」 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,1}
};

//--------------------------------------------------------------------------
// TAG:	  OwariMtx
//
// ABSTRACT:    未登録語終了判別マトリックス
//
// NOTE:
//    未登録語終了判別マトリックス
//    ここで1の場合未登録語検出を終了する
//    OwariMtx[X][Y]のX,Yはそれぞれ文字種を表す。
//    なお、「々」の行及び列の値は、漢字奇と同一である。
//
static const ucharT OwariMtx[43][43] = { // 未登録語終了判別マトリックス
/*           漢漢数英英カ記ひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひひカ漢   */
/*           字字  字字    らららららららららららららららららららららららららららららららららナ数々 */
/*           偶奇字大小ナ号010203040506070809101112131415161718192021222324252627282930313233小字   */
/* 漢字偶 */ {1,0,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,1,1,1,0,1,1,1,0,0,1,1,1,1,0},
/* 漢字奇 */ {0,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1},
/*  数字  */ {1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* 英字大 */ {1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* 英字小 */ {1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/*  カナ  */ {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
/*  記号  */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
/* ひら01 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,1,0,1,0,1,1,1},
/* ひら02 */ {1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1},
/* ひら03 */ {1,1,1,1,1,1,1,0,0,0,1,0,1,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1},
/* ひら04 */ {1,1,1,1,1,1,1,0,1,0,0,0,0,1,1,0,0,1,1,1,0,0,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
/* ひら05 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1},
/* ひら06 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,1,1,1},
/* ひら07 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1},
/* ひら08 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,1,1,0,0,0,0,0,1,1,1},
/* ひら09 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら10 */ {1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,0,0,0,0,1,1,1},
/* ひら11 */ {1,1,1,1,1,1,1,0,0,0,1,0,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,0,0,0,0,1,1,1},
/* ひら12 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,0,1,1,1,1,1,0,0,1,1,0,0,1,0,0,0,1,0,1,1,0,0,0,1,0,1,1,1},
/* ひら13 */ {1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,0,0,1,1,1},
/* ひら14 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,1,0,1,0,0,0,0,1,1,1},
/* ひら15 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,0,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら16 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,0,1,1,0,0,0,1,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1},
/* ひら17 */ {1,1,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,0,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら18 */ {1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら19 */ {1,1,1,1,1,1,1,0,0,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,0,0,0,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら20 */ {1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1},
/* ひら21 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,0,0,1,1,1,1},
/* ひら22 */ {1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1},
/* ひら23 */ {1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,1,1},
/* ひら24 */ {1,1,1,1,1,1,1,0,1,0,0,0,0,1,1,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,1,0,0,1,1,1,1,1},
/* ひら25 */ {1,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,0,0,1,0,1,1,1},
/* ひら26 */ {1,1,1,1,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1},
/* ひら27 */ {1,1,1,1,1,1,1,0,0,0,1,0,0,1,0,0,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,1,1,1,1},
/* ひら28 */ {1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1},
/* ひら29 */ {1,1,1,1,1,1,1,0,1,0,1,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,1,1},
/* ひら30 */ {1,1,1,1,1,1,1,0,0,0,0,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら31 */ {1,1,1,1,1,1,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1},
/* ひら32 */ {1,1,1,1,1,1,1,0,1,1,0,0,1,1,0,1,1,1,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0,1,1,1,0,0,1,1,1,1,1},
/* ひら33 */ {1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,1,0,0,1,1,1,1,1},
/* カナ小 */ {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
/* 漢数字 */ {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
/* 「々」 */ {0,1,1,1,1,1,1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1}
};

//--------------------------------------------------------------------------
// MODULE:	  unaMdicUnknown_init
//
// ABSTRACT:    未登録語処理モジュールの初期化
//
// FUNCTION:
//	  未登録語処理モジュールの初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//	  なし
//
int unaMdicUnknown_init(
		unaMdUnkHandleT *uh,	 /* ハンドラ */
		const char *unkMKTblImg, /* 未登録語用文字種別テーブルのイメージ */
		const char *uCTblImg,	 /* 未登録語コスト推定テーブルのイメージ */
		int japaneaseMode	     /* 日本語として動作するか否か */
)
{
	const char *imgPtr;			 /* テーブルを格納したメモリへのポインタ */
	int i;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	char *mktbl;
#endif

	imgPtr = unkMKTblImg + UNA_COM_SIZE;
	if (strcmp(imgPtr,UNA_UMK_VER)!=0){	/* バージョンが違う */
		UNA_RETURN(ERR_VERSION_UMK,NULL);
	}
	imgPtr += UNA_VER_SIZE;
	uh->unkMKTblImg = imgPtr;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	/* 辞書のバージョンによらずサロゲートペアを扱えるように
	 * ここで未登録語用文字種別テーブルの内容を変更する */
	mktbl = const_cast<char *>(uh->unkMKTblImg);
	for (i = 0xd800; i <= 0xd83f; i++) {
		/* 追加多言語面(U+10000～U+1FFFF)のサロゲートペア前半 */
		mktbl[i] = HIGH_SURROGATE;
	}
	for (i = 0xd840; i <= 0xd8bf; i++) {
		/* 追加漢字面(U+20000～U+2FFFF), 第三漢字面(U+30000～U+3FFFF)の
		 * サロゲートペア前半 */
		mktbl[i] = HIGH_SURROGATE_IDEOGRAPH;
	}
	for (i = 0xd8c0; i <= 0xdbff; i++) {
		/* 未使用(U+40000～U+DFFFF), 追加特殊用途面(U+E0000～U+EFFFF), 
		 * 私用面(U+F0000～U+10FFFF)のサロゲートペア前半 */
		mktbl[i] = HIGH_SURROGATE;
	}
	for (i = 0xdc00; i <= 0xdfff; i++) {
		/* サロゲートペア後半 */
		mktbl[i] = LOW_SURROGATE;
	}
#endif

	imgPtr = uCTblImg + UNA_COM_SIZE;
	if (strcmp(imgPtr,UNA_UC_VER)!=0){	/* バージョンが違う */
		UNA_RETURN(ERR_VERSION_UC,NULL);
	}
	imgPtr += UNA_VER_SIZE;
	uh->uCTblImg = (const ushortT *)(imgPtr);

	/* 実際に呼び出す関数の振り分け */
	uh->searchFunc = (unaDicFuncT)SearchMorphJa;
	uh->searchFuncBug = (unaDicFuncT)SearchMorphJaBug;
	if ( ! japaneaseMode){
		uh->searchFunc = (unaDicFuncT)SearchMorphNotJa;
		uh->searchFuncBug = (unaDicFuncT)SearchMorphNotJaBug;
	}

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicUnknown_term
//
// ABSTRACT:    未登録語処理モジュールの終了処理
//
// FUNCTION:
//	  未登録語処理モジュールの終了処理を行う
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//
int unaMdicUnknown_term(
		unaMdUnkHandleT *uh		/* ハンドラ */
)
{
	/* 値のリセット */
	uh->unkMKTblImg = (const char *)NULL;
	uh->uCTblImg	= (const ushortT *)NULL;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMdicUnknown_searchMorph
//
// ABSTRACT:    未登録語を検出
//
// FUNCTION:
//	  指定した文字位置から始まる未登録語を格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//	  SearchMorphJa()とSearchMorphNotJa()を呼び分ける
int unaMdicUnknown_searchMorph(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* グローバルデータのテキスト上の文字位置 */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 登録した形態素数 */
	void *uHandle			/* 未登録語検出用ハンドラ(キャストして使用) */
)
{
	unaMdUnkHandleT *uh;	/* ハンドラ(未登録語検出用) */
	uh = (unaMdUnkHandleT *)uHandle;
	if ( uh->emulateBug){
		return ((*(uh->searchFuncBug))(mh,txtPos,dicNum,morCount,uHandle));
	}
	else{
		return ((*(uh->searchFunc))(mh,txtPos,dicNum,morCount,uHandle));
	}
}

//--------------------------------------------------------------------------
// MODULE:   SearchMorphJa
//
// ABSTRACT:    未登録語を検出(日本語)
//
// FUNCTION:
//	  指定した文字位置から始まる未登録語を格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//	  - 可変文字種別 98 は、状況に応じて別のコードにマップされるコードである
//	  - 漢字の文字種別コード 1 も振られ直される可能性があり漢字列の文字種別は
//		以下のような形になる
//			例)  1 0 1 0 1 0 1 0 や  1 0 1 0 1 等
//		但し、漢字「々」から始っている漢字の文字種別は以下の様な形になる
//			例) 42 0 1 0 1 0 1 0 や 42 0 1 0 1 等
//	  - リマップされたコード 0 及び 42 は、文字種番号への変換テーブルには
//		のらないが、登録及び終了判別の対象になるコードである。
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
//	  - サロゲートペアはまとめて1字として扱われ、文字種は漢字または記号となる
//		サロゲートペア前半・後半が単体で出現したときもやはり記号とする
#endif
//
//	  (注1)
//		表記の長さの重みの初期値表より、カタカナ語以外は、未登録語最大長
//		より15小さい値が設定される。これにより、表記の重みが15文字までは、
//		未登録語最大長制限によって強制終了する事はない。
//
//	  (注2)
//		まだ候補がなければ必ず候補出しをする。
//		これは、辞書に登録された形態素が1個もなくても未登録語
//		だけでつながったパスができる様に(形態素解析ができる様に)
//		するためである。
//
int SearchMorphJa(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* グローバルデータのテキスト上の文字位置 */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 登録した形態素数 */
	void *uHandle			/* 未登録語検出用ハンドラ(キャストして使用) */
)
{
	unaMdUnkHandleT *uh;	/* ハンドラ(未登録語検出用) */
	uintT startK;			/* 未登録語開始位置文字種 */
	uintT endK;				/* 未登録語終了位置文字種 */
	uintT nextK;			/* 未登録語終了位置の次の文字の文字種 */
	int morLen;				/* 形態素(未登録語)の文字長 */
	uintT hyokiLenPoint;	/* 表記の長さの重み */
	int tourokuFlg;			/* 登録フラグ(1の場合登録) */
	int owariFlg;			/* 終了フラグ(1の場合終わり) */
	int knjFlg;				/* 漢字の特殊処理用のフラグ */
	uintT appI;				/* アプリケーションインデックス情報 */
	int i;					/* ループ変数 */
	int rv;					/* 関数の返り値 */
	int unkMorType;			/* 未登録語の種類 */
	ushortT unkMorCost;		/* 未登録語単語コスト */
	int unkHin;				/* 未登録語に付与された品詞 */
	int startPos;			/* ループ開始時のtxtPos */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	int isSurrogatePair;	/* 現在の文字がサロゲートペア */
#endif

	/* 必要な変数の初期化 */
	*morCount = 0;
	morLen = 0;
	tourokuFlg = 0;
	owariFlg = 0;
	knjFlg = 1;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	isSurrogatePair = 0;
#endif
	uh = (unaMdUnkHandleT *)uHandle;
	startPos = txtPos;
	startK = (uh->unkMKTblImg)[mh->lat.tbuf[txtPos]];
	if (startK == KAHEN){	/* 可変文字種文字の処理 */
		if (mh->lat.tbuf[txtPos] == 0x3005) {	/* 「々」の時 */
			startK = DOUNOJITEN;	/* 「々」で始った事を示す */
		}
		else {
			startK = SIGN;		/* 形態素先頭では、通常の記号 */
		}
	}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	/* サロゲートペアはまとめて1つの漢字または記号として扱う
	 * サロゲートペア前半または後半が単独で出現した場合は記号として扱う */
	else if (startK == HIGH_SURROGATE || startK == HIGH_SURROGATE_IDEOGRAPH) {
		if (txtPos + 1 < mh->lat.txtLen &&
				(uh->unkMKTblImg)[mh->lat.tbuf[txtPos + 1]] == LOW_SURROGATE) {
			if (startK == HIGH_SURROGATE_IDEOGRAPH) {
				startK = 1;	/* CHINESE_CHARACTERだが位置によって0または1の値をとる */
			} else {
				startK = SIGN;
			}
			startPos = txtPos + 1;
			isSurrogatePair = 1;
		}
		else {
			startK = SIGN;
		}
	}
	else if (startK == LOW_SURROGATE) {
		startK = SIGN;
	}
#endif

	/* 表記の重みの初期値を設定する(注1) */
	hyokiLenPoint = HyokiPointInit[startK];

	nextK = startK;

	/* １文字ずつ見ながら、未登録語候補を形態素枝データに格納する */
	for(i = startPos;;i++){	/* forever */

		/* 現在の未登録語候補文字列の末尾文字種を更新する */
		endK = nextK;
		/* 文字列長をこれから登録しようとする未登録語の長さに更新する */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		if (isSurrogatePair) {
			/* サロゲートペアのとき */
			morLen += 2;
			isSurrogatePair = 0;
		}
		else {
			morLen++;
		}
#else
		morLen++;
#endif

		/* 現在が文字列の最後の文字か判断(ここまでで文字列が終了するか) */
		if (i + 1 >= mh->lat.txtLen || mh->lat.tbuf[i + 1] == 0x0000) {
			tourokuFlg	= 1; /* 未登録語として登録 */
			owariFlg	= 1; /* 未登録語処理終了 */
		}
		else{	/* 終了しない場合はきちんと解析する */
			/* 現在の未登録語候補文字列の次の文字種を求める */
			nextK = (uh->unkMKTblImg)[mh->lat.tbuf[i + 1]];
			if (nextK == KAHEN){	/* 可変文字種文字の処理 */
				nextK = UnkGetKind(uh, mh->lat.tbuf, i + 1, endK);
			}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
			/* サロゲートペアはまとめて1つの漢字または記号として扱う
			 * サロゲートペア前半または後半が単独で出現した場合は記号として扱う */
			else if (nextK == HIGH_SURROGATE || nextK == HIGH_SURROGATE_IDEOGRAPH) {
				if (i + 2 < mh->lat.txtLen &&
						(uh->unkMKTblImg)[mh->lat.tbuf[i + 2]] == LOW_SURROGATE) {
					if (nextK == HIGH_SURROGATE_IDEOGRAPH) {
						nextK = 1;	/* CHINESE_CHARACTERだがいったん1とする */
					} else {
						nextK = SIGN;
					}
					isSurrogatePair = 1;
					i++;
				}
				else {
					nextK = SIGN;
				}
			}
			else if (nextK == LOW_SURROGATE) {
				nextK = SIGN;
			}
#endif
			/* 漢字の場合の特殊処理(奇数番目と偶数番目で違う文字種を振る) */
			if (nextK == 1) {	/* とにかく漢字なら必ずこちらを通る */
				knjFlg = 1 - knjFlg; /* 性能向上のためややこしくなってる */
				nextK = knjFlg;		 /* 文字種が振られ直す場合がある */
				hyokiLenPoint += 2;	 /* 漢字は長さの重み2文字分に数える */
			}
			else {
				knjFlg = 0;
				hyokiLenPoint ++;	/* 漢字以外は長さの重み1文字分に数える */
			}
			/* 終了文字種と次の文字種から未登録語登録及び
									未登録語終了判別マトリクスを参照する */
			tourokuFlg	= TourokuMtx[endK][nextK];
			owariFlg	= OwariMtx[endK][nextK];
			/* 未登録語最大長制限によって強制的に処理を終わらせる場合の判断*/
			if (hyokiLenPoint >= UNA_UNK_HYOKI_LIMIT ||
				(unsigned)morLen >= mh->mwLen){ /* 長さの重み判断 */
				/* 強制終了*/
				owariFlg	= 1;		/* 未登録語処理終了 */
				if (*morCount == 0) {	/* 注2 */
					tourokuFlg = 1;
				}
			}
		}

		/*
		 * 登録、終了フラグによって候補格納／処理終了を行う
		 */
		/* 候補格納 */
		if (tourokuFlg != 0) {
			/* 未登録語の種類を確定する */
			unkMorType = UnkGetType(startK, endK, morLen);

			/* 未登録語形態素コストを設定 */
			unkMorCost = uh->uCTblImg[unkMorType * UNA_HYOKI_LEN_MAX
															+ (morLen - 1)];

#if defined(OUTPUT_SAMELEN_UNK)
			if (unkMorCost <= PUT_COST )/* コストが PUT_COST 以下は必ず登録 */
#else
			/* 同表記未登録語候補の排除
								(同長登録語があれば未登録語は登録しない) */
			if (unkMorCost <= PUT_COST /* コストが PUT_COST 以下は必ず登録 */
			|| (mh->lat.morChk)[morLen] != 1) 	/* 同表記が未登録の時も登録*/
#endif
			{
				/* appI を設定 */
				appI = ((dicNum << 24) | unkMorType);

				/* 品詞を設定 */
				unkHin = UnkMorHinNo[unkMorType];
				if ( unkHin == UNA_HIN_UNKNOWN_KIGOU && (mh->lat).tbuf[txtPos] < 0x0020){ /* 制御記号 */
					unkHin = UNA_HIN_KUTEN; /* 文末と同じ扱いにする */
				}

				/* ラティスに登録 */
				rv = unaMorph_latSet(mh,txtPos,morLen,unkHin,
								unkMorCost,appI,0,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
			}
		}
		/* 処理終了 */
		if (owariFlg == 1) {
			break;
		}
	}

	/* 終了 */
	return UNA_OK;
}

int SearchMorphJaBug(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* グローバルデータのテキスト上の文字位置 */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 登録した形態素数 */
	void *uHandle			/* 未登録語検出用ハンドラ(キャストして使用) */
)
{
	unaMdUnkHandleT *uh;	/* ハンドラ(未登録語検出用) */
	uintT startK;			/* 未登録語開始位置文字種 */
	uintT endK;				/* 未登録語終了位置文字種 */
	uintT nextK;			/* 未登録語終了位置の次の文字の文字種 */
	int morLen;				/* 形態素(未登録語)の文字長 */
	uintT hyokiLenPoint;	/* 表記の長さの重み */
	int tourokuFlg;			/* 登録フラグ(1の場合登録) */
	int owariFlg;				/* 終了フラグ(1の場合終わり) */
	int knjFlg;				/* 漢字の特殊処理用のフラグ */
	uintT appI;				/* アプリケーションインデックス情報 */
	int i;					/* ループ変数 */
	int rv;					/* 関数の返り値 */
	int unkMorType;			/* 未登録語の種類 */
	ushortT unkMorCost;		/* 未登録語単語コスト */
	int unkHin;				/* 未登録語に付与された品詞 */
	int startPos;			/* ループ開始時のtxtPos */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	int isSurrogatePair;	/* 現在の文字がサロゲートペア */
#endif

	/* 必要な変数の初期化 */
	*morCount = 0;
	morLen = 0;
	tourokuFlg = 0;
	owariFlg = 0;
	knjFlg = 1;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	isSurrogatePair = 0;
#endif
	uh = (unaMdUnkHandleT *)uHandle;
	startPos = txtPos;
	startK = (uh->unkMKTblImg)[mh->lat.tbuf[txtPos]];
	if (startK == KAHEN){	/* 可変文字種文字の処理 */
		if (mh->lat.tbuf[txtPos] == 0x3005) {	/* 「々」の時 */
			startK = DOUNOJITEN;	/* 「々」で始った事を示す */
		}
		else {
			startK = SIGN;		/* 形態素先頭では、通常の記号 */
		}
	}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	/* サロゲートペアはまとめて1つの漢字または記号として扱う
	 * サロゲートペア前半または後半が単独で出現した場合は記号として扱う */
	else if (startK == HIGH_SURROGATE || startK == HIGH_SURROGATE_IDEOGRAPH) {
		if (txtPos + 1 < mh->lat.txtLen &&
				(uh->unkMKTblImg)[mh->lat.tbuf[txtPos + 1]] == LOW_SURROGATE) {
			if (startK == HIGH_SURROGATE_IDEOGRAPH) {
				startK = 1;	/* CHINESE_CHARACTERだが位置によって0または1の値をとる */
			} else {
				startK = SIGN;
			}
			startPos = txtPos + 1;
			isSurrogatePair = 1;
		}
		else {
			startK = SIGN;
		}
	}
	else if (startK == LOW_SURROGATE) {
		startK = SIGN;
	}
#endif

	/* 表記の重みの初期値を設定する(注1) */
	hyokiLenPoint = HyokiPointInit[startK];

	nextK = startK;

	/* １文字ずつ見ながら、未登録語候補を形態素枝データに格納する */
	for(i = startPos;;i++){	/* forever */

		/* 現在の未登録語候補文字列の末尾文字種を更新する */
		endK = nextK;
		/* 文字列長をこれから登録しようとする未登録語の長さに更新する */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		if (isSurrogatePair) {
			/* サロゲートペアのとき */
			morLen += 2;
			isSurrogatePair = 0;
		}
		else {
			morLen++;
		}
#else
		morLen++;
#endif

		/* 現在が文字列の最後の文字か判断(ここまでで文字列が終了するか) */
		if (i + 1 >= mh->lat.txtLen || mh->lat.tbuf[i + 1] == 0x0000) {
			tourokuFlg	= 1; /* 未登録語として登録 */
			owariFlg	= 1; /* 未登録語処理終了 */
		}
		else{	/* 終了しない場合はきちんと解析する */
			/* 現在の未登録語候補文字列の次の文字種を求める */
			nextK = (uh->unkMKTblImg)[mh->lat.tbuf[i + 1]];
			if (nextK == KAHEN){	/* 可変文字種文字の処理 */
				nextK = UnkGetKind(uh, mh->lat.tbuf, i + 1, endK);
			}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
			/* サロゲートペアはまとめて1つの漢字または記号として扱う
			 * サロゲートペア前半または後半が単独で出現した場合は記号として扱う */
			else if (nextK == HIGH_SURROGATE || nextK == HIGH_SURROGATE_IDEOGRAPH) {
				if (i + 2 < mh->lat.txtLen &&
						(uh->unkMKTblImg)[mh->lat.tbuf[i + 2]] == LOW_SURROGATE) {
					if (nextK == HIGH_SURROGATE_IDEOGRAPH) {
						nextK = 1;	/* CHINESE_CHARACTERだがいったん1とする */
					} else {
						nextK = SIGN;
					}
					isSurrogatePair = 1;
					i++;
				}
				else {
					nextK = SIGN;
				}
			}
			else if (nextK == LOW_SURROGATE) {
				nextK = SIGN;
			}
#endif
			/* 漢字の場合の特殊処理(奇数番目と偶数番目で違う文字種を振る) */
			if (nextK == CHINESE_CHARACTER) {	/* とにかく漢字なら必ずこちらを通る */
				knjFlg = 1 - knjFlg; /* 性能向上のためややこしくなってる */
				nextK = knjFlg;		 /* 文字種が振られ直す場合がある */
				hyokiLenPoint += 2;	 /* 漢字は長さの重み2文字分に数える */
			}
			else {
				knjFlg = 0;
				hyokiLenPoint ++;	/* 漢字以外は長さの重み1文字分に数える */
			}
			/* 終了文字種と次の文字種から未登録語登録及び
									未登録語終了判別マトリクスを参照する */
			tourokuFlg	= TourokuMtx[endK][nextK];
			owariFlg	= OwariMtx[endK][nextK];
			/* 未登録語最大長制限によって強制的に処理を終わらせる場合の判断*/
			if (hyokiLenPoint >= mh->mwLen){ /* 長さの重み判断 */
				/* 強制終了*/
				owariFlg	= 1;		/* 未登録語処理終了 */
				if (*morCount == 0) {	/* 注2 */
					tourokuFlg = 1;
				}
			}
		}

		/*
		 * 登録、終了フラグによって候補格納／処理終了を行う
		 */
		/* 候補格納 */
		if (tourokuFlg != 0) {
			/* 未登録語の種類を確定する */
			unkMorType = UnkGetType(startK, endK, morLen);

			/* 未登録語形態素コストを設定 */
			unkMorCost = uh->uCTblImg[unkMorType * UNA_HYOKI_LEN_MAX
															+ (morLen - 1)];

#if defined(OUTPUT_SAMELEN_UNK)
			if (unkMorCost <= PUT_COST )/* コストが PUT_COST 以下は必ず登録 */
#else
			/* 同表記未登録語候補の排除
								(同長登録語があれば未登録語は登録しない) */
			if (unkMorCost <= PUT_COST /* コストが PUT_COST 以下は必ず登録 */
			|| (mh->lat.morChk)[morLen] != 1) 	/* 同表記が未登録の時も登録*/
#endif
			{
				/* appI を設定 */
				appI = ((dicNum << 24) | unkMorType);

				/* 品詞を設定 */
				unkHin = UnkMorHinNo[unkMorType];
				if ( unkHin == UNA_HIN_UNKNOWN_KIGOU && 
					(mh->lat).tbuf[txtPos] < 0x0020){ /* 制御記号 */
					unkHin = UNA_HIN_KUTEN; /* 文末と同じ扱いにする */
				}
				
				/* ラティスに登録 */
				rv = unaMorph_latSet(mh,txtPos,morLen,unkHin,
								unkMorCost,appI,0,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
			}
		}
		/* 処理終了 */
		if (owariFlg == 1) {
			break;
		}
	}

	/* 終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  SearchMorphNotJaBug
//
// ABSTRACT:    A non-registered word is detected (non-Japanese).
//
// FUNCTION:
//	  The non-registered word which begins from the specified character position is stored and returned.
//
// RETURN:
//	  UNA_OK	Normal end
//	  Others	Error
//
// NOTE:
//	  A Chinese character, a hiragana, and katakana are divided per one character, and are returned.
//    In order to make operation to the other character the same, it is from a character kind table purposely.
//    The composition which overwrites a map result was taken.
//
int SearchMorphNotJaBug(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* グローバルデータのテキスト上の文字位置 */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 登録した形態素数 */
	void *uHandle			/* 未登録語検出用ハンドラ(キャストして使用) */
)
{
	unaMdUnkHandleT *uh;	/* ハンドラ(未登録語検出用) */
	uintT startK;			/* 未登録語開始位置文字種 */
	uintT endK;				/* 未登録語終了位置文字種 */
	uintT nextK;			/* 未登録語終了位置の次の文字の文字種 */
	int morLen;				/* 形態素(未登録語)の文字長 */
	uintT hyokiLenPoint;	/* 表記の長さの重み */
	int tourokuFlg;			/* 登録フラグ(1の場合登録) */
	int owariFlg;			/* 終了フラグ(1の場合終わり) */
	uintT appI;				/* アプリケーションインデックス情報 */
	int i;					/* ループ変数 */
	int rv;					/* 関数の返り値 */
	int unkMorType;			/* 未登録語の種類 */
	ushortT unkMorCost;		/* 未登録語単語コスト */
	int unkHin;				/* 未登録語に付与された品詞 */
	unaCharT c;
	int startPos;			/* ループ開始時のtxtPos */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	int isSurrogatePair;	/* 現在の文字がサロゲートペア */
#endif

	/* 必要な変数の初期化 */
	*morCount = 0;
	morLen = 0;
	tourokuFlg = 0;
	owariFlg = 0;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	isSurrogatePair = 0;
#endif
	uh = (unaMdUnkHandleT *)uHandle;
	startPos = txtPos;
	c = mh->lat.tbuf[txtPos];
	startK = (uh->unkMKTblImg)[c];
	if ( (c >= 0x3040 && c <= 0x30ff) ||  /* ひらがな,カタカナ */
	          (c >= 0x4e00 && c <= 0x9fff) ||  /* 漢字 */
	          (c >= 0xff66 && c <= 0xff9f) ||  /* 半角文字 */
	          startK == KAHEN || startK == CHINESE_CHARACTER){	  /* 可変文字種/漢字の処理 */
		/* 強制的に修正する */
		startK = SIGN; /* 記号 */
	}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	/* サロゲートペアはまとめてでも単体でも記号とする */
	else if (startK == HIGH_SURROGATE || startK == HIGH_SURROGATE_IDEOGRAPH) {
		if (txtPos + 1 < mh->lat.txtLen &&
				(uh->unkMKTblImg)[mh->lat.tbuf[txtPos + 1]] == LOW_SURROGATE) {
			startPos = txtPos + 1;
			isSurrogatePair = 1;
		}
		startK = SIGN;
	}
	else if (startK == LOW_SURROGATE) {
		startK = SIGN;
	}
#endif

	/* 表記の重みの初期値を設定する(注1) */
	hyokiLenPoint = HyokiPointInit[startK];

	nextK = startK;

	/* １文字ずつ見ながら、未登録語候補を形態素枝データに格納する */
	for(i = startPos;;i++){	/* forever */

		/* 現在の未登録語候補文字列の末尾文字種を更新する */
		endK = nextK;
		/* 文字列長をこれから登録しようとする未登録語の長さに更新する */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		if (isSurrogatePair) {
			/* サロゲートペアのとき */
			morLen += 2;
			isSurrogatePair = 0;
		}
		else {
			morLen++;
		}
#else
		morLen++;
#endif

		/* 現在が文字列の最後の文字か判断(ここまでで文字列が終了するか) */
		if (i + 1 >= mh->lat.txtLen || mh->lat.tbuf[i + 1] == 0x0000) {
			tourokuFlg	= 1; /* 未登録語として登録 */
			owariFlg	= 1; /* 未登録語処理終了 */
		}
		else{	/* 終了しない場合はきちんと解析する */
			/* 現在の未登録語候補文字列の次の文字種を求める */
			c = mh->lat.tbuf[i + 1];
			nextK = (uh->unkMKTblImg)[c];
			if ( (c >= 0x3040 && c <= 0x30ff) ||  /* ひらがな,カタカナ */
			          (c >= 0x4e00 && c <= 0x9fff) ||  /* 漢字 */
			          (c >= 0xff66 && c <= 0xff9f) ||  /* 半角文字 */
			          nextK == KAHEN || nextK == CHINESE_CHARACTER){	  /* 可変文字種/漢字の処理 */
				/* 強制的に修正する */
				nextK = SIGN; /* 記号 */
			}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
			/* サロゲートペアはまとめて1つの記号として扱う
			 * サロゲートペア前半または後半が単独で出現した場合も同様とする */
			else if (nextK == HIGH_SURROGATE || nextK == HIGH_SURROGATE_IDEOGRAPH) {
				if (i + 2 < mh->lat.txtLen &&
						(uh->unkMKTblImg)[mh->lat.tbuf[i + 2]] == LOW_SURROGATE) {
					isSurrogatePair = 1;
					i++;
				}
				nextK = SIGN;
			}
			else if (nextK == LOW_SURROGATE) {
				nextK = SIGN;
			}
#endif
			hyokiLenPoint ++;	/* 長さの重み1文字分に数える */
			
			/* 終了文字種と次の文字種から未登録語登録及び
									未登録語終了判別マトリクスを参照する */
			tourokuFlg	= TourokuMtx[endK][nextK];
			owariFlg	= OwariMtx[endK][nextK];
			/* 未登録語最大長制限によって強制的に処理を終わらせる場合の判断*/
			if (hyokiLenPoint >= mh->mwLen){ /* 長さの重み判断 */
				/* 強制終了*/
				owariFlg	= 1;		/* 未登録語処理終了 */
				if (*morCount == 0) {	/* 注2 */
					tourokuFlg = 1;
				}
			}
		}

		/*
		 * 登録、終了フラグによって候補格納／処理終了を行う
		 */
		/* 候補格納 */
		if (tourokuFlg != 0) {
			/* 未登録語の種類を確定する */
			unkMorType = UnkGetType(startK, endK, morLen);

			/* 未登録語形態素コストを設定 */
			unkMorCost = uh->uCTblImg[unkMorType * UNA_HYOKI_LEN_MAX
															+ (morLen - 1)];

#if defined(OUTPUT_SAMELEN_UNK)
			if (unkMorCost <= PUT_COST )/* コストが PUT_COST 以下は必ず登録 */
#else
			/* 同表記未登録語候補の排除
								(同長登録語があれば未登録語は登録しない) */
			if (unkMorCost <= PUT_COST /* コストが PUT_COST 以下は必ず登録 */
			|| (mh->lat.morChk)[morLen] != 1) 	/* 同表記が未登録の時も登録*/
#endif
			{
				/* appI を設定 */
				appI = ((dicNum << 24) | unkMorType);

				/* 品詞を設定 */
				unkHin = UnkMorHinNo[unkMorType];
				if ( unkHin == UNA_HIN_UNKNOWN_KIGOU && 
					(mh->lat).tbuf[txtPos] < 0x0020){ /* 制御記号 */
					unkHin = UNA_HIN_KUTEN; /* 文末と同じ扱いにする */
				}
				
				/* ラティスに登録 */
				rv = unaMorph_latSet(mh,txtPos,morLen,unkHin,
								unkMorCost,appI,0,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
			}
		}
		/* 処理終了 */
		if (owariFlg == 1) {
			break;
		}
	}

	/* 終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:   SearchMorphNotJa
//
// ABSTRACT:    未登録語を検出(非日本語)
//
// FUNCTION:
//	  指定した文字位置から始まる未登録語を格納して返す。
//
// RETURN:
//	  UNA_OK	正常終了
//	  その他	エラー
//
// NOTE:
//	  漢字、ひらがな、カタカナは１文字単位に分割して返す
//    それ以外の文字への動作を同じにするため、わざわざ文字種テーブルからの
//    マップ結果を上書きする構成をとった
//
int SearchMorphNotJa(
	unaMorphHandleT *mh,	/* 形態素解析メインモジュール用ハンドラ */
	int txtPos,				/* グローバルデータのテキスト上の文字位置 */
	int dicNum,				/* 辞書ナンバー(appI格納のため) */
	int *morCount,			/* 登録した形態素数 */
	void *uHandle			/* 未登録語検出用ハンドラ(キャストして使用) */
)
{
	unaMdUnkHandleT *uh;	/* ハンドラ(未登録語検出用) */
	uintT startK;			/* 未登録語開始位置文字種 */
	uintT endK;				/* 未登録語終了位置文字種 */
	uintT nextK;			/* 未登録語終了位置の次の文字の文字種 */
	int morLen;				/* 形態素(未登録語)の文字長 */
	uintT hyokiLenPoint;	/* 表記の長さの重み */
	int tourokuFlg;			/* 登録フラグ(1の場合登録) */
	int owariFlg;			/* 終了フラグ(1の場合終わり) */
	uintT appI;				/* アプリケーションインデックス情報 */
	int i;					/* ループ変数 */
	int rv;					/* 関数の返り値 */
	int unkMorType;			/* 未登録語の種類 */
	ushortT unkMorCost;		/* 未登録語単語コスト */
	int unkHin;				/* 未登録語に付与された品詞 */
	unaCharT c;
	int startPos;			/* ループ開始時のtxtPos */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	int isSurrogatePair;	/* 現在の文字がサロゲートペア */
#endif

	/* 必要な変数の初期化 */
	*morCount = 0;
	morLen = 0;
	tourokuFlg = 0;
	owariFlg = 0;
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	isSurrogatePair = 0;
#endif
	uh = (unaMdUnkHandleT *)uHandle;
	startPos = txtPos;
	c = mh->lat.tbuf[txtPos];
	startK = (uh->unkMKTblImg)[c];
	if ( (c >= 0x3040 && c <= 0x30ff) ||  /* ひらがな,カタカナ */
	          (c >= 0x4e00 && c <= 0x9fff) ||  /* 漢字 */
	          (c >= 0xff66 && c <= 0xff9f) ||  /* 半角文字 */
	          startK == KAHEN || startK == CHINESE_CHARACTER){	  /* 可変文字種/漢字の処理 */
		/* 強制的に修正する */
		startK = SIGN; /* 記号 */
	}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
	/* サロゲートペアはまとめてでも単体でも記号とする */
	else if (startK == HIGH_SURROGATE || startK == HIGH_SURROGATE_IDEOGRAPH) {
		if (txtPos + 1 < mh->lat.txtLen &&
				(uh->unkMKTblImg)[mh->lat.tbuf[txtPos + 1]] == LOW_SURROGATE) {
			startPos = txtPos + 1;
			isSurrogatePair = 1;
		}
		startK = SIGN;
	}
	else if (startK == LOW_SURROGATE) {
		startK = SIGN;
	}
#endif

	/* 表記の重みの初期値を設定する(注1) */
	hyokiLenPoint = HyokiPointInit[startK];

	nextK = startK;

	/* １文字ずつ見ながら、未登録語候補を形態素枝データに格納する */
	for(i = startPos;;i++){	/* forever */

		/* 現在の未登録語候補文字列の末尾文字種を更新する */
		endK = nextK;
		/* 文字列長をこれから登録しようとする未登録語の長さに更新する */
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		if (isSurrogatePair) {
			/* サロゲートペアのとき */
			morLen += 2;
			isSurrogatePair = 0;
		}
		else {
			morLen++;
		}
#else
		morLen++;
#endif

		/* 現在が文字列の最後の文字か判断(ここまでで文字列が終了するか) */
		if (i + 1 >= mh->lat.txtLen || mh->lat.tbuf[i + 1] == 0x0000) {
			tourokuFlg	= 1; /* 未登録語として登録 */
			owariFlg	= 1; /* 未登録語処理終了 */
		}
		else{	/* 終了しない場合はきちんと解析する */
			/* 現在の未登録語候補文字列の次の文字種を求める */
			c = mh->lat.tbuf[i + 1];
			nextK = (uh->unkMKTblImg)[c];
			if ( (c >= 0x3040 && c <= 0x30ff) ||  /* ひらがな,カタカナ */
			          (c >= 0x4e00 && c <= 0x9fff) ||  /* 漢字 */
			          (c >= 0xff66 && c <= 0xff9f) ||  /* 半角文字 */
			          nextK == KAHEN || nextK == CHINESE_CHARACTER){	  /* 可変文字種/漢字の処理 */
				/* 強制的に修正する */
				nextK = SIGN; /* 記号 */
			}
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
			/* サロゲートペアはまとめて1つの記号として扱う
			 * サロゲートペア前半または後半が単独で出現した場合も同様とする */
			else if (nextK == HIGH_SURROGATE || nextK == HIGH_SURROGATE_IDEOGRAPH) {
				if (i + 2 < mh->lat.txtLen &&
						(uh->unkMKTblImg)[mh->lat.tbuf[i + 2]] == LOW_SURROGATE) {
					isSurrogatePair = 1;
					i++;
				}
				nextK = SIGN;
			}
			else if (nextK == LOW_SURROGATE) {
				nextK = SIGN;
			}
#endif
			hyokiLenPoint ++;	/* 長さの重み1文字分に数える */
			
			/* 終了文字種と次の文字種から未登録語登録及び
									未登録語終了判別マトリクスを参照する */
			tourokuFlg	= TourokuMtx[endK][nextK];
			owariFlg	= OwariMtx[endK][nextK];
			/* 未登録語最大長制限によって強制的に処理を終わらせる場合の判断*/
			if (hyokiLenPoint >= UNA_UNK_HYOKI_LIMIT ||
				(unsigned)morLen >= mh->mwLen){   /* 長さの重み判断 */
				/* 強制終了*/
				owariFlg	= 1;		/* 未登録語処理終了 */
				if (*morCount == 0) {	/* 注2 */
					tourokuFlg = 1;
				}
			}
		}

		/*
		 * 登録、終了フラグによって候補格納／処理終了を行う
		 */
		/* 候補格納 */
		if (tourokuFlg != 0) {
			/* 未登録語の種類を確定する */
			unkMorType = UnkGetType(startK, endK, morLen);

			/* 未登録語形態素コストを設定 */
			unkMorCost = uh->uCTblImg[unkMorType * UNA_HYOKI_LEN_MAX
															+ (morLen - 1)];

#if defined(OUTPUT_SAMELEN_UNK)
			if (unkMorCost <= PUT_COST )/* コストが PUT_COST 以下は必ず登録 */
#else
			/* 同表記未登録語候補の排除
								(同長登録語があれば未登録語は登録しない) */
			if (unkMorCost <= PUT_COST /* コストが PUT_COST 以下は必ず登録 */
			|| (mh->lat.morChk)[morLen] != 1) 	/* 同表記が未登録の時も登録*/
#endif
			{
				/* appI を設定 */
				appI = ((dicNum << 24) | unkMorType);

				/* 品詞を設定 */
				unkHin = UnkMorHinNo[unkMorType];
				if ( unkHin == UNA_HIN_UNKNOWN_KIGOU && 
					(mh->lat).tbuf[txtPos] < 0x0020){ /* 制御記号 */
					unkHin = UNA_HIN_KUTEN; /* 文末と同じ扱いにする */
				}
				
				/* ラティスに登録 */
				rv = unaMorph_latSet(mh,txtPos,morLen,unkHin,
								unkMorCost,appI,0,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
				/* 登録数を更新 */
				(*morCount)++;
				if (rv < 0) {	/* エラー(UNA_ERR_BRNCH_SIZE のみ) */
					return UNA_SYUSOKU;	/* 収束したものとする */
				}
			}
		}
		/* 処理終了 */
		if (owariFlg == 1) {
			break;
		}
	}

	/* 終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:   UnkGetType
//
// ABSTRACT:    未登録語種類取得関数
//
// FUNCTION:
//	  未登録語の先頭文字種と末尾文字種から未登録語の種類を求める
//
// RETURN:
//	  未登録語の種類
//
//	   0 : 漢字ではじまり漢字でおわる(漢字列)
//	   1 : 漢字ではじまりひらがなでおわる(漢字ひらがな列)
//	   2 : ひらがなではじまりひらがなでおわる(ひらがな列)
//	   3 : 数字で始まり数字で終わる(数字列)
//	   4 : 漢数字で始まり漢数字で終わる(漢数字列)
//	   5 : カタカナ列
//	   6 : カタカナ小文字等から始まるカタカナ列
//	   7 : 記号
//	   8 : 漢字「々」ではじまり漢字でおわる(々漢字列)
//	   9 : 漢字「々」ではじまりひらがなでおわる(々漢字ひらがな列)
//	  10 : アルファベット1文字
//	  11 : 英大文字からはじまるアルファベット列
//	  12 : 英小文字からはじまるアルファベット列
//
// NOTE:
//	  コメント中の ek は、末尾の文字種を sk と同方法でリマップした場合の値。
//	  (但し、ek == 0 も考えられる) 又、ここでの連接とは、登録マトリックス
//	  の事である。
//
static int UnkGetType(
	int startKind,			/* 先頭の文字種 */
	int endKind,			/* 末尾の文字種 */
	int morLen				/* 形態素(未登録語)の文字長 */
)
{
	 int sk;				/* 先頭の文字種(マッピングし直したもの) */

	/* 先頭の文字の種類を適当な範囲にまとめる */
	if(startKind >= HIRAGANA_7 && startKind <= HIRAGANA_39){	/* ひらがな */
		sk = SIGN_STRINGS;
	}
	else{
		sk = startKind; /* 1,2,3,4,5,6,40,41,42のどれか。skに 0 は有得ない */
	}

	if (sk == CHINESE_CHARACTER) {
		/* 終わり位置の文字種を求める */
		if (endKind >= HIRAGANA_7 && endKind <= HIRAGANA_39) { /* ひらがな */
			return CHINESE_CHARACTER_HIRAGANA_STRINGS; /* ek == 7 の時 */
		}
		else{
			return CHINESE_CHARACTER_STRINGS; /* こちらは ek == 0 || ek = 1の時。連接より自明 */
		}
	}
	else if (sk == HIRAGANA_7) {			/*ek == 7 は、連接より自明 */
		return HIRAGANA_STRINGS;
	}
	else if (sk == NUMBER) {				/* ek == 2 は、連接より自明 */
		return NUMERIC_STRINGS;
	}
	else if (sk == CHINESE_NUMERALS) {		/* ek == 41 は、連接より自明 */
		return CHINESE_NUMERALS_STRINGS;
	}
	else if (sk == KATAKANA) {				/* ek == 5 || ek == 40 は、連接より自明 */
		return KATAKANA_NUMERIC_STRINGS;
	}
	else if (sk == SMALL_LETTER_KATAKANA) {	/* ek == 5 || ek == 40 は、連接より自明 */
		return SMALL_LETTER_KATAKANA_STRINGS;
	}
	else if (sk == SIGN) {					/* ek == 7 は、連接より自明 */
		return SIGN_STRINGS;
	}
	else if (sk == DOUNOJITEN) {			/* ek == 0 || ek = 1 は、連接より自明 */
		/* 終わり位置の文字種を求める */
		if (endKind >= HIRAGANA_7 && endKind <= HIRAGANA_39) { /* ひらがな */
			return DOUNOJITEN_HIRAGANA_STRINGS;	/* ek == 7 の時 */
		}
		else{
			return DOUNOJITEN_CHINESE_CHARACTER_STRINGS; /* こちらは ek == 0 || ek = 1の時。連接より自明 */
		}
	}

	/* ここまで来たということは、アルファベット列という事 */

	if (morLen == 1) {	/* 長さが1のアルファベット */
		return ALPHABET_1_LETTER;
	}

	if (sk == ENGLISH_CAPITAL_LETTER) {	/* ek == 3 || ek == 4 は、連接より自明 */
		return ENGLISH_CAPITAL_LETTER_STRINGS;
	}

	return ENGLISH_SMALL_LETTER_STRINGS; /* その他(英小文字からはじまるアルファベット列) */
}

//--------------------------------------------------------------------------
// MODULE:   UnkGetKind
//
// ABSTRACT:    可変文字種文字の文字種の決定
//
// FUNCTION:
//	  可変文字種文字の文字種を決定する
//
// RETURN:
//	  文字種番号を返す。
//		1		漢字
//		6		通常の記号
//		32		「っ」と同じ扱いのひらがな
//		40		カタカナ小文字等
//
// NOTE:
//	  文字種別テーブルに 98 が指定されていた場合、当該文字は可変文字種別文字
//	  ということになり、この関数に飛んでくる。
//	  そして、その文字に対する処理が記述されていれば登録及び終了判別の対象に
//	  なる文字種にマップされ直す。
//	  記述が無い場合には、文字種は、コード 6 (記号)になる。
//
//	  ひらがなには、各文字に文字種が割り当てられておりひらがなという
//	  文字種はない。従って、可変文字種文字をひらがなの一種としてマップし直す
//	  場合には、「っ」(コード32)をひらがなの代表として返す。
//
static int UnkGetKind(
	unaMdUnkHandleT *uh,	/* ハンドラ(未登録語検出用) */
	unaCharT *text,			/* 文字列 */
	int txtPos,				/* 文字位置(オフセット) */
	int preK				/* 直前の文字種 */
)
{
	int kind;				/* 1つ前の文字位置の文字コード */

	assert(txtPos >= 1);
	assert(preK>=0 && preK<=DOUNOJITEN);

	kind = preK;

	switch (text[txtPos]) {	/* 現文字位置の文字コード */
	case 0x3005: /* 「々」なら漢字 */
		return CHINESE_CHARACTER;
		break;
	case 0x30FC: /* 長音 */
	case 0xFF70: /* HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK */
	case 0x002D: /* HYPHEN-MINUS */
	case 0x2010: /* HYPHEN */
	case 0x2011: /* NON-BREAKING HYPHEN */
	case 0x2015: /* HORIZONAL BAR */
	case 0x207B: /* SUPERSCRIPT MINUS */
	case 0x208B: /* SUBSCRIPT MINUS */
	case 0x2212: /* MINUS SIGN */
	case 0xFE63: /* SMALL HYPHEN-MINUS */
	case 0xFF0D: /* FULLWIDTH HYPHEN-MINUS */
		if (kind == KATAKANA || kind == SMALL_LETTER_KATAKANA) {
			/* カタカナ又はカタカナ小文字等の後ろならカタカナ小文字等 */
			return SMALL_LETTER_KATAKANA;
		}
		else if (kind >= HIRAGANA_7 && kind <= HIRAGANA_39){
			/* ひらがなの後ろなら「っ」と同じ扱いのひらがな */
			return HIRAGANA_TSU;
		}
		break;
	case 0x3099: /* COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK */
	case 0x309A: /* COMBINING KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
	case 0x309B: /* KATAKANA-HIRAGANA VOICED SOUND MARK */
	case 0x309C: /* KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK */
	case 0xFF9E: /* HALFWIDTH KATAKANA VOICED SOUND MARK */
	case 0xFF9F: /* HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK */
		if (kind == KATAKANA || kind == SMALL_LETTER_KATAKANA) {
			/* カタカナ又はカタカナ小文字等の後ろならカタカナ小文字など */
			return SMALL_LETTER_KATAKANA;
		}
		else if (kind >= HIRAGANA_7 && kind <= HIRAGANA_39){
			/* ひらがなの後ろなら「っ」と同じ扱いのひらがな */
			return HIRAGANA_TSU;
		}
		break;
	case 0x309D: /* 「ゝ」(HIRAGANA ITERATION MARK) */
	case 0x309E: /* 「ゞ」(HIRAGANA VOICED ITERATION MARK) */
		if (kind >= HIRAGANA_7 && kind <= HIRAGANA_39){
			/* ひらがなの後ろなら「っ」と同じ扱いのひらがな */
			return HIRAGANA_TSU;
		}
		break;
	case 0x30FD: /* 「ヽ」(KATAKANA ITERATION MARK) */
	case 0x30FE: /* 「ヾ」(KATAKANA VOICED ITERATION MARK) */
		if (kind == KATAKANA || kind == SMALL_LETTER_KATAKANA) {
			/* カタカナ又はカタカナ小文字等の後ろならカタカナ小文字等 */
			return SMALL_LETTER_KATAKANA;
		}
		break;
	}

	/* それ以外は普通の記号 */
	return SIGN;
}

void unaMdicUnknown_setEmulateBug(
	unaMdUnkHandleT *uh,	// handler
	int emulateBug)	// switch for emulate max-word-length bug
{
	uh->emulateBug = emulateBug;
}
//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------
