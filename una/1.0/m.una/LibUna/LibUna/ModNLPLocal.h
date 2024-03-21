// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLPLocal.h -- Hedaer file of the base class of local analyzers and resources
// 
// Copyright (c) 2005, 2008-2010, 2023, 2024 Ricoh Company, Ltd.
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
#ifndef __UNA_MODNLPLOCAL_H
#define __UNA_MODNLPLOCAL_H

#include "UnaDLL.h"
#include "UNA_UNIFY_TAG.h"
#include "ModVector.h"
#include "ModMap.h"
#include "ModLanguageSet.h"
#include "Morph.h"
#include "ModNLPText.h"

namespace UNA {


class Morph;
class DicSet;
//
// CLASS
//	ModNlpLocalResource -- a base calss of local resources
//
// NOTES
//	This resource class used by ModNlpLocalAnalyzer
//
class UNA_LOCAL_FUNCTION ModNlpLocalResource : public ModDefaultObject
{
public:
	enum STEMMERS
	{
		defaultStemmer = 0
	};
private:
	STEMMERS m_stemmer;
protected:
	ModUnicodeString	m_resourcePath;
	ModSize			m_ulResType;
public:

	ModNlpLocalResource():m_ulResType(0){m_stemmer = defaultStemmer;}
#ifdef STD_CPP11
	virtual ~ModNlpLocalResource() noexcept(false){}
#else
	virtual ~ModNlpLocalResource(){}
#endif
	virtual ModUnicodeString getResourcePath() {  return m_resourcePath;}

	virtual ModSize getResourceType() { return m_ulResType;}
	
//
// FUNCTION public
//	ModNlpLocalResource::load -- resource file is checked
//
// NOTES
//	set resource path and memory save switch and resource file is checked.
//	Loading resource data is done in activate() methode.
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
  virtual ModBoolean load(const ModUnicodeString& resourcePath, 
						  const ModBoolean memorySave)=0;
//
// FUNCTION public
//	ModNlpLocalResource::unload -- resource file is freed
//
// NOTES
//	resource file is freed
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
  virtual ModBoolean unload()=0;
//
	virtual void setStemmerID(STEMMERS id)
	{
		m_stemmer = id;
	}
	virtual STEMMERS getStemmerID()
	{
		return m_stemmer;
	}
};


class ModNlpResource;
class ModNlpAnalyzer;
//
// CLASS
//	ModNlpLocalAnalyzer -- a base class of local analyzers
//
// NOTES
//	This class is used for each analyzing of documents.
//
class UNA_LOCAL_FUNCTION ModNlpLocalAnalyzer: public ModDefaultObject
{
	ModNlpAnalyzer *m_pAnalyzer;
protected:
	// resources
	ModNlpLocalResource*   m_resource;
	//
	ModUnicodeString	_cstrIndicatedAnalyzer;
 	ModUnicodeString	_cstrDisableAnalyzer;
	bool m_doDiv;
	bool m_doStem;
	bool m_doCompoundDiv;
	bool m_doCarriageRet;

	ModVector<ModUnicodeString> 	m_normVector;
	ModVector<ModUnicodeString> 	m_orgVector;
	ModVector<int>					m_posVector;
	ModVector<ModUnicodeString>		m_devidedWord;
	ModVector<TagCode>				m_devidedPos;
	
	DicSet*			m_dicSet;
public:
	ModNlpLocalAnalyzer():m_pAnalyzer(0),_cstrDisableAnalyzer(""),m_resource(0),m_dicSet(0)
	{
		// init members
		m_devidedWord.clear();
		m_devidedPos.clear();
		m_normVector.clear();
		m_orgVector.clear();
		m_posVector.clear();
	}
	virtual ~ModNlpLocalAnalyzer(){}
	virtual void reset()
	{
		// init members
		m_devidedWord.clear();
		m_devidedPos.clear();
		m_normVector.clear();
		m_orgVector.clear();
		m_posVector.clear();
	}
	DicSet * getDicSet(){return m_dicSet;}
	ModNlpLocalResource* getResource(){ return m_resource;}
//
// FUNCTION public
//	ModNlpLocalAnalyzer::setResource -- set resource
//
// NOTES
//	set resource
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalResource*
//		the address of ModNlpLocalResource
//
// EXCEPTIONS
//	exception of lower modules
//
	virtual ModBoolean setResource(ModNlpLocalResource*)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::releaseResource -- decrease reference counter if it is needed
//
// NOTES
//	decrease reference counter if it is needed
//
// ARGUMENTS
//	none
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
	virtual ModBoolean releaseResource()=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::getAbility -- return ability
//
// NOTES
//	return ability
//
// ARGUMENTS
//	funcName
//		I: Name of function
//
// RETURN
//	ModSize
//		4: prefered ( will modify all return abilities to 4)
//		3: perfect ability
//		2: percial ability
//		1: no ability and default processing
//		0: no ability and no processing
//
// EXCEPTIONS
//	exception of lower modules
//
	virtual ModSize getAbility(const ModLanguageSet& lang)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::prepare -- set language and parameters
//
// NOTES
//	set language and parameters
//
// ARGUMENTS
//	const ModLanguageSet&
//		I: language
//	const ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >&
//		I: parameters
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
	typedef ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > Parameters;
	virtual ModBoolean prepare( const Parameters& parametersToModifyAnalyze)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::set -- set a text to be analyzed
//
// NOTES
//	set a whole text to be analyzed
//
// ARGUMENTS
//	const ModUnicodeString&
//		I: a text to be analyzed
//
// RETURN
//	ModBoolean
//		ModTrue
//
// EXCEPTIONS
//	exception of lower modules
//
	virtual ModBoolean set(const ModUnicodeString& target, const ModLanguageSet& lang)=0;

//
// FUNCTION public
//	ModNlpLocalAnalyzer::isExpStrDataLoad -- Whether the data for expanding strings is loaded is confirmed.
//
// NOTES
//	As for this function, when only the ModNlpUnaJp module is used,
//	and executed, the function of the inherited class is called.
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
	virtual ModBoolean isExpStrDataLoad();

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getWord -- get a word from document in turn
//
// NOTES
//	get a word from document in turn
//
// ARGUMENTS
//	ModUnicodeString& normalized
//		O: normalized word
//	ModUnicodeString* original
//		O: original word
//	int* pos
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
	virtual ModBoolean getWord(ModUnicodeString& normalized, ModUnicodeString* original, int* pos)=0;

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getConcept -- get one NounPhrase from the original document
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
	virtual ModBoolean getConcept(
			ModUnicodeString& extractedConcept_,
			ModVector<ModUnicodeString>& normVector_,
			ModVector<ModUnicodeString>& origVector_,
			ModVector<int>& posVector_);

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getConcept -- get one NounPhrase from the original document
//
// NOTES
//	get one NounPhrase from the original document
//
// ARGUMENTS
//	ModUnicodeString& normalized_
//		O: the normalized NounPhrase
//	ModUnicodeString& original_
//		O: the original NounPhrase
//	double& npCost_
//		O: cost of NounPhrase
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
	virtual ModBoolean getConcept(
			ModUnicodeString& normalized_,
			ModUnicodeString& original_,
			double& npCost_,
			ModVector<ModUnicodeString>* normVector_ = 0,
			ModVector<ModUnicodeString>* origVector_ = 0,
			ModVector<int>* posVector_ = 0)=0;

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getExpandWords -- get expanded words from document in turn
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
	virtual ModBoolean getExpandWords(ModVector<ModUnicodeString>& expanded, ModUnicodeString& original, int& pos)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::getBlock -- get words from document together
//
// NOTES
//	get words from document together
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& normVector
//		O: vector including normalized words
//	ModVector<int>& posvector
//		O: vector including part of speeches
//	ModVector<ModUnicodeString>* origVector
//		O: vector including original words
//	ModVector<ModUnicodeString>* costVector
//		O: vector including words cost
//	ModVector<int>* uposVector
//		O: vector including unified part of speeches
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
	virtual ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
  							ModVector<int>& posvector,
  							ModVector<ModUnicodeString>* origVector,
  							ModVector<int>* costVector = 0,
							ModVector<int>* uposVector = 0,
							ModBoolean ignore = ModTrue) = 0;

	virtual ModBoolean getBlock(ModUnicodeString& result, ModUnicodeChar sep1, ModUnicodeChar sep2)=0;

	virtual ModBoolean getBlock(ModVector<ModUnicodeString>& normVector,
							ModVector<ModUnicodeString>& hinVector);
//
// FUNCTION public
//	ModNlpLocalAnalyzer::getDicName -- get dictionary base name for each morpheme
//
// NOTES
//	get dictionary base name for each morpheme which getBlock() returned
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
	virtual ModBoolean getDicName(ModVector<ModUnicodeString>& dicNameVector_);

//
// FUNCTION public
//	ModNlpLocalAnalyzer::identify -- identify in which language the document is written
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
	virtual ModLanguageCode identify(ModSize endPos)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::getWholeText -- get whole text
//
// NOTES
//	If NormalizeOnly parameter is specified inparameter,
//	this method returns normalized text of ModNormalizer::normalizeBuf.
//	else if GetOrg parameter is specified in parameter,
//	this method returns original text.
//	else
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
	virtual ModBoolean getWholeText(ModUnicodeString& wholeText)=0;
//
// FUNCTION public
//	ModNlpLocalAnalyzer::getNormalizeBuf -- get normalized text
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
	virtual ModBoolean getNormalizeBuf(ModUnicodeString& normalized)=0;

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getExpandBuf -- get expanded text
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
	virtual ModBoolean getExpandBuf(ModVector<ModUnicodeString>& expanded)=0;

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getExpandStrings -- get expanded strings
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
//	ModBoolean
//		ModTrue:	normal termination
//		ModFalse:	something error
//
// EXCEPTIONS
//	exception of lower modules
//
	virtual ModBoolean getExpandStrings(ModSize& expandResult, ModVector<ModUnicodeString>& expanded);

//
// FUNCTION public
//	ModNlpLocalAnalyzer::poolTerm -- Extraction of noun phrase
//
// NOTES
//	The noun phrase is extracted from the text 
//	and it registers in the pool.
//
// ARGUMENTS
//	 const ModBoolean
//		O: useStopDict		whether to use the stop dictionary
//	 const ModSize
//		O: maxText			The text maximum length
//							(The number of morphological analysises.
//							 If it is 0, it is unrestricted). 
//	 const ModBoolean
//		O: useStopTypeDict	whether to use the stop type dictionary
//	 ModTermPool&
//		O: pool				Noun phrase pool
//
// RETURN
//	 none
//
// EXCEPTIONS
//
	virtual void poolTerm(const ModBoolean useStopDict,
						const ModSize maxText,
						const ModBoolean useStopTypeDict){}

	virtual void makeMorphArray(ModVector<Morph>& result_,
						ModUnicodeString &cOrigBuffer_,
						ModUnicodeString &cNormBuffer_){}

	virtual ModNlpAnalyzer * getParent()
	{
		return m_pAnalyzer;
	}

	virtual void setParent(	 ModNlpAnalyzer *pAnalyzer_)
	{
		m_pAnalyzer = pAnalyzer_;
	}

//
// FUNCTION public
//	ModNlpLocalAnalyzer::getNpCost -- get cost of noun phrase
//
// NOTES
//	The noun phrase is extracted from the text 
//	and it registers in the pool.
//
// ARGUMENTS
//	 ModVector<ModUnicodeString>
//		I: normVector_		vector including normalized morpheme
//							that composes noun phrase
//	 const ModVector<int>
//		I: posVector_		vector including part of speech of morpheme
//							that composes noun phrase
//
// RETURN
//	 double
//		cost of noun phrase
//
// EXCEPTIONS
//
	virtual double getNpCost(ModVector<ModUnicodeString> normVector_, ModVector<int> posVector_)
	{
		return 0;
	}
};


//
// CLASS
//	ModNlpLocal -- Abstract Factory Class
//
// NOTES
//	This class is used to generate concrete factory class.
//
class UNA_LOCAL_FUNCTION ModNlpLocal: public ModDefaultObject
{

public:
	ModNlpLocal(){}
	virtual ~ModNlpLocal(){}
	
//
// FUNCTION public
//	ModNlpLocal::createAnalyzerObject -- construct ModNlpLocalAnalyzer
//
// NOTES
//	construct ModNlpLocalAnalyzer
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalAnalyzer*
//		the address of ModNlpLocalAnalyzer
//
// EXCEPTIONS
//	exception of lower modules
//
  virtual ModNlpLocalAnalyzer* createAnalyzerObject(ModNlpAnalyzer *pAnalyzer_) = 0;
//
// FUNCTION public
//	ModNlpLocal::destroyAnalyzerObject -- destruct ModNlpLocalAnalyzer
//
// NOTES
//	destruct ModNlpLocalAnalyzer
//
// ARGUMENTS
//	ModNlpLocalAnalyzer*
//		I: the address of ModNlpLocalAnalyzer
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
  virtual void destroyAnalyzerObject(ModNlpLocalAnalyzer *analyzer_){delete analyzer_;}
//
// FUNCTION public
//	ModNlpLocal::createResourceObject -- construct ModNlpLocalResource
//
// NOTES
//	construct ModNlpLocalResource
//
// ARGUMENTS
//	none
//
// RETURN
//	ModNlpLocalResource*
//		the address of ModNlpLocalResource
//
// EXCEPTIONS
//	exception of lower modules
//
  virtual ModNlpLocalResource* createResourceObject() = 0;
//
// FUNCTION public
//	ModNlpLocal::destroyResourceObject -- destruct ModNlpLocalResource
//
// NOTES
//	destruct ModNlpLocalResource
//
// ARGUMENTS
//	ModNlpLocalResource*
//		I: the address of ModNlpLocalResource
//
// RETURN
//	none
//
// EXCEPTIONS
//	exception of lower modules
//
  virtual void destroyResourceObject(ModNlpLocalResource *resource_){delete resource_;}
};

//
//	CLASS
//	
//
class UNA_LOCAL_FUNCTION ParameterWrapper
{
	typedef ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> > ParamMap;
	
public:
	ParameterWrapper(const ParamMap& cParam_);
	virtual ~ParameterWrapper();

	// get string
	const ModUnicodeString& getString(const ModUnicodeString& cKey_) const;
	// get boolean
	bool getBoolean(const ModUnicodeString& cKey_, bool default_) const;
	// get ModSize
	ModSize getModSize(const ModUnicodeString& cKey_, ModSize default_) const;

private:
	const ParamMap& m_cParam;
};

}

#endif//__UNA_MODNLPLOCAL_H

//
// Copyright (c) 2005, 2008-2010, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
