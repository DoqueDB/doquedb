// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLP.cpp -- 言語処理クラスの実装
// 
// Copyright (c) 2001, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#include "ModMessage.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModParameter.h"
#include "ModAutoMutex.h"
#include "ModFile.h"
#include "ModException.h"
#include "LibUna/Keyword.h"
#include "LibUna/DicSet.h"
#include "ModNLP.h"
#include "ModNlpUnaJp/ModNLPX.h"
#include "EnStem/ModEnglishWordStemmer.h"


_UNA_USING
_UNA_UNAJP_USING

////////////////////////////////////////////////////////////////////////
// FUNCTION
// ModNlpResource::setResource -- リソースのセット(実質的なコンストラクタ)
//
// NOTES
// 解析用リソースの実質的コンストラクタ。
// リソースはリソースディレクトリへのパスを指定する。
//
// ARGUMENTS
// const ModUnicodeString& rscPath_
// 	リソースディレクトリへのパス
// const ModLanguageSet& languageSet_
// 	対応言語の指定
// const ModSize memReduceLevel_
// 	省メモリスイッチ
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//  言語指定に誤りがある
//
void
ModNlpResourceX::setResource(const ModUnicodeString& rscPath_,
							 const ModLanguageSet& languageSet_,
							 ModSize memReduceLevel_)
{
	languageSetForNlpResource = languageSet_.round();

	canProcessJapanese = ModFalse;
	canIdentifyingHiraganaKatakana = ModFalse;

	// preparation of normalizer(norm data must exist)
	try {
		ModUnicodeString tmpPath;

		// for normRule
		tmpPath = rscPath_;
		tmpPath += "norm/";
		normRule = new ModNormRule(tmpPath);

		// make temporal normalizer for language check
		ModNormalizer a(normRule, ModFalse);

		// check weather the data can process Japanese
		const ModUnicodeChar  jaCharIn[] = {0x30ab,0x309b,0};
		const ModUnicodeChar  jaCharOut[] = {0x30ac,0};
		if (a.normalizeBuf(jaCharIn,0,0,ModNormalized) == jaCharOut) {
			canProcessJapanese = ModTrue;

			// check identifying hiragana to katanaka
			const ModUnicodeChar jaCharIn2[] = {0x304d, 0};	// HIRAGANA KI
			const ModUnicodeChar jaCharOut2[] = {0x30ad, 0}; // KATAKANA KI
			if (a.normalizeBuf(jaCharIn2,0,0,ModNormalized)
				== jaCharOut2) {
				canIdentifyingHiraganaKatakana = ModTrue;
			}
		}

	} catch (ModException& e) {
		ModErrorMessage << "ModNlpResourceX: " << e << ModEndl;
		delete normRule;
		ModRethrow(e);
	}

	// for unaResource(トークナイザ)
	try {
		ModUnicodeString tmpPath;
		tmpPath = rscPath_;
		tmpPath += "una/";
		skipHighSpeedSwitch = ModTrue;

		if ( memReduceLevel_ == 0){ // no memory reducing
			skipHighSpeedSwitch = ModFalse;	// high speed processing
		}
		
		unaResource = new ModUnaResource(tmpPath,skipHighSpeedSwitch);

	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "ModNlpResourceX: " << e << ModEndl;
#endif
		delete unaResource;
		delete normRule;
		ModRethrow(e);
	}

	// for engWordStemmer(オプショナル(優先)ステマー)
	try{
		ModUnicodeString tmpPath(rscPath_);
		tmpPath += "stem/stemmer.dat";
		if (ModFile::doesExist(tmpPath) == ModTrue)
			// 必ずlibEnStemXX.dllは使用するのでpreloadでも問題ない
			engWordStemmer = new ENSTEM::ModEnglishWordStemmer(tmpPath);

	} catch(ModException&){
		ModErrorHandle::reset();
		// stemmer.datが無いことはありえるので、何もしない
	}

	// for termResource
	try{
		ModUnicodeString tmpPath(rscPath_);
		tmpPath += "np-ja/";

		if(ModFile::doesExist(tmpPath)){
			termResource = new ModTermResource(tmpPath);
		}
	} catch (ModException& e) {
		ModErrorMessage << "ModNlpX::setResource::termResource" << ModEndl;
		if(termResource != 0) {
			delete termResource;
		}
		ModRethrow(e);
	} catch (...) {
		ModErrorMessage << "ModNlpX::setResource::termResource" << ModEndl;
		if(termResource != 0) {
			delete termResource;
		}
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument, ModErrorLevelError);
	}

	// confirmation of language specification
	if (checkLanguage(languageSetForNlpResource) == ModFalse) {
		languageSetForNlpResource.clear();
		if (canProcessJapanese)
			languageSetForNlpResource.add(ModLanguage::ja);
		languageSetForNlpResource.add(ModLanguage::en);
	}
}

//
// FUNCTION
// ModNlpResourceX::~ModNlpResourceX -- デストラクタ
//
// NOTES
// 解析用リソースのデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//
ModNlpResourceX::~ModNlpResourceX()
{
	delete unaResource;
	delete normRule;
	delete engWordStemmer;
	delete termResource;
}

//
//	FUNCTION private
//	UNAJP::ModNlpResourceX::checkLanguage -- check supported language
//
//	NOTES
//
//	ARGUMENTS
//	const ModLanguageSet& cLang_
//		language
//
//	RETURN
//	ModBoolean
//
//	EXCEPTIONS
//
ModBoolean
ModNlpResourceX::checkLanguage(const ModLanguageSet& cLang_) const
{
	if (cLang_.getSize() == 2)
	{
		if (canProcessJapanese && cLang_.isContained(ModLanguage::ja))
		if (cLang_.isContained(ModLanguage::es) ||
			cLang_.isContained(ModLanguage::en) ||
			cLang_.isContained(ModLanguage::de) ||
			cLang_.isContained(ModLanguage::nl) ||
			cLang_.isContained(ModLanguage::it) ||
			cLang_.isContained(ModLanguage::fr))
			return ModTrue;
	}
	else if (cLang_.getSize() == 1)
	{
		if ((canProcessJapanese && cLang_.isContained(ModLanguage::ja)) ||
			cLang_.isContained(ModLanguage::es) ||
			cLang_.isContained(ModLanguage::en) ||
			cLang_.isContained(ModLanguage::de) ||
			cLang_.isContained(ModLanguage::nl) ||
			cLang_.isContained(ModLanguage::it) ||
			cLang_.isContained(ModLanguage::fr))
			return ModTrue;
	}

	return ModFalse;
}

//
// FUNCTION
// ModNlpAnalyzerX::ModNlpAnalyzerX -- Constructor
//
// NOTES
// 言語解析器のコンストラクタ
//
// ARGUMENTS
// const ModNlpResourceX * const resource_
//	  リソース
// DicSet *dicSet_
//	  DicSet
// ModSize maxWordLen_
//	  最大単語長の指示(0は単語長の指示なし)
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorNotInitialized
//  リソースが初期化されていない
//
ModNlpAnalyzerX::ModNlpAnalyzerX(const ModNlpResourceX* resource_,
								DicSet	*dicSet_,
								ModSize maxWordLen_)
	: unaAnalyzer(0),
	  jakanaAnalyzer(0),
	  subAnalyzer(0),
	  defaultStemmer(0),
	  optionalStemmer(0),
	  expStr(0),
	  resource(resource_),
	  normModify(ModNlpAsIs)
{
	if (resource_ == 0) {
#ifdef DEBUG
		ModErrorMessage << "resource is zero" << ModEndl;
#endif
		ModThrow(ModModuleStandard,
			ModCommonErrorNotInitialized, ModErrorLevelError);
	}
	try {
		// initialize ModUnaAnalyzer
		// ノーマライザの準備
		subAnalyzer = new ModNormalizer(resource_->normRule, ModFalse);
		
		// prepare expanding string
		if (resource_->normRule->isExpStrDicLoad()) {
			expStr = new ModNlpExpStr(subAnalyzer);
		}

		// オプショナルステマの準備
		if (resource_->engWordStemmer) {
			optionalStemmer = resource_->engWordStemmer;
		}
		// デフォルトステマの準備
		defaultStemmer = new UNA::ModWordStemmer();

		// 英語指定かつModEnglishWordStemmerあり
		if ((resource_->languageSetForNlpResource).isContained(ModLanguage::en)
			&& optionalStemmer ){
			unaAnalyzer = new ModUnaAnalyzer(
				resource_->unaResource,
				subAnalyzer,
				optionalStemmer,
				expStr,
				dicSet_,
				resource_->canProcessJapanese,
				maxWordLen_);
		}
		// ModEnglishWordStemmerなし
		else {
			unaAnalyzer = new ModUnaAnalyzer(
				resource_->unaResource,
				subAnalyzer,
				defaultStemmer,
				expStr,
				dicSet_,
				resource_->canProcessJapanese,
				maxWordLen_);
		}
		languageSetOnSettingDocument = resource_->languageSetForNlpResource;
		skipHighSpeed = resource_->skipHighSpeedSwitch;
		
		if (resource_->canIdentifyingHiraganaKatakana == ModTrue) {
			// prepare jakanaAnalyzer
			jakanaAnalyzer = new ModUnaMiddleAnalyzer(resource_->unaResource,
													dicSet_,
													ModTrue,
													ModTrue);
		}
	} catch (ModException& e) {
#ifdef DEBUG
		ModErrorMessage << "ModNlpAnalyzerX: " << e << ModEndl;
#endif
		delete subAnalyzer, subAnalyzer = 0;
		if (unaAnalyzer)
		{
			if (languageSetOnSettingDocument.isContained(ModLanguage::ja)
				== ModTrue) {
				unaAnalyzer->delDictionary();
			}
			delete unaAnalyzer, unaAnalyzer = 0;
		}
		if (expStr) {
			delete expStr, expStr = 0;
		}
		delete defaultStemmer, defaultStemmer = 0;
		ModRethrow(e);
	}
}

//
// FUNCTION
// ModNlpAnalyzerX::~ModNlpAnalyzerX -- デストラクタ
//
// NOTES
// 言語解析器のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
ModNlpAnalyzerX::~ModNlpAnalyzerX()
{
	// delete special analyzer for ja_kana
	if (jakanaAnalyzer) {
		delete jakanaAnalyzer;
	}

	if (unaAnalyzer) {
		delete unaAnalyzer;
	}

	delete subAnalyzer;

	if (expStr) {
		delete expStr;
	}
	delete defaultStemmer;
}

//
// FUNCTION
// ModNlpAnalyzer::set -- 処理対象テキストのセット2
//
// NOTES
// 解析対象テキストを設定する
//
// ARGUMENTS
// const ModUnicodeString& target_
//    解析対象テキスト
// const ModNlpNormMode mode_
//    解析モード
// const ModLanguageSet& languageSet_
//    言語の指定
// const ModUnicodeString& tagString_
//    タグ文字列の指定(ヌル文字列でタグ処理無しの動作となる) : 未実装
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//  言語指定に誤りがある
//
void
ModNlpAnalyzerX::set(const ModUnicodeString& target_,
					 const ModNlpNormMode mode_,
					 const ModLanguageSet& lang_,
					 const ModNlpNormModifier& normModify_)
{
	// 指定された言語セットを丸める
	ModLanguageSet rLang = lang_.round();

	// check language
	if (rLang.getSize() == 1 && rLang.isContained(ModLanguage::ja))
	{
		// 日本語のみの場合は、英語を付加
		rLang.add(ModLanguage::en);
	}
	else
	{
		if (rLang.getSize() == 0)
		{
			// 言語が空ならリソースの言語を利用
			rLang = resource->languageSetForNlpResource;
		}
		else if (resource->checkLanguage(rLang) == ModFalse)
		{
			// 処理できない言語は英語とみなす
			rLang.clear();
			rLang.add(ModLanguage::en);
		}
	}
	
	// setting to execute bug fixed process
	unaAnalyzer->emulateMaxWordOld(ModFalse);
	unaAnalyzer->setExecQuick(ModTrue);
	if (resource->canProcessJapanese == ModTrue)
	{
		// can process japanese?
		subAnalyzer->enableMetaNormTable(ModTrue,ModTrue,ModTrue,ModTrue);
	}
	else
	{
		subAnalyzer->enableMetaNormTable(ModFalse,ModFalse,ModFalse,ModTrue);
	}

	unaAnalyzer->set(target_, mode_);

	setLanguage(rLang, static_cast<ModNlpAreaTreatment>(normModify_.areaData));
}

//
// FUNCTION
// ModNlpAnalyzerX::setLanguage -- language setting
//
// NOTES
// It sets up by interpreting language specification as the language used for deployment.
//
// ARGUMENTS
// const ModLanguageSet& lang_
//    Regular-ized directions
//
// RETURN
//
// EXCEPTIONS
void
ModNlpAnalyzerX::setLanguage(const ModLanguageSet& lang_,
							 ModNlpAreaTreatment eSpace_)
{
	if (languageSetOnSettingDocument != lang_)
	{
		defaultStemmer->setLanguageSet(lang_);
		// The change of stemmer by change of language specification
		if (optionalStemmer) {
			if (lang_.isContained(ModLanguage::en)==ModTrue) {
				unaAnalyzer->setStemmer(optionalStemmer);
			} else {
				unaAnalyzer->setStemmer(defaultStemmer);
			}
		}

		// The change of tokenizer by change of language specification of
		// operation (if required dictionary load)
		if ((languageSetOnSettingDocument.isContained(ModLanguage::ja)==ModFalse
			 && lang_.isContained(ModLanguage::ja)==ModTrue)) {
			unaAnalyzer->addDictionary();
		}
		else if ((languageSetOnSettingDocument.isContained(ModLanguage::ja)
				  ==ModTrue
				  && lang_.isContained(ModLanguage::ja)==ModFalse)) {
			unaAnalyzer->delDictionary();
		}
		
		// The appointed memory
		languageSetOnSettingDocument = lang_;
	}

    // Blank processing change
    if (eSpace_ != normModify){
        if (eSpace_ == ModNlpNoNormalize){
            subAnalyzer->switchNormalizeRule(3);
        }
        else if (eSpace_ == ModNlpDelete) {
            subAnalyzer->switchNormalizeRule(4);
        }
        else if (eSpace_ == ModNlpReset) {
            subAnalyzer->switchNormalizeRule(1);
        }
        normModify = eSpace_;
    }
}

//
// FUNCTION public
//	ModNlpAnalyzerX::isExpStrDataLoad -- Whether the data for expanding strings is loaded is confirmed.
//
// NOTES
//	This function is confirmed whether the expanding data set by the parameter was loaded.
//	It is executed before the acquisition function of the expanded strings result is called.
//	If the return value is ModTrue, the acquisition function is called.
//
// ARGUMENTS
//
// RETURN
//	ModBoolean
//		ModTrue		The data of the expanding function set by the parameter was loaded.
//		ModFalse	The data of the expanding function set by the parameter was not loaded.
//
// EXCEPTIONS
//
ModBoolean
ModNlpAnalyzerX::isExpStrDicLoad()
{
	return resource->normRule->isExpStrDicLoad();
}

//
// FUNCTION public
//	ModNlpAnalyzerX::isExpStrStrDicLoad -- Whether the user data for expanding strings is loaded is confirmed.
//
// NOTES
//	This function is confirmed whether the user expanding data set by the parameter was loaded.
//	It is executed before the acquisition function of the expanded strings result
//	is called.
//	If the return value is ModTrue, the acquisition function is called.
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue		The data of the user expanding function set by the parameter was loaded.
//		ModFalse	The data of the user expanding function set by the parameter was not loaded.
//
// EXCEPTIONS
//
ModBoolean
ModNlpAnalyzerX::isExpStrStrDicLoad()
{
	return resource->normRule->isExpStrStrDicLoad();
}

//
// FUNCTION public
//	ModNlpAnalyzerX::isExpStrMorDicLoad -- Whether the system data for expanding strings is loaded is confirmed.
//
// NOTES
//	This function is confirmed whether the system expanding data set by the parameter was loaded.
//	It is executed before the acquisition function of the expanded strings result
//	is called.
//	If the return value is ModTrue, the acquisition function is called.
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue		The data of the system expanding function set by the parameter was loaded.
//		ModFalse	The data of the system expanding function set by the parameter was not loaded.
//
// EXCEPTIONS
//
ModBoolean
ModNlpAnalyzerX::isExpStrMorDicLoad()
{
	return resource->normRule->isExpStrMorDicLoad();
}

//
// FUNCTION
// ModNlpAnalyzer::getWord -- 単語検出結果の取得
//
// NOTES
// 形態素解析結果１単語を原表記付きで取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    検出結果
// ModUnicodeString& original_
//    元表記
// const ModBoolean normalize_
//    正規化指示
//
// RETURN
// 解析対象テキストが残っていればModTrue, なければModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getWord(ModUnicodeString& result_,
						 ModUnicodeString& original_,
						 const ModBoolean normalize_)
{
	return unaAnalyzer->getMorph(result_, original_, normalize_);
}

//
// FUNCTION
// ModNlpAnalyzer::getWord -- 単語検出結果の取得
//
// NOTES
// 形態素解析結果１単語を異表記正規化し取得する
//
// ARGUMENTS
// ModUnicodeString &normalized_
//    異表記正規化済み表記
// ModUnicodeChar* &original_
//    原表記開始位置へのポインタ
// ModSize &len_
//    原表記の長さ
// int &pos_
//    形態素品詞番号
//
// RETURN
// 解析対象テキストが残っていればModTrue, なければModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getWord(ModUnicodeString &normalized_,
						 ModUnicodeChar* &original_, ModSize &len_, int &pos_)
{
	return unaAnalyzer->getMorph(normalized_, original_, len_, pos_);
}

//
// FUNCTION
// ModNlpAnalyzer::getWord -- 単語検出結果の取得
//
// NOTES
// 形態素解析結果１単語を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    検出結果
// const ModBoolean normalize_
//    正規化指示
//
// RETURN
// 解析対象テキストが残っていればModTrue, なければModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getWord(ModUnicodeString& result_, const ModBoolean normalize_)
{
	return unaAnalyzer->getMorph(result_, normalize_);
}

///
// FUNCTION
// ModNlpAnalyzer::getExpandWords -- 単語を取得し、展開して返す
//
// NOTES
// 単語検出結果を取得し、展開して返す
// 呼ぶ側で、delete [] expanded_;をしないと、メモリリークが発生する。
//
// ARGUMENTS
// ModUnicodeString& expanded_
//    展開した解析結果
// ModUnicodeString& ostr
//
// int& pos_
//
// const ModNlpExpMode chkOrg_
//    展開の指示(展開モード)
//
// RETURN
// expanded_配列の要素数
//
// EXCEPTIONS
ModSize
ModNlpAnalyzerX::getExpandWords(ModUnicodeString*& expanded_,
								ModUnicodeString& ostr,
								int& pos_,
								const ModNlpExpMode chkOrg_)
{
	if (resource->canIdentifyingHiraganaKatakana == ModTrue)
	{
		ModSize rv;
		ModUnicodeCharType r;

		// Normal processing
		ostr.clear();
		if ( ModNlpExpNoChk == chkOrg_ ){
			rv = unaAnalyzer->getExpand(expanded_,ModUnaExpNoChk,ostr,pos_);
		}
		else{
			rv = unaAnalyzer->getExpand(expanded_,ModUnaExpChkOrigStr,ostr,pos_);
		}
		if (rv==0){
			return 0;
		}

		// check if dataset==ja_kana and problem characters(hiragana,katakana)
		r = ModUnicodeCharTrait::getType(ostr[0]);

		ModUnicodeCharType ukanamask = ModUnicodeCharTypes::hiragana | ModUnicodeCharTypes::katakana | ModUnicodeCharTypes::hankakuKana;
		if ((ModUnicodeCharTrait::getType(ostr[0])&ukanamask) != 0){
			if (ModUnicodeCharTrait::isHankakuKana(ostr[0])){
				// 30a2 = fullwidth katakana: means hankaku katakana is treated zenkaku
				r = ModUnicodeCharTrait::getType( ModUnicodeChar(0x30a2));
			}
			; // 341
		}
		else{
			return rv;
		}
		{
			// -- special processing for ja_kana problem characters
			delete [] expanded_;

			// step1:make whole original string (series of katakana/hiragana)
			ModUnicodeString norm;
			ModUnicodeCharType pr(r);

			ModUnicodeChar tuc;
			//ostr.clear();

			while (1){
				tuc = unaAnalyzer->checkNextChar();
				if ( tuc == 0x30fc || tuc == 0x002d || tuc == 0x2010 ||
					 tuc == 0x2011 || tuc == 0x2015 || tuc == 0x207b ||
					 tuc == 0x207b || tuc == 0x208b || tuc == 0x208b ||
					 tuc == 0x2212 || tuc == 0xfe63 || tuc == 0xff0d ){
					r = pr;

				} // prolong symbols
				else{
					r = ModUnicodeCharTrait::getType(tuc);
				}
				if ( (pr&ukanamask) != (r&ukanamask)){
					break;
				}
				{
					ModUnicodeChar *subOstrStart;
					ModSize subOstrLen;
					int pos;

					if ( getWord( norm, subOstrStart, subOstrLen, pos) == ModFalse){ // end of text
						break;
					}
					ostr.append(subOstrStart, subOstrLen);
				}
			}
			if (ostr.getLength() == 0){ // return 0 if no original string
				return 0;
			}

			// step2:get first expanded results
			ModSize en;
			ModSize en2;
			ModUnicodeString* expanded2;

			try {
				if ( ModNlpExpNoChk == chkOrg_ ){
					en2 = subAnalyzer->expandBuf(ostr, expanded2, ModNormExpNoChk);
				}
				else{
					en2 = subAnalyzer->expandBuf(ostr, expanded2, ModNormExpChkOrigStr);
				}

				// step3:make addstr1 as an additional result
				ModUnicodeString tnorm;
				ModUnicodeString addstr1;
				ModUnicodeString rstr;
				ModSize i;

				jakanaAnalyzer->set( ostr, 1);
				addstr1.clear();

				while (1){
					if ( jakanaAnalyzer->get( rstr, ModTrue) == ModFalse){
						break;
					}
					tnorm = subAnalyzer->normalizeBuf(rstr, 0, 0, ModNormalized);
					addstr1 += tnorm;
				}

				// step4:make addstr2 as an additional result
				// step4-1:make ostr2 with hiragana <-> katakana replacement
				ModUnicodeString ostr2;
				ostr2.reallocate(ostr.getLength()+1);
				ModBoolean isOstr2Hiragana(ModTrue);
				ostr2.clear();

				const ModUnicodeChar* p= ostr;
				for (;*p!=0;p++){
					if ( (*p >= 0x3041) && ( *p <=0x3094)){ // hiragana
						ostr2 += ModUnicodeChar((*p -0x3041)+0x30a1);
						isOstr2Hiragana = ModFalse;
					}
					else if ( (*p >= 0x30a1) && (*p <= 0x30fa)){ // katakana
						if ( *p >= 0x30f7){ // katakana special charactors
							ostr2 += ModUnicodeChar((*p-0x30f7)+0x308f);
							ostr2 += ModUnicodeChar(0x309b);
						}
						else{
							ostr2 += ModUnicodeChar((*p-0x30a1) + 0x3041);
						}
					}
					else if ( (*p >= 0xff66) && (*p <= 0xff9f)){ // hankaku
						ModBoolean nextDakuten(ModFalse);
						ModBoolean nextHandakuten(ModFalse);
						if (*(p+1) == 0xff9e){
							nextDakuten = ModTrue;
						}
						else if (*(p+1) == 0xff9f){
							nextHandakuten = ModTrue;
						}
						if ( *p == 0xff66){ // wo
							ostr2 += ModUnicodeChar(0x3092);
						}
						else if ( (*p >= 0xff67) && (*p <= 0xff6b)){ // small a-o
							ostr2 += ModUnicodeChar((*p - 0xff67)*2 + 0x3041);
						}
						else if ( (*p >= 0xff6c) && (*p <= 0xff6e)){ // small ya-yo
							ostr2 += ModUnicodeChar((*p - 0xff6c)*2 + 0x3083);
						}
						else if ( *p == 0xff6f){ // small tsu
							ostr2 += ModUnicodeChar(0x3063);
						}
						else if ( *p == 0xff70){ // prolonged symbol
							ostr2 += ModUnicodeChar(0x30fc);
						}
						else if ( (*p >= 0xff71) && (*p <= 0xff75)){ //  a-o
							if ( nextDakuten && *p == 0xff73){ //  vu
								ostr2 += ModUnicodeChar(0x3094);
								p++;
							}
							else{
								ostr2 += ModUnicodeChar((*p - 0xff71)*2 + 0x3042);
							}
						}
						else if ( (*p >= 0xff76) && (*p <= 0xff81)){ //  ka-chi
							if ( nextDakuten){ //  dakuten
								ostr2 += ModUnicodeChar((*p - 0xff76)*2 + 0x304c);
								p++;
							}
							else{
								ostr2 += ModUnicodeChar((*p - 0xff76)*2 + 0x304b);
							}
						}
						else if ( (*p >= 0xff82) && (*p <= 0xff84)){ //  tsu-to
							if ( nextDakuten){ //  dakuten
								ostr2 += ModUnicodeChar((*p - 0xff82)*2 + 0x3065);
								p++;
							}
							else{
								ostr2 += ModUnicodeChar((*p - 0xff82)*2 + 0x3064);
							}
						}
						else if ( (*p >= 0xff85) && (*p <= 0xff89)){ //  na-no
							ostr2 += ModUnicodeChar((*p - 0xff85) + 0x306a);
						}
						else if ( (*p >= 0xff8a) && (*p <= 0xff8e)){ //  ha-ho
							if (  nextDakuten){ //  dakuten
								ostr2 += ModUnicodeChar((*p - 0xff8a)*3 + 0x3070);
								p++;
							}
							else if ( nextHandakuten){ //  han-dakuten
								ostr2 += ModUnicodeChar((*p - 0xff8a)*3 + 0x3071);
								p++;
							}
							else{
								ostr2 += ModUnicodeChar((*p - 0xff8a)*3 + 0x306f);
							}
						}
						else if ( (*p >= 0xff8f) && (*p <= 0xff93)){ //  ma-mo
							ostr2 += ModUnicodeChar((*p - 0xff8f) + 0x307e);
						}
						else if ( (*p >= 0xff94) && (*p <= 0xff96)){ //  ya-yo
							ostr2 += ModUnicodeChar((*p - 0xff94)*2 + 0x3084);
						}
						else if ( (*p >= 0xff97) && (*p <= 0xff9b)){ //  ra-ro
							ostr2 += ModUnicodeChar((*p - 0xff97) + 0x3089);
						}
						else if ( *p == 0xff9c){ //  wa
							ostr2 += ModUnicodeChar(0x308f);
						}
						else if ( *p == 0xff9d){ //  nn
							ostr2 += ModUnicodeChar(0x3093);
						}
						else if ( *p == 0xff9e){ //  dakuten
							ostr2 += ModUnicodeChar(0x309b);
						}
						else if ( *p == 0xff9f){ //  han-dakuten
							ostr2 += ModUnicodeChar(0x309c);
						}
					}
					else{ //others
						ostr2 += *p;
					}
				}

				// step4-2:make addstr2 as an additional result
				ModUnicodeString addstr2;
				jakanaAnalyzer->set( ostr2, 1);
				addstr2.clear();

				while (1){
					if ( jakanaAnalyzer->getMorph( rstr, ModFalse) == ModFalse){
						break;
					}
					tnorm = subAnalyzer->normalizeBuf(rstr, 0, 0, ModNormalized);
					addstr2 += tnorm;
				}

				// step5: check duplicated string
				// we have (1)expanded2 (2)addstr1 and (3)addstr2
				ModSize addstr1_add(1);
				ModSize addstr2_add(1);

				for ( i=0; i < en2; ++i){
					if (addstr1 == expanded2[i]){
						addstr1_add = 0;
					}
					if (addstr2 == expanded2[i]){
						addstr2_add = 0;
					}
				}
				if ( addstr1 == addstr2){
					addstr2_add = 0;
				}

				// step6: make results

				en = en2 + addstr1_add + addstr2_add;
				expanded_ = new ModUnicodeString[en];

				// insert expanded items
				ModSize x;
				for (x =0; x<en2; ++x){
					expanded_[x] = expanded2[x];
				}
	 			// keep the order of insertion katakana -> hiragana
				if ( isOstr2Hiragana ){
					if ( addstr1_add > 0){
						expanded_[x++] = addstr1;
					}
					if ( addstr2_add > 0){
						expanded_[x++] = addstr2;
					}
				}
				else{
					if ( addstr1_add > 0){
						expanded_[x++] = addstr1;
					}
					if ( addstr2_add > 0){
						expanded_[x++] = addstr2;
					}
				}
				// delete temporaly buffer
				if ( en2>0){
					delete [] expanded2;
				}
			} catch (ModException&e) {
				// delete temporaly buffer
				if ( en2>0){
					delete [] expanded2;
				}
				ModRethrow(e);
			}

			// return number of buffer items
			return en;
		}
	}

	// normal processing
	if ( ModNlpExpNoChk == chkOrg_ ){
		return unaAnalyzer->getExpand(expanded_,ModUnaExpNoChk,ostr,pos_);
	}
	return unaAnalyzer->getExpand(expanded_,ModUnaExpChkOrigStr,ostr,pos_);
}

//
// FUNCTION
// ModNlpAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 構文解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModUnicodeString& result_
//    解析結果の情報
// const ModUnicodeChar sep1_
//    セパレータ１（フィールド区切り）
// const ModUnicodeChar sep2_
//    セパレータ２（レコード区切り）
// const ModBoolean normalize_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getBlock(ModUnicodeString& result_,
	const ModUnicodeChar separator1_, const ModUnicodeChar separator2_,
	const ModBoolean normalize_)
{
	return unaAnalyzer->getBlock(result_, separator1_, separator2_, normalize_);
}

//
// FUNCTION
// ModNlpAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 構文解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// const ModBoolean normalize_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getBlock( ModVector<ModUnicodeString>& formVector_,
	ModVector<int>& posVector_, const ModBoolean normalize_)
{
	return unaAnalyzer->getBlock(formVector_, posVector_, normalize_);
}

//
// FUNCTION
// ModNlpAnalyzer::getBlock -- 解析結果の取得
//
// NOTES
// 構文解析したブロックごとの結果を取得する
//
// ARGUMENTS
// ModVector<ModUnicodeString>& formVector_
//    解析結果の表記ベクター
// ModVector<ModUnicodeString>& ostrVector_
//    解析結果の原表記ベクター
// ModVector<int>& posVector_
//    解析結果の品詞ベクター
// const ModBoolean normalize_
//    正規化指示
//
// RETURN
// 解析結果が残っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getBlock( ModVector<ModUnicodeString>& formVector_,
	ModVector<ModUnicodeString>& ostrVector_, ModVector<int>& posVector_,
	const ModBoolean normalize_,
	ModVector<int> *costVector_,
	ModVector<int> *uposVector_,
	ModBoolean ignore_)
{
	return unaAnalyzer->getBlock(formVector_, ostrVector_, posVector_, normalize_, costVector_, uposVector_, ignore_);
}

//
// FUNCTION
// ModNlpAnalyzer::getDicName -- 辞書ベース名の取得
//
// NOTES
// 形態素解析したブロックごとの辞書ベース名を取得する
// 形態素と辞書ベース名の個数および位置は一対一対応している
//
// ARGUMENTS
// ModVector<ModUnicodeString>& dicNameVector_
//    辞書ベース名のベクター
//
// RETURN
// 解析結果があれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
ModBoolean
ModNlpAnalyzerX::getDicName( ModVector<ModUnicodeString>& dicNameVector_)
{
	return unaAnalyzer->getDicName(dicNameVector_);
}

//
// FUNCTION
// ModNlpAnalyzerX::getNormalizeBuf -- wrapper of normalizeBuf
//
// NOTES
// wrapper of normalizeBuf
//
// ARGUMENTS
// const ModUnicodeString& cstrInputStr_
//	 Input string
// ModUnicodeString& cstrOutputStr_
//	 Output string
// ModSize ulLenOfNormalization_
//	 Length of normalization
//
// RETURN
//
// EXCEPTIONS
void
ModNlpAnalyzerX::getNormalizeBuf(const ModUnicodeString& cstrInputStr_,
								 ModUnicodeString& cstrOutputStr_,
								 ModSize ulLenOfNormalization_)
{
	subAnalyzer->normalizeBuf(cstrInputStr_, cstrOutputStr_,
								0, ulLenOfNormalization_, ModNormalized);
}

//
// FUNCTION
// ModNlpAnalyzerX::getExpandBuf -- wrapper of expandBuf
//
// NOTES
// wrapper of expandBuf
//
// ARGUMENTS
// const ModUnicodeString& cstrInputStr_
//	 Input string
// ModUnicodeString*& pcstrExpanded_
//	 Expanded string
// ModSize ulLenOfNormalization_
//	 Length of normalization
// ModBoolean eExpandOnly_
//	 ModTrue : expand only
//	 ModFalse: normalize and expand
//
// RETURN
//	 count of expanded
//
// EXCEPTIONS
ModSize
ModNlpAnalyzerX::getExpandBuf(const ModUnicodeString& cstrInputStr_,
							  ModUnicodeString*& pcstrExpanded_,
							  ModSize ulLenOfNormalization_,
							  ModBoolean eExpandOnly_)
{
	return subAnalyzer->expandBuf(cstrInputStr_,
								  pcstrExpanded_,
								  ModNormExpNoChk,
								  0,
								  ulLenOfNormalization_,
								  eExpandOnly_);
}

//
// FUNCTION public
//	ModNlpAnalyzerX::getExpandStrings -- get expanded strings patterns for morphem match
//
// NOTES
//  This function is used to get expanded strings patterns of ModNlpExpStr::expandStrings
//  only in the ModNlpUnaJp module.
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& expanded_
//		O: expanded strings patterns
//	ModSize maxExpPatternNum_
//		I: number of maximum expanded character string patterns
//
// RETURN
//	ModSize
//		number of expanded strings patterns
//
// EXCEPTIONS
//	exception of lower modules
//
ModSize
ModNlpAnalyzerX::getExpandStrings(ModVector<ModUnicodeString>& expanded_, ModSize maxExpPatternNum_)
{
	// The morphological analysis result is obtained.
	// It always don't normalize it.
	ModBoolean doNorm = ModFalse;

	// Normalization for the synonym expanding is set.
	subAnalyzer->setExpStrMode(ModTrue);

	ModVector<ModUnicodeString> normVector;
	ModVector<int> posVector;
	ModVector<ModUnicodeString> formVector;
	formVector.clear();

	while(1){
		normVector.clear();
		posVector.clear();
		ModBoolean rv = unaAnalyzer->getBlock(normVector, posVector, doNorm);

		ModVector<ModUnicodeString>::Iterator normVectorIte = normVector.begin();
		while(normVectorIte != normVector.end()){
			formVector.pushBack(subAnalyzer->normalizeBuf(*normVectorIte++, 0, 0, ModNormalized));
		}

		// the analytical result doesn't remain
		if(rv == ModFalse) {
			break;
		}
	}

	// Normalization for the synonym expanding is reset.
	subAnalyzer->setExpStrMode(ModFalse);

	// get expanded strings patterns
	return expStr->expandStrings(formVector, expanded_, maxExpPatternNum_);
}

//
// FUNCTION public
//	ModNlpAnalyzerX::getExpandStrings -- get expanded strings patterns for string match
//
// NOTES
//  This function is used to get expanded strings patterns of ModNlpExpStr::expandStrings
//  only in the ModNlpUnaJp module.
//
// ARGUMENTS
//	const ModUnicodeString& cstrInputStr_
//		I: input string
//	ModVector<ModUnicodeString>& expanded_
//		O: expanded strings patterns
//	ModSize maxExpPatternNum_
//		O: :number of maximum expanded character string patterns
//
// RETURN
//	ModSize
//		number of expanded strings patterns
//
// EXCEPTIONS
//	exception of lower modules
//
ModSize
ModNlpAnalyzerX::getExpandStrings(const ModUnicodeString& cstrInputStr_, 
									ModVector<ModUnicodeString>& expanded_, ModSize maxExpPatternNum_)
{
	// search expanded strings from expand data
	ModVector<ModUnicodeString> formVector;
	const ModUnicodeChar* input_str = cstrInputStr_;
	ModUnicodeString vecString;
	ModSize input_len(cstrInputStr_.getLength());
	ModSize vecPushCount;

	ModBoolean normFlag_ = ModTrue;
	
	formVector.clear();
	for(vecPushCount = 0; vecPushCount < input_len; vecPushCount++){
		vecString = input_str[vecPushCount];
		formVector.pushBack(vecString);
	}

	return expStr->expandStrings(formVector, expanded_, maxExpPatternNum_, normFlag_);
}

ModTermResource*
ModNlpAnalyzerX::getTermResource()
{
	return resource->termResource;
}

// FUNCTION
// ModNlpResourceX::ModNlpResourceX -- Constructor1
//
// NOTES
//
// ARGUMENTS
// const ModCharString& path_  -- resource directory path
// const ModLanguageSet& lang_ -- language setting
//
// RETURN
// None
//
// EXCEPTIONS
//
ModNlpResourceX::ModNlpResourceX(const ModUnicodeString& path_,
								 const ModLanguageSet& lang_,
								 ModSize memReduceLevel_)
	: unaResource(0), normRule(0), engWordStemmer(0),
	  memSwitch(ModFalse), termResource(0)
{
	this->setResource(path_, lang_, memReduceLevel_);
}

ModNlpNormModifier::ModNlpNormModifier()
:areaData(0)
{}

ModNlpNormModifier::~ModNlpNormModifier()
{}

void
ModNlpNormModifier::space(ModNlpAreaTreatment x)
{
	areaData=x;
}

//
// Copyright (c) 2001, 2009, 2010, 2023 RICOH Company, Ltd.
// All rights reserved.
//
