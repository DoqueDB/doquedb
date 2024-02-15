// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormalizer.h -- ModNormalizer のクラス定義
// 
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModNormalizer_H_
#define __ModNormalizer_H_

#include "ModCommonDLL.h"
#include "ModHashMap.h"
#include "ModNormDLL.h"
#include "ModNormType.h"
#include "ModNormUNA.h"
#include "ModNormString.h"
#include "Module.h"
_UNA_BEGIN
_UNA_UNAJP_BEGIN

class ModNormRule;

//
// MACRO
// MOD_NORM_NAKATEN_CHECK
//
// NOTES
//
// これを定義すると、中点類の削除を、カタカナに挟まれた場合にのみ行うように
// なる。定義していないと、カタカナに後続している場合に削除が行われる。
// デフォルトでは定義する。
//
#define MOD_NORM_NAKATEN_CHECK


//
// MACRO
// MOD_NORM_IGNORE_BOTH
//
// NOTES
//
// これを定義すると、ModNormBoth の場合に改行・復帰の制御文字が削除されても
// 出力しなくなる。定義していないと、出力する。
// デフォルトでは定義しない。
//
// #define MOD_NORM_IGNORE_BOTH


//  ENUM
//  ModNormExpMode --  展開モード
//
//  NOTES
//	ModNormalizer::expandBuf() の引数として利用される。
//
//	ModNormExpChkOrigStr -- 別の展開文字列を含む展開文字列を省略する
//	ModNormExpNoChk	  -- 省略しない~(デフォルト)
//
enum ModNormExpMode
{
	ModNormExpChkOrigStr,  // for expandBuf()
	ModNormExpNoChk		// default
};

//  ENUM
//  ModNormOutMode --  出力モード
//
//  NOTES
//	ModNormalizer::normalizeBuf()かModNormalizer::extractInit()の引数として
// 利用される。 
//
//	ModNormalized   -- 正規化表記だけ
//	ModNormOriginal -- 原表記だけ
//	ModNormBoth	 -- 引数のデリミタ d1 d2 d3 を区切りとして、
//						 原表記と正規化表記を出力する。
//
enum ModNormOutMode
{
	ModNormalized,
	ModNormOriginal,
	ModNormBoth
};

//  ENUM
//  ModNormUNAStringType --  UNA対象の字種
//
//  NOTES
//  内部用。UNAに渡す文字列はカタカナか英字か。
//
enum ModNormUNAStringType {
	ModNormNull,
	ModNormKana,
	ModNormAlpha
};

// CONST
// ModNormDefaultDelimiter0 -- 区切り文字０のデフォルト
//
// NOTES
// 前処理モードにおける、相違部分の開始位置を示す区切り文字のデフォルト。
// 0xee7b は UNICODE の private use 領域の文字。
//
const ModUnicodeChar ModNormDefaultDelimiter0 = 0xee7b;

// CONST
// ModNormDefaultDelimiter1 -- 区切り文字１のデフォルト
//
// NOTES
// 前処理モードにおける、相違部分の開始位置を示す区切り文字のデフォルト。
// 0xee2c は UNICODE の private use 領域の文字。
//
const ModUnicodeChar ModNormDefaultDelimiter1 = 0xee2c;

// CONST
// ModNormDefaultDelimiter2 -- 区切り文字２のデフォルト
//
// NOTES
// 前処理モードにおける、相違部分の開始位置を示す区切り文字のデフォルト。
// 0xee7d は UNICODE の private use 領域の文字。
//
const ModUnicodeChar ModNormDefaultDelimiter2 = 0xee7d;

// CONST
// ModNormOpeningParenthesis -- Opening Parenthesis letter
//
// NOTES
// Conversion character in order to distinguish it from the delimiter which shows the start position of the different part in pretreatment mode.
// As for 0xee8b letter of the private use territory of UNICODE.
//
const ModUnicodeChar ModNormOpeningParenthesis = 0xee8b;

// CONST
// ModNormClosingParenthesis -- Closing Parenthesis letter
//
// NOTES
// Conversion character in order to distinguish it from the delimiter which shows the start position of the different part in pretreatment mode.
// As for 0xee8c letter of the private use territory of UNICODE.
//
const ModUnicodeChar ModNormClosingParenthesis = 0xee8c;

// CONST
// ModNormDefaultEscape -- エスケープ文字のデフォルト
//
// NOTES
// 前処理モードにおける、エスケープ文字のデフォルト。
// 0xee5c は UNICODE の private use 領域の文字。
//
const ModUnicodeChar ModNormDefaultEscape = 0xee5c;

// CONST
// ModNormDefaultBufferLengthMax -- 最大バッファー長のデフォルト
//
// NOTES
// 正規化処理のバッファー長の最大値のデフォルト値。
//
const ModSize ModNormDefaultBufferLengthMax = 100000;


//
// CLASS
// ModNormalizer -- Unicode 正規化器クラスの定義 
//
// NOTES
//
class ModNormDLL ModNormalizer
	: public ModDefaultObject
{
public:
	// コンストラクタ
	ModNormalizer(const ModNormRule* const rule);
	ModNormalizer(const ModNormRule* const rule, const ModBoolean english);

	// デストラクタ
	~ModNormalizer();

	void addNormalizeRule(const ModNormRule* const rule);
	void switchNormalizeRule(int n);
	ModBoolean existSubRule();

	// 正規化
	void normalizeBuf(
		const ModUnicodeString& input_str,
		ModUnicodeString& output_str,
		const ModSize begin_index = 0,
		const ModSize end_index = 0,
		const ModNormOutMode out_mode = ModNormBoth,
		const ModUnicodeChar d0 = ModNormDefaultDelimiter0,
		const ModUnicodeChar d1 = ModNormDefaultDelimiter1,
		const ModUnicodeChar d2 = ModNormDefaultDelimiter2,
		const ModUnicodeChar d3 = ModNormDefaultEscape,
		const ModSize max_normbuf_len = ModNormDefaultBufferLengthMax);
	ModUnicodeString normalizeBuf(
		const ModUnicodeString& input_str,
		const ModSize begin_index = 0,
		const ModSize end_index = 0,
		const ModNormOutMode out_mode = ModNormBoth,
		const ModUnicodeChar d0 = ModNormDefaultDelimiter0,
		const ModUnicodeChar d1 = ModNormDefaultDelimiter1,
		const ModUnicodeChar d2 = ModNormDefaultDelimiter2,
		const ModUnicodeChar d3 = ModNormDefaultEscape,
		const ModSize max_normbuf_len = ModNormDefaultBufferLengthMax);

	// 異表記展開
	ModSize expandBuf(
		const ModUnicodeString& input_str,
		ModUnicodeString*& expanded,
		const ModNormExpMode chkorg = ModNormExpNoChk,
		const ModSize begin_index = 0,
		const ModSize end_index = 0,
		const ModBoolean expandOnly = ModFalse);

	// 前処理テキストからの情報抽出
	void extractInit(
		const ModUnicodeString& input_str,
		const ModNormOutMode out_mode = ModNormalized,
		const ModUnicodeChar d0 = ModNormDefaultDelimiter0,
		const ModUnicodeChar d1 = ModNormDefaultDelimiter1,
		const ModUnicodeChar d2 = ModNormDefaultDelimiter2,
		const ModUnicodeChar d3 = ModNormDefaultEscape);

	ModUnicodeChar extractGetc();

	void enableMetaNormTable(ModBoolean x1, ModBoolean x2, ModBoolean x3, ModBoolean x4);

	void setExpStrMode(ModBoolean expStrMode_);
	// V125形式の展開データからマッチした展開パターンを返す
	ModBoolean getExpStr(const ModUnicodeString& targetExpStr_, ModUnicodeString& result_);

	ModSize _expand_array(const ModUnicodeString& tmp_str,
						  ModUnicodeString*& expanded);
	
	// 要望により公開
#ifdef MOD_NORM_IGNORE_BOTH
	ModUnicodeChar *ignore_both;
#endif
	ModUnicodeChar *nakaten_meta ;
	ModUnicodeChar *chouon_meta ;
	ModUnicodeChar *hyphen_meta ;

	static const ModUnicodeChar expandDelimiters[];

private:
	static const ModUnicodeChar englishDummyCharacter;
	static const ModUnicodeChar ruleDelimiters[];

	// 原表記と正規化表記の入出力
	ModNormString 	gen_str;			// pre_una
	ModSize 		gen_len;
	ModSize			max_nbuf_len;		// 00.5 added for clipping
	ModUnicodeString seikika_str;		// post-una

	ModBoolean normalizeEnglish;

	// 正規化用データ
	const ModUnicodeChar  *preMap;		// 前処理マップ
	ModUnicodeChar  postMap[0x10000];		// 後処理マップ
	const ModNormSurrogateChar* preSurrogateMap;	// 前処理用サロゲート文字マップ
	const ModNormSurrogateChar* postSurrogateMap;	// 後処理用サロゲート文字マップ
	const ModNormChar* preDecompMap;	// 前処理用分割処理マップ
	const ModNormChar* postDecompMap;	// 後処理用分割処理マップ
	const ModNormCombiMap* combiMap;	// 合字処理マップ

	ModNormUNA*	unaRule;				// 正規化規則
	ModNormUNA*	unaExpand;				// 異表記展開規則
	ModNormUNA*	unaExpStr;				// 異表記展開規則 for getExpandStrings
	char *ruleDic;
	char *ruleApp;
	char *expandDic;
	char *expandApp;
	char *expStrWrdDic;
	char *expStrAppDic;
	char *connectTbl;
	char *unknownTbl;
	char *unknownCost;
	char *normalTbl;

	ModBoolean existRule2;
	ModBoolean normalizeEnglish1;
	const ModUnicodeChar* preMap1;		// 前処理マップ
	const ModUnicodeChar* postMap1;		// 後処理マップ
	const ModNormSurrogateChar* preSurrogateMap1;	// 前処理用サロゲート文字マップ
	const ModNormSurrogateChar* postSurrogateMap1;	// 後処理用サロゲート文字マップ
	const ModNormChar* preDecompMap1;	// 前処理用分割処理マップ
	const ModNormChar* postDecompMap1;	// 後処理用分割処理マップ
	const ModNormCombiMap* combiMap1;	// 合字処理マップ
	ModBoolean normalizeEnglish2;
	const ModUnicodeChar* preMap2;		// 前処理マップ
	const ModUnicodeChar* postMap2;		// 後処理マップ
	const ModNormSurrogateChar* preSurrogateMap2;	// 前処理用サロゲート文字マップ
	const ModNormSurrogateChar* postSurrogateMap2;	// 後処理用サロゲート文字マップ
	const ModNormChar* preDecompMap2;	// 前処理用分割処理マップ
	const ModNormChar* postDecompMap2;	// 後処理用分割処理マップ
	const ModNormCombiMap* combiMap2;	// 合字処理マップ

	// extract 中間データ

	// extract 中間データ

	const ModUnicodeChar*  extr_txt_ptr;
	ModUnicodeChar   extr_ret;
	ModNormOutMode   extr_stat;

	// 正規化中間データ

	ModNormOutMode   _out_mode;
	ModUnicodeChar	 _map_ret[2];
	ModUnicodeChar	 _map2_ret[3];

	// Normalization for the synonym expanding is set.
	ModBoolean expStrMode;

	// 分割正規化関数
	ModBoolean _isBreakpoint(const ModUnicodeChar this_char) const;

	ModSize _find_clip_point(const ModUnicodeString& input_str,
							 const ModSize start_index,
							 const ModSize end_index) const;

	void _normalizeInPieces(const ModUnicodeString& input_str,
							ModUnicodeString& output_str,
							const ModSize begin_index,
							const ModSize end_index,
							const ModNormOutMode out_mode) ;

	// 正規化関数
	const ModUnicodeChar* _map(const ModUnicodeChar g_muc,
							   const ModUnicodeChar* myMap,
							   const ModNormChar* myDecompMap);
	const ModUnicodeChar* _map2(const ModUnicodeChar g_muc1,
							    const ModUnicodeChar g_muc2,
							    const ModNormSurrogateChar* mySurrogateMap);
	const ModUnicodeChar* _decomp(const ModUnicodeChar g_muc,
								  const ModNormChar* myDecompMap) const;

	ModUnicodeChar 	_combine (const ModUnicodeChar combinee,
							  const ModUnicodeChar combiner) const;

	void			_do_rule(ModUnicodeString& una_str,
							 const ModNormUNAStringType una_type);

	void _chk_pre();

	// 正規化関数 ModNormalized 

	void _chk_rule();
	void _chk_post(ModUnicodeString& output_str);

	ModUnicodeChar getNextChar(ModSize, ModSize, ModSize) const;

	// 正規化関数 ModNormBoth

	ModUnicodeChar _both_delims[5];		// 区切り文字
										// null terminate されている必要が
										// あるので 4 個分の領域がいる
	ModUnicodeChar _both_escape;		// エスケープ文字
 	ModUnicodeString _una_str;			// for _chk_rule
	void _do_both(ModUnicodeString& output_str,
				  ModUnicodeString& una_str,
				  const ModNormUNAStringType una_type);
	void _chk_both(ModUnicodeString& output_str);

	ModSize _stri_left;	// orig_str index, left
	ModSize _stri_right;
	void _set_stri_left(const ModSize stri);
	void _set_stri_right(const ModSize stri);

	// within una, for kana & alpha
	ModBoolean _una_do_prn;

	void _output_original(ModUnicodeString& output_str) const;
	void _output_str(ModUnicodeString& output_str,
					 const ModUnicodeChar* mapped_str) const;
	void _output_char_null(ModUnicodeString& output_str,
						   const ModUnicodeChar c) const;
	void _output_char_simple(ModUnicodeString& output_str,
							 const ModUnicodeChar c);
	void _output_char(ModUnicodeString& output_str, const ModSize i);
	void _output_char(ModUnicodeString& output_str,
					  const ModUnicodeChar orig_char,
					  const ModUnicodeChar out_char);
	void _output_post_simple(ModUnicodeString& output_str,
							 const ModUnicodeChar this_char);

	// 異表記展開関数

	void _do_exp(ModUnicodeString& una_str,
				 const ModNormUNAStringType una_type,
				 const ModBoolean skipFirstStep);
	void _chk_exp(const ModBoolean skipFirstStep);
	ModSize _exp_chk_str(ModUnicodeString*& expanded, ModSize exp_cnt);
};

_UNA_UNAJP_END
_UNA_END

#endif // __ModNormalizer_H_
//
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
