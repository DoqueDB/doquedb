// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModNormalizer.cpp -- Unicode対応異表記正規化の正規化器定義
// 
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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

#include "ModNlpUnaJp/ModNormalizer.h"
#include "ModNlpUnaJp/ModNormRule.h"
_UNA_USING
_UNA_UNAJP_USING

#ifdef MOD_NORM_IGNORE_BOTH
ModUnicodeChar ignore_both_default[] = {
#include "ModNormIgnoreBoth.tbl"
};
#endif

// CONST
// nakaten_meta_default -- 中点メタ処理のためのテーブル
//
// NOTES
// 中点メタ処理のために中点類として扱う文字を保持するテーブル。
//
ModUnicodeChar nakaten_meta_default[] = {
#include "ModNormMetaNakaten.tbl"
};

// CONST
// chouon_meta_default -- 長音メタ処理のためのテーブル
//
// NOTES
// 長音メタ処理のために中点類として扱う文字を保持するテーブル。
//
ModUnicodeChar chouon_meta_default[] = {
#include "ModNormMetaChouon.tbl"
};

// CONST
// hyphen_meta_default -- ハイフン・メタ処理のためのテーブル
//
// NOTES
// ハイフン・メタ処理のために中点類として扱う文字を保持するテーブル。
//
ModUnicodeChar hyphen_meta_default[] = {
#include "ModNormMetaHyphen.tbl"
};

// CONST
// null_meta -- メタ処理(無し)のためのテーブル
//
// NOTES
// メタ処理をスキップする場合のカラテーブル
//
ModUnicodeChar null_meta[] = {0};

// CONST
// ModNormBothBousFactor -- 出力文字列の増加係数
//
// NOTES
// 出力文字列の入力文字列に対する増加係数。
// 出力文字列の領域確保に使用する。
//
const double ModNormBothBonusFactor(1.4);

// CONST
// ModNormalizer::englishDummyCharacter -- 英語用ダミー文字
//
// NOTES
// 英語正規化の際に、前後に付与すべきダミー文字
//
const ModUnicodeChar
ModNormalizer::englishDummyCharacter(ModUnicodeChar('_'));

// CONST
// ModNormalizer::ruleDelimiters -- 正規化用区切り文字
//
// NOTES
// 正規化ルールに使用されている区切り文字。
// ※ ルールファイルの記述とあっていること。
//
const ModUnicodeChar
ModNormalizer::ruleDelimiters[] = {
	ModUnicodeChar('{'), ModUnicodeChar(','), ModUnicodeChar('}') };

// CONST
// ModNormalizer::ruleDelimiters -- 展開用区切り文字
//
// NOTES
// 展開ルールに使用されている区切り文字。
// ※ ルールファイルの記述とあっていること。
//
const ModUnicodeChar
ModNormalizer::expandDelimiters[] = {
	ModUnicodeChar('('), ModUnicodeChar(','), ModUnicodeChar(')') };



// FUNCTION
// ModNormalizer::ModNormalizer -- 正規化器のコンストラクタ
//
// NOTES
// 与えられたルールに基づいて初期化を行なう。
// 英語処理指定が行われなければ、ルールの指定にしたがう。
//
// ARGUMENTS
// const ModNormRule* const rule
//		ルールオブジェクト
// const ModBoolean english
//		英語処理指定
//
// RETURN
// なし
//
// EXCEPTIONS
// なし。下位の初期化関数から例外が直接に投げられる。
// 
ModNormalizer::ModNormalizer(const ModNormRule* const rule)
	: gen_len(0),
	  seikika_str(),
	  _out_mode(ModNormBoth),
	  unaRule(0),
	  unaExpand(0),
	  unaExpStr(0),
	  combiMap(0),
	  normalizeEnglish(ModFalse),
	  existRule2(ModFalse),
	  expStrMode(ModFalse)
{
	if (rule == 0) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	try {
		// UNA を使うための準備
		// normalization rule
		unaRule
			= new ModNormUNA(rule->ruleDic, rule->ruleApp,
							 rule->connectTbl, rule->unknownTbl,
							 rule->unknownCost, rule->normalTbl);

		// normal expanding rule
		unaExpand
			= new ModNormUNA(rule->expandDic, rule->expandApp,
							 rule->connectTbl, rule->unknownTbl,
							 rule->unknownCost, rule->normalTbl);
		
		if(rule->expStrWrdDic != 0 && rule->expStrAppDic != 0){
			unaExpStr
				= new ModNormUNA(rule->expStrWrdDic, rule->expStrAppDic,
								 rule->connectTbl, rule->unknownTbl,
								 rule->unknownCost, rule->normalTbl);
		}
	} catch (ModException& e) {
		delete unaRule;
		delete unaExpand;
		if (unaExpStr){
			delete unaExpStr;
		}
		ModRethrow(e);
	}
	ruleDic = rule->ruleDic;
	ruleApp = rule->ruleApp;
	expandDic = rule->expandDic;
	expandApp = rule->expandApp;
	if(rule->expStrWrdDic != 0 && rule->expStrAppDic != 0){
		expStrWrdDic = rule->expStrWrdDic;
		expStrAppDic = rule->expStrAppDic;
	}
	connectTbl = rule->connectTbl;
	unknownTbl = rule->unknownTbl;
	unknownCost = rule->unknownCost;
	normalTbl = rule->normalTbl;
	normalizeEnglish = rule->normalizeEnglish;
	preMap = rule->preMap;
	int ii;
	for (ii=0;ii<0x10000;++ii){
		postMap[ii] = (rule->postMap)[ii];
	}
	preSurrogateMap = rule->preSurrogateMap;
	postSurrogateMap = rule->postSurrogateMap;
	preDecompMap = rule->preDecompMap;
	postDecompMap = rule->postDecompMap;
	combiMap = rule->combiMap;

	normalizeEnglish1 = rule->normalizeEnglish;
	preMap1           = rule->preMap;
	postMap1          = rule->postMap;
	preSurrogateMap1  = rule->preSurrogateMap;
	postSurrogateMap1 = rule->postSurrogateMap;
	preDecompMap1     = rule->preDecompMap;
	postDecompMap1    = rule->postDecompMap;
	combiMap1         = rule->combiMap;

	_map_ret[0] = 0;
	_map_ret[1] = 0;

	_map2_ret[0] = 0;
	_map2_ret[1] = 0;
	_map2_ret[2] = 0;

	// 区切り文字の terminater を設定する
	_both_delims[4] = 0;

	// set meta table using rule->useMetaDef
	if ( rule->useMetaDef == ModTrue){
#if MOD_NORM_IGNORE_BOTH
		ignore_both  = ignore_both_default;
#endif
		nakaten_meta = nakaten_meta_default;
		chouon_meta  = chouon_meta_default;
		hyphen_meta  = hyphen_meta_default;
	}
	else{
#if MOD_NORM_IGNORE_BOTH
		ignore_both  = null_meta;
#endif
		nakaten_meta = null_meta;
		chouon_meta  = null_meta;
		hyphen_meta  = null_meta;
	}
}

ModNormalizer::ModNormalizer(const ModNormRule* const rule,
							 const ModBoolean english)
	: gen_len(0),
	  seikika_str(),
	  _out_mode(ModNormBoth),
	  unaRule(0),
	  unaExpand(0),
	  unaExpStr(0),
	  combiMap(0),
	  normalizeEnglish(english),
	  existRule2(ModFalse),
  	  expStrMode(ModFalse)
{
	if (rule == 0) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	try {
		// UNA を使うための準備
		unaRule
			= new ModNormUNA(rule->ruleDic, rule->ruleApp,
							 rule->connectTbl, rule->unknownTbl,
							 rule->unknownCost, rule->normalTbl);
		unaExpand
			= new ModNormUNA(rule->expandDic, rule->expandApp,
							 rule->connectTbl, rule->unknownTbl,
							 rule->unknownCost, rule->normalTbl);
		if(rule->expStrWrdDic != 0 && rule->expStrAppDic != 0){
			unaExpStr
				= new ModNormUNA(rule->expStrWrdDic, rule->expStrAppDic,
								 rule->connectTbl, rule->unknownTbl,
								 rule->unknownCost, rule->normalTbl);
		}
	} catch (ModException& e) {
		if ( unaRule){
			delete unaRule;
		}
		if ( unaExpand){
			delete unaExpand;
		}
		if ( unaExpStr){
			delete unaExpStr;
		}
		ModRethrow(e);
	}

	ruleDic = rule->ruleDic;
	ruleApp = rule->ruleApp;
	expandDic = rule->expandDic;
	expandApp = rule->expandApp;
	if(rule->expStrWrdDic != 0 && rule->expStrAppDic != 0){
		expStrWrdDic = rule->expStrWrdDic;
		expStrAppDic = rule->expStrAppDic;
	}
	connectTbl = rule->connectTbl;
	unknownTbl = rule->unknownTbl;
	unknownCost = rule->unknownCost;
	normalTbl = rule->normalTbl;
	preMap = rule->preMap;
	int ii;
	for (ii=0;ii<0x10000;++ii){
		postMap[ii] = (rule->postMap)[ii];
	}
	preSurrogateMap = rule->preSurrogateMap;
	postSurrogateMap = rule->postSurrogateMap;
	preDecompMap = rule->preDecompMap;
	postDecompMap = rule->postDecompMap;
	combiMap = rule->combiMap;

	normalizeEnglish1 = rule->normalizeEnglish;
	preMap1           = rule->preMap;
	postMap1          = rule->postMap;
	preSurrogateMap1  = rule->preSurrogateMap;
	postSurrogateMap1 = rule->postSurrogateMap;
	preDecompMap1     = rule->preDecompMap;
	postDecompMap1    = rule->postDecompMap;
	combiMap1         = rule->combiMap;

	_map_ret[0] = 0;
	_map_ret[1] = 0;

	_map2_ret[0] = 0;
	_map2_ret[1] = 0;
	_map2_ret[2] = 0;

	// 区切り文字の terminater を設定する
	_both_delims[4] = 0;
	 
	_una_str.reallocate(256);	// 領域確保

	// set meta table using rule->useMetaDef
	if ( rule->useMetaDef == ModTrue){
#if MOD_NORM_IGNORE_BOTH
		ignore_both  = ignore_both_default;
#endif
		nakaten_meta = nakaten_meta_default;
		chouon_meta  = chouon_meta_default;
		hyphen_meta  = hyphen_meta_default;
	}
	else{
#if MOD_NORM_IGNORE_BOTH
		ignore_both  = null_meta;
#endif
		nakaten_meta = null_meta;
		chouon_meta  = null_meta;
		hyphen_meta  = null_meta;
	}
}

// FUNCTION
// ModNormalizer::addNormalizeRule -- ルールを追加する
//
// NOTES
// 与えられたルールをノーマライザに追加する。
// UNAデータ部分は共有が前提となっている。
// 複数回呼ばれた場合は上書きされる。
//
// ARGUMENTS
// const ModNormRule* const rule
//		ルールオブジェクト
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		ルールオブジェクトが無い/一部が欠けている/共有部が共有不能
//
void 
ModNormalizer::addNormalizeRule(const ModNormRule* const rule_)
{
	// チェック
	if (rule_ == 0 ||
		rule_->ruleDic != ruleDic || rule_->ruleApp != ruleApp ||
		rule_->connectTbl!= connectTbl || rule_->unknownTbl !=unknownTbl||
		rule_->unknownCost!=unknownCost || rule_->normalTbl!=normalTbl||
		rule_->expandDic !=expandDic || rule_->expandApp !=expandApp ||
		rule_->preMap == 0 || rule_->postMap == 0 ||
		rule_->preSurrogateMap == 0 || rule_->postSurrogateMap == 0 ||
	 	rule_->preDecompMap == 0 || rule_->postDecompMap == 0 ||
		rule_->combiMap == 0){
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	normalizeEnglish2 = rule_->normalizeEnglish;
	preMap2           = rule_->preMap;
	postMap2          = rule_->postMap;
	preSurrogateMap2  = rule_->preSurrogateMap;
	postSurrogateMap2 = rule_->postSurrogateMap;
	preDecompMap2     = rule_->preDecompMap;
	postDecompMap2    = rule_->postDecompMap;
	combiMap2         = rule_->combiMap;
	existRule2 = ModTrue;

}

// FUNCTION
// ModNormalizer::enableMetaNormTable -- metadef treatment switching
//
// NOTES
// This function will switch treatment of metadef table
// value sn = ModTrue  --> use default metafed table
// value sn = ModFalse --> ignore metadef table
//
// ARGUMENTS
// ModBoolean s1,s2,s3,s4
//	switch for the treatments of metadef table
//
// RETURN
//  non
//
// EXCEPTIONS
//
void 
ModNormalizer::enableMetaNormTable(
	ModBoolean s1,
	ModBoolean s2,
	ModBoolean s3,
	ModBoolean s4)
{
#if MOD_NORM_IGNORE_BOTH
	if ( s1 == ModTrue){
		ignore_both = ignore_both_default;
	}
	else{
		ignore_both  = null_meta;
	}
#endif
	if ( s2 == ModTrue){
		nakaten_meta = nakaten_meta_default;
	}
	else{
		nakaten_meta = null_meta;
	}

	if ( s3 == ModTrue){
		chouon_meta  = chouon_meta_default;
	}
	else{
		chouon_meta  = null_meta;
	}
	
	if ( s4 == ModTrue){
		hyphen_meta  = hyphen_meta_default;
	}
	else{
		hyphen_meta  = null_meta;
	}

}

// FUNCTION
// ModNormalizer::switchNormalizeRule -- ルールを変更する
//
// NOTES
// ノーマライザのルールを指定されたものに変更する。
// 現在は1/2のみの番号が指定可能である。
//
// ARGUMENTS
// int n
//		変更すべきルールの指示
//
// RETURN
// なし
// 
void
ModNormalizer::switchNormalizeRule(int n)
{
	if ( n == 1){ // reset rules
		normalizeEnglish = normalizeEnglish1;
		preMap           = preMap1;
		int ii;
		for(ii=0;ii<0x10000;++ii){
			postMap[ii]=postMap1[ii];
		}
		preSurrogateMap  = preSurrogateMap1;
		postSurrogateMap = postSurrogateMap1;
		preDecompMap     = preDecompMap1;
		postDecompMap    = postDecompMap1;
		combiMap         = combiMap1;
	}
	else if (n == 2){
		if (existRule2) {
			normalizeEnglish = normalizeEnglish2;
			preMap           = preMap2;
			int ii;
			for(ii=0;ii<0x10000;++ii){
				postMap[ii]=postMap2[ii];
			}
			preSurrogateMap  = preSurrogateMap2;
			postSurrogateMap = postSurrogateMap2;
			preDecompMap     = preDecompMap2;
			postDecompMap    = postDecompMap2;
			combiMap         = combiMap2;
		}
		else {
			normalizeEnglish = normalizeEnglish1;
			preMap           = preMap1;
			int ii;
			for(ii=0;ii<0x10000;++ii){
				postMap[ii]=postMap1[ii];
			}
			preSurrogateMap  = preSurrogateMap1;
			postSurrogateMap = postSurrogateMap1;
			preDecompMap     = preDecompMap1;
			postDecompMap    = postDecompMap1;
			combiMap         = combiMap1;
		}
	}
	else if (n == 3){ // replace space rule in postMap
		int ii;
		for(ii=0;ii<0x10000;++ii){
			if ( postMap1[ii] == 0x20){
				postMap[ii]=ii;
			}
		}
	}
	else if (n == 4){ // set space rule postMap
		int ii;
		for(ii=0;ii<0x10000;++ii){
			if ( postMap1[ii] == 0x20){
				postMap[ii]=0x0;
			}
		}
	}
	_map_ret[0] = 0;
	_map_ret[1] = 0;

	_map2_ret[0] = 0;
	_map2_ret[1] = 0;
	_map2_ret[2] = 0;

	// Terminater of pause letter is set
	_both_delims[4] = 0;

}

// FUNCTION
// ModNormalizer::~ModNormalizer -- デストラクタ
//
// NOTES
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModNormalizer::~ModNormalizer()
{
	if ( unaRule){
		delete unaRule;
	}
	if ( unaExpand){
		delete unaExpand;
	}
	if ( unaExpStr){
		delete unaExpStr;
	}
}

ModBoolean
ModNormalizer::existSubRule()
{return existRule2;}

// FUNCTION
// ModNormalizer::setExpStrMode -- set normalizer for the synonym expanding
// 
// NOTES
// Normalizer for the synonym expanding is set.
// 
// ARGUMENTS
// ModBoolean expStrMode_
// 		If it is ModTrue, normalizer for the synonym development is set.
// 
// RETURN
// void
//
void
ModNormalizer::setExpStrMode(ModBoolean expStrMode_)
{
	expStrMode = expStrMode_;
}

// FUNCTION
// ModNormalizer::getExpStr -- get expand pattern
// 
// NOTES
// This function is used to get expand pattern from V124 expand data.
// 
// ARGUMENTS
// const ModUnicodeString& targetExpStr_
// 		expand target string
// ModUnicodeString& result_
// 		expand string
// 
// RETURN
// ModTrue UNA_KNOWN_WORD
// ModFalse UNA_UNKNOWN_WORD
//
ModBoolean
ModNormalizer::getExpStr(const ModUnicodeString& targetExpStr_, ModUnicodeString& result_)
{
	ModBoolean regUnaWord = ModTrue;
	unaExpStr->getAppInfo(targetExpStr_, result_, regUnaWord);
	return regUnaWord;
}

//
//  正規化 private
//

// FUNCTION private
// ModNormalizer::_decomp -- 文字の分割
// NOTES
// myDecompMap を用いて入力文字を分解する。
// 前処理、後処理で引数を取り替えて使う。
//
// ARGUMENTS
// const ModUnicodeChar g_muc
//		対象文字
// const ModNormChar* myDecompMap
//		１対Ｎ変換のための分解用文字マップ
//
// RETURN
// 分解した結果の文字列
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		入力文字が不適切で分解マップで見つかるべきものが見つからなかった
// 
const ModUnicodeChar* 
ModNormalizer::_decomp(const ModUnicodeChar g_muc,
					   const ModNormChar* myDecompMap) const
{
	// 分解用テーブルを線形検索する
	for (ModSize i = 0; i < ModNormDecompMapMax; i++) {
		ModUnicodeChar d_char(myDecompMap[i].orig_char);
		if (d_char == g_muc)  {
			return (const ModUnicodeChar*)(myDecompMap[i].repl_str);
		}
		if (d_char == 0)
			break;
	}

	// _map からしか呼び出されないので、結果が見つからないはずはない
#ifdef DEBUG
	ModErrorMessage << "DecompMap search failed: " << g_muc << ModEndl;
#endif
	ModThrow(ModModuleStandard,
			 ModCommonErrorBadArgument, ModErrorLevelError);

	return 0;
}

// FUNCTION private
// ModNormalizer::_map -- 文字の変換
//
// NOTES
// myMap, myDecompMap を用いて入力文字をマップする。
// 前処理、後処理で引数を取り替えて使う。
//
// ARGUMENTS
// const ModUnicodeChar g_muc
//		対象文字
// const ModUnicodeChar* myMap
//		１対１変換のための文字マップ
// const ModNormChar* myDecompMap
//		１対Ｎ変換のための分解用文字マップ
//
// RETURN
// マップした結果の文字列
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
const ModUnicodeChar*  
ModNormalizer::_map(const ModUnicodeChar g_muc, 
					const ModUnicodeChar* myMap,
					const ModNormChar* myDecompMap)
{
	ModUnicodeChar map_muc = myMap[g_muc];
	if (map_muc == 0xFFFF && g_muc != 0xFFFF) {
		// 0xFFFF の場合、分解する
		return _decomp(g_muc, myDecompMap);
	}

	_map_ret[0] = map_muc;

	return _map_ret;
}

// FUNCTION private
// ModNormalizer::_map2 -- サロゲート文字の変換
//
// NOTES
// mySurrogateMap を用いて入力サロゲート文字をマップする。
// 前処理、後処理で引数を取り替えて使う。
//
// ARGUMENTS
// const ModUnicodeChar g_muc1
//		対象ハイサロゲート
// const ModUnicodeChar g_muc2
//		対象ローサロゲート
// const ModNormSurrogateChar* mySurrogateMap
//		１対１～Ｎ変換のためのサロゲート文字マップ
//
// RETURN
// マップした結果の文字列
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
const ModUnicodeChar*  
ModNormalizer::_map2(const ModUnicodeChar g_muc1,
					 const ModUnicodeChar g_muc2,
					 const ModNormSurrogateChar* mySurrogateMap)
{
	// サロゲート文字用テーブルを線形検索する
	for (ModSize i = 0; i < ModNormSurrogateMapMax; i++) {
		if (mySurrogateMap[i].orig_high == 0) {
			// テーブルのこれ以降の部分は使われていない
			break;
		}
		ModUnicodeChar s_char1(mySurrogateMap[i].orig_high);
		ModUnicodeChar s_char2(mySurrogateMap[i].orig_low);
		if (s_char1 == g_muc1 && s_char2 == g_muc2)  {
			return (const ModUnicodeChar*)(mySurrogateMap[i].repl_str);
		}
	}

	// 見つからなかったときは原表記をそのままマップとして返す
	_map2_ret[0] = g_muc1;
	_map2_ret[1] = g_muc2;
	return _map2_ret;
}
 
// FUNCTION private
// ModNormalizer::_combine -- 文字の結合
//
// NOTES
// combineeとcombinerが結合可能であれば、結合した結果を返す。
// 結合可能でなければ、NULL文字を返す。
//
// ARGUMENTS
// const ModUnicodeChar combinee
//		結合対象文字
// const ModUnicodeChar combiner
//		結合文字
//
// RETURN
// 結合した結果の文字列
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModUnicodeChar  
ModNormalizer::_combine(const ModUnicodeChar combinee,
						const ModUnicodeChar combiner) const
{
	ModNormCombiMap::ConstIterator result(
		combiMap->find((combinee << 16) + combiner));

	return (result == combiMap->end()) ?
		ModUnicodeChar(0) : ModNormCombiMap::getValue(result);
}

// FUNCTION
// ModNormalizer::_output_original -- もと表記の出力
//
// NOTES
// 文字列変換処理するもと表記部分を output_str に書く。
// _do_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_output_original(ModUnicodeString& output_str) const
{
	for (ModSize i = _stri_left; i < _stri_right; i++) {
#ifdef MOD_NORM_IGNORE_BOTH
		ModUnicodeChar this_char(gen_str.orig_str[i].orig_char);
		if (ModUnicodeCharTrait::find(ignore_both, this_char))
			continue;
		output_str.append(this_char);
#else
		output_str.append(gen_str.orig_str[i].orig_char);
#endif
	}
}

// FUNCTION private
// ModNormalizer::_chk_pre -- 前処理の実施
//
// NOTES
// 前処理（前処理用マップを用いた文字変換）を入力文字列全体に実施する。
// ※ このなかでは seikika_str にセットはしない。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void 
ModNormalizer::_chk_pre()
{
	ModUnicodeChar target, combined, next;
	ModSize except;
	ModSize i;

	for (i=0; i < gen_len;) {
		target = gen_str.orig_str[i].orig_char;
		if (target >= 0xd800 && target <= 0xdbff && i < gen_len - 1) {
			// サロゲートペアの前半が来た
			next = gen_str.orig_str[i+1].orig_char;
			if (next >= 0xdc00 && next <= 0xdfff) {
				// 次の文字がサロゲートペアの後半だった
				// preSurrogateMapでマップした結果を置換文字列とする
				gen_str.orig_str[i].repl_str = _map2(target, next, preSurrogateMap);

#ifdef NORM_DEBUG
				ModDebugMessage << i << ' ' << i+1 << ' '
						   << target << '(' << ModHex << int(target) << ')' << ' '
						   << next   << '(' << ModHex << int(next)   << ')' << ' '
						   << gen_str.orig_str[i].repl_str << ModEndl;
#endif

				i += 2;
				continue;
			}
			// 不正なサロゲートペアに対しては何もしない
		}
		except = 0;
		ModSize j;
		for (j = i + 1; j < gen_len; ++j) {
			next = gen_str.orig_str[j].orig_char;
			if (preMap[next] != 0 && preMap[next] != 0x077f) {
				// 結合文字ではない
				break;
			}
			else if ( (target == 0xFFFE || target == 0xFFFF) && j == i+1){
				// 単独出現のFFFE/FFFFは結合の対象とならない文字
				break;
			}

			// 結合文字があった
			combined = _combine(target, next);
#ifdef NORM_DEBUG
			ModDebugMessage << j << ' '
					   << target << '(' << ModHex << int(target) << ')'
					   << next << '(' << ModHex << int(next) << ')' << '='
					   << combined << '(' << ModHex << int(combined) << ')'
					   << ModEndl;
#endif
			if (combined == 0) {
				if ( preMap[next] == 0){
					// 結合しない -- 結合文字でない文字に現在位置を進める
					++j;
					break;
				}
				else{
					// 結合しない -- 残しておく
					break;
				}
			}
			else if (combined == 0xFFFE || combined ==  0xFFFF) {
				// ２つの結合文字で新しい文字になる可能性がある
				++except;
				target = combined;
			}
			else {
				// 結合する
				except = 0;
				target = combined;
			}
		}
		if (except > 0) {
			// ２つの結合文字で新しい文字になりそこなったので最初の文字に戻す
			; ModAssert(target == 0xFFFE || target ==  0xFFFF);
			target = gen_str.orig_str[i].orig_char;
		}

		// When the expStrData is set,
		// conversion hiragana to katakana is skipped.
		if(expStrMode == ModTrue && ModUnicodeCharTrait::isHiragana(target)){
			gen_str.orig_str[i].repl_str = target;
		} else{
			gen_str.orig_str[i].repl_str = _map(target, preMap, preDecompMap);
		}

		// 削除してはいけない結合文字を結合フラグでなく、元の文字に戻す
		if ( gen_str.orig_str[i].repl_str[0] == 0x077F){
			gen_str.orig_str[i].repl_str = gen_str.orig_str[i].orig_char;
		}

#ifdef NORM_DEBUG
		ModDebugMessage << i << ' ' << j << ' '
				   << gen_str.orig_str[i].orig_char
				   << '(' << ModHex 
				   << int(gen_str.orig_str[i].orig_char) << ')' << ' '
				   << target << '(' << ModHex << int(target) << ')' << ' '
				   << gen_str.orig_str[i].repl_str << ModEndl;
#endif

		i = j;
	}
}

// FUNCTION private
// ModNormalizer::_do_rule -- 文字列変換処理
//
// NOTES
// 文字列の種類（カタカナあるいはASCII）に応じて文字列変換処理を実施する。
// 実施結果は seikika_str にアペンドされる。
//
// ※ この関数は正規化表記出力時のみに呼び出される。
//
// ARGUMENTS
// ModUnicodeString& una_str
//		文字列変換対象
// const ModNormUNAStringType una_type
//		文字列の種類
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_do_rule(ModUnicodeString& una_str,
						const ModNormUNAStringType una_type)
{
	; ModAssert(_out_mode != ModNormBoth);

	if (0 == una_str.getLength())
		return;

	if (una_type == ModNormAlpha) {
		// 英文字列の場合、末尾にダミー文字を付与する
		una_str.append(englishDummyCharacter);
	} else {// ModNormKana
		;
	}

	// 正規化ルールを適用する
	ModUnicodeString result;
	if(expStrMode == ModTrue) {
		result = una_str;
	} else {
		unaRule->getAppInfo(una_str, result);
	}
#ifdef NORM_DEBUG
	ModDebugMessage << una_str << "->" << result << ModEndl;
#endif

	const ModUnicodeChar* _left = (const ModUnicodeChar*)result;
	const ModUnicodeChar* _cntr = _left;
	const ModUnicodeChar* _rite = _left;
	const ModUnicodeChar* _next = result.search(ruleDelimiters[0]);
	const ModUnicodeChar* _last = _left + result.getLength();

	ModUnicodeString una_result_str;
	una_result_str.reallocate(result.getLength());

	// 正規化表記を書く cf. X{A,B}Y から XBY を作成する

	// ルール適用後 { の前までを書く
	for (; _left < _next; _left++) 
		una_result_str.append(*_left);

	while (_next != 0) {
		_left++;
		_cntr = ModUnicodeCharTrait::find(_left, ruleDelimiters[1]);
		_cntr ++;
		_rite = ModUnicodeCharTrait::find(_cntr, ruleDelimiters[2]);
		// ルール適用後 , と } の間を書く
		for (; _cntr < _rite; _cntr++) 
			una_result_str.append(*_cntr);
		_rite++;
		_next = ModUnicodeCharTrait::find(_rite, ruleDelimiters[0]);
		// ルール適用後 { の前までを書く
		for (; _rite < _next; _rite++) 
			una_result_str.append(*_rite);
		_left = _rite;
	}

	// ルール適用後で、} 以降に残った部分を書く
	for (; _left < _last; _left++) 
		una_result_str.append(*_left);

	seikika_str.append(una_result_str);

#ifdef NORM_DEBUG
	ModDebugMessage << una_str << "=>" << una_result_str << ModEndl;
#endif

	_una_do_prn = ModFalse;
	una_str.clear();
}

// FUNCTION private
// ModNormalizer::getNextChar -- 次文字の取得
//
// NOTES
// もとテキストのi番目の文字の前処理した文字列のj番目の文字を次のnullでない
// 文字を返す。
//
// ARGUMENTS
// ModSize i
//		もとテキスト上の文字位置
// ModSize j
//		前処理後の文字位置
//		文字列変換対象
// ModSize repl_len
//		前処理後の文字数
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
ModUnicodeChar
ModNormalizer::getNextChar(ModSize i, ModSize j, ModSize repl_len) const
{
	while (1) {
		if (++j < repl_len) {
			ModUnicodeChar nextChar(gen_str.orig_str[i].repl_str[j]);
			; ModAssert(nextChar != 0);
				// NULL でない文字が見つかった
			return nextChar;
		} else if (++i < gen_len) {
			repl_len = gen_str.orig_str[i].repl_str.getLength();
			j = -1;		// つぎに ++ するので、ここでは 0 でなく -1 とする
		} else {
			// 末尾に達した
			break;
		}
	}
	return 0;
}

// FUNCTION private
// ModNormalizer::_chk_rule -- 文字列変換処理の実施
//
// NOTES
// 文字列変換処理を入力文字列全体に実施する。
// メタ処理を施しながらカタカナあるいはASCIIの連続部分を切り出し、
// 切り出した文字列にルールに基づく文字列変換処理を施す。
// 実施結果は seikika_str にアペンドされる。
//
// ※ この関数は正規化表記出力時のみに呼び出される。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModNormalizer::_chk_rule()
{
	ModNormUNAStringType current_context(ModNormNull);
	_una_str.reallocate(256);	// 領域を確保してしまう（256は適当）

	for (ModSize i(0); i < gen_len; ++i) {
		// もとテキストを１文字ずつ処理する
		ModSize repl_len = gen_str.orig_str[i].repl_str.getLength();
		if (0 == repl_len) {
			// 前処理でNULL文字に置き換えられた場合
			continue;	
		} else {
			// 前処理でNULL文字に置き換えられた以外の場合
			// - 置き換えられた文字ごとに処理する
			for (ModSize j(0); j < repl_len; ++j) {
				ModUnicodeChar this_char = gen_str.orig_str[i].repl_str[j];

				if ((ModUnicodeCharTrait::isKatakana(this_char)) ||
					(ModUnicodeCharTrait::isHankakuKana(this_char)) ) {
					// カタカナの場合
					if (current_context == ModNormAlpha) {
						_do_rule(_una_str, current_context);
						current_context = ModNormNull;
					}
					if (current_context != ModNormKana) {
						// カタカナ処理モード以外の場合
						if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
							// 中点類はそのまま残す
							seikika_str.append(this_char);
							current_context = ModNormNull;
							continue;
						}
						if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
							// ハイフン類はハイフンに統一する
							seikika_str.append(hyphen_meta[0]);
							current_context = ModNormNull;
							continue;
						}
						_una_str.append(this_char);
						current_context = ModNormKana;
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
						// 中点類は、先読みして次の文字を調べる
						// カタカナ処理モード以外では特別扱いは不要
#ifdef MOD_NORM_NAKATEN_CHECK
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar == 0 ||
							(!ModUnicodeCharTrait::isKatakana(nextChar) &&
							 !ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							// 次の文字がないか、カタカナ以外なら中点類は残す
							// （＝次の文字がカタカナなら中点類は削除する）
							_una_str.append(this_char);
						}
#endif
					} else {
						// 中点類以外（長音も含まれる）はそのまま残す
						_una_str.append(this_char);
					}
					continue;
				}
				if (normalizeEnglish == ModTrue &&
					ModUnicodeCharTrait::isAscii(this_char) == ModTrue &&
					ModUnicodeCharTrait::isAlphabet(this_char) == ModTrue ) {
					// 英語処理モードでアルファベットの場合
					if (current_context == ModNormKana) {
						if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
							// 長音類は長音に統一する
							_una_str.append(chouon_meta[0]);
							continue;
						}
						_do_rule(_una_str, current_context);
					}
					if (_una_str.getLength() == 0) {
						// 英文字列の先頭にはダミー文字を付与する
						_una_str.append(englishDummyCharacter);
					}
					_una_str.append(this_char);
					current_context = ModNormAlpha;
					continue;
				} 
				if (current_context == ModNormKana) {
					// カタカナ処理モードの場合
					if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
						// 長音類は長音に統一する
						_una_str.append(chouon_meta[0]);
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
#ifdef MOD_NORM_NAKATEN_CHECK
						// 中点類は次の文字がカタカナであれば削除する
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar != 0 &&
							(ModUnicodeCharTrait::isKatakana(nextChar) ||
							 ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							continue;
						}
#else
						// 中点類は削除する
						continue;
#endif
					}
				}
				if (current_context != ModNormNull) {
					_do_rule(_una_str, current_context);
				}
				if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
					// ハイフン類はハイフンに統一する
					this_char = hyphen_meta[0];
				}
				seikika_str.append(this_char);
				current_context = ModNormNull;
			}
		}
	}

	// 残っている部分があれば文字列変換処理する
	if (current_context != ModNormNull) {
		_do_rule(_una_str, current_context);
	}
}

// FUNCTION private
// ModNormalizer::_chk_post -- 後処理の実施
//
// NOTES
// seikika_str に後処理をした結果を output_str に書く。
// 前処理モードではない場合に呼び出されるので、エスケープを気にしない。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void 
ModNormalizer::_chk_post(ModUnicodeString& output_str)
{
	ModSize tot_repl_len = seikika_str.getLength();

	// 高速化のため、これから書き込む長さ分だけ領域を確保する
	// ただし、後処理により文字数が変わることもあるのでここで確保する長さは
	// 正確ではない
	output_str.reallocate(output_str.getLength() + tot_repl_len);

	for (ModSize i = 0; i < tot_repl_len; i++) {
		output_str.append(_map(seikika_str[i], postMap, postDecompMap));
	}
}

// FUNCTION private
// ModNormalizer::_set_stri_left -- 文字列変換処理対象の開始位置の設定
//
// NOTES
// 文字列変換処理対象の開始位置を内部的に記憶しておく。
//
// ARGUMENTS
// const ModSize stri
//		もとテキストにおける開始位置
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
void
ModNormalizer::_set_stri_left(const ModSize stri)
{
	_stri_left = stri;
	_stri_right = stri;
}

// FUNCTION private
// ModNormalizer::_set_stri_right -- 文字列変換処理対象の終了位置の設定
//
// NOTES
// 文字列変換処理対象の終了位置を内部的に記憶しておく。
//
// ARGUMENTS
// const ModSize stri
//		もとテキストにおける終了位置
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
void
ModNormalizer::_set_stri_right(const ModSize stri)
{
	_stri_right = stri;
}

// FUNCTION
// ModNormalizer::_output_str -- 後処理結果文字列の出力
//
// NOTES
// 後処理結果文字列をoutput_strに書く。
// _do_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModUnicodeChar* mapped_str
//		後処理結果文字列
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void 
ModNormalizer::_output_str(ModUnicodeString& output_str,
						   const ModUnicodeChar* mapped_str) const
{
	for (; *mapped_str != 0; ++mapped_str) {
		if (ModUnicodeCharTrait::find(_both_delims, *mapped_str)) {
			// エスケープする
			output_str.append(_both_escape);
		}
		output_str.append(*mapped_str);
	}
}

// FUNCTION
// ModNormalizer::_output_char_simple -- 前処理で変化しない文字の出力
//
// NOTES
// 前処理モードにおいて、前処理で変化しない文字に対応した出力を
// output_strに書く。
// _chk_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModUnicodeChar this_char
//		もと文字
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_output_char_null(ModUnicodeString& output_str,
								 const ModUnicodeChar this_char) const
{
	if (ModUnicodeCharTrait::find(_both_delims, this_char)) {
		// 区切り文字が含まれる場合 -- エスケープする
		ModUnicodeChar tmp[]
			= { _both_delims[0], _both_escape, this_char,
				_both_delims[1], _both_delims[2], 0 };
		output_str.append(tmp);
	} else {
		// 区切り文字が含まれない場合
		ModUnicodeChar tmp[]
			= { _both_delims[0], this_char,
				_both_delims[1], _both_delims[2], 0 };
		output_str.append(tmp);
	}
}

// FUNCTION
// ModNormalizer::_output_char_simple -- 前処理で変化しない文字の出力
//
// NOTES
// 前処理モードにおいて、前処理で変化しない文字に対応した出力を
// output_strに書く。
// _chk_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModUnicodeChar this_char
//		もと文字
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_output_char_simple(ModUnicodeString& output_str,
								   const ModUnicodeChar this_char)
{
	// 後処理をかける
	const ModUnicodeChar* mapped_str = _map(this_char, postMap, postDecompMap);

	if (ModUnicodeCharTrait::find(_both_delims, this_char)) {
		// 区切り文字が含まれる場合 -- エスケープする
		if (mapped_str[0] == this_char && mapped_str[1] == 0) {
			// 後処理でも文字が変わらない
			ModUnicodeChar tmp[]
				= { _both_delims[0], _both_escape, this_char,
					_both_delims[1], _both_escape, this_char,
					_both_delims[2], 0 };
			output_str.append(tmp);
			return;
		} else {
			ModUnicodeChar tmp[]
				= { _both_delims[0], _both_escape, this_char,
					_both_delims[1], 0 };
			output_str.append(tmp);
		}
	} else {
		// 区切り文字が含まれない場合
		if (mapped_str[0] == this_char && mapped_str[1] == 0) {
			// 後処理でも文字が変わらない
			output_str.append(this_char);
			return;
		} else {
			ModUnicodeChar tmp[]
				= { _both_delims[0], this_char, _both_delims[1], 0 };
			output_str.append(tmp);
		}
	}

	// 後処理後の文字列を書く
	_output_str(output_str, mapped_str);

	output_str.append(_both_delims[2]);
}

// FUNCTION
// ModNormalizer::_output_char -- 前処理で１対１変換される文字の出力
//
// NOTES
// 前処理モードにおいて、前処理で１対１変換される文字に対応した出力を
// output_strに書く。
// _chk_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModUnicodeChar orig_char
//		もと文字
// const ModUnicodeChar out_char
//		前処理結果文字
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_output_char(ModUnicodeString& output_str,
							const ModUnicodeChar orig_char,
							const ModUnicodeChar out_char)
{
	if (ModUnicodeCharTrait::find(_both_delims, orig_char)) {
		// 区切り文字が含まれる場合 -- エスケープする
		ModUnicodeChar tmp[]
			= { _both_delims[0], _both_escape, orig_char,
				_both_delims[1], 0 };
		output_str.append(tmp);
	}
	else {
		// 区切り文字が含まれない場合
		ModUnicodeChar tmp[]
			= { _both_delims[0], orig_char, _both_delims[1], 0 };
		output_str.append(tmp);
	}
	_output_str(output_str, _map(out_char, postMap, postDecompMap));
	output_str.append(_both_delims[2]);
}

// FUNCTION
// ModNormalizer::_output_char -- 前処理で１対Ｎ変換される文字の出力
//
// NOTES
// 前処理モードにおいて、前処理で１対Ｎ変換される文字に対応した出力を
// output_strに書く。
// _chk_both からのみ呼び出される。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModSize orig_char
//		もとテキスト上の文字位置
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModNormalizer::_output_char(ModUnicodeString& output_str,
							const ModSize i)
{
	ModUnicodeChar orig_char = gen_str.orig_str[i].orig_char;
	if (ModUnicodeCharTrait::find(_both_delims, orig_char)) {
		// 区切り文字が含まれる場合 -- エスケープする
		ModUnicodeChar tmp[]
			= { _both_delims[0], _both_escape, orig_char,
				_both_delims[1], 0 };
		output_str.append(tmp);
	}
	else {
		// 区切り文字が含まれない場合
		ModUnicodeChar tmp[]
			= { _both_delims[0], orig_char, _both_delims[1], 0 };
		output_str.append(tmp);
	}
	// 前処理結果文字列の処理
	ModUnicodeString& repl_str = gen_str.orig_str[i].repl_str;
	for (ModSize n(0); n < repl_str.getLength(); ++n) {
		// 後処理をかける
		_output_str(output_str, _map(repl_str[n], postMap, postDecompMap));
	}
	output_str.append(_both_delims[2]);
}

// FUNCTION
// ModNormalizer::_output_post_simple -- 文字列処理で変換された文字の出力
//
// NOTES
// 前処理モードにおいて、文字列処理で変換された文字に後処理を施した結果を
// output_strに書く。
// _do_both からのみ呼び出される。
// 
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// const ModUnicodeChar this_char
//		後処理の対象文字
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void 
ModNormalizer::_output_post_simple(ModUnicodeString& output_str,
								   const ModUnicodeChar this_char)
{
	_output_str(output_str, _map(this_char, postMap, postDecompMap));
}

// FUNCTION private
// ModNormalizer::_do_both -- 前処理モードの文字列変換処理
//
// NOTES
// 前処理モードにおいて、文字列の種類（カタカナあるいはASCII）に応じて
// 文字列変換処理を実施する。
//
// このなかでは seikika_str を用いず、文字列変換処理の後に後処理を実施しながら
// もとテキストとの差分を output_str に直接アペンドする。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
// ModUnicodeString& una_str
//		文字列変換対象
// const ModNormUNAStringType una_type
//		文字列の種類
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_do_both(ModUnicodeString& output_str,
						ModUnicodeString& una_str,
						const ModNormUNAStringType una_type)
{
	; ModAssert(_out_mode == ModNormBoth);

	if (0 == una_str.getLength())
		return;

	if (una_type == ModNormAlpha) {
		// 英文字列の場合、末尾にダミー文字を付与する
		una_str.append(englishDummyCharacter);
	} else {// ModNormKana
		;
	}

	ModUnicodeString result;
	unaRule->getAppInfo(una_str, result);
#ifdef NORM_DEBUG
	ModDebugMessage << una_str << "->" << result << ModEndl;
#endif

	const ModUnicodeChar* _left = (const ModUnicodeChar*)result;
	const ModUnicodeChar* _cntr = _left;
	const ModUnicodeChar* _rite = _left;
	const ModUnicodeChar* _next = result.search(ruleDelimiters[0]);
	const ModUnicodeChar* _last = _left + result.getLength();

	if (_next == 0) {
		ModUnicodeString tmp, tmp2;
		_output_original(tmp);
		for (ModSize n(0); n < una_str.getLength(); n++) 
			_output_post_simple(tmp2, una_str[n]);
		if (tmp != tmp2) {
			// 前処理によって結果が変わっている
			output_str.append(_both_delims[0]);
			output_str.append(tmp);
			output_str.append(_both_delims[1]);
			output_str.append(tmp2);
			output_str.append(_both_delims[2]);
		} else {
			// 前処理によって結果が変わっていない
			// ※ 現状、後処理によってカタカナが変化することはないので、
			//    そのまま出力すれば良い
			output_str.append(tmp);
		}
		una_str.clear();
		return;
	}

	// この場合、ルールによって表記が変わった
	_una_do_prn = ModTrue;

	// "{もと表記," を書く
	output_str.append(_both_delims[0]);
	_output_original(output_str);
	output_str.append(_both_delims[1]);

	// 正規化表記を書く

	// ルール適用後 { の前までを書く
	for (; _left < _next; _left++) 
		_output_post_simple(output_str, *_left);

	while (_next != 0) {
		_left++;
		_cntr = ModUnicodeCharTrait::find(_left, ruleDelimiters[1]);
		_cntr ++;
		_rite = ModUnicodeCharTrait::find(_cntr, ruleDelimiters[2]);
		// ルール適用後 , と } の間を書く
		for (; _cntr < _rite; _cntr++) 
			_output_post_simple(output_str, *_cntr);
		_rite++;
		_next = ModUnicodeCharTrait::find(_rite, ruleDelimiters[0]);
		// ルール適用後 { の前までを書く
		for (; _rite < _next; _rite++) 
			_output_post_simple(output_str, *_rite);
		_left = _rite;
	}

	// ルール適用後で、} 以降に残った部分を書く
	for (; _left < _last; _left++) 
		_output_post_simple(output_str, *_left);

	// "}" を書く
	output_str.append(_both_delims[2]);

	_una_do_prn = ModFalse;
	una_str.clear();
}

// FUNCTION private
// ModNormalizer::_chk_both -- 前処理モードにおける文字列変換処理の実施
//
// NOTES
// 前処理モードにおいて、文字列変換処理を入力文字列全体に実施する。
// メタ処理を施しながらカタカナあるいはASCIIの連続部分を切り出し、
// 切り出した文字列にルールに基づく文字列変換処理を施す。
//
// このなかでは seikika_str を用いず、文字列変換処理の後に後処理を実施しながら
// もとテキストとの差分を output_str に直接アペンドする。
//
// ARGUMENTS
// ModUnicodeString& output_str
//		結果の出力先
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
// 
void
ModNormalizer::_chk_both(ModUnicodeString& output_str)
{
	; ModAssert(_out_mode == ModNormBoth);

	ModNormUNAStringType current_context(ModNormNull);
	ModUnicodeString una_str;
	una_str.reallocate(256);	// 領域を確保する（256は適当）

	_stri_left  = 0;
	_stri_right = 0;
	_una_do_prn = ModFalse;

	ModSize i;
	for (i = 0; i < gen_len; i++) {
		// もとテキストを１文字ずつ処理する
		ModSize repl_len = gen_str.orig_str[i].repl_str.getLength();
		if (0 == repl_len) {
			// 前処理でNULL文字に置き換えられた場合
#ifdef MOD_NORM_IGNORE_BOTH
			if (ModUnicodeCharTrait::find(ignore_both,
										  gen_str.orig_str[i].orig_char))
				continue;
#endif
			if (current_context == ModNormNull)
				_output_char_null(output_str, gen_str.orig_str[i].orig_char); 
			else 
				_una_do_prn = ModTrue;
		} else {
			// 前処理でNULL文字に置き換えられた以外の場合
			// - 置き換えられた文字ごとに処理する
			for (ModSize j = 0; j < repl_len; j++) {
				ModUnicodeChar this_char = gen_str.orig_str[i].repl_str[j];

				if (ModUnicodeCharTrait::isKatakana(this_char) == ModTrue ||
					ModUnicodeCharTrait::isHankakuKana(this_char) == ModTrue) {
					// カタカナの場合
					if ((_una_do_prn == ModFalse) && 
						(ModUnicodeCharTrait::isHankakuKana
						(gen_str.orig_str[i].orig_char)))
							_una_do_prn = ModTrue;
					if (current_context == ModNormAlpha) {
						_set_stri_right(i);
						_do_both(output_str, una_str, current_context);
						current_context = ModNormNull;
					}
					if (current_context != ModNormKana) {
						// カタカナが始まる以前の部分の処理
						if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
							// 中点類はそのまま残す
							if (this_char != gen_str.orig_str[i].orig_char) {
								; ModAssert(j == 0 && repl_len == 1);
								_output_char(output_str,
											 gen_str.orig_str[i].orig_char,
											 this_char);
							} else {
								_output_char_simple(output_str, this_char);
							}
							current_context = ModNormNull;
							continue;
						}
						if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
							// ハイフン類はハイフンに統一する
							this_char = hyphen_meta[0];
							if (this_char != gen_str.orig_str[i].orig_char) {
								; ModAssert(j == 0 && repl_len == 1);
								_output_char(output_str,
											 gen_str.orig_str[i].orig_char,
											 this_char);
							} else {
								_output_char_simple(output_str, this_char);
							}
							current_context = ModNormNull;
							continue;
						}
						_set_stri_left(i);
						una_str.append(this_char);
						current_context = ModNormKana;
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
						// 中点類は、先読みして次の文字を調べる
						// カタカナ処理モード以外では特別扱いは不要
#ifdef MOD_NORM_NAKATEN_CHECK
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar == 0 ||
							(!ModUnicodeCharTrait::isKatakana(nextChar) &&
							 !ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							// 次の文字がないか、カタカナ以外なら中点類は残す
							// （＝次の文字がカタカナなら中点類は削除する）
							una_str.append(this_char);
						} else {
							_una_do_prn = ModTrue;
						}
#else
						_una_do_prn = ModTrue;
#endif
					} else {
						// 中点類以外（長音も含まれる）はそのまま残す
						una_str.append(this_char);
					}
					continue;
				}
				if (normalizeEnglish == ModTrue &&
					ModUnicodeCharTrait::isAscii(this_char) &&
					ModUnicodeCharTrait::isAlphabet(this_char)) {
					// 英語処理モードでアルファベットの場合
					if ((_una_do_prn == ModFalse) && 
						(!((ModUnicodeCharTrait::isAscii
							(gen_str.orig_str[i].orig_char)) &&
						   (ModUnicodeCharTrait::isLower
							(gen_str.orig_str[i].orig_char)) )))
						_una_do_prn = ModTrue;
					if (current_context == ModNormKana) {
						if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
							// 長音類は長音に統一する
							una_str.append(chouon_meta[0]);
							continue;
						}
						_set_stri_right(i);
						_do_both(output_str, una_str, current_context);
					}
					if (current_context != ModNormAlpha) {
						_set_stri_left(i);		// これでよい???
					}
					if (una_str.getLength() == 0) {
						// 英文字列の先頭にはダミー文字を付与する
						una_str.append(englishDummyCharacter);
					}
					una_str.append(this_char);
					current_context = ModNormAlpha;
					continue;
				} 
				if (current_context == ModNormKana) {
					// カタカナ処理モードの場合
					if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
						// 長音類は長音に統一する
						una_str.append(chouon_meta[0]);
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
#ifdef MOD_NORM_NAKATEN_CHECK
						// 中点類は次の文字がカタカナであれば削除する
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar != 0 &&
							(ModUnicodeCharTrait::isKatakana(nextChar) ||
							 ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							continue;
						}
#else
						// 中点類は削除する
						continue;
#endif
					}
				}
				if (current_context != ModNormNull) {
					if (_out_mode == ModNormBoth) {
						_set_stri_right(i);
					}
					_do_both(output_str, una_str, current_context);
				}
				if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
					// ハイフン類はハイフンに統一する
					; ModAssert(repl_len == 1);
					if (this_char != gen_str.orig_str[i].orig_char) {
						_output_char(output_str,
									 gen_str.orig_str[i].orig_char, this_char);
					} else {
						_output_char_simple(output_str, this_char);
					}
				} else {
					if (this_char != gen_str.orig_str[i].orig_char) {
						if (j == 0) {
							_output_char(output_str, i);
						}
					} else {
						_output_char_simple(output_str, this_char);
					}
				}
				current_context = ModNormNull;
			}
		}
	}

	// 残っている部分があれば文字列変換処理する
	if (current_context != ModNormNull) {
		if (_out_mode == ModNormBoth) {
			_set_stri_right(i);
		}
		_do_both(output_str, una_str, current_context);
	}
}

// FUNCTION private
// ModNormalizer::_isBreakpoint -- 分割可能な文字かの判定
//
// NOTES
// 与えられた文字が分割可能な文字か判定する。
//
// ARGUMENTS
// const ModUnicodeChar this_char
//		判定対象文字
//
// RETURN
// 分割可能な文字ならば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
ModBoolean 
ModNormalizer::_isBreakpoint(const ModUnicodeChar this_char) const
{
	return ModBoolean(
		(ModUnicodeCharTrait::isKanji (this_char)) ||
		(ModUnicodeCharTrait::isControl (this_char)) ||
		(ModUnicodeCharTrait::isSpace (this_char)) ||
		(ModUnicodeCharTrait::isLine (this_char)) ||
		(ModUnicodeCharTrait::isDigit (this_char)) );
}

// FUNCTION private
// ModNormalizer::_find_clip_point -- 分割位置の探索
//
// NOTES
// 文書の分割位置を見つける。
//
// ARGUMENTS
// const ModUnicodeString& input_str
//		もとテキスト
// const ModSize begin_index
//		探索開始位置
// const ModSize end_index
//		探索終了位置（この位置は含まない）
//
// RETURN
// 分割位置（先頭からの文字数）
//
// EXCEPTIONS
// なし
// 

ModSize
ModNormalizer::_find_clip_point(const ModUnicodeString& input_str, 
								const ModSize begin_index,
								const ModSize end_index)  const
{
	ModSize final_check(begin_index + max_nbuf_len);
	for (ModSize i(final_check); i < end_index; ++i) {
		if (_isBreakpoint(input_str[i])) {
			return i;
		}
	}
	return end_index;
}


// FUNCTION private
// ModNormalizer::_normalizeInPieces
//
// NOTES
// 文書を分割し、その部分を正規化する。
//
// ARGUMENTS
// const ModUnicodeString& input_str
//		原表記である対象文字列を含む Unicode 文字列
// ModUnicodeString& output_str
//		結果の出力先
// ModSize begin_index
//		input_str中の対象文字列の開始位置
// ModSize end_index
//		input_str中の対象文字列の終了位置（この位置は含まない）
//		begin_indexとend_indexが 0 または指定されないとき、
//		文字列全体を対象とする
// ModNormOutMode out_mode
//		出力モード
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModNormalizer::_normalizeInPieces(const ModUnicodeString& input_str,
								  ModUnicodeString& output_str,
								  const ModSize begin_index,
								  const ModSize end_index,
								  const ModNormOutMode out_mode)
{
	ModSize last_clipped_at, piece_start;
	piece_start = last_clipped_at = begin_index; 

	while (piece_start < end_index) {

		last_clipped_at = _find_clip_point(input_str, piece_start, end_index);

		gen_str.clear();
		gen_len = last_clipped_at - piece_start;
		gen_str = input_str.copy(piece_start, gen_len);

		_chk_pre();

		if (_out_mode == ModNormBoth) {
			// seikika_str は使わない
			_chk_both(output_str);

		} else {
			seikika_str.clear();
			seikika_str.reallocate(gen_len);

			_chk_rule();
			_chk_post(output_str);
		}
		piece_start = last_clipped_at;
	}
}

//  正規化 public

// FUNCTION public
// ModNormalizer::normalizeBuf -- 異表記を正規化する
//
// NOTES
// 与えられた文字列の対象部分をコピーし、
// 正規化規則と文字交換マップに従って、正規化する。
// 出力モードによって、前処理結果または正規化結果を返す。
// 区切り文字・エスケープ文字は異なっており、かつカタカナでないこと。
//
// 入力文がmax_normbuf_len字以上なら、漢字や空白でこの長さ以下に分割し、
// 部分毎に正規化する。
// (デフォルトは、 100000である。実験によれば、より少なくしても、
// 瞬間メモリ使用量はあまり減らなかったためである。)
//
// ※ 呼び出す前、規則と正規化の初期化が必要
//
// ARGUMENTS
// const ModUnicodeString& input_str
//		原表記である対象文字列を含む Unicode 文字列
// ModSize begin_index
//		input_str中の対象文字列の開始位置
// ModSize end_index
//		input_str中の対象文字列の終了位置（この位置は含まない）
//		begin_indexとend_indexが 0 または指定されないとき、
//		文字列全体を対象とする
// ModNormOutMode out_mode
//		出力モード(デフォルトが ModNormBoth)
// ModUnicodeChar d1, d2, d3, d4
//		ModNormBothのためのデリミター文字(３種類)とエスケープ文字
// ModSize max_normbuf_len
//		分割長
//
// RETURN
// ModUnicodeString
//		正規化結果のUnicode文字列。出力モードによって、形式が次のように異なる。
//		ModNormBothの場合、デリミターを区切りとして原表記と正規化表記を含む文字列
//		ModNormalizedの場合、正規化表記
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		区切り文字・エスケープ文字に同じものがある、あるいはカタカナが含まれる
//		out_modeにオリジナルを指定している
//		end_index, begin_index が不適切である
// その他、下位からの例外をそのまま返す
//
void
ModNormalizer::normalizeBuf(const ModUnicodeString& input_str,
							ModUnicodeString& output_str,
							const ModSize begin_index,
							const ModSize end_index,
							const ModNormOutMode out_mode,
							const ModUnicodeChar d1,
							const ModUnicodeChar d2,
							const ModUnicodeChar d3,
							const ModUnicodeChar d4,
							const ModSize max_normbuf_len)
{
	if (d1 == d2 || d1 == d3 || d1 == d4 || d2 == d3 || d2 == d4 || d3 == d4 ||
		ModUnicodeCharTrait::isKatakana(d1) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d2) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d3) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d4) == ModTrue) {
		// 区切り文字・エスケープ文字は異なっており、カタカナでないこと
		ModErrorMessage << "Invalid terminater/escape: "
						<< d1 << ' ' << d2 << ' ' << d3 << ' ' << d4 << ModEndl;
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	if (out_mode == ModNormOriginal) {
		// オリジナルを指定してはいけない
#ifdef DEBUG
		ModErrorMessage << "Invalid output mode: " << int(out_mode) << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	// 処理すべき文字列の長さを求める
	ModSize input_len(input_str.getLength());
	if (end_index == 0) {
		if (input_len > 0 && input_len <= begin_index) {
			ModErrorMessage << "Invalid begin_index: " << begin_index << ' '
							<< input_len << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		gen_len = input_len - begin_index;
	} else {
		if (end_index < begin_index || input_len < end_index) {
			ModErrorMessage << "Invalid end_index: " << begin_index << ' '
							<< end_index << ' ' << input_len << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		if (end_index == begin_index) {
			// 開始位置と終了位置が同じ場合、空文字列が対象となる。
			// 空文字列の正規化結果は空文字列なので、何もしなくて良い
			return;
		}
		gen_len = end_index - begin_index;
	}

	// set the modes and init
	_out_mode = out_mode;
	_both_delims[0] = d1;
	_both_delims[1] = d2;
	_both_delims[2] = d3;
	_both_delims[3] = d4;
	_both_escape = d4;

	_stri_left  = 0;
	_stri_right = 0;
	_una_do_prn = ModFalse;

	// 出力先の領域確保
	//   ModNormBoth でない場合は _chk_post のなかで呼ばれる
	output_str.setAllocateStep(64);
	if (_out_mode == ModNormBoth) {
		output_str.reallocate(ModSize(ModNormBothBonusFactor*gen_len));
	}

	//  check to see if we have to sub-divide
	max_nbuf_len = max_normbuf_len;
	if (gen_len > max_nbuf_len) {
		// テキストが大きいので分割して処理する
		_normalizeInPieces(input_str, output_str,
						   begin_index, begin_index + gen_len, out_mode);

	} else {
		// テキストが小さいのでまとめて処理する
		gen_str.clear();
		if (begin_index == 0 && end_index == 0) {
			gen_str = input_str;
		} else {
			gen_str = input_str.copy(begin_index, gen_len);
		}
		; ModAssert(gen_len == gen_str.getLength());

		// 前処理
		_chk_pre();						// gen_str に前処理結果が反映される

		if (_out_mode == ModNormBoth) {
			// seikika_str は使わない
			_chk_both(output_str);
		} else {
			seikika_str.clear();
			seikika_str.reallocate(gen_len);
			_chk_rule();				// seikika_str に中間結果をセットする
			_chk_post(output_str);		// seikika_str から output_str に
										// 最終結果をセットする
		}
	}
}

ModUnicodeString
ModNormalizer::normalizeBuf(const ModUnicodeString& input_str, 
							const ModSize begin_index,
							const ModSize end_index,
							const ModNormOutMode out_mode,
							const ModUnicodeChar d1,
							const ModUnicodeChar d2,
							const ModUnicodeChar d3,
							const ModUnicodeChar d4,
							const ModSize max_normbuf_len)
{
	ModUnicodeString output_str;
	normalizeBuf(input_str, output_str,
				 begin_index, end_index, out_mode, d1, d2, d3, d4,
				 max_normbuf_len);
	return output_str;
}

//
//  展開 private
//

// FUNCTION private
// ModNormalizer::_do_exp -- 展開モードの文字列変換処理
//
// NOTES
// 展開モードにおいて、文字列の種類（カタカナあるいはASCII）に応じて
// 文字列変換処理を実施する。
// 実施結果は seikika_str にアペンドされる。
//
// ※ この関数は正規化表記出力時のみに呼び出される。
//
// ARGUMENTS
// ModUnicodeString& una_str
//		文字列変換対象
// const ModNormUNAStringType una_type
//		文字列の種類
// const ModBoolean skipFirstStep
//		正規化処理をスキップする
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModNormalizer::_do_exp(ModUnicodeString& una_str,
					   const ModNormUNAStringType una_type,
					   const ModBoolean skipFirstStep)
{
	if (0 == una_str.getLength())
		return;

	if (una_type == ModNormAlpha) {
		// 英文字列の場合、末尾にダミー文字を付与する
		una_str.append(englishDummyCharacter);
	} else {// ModNormKana
		;
	}

	ModUnicodeString una_rule_result_str, result;

	if (skipFirstStep == ModTrue) {
		// 正規化ルールをスキップする
		una_rule_result_str = una_str;

	} else {
		// 正規化ルールを適用する
		unaRule->getAppInfo(una_str, result);
#ifdef NORM_DEBUG
		ModDebugMessage << una_str << "->" << result << ModEndl;
#endif

		// 正規化結果から正規化文字列を作成する
		// cf. X{A,B}Y から XBY を作成する
		const ModUnicodeChar* _left = (const ModUnicodeChar*)result;
		const ModUnicodeChar* _cntr = _left;
		const ModUnicodeChar* _rite = _left;
		const ModUnicodeChar* _next = result.search(ruleDelimiters[0]);
		const ModUnicodeChar* _last = _left + result.getLength();

		una_rule_result_str.reallocate(result.getLength());

		if (una_type == ModNormAlpha) {
			una_rule_result_str = englishDummyCharacter;
		}

		for (; _left < _next; _left++) 
			una_rule_result_str.append(*_left);

		while (_next != 0) {
			_left++;
			_cntr = ModUnicodeCharTrait::find(_left, ruleDelimiters[1]);
			_cntr ++;
			_rite = ModUnicodeCharTrait::find(_cntr, ruleDelimiters[2]);
			for (; _cntr < _rite; _cntr++) 
				una_rule_result_str.append(*_cntr);
			_rite++;
			_next = ModUnicodeCharTrait::find(_rite, ruleDelimiters[0]);
			for (; _rite < _next; _rite++) 
				una_rule_result_str.append(*_rite);
			_left = _rite;
		}

		for (; _left < _last; _left++) 
			una_rule_result_str.append(*_left);

		if (una_type == ModNormAlpha) {
			una_rule_result_str.append(englishDummyCharacter);
		}
	}

	// 正規化文字列ができたので展開ルールを適用する
	unaExpand->getAppInfo(una_rule_result_str, result);
#ifdef NORM_DEBUG
	ModDebugMessage << una_rule_result_str << "=>" << result << ModEndl;
#endif

	// 同様の処理を３回行うのは速度的に問題だが、単純なループを回すだけなので
	// コードがシンプルになるように、以下のままとする
	result.replace(expandDelimiters[0], ModNormDefaultDelimiter0);
	result.replace(expandDelimiters[1], ModNormDefaultDelimiter1);
	result.replace(expandDelimiters[2], ModNormDefaultDelimiter2);

	seikika_str.append(result);

	una_str.clear();
}

// FUNCTION private
// ModNormalizer::_chk_exp -- 展開モードにおける文字列変換処理の実施
//
// NOTES
// 展開モードにおいて、文字列変換処理を入力文字列全体に実施する。
// メタ処理を施しながらカタカナあるいはASCIIの連続部分を切り出し、
// 切り出した文字列にルールに基づく文字列変換処理を施す。
// 実施結果は seikika_str にアペンドされる。
//
// ARGUMENTS
// const ModBoolean skipFirstStep
//		正規化処理をスキップする
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void 
ModNormalizer::_chk_exp(const ModBoolean skipFirstStep)
{
	ModNormUNAStringType current_context(ModNormNull);
	ModUnicodeString una_str;
	una_str.reallocate(256);	// 領域を確保する（256は適当）

	for (ModSize i = 0; i < gen_len; i++) {
		// もとテキストを１文字ずつ処理する
		ModSize repl_len = gen_str.orig_str[i].repl_str.getLength();
		if (0 == repl_len) {	
			// 前処理でNULL文字に置き換えられた場合
			continue;	
		} else {
			// 前処理でNULL文字に置き換えられた以外の場合
			// - 置き換えられた文字ごとに処理する
			for (ModSize j = 0; j < repl_len; j++) {
				ModUnicodeChar this_char = gen_str.orig_str[i].repl_str[j];

				if (ModUnicodeCharTrait::isKatakana(this_char) == ModTrue ||
					ModUnicodeCharTrait::isHankakuKana(this_char) == ModTrue) {
					// カタカナの場合
					if (current_context == ModNormAlpha) {
						_do_exp(una_str, current_context, skipFirstStep);
						current_context = ModNormNull;
					}
					if (current_context != ModNormKana) {
						// カタカナ処理モード以外の場合
						if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
							// 中点類はそのまま残す
							seikika_str.append(this_char);
							current_context = ModNormNull;
							continue;
						}
						if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
							// ハイフン類はハイフンに統一する
							seikika_str.append(hyphen_meta[0]);
							current_context = ModNormNull;
							continue;
						}
						una_str.append(this_char);
						current_context = ModNormKana;
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
						// 中点類は、先読みして次の文字を調べる
						// カタカナ処理モード以外では特別扱いは不要
#ifdef MOD_NORM_NAKATEN_CHECK
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar == 0 ||
							(!ModUnicodeCharTrait::isKatakana(nextChar) &&
							 !ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							// 次の文字がないか、カタカナ以外なら中点類は残す
							// （＝次の文字がカタカナなら中点類は削除する）
							una_str.append(this_char);
						}
#endif
					} else {
						// 中点類以外（長音も含まれる）はそのまま残す
						una_str.append(this_char);
					}
					continue;
				}
				if (normalizeEnglish == ModTrue &&
					ModUnicodeCharTrait::isAscii(this_char) == ModTrue &&
					ModUnicodeCharTrait::isAlphabet(this_char) == ModTrue) {
					// 英語処理モードでアルファベットの場合
					if (current_context == ModNormKana) {
						if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
							// 長音類は長音に統一する
							una_str.append(chouon_meta[0]);
							continue;
						}
						_do_exp(una_str, current_context, skipFirstStep);
					}
					if (una_str.getLength() == 0) {
						// 英文字列の先頭にはダミー文字を付与する
						una_str.append(englishDummyCharacter);
					}
					una_str.append(this_char);
					current_context = ModNormAlpha;
					continue;
				} 
				if (current_context == ModNormKana) {
					// カタカナ処理モードの場合
					if (ModUnicodeCharTrait::find(chouon_meta, this_char)) {
						// 長音類は長音に統一する
						una_str.append(chouon_meta[0]);
						continue;
					}
					if (ModUnicodeCharTrait::find(nakaten_meta, this_char)) {
#ifdef MOD_NORM_NAKATEN_CHECK
						// 中点類は次の文字がカタカナであれば削除する
						ModUnicodeChar nextChar(getNextChar(i, j, repl_len));
						if (nextChar != 0 &&
							(ModUnicodeCharTrait::isKatakana(nextChar) ||
							 ModUnicodeCharTrait::isHankakuKana(nextChar)
								)) {
							continue;
						}
#else
						// 中点類は削除する
						continue;
#endif
					}
				}
				if (current_context != ModNormNull) {
					_do_exp(una_str, current_context, skipFirstStep);
				}
				if (ModUnicodeCharTrait::find(hyphen_meta, this_char)) {
					// ハイフン類はハイフンに統一する
					this_char = hyphen_meta[0];
				}
				seikika_str.append(this_char);
				current_context = ModNormNull;
			}
		}
	}

	// 残っている部分があれば文字列変換処理する
	if (current_context != ModNormNull) {
		_do_exp(una_str, current_context, skipFirstStep);
	}
}

// FUNCTION private
// ModNormalizer::_expand_array -- 展開パターンを含む正規化文字列の展開
//
// NOTES
// 展開パターンを含む正規化文字列から、それらを組み合わせた正規化文字列の
// 配列を生成する。
// たとえば、"(アメリカ,アメリカン)コウヒ" から以下の文字列を生成する。
//		アメリカコウヒ
//		アメリカンコウヒ
//
// ARGUMENTS
// const ModUnicodeString& tmp_str
//		展開パターンを含む文字列
// ModUnicodeString*& expanded
//		展開結果をセットする配列
//
// RETURN
// 展開数
//
// EXCEPTIONS
// その他、下位からの例外をそのまま返す
// 
ModSize 
ModNormalizer::_expand_array(const ModUnicodeString& tmp_str,
							 ModUnicodeString*& expanded)
{
	ModSize exp_cnt(1);
	
	const ModUnicodeChar* _left = (const ModUnicodeChar*)tmp_str;
	const ModUnicodeChar* _cntr = _left;
	const ModUnicodeChar* _rite = _left;
	const ModUnicodeChar* _next = tmp_str.search(ModNormDefaultDelimiter0);
	const ModUnicodeChar* _last = _left + tmp_str.getLength();

	ModUnicodeString* wd_array = 0;
	ModUnicodeString* oldexpanded = 0;
	expanded = 0;
	expanded = new ModUnicodeString[exp_cnt];

	try
	{
		if (_next == 0)  {
			expanded[0] = tmp_str;
			return exp_cnt;
		}

		for (; _left < _next; _left++) 
			expanded[0].append(*_left);

		ModSize i,j, x;
		ModUnicodeString int_wd;
		while (_next != 0) {
			_left = _next;
			_cntr = ModUnicodeCharTrait::find(_left, ModNormDefaultDelimiter1);
			_rite = ModUnicodeCharTrait::find(_left, ModNormDefaultDelimiter2);
			ModSize exp_wd_cnt = 1;
			while ((_cntr != 0) && (_cntr < _rite)) {
				exp_wd_cnt++;
				_cntr++;
				_cntr = ModUnicodeCharTrait::find(_cntr, ModNormDefaultDelimiter1);
			}
			wd_array = new ModUnicodeString[exp_wd_cnt];

			_cntr = ModUnicodeCharTrait::find(_left, ModNormDefaultDelimiter1);
			for (i = 0; i < exp_wd_cnt; i++) {
				_left++;
				for (; _left < _cntr; _left++) 
					wd_array[i].append(*_left);
				_cntr++;
				_cntr = ModUnicodeCharTrait::find(_cntr, ModNormDefaultDelimiter1);
				if ((0 == _cntr) ||(_cntr > _rite))
					_cntr = _rite;
			}
				
			ModSize new_exp_cnt = exp_cnt * exp_wd_cnt;
			oldexpanded = expanded;
			expanded = new ModUnicodeString[new_exp_cnt];

			for (i = 0; i < exp_cnt; i++) {
				for (j = 0; j < exp_wd_cnt; j++) {
					x = i*exp_wd_cnt + j;
					expanded[x].clear();
					expanded[x].append(oldexpanded[i]);
					expanded[x].append(wd_array[j]);
				}
			}
			exp_cnt = new_exp_cnt;
			delete [] oldexpanded, oldexpanded = 0;
			delete [] wd_array, wd_array = 0;

			_next = ModUnicodeCharTrait::find(_rite, ModNormDefaultDelimiter0);
			_rite ++;

			int_wd.clear();
			for (; _rite < _next; _rite++) 
				int_wd.append(*_rite);
			for (i = 0; i < exp_cnt; i++) 
				expanded[i].append(int_wd);
		}
		int_wd.clear();
		for (; _rite < _last; _rite++) 
			int_wd.append(*_rite);
		for (i = 0; i < exp_cnt; i++) 
			expanded[i].append(int_wd);
	}
	catch(...)
	{
		delete [] expanded, expanded = 0;
		delete [] oldexpanded, oldexpanded = 0;
		delete [] wd_array, wd_array = 0;
		throw;
	}

	return exp_cnt;
}

// FUNCTION private
// ModNormalizer::_exp_chk_str -- 展開された正規化文字列間の包含関係検査
//
// NOTES
// 展開された正規化文字列間の包含関係を検査し、他の正規化文字列を包含する
// ものは削除する。
// たとえば、"アメリカ", "アメリカン" が展開文字列であれば、後者は前者を包含
// するので削除し、"アメリカ" だけにする。
//
// ARGUMENTS
// ModUnicodeString*& expanded
//		展開結果をセットする配列
// const ModSize exp_cnt
//		展開数
//
// RETURN
// 検査後の展開数
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModSize 
ModNormalizer::_exp_chk_str(ModUnicodeString*& expanded, ModSize exp_cnt)
{
	ModSize i, j;

	for (i = 0; i < exp_cnt; i++) {
		if (0 == expanded[i].getLength()) 
			continue;
		for (j = i + 1; j < exp_cnt; j++) {
			if (0 == expanded[j].getLength()) 
				continue;
			if (0 != expanded[i].search(expanded[j])) 
				expanded[i].clear();
			else if (0 != expanded[j].search(expanded[i])) 
				expanded[j].clear();
		}
	}
	ModSize new_cnt = 0;
	for (i = 0; i < exp_cnt; i++) {
		if (0 == expanded[i].getLength()) 
			continue;
		if (new_cnt == i) {
			new_cnt ++; 
			continue;
		}
		expanded[new_cnt] = expanded[i];
		new_cnt ++;
	}
	
	exp_cnt = new_cnt;
	
	return exp_cnt;
}

// FUNCTION public
// ModNormalizer::expandBuf -- 異表記を正規化し、展開する
//
// NOTES
// 与えられた文字列の対象部分をコピーし、
// normalizeBuf()と同様に正規化する。
// 正規化結果を展開規則に従って、展開する。
// 展開モードによって、展開結果を省略できる。
// ModNormExpChkOrigStrの場合、別の展開文字列を含む展開文字列を省略する。
// ModNormExpNoChkの場合、省略しない。
//
// 呼び出す前、規則と正規化の初期化が必要
// 呼ぶ側に、delete [] expanded;をしないと、メモリリークが発生する。
//
// ARGUMENTS
// const ModUnicodeString& input_str
//		原表記である対象文字列を含む Unicode 文字列
// ModUnicodeString*& expanded
//		展開結果を返すために、結果を格納する変数
// ModNormExpMode chkorg
//		展開モード
// ModSize begin_index
//		input_str中の対象文字列の開始位置
// ModSize end_index
//		input_str中の対象文字列の終了位置（この位置は含まない）
//		begin_indexとend_indexが 0 または指定されないとき、
//		文字列全体を対象とする
// const ModBoolean expandOnly
//		正規化結果が渡されることを前提に異表記展開のみを行う
//
// RETURN
// expanded配列の要素数
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		end_index, begin_index が不適切である
// その他、下位からの例外をそのまま返す
// 
ModSize 
ModNormalizer::expandBuf(const ModUnicodeString& input_str,
						 ModUnicodeString*& expanded, 
						 const ModNormExpMode chkorg,
						 const ModSize begin_index,
						 const ModSize end_index,
						 const ModBoolean expandOnly)
{
	// 処理すべき文字列の長さを求める
	//
	ModSize input_len(input_str.getLength());
	if (end_index == 0) {
		if (input_len > 0 && input_len <= begin_index) {
			ModErrorMessage << "Invalid begin_index: " << begin_index << ' '
							<< input_len << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		gen_len = input_len - begin_index;
	} else {
		if (end_index < begin_index || input_len < end_index) {
			ModErrorMessage << "Invalid end_index: " << begin_index << ' '
							<< end_index << ' ' << input_len << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		if (end_index == begin_index) {
			// 開始位置と終了位置が同じ場合、空文字列が対象となる。
			// 空文字列の正規化結果は空文字列なので、何もしなくて良い
			expanded = 0;
			return 0;
		}
		gen_len = end_index - begin_index;
	}

	gen_str.clear();
	if (begin_index == 0 && end_index == 0) {
		gen_str = input_str;
	} else {
		gen_str = input_str.copy(begin_index, gen_len);
	}
	; ModAssert(gen_len == gen_str.getLength());

	// set return text;
	seikika_str.clear();
	seikika_str.reallocate(gen_len);

	// set the modes and init
	_out_mode = ModNormalized;
	_una_do_prn = ModFalse;

	if (expandOnly == ModFalse) {
		// まず入力文字列を正規化する
		// do the real work (w/ private-use chars)
		_chk_pre();						// gen_str に前処理結果が反映される
		_chk_exp(ModFalse);				// seikika_str に中間結果をセットする

	} else {
		// 入力文字列が正規化済みの場合
		// gen_str の repl_str に正規化済みのオリジナルをセットする
		for (ModSize i(0); i < gen_len; i++) {
			gen_str.orig_str[i].repl_str = gen_str.orig_str[i].orig_char;
		}
		_chk_exp(ModTrue);				// seikika_str に中間結果をセットする
	}

	ModUnicodeString tmp_str;
	tmp_str.reallocate(gen_len);
	_chk_post(tmp_str);					// seikika_str から output_str に
										// 最終結果をセットする

	// 展開する
	ModSize exp_cnt = _expand_array(tmp_str, expanded); 

	if (chkorg == ModNormExpChkOrigStr) {
		// 展開文字列間の包含関係をチェックする場合
		exp_cnt = _exp_chk_str(expanded, exp_cnt);
		if ((0 == exp_cnt) ||
			((1 == exp_cnt) &&
			 (0 != expanded[0].search(input_str.copy(begin_index, gen_len))))) {
			// チェックの結果、展開数が1でオリジナルに包含される場合は
			// 展開を行わないように展開数を0とする（？）
			// _exp_chk_strが0を返すことがあるのか？
			delete [] expanded; 
			expanded = 0;
			exp_cnt = 0;
		}
	}

	return exp_cnt;
}

// FUNCTION public
// ModNormalizer::extractInit -- 抽出対象文字列を設定する
//
// NOTES
// 与えられた文字列によって、抽出の初期化を行なう。
//
// ARGUMENTS
// const ModUnicodeString& input_str
//		前処理結果である対象文字列を含む Unicode 文字列
// ModNormOutMode out_mode
//		出力モード(デフォルトが ModNormalized)
// ModUnicodeChar d1, d2, d3, d4
//		ModNormBothのためのデリミター文字(３種類)とエスケープ文字
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		区切り文字・エスケープ文字に同じものがある、あるいはカタカナが含まれる
// 

void 
ModNormalizer::extractInit(const ModUnicodeString& input_str,
						   const ModNormOutMode out_mode,
						   const ModUnicodeChar d1,
						   const ModUnicodeChar d2,
						   const ModUnicodeChar d3,
						   const ModUnicodeChar d4)
{
	if (d1 == d2 || d1 == d3 || d1 == d4 || d2 == d3 || d2 == d4 || d3 == d4 ||
		ModUnicodeCharTrait::isKatakana(d1) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d2) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d3) == ModTrue ||
		ModUnicodeCharTrait::isKatakana(d4) == ModTrue) {
		// 区切り文字・エスケープ文字は異なっており、カタカナでないこと
		ModErrorMessage << "Invalid terminater/escape: "
						<< d1 << ' ' << d2 << ' ' << d3 << ' ' << d4 << ModEndl;
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	_out_mode = out_mode;

	_both_delims[0] = d1;
	_both_delims[1] = d2;
	_both_delims[2] = d3;
	_both_delims[3] = d4;
	_both_escape = d4;

	extr_txt_ptr = (const ModUnicodeChar*)input_str;
	extr_stat = ModNormBoth;
}

// FUNCTION public
// ModNormalizer::extractGetc -- 前処理結果から表記を一字ずつ抽出する
//
// NOTES
//   normalizeBuf()で得られた前処理結果から、
//   原表記または正規化表記を一字ずつ抽出し、返す。
//
// ARGUMENTS
// なし
//
// RETURN
// ModUnicodeChar -- 抽出された文字
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//		対象文字列が設定されていない場合
//
ModUnicodeChar ModNormalizer::extractGetc()
{
	if (0 == extr_txt_ptr) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorNotInitialized, ModErrorLevelError);
	}

	extr_ret = *extr_txt_ptr;

	if (_out_mode != ModNormBoth) {
		ModBoolean isEscaped(ModFalse);
		while ((extr_ret != 0) &&
			   ((ModUnicodeCharTrait::find(_both_delims, extr_ret)) ||
				((extr_stat != ModNormBoth) && (extr_stat != _out_mode)))) {
			if (isEscaped == ModTrue) {
				// エスケープ状態をもとに戻す
				if (extr_stat == _out_mode)
					break;
				isEscaped = ModFalse;
			} else if ((extr_ret == _both_escape) &&
					   (extr_stat != ModNormBoth)) {
				// 区切りのなかにエスケープ文字があったので、エスケープ状態にする
				isEscaped = ModTrue;
			} else if ((extr_ret == _both_delims[0]) &&
					   (extr_stat == ModNormBoth)) {
				extr_stat = ModNormOriginal;
			} else if ((extr_ret == _both_delims[1]) &&
					   (extr_stat == ModNormOriginal)) {
				extr_stat = ModNormalized;
			} else if ((extr_ret == _both_delims[2]) &&
					   (extr_stat == ModNormalized)) {
				extr_stat = ModNormBoth;
			} else if (ModUnicodeCharTrait::find(_both_delims, extr_ret)) {
				break;
			}
			extr_txt_ptr ++;
			extr_ret = *extr_txt_ptr;
		}
	}

	extr_txt_ptr ++;
	if (extr_ret == 0)
		extr_txt_ptr = 0;

	return extr_ret;
}

//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
