// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// Tokenizer.h -- 
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_UTILITY_TOKENIZER_H__
#define	__SYDNEY_UTILITY_TOKENIZER_H__

#include "Utility/Module.h"

#include "ModLanguageSet.h"
#include "ModPair.h"
#include "ModTypes.h"
#include "ModVector.h"

class ModUnicodeString;

namespace UNA
{
	class ModNlpAnalyzer;
}

_TRMEISTER_BEGIN

namespace Common
{
	class DataArrayData;
}

_TRMEISTER_UTILITY_BEGIN

class SearchTermData;

//
// CLASS
// Tokenizer -- 
//
// NOTES
//
// HOW TO USE
// Tokenizer t;
// t.set(...);
// t.getWordInfo(cstrSrc0_, ... cstrDst0_, vecWordInfo0_);
// t.getWordInfo(cstrSrc1_, ... cstrDst1_, vecWordInfo1_);
// ...
// t.clear();
// t.set(...);
// t.getWordInfo(...);
// ...
//
class Tokenizer
{
public:
	//
	//	TYPEDEF
	//	Utility::Tokenizer::WordInfoLocation
	//
	//	NOTES
	//	The locations are the head and tail of the word,
	//	and these are represented by 0-base.
	//
	typedef ModPair<ModSize, ModSize> WordInfoLocation;
	
	//
	//	TYPEDEF
	//	Utility::Tokenizer::WordInfoSubElement
	//
	//	NOTES
	//	location before normalizing and POS
	//
	typedef ModPair<WordInfoLocation, int> WordInfoSubElement;
	
	//
	//	TYPEDEF
	//	Utility::Tokenizer::WordInfoElement
	//
	//	NOTES
	//	location after normalizing and (location before normalizing and POS)
	//
	typedef ModPair<WordInfoLocation, WordInfoSubElement> WordInfoElement;
	
	//
	//	TYPEDEF
	//	Utility::Tokenizer::WordInfoVec
	//
	typedef ModVector<WordInfoElement> WordInfoVec;

	Tokenizer();
	virtual ~Tokenizer();

	void set(const Common::DataArrayData* pPropertyKey_,
			 const Common::DataArrayData* pPropertyValue_);
	
	void clear();

	void getWordInfo(const ModUnicodeString& cstrSrc_,
					 const ModLanguageSet& cLanguageSet_,
					 ModUnicodeChar usWordSeparator_,
					 ModUnicodeString& cstrDst_,
					 WordInfoVec& vecWordInfo_) const;

	void setSearchTerm(const SearchTermData* pSearchTerm_);
	bool getExpandWords(ModVector<ModUnicodeString>& vecMorpheme_);

protected:
private:
	
	int getIgnoredCharacterCount(const ModUnicodeChar*& p_,
								 const ModUnicodeChar* e,
								 const ModUnicodeString& cstrWord_) const;

	UNA::ModNlpAnalyzer* m_pAnalyzer;
	ModLanguageSet m_cDefaultLanguageSet;
	bool m_bNormalize;
};

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif	// __SYDNEY_UTILITY_TOKENIZER_H__

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
