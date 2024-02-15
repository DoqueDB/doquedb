// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RuleScanner.cpp -- Implement file of RuleScanner class
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/RuleScanner.h"
#include "LibUna/RuleElementSet.h"
#include "LibUna/UnicodeChar.h"
#include "LibUna/RuleParser.tab.hpp"

_UNA_USING

//
// FUNCTION public
//	RuleScanner::RuleScanner
//		-- RuleScanner class constructor
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
RuleScanner::RuleScanner()
{
}

//
// FUNCTION public
//	RuleScanner::~RuleScanner
//		-- RuleScanner class destructor
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
RuleScanner::~RuleScanner()
{
}	

namespace ScannerLocal {

// #T2#
	// name of command
	namespace dummy {
		ModUnicodeString Collect		= ModUnicodeString("collect");
		ModUnicodeString DeleteFromWhole	= ModUnicodeString("deleteword");
		ModUnicodeString DeleteFromBlock	= ModUnicodeString("delete");
		ModUnicodeString DeleteFromWholeDic	= ModUnicodeString("deleteword_dic");
		ModUnicodeString Group			= ModUnicodeString("group");
		ModUnicodeString PickupFromBlock	= ModUnicodeString("keyword");
		ModUnicodeString Language		= ModUnicodeString("language");
		ModUnicodeString PickupFromWhole	= ModUnicodeString("pickup");
		ModUnicodeString Sametype		= ModUnicodeString("sametype");
		ModUnicodeString DeleteTail		= ModUnicodeString("delete_tail");
		ModUnicodeString Version		= ModUnicodeString("version");
	}

// #T3#
	// command -- command string value
	static struct CommandPair {
		const ModUnicodeChar*	_comstr;
		Rule::RuleCommand	_value;
	} l_CommandPair[] = {
		{ dummy::Collect		, Rule::Collect	},
		{ dummy::DeleteFromWhole	, Rule::DeleteFromWhole	},
		{ dummy::DeleteFromBlock	, Rule::DeleteFromBlock	},
		{ dummy::Group			, Rule::Group },
		{ dummy::PickupFromBlock	, Rule::PickupFromBlock	},
		{ dummy::Language		, Rule::Language },
		{ dummy::PickupFromWhole	, Rule::PickupFromWhole	},
		{ dummy::Sametype		, Rule::SameType },
		{ dummy::DeleteTail		, Rule::DeleteTail },
		{ dummy::Version		, Rule::Version },
		{ 0, Rule::Num }
	};

// #T4#
	// FUNCTION pubic
	// getCommandValue -- get the message command
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeString& str_
	//		search command message
	//
	// RETURN
	//	ExtructRule::Command::Value
	//		command value
	//		If it is not found, Rule::Command::Num return
	//
	// EXCEPTIONS
	Rule::RuleCommand
	getCommandValue(const ModUnicodeString& str_)
	{
		for ( int i = 0; ScannerLocal::l_CommandPair[i]._comstr != 0; ++i ) {
			if ( str_ == ScannerLocal::l_CommandPair[i]._comstr ) {
				return ScannerLocal::l_CommandPair[i]._value;
			}
		}
		return Rule::Num;
	}

// #T5#
} // end of namesapce Local

namespace {
	ModBoolean
	isSeparator(const ModUnicodeChar*  tgt_)
	{
		using namespace UnicodeChar;

// #T6#
		// separater is ...
		//	( , ) , { , }, &, ~, |, =
		if ( *tgt_ == usLparent ||
			 *tgt_ == usRparent ||
			 *tgt_ == usLbrace  ||
			 *tgt_ == usRbrace  ||
			 *tgt_ == usAmpersand ||
			 *tgt_ == usAboveLine ||
			 (*tgt_ == usVerticalLine && *(tgt_+1) == usEqual) ||
			 *tgt_ == usEqual ) {
			return ModTrue;
		}
		return ModFalse;
	}
// #T7#
} // end of namesapce


// #T8#
// FUNCTION pubic
// 	RuleScanner::lex
//		-- rule lexcal analyzer for Bison
//
// NOTES
//
// ARGUMENTS
//	void* param_
//		param_ is LexScanner object which was defined RuleMaker.
//
// RETURN
//	int
//		token value
//
// EXCEPTIONS
int
RuleScanner::common_lex(RuleElementSet::Data* elem_)
{
	const ModUnicodeChar*&  strpos = elem_->_strpos;
	const ModUnicodeChar*&  endpos = elem_->_endpos;
	ModUnicodeString&		tmpstr = elem_->_tmpstr;

// #T9#
	// main loop for scanning
	using namespace UnicodeChar;
	for ( ; strpos < endpos; ++strpos ) {

// #T10#
		//--- detection of string end
		if ( *strpos == 0 ) return NL;

// #T11#
		//--- skip for comment
		if ( *strpos == usSharp ) {
			while ( *strpos != usCtrlRet && strpos < endpos ) ++strpos;
			continue;
		}

// #T12#
		//--- skip for blank space
		if ( *strpos == usSpace ) {
			continue;
		}

// #T13#
		//--- except process
		if ( *strpos == usCtrlRet ) {
			++strpos;
			return NL;
		}

// #T14#
		//--- return the separator
		if ( isSeparator(strpos) ) {
			++strpos;
			return int(*(strpos-1));
		}

// #T15#
		//--- word whith some meanings

// #T16#
		// start position
		const ModUnicodeChar* startpos = strpos;

// #T17#
		// Find the delimitation
		while ( *strpos != usCtrlRet && strpos < endpos ) {

// #T18#
			// detection of place enclosed with '"'
			if ( *strpos == usWquate ) {
				while ( *strpos != usCtrlRet && *strpos == usWquate
						&&strpos < endpos ) ++strpos;
			}

// #T19#
			// skip for back slash
			if ( *strpos == usBackSlash ) {
				for ( int i = 0;
					  i < 2 && *strpos != usCtrlRet && strpos < endpos;
					  ++i, ++strpos )	;
			}

			if ( isSeparator(strpos) || *strpos == usSpace ) {
// #T20#
				// it discovered the delimitation

// #T21#
				// making of string
				//  - target it from starting position(startpos) to strpos
				tmpstr = ModUnicodeString(startpos, (ModSize)(strpos - startpos));

// #T22#
				// regognizing the string
				return RULE_STRING;
			}

// #T23#
			// increase the charactor position
			++strpos;

// #T24#
		} // end of Find the delimitation

// #T25#
	} // end of main loop for scanning

	return 0;
}

// #T26#
// FUNCTION pubic
// RuleScanner::setElement
//
// NOTES
//
// ARGUMENTS
//	ModVector<ModUnicodeString>& ary_
//	const ModUnicodeString& elem_
//	ModSize n_
//
// RETURN
//
// EXCEPTIONS
void
RuleScanner::setElement(ModVector<ModUnicodeString>& ary_,
			const ModUnicodeString& elem_,
			ModSize n_)
{
	ModSize max = ary_.getSize();
	if ( n_ >= max ) {
		ary_.insert(ary_.end(), n_ - max, ModUnicodeString());
		ary_.pushBack(elem_);
	} else {
		ary_[n_] = elem_;
	}
}


// #T27#
// FUNCTION pubic
// RuleScanner::lex
//
// NOTES
//
// ARGUMENTS
//	RuleElementSet* elem_
//
// RETURN
//	int
//
// EXCEPTIONS
int
RuleScanner::lex(RuleElementSet::Data* elem_)
{
	int ret = common_lex(elem_);

// #T28#
	// setting of command id
	switch ( elem_->_prestep ) {
	case Rule::Step::RuleType:
		elem_->_rule->setCommand(ruleID(elem_->_tmpstr));
		break;
	case Rule::Step::ValueName:
		setElement(elem_->_strary,
				   elem_->_tmpstr,
				   Position::ValueName);
		break;
	case Rule::Step::Operate:
		setElement(elem_->_strary,
				   elem_->_tmpstr,
				   Position::Operate);
		break;
	case Rule::Step::Option:
		setElement(elem_->_strary,
				   elem_->_tmpstr,
				   Position::Option);
		break;
	case Rule::Step::Arg1:
		setElement(elem_->_strary,
				   elem_->_tmpstr,
				   Position::POSRule);
		break;
	case Rule::Step::Arg2:
		setElement(elem_->_strary,
				   elem_->_tmpstr,
				   Position::MarkRule);
		break;
// #T29#
	} // end of setting of command id

	return ret;
}

// #T30#
// FUNCTION pubic
// RuleScanner::ruleID
//		-- acquire command ID from the rule name
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& command_
//
// RETURN
//	Rule::RuleCommand
//
// EXCEPTIONS
Rule::RuleCommand
RuleScanner::ruleID(const ModUnicodeString& command_)
{
	return ScannerLocal::getCommandValue(command_);
}

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
