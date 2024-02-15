// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RuleMaker.h -- Definition file of RuleMaker class
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

//
//	outline: move yyparse() to local namespace
//

#ifndef __RULEMAKER__HEADER__
#define __RULEMAKER__HEADER__

#include "RuleElementSet.h"
#include "LibUna/DicSet.h"
#include "Rule.h"
#include "UnicodeFile.h"
#include "ModLanguageSet.h"
#include "ModVector.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "Module.h"

_UNA_BEGIN

	int yyparse (void * param_);
	
	struct RuleElementSet;
	//
	//	CLASS
	//		RuleMaker --
	//
	class RuleMaker : public UnicodeFile
	{
	public:

		RuleMaker(DicSet *dicSet_,
			  const ModUnicodeString& file_);
		~RuleMaker();

		void			execute(RuleElementSet::Data* data_);

		static
		int			parse(RuleScanner* scanner_, RuleMaker*  maker_);
	private:
		DicSet	*m_dicSet;
		ModBoolean 	preProcess(Rule::RuleCommand cmd_, ModVector<ModUnicodeString>& option_);

		// group コマンドのリストを保存する
		void registGroup(const ModVector<ModUnicodeString>& option_);

		// 同グループに指定する品詞名を指定する
		void registSameType(const ModVector<ModUnicodeString>& option_);

		Data::Object*
		getValueName(Rule::RuleCommand value_,
					 const ModUnicodeString& option_);

		Data::Object*
		getOperator(Rule::RuleCommand value_,
					const ModUnicodeString& option_);

		Data::Object*
		getOperate(Rule::RuleCommand value_,
				   const ModUnicodeString& option_);

		Data::Object*
		getOption(Rule::RuleCommand value_,
				  const ModUnicodeString& option_);

		Data::Object*
		getPosRule(Rule::RuleCommand value_,
				   const ModUnicodeString& option_);

		Data::Object*
		getMarkRule(Rule::RuleCommand value_,
					const ModUnicodeString& option_);

		Data::Object*
		getOperatorForInterchnageability(Rule::RuleCommand value_,
						 const ModUnicodeString& option_);

		// TYPEDEF
		// 	GroupString -- group コマンド用の文字列スタック
		typedef		ModMap< ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
							GroupString;

		ModAutoPointer<GroupString>		_groupstring;

	};
_UNA_END
#endif //__RULEMAKER__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
