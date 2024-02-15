// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLP.h -- Hedaer file of interface
// 
// Copyright (c) 2001, 2005-2010, 2023 Ricoh Company, Ltd.
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
#ifndef __UNA_MODNLP_H
#define __UNA_MODNLP_H
#define UNA_FUNCTION

#include "UNA_UNIFY_TAG.h"
#include "ModCriticalSection.h"
#include "ModCommonDLL.h"
#include "ModConfig.h"
#include "ModDefaultManager.h"
#include "ModLanguageSet.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "ModMap.h"

#include "ModNLPText.h"
#include "UnaVersion.h"

namespace UNA {

class Morph;
class Keyword;
class ModNlpLocalResource;
class ModNlpLocalAnalyzer;

#define DLLNAME(_x_) _x_ DLLSUFFIX
class ModNlpLocal;

	class UNA_FUNCTION ModNlp : public ModDefaultObject
	{

	public:
		static ModNlpLocal* create(int i_);
		static void initialize();
		static void terminate();
		ModNlp();
		~ModNlp();
		static void loadLibrary(const ModUnicodeString& cstrBaseName_);
		static void freeLibrary();
		static void* getProcAddress(const ModUnicodeString& cstrBaseName_,const char* pszFuncName_);
	};


//
// CLASS
//	ModNlpResource -- resource class
//
// NOTES
//	A resource class used by ModNlpAnalyzer
//
	class ModNlpResource : public ModDefaultObject
	{

		friend class ModNlpAnalyzer;
	
	public:
//
// FUNCTION public
//	ModNlpResource::ModNlpResource -- constructor
//
// NOTES
//	default constructor of ModNlpResource
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class constructor.
*/
		UNA_FUNCTION ModNlpResource();
//
// FUNCTION public
//	ModNlpResource::~ModNlpResource -- destructor
//
// NOTES
//	destructor of ModNlpResource
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class destructor.
*/
		UNA_FUNCTION  virtual ~ModNlpResource();
//
// FUNCTION public
//	ModNlpResource::getResource -- return a local resource contained inside resource
//
// NOTES
//	ModNlpResource has some local(internal) resources.
//	Never use this method in applications.
//	Behavior will be unsettled if a bad resouce number is set in argument.
//
// ARGUMENTS
//	ModSize resourceNum
//		I: resource number(0 base)
//
// RETURN
//	ModNlpLocalResource*
//		local resource
//
// EXCEPTIONS
//	none
//
/*!
	return a local resource contained inside resource.
	Notes: ModNlpResource has some local(internal) resources.
					Never use this method in applications.
					Behavior will be unsettled if a bad resouce number is set in argument.
	@param[in]	resourceNum	 resource number(0 base)
	\return	ModNlpLocalResource*  local resource
*/
		ModNlpLocalResource* getResource(ModSize resourceNum);

//
// FUNCTION public
//	ModNlpResource::load -- ModNlpLocalResource is created
//
// NOTES
//	set resource path and memory save switch and ModNlpLocalResource is created
//
// ARGUMENTS
//	const ModUnicodeString& resourcePath, 
//		I: resource path
//	const ModBoolean memorySave
//		I: memory save switch
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	ModNlpLocalResource is created.
	Notes: set resource path and memory save switch and ModNlpLocalResource is created.
	@param[in]	resourcePath	 resource path
	@param[in]	memorySave	 memory save switch
	\return	ModTrue
*/
		UNA_FUNCTION
  		ModBoolean load(
			const ModUnicodeString& resourcePath, 
			const ModBoolean memorySave);
//
// FUNCTION public
//	ModNlpResource::unload -- ModNlpLocalResource is destroyed
//
// NOTES
//	ModNlpLocalResource is destroyed
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue		ModNlpLocalResource is destroyed successfully
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	ModNlpLocalResource is destroyed.
	\return	ModTrue
*/
		UNA_FUNCTION ModBoolean unload();


	private:
//
// FUNCTION private
//	ModNlpResource::getAnalyzerList -- get analyzer list
//
// NOTES
//	get analyzer list
//
// ARGUMENTS
//	const ModUnicodeString& resourcePath_
//		resource path
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
		ModBoolean getAnalyzerList(const ModUnicodeString& resourcePath_);

		ModVector<ModNlpLocalResource*> m_vecResources;
		ModVector<int> m_vecFactoryNo;
	};

	class DicSet;

//
// CLASS
//	ModNlpAnalyzer -- analyzer class
//
// NOTES
//	This class is for analyzing documents.
//
	class UNA_FUNCTION ModNlpAnalyzer : public ModDefaultObject
	{
	public:
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ENUM
// ExpandResult -- 同義語展開結果のステータスの種類
//
// NOTES
// この列挙型は同義語展開結果のステータスを示す。
// 
		enum ExpandResult {
			ExpandResultNone,				// 同義語展開パターンなし
			ExpandResultExist,				// 同義語展開パターンを取得
			ExpandResultMaxExpTargetStrLen,	// 同義語展開対象文字列長の上限による同義語展開パターンの取得失敗
			ExpandResultMaxExpPatternNum	// 同義語展開パターン総数の上限による同義語展開パターンの取得失敗
		};

//
// FUNCTION public
//	ModNlpAnalyzer::ModNlpAnalyzer -- constructor
//
// NOTES
//	default constructor of ModNlpAnalyzer
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class constructor.
*/
		ModNlpAnalyzer();

//
// FUNCTION public
//	ModNlpAnalyzer::~ModNlpAnalyzer -- destructor
//
// NOTES
//	destructor of ModNlpAnalyzer
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	none
//
/*!
	Class destructor.
*/
		virtual ~ModNlpAnalyzer();

//
// FUNCTION public
//	ModNlpAnalyzer::setResource -- set nlp resource
//
// NOTES
//	set nlp resource needed to analyze
//
// ARGUMENTS
//	ModNlpResource* resource
//		I: resource pointer to instance of ModNlpLocalResource
//
// RETURN
//	ModBoolean
//		ModTrue		set successfully
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Set resource for analyzer.
	@param[in]	resource pointer to instance of ModNlpLocalResource
	\return	whether set successfully
*/
		ModBoolean setResource(ModNlpResource* resource);

//
// FUNCTION public
//	ModNlpAnalyzer::releaseResource -- release nlp resource
//
// NOTES
//	release nlp resource set by setResource method.
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue		resource is released successfully.
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Release resource.
	\return whether resource is released successfully.
*/
		ModBoolean releaseResource();

//
// FUNCTION public
//	ModNlpAnalyzer::prepare -- set language and parameters
//
// NOTES
//	set language and parameters
//
// ARGUMENTS
//	const ModLanguageSet& lang
//		I: language
//	const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >& parametersToModifyAnalyze
//		I: parameters for processing
//
// RETURN
//	ModBoolean
//		ModTrue		set successfully.
//
// EXCEPTIONS
//	exception of lower modules
//
		typedef ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > Parameters;
/*!
	Set language and parameters.
	@param[in]	parametersToModifyAnalyze		parameters for processing
	\return whether set successfully.
*/
		ModBoolean prepare(const Parameters& parametersToModifyAnalyze);

//
// FUNCTION public
//	ModNlpAnalyzer::set -- set target document
//
// NOTES
//	set target document
//
// ARGUMENTS
//	const ModUnicodeString& target
//		I: target document for analyzing
//
// RETURN
//	ModBoolean
//		ModTrue		set successfully.
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Set target document.
	@param[in]	target	target document for analyzing
	\return whether set successfully.
*/
		ModBoolean set(	const ModUnicodeString& target,
						const ModLanguageSet& lang);

//
// FUNCTION public
//	ModNlpAnalyzer::isExpStrDataLoad -- Whether the data for expanding function is loaded is confirmed.
//
// NOTES
//	This function is confirmed whether the expanding data set by the parameter was loaded.
//	It is executed before the acquisition function of the expanded strings result
//	is called.
//	If the return value is ModTrue, the acquisition function is called.
//
// ARGUMENTS
//	It is not.
//
// RETURN
//	ModBoolean
//		ModTrue		The data for the expanding function set by the parameter was loaded.
//		ModFalse	The data for the expanding function set by the parameter was not loaded.
//
// EXCEPTIONS
//	exception of lower modules
//
		ModBoolean isExpStrDataLoad();

//
// FUNCTION public
//	ModNlpAnalyzer::getWord -- get a word from document in turn
//
// NOTES
//	get a word from document in turn
//
// ARGUMENTS
//	ModUnicodeString& normalized
//		O: normalized word
//	ModUnicodeString& original
//		O: original word
//	int& pos
//		O: part of speech
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get a word from document in turn.
	@param[out]	normalized	normalized word
	@param[out]	original	original word
	@param[out]	part of speech
	\return true if more words still remain.
					return false if no more words.
*/
		ModBoolean getWord(	ModUnicodeString& normalized);

		ModBoolean getWord(	ModUnicodeString& normalized,
							ModUnicodeString& original);

		ModBoolean getWord(	ModUnicodeString& normalized, 
							ModUnicodeString& original, 
							int& pos);

//
// FUNCTION public
//	ModNlpAnalyzer::getExpandWords -- get expanded words from document in turn
//
// NOTES
//	get expanded words from document in turn
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& expanded
//		O: expanded word
//	ModUnicodeString& original
//		O: original word
//	int& pos
//		O: part of speech
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get expanded words from document in turn.
	@param[out]	expanded  expanded word
	@param[out]	original	original word
	@param[out]	pos	whole  part of speech
	\return true if more words still remain,
					return false if there is no more words.
*/
		ModBoolean getExpandWords(	ModVector<ModUnicodeString>& expanded,
									ModUnicodeString& original,
									int& pos);

//
// FUNCTION public
//	ModNlpAnalyzer::getBlock -- get words from document together
//
// NOTES
//	get words from document together
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& normVector
//		O: vector including normalized words
//	ModVector<ModUnicodeString>& origVector
//		O: vector including original words
//	ModVector<int>& posvector
//		O: vector including part of speeches
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get words from document together
	@param[out]	normVector  vector including normalized words
	@param[out]	origVector	vector including original words
	@param[out]	posvector	 vector including part of speeches
	\return true if more words still remain,
					return false if there is no more words.
*/
		ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<int>& posvector);

		ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& origVector,
							ModVector<int>& posvector);

		ModBoolean getBlock(ModUnicodeString& result,
							ModUnicodeChar sep1, ModUnicodeChar sep2);

		ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& hinVector);

//
// FUNCTION public
//	ModNlpAnalyzer::getDicName -- get dictionary base name for each morpheme
//
// NOTES
//	get dictionary names for each morpheme which getBlock() returned
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& dicNameVector_
//		O: vector including dictionary base names
//
// RETURN
//	ModBoolean
//		ModTrue:  result found
//		ModFalse: no result found
//
// EXCEPTIONS
//
		ModBoolean getDicName(ModVector<ModUnicodeString>& dicNameVector_);

//
// FUNCTION public
//	ModNlpAnalyzer::getBlock -- get words from document together
//
// NOTES
//	get words from document together
//
// ARGUMENTS
// get a word-cost and UNA unified tag verision
//
//	ModVector<ModUnicodeString>& normVector
//		O: vector including normalized words
//	ModVector<ModUnicodeString>& origVector
//		O: vector including original words
//	ModVector<int>& posvector
//		O: vector including part of speeches
//	ModVector<int>& costVector
//		O: vector including cost of words
//	ModVector<int>& uposVector
//		O: vector including UNA unified tag
//	ModBoolean ignore
//		I: ModTrue: the character string deleted by regularization
//					is disregarded. Default setting.
//		   ModFalse:it doesn't disregard it. 
//
// RETURN
//	ModBoolean
//		ModTrue:  more words still remain.
//		ModFalse: no more words
//
// EXCEPTIONS
//	exception of lower modules
//
		ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& origVector,
							ModVector<int>& posvector,
							ModVector<int>& costVector,
							ModVector<int>& uposVector);

		ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& origVector,
							ModVector<int>& posvector,
							ModVector<int>& costVector,
							ModVector<int>& uposVector,
							ModBoolean ignore);

//
// FUNCTION public
//	ModNlpAnalyzer::getConcept -- get one NounPhrase from the original document
//
// NOTES
//	get one NounPhrase from the original document
//
// ARGUMENTS
//	ModUnicodeString& extractedConcept
//		O: the original keyword
//	ModVector<ModUnicodeString>& normVector
//		O: the normalized words vector in one NounPhrase
//	ModVector<ModUnicodeString>& origVector
//		O: the original words vector in one NounPhrase
//	ModVector<int>& posVector
//		O: the part of speech vector of each words in one NounPhrase
//
// RETURN
//	ModBoolean
//		ModTrue : success to get one NounPhrase vector
//		ModFalse: finish processing, no more nourphrases
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	Get noun phrase from target document.
	@param[out]	extractedConcept	the original keyword
	@param[out]	normVector	the normalized words vector in one NounPhrase
	@param[out]	origVector	the original words vector in one NounPhrase
	@param[out]	posVector		the part of speech vector of each words in one NounPhrase
	\return true if success to get one NounPhrase vector
					return false if finish processing, no more nourphrases
*/
		ModBoolean getConcept(ModUnicodeString& extractedConcept,
							ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& origVector,
							ModVector<int>& posVector);

//
// FUNCTION public
//	ModNlpAnalyzer::getConcept -- get one NounPhrase from the original document
//
// NOTES
//	get one NounPhrase from the original document
//
// ARGUMENTS
//	ModUnicodeString& normalized
//		O: the normalized NounPhrase
//	ModUnicodeString& original
//		O: the original NounPhrase
//	double& npCost
//		O: the NounPhrase cost
//	ModVector<ModUnicodeString>& normVector
//		O: the normalized words vector in one NounPhrase
//	ModVector<ModUnicodeString>& origVector
//		O: the original words vector in one NounPhrase
//	ModVector<int>& posVector
//		O: the part of speech vector of each words in one NounPhrase
//
// RETURN
//	ModBoolean
//		ModTrue : success to get one NounPhrase vector
//		ModFalse: finish processing, no more nourphrases
//
// EXCEPTIONS
//	exception of lower modules
//
		ModBoolean getConcept(ModUnicodeString& normalized,
							ModUnicodeString& original,
							double& npCost,
							ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& origVector,
							ModVector<int>& posVector);

		ModBoolean getConcept(ModUnicodeString& normalized,
							ModUnicodeString& original,
							double& npCost);

//
//  getConceptImpはUnaJpから呼ばれる関数
//  この関数は、LibUna.DLLのexport関数になる
		ModBoolean getConceptImp(ModUnicodeString& keyword_,
							ModVector<ModUnicodeString>& normVector_,
							ModVector<ModUnicodeString>& orgVector_,
							ModVector<int>& posVector_,
							ModSize& len_);

		ModBoolean getConceptImp(ModUnicodeString& normalized_,
							ModUnicodeString& original_,
							double& npCost_,
							ModSize& len_,
							ModVector<ModUnicodeString>* normVector_ = 0,
							ModVector<ModUnicodeString>* orgVector_ = 0,
							ModVector<int>* posVector_ = 0);

//
// FUNCTION public
//	ModNlpAnalyzer::getWholeText -- get whole text
//
// NOTES
//	this method returns normalized text created by joining words got from getWord.
//
// ARGUMENTS
//	ModUnicodeString& wholeText
//		O: whole text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get whole text.
	this method returns normalized text created by joining words got from getWord.
	@param[out] wholeText  whole text
	\return ModTrue:  succeed in getting text
					ModFalse:  no more text
*/
		ModBoolean getWholeText(ModUnicodeString& wholeText);

//
// FUNCTION public
//	ModNlpAnalyzer::getNormalizeBuf -- get normalized text
//
// NOTES
//	get normalized text of ModNormalizer::normalizeBuf.
//
// ARGUMENTS
//  ModUnicodeString& normalized
//		O: normalized text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get normalized text.
	Notes: get normalized text of ModNormalizer::normalizeBuf.
	@param[out] normalized  normalized text
	\return ModTrue:  succeed in getting text
					ModFalse:  no more text
*/
		ModBoolean getNormalizeBuf(ModUnicodeString& normalized);

//
// FUNCTION public
//	ModNlpAnalyzer::getExpandBuf -- get expanded text
//
// NOTES
//	get expanded text of ModNormalizer::expandBuf.
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& expanded
//		O: expanded text
//
// RETURN
//	ModBoolean
//		ModTrue:  succeed in getting text
//		ModFalse: no more text
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	get expanded text.
	Notes: get expanded text of ModNormalizer::expandBuf.
	@param[out] expanded  expanded text
	\return ModTrue:  succeed in getting text
					ModFalse:  no more text
*/
		ModBoolean getExpandBuf(ModVector<ModUnicodeString>& expanded);

//
// FUNCTION public
//	ModNlpAnalyzer::getExpandStrings -- get expanded strings patterns
//
// NOTES
//  This function is used to get expanded strings patterns of ModNlpExpStr::expandStrings
//  only in the ModNlpUnaJp module.
//
// ARGUMENTS
//	ExpandResult& expandResult
//		O:	status of acquisition result of expanded string patterns
//			 0:	no expanded strings patterns
//			 1:	succeed in getting expanded strings patterns
//			 2:	failure of getting expanded strings patterns 
//				by upper bound of length of character string for expanding
//			 3: failure of getting expanded strings patterns 
//				by upper bounds of number of expanded strings patterns
//	ModVector<ModUnicodeString>& expanded
//		O: expanded strings patterns
//
// RETURN
//	None
//
// EXCEPTIONS
//	exception of lower modules
//
		void getExpandStrings(ExpandResult& expandResult,
								ModVector<ModUnicodeString>& expanded);

//
// FUNCTION public
//	ModNlpAnalyzer::identify -- identify in which language the document is written
//
// NOTES
//	****************** Don't use this method cullentry **********************
//
// ARGUMENTS
//	ModSize endPos
//		I: end position of target document to be used to identify
//
// RETURN
//	ModLanguageCode
//		ModLanguageCode::undefined : unable to identify
//
// EXCEPTIONS
//	exception of lower modules
//
/*!
	identify in which language the document is written.
	**** Don't use this method cullentry ****
	@param[in] endPos  end position of target document to be used to identify
	\return ModLanguageCode
					ModLanguageCode::undefined : unable to identify
*/
		ModLanguageCode identify(ModSize endPos);

//
// FUNCTION private
// ModNlpAnalyzer::getVersion
//
// NOTES
//	get version string of UNA
//
// ARGUMENTS
//	none
//
// RETURN
//  version string of UNA
//
// EXCEPTIONS
//	none
//
		static ModUnicodeString getVersion();

	private:
//
// FUNCTION private
// ModNlpAnalyzer::extractNP
//
// NOTES
//	extract noun phrase from the target document
//	make keyword candidates with ExRule from concepts
//
// ARGUMENTS
//	none
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
		void extractNP();

//
// FUNCTION private
// ModNlpAnalyzer::makeExRule
//
// NOTES
//	load and make the ExtractRule for specified language
//
// ARGUMENTS
//	I:UNA::DicSet *dicSet_
//		ModLanguageCode language
//
// RETURN
//	none
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		failure of loading ExtractRule.
//
		void makeExRule(UNA::DicSet *dictSet_);

	private:
		//buffer pool for orig string
		ModUnicodeString m_OrigBuffer;

		//buffer pool for norm string
		ModUnicodeString m_NormBuffer;

		// index of the current NP in keyword candidate vector
		ModVector<Keyword>::ConstIterator 	keywordIndex;
		// morph vector
		ModVector<Morph>	morphVector;
		ModVector<Keyword>	keywordVector;


		ModVector<ModNlpLocalAnalyzer*>m_vecAnalyzers;
		ModVector<ModNlpLocalAnalyzer*>::Iterator m_pAnalyzer;
		ModLanguageSet m_cLang;
		ModNlpResource* m_pResource;  
	};
} // namespace UNA

#endif // __UNA_MODNLP_H

//
// Copyright (c) 2001, 2005-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
