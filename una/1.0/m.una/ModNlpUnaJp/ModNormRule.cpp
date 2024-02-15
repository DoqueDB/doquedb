// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormRule.cpp -- ModNormRule のクラス定義
// 
// Copyright (c) 2000-2012, 2023 Ricoh Company, Ltd.
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

#include "ModFile.h"
#include "ModArchive.h"
#include "ModAutoPointer.h"
#include "ModCharString.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormString.h"
#include "ModNlpUnaJp/ModNormalizer.h"
#include "ModNlpUnaJp/unakapi.h"

#include <stdlib.h>

_UNA_USING
_UNA_UNAJP_USING

//
// CONST
// ModNormDataLineMax::ruleDicName -- マップファイルの１行の長さの最大
//
// NOTES
// マップファイルの１行の長さの最大。
//
const ModSize ModNormDataLineMax = 256;

//
// CONST
// ModNormRule::ruleDicName --ルール辞書ファイル名
//
// NOTES
// データディレクトリにおけるデフォルトのファイル名。
//
/*static*/ const ModUnicodeString ModNormRule::ruleDicName("ruleWrd.dic");
/*static*/ const ModUnicodeString ModNormRule::ruleAppName("ruleApp.dic");
/*static*/ const ModUnicodeString ModNormRule::expandDicName("expWrd.dic");
/*static*/ const ModUnicodeString ModNormRule::expandAppName("expApp.dic");
/*static*/ const ModUnicodeString ModNormRule::connectTblName("connect.tbl");
/*static*/ const ModUnicodeString ModNormRule::unknownTblName("unkmk.tbl");
/*static*/ const ModUnicodeString ModNormRule::unknownCostName("unkcost.tbl");
/*static*/ const ModUnicodeString ModNormRule::normalTblName("unastd.tbl");
/*static*/ const ModUnicodeString ModNormRule::preMapName("preMap.dat");
/*static*/ const ModUnicodeString ModNormRule::postMapName("postMap.dat");
/*static*/ const ModUnicodeString ModNormRule::combiMapName("combiMap.dat");
/*static*/ const ModUnicodeString ModNormRule::metaDefName("metaDef.tbl");
// file name of user word dictionary for getExpandStrings.
/*static*/ const ModUnicodeString ModNormRule::expStrStrWrdDicName("expStrStrWrd.dic");
// file name of user application dictionary for getExpandStrings.
/*static*/ const ModUnicodeString ModNormRule::expStrStrAppDicName("expStrStrApp.dic");
// file name of user word dictionary for getExpandStrings.
/*static*/ const ModUnicodeString ModNormRule::expStrMorWrdDicName("expStrMorWrd.dic");
// file name of user application dictionary for getExpandStrings.
/*static*/ const ModUnicodeString ModNormRule::expStrMorAppDicName("expStrMorApp.dic");

//
// FUNCTION
// ModNormRule::ModNormRule -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModCharString& resourceDirPath
//		正規化用辞書のファイルパス
//
// const ModCharString& ruleDicPath
//		正規化用辞書のファイルパス
// const ModCharString& ruleAppPath
//		正規化用アプリ情報のファイルパス
// const ModCharString& expandDicPath
//		展開用辞書のファイルパス
// const ModCharString& expandAppPath
//		展開用アプリ情報のファイルパス
// const ModCharString& connectTblPath
//		接続表のファイルパス
// const ModCharString& preMapPath
//		前処理マップのファイルパス
// const ModCharString& postMapPath
//		後処理マップのファイルパス
// const ModCharString& combiMapPath
//		結合文字マップのファイルパス
// const ModBoolean english
//		英語処理指定
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		ファイルパスが正しくない
// その他、下位からの例外をそのまま返す
//
ModNormRule::ModNormRule(const ModCharString& dataDirPathC,
						 const ModBoolean english)
	: ruleDic(0), ruleApp(0), expandDic(0), expandApp(0), connectTbl(0),
	  unknownTbl(0), unknownCost(0), normalTbl(0),
	  combiMap(0), preMap(0), preSurrogateMap(0), preDecompMap(0),
	  postMap(0), postSurrogateMap(0), postDecompMap(0),
	  normalizeEnglish(english),isMinimize(ModFalse)
{
	try {
		ModUnicodeString dataDirPath(dataDirPathC);
		load(dataDirPath + ruleDicName, dataDirPath + ruleAppName ,
			 dataDirPath + expandDicName, dataDirPath + expandAppName,
			 dataDirPath + connectTblName, dataDirPath + unknownTblName,
			 dataDirPath + unknownCostName, dataDirPath + normalTblName,
			 dataDirPath + preMapName, dataDirPath + postMapName,
			 dataDirPath + combiMapName, dataDirPath + metaDefName);
	} catch (ModException& e) {
		ModRethrow(e);
	}
}

ModNormRule::ModNormRule(const ModUnicodeString& dataDirPath,
						 const ModBoolean english)
	: ruleDic(0), ruleApp(0), expandDic(0), expandApp(0), connectTbl(0),
	  unknownTbl(0), unknownCost(0), normalTbl(0),
	  combiMap(0), preMap(0), preSurrogateMap(0), preDecompMap(0),
	  postMap(0), postSurrogateMap(0), postDecompMap(0),
	  normalizeEnglish(english), isMinimize(ModFalse), expStrWrdDic(0), expStrAppDic(0),
	  expStrDicLoad(ModFalse), expStrStrDicLoad(ModFalse), expStrMorDicLoad(ModFalse)
{
	try {
		ModUnicodeString dataDirPathV(dataDirPath);
		// normディレクトリに存在する方の展開処理データロードする
		if(ModFile::doesExist(dataDirPathV + expStrStrWrdDicName) 
			&& ModFile::doesExist(dataDirPathV + expStrStrAppDicName)){
			load(dataDirPathV + ruleDicName, dataDirPathV + ruleAppName ,
				 dataDirPathV + expandDicName, dataDirPathV + expandAppName,
				 dataDirPathV + connectTblName, dataDirPathV + unknownTblName,
				 dataDirPathV + unknownCostName, dataDirPathV + normalTblName,
				 dataDirPathV + preMapName, dataDirPathV + postMapName,
				 dataDirPathV + combiMapName, dataDirPathV + metaDefName,
				 dataDirPathV + expStrStrWrdDicName, dataDirPathV + expStrStrAppDicName);
			expStrStrDicLoad = ModTrue;
		} else if(ModFile::doesExist(dataDirPathV + expStrMorWrdDicName) 
			&& ModFile::doesExist(dataDirPathV + expStrMorAppDicName)){
			load(dataDirPathV + ruleDicName, dataDirPathV + ruleAppName ,
				 dataDirPathV + expandDicName, dataDirPathV + expandAppName,
				 dataDirPathV + connectTblName, dataDirPathV + unknownTblName,
				 dataDirPathV + unknownCostName, dataDirPathV + normalTblName,
				 dataDirPathV + preMapName, dataDirPathV + postMapName,
				 dataDirPathV + combiMapName, dataDirPathV + metaDefName,
				 dataDirPathV + expStrMorWrdDicName, dataDirPathV + expStrMorAppDicName);
			expStrMorDicLoad = ModTrue;
		} else{
			load(dataDirPathV + ruleDicName, dataDirPathV + ruleAppName ,
				 dataDirPathV + expandDicName, dataDirPathV + expandAppName,
				 dataDirPathV + connectTblName, dataDirPathV + unknownTblName,
				 dataDirPathV + unknownCostName, dataDirPathV + normalTblName,
				 dataDirPathV + preMapName, dataDirPathV + postMapName,
				 dataDirPathV + combiMapName, dataDirPathV + metaDefName);
		}
		if(expStrStrDicLoad || expStrMorDicLoad){
			expStrDicLoad = ModTrue;
		}
	} catch (ModException& e) {
		expStrDicLoad = ModFalse;
		expStrStrDicLoad = ModFalse;
		expStrMorDicLoad = ModFalse;
		ModRethrow(e);
	}
}

ModNormRule::ModNormRule(
  const ModCharString& ruleDicPathC, const ModCharString& ruleAppPathC,
  const ModCharString& expandDicPathC, const ModCharString& expandAppPathC,
  const ModCharString& connectTblPathC, const ModCharString& unknownTblPathC,
  const ModCharString& unknownCostPathC, const ModCharString& normalTblPathC,
  const ModCharString& preMapPathC, const ModCharString& postMapPathC,
  const ModCharString& combiMapPathC, const ModBoolean english)
	: ruleDic(0), ruleApp(0), expandDic(0), expandApp(0), connectTbl(0),
	  unknownTbl(0), unknownCost(0), normalTbl(0),
	  combiMap(0), preMap(0), preSurrogateMap(0), preDecompMap(0),
	  postMap(0), postSurrogateMap(0), postDecompMap(0),
	  normalizeEnglish(english), isMinimize(ModFalse)
{
	try {
		load(ModUnicodeString(ruleDicPathC), ModUnicodeString(ruleAppPathC),
			 ModUnicodeString(expandDicPathC), ModUnicodeString(expandAppPathC),
			 ModUnicodeString(connectTblPathC),
			 ModUnicodeString(unknownTblPathC),
			 ModUnicodeString(unknownCostPathC),
			 ModUnicodeString(normalTblPathC), ModUnicodeString(preMapPathC),
			 ModUnicodeString(postMapPathC), ModUnicodeString(combiMapPathC),
			 ModUnicodeString(""));
	} catch (ModException& e) {ModRethrow(e);}
}

ModNormRule::ModNormRule(
  const ModUnicodeString& ruleDicPath, const ModUnicodeString& ruleAppPath,
  const ModUnicodeString& expandDicPath, const ModUnicodeString& expandAppPath,
  const ModUnicodeString& connectTblPath, const ModUnicodeString& unknownTblPath,
  const ModUnicodeString& unknownCostPath, const ModUnicodeString& normalTblPath,
  const ModUnicodeString& preMapPath, const ModUnicodeString& postMapPath,
  const ModUnicodeString& combiMapPath, const ModBoolean english)
	: ruleDic(0), ruleApp(0), expandDic(0), expandApp(0), connectTbl(0),
	  unknownTbl(0), unknownCost(0), normalTbl(0),
	  combiMap(0),preMap(0),postMap(0),preSurrogateMap(0),
	  postSurrogateMap(0),preDecompMap(0),postDecompMap(0),
	  normalizeEnglish(english), isMinimize(ModFalse)
{
	try {
		load(ruleDicPath, ruleAppPath, expandDicPath, expandAppPath,
			 connectTblPath, unknownTblPath, unknownCostPath, normalTblPath,
			 preMapPath, postMapPath, combiMapPath, ModUnicodeString(""));
	} catch (ModException& e) {ModRethrow(e);}
}

//
// FUNCTION
// ModNormRule::minimize -- リソースの最小化
//
// NOTES
// UNA関連のリソースを既存のModNormRuleから参照し、余分な保持を避ける
//
// ARGUMENTS
// const ModNormRule& normRule_
//		正規化ルール
//
// RETURN
//
// EXCEPTIONS
//
ModBoolean
ModNormRule::minimize(const ModNormRule* normRule_)
{
	if (normRule_->ruleDic != 0    && normRule_->ruleApp != 0 &&
	    normRule_->expandDic != 0  && normRule_->expandApp != 0 &&
	    normRule_->connectTbl != 0 && normRule_->unknownTbl != 0 &&
	    normRule_->unknownCost != 0 ){
		unaKApi_freeImg(ruleDic);
		unaKApi_freeImg(ruleApp);
		unaKApi_freeImg(expandDic);
		unaKApi_freeImg(expandApp);
		unaKApi_freeImg(connectTbl);
		unaKApi_freeImg(unknownTbl);
		unaKApi_freeImg(unknownCost);
		ruleDic     = normRule_->ruleDic;
		ruleApp     = normRule_->ruleApp;
		expandDic   = normRule_->expandDic;
		expandApp   = normRule_->expandApp;
		connectTbl  = normRule_->connectTbl;
		unknownTbl  = normRule_->unknownTbl;
		unknownCost = normRule_->unknownCost;
		isMinimize  = ModTrue;
	}

	return isMinimize;

}

// The resource is returned directly, (for Highlighter)
// Ligature map
const 
ModNormCombiMap* 
ModNormRule::getCombiMap() const
{return combiMap;}

// Pretreatment map
const 
ModUnicodeChar* 
ModNormRule::getPreMap() const
{return preMap;}

// Map
const 
ModUnicodeChar* 
ModNormRule::getPostMap() const
{return postMap;}

// Disassembly map for pretreatment
const 
ModNormChar* 
ModNormRule::getPreDecompMap() const
{return preDecompMap;}

//
// ModNormRule::load -- ロード
//
// NOTES
// 正規化に必要なデータファイルをロードする。
// 前処理マップにはUnicode追加多言語面(SMP)の文字(16進5～6桁)を記述できる。
// 後処理マップと結合処理マップにはSMPの文字を記述できない。
//
// ARGUMENTS
// const ModUnicodeString& ruleDicPath
//		正規化用辞書のファイルパス
// const ModUnicodeString& ruleAppPath
//		正規化用アプリ情報のファイルパス
// const ModUnicodeString& expandDicPath
//		展開用辞書のファイルパス
// const ModUnicodeString& expandAppPath
//		展開用アプリ情報のファイルパス
// const ModUnicodeString& connectTblPath
//		接続表のファイルパス
// const ModUnicodeString& preMapPath
//		前処理マップのファイルパス
// const ModUnicodeString& postMapPath
//		後処理マップのファイルパス
// const ModUnicodeString& combiMapPath
//		結合文字マップのファイルパス
// const ModUnicodeString& combiMapPath
//		メタ文字テーブルファイルパス
// const ModUnicodeString & expStrWrdDicPath
//		File path of dictionary for expand string
// const ModUnicodeString & expStrAppDicPath
//		File path of application information for expand string
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		ファイルパスが正しくない
//		すでにルールがロードされている
// その他、下位からの例外をそのまま返す
//
void
ModNormRule::load(const ModUnicodeString& ruleDicPath,
  const ModUnicodeString& ruleAppPath, const ModUnicodeString& expandDicPath,
  const ModUnicodeString& expandAppPath, const ModUnicodeString& connectTblPath,
  const ModUnicodeString& unknownTblPath,
  const ModUnicodeString& unknownCostPath,
  const ModUnicodeString& normalTblPath, const ModUnicodeString& preMapPath,
  const ModUnicodeString& postMapPath, const ModUnicodeString& combiMapPath,
  const ModUnicodeString& metaDefPath,
  const ModUnicodeString& expStrWrdDicPath, const ModUnicodeString& expStrAppDicPath)
{
	if (ruleDic != 0 || ruleApp != 0 || expandDic != 0 || expandApp != 0 ||
		connectTbl != 0 || unknownTbl != 0 || unknownCost != 0 ||
		combiMap != 0 || preMap !=0 || postMap !=0 || preSurrogateMap != 0 ||
		postSurrogateMap != 0 || preDecompMap != 0 ||
		postDecompMap !=0 || expStrWrdDic != 0 || expStrAppDic != 0) {
#ifdef DEBUG
		ModErrorMessage << "already initialized" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	int ret;

	try {
		preMap  = new ModUnicodeChar[ModNormUnicodeCharMax];
		postMap = new ModUnicodeChar[ModNormUnicodeCharMax];
		preSurrogateMap  = new ModNormSurrogateChar[ModNormSurrogateMapMax];
		postSurrogateMap = new ModNormSurrogateChar[ModNormSurrogateMapMax];
		preDecompMap  = new ModNormChar[ModNormDecompMapMax];
		postDecompMap = new ModNormChar[ModNormDecompMapMax];

		ModUnicodeString a;

		// check metaDefTabl ( ruleDic is only dummy for this place)
		a = metaDefPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&ruleDic);
		if (ret < 0) {
			// there is no metaDefTbl -> default setting will be effective
			useMetaDef = ModTrue;
		}
		else{
			// there is metaDefTbl -> set metaDefFlag
			useMetaDef = ModFalse;
			unaKApi_freeImg(ruleDic);
		}

		a = ruleDicPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&ruleDic);
		if (ret < 0) {
			// it is possible for optional normalizer
			// therefor output no error message
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = ruleAppPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&ruleApp);
		if (ret < 0) {
			ModErrorMessage << "ruleAppPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << ruleAppPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = expandDicPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&expandDic);
		if (ret < 0) {
			ModErrorMessage << "expandDicPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << expandDicPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = expandAppPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&expandApp);
		if (ret < 0) {
			ModErrorMessage << "expandAppPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << expandAppPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = connectTblPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&connectTbl);
		if (ret < 0) {
			ModErrorMessage << "connectTblPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << connectTblPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = unknownTblPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&unknownTbl);
		if (ret < 0) {
			ModErrorMessage << "unknownTblPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << unknownTblPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		a = unknownCostPath;
		ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&unknownCost);
		if (ret < 0) {
			ModErrorMessage << "unknownCostPath invalid: "
				<< unaKApi_getErrMsg(ret) << ": " << unknownCostPath << ModEndl;
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument, ModErrorLevelError);
		}
		try {
			loadMap(preMapPath, preMap, preSurrogateMap, preDecompMap);
		} catch (ModException& e) {
			ModErrorMessage << "preMapPath invalid: " << preMapPath << ModEndl;
			ModRethrow(e);
		}
		try {
			loadMap(postMapPath, postMap, postSurrogateMap, postDecompMap);
		} catch (ModException& e) {
			ModErrorMessage << "postMapPath invalid: " << postMapPath << ModEndl;
			ModRethrow(e);
		}
		try {
			loadCombiMap(combiMapPath);
		} catch (ModException& e) {
			ModErrorMessage << "combiMapPath invalid: " << combiMapPath
							<< ModEndl;
			ModRethrow(e);
		}

		// load expand string data
		if(ModFile::doesExist(expStrWrdDicPath) && ModFile::doesExist(expStrAppDicPath)){
			a = expStrWrdDicPath;

			ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&expStrWrdDic);
			if (ret < 0) {
				ModErrorMessage << "expStrWrdDicPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << expStrWrdDicPath << ModEndl;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
			a = expStrAppDicPath;
			ret = unaKApi_readFileImg(a.getString(ModOs::Process::getEncodingType()),&expStrAppDic);
			if (ret < 0) {
				ModErrorMessage << "expStrAppDicPath invalid: "
					<< unaKApi_getErrMsg(ret) << ": " << expStrAppDicPath << ModEndl;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument, ModErrorLevelError);
			}
		}
	} catch (ModException& e) {
		if ( preMap)
			delete[] preMap;
		if ( postMap)
			delete[] postMap;
		if ( preSurrogateMap)
			delete[] preSurrogateMap;
		if ( postSurrogateMap)
			delete[] postSurrogateMap;
		if ( preDecompMap)
			delete[] preDecompMap;
		if ( postDecompMap)
			delete[] postDecompMap;
// add to avoid memory leak (RST 05/04/22)
		if (combiMap != 0)
			delete combiMap;
		if (ruleDic != 0)
			unaKApi_freeImg(ruleDic);
		if (ruleApp != 0)
			unaKApi_freeImg(ruleApp);
		if (expandDic != 0)
			unaKApi_freeImg(expandDic);
		if (expandApp != 0)
			unaKApi_freeImg(expandApp);
		if (connectTbl != 0)
			unaKApi_freeImg(connectTbl);
		if (unknownTbl != 0)
			unaKApi_freeImg(unknownTbl);
		if (unknownCost != 0)
			unaKApi_freeImg(unknownCost);
		if (expStrWrdDic != 0)
			unaKApi_freeImg(expStrWrdDic);
		if (expStrAppDic != 0)
			unaKApi_freeImg(expStrAppDic);

		preMap=0;
		postMap=0;
		preSurrogateMap=0;
		postSurrogateMap=0;
		preDecompMap=0;
		postDecompMap=0;
		combiMap=0;
		ruleDic = 0;
		ruleApp = 0;
		expandDic = 0;
		expandApp = 0;
		connectTbl = 0;
		unknownTbl = 0;
		unknownCost = 0;
		expStrWrdDic = 0;
		expStrAppDic = 0;
		expStrDicLoad = ModFalse;
		expStrStrDicLoad = ModFalse;
		expStrMorDicLoad = ModFalse;

		ModRethrow(e);
	}
}

// FUNCTION
// ModNormRule::~ModNormRule -- デストラクタ
//
// NOTES
// デストラクタ
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
ModNormRule::~ModNormRule()
{
	if ( preMap)
		delete[] preMap;
	if ( preSurrogateMap)
		delete[] preSurrogateMap;
	if ( preDecompMap)
		delete[] preDecompMap;
	if ( postMap)
		delete[] postMap;
	if ( postSurrogateMap)
		delete[] postSurrogateMap;
	if ( postDecompMap)
		delete[] postDecompMap;
	if (combiMap != 0)
		delete combiMap;

	if ( isMinimize ){
		return;
	}
	if (ruleDic != 0)
		unaKApi_freeImg(ruleDic);
	if (ruleApp != 0)
		unaKApi_freeImg(ruleApp);
	if (expandDic != 0)
		unaKApi_freeImg(expandDic);
	if (expandApp != 0)
		unaKApi_freeImg(expandApp);
	if (connectTbl != 0)
		unaKApi_freeImg(connectTbl);
	if (unknownTbl != 0)
		unaKApi_freeImg(unknownTbl);
	if (unknownCost != 0)
		unaKApi_freeImg(unknownCost);
	if (expStrWrdDic != 0)
		unaKApi_freeImg(expStrWrdDic);
	if (expStrAppDic != 0)
		unaKApi_freeImg(expStrAppDic);
	expStrDicLoad = ModFalse;
	expStrStrDicLoad = ModFalse;
	expStrMorDicLoad = ModFalse;
}

//
// FUNCTION
// ModNormRule::isExpStrDicLoad -- isExpStrDicLoad
//
// NOTES
// load of expanding data success or failure.
//
// ARGUMENTS
// none
//
// RETURN
// ModTrue		successful load of data
// ModBoolean	failed to load data
//
// EXCEPTIONS
//
ModBoolean
ModNormRule::isExpStrDicLoad()
{
	return expStrDicLoad;
}

//
// FUNCTION
// ModNormRule::isExpStrStrDicLoad -- isExpStrStrDicLoad
//
// NOTES
// load of expanding data success or failure.
//
// ARGUMENTS
// none
//
// RETURN
// ModTrue		successful load of data
// ModBoolean	failed to load data
//
// EXCEPTIONS
//
ModBoolean
ModNormRule::isExpStrStrDicLoad()
{
	return expStrStrDicLoad;
}

//
// FUNCTION
// ModNormRule::isExpStrMorDicLoad -- isExpStrMorDicLoad
//
// NOTES
// load of expanding data success or failure.
//
// ARGUMENTS
// none
//
// RETURN
// ModTrue		successful load of data
// ModBoolean	failed to load data
//
// EXCEPTIONS
//
ModBoolean
ModNormRule::isExpStrMorDicLoad()
{
	return expStrMorDicLoad;
}

// FUNCTION private
// ModNormRule::loadMap -- マップのロード
//
// NOTES
// 前処理あるいは後処理用のマップをロードする。
//
// ARGUMENTS
// const ModCharString& fname
//		パス名
// ModUnicodeChar* map
//		データをセットするマップ
// ModNormSurrogateChar* surrogate
//		データをセットするサロゲート文字マップ
// ModNormChar* decomp
//		データをセットする分解マップ
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		ファイルパスが正しくない
// ModCommonErrorNotInitialized
//		ファイルの記述が正しくない
// その他、下位からの例外をそのまま返す
//
void
ModNormRule::loadMap(const ModUnicodeString& fname,
		 ModUnicodeChar* map, ModNormSurrogateChar* surrogate, ModNormChar* decomp)
{
	if (map == 0)
		ModThrow(ModModuleStandard,
				ModCommonErrorNotInitialized, ModErrorLevelError);

	for (ModSize j = 0; j < ModNormUnicodeCharMax; ++j) {
		map[j] = ModUnicodeChar(j);
	}

	ModNormSurrogateChar null_msc;

	for (ModSize i = 0; i < ModNormSurrogateMapMax; i++) {
		surrogate[i] = null_msc;
	}

	ModNormChar null_mc;

	for (ModSize i = 0; i < ModNormDecompMapMax; i++) {
		decomp[i] = null_mc;
	}
#if defined(V1_6)
	ModAutoPointer<ModFile> file(new ModFile(fname, ModFile::readMode));
#else
	ModUnicodeString a(fname);
	ModCharString b(a.getString(ModOs::Process::getEncodingType()));
	ModAutoPointer<ModFile> file(new ModFile( b, ModFile::readMode));
#endif
	ModArchive ar(*file, ModArchive::ModeLoadArchive);

	char* dividers = (char*)"; ";

	char* g_code;	   // 原表記	 一字Unicode値 16進数
	char* s_onecode;	// 正規化表記 一字Unicode値 16進数
	unsigned long g_tmp, s_onetmp;	// 原表記, 正規化表記 作業用
	ModUnicodeChar  g_val, g_val2;  // 原表記, 原表記2 一字Unicode値
	ModUnicodeString  s_str(" ");		  // 正規化表記
	ModSize surrogate_max_idx = 0;
	ModSize decomp_max_idx = 0;
	ModCharString aline;

	while (1){
		aline.clear();
		try{
			while (1) {
				char c;
				ar >> c;
				if (c == '\n')
					break;
				aline += c;
			}
		}
		catch (ModException&e){
			if ( e.getErrorNumber() == ModOsErrorEndOfFile){
				ModErrorHandle::reset();
				break;
			}
			else{
				ModRethrow(e);
			}
		}
		g_code = strtok (aline.getBuffer(), dividers);
		g_tmp = strtoul (g_code, (char**)NULL, 16); // 原表記 
		if (g_tmp < 0x10000) {
			// 基本多言語面の文字
			g_val  = (ModUnicodeChar)g_tmp;
			g_val2 = (ModUnicodeChar)0;
		} else {
			// 追加多言語面の文字；サロゲートペアに変換する
			g_tmp -= 0x10000;
			g_val  = (ModUnicodeChar)(g_tmp / 0x0400 + 0xD800);
			g_val2 = (ModUnicodeChar)(g_tmp % 0x0400 + 0xDC00);
		}
		s_str = "";
		while (s_onecode = strtok ((char*)0, dividers)) {
			s_onetmp = strtoul (s_onecode, (char**)NULL, 16);
			if (s_onetmp < 0x10000) {
				// 基本多言語面の文字
				if (s_onetmp) {
					s_str += (ModUnicodeChar)s_onetmp;
				}
			} else {
				// 追加多言語面の文字；サロゲートペアに変換する
				s_onetmp -= 0x10000;
				s_str += (ModUnicodeChar)(s_onetmp / 0x0400 + 0xD800);
				s_str += (ModUnicodeChar)(s_onetmp % 0x0400 + 0xDC00);
			}
		}
		if (g_val2) {
			// 追加多言語面の文字
			map[g_val] = 0xFFFE;
			surrogate[surrogate_max_idx++] = ModNormSurrogateChar(g_val, g_val2, s_str);
			if (surrogate_max_idx >= ModNormSurrogateMapMax) {
#ifdef DEBUG
				ModErrorMessage << "surrogate data invalid " << surrogate_max_idx << ModEndl;
#endif
				ModThrow(ModModuleStandard,
						 ModCommonErrorNotInitialized, ModErrorLevelError);
			}
		} else {
			// 基本多言語面の文字
			if (s_str.getLength() < 2)	   // be careful of null, eg for combi
				map[g_val] = s_str[0];	// 正規化表記 ModUnicodeChar
			else  {
				map[g_val] = 0xFFFF;	  // 正規化表記 ModUnicodeString
				decomp[decomp_max_idx++] = ModNormChar (g_val, s_str);
				if (decomp_max_idx >= ModNormDecompMapMax) {
#ifdef DEBUG
					ModErrorMessage << "decomp data invalid " << decomp_max_idx << ModEndl;
#endif
					ModThrow(ModModuleStandard,
							 ModCommonErrorNotInitialized, ModErrorLevelError);
				}
			}
		}
	}
}

// FUNCTION private
// ModNormRule::loadCombiMap -- 合字マップのロード
//
// NOTES
// 合字マップをロードする。
//
// ARGUMENTS
// const ModCharString& fname
//		パス名
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		ファイルパスが正しくない
// ModCommonErrorNotInitialized
//		ファイルの記述が正しくない
// その他、下位からの例外をそのまま返す
//
void
ModNormRule::loadCombiMap(const ModUnicodeString& fname)
{
#if defined(V1_6)
	ModAutoPointer<ModFile> file(new ModFile(fname, ModFile::readMode));
#else
	ModUnicodeString a(fname);
	ModCharString b(a.getString(ModOs::Process::getEncodingType()));
	ModAutoPointer<ModFile> file(new ModFile( b, ModFile::readMode));
#endif
	ModArchive ar(*file, ModArchive::ModeLoadArchive);

	char c, aline[16];
	ModCharString buf;
	ModUnicodeChar combinee, combiner, combined;

	// ルールの行数を知る
	while (1) {
		ar >> c;
		if (c == '\n')
			break;
		buf += c;
	}

	if (buf.getLength() == 0) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	ModSize num = buf.toInt();
	combiMap = new ModNormCombiMap();

	// ルールを読み込む
	for (ModSize n(0); n < num; ++n) {
		ar(aline, 15);

		if (aline[4] != ' ' || aline[9] != ';' || aline[14] != '\n') {
			ModThrow(ModModuleStandard,
					 ModCommonErrorNotInitialized, ModErrorLevelError);
		}
		aline[4] = '\0';
		combinee = ModCharTrait::toInt(aline, 16);
		aline[9] = '\0';
		combiner = ModCharTrait::toInt(aline + 5, 16);
		aline[14] = '\0';
		combined = ModCharTrait::toInt(aline + 10, 16);

#ifdef NORM_DEBUG
		ModDebugMessage << combinee << ' ' << combiner << ' ' << combined
						<< ModEndl;
#endif
		combiMap->insert((combinee << 16) + combiner, combined);
	}
}

//
// Copyright (c) 2000-2012, 2023 RICOH Company, Ltd.
// All rights reserved.
//
