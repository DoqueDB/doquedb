// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executer.h -- Definition file of Executer class
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_LIBUNA_EXECUTER_H
#define __UNA_LIBUNA_EXECUTER_H

#include "Module.h"
#include "Bitset.h"
#include "ModMap.h"
#include "ModVector.h"


#define		_EXECUTER_BEGIN		namespace Executer {
#define		_EXECUTER_END		}
#define 	_EXECUTER_USING		using namespace Executer;

// #T2#
// Empty definition
class ModUnicodeRegularExpression;
class ModUnicodeString;

_UNA_BEGIN

	class Keyword;
// #T3#
	//class Sentence;
	class Morph;
	namespace Type
	{
		struct RegularExpression;
	}


namespace Executer {

// #T4#
	// Combination of all keywords is added from the line of morpheme
	void				getKeywordCombination(ModVector<Morph>::ConstIterator it_,
							  	ModVector<Morph>::ConstIterator fin_,
							  	ModMap<ModUnicodeString, Keyword,
								ModLess<ModUnicodeString> >& keyword_,
								ModVector<Keyword*>* pNewKeyword = 0);

// #T5#
	// The part of speech line regular expression result of morpheme is acquired
	void				getWordTypeRx(Type::RegularExpression& rxdata_,
							const ModUnicodeChar* typeString_,
							unsigned int start_,
							unsigned int len_,
							Bitset& result_,
							ModBoolean flag_ = ModTrue);

// #T6#
	// The declared regular expression result of morpheme is acquired
	void				getWordTextRx(Type::RegularExpression& rxdata_,
							ModVector<Morph>::ConstIterator wordIt_,
							unsigned int start_,
							unsigned int len_,
							Bitset& keyword_,
							ModBoolean flag_ = ModTrue);

// #T7#
	// The declared regular expression result of morpheme is acquired (document entire version)
	void				getWordTextRxForAll(Type::RegularExpression& rxdata_,
								ModVector<Morph>::ConstIterator startMorph_,
								ModVector<Morph>::ConstIterator endMorph_,
								const ModUnicodeChar* targetstr_,
								unsigned int start_,
								unsigned int len_,
								Bitset& keyword_,
								ModBoolean flag_);


// #T8#
} // end of namespace Executer

_UNA_END

// #T9#
#endif // __UNA_LIBUNA_EXECUTER_H

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
