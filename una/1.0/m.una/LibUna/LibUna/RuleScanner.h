// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RuleScanner.h -- Definition file of RuleScanner class
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

#ifndef __RULESCANNER__HEADER__
#define __RULESCANNER__HEADER__

#include "RuleElementSet.h"
#include "Rule.h"
#include "ModUnicodeString.h"
#include "Module.h"

_UNA_BEGIN


	struct RuleElementSet;

// #T2#
	//
	//	CLASS
	//		RuleScanner --
	//
	class RuleScanner
	{
	public:
// #T3#
		// Constructor, Destructor
		RuleScanner();
		~RuleScanner();

		class Position {
		public:
			enum Type {
				ValueName = 0,
				Operate,
				RuleType,
				Option,
				POSRule,
				MarkRule,
				Unknown
			};
// #T4#
		}; // end of class Position

// #T5#
		// lexcal analyzer
		int					lex(RuleElementSet::Data* elem_);

		static
		Rule::RuleCommand			ruleID(const ModUnicodeString& command_);

	protected:

// #T6#
		// common lexcal analyzer
		int					common_lex(RuleElementSet::Data* elem_);


// #T7#
		// Convenient function
		void					setElement(ModVector<ModUnicodeString>& ary_,
								   const ModUnicodeString& elem_,
								   ModSize n_);
	private:

	};

_UNA_END

// #T10#
#endif // __RULESCANNER__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
