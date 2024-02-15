// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModWordStemmer.cpp -- Implementation of English word regular-ized machine
// 
// Copyright (c) 2002-2005, 2023 Ricoh Company, Ltd.
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

#include "ModParameter.h"
#include "LibUna/Module.h"
#include "LibUna/ModWordStemmer.h"


_UNA_USING

//
// -------------------------------------
// ModWordStemmer Function definition
// -------------------------------------

//
// FUNCTION public
//	ModWordStemmer::stem -- stemmer
//
// NOTES
//	String for stemmer
//
// ARGUMENTS
//	const ModUnicodeString& target
//		Target string for stemmer 
//	ModUnicodeString& result
//		String of stemmer result
//
// RETURN
//	ModBoolean
//		Return ModTrue if get result succussfully
//		Return ModFalse if failed  
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//
ModBoolean
ModWordStemmer::stem(const ModUnicodeString& target,
		ModUnicodeString& result)
{
    result = target;
    return ModFalse;
}


//
// FUNCTION public
//	ModWordStemmer::ModWordStemmer
//	-- Constructor 1
//
// NOTES
//	Constructor of ModWordStemmer
//
// ARGUMENTS
//	const ModLanguageSet& languageSet
//
// RETURN
//	Nothing
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//

ModWordStemmer::ModWordStemmer(const ModLanguageSet& languageSet_)
{
	langNum = 0;
	this->setLanguageSet(languageSet_);
}


//
// FUNCTION public
//	ModWordStemmer::~ModWordStemmer
//	-- Destructor
//
// NOTES
//	Destructor
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//
ModWordStemmer::~ModWordStemmer()
{
    lang.clear();
    langNum = 0;
}


//
// FUNCTION public
//	ModWordStemmer::setLanguageSet
//	-- Specify language
//
// NOTES
//	Language is specified to ModWordStemmer
//
// ARGUMENTS
//	const ModLanguageSet& languageSet
//
// RETURN
//	Nothing
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//
ModBoolean ModWordStemmer::setLanguageSet(
	const ModLanguageSet& languageSet_)
{
    if ( langNum >0){
        langNum = 0;
    }
    if ( languageSet_.isContained(ModLanguage::en)) {
        langNum++;
    }
    if ( languageSet_.isContained(ModLanguage::es)) {
        langNum++;
    }
    if ( languageSet_.isContained(ModLanguage::de)) {
        langNum++;
    }
    if ( languageSet_.isContained(ModLanguage::nl)) {
        langNum++;
    }
    if ( languageSet_.isContained(ModLanguage::fr)) {
       langNum++;
    }
    if ( languageSet_.isContained(ModLanguage::it)) {
        langNum++;
    }

    if ( langNum == 0){
#ifdef DEBUG
        ModErrorMessage << "ModWordStemmer: language invalid" 
            << ModEndl; 
#endif
        ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
            ModErrorLevelError);
    }

    lang = languageSet_;

    return ModTrue;
}

//
// FUNCTION public
//	ModWordStemmer::look -- Consultation of a dictionary
//
// NOTES
//	It is confirmed whether the object character sequence is registered into the dictionary.
//
// ARGUMENTS
// 	const ModUnicodeString& target
//		Target string for stemmer 
//
// RETURN
//	ModBoolean
//		ModFalse is returned.
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//
ModBoolean
ModWordStemmer::look(const ModUnicodeString& target)
{
	return ModFalse;
}


//
// FUNCTION public
//	ModWordStemmer::expand -- stemmer and expand
//
// NOTES
//	It is regular-ization about an object character sequence. 
//
// ARGUMENTS
//	const ModUnicodeString& target
//		target String
//	ModVector <ModUnicodeString>& result
//		vector of stemmer result
//
// RETURN
//	ModBoolean
//		Return ModTrue if get result succussfully
//		Return ModFalse if failed
//
// EXCEPTIONS
//	The exception from a low rank is returned.
//
ModBoolean
ModWordStemmer::expand(const ModUnicodeString& target, ModVector <ModUnicodeString>& result)
{
	result.pushBack(target);
	return ModFalse;
}

//
// Copyright (c) 2002-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
