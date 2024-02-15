// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RuleApplier.h -- Definition file of RuleApplier class
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

#ifndef __RULEAPPLIER__HEADER__
#define __RULEAPPLIER__HEADER__

#include "LibUna/Data.h"
#include "LibUna/Bitset.h"
#include "LibUna/Rule.h"
#include "Morph.h"
#include "LibUna/Keyword.h"
#include "LibUna/Type.h"
#include "ModLanguage.h"
#include "ModVector.h"
#include "Module.h"

_UNA_BEGIN

	class DicSet;
	// ENUM
	// ProcessTarget::Type -- process target identifier
	enum RuleType
	{
		    Unknown = 0,	// unkown
		    POS,		// part of speech
		    WRITE		// 表記
	};

	//
	//	CLASS
	//		RuleApplier -- rule applier
	//
	//	MEMO
	//		extract keyword candidate from morph array

	class RuleApplier
	{
	public:
		// Constructor, Destructor
		RuleApplier(DicSet *dicSet_);
		~RuleApplier();

		// apply rules
		void				applies(ModVector<Morph>::ConstIterator srcIt_,
							ModVector<Morph>::ConstIterator srcFin_,
							ModVector<Keyword>& keyword_);

	protected:

		const ModUnicodeString&		getValueName(const Rule::CommandOption& options_);

		// make keyword
		void				makeKeyword(ModVector<Morph>::ConstIterator tgtWord_,
							    ModVector<Morph>::ConstIterator finWord_,
							    const Bitset& flag_,
							    const ModVector< ModPair<Type::CalcMaterial,
							    Bitset> >& scoreinfo_,
							    KeywordMap& keyword_);

		// apply pickupword rule
		void				pickupFromWhole(ModVector<Morph>::ConstIterator srcIt_,
								ModVector<Morph>::ConstIterator srcFin_,
								const ModUnicodeChar* typeString_,
								const Rule::CommandOption& option_,
								Bitset& keywordFlag_,
								RuleType processing_,
								ModBoolean setFlag_ = ModTrue);

		// apply pickupKeyword rule
		void				pickupFromBlock(ModVector<Morph>::ConstIterator wordIt_,
								const ModUnicodeChar* typeString_,
								const Rule::CommandOption& option_,
								Bitset& wordFlag_,
								RuleType processing_,
								ModBoolean setFlag_ = ModTrue);

		/*
		void				makeScorePreParation(ModVector<Morph>::ConstIterator wordIt_,
									ModVector<Morph>::ConstIterator wordFin_,
								    const ModUnicodeChar* typeString_,
								    const Rule::CommandOption& option_,
								    Bitset& wordFlag_,
									RuleType processing_,
								    ModVector< ModPair< Type::CalcMaterial,
								    Bitset> >& scoreinfo_);
		*/

		// set NounPhrase flag for keyword from concept extract result
		void				setNPflag(ModVector<Morph>::ConstIterator srcIt_,
									ModVector<Morph>::ConstIterator srcFin_,
									Bitset& wordFlag_);

		// delete the word at the end of NounPhrase
		void				deleteTail(ModVector<Morph>::ConstIterator wordIt_,
									const ModUnicodeChar* typeString_,
									const Rule::CommandOption& option_,
									Bitset& wordFlag_,
									RuleType processing_,
									ModBoolean setFlag_ = ModTrue);
	private:
		DicSet *m_dicSet;
	};

_UNA_END
#endif // __RULEAPPLIER__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
