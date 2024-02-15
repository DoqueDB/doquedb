// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModUNA.h -- UNA ラッパークラス（表記正規化機能付き）の定義
// 
// Copyright (c) 2000-2010, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModUNA_H__
#define __ModUNA_H__

#include "ModVector.h"
#include "UNA_UNIFY_TAG.h"
#include "ModNlpUnaJp/ModNormDLL.h"
#include "ModNlpUnaJp/ModUnaMiddle.h"
#include "ModNlpUnaJp/Module.h"
#include "ModNlpUnaJp/ModNlpExpStr.h"
	
_UNA_BEGIN
class ModWordStemmer;
_UNA_UNAJP_BEGIN


class ModUnaAnalyzer;
class ModNormalizer;


//  ENUM
//  ModUnaExpMode --  展開モード
//
//  NOTES
//      ModUnaAnalyzer::getExpand() の引数として利用される。
//
//      ModUnaExpChkOrigStr -- 別の展開文字列を含む展開文字列を省略する
//      ModUnaExpNoChk   -- 省略しない~(デフォルト)
//
enum ModUnaExpMode
{
	ModUnaExpChkOrigStr,	// for expandBuf()
	ModUnaExpNoChk 				// default
};

//
// 【注意】
// libUna に以下のクラスを含めると、libNormalizer と libUna が相互参照
// してしまうので、libNormalizer に含めることとした。
//


//
// CLASS
// ModUnaAnalyzer -- UNA 解析器クラス（表記正規化機能付き）
//
// NOTES
// 表記正規化機能付きの UNA の解析機能を提供するラッパークラス。
//
class ModNormDLL ModUnaAnalyzer : public ModUnaMiddleAnalyzer
{
private:
	DicSet	*m_dicSet;
public:
	// NOTES コンストラクタ
	ModUnaAnalyzer(ModUnaResource* const resource,
						ModNormalizer* const,
						ModWordStemmer* const,
						ModNlpExpStr	*expStr_ = 0,
						DicSet	*dicSet_ = 0,
						ModBoolean useDic=ModTrue,
						ModSize maxWordLength=0);

	// NOTES ステマーの変更
	void setStemmer(ModWordStemmer* const stemmer_);

	void setExecQuick(ModBoolean);

	// NOTES ノーマライザの変更
	void setNormalizer(ModNormalizer* const normalizer_);

	// NOTES １形態素の取得
	virtual ModBoolean getMorph(
		ModUnicodeString& result,
		const ModBoolean normalize);

	// NOTES １形態素の原表記付き取得
	virtual ModBoolean getMorph(
		ModUnicodeString& result,
		ModUnicodeString& ostr,
		const ModBoolean normalize);

	// NOTES １形態素の品詞付 異表記正規化実施 原表記付 取得
	virtual ModBoolean getMorph(
		ModUnicodeString& result,
		ModUnicodeChar* &original,
		ModSize &len,
		int &pos);

	// NOTES １形態素の展開取得 (bug correspondence)
	ModSize getExpand(
		ModUnicodeString*& expanded,
		const ModUnaExpMode chkOrg,
		ModUnicodeString& ostr,
		int& pos_);

	// NOTES 形態素ブロックの取得
	ModBoolean getBlock(
		ModUnicodeString& result,
		const ModUnicodeChar separator1,
		const ModUnicodeChar separator2,
		const ModBoolean normalize = ModFalse);


	// NOTES 形態素ブロックの取得 ( for modeterm2)
	ModBoolean getBlock(
		ModVector<ModUnicodeString>& formVector,
		ModVector<int>& posVector,
		const ModBoolean normalize = ModFalse);

	// NOTES 形態素ブロックの原表記付き取得 ( for ModTermV2)
	ModBoolean getBlock( ModVector<ModUnicodeString>& formVector_,
		ModVector<ModUnicodeString>& ostrVector_,
		ModVector<int>& posVector_,
		const ModBoolean getNormalized_,
		ModVector<int> *costVector_ = 0,
		ModVector<int> *uposVector_ = 0,
		ModBoolean ignore = ModTrue);

	// NOTES ModUnaMiddleanalyzer::analyze()で取得した形態素の辞書ベース名を取得
	ModBoolean getDicName(
		ModVector<ModUnicodeString>& dicNameVector_);

	// for backword compatibility
	ModBoolean getNormalized(ModUnicodeString& result,
				 ModNormalizer* const,
				 ModWordStemmer* const);

private:

	// １形態素の取得下位関数
	ModBoolean getNormalized(
			ModUnicodeString& result,
			ModUnicodeString& ostr,
			ModBoolean getOriginal);
		ModBoolean getSimpleNormalized(
				ModUnicodeString& result,
				ModUnicodeString& ostr,
				ModBoolean getOriginal,
				int& pos_);

		ModBoolean getCheckNormalized(
				ModUnicodeString& result,
				ModUnicodeString& ostr,
				ModBoolean getOriginal,
				ModBoolean execStemming,
				int& pos_);

		// １形態素の情報取得(高速動作)
		ModBoolean getCheckNormalizedSpeed(
				ModUnicodeString& result,
				ModUnicodeChar* &ostr,
				ModSize &len,
				ModBoolean getOriginal,
				int &pos,
				ModBoolean execStemming);

		ModBoolean getCheck(
				ModUnicodeString& result,
				ModUnicodeString& ostr,
				ModBoolean getOriginal);

	// 形態素ブロックの取得下位関数
	ModBoolean getBlockNormalized(
			ModUnicodeString&,
			const ModUnicodeChar separator1,
			const ModUnicodeChar separator2);

	ModBoolean getBlockSimple(
			ModUnicodeString&,
			const ModUnicodeChar separator1,
			const ModUnicodeChar separator2);

	ModBoolean getBlockCheck(
			ModUnicodeString&,
			const ModUnicodeChar separator1,
			const ModUnicodeChar separator2);

	// 形態素ブロックの原表記付き取得下位関数(for ModTermV2)
		ModBoolean 	getBlockNormalized( ModVector<ModUnicodeString>& formVector_,
										ModVector<ModUnicodeString>& ostrVector_,
										ModVector<int>& posVector_,
										ModBoolean getOriginal_,
										ModVector<int>* costVector_ ,
										ModVector<int>* uposVector_,
										ModBoolean ignore_ = ModTrue);
		
		ModBoolean 	getBlockSimple( ModVector<ModUnicodeString>& formVector_,
									ModVector<ModUnicodeString>& ostrVector_,
									ModVector<int>& posVector_,
									ModBoolean getOriginal_,
									ModVector<int> *costVector_,
									ModVector<int> *uposVector_,
									ModBoolean ignore_ = ModTrue);

		ModBoolean getBlockCheck( 	ModVector<ModUnicodeString>& formVector_,
									ModVector<ModUnicodeString>& ostrVector_,
									ModVector<int>& posVector_,
									ModBoolean getOriginal_,
									ModVector<int> *costVector_,
									ModVector<int> *uposVector_,
									ModBoolean ignore_ = ModTrue);

		ModBoolean getBlockCheckSpeed(	ModVector<ModUnicodeString>& formVector_,
										ModVector<ModUnicodeString>& ostrVector_,
										ModVector<int>& posVector_,
										ModBoolean getOriginal_,
										ModVector<int > *costVector_,
										ModVector<int> *uposVector_,
										ModBoolean ignore_ = ModTrue);

	ModBoolean getBlockCompoundDivOnly( ModVector<ModUnicodeString>& formVector_,
										ModVector<int>& posVector_,
										ModVector<ModUnicodeString>* ostrVector_ = 0,
										ModVector<int > *costVector_ = 0,
										ModVector<int> *uposVector_ = 0);

	ModNormalizer* /*const*/ normalizer;
	ModWordStemmer* /*const*/ stemmer;
	ModNlpExpStr* expstr;
	ModUInt32 tokenType;
	ModUnicodeString work1, work2;			// getCheckNormalized 用
	ModUnicodeString work3;					// 原表記取得用
	ModVector<ModUnicodeString> work4;		// 同上
	ModUnicodeString cache1[64],cache2[64];	// 異表記正規化結果用
	ModBoolean execQuick;
};

_UNA_UNAJP_END
_UNA_END

#endif // __ModUNA_H__
//
// Copyright (c) 2000-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
