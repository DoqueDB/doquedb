// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNLP.cpp
// 
// Copyright (c) 2005-2010, 2023 Ricoh Company, Ltd.
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

extern "C"
{
#include <dlfcn.h>
}
#include "LibUna/UnaNameSpace.h"
#include "LibUna/ModNLPLocal.h"
#include "LibUna/ModNLP.h"
#include "LibUna/DicSet.h"
#include "LibUna/RuleHolder.h"
#include "LibUna/RuleApplier.h"
#include "LibUna/RuleScanner.h"
#include "LibUna/RuleMaker.h"

#include <stdio.h>

_UNA_USING

namespace RuleMakerLock {

// #T5#
	// getLanguage(), ModuleID::Extract
	//		TYPEDEF
	//		RuleMakerLockManager -- lock management map at each rule file
	//
	// MEMO
	typedef AutoMapSecondPointer < RuleHolder::RuleID,
					ModCriticalSection*,
					ModLess<RuleHolder::RuleID> >
					RuleMakerLockManager;

// #T6#
	// lock management map
	RuleMakerLockManager _manager;
// #T7#
	// critical section for lock management map
	ModCriticalSection	_manager_cs;
// #T8#
	// acquire a critical section
	ModCriticalSection*
	getMakingRight( UNA::DicSet *dicSet_ )
	{
		ModAutoMutex<ModCriticalSection> mutex(&_manager_cs);
		mutex.lock();
		RuleHolder::RuleID ruleid = dicSet_->getRuleID();
		RuleMakerLockManager::Iterator it = _manager.find(ruleid);
		if ( it == _manager.end() )
		{
// #T9#
			// make it newly
			_manager.insert(ruleid, new ModCriticalSection);
			it = _manager.find(ruleid);

			return (*it).second;
		}
		else
		{
			return (*it).second;
		}
	}
}
namespace
{
	ModCriticalSection	_lock;
}

namespace
{
	namespace _Literal
	{
		const ModUnicodeString	_Prefix("");
		const ModUnicodeString _Suffix(".so");
	}

	namespace _Library
	{
		typedef ModMap<ModUnicodeString, void*, ModLess<ModUnicodeString> >	_Map;

		ModCriticalSection	l_cLockLibrary;
		// Map to manage the library still be loaded
		_Map				l_cLibraryLoadMap;
		// Initialized count
		int					l_iInitialized = 0;
	}
}

namespace _FactoryType
{
	enum FactoryType
	{
		UnaJp = 0,
		last
	};
}

ModNlpLocal*
ModNlp::create(int i_)
{
	DEBUGPRINT("ModNlp::create\n",0);
	//get library name of abstruct factory
	ModUnicodeString cstrLibName;
	switch(i_)
	{
	case _FactoryType::UnaJp:
		cstrLibName = DLLNAME("libunajp");
		break;
	default:
		return 0;
	}
	//load library including abstruct factory if it is not loaded yet
	ModNlp::loadLibrary(cstrLibName);
	//get function address that returns address of abstruct factory
	ModNlpLocal* (*f)(void);
	f = (ModNlpLocal*(*)(void))ModNlp::getProcAddress(cstrLibName,"getFactoryInstance");
	return (*f)();
}

void
ModNlp::initialize()
{
	DEBUGPRINT("ModNlp::initialize\n",0);

	ModAutoMutex<ModCriticalSection> mutex(&_Library::l_cLockLibrary);
	mutex.lock();
	++_Library::l_iInitialized;
}

//static
void
ModNlp::terminate()
{
	DEBUGPRINT("ModNlp::terminate\n",0);

	ModAutoMutex<ModCriticalSection> mutex(&_Library::l_cLockLibrary);
	mutex.lock();
	if (--_Library::l_iInitialized == 0) {
		freeLibrary();
	}
}


void
ModNlp::loadLibrary(const ModUnicodeString& cstrBaseName_)
{
	DEBUGPRINT("ModNlp::loadLibrary\n",0);

	ModAutoMutex<ModCriticalSection> mutex(&_Library::l_cLockLibrary);
	mutex.lock();

	_Library::_Map::Iterator iterator = _Library::l_cLibraryLoadMap.find(cstrBaseName_);
	if (iterator == _Library::l_cLibraryLoadMap.end())
	//Library has not been loaded yet.
	{
		ModUnicodeString cstrLibName;
		cstrLibName =  _Literal::_Prefix;
		cstrLibName += cstrBaseName_;
		cstrLibName += _Literal::_Suffix;

		void* p = 0;

		p =	::dlopen(cstrLibName.getString(
				ModOs::Process::getEncodingType()), RTLD_LAZY );
		if (p==0)
		//error
		{
			ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
		}

		_Library::l_cLibraryLoadMap.insert(cstrBaseName_,p);
	}
}

void
ModNlp::freeLibrary()
{
	DEBUGPRINT("ModNlp::freeLibrary\n",0);

	ModAutoMutex<ModCriticalSection> mutex(&_Library::l_cLockLibrary);
	mutex.lock();

	const _Library::_Map::Iterator& begin = _Library::l_cLibraryLoadMap.begin();
	_Library::_Map::Iterator ite(begin);
	const _Library::_Map::Iterator& end = _Library::l_cLibraryLoadMap.end();

	for (; ite != end; ++ite)
	{
		//Caution: error is ignored

		(void) ::dlclose((*ite).second);
	}

	_Library::l_cLibraryLoadMap.erase(begin, end);

}

void*
ModNlp::getProcAddress(const ModUnicodeString& cstrBaseName_,const char* pszFuncName_)
{
	DEBUGPRINT("ModNlp::getProcAddress\n",0);

	ModAutoMutex<ModCriticalSection> mutex(&_Library::l_cLockLibrary);
	mutex.lock();

	const _Library::_Map::Iterator& ite = _Library::l_cLibraryLoadMap.find(cstrBaseName_);
	if (ite == _Library::l_cLibraryLoadMap.end())
	{
		// Library has not been loaded.
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
	}

	//caution: function name must be ascii

	void* p = 0;
	p =	::dlsym((*ite).second, pszFuncName_);

	if (p == 0)
	//error
	{
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
	}

	return p;
}

ModNlpAnalyzer::ModNlpAnalyzer()
{
	DEBUGPRINT("ModNlpAnalyzer::ModNlpAnalyzer\n",0);
}
    
ModNlpAnalyzer::~ModNlpAnalyzer()
{ 
  DEBUGPRINT("ModNlpAnalyzer::~ModNlpAnalyzer\n",0);
  releaseResource();
}
    
ModBoolean ModNlpAnalyzer::setResource(ModNlpResource* resource_)
{ 
  DEBUGPRINT("ModNlpAnalyzer::setResource\n",0);
   for( ModVector<int>::Iterator iter = resource_->m_vecFactoryNo.begin(); 
		iter != resource_->m_vecFactoryNo.end();iter++){
		ModAutoPointer<ModNlpLocal> p = ModNlp::create(*iter);
		int i = iter - resource_->m_vecFactoryNo.begin();
        m_vecAnalyzers.pushBack(p->createAnalyzerObject(this));
		(m_vecAnalyzers[i]->setResource)(((resource_->getResource)(i)));
  }
  m_pResource = resource_;
  return ModTrue;
}
        
ModBoolean ModNlpAnalyzer::releaseResource()
{
  DEBUGPRINT("ModNlpAnalyzer::releaseResource\n",0);
  for (ModSize i=0; i < m_vecAnalyzers.getSize(); ++i){
	if (m_vecAnalyzers[i]) {
		m_vecAnalyzers[i]->releaseResource();
		ModAutoPointer<ModNlpLocal> p
			= ModNlp::create(m_pResource->m_vecFactoryNo[i]);
		p->destroyAnalyzerObject(m_vecAnalyzers[i]);
		m_vecAnalyzers[i] = 0;
	}
  }
  return ModTrue;
}

ModBoolean ModNlpAnalyzer::prepare(
  const Parameters& parametersToModifyAnalyze)
{ 
	// 言語指定
	DEBUGPRINT("ModNlpAnalyzer::prepare\n",0);
	for (ModVector<ModNlpLocalAnalyzer*>::Iterator iter = m_vecAnalyzers.begin();
		iter != m_vecAnalyzers.end(); ++iter)
	{
		(*iter)->prepare(parametersToModifyAnalyze);
	}
	return ModTrue;
}

ModBoolean ModNlpAnalyzer::set(
  const ModUnicodeString& target,
  const ModLanguageSet& lang)
{
	// 解析対象テキストの設定と言語指定
	if (lang.getSize() == 0)
	{
		// defaultはresourceの先頭に記述された言語
		m_pAnalyzer = m_vecAnalyzers.begin();
	}
	else if (m_cLang != lang)
	{
	// 従来の言語指定の異なる場合は、新たに適切な言語処理系を探す
		int maxAbility = 0,n;
		{
			// 最適な言語解析系を設定する
			for (ModVector<ModNlpLocalAnalyzer*>::Iterator iter = m_vecAnalyzers.begin();
				iter != m_vecAnalyzers.end(); ++iter)
			{
				if ((n = (*iter)->getAbility(lang)) > maxAbility)
				{
					maxAbility = n;
					m_pAnalyzer = iter;
				}
			}
		}
		m_cLang = lang;
	}
	(*m_pAnalyzer)->set(target,lang);

	// clear the keyword vector index
	keywordIndex = 0;
	morphVector.clear();
	keywordVector.clear();

	return ModTrue;
}

// It is confirmed whether the data of the expanding function set by the parameter is loaded.
ModBoolean ModNlpAnalyzer::isExpStrDataLoad()
{
	return (*m_pAnalyzer)->isExpStrDataLoad();
}

ModBoolean ModNlpAnalyzer::getWord(
	ModUnicodeString& normalized_)
{
	DEBUGPRINT("ModNlpAnalyzer::getWord %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getWord(normalized_,0,0);
}
    
ModBoolean ModNlpAnalyzer::getWord(
	ModUnicodeString& normalized_, 
	ModUnicodeString& original_)
{ 
	DEBUGPRINT("ModNlpAnalyzer::getWord %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getWord(normalized_,&original_,0);
}
    
ModBoolean ModNlpAnalyzer::getWord(
	ModUnicodeString& normalized_, 
	ModUnicodeString& original_, 
	int& pos_)
{ 
	DEBUGPRINT("ModNlpAnalyzer::getWord %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getWord(normalized_,&original_,&pos_);
}

//
// Note: このAPIの実装は、他のAPIの実装とは異なるので注意を要する
//
//
ModBoolean ModNlpAnalyzer::getConcept(
	ModUnicodeString& extractedConcept_,
	ModVector<ModUnicodeString>& normVector_,
	ModVector<ModUnicodeString>& origVector_,
	ModVector<int>& posVector_)
{ 
	DEBUGPRINT("ModNlpAnalyzer::getConcept %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	// UnaJPはLibUna中のgetConceptImp()を呼ぶ
	return (*m_pAnalyzer)->getConcept(extractedConcept_, normVector_, origVector_, posVector_);
}

ModBoolean ModNlpAnalyzer::getConcept(
	ModUnicodeString& normalized_,
	ModUnicodeString& original_,
	double& npCost_,
	ModVector<ModUnicodeString>& normVector_,
	ModVector<ModUnicodeString>& origVector_,
	ModVector<int>& posVector_)
{ 
	DEBUGPRINT("ModNlpAnalyzer::getConcept %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getConcept(normalized_, original_, npCost_, &normVector_, &origVector_, &posVector_);
}

ModBoolean ModNlpAnalyzer::getConcept(
	ModUnicodeString& normalized_,
	ModUnicodeString& original_,
	double& npCost_)
{ 
	DEBUGPRINT("ModNlpAnalyzer::getConcept %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getConcept(normalized_, original_, npCost_);
}

ModBoolean
ModNlpAnalyzer::getExpandWords(
    ModVector<ModUnicodeString>& expanded,
    ModUnicodeString& original,
    int& pos)
{
	DEBUGPRINT("ModNlpAnalyzer::getExpandWords %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getExpandWords(expanded,original,pos);
}

ModBoolean 
ModNlpAnalyzer::getBlock(
    ModVector<ModUnicodeString>& normVector,
    ModVector<int>& posvector)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(normVector,posvector,0);
}

ModBoolean 
ModNlpAnalyzer::getBlock(
    ModVector<ModUnicodeString>& normVector,
    ModVector<ModUnicodeString>& origVector,
    ModVector<int>& posvector)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(normVector,posvector,&origVector);
}

ModBoolean 
ModNlpAnalyzer::getBlock(
    ModUnicodeString& result,
	ModUnicodeChar sep1, ModUnicodeChar sep2)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(result,sep1,sep2);
}

ModBoolean
ModNlpAnalyzer::getBlock(
	ModVector<ModUnicodeString>& normVector,
	ModVector<ModUnicodeString>& hinVector)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(normVector,hinVector);
}

ModBoolean 
ModNlpAnalyzer::getBlock(
	ModVector<ModUnicodeString>& normVector,
	ModVector<ModUnicodeString>& origVector,
	ModVector<int>& posvector,
	ModVector<int>& costVector,
	ModVector<int>& uposVector)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(normVector,posvector,&origVector,&costVector,&uposVector);
}

ModBoolean 
ModNlpAnalyzer::getBlock(
	ModVector<ModUnicodeString>& normVector,
	ModVector<ModUnicodeString>& origVector,
	ModVector<int>& posvector,
	ModVector<int>& costVector,
	ModVector<int>& uposVector,
	ModBoolean ignore)
{
	DEBUGPRINT("ModNlpAnalyzer::getBlock %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getBlock(normVector,posvector,&origVector,&costVector,&uposVector,ignore);
}

ModBoolean 
ModNlpAnalyzer::getDicName(
    ModVector<ModUnicodeString>& dicNameVector_)
{
	DEBUGPRINT("ModNlpAnalyzer::getDicName %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getDicName(dicNameVector_);
}

ModLanguageCode
ModNlpAnalyzer::identify(
	ModSize endPos)
{
	DEBUGPRINT("ModNlpAnalyzer::identify %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->identify(endPos);
}

ModBoolean
ModNlpAnalyzer::getWholeText(
    ModUnicodeString& wholeText)
{
	DEBUGPRINT("ModNlpAnalyzer::getWholeText %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getWholeText(wholeText);
}
ModBoolean
ModNlpAnalyzer::getNormalizeBuf(
    ModUnicodeString& normalized)
{
	DEBUGPRINT("ModNlpAnalyzer::getNormalizeBuf %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getNormalizeBuf(normalized);
}

ModBoolean 
ModNlpAnalyzer::getExpandBuf(
    ModVector<ModUnicodeString>& expanded)
{
	DEBUGPRINT("ModNlpAnalyzer::getExpandBuf %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	return (*m_pAnalyzer)->getExpandBuf(expanded);
}

void
ModNlpAnalyzer::getExpandStrings(
	ExpandResult& expandResult,
	ModVector<ModUnicodeString>& expanded)
{
	DEBUGPRINT("ModNlpAnalyzer::getExpandStrings %d\n",m_pAnalyzer - m_vecAnalyzers.begin());
	if((*m_pAnalyzer)->getExpandStrings((ModSize&)expandResult, expanded) == ModFalse){
		ModErrorMessage << "ModNlpAnalyzer::getExpandStrings" << ModEndl;
		ModThrow( ModModuleStandard,
			  ModCommonErrorBadArgument,
			  ModErrorLevelError );
	}
}

ModUnicodeString
ModNlpAnalyzer::getVersion()
{
	return ModUnicodeString(UNA_VERSION);
}

//
// 以下getConcept()のサポート関数
// UnaJPのDLLから呼ばれるEXPORT関数
//  UnaJPとの互換性を考慮して実装すること

// FUNCTION
// ModNlpAnalyzer::getConceptImp
//
// NOTES
//	get one NounPhrase from the original document
//	tag the noun phrase, get each word in the NP
//	including the original words, normalized, original
//	and part of speech of each word, and also return
//	the word number of the NP
//
// ARGUMENTS
//	ModUnicodeString& keyword_
//		--- the original keyword
//	Modvector<ModUnicodeString>& normVector_
//		--- the normalized words vector in one NounPhrase
//	ModVector<ModUnicodeString>& orgVector_
//		--- the original words vector in one NounPhrase
//	ModVector<int>& posVector
//		--- the part of speech vector of each words
//		    in one NounPhrase
//	ModSize& len_
//		--- the word number of the NounPhrase
//
// RETURN
//	ModBoolean
//		ModTrue --- success to get one NounPhrase vector
//		ModFalse --- finish processing, no more nourphrases
//
// EXCEPTIONS
//	exception of lower modules
//
ModBoolean
ModNlpAnalyzer::getConceptImp(ModUnicodeString& keyword_,
							ModVector<ModUnicodeString>& normVector_,
							ModVector<ModUnicodeString>& orgVector_,
							ModVector<int>& posVector_,
							ModSize& len_)
{
	try
	{
		ModAutoMutex<ModCriticalSection> cAuto(&_lock);
		cAuto.lock();

		normVector_.clear();
		orgVector_.clear();
		posVector_.clear();

		// first time running getConcept() function
		if ( keywordIndex == 0 )
		{
			// extract nour phrase from the target document
			// reserve keyword candidate in the vector
			extractNP();

			// get the start position of the keyword candidate vector
			keywordIndex = keywordVector.begin();
		}

		// reach the end of the keyword vector, then exit
		if ( keywordIndex == keywordVector.end() )
			return ModFalse;

		// get the original string of the keyword
		// with the European style, have space between morphs
		keyword_ = keywordIndex->getOrg();

		// get keyword length
		len_ = keywordIndex->getSize();

		unsigned int posId;
		ModUnicodeString normWord;
		ModUnicodeString orgWord;

		for(ModSize i = 0; i < len_; i++)
		{
			// get part of speech for each language
			posId = (*keywordIndex)[i].getType();
			posVector_.pushBack( posId );

			// insert norm word of morphs in keyword
			normWord = (*keywordIndex)[i].getNorm();
			normVector_.pushBack( normWord );

			// insert org word of morphs in keyword
			orgWord = (*keywordIndex)[i].getOrg();
			orgVector_.pushBack( orgWord );
		}

		// turn to next keyword
		keywordIndex++;

		return ModTrue;
	}
	catch (ModException& e)
	{
		ModErrorMessage << "ModNlpAnalyzer::getConcept " << e << ModEndl;
		ModRethrow(e);
	}
	catch (...)
	{
		ModErrorMessage << "ModNlpAnalyzer::getConcept" << ModEndl;
		ModThrow( ModModuleStandard,
			  ModCommonErrorBadArgument,
			  ModErrorLevelError );
	}
}

// FUNCTION
// ModNlpAnalyzer::getConceptImp
//
// NOTES
//	get one NounPhrase from the original document
//	tag the noun phrase, get each word in the NP
//	including the original words, normalized, original
//	and part of speech of each word, and also return
//	the word number of the NP
//
// ARGUMENTS
//	ModUnicodeString& normalized_
//		--- the normalized NP
//	ModUnicodeString& original_
//		--- the original NP
//	double& npCost_
//		--- the cost of NP
//	ModSize& len_
//		--- the word number of the NounPhrase
//	Modvector<ModUnicodeString>& normVector_
//		--- the normalized words vector in one NounPhrase
//	ModVector<ModUnicodeString>& orgVector_
//		--- the original words vector in one NounPhrase
//	ModVector<int>& posVector
//		--- the part of speech vector of each words
//		    in one NounPhrase
//
// RETURN
//	ModBoolean
//		ModTrue --- success to get one NounPhrase vector
//		ModFalse --- finish processing, no more nourphrases
//
// EXCEPTIONS
//
ModBoolean
ModNlpAnalyzer::getConceptImp(ModUnicodeString& normalized_,
							ModUnicodeString& original_,
							double& npCost_,
							ModSize& len_,
							ModVector<ModUnicodeString>* normVector_,
							ModVector<ModUnicodeString>* orgVector_,
							ModVector<int>* posVector_)
{
	try
	{
		ModAutoMutex<ModCriticalSection> cAuto(&_lock);
		cAuto.lock();

		// first time running getConcept() function
		if ( keywordIndex == 0 )
		{
			// extract nour phrase from the target document
			// reserve keyword candidate in the vector
			extractNP();
			// get the start position of the keyword candidate vector
			keywordIndex = keywordVector.begin();
		}
		// reach the end of the keyword vector, then exit
		if ( keywordIndex == keywordVector.end() )
			return ModFalse;

		// get the original string of the keyword
		normalized_ = keywordIndex->getNorm();
		original_ = keywordIndex->getOrg();

		// get keyword length
		len_ = keywordIndex->getSize();

		if(normVector_)
		{
			normVector_->clear();
			orgVector_->clear();
			posVector_->clear();

			ModUnicodeString normWord;
			ModUnicodeString orgWord;
			unsigned int posId;

			for(ModSize i = 0; i < len_; i++)
			{
				// insert norm word of morphs in keyword
				normWord = (*keywordIndex)[i].getNorm();
				normVector_->pushBack( normWord );

				// insert org word of morphs in keyword
				orgWord = (*keywordIndex)[i].getOrg();
				orgVector_->pushBack( orgWord );

				// get part of speech for each language
				posId = (*keywordIndex)[i].getType();
				posVector_->pushBack( posId );
			}
			npCost_ = (*m_pAnalyzer)->getNpCost(*normVector_, *posVector_);
		}
		else
		{
			ModUnicodeString normWord;
			unsigned int posId;
			ModVector<ModUnicodeString> normVector;
			normVector.clear();
			ModVector<int> posVector;
			posVector.clear();

			for(ModSize i = 0; i < len_; i++)
			{
				// insert norm word of morphs in keyword
				normWord = (*keywordIndex)[i].getNorm();
				normVector.pushBack( normWord );

				// get part of speech for each language
				posId = (*keywordIndex)[i].getType();
				posVector.pushBack( posId );
			}
			npCost_ = (*m_pAnalyzer)->getNpCost(normVector, posVector);
		}

		// turn to next keyword
		keywordIndex++;

		return ModTrue;
	}
	catch (ModException& e)
	{
		ModErrorMessage << "ModNlpAnalyzer::getConcept " << e << ModEndl;
		ModRethrow(e);
	}
	catch (...)
	{
		ModErrorMessage << "ModNlpAnalyzer::getConcept" << ModEndl;
		ModThrow( ModModuleStandard,
			  ModCommonErrorBadArgument,
			  ModErrorLevelError );
	}
}

// FUNCTION
// ModNlpAnalyzer::extractNP
//
// NOTES
//	extract noun phrase from the target document
//	make keyword candidates with ExRule from Teragram concepts
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
void
ModNlpAnalyzer::extractNP()
{
	try
	{

		m_OrigBuffer.clear();
		m_NormBuffer.clear();

		(*m_pAnalyzer)->makeMorphArray( morphVector, m_OrigBuffer, m_NormBuffer );

		// make extract rule
		makeExRule( (*m_pAnalyzer)->getDicSet() );

		// Making of rule application machine
		RuleApplier applier( (*m_pAnalyzer)->getDicSet() );

		// Extract the key word candidate from morph array
		applier.applies( morphVector.begin(), morphVector.end(), keywordVector );
	}
	catch (ModException& e)
	{
		ModErrorMessage << "ModNlpAnalyzer::extractNp " << ModEndl;
		ModRethrow(e);
	}
	catch (...)
	{
		ModErrorMessage << "ModNlpAnalyzer::extractNp " << ModEndl;
		ModThrow( ModModuleStandard,
			ModCommonErrorBadArgument,
			ModErrorLevelError );
	}
}

// FUNCTION
// ModNlpAnalyzer::makeExRule
//
// NOTES
//	load and make the ExtractRule for specified language
//
// ARGUMENTS
//	I:UNA::DicSet *dicSet_
//		dictionary
//
// RETURN
//	none
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		failure of loading ExtractRule.
//
void
ModNlpAnalyzer::makeExRule(UNA::DicSet *dicSet_)
{
	// all rules are sure to be able to done in the session of this lock.
	ModCriticalSection* ex_cs
		= RuleMakerLock::getMakingRight( dicSet_ );
	ModAutoMutex<ModCriticalSection> mutex(ex_cs);
	mutex.lock();
	ModNlpLocalResource*  m = (*m_pAnalyzer)->getResource();
	ModUnicodeString rulefile = (*m_pAnalyzer)->getResource()->getResourcePath();

	rulefile += "rule/";

	rulefile += dicSet_->getRule();


	if ( !RuleHolder::isRule( dicSet_ ) )
	{
		if ( ModFile::doesExist(rulefile) )
		{
			RuleScanner scanner;
			RuleMaker exMaker(dicSet_, rulefile);
			RuleMaker::parse(&scanner, &exMaker);
		}
		else
		{
			ModErrorMessage << "Error in loading ExtractRule" << ModEndl;
			ModThrow( ModModuleStandard,
				ModCommonErrorBadArgument,
				ModErrorLevelError);
		}
	}
}

ModNlpResource::ModNlpResource()
	: m_vecResources()
{
	DEBUGPRINT("ModNlpResource::ModNlpResource\n",0);
	ModNlp::initialize();
}
    
ModNlpResource::~ModNlpResource()
{
	DEBUGPRINT("ModNlpResource::~ModNlpResource\n",0);
	unload();
	m_vecResources.clear();
	ModNlp::terminate();
}
    
ModNlpLocalResource* ModNlpResource::getResource(
	ModSize resourceNum_)
{ 
	DEBUGPRINT("ModNlpResource::getResource\n",0);
	return m_vecResources[resourceNum_];
}

ModBoolean ModNlpResource::load(
	const ModUnicodeString& resourcePath_, 
	const ModBoolean memorySave_)
{ 
	DEBUGPRINT("ModNlpResource::load\n",0);
	this->getAnalyzerList(resourcePath_);
	for(ModVector<int>::Iterator iter = m_vecFactoryNo.begin();
			iter != m_vecFactoryNo.end(); iter++)
	{
		ModAutoPointer<ModNlpLocal> p = ModNlp::create(*iter);
		m_vecResources.pushBack(p->createResourceObject());
		m_vecResources[iter - m_vecFactoryNo.begin()]->load(resourcePath_, memorySave_);
	}
	return ModTrue;
}

ModBoolean 
ModNlpResource::unload()
{ 
	DEBUGPRINT("ModNlpResource::unload\n",0);
	int i=0;
	for (i = 0; i < static_cast<int>(m_vecResources.getSize()); ++i){
		if (m_vecResources[i]) {
			m_vecResources[i]->unload();
			ModAutoPointer<ModNlpLocal> p = ModNlp::create(m_vecFactoryNo[i]);
			p->destroyResourceObject(m_vecResources[i]);
			m_vecResources[i] = 0;
		}
	}
	return ModTrue;
}

ModBoolean
ModNlpResource::getAnalyzerList(
	const ModUnicodeString& resourcePath_)
{
	DEBUGPRINT("ModNlpResource::getAnalyzerList\n",0);

	if (!ModFile::doesExist(resourcePath_))
	{
		ModMessage << "No resource data. Please check resource path." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
	}

	ModUnicodeString cstrUnaparam = resourcePath_;
	cstrUnaparam += "unaparam.dat";

	m_vecFactoryNo.clear();
	FILE* fp;
	char buf[256];
	char para[32];
	int i = 0;
	fp = fopen(cstrUnaparam.getString(),"r");
	if (fp){
		ModMap<char *,int,ModLess<char*> > chkdup;
		while(fgets(buf, sizeof(buf), fp))
		{
			// 行頭が#か、改行コードの時は読み捨てる
			if( strncmp(buf,"#",1) == 0 || strcmp(buf,"\n") == 0 ) continue;

			// para配列に格納できる長さかチェックする
			if(strlen(buf) < sizeof(para))
			{
				sscanf(buf,"%s",para);
			}
			else
			{
				// paraの配列より長い時は規定外とみなす
				ModMessage << ". Please check the description unaparam.dat." << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
			}

			// ２重登録を防止するために、mapで二重登録検査してから、FactoryNoにpushBack
			// 
			if(chkdup[para] == 1)
				continue;		// すでに登録済み
			chkdup.insert(ModPair<char*,int>(para,1));
			// resourceに記述された順に、factoryにpush backされる
			if (strcmp(para,"unajp")==0)
			{
				m_vecFactoryNo.pushBack(_FactoryType::UnaJp);
			}
			else
			{
				ModMessage << "Wrong unaparam.dat." << ModEndl;
				ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
			}

			// バッファをクリアする
			memset(buf,0x00,sizeof(buf));
			memset(para,0x00,sizeof(para));
		}
		fclose(fp);
	}
	else
	{
		ModMessage << "No unaparam.dat." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,ModErrorLevelError);
	}
	return ModTrue;
}

//
// Copyright (c) 2005-2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
