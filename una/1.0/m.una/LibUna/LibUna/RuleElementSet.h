// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RuleElementSet.h -- Definition file of RuleElementSet class
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

#ifndef __RULEELEMENTSET__HEADER__
#define __RULEELEMENTSET__HEADER__

#include "Rule.h"
#include "ModAutoPointer.h"
#include "ModLanguage.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "Module.h"

_UNA_BEGIN


	class Bitset;
	class RuleScanner;
	class RuleMaker;
	
// #T2#
	//
	//	STRUCT
	//		RuleElementSet -- element set for making rule
	//
	struct RuleElementSet
	{
// #T3#
		// the following value is datas for parse and lex
		struct Data {
// #T4#
			ModLanguageCode			_lang;		// value of target language
			const ModUnicodeChar*		_string;	// start position of target string
			const ModUnicodeChar*		_strpos;	// target position of target string
			const ModUnicodeChar*		_endpos;	// end of target string
			ModUnicodeString		_tmpstr;	// temporary string
	
// #T5#
			// using rule
			Rule::Step::Type		_prestep;	// type of immediately before step 
			
// #T6#
			ModAutoPointer<Rule>		_rule;		// substance of rule
			ModVector<ModUnicodeString>	_strary;	// element string of rule
		} _data;
	
// #T7#
		// the following value is objects for parse and lex
		struct Object {
// #T8#
			RuleScanner*			_scanner;	// pointer of RuleScanner
			RuleMaker*			_maker;		// pointer of RuleMaker
			Bitset*				_targetresult;	// target result in rule
		} _object;
	
	};

_UNA_END
// #T9#
#endif // __RULEELEMENTSET__HEADER__

//
// Copyright (c) 2002-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
