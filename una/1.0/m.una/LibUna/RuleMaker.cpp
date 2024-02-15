// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RuleMaker.cpp -- Impliment file of RuleMaker
// 
// Copyright (c) 2004-2008, 2023 Ricoh Company, Ltd.
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

#include "LibUna/Rule.h"
#include "LibUna/RuleMaker.h"
#include "LibUna/RxTools.h"
#include "LibUna/RuleScanner.h"
#include "LibUna/RuleHolder.h"
#include "ModAutoPointer.h"

_UNA_USING

#define	OPTION_POS RuleScanner::Position

#ifdef _DEBUG
#define	OBJECT_USTRING(_obj_) dynamic_cast<Data::UnicodeString*>(_obj_)->getData()
#else
#define	OBJECT_USTRING(_obj_) static_cast<Data::UnicodeString*>(_obj_)->getData()
#endif

//
// FUNCTION public
//	RuleMaker::RuleMaker
//		-- RuleMaker class constructor
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//

RuleMaker::RuleMaker(DicSet* dicSet_, const ModUnicodeString& file_)
	 : UnicodeFile(file_, ModFile::readMode), m_dicSet(dicSet_)
{
}

//
// FUNCTION public
//	RuleMaker::~RuleMaker
//		-- RuleMaker class destructor
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
RuleMaker::~RuleMaker()
{
}

// define the unit length UsingLen( size ) rule
// processing document length = size * 1024
//const int UnitLength = 1024;

/*---- Implementation of RuleMaker Class -------------------------------*/

namespace Local {

	const ModUnicodeChar	GroupDelim		=	UnicodeChar::usPercent;

	// initial value of value name
	const ModUnicodeString	DefaultValueName	=	ModUnicodeString("_DEF_VALUE_");

	const ModUnicodeString	And			=	ModUnicodeString("and");
	const ModUnicodeString	Or			=	ModUnicodeString("or");
	const ModUnicodeString	ExOr			=	ModUnicodeString("xor");

} // end of namespace Local

//
// FUNCTION pubic
// RuleMaker::execute  --
//
// NOTES
//
// ARGUMENTS
// 	Common::RuleElementSet::Data* data_
// 		data set for parse and lexical analyze
//
// RETURN
//	void
//
// EXCEPTIONS
//
void
RuleMaker::execute(RuleElementSet::Data* data_)
{
	ModAutoPointer<Rule>& rule = data_->_rule;
	ModVector<ModUnicodeString> & argument = data_->_strary;

	// type of rule
	rule->setCommand(RuleScanner::ruleID(argument[OPTION_POS::RuleType]));

	// analytical execution
	Rule::RuleCommand cmd = (Rule::RuleCommand)rule->getCommand();
	if ( preProcess(cmd, argument) ) {
		ModAutoPointer<Data::Object> valueName;
		ModAutoPointer<Data::Object> operate;
		ModAutoPointer<Data::Object> option;
		ModAutoPointer<Data::Object> posrule;
		ModAutoPointer<Data::Object> markrule;

		// obtain value name
		if ( argument.getSize() > OPTION_POS::ValueName ) {
			valueName = getValueName(cmd, argument[OPTION_POS::ValueName]);
		}

		// obtain the operator
		if ( argument.getSize() > OPTION_POS::Operate ) {
			operate = getOperator(cmd, argument[OPTION_POS::Operate]);

			// interchnageability check
			if ( OBJECT_USTRING(valueName.get()) == Local::DefaultValueName )
				operate = getOperatorForInterchnageability(cmd,
				 argument[OPTION_POS::Option]);

		}

		// obtain the option
		if ( argument.getSize() > OPTION_POS::Option ) {
			option = getOption(cmd, argument[OPTION_POS::Option]);
		}

		// obtain POS information
		if ( argument.getSize() > OPTION_POS::POSRule ) {
			posrule = getPosRule(cmd, argument[OPTION_POS::POSRule]);
		}

		// obtain mark information
		if ( argument.getSize() > OPTION_POS::MarkRule ) {
			markrule = getMarkRule(cmd, argument[OPTION_POS::MarkRule]);
		}

		// make rule because the material became complete.
		//  do the generation management of Data::Object in rule
		rule->pushBackOption(valueName.release());
		rule->pushBackOption(operate.release());
		rule->pushBackOption(0); // dummy for RuleType
		rule->pushBackOption(option.release());
		rule->pushBackOption(posrule.release());
		rule->pushBackOption(markrule.release());

		RuleHolder::addRule( m_dicSet, rule);

		// init of RuleElementSet
		{
			rule = new Rule;
		}
	}

	// initialize of buffer
	argument.clear();

}

//
// FUNCTION pubic
// RuleMaker::preProcess -- preprocess before compile
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand::Value cmd_
//		command type
//	const ModVector<ModCharString>& option_
//		option string
//
// RETURN
//	ModBoolean  	ModTrue  : need compile
// 			ModFalse : needn't compile
//
// EXCEPTIONS
//
ModBoolean
RuleMaker::preProcess(Rule::RuleCommand cmd_,
		   ModVector<ModUnicodeString>& option_)
{
	switch ( cmd_ ) {
	case Rule::Group:
	// group
		registGroup(option_);

	// command don't need compiling
		return ModFalse;

	case Rule::SameType:
	// set same type part of speech

		registSameType(option_);

		// command don't need compiling
		return ModFalse;

	case Rule::DeleteFromWhole:// deleteKeyword
	case Rule::DeleteFromBlock:// deleteWord
	case Rule::PickupFromBlock:// pickupkeyword
	case Rule::PickupFromWhole:// pickupword
	case Rule::DeleteTail:// pickupword
		{
			// replace group string

			// 品詞表記に関して処理
			if ( option_.getSize() > OPTION_POS::POSRule &&
				 option_[OPTION_POS::POSRule].getLength() > 0 ) {

				ModUnicodeString tmp;
				RxTools::findReplaceString(option_[OPTION_POS::POSRule],
										   *_groupstring,
										   Local::GroupDelim,
										   tmp);
				option_[OPTION_POS::POSRule] = tmp;
			}

			// 表記に関して処理
			if ( option_.getSize() > OPTION_POS::MarkRule &&
				 option_[OPTION_POS::MarkRule].getLength() > 0 ) {

				ModUnicodeString tmp;
				RxTools::findReplaceString(option_[OPTION_POS::MarkRule],
										   *_groupstring,
										   Local::GroupDelim,
										   tmp);
				option_[OPTION_POS::MarkRule] = tmp;
			}
		}
		// command need compiling
		break;
	default:
		// コンパイルが必要なコマンドとみなす
		;
	}
	return ModTrue;
}

//=== 非ルールコマンド ======================================================
//
// FUNCTION pubic
//	RuleMaker::registGroup -- reserve group command list
//
// NOTES
//
// ARGUMENTS
//	const ModVector<ModUnicodeString>& option_
//		option string
//
// RETURN
//
// EXCEPTIONS
//
void
RuleMaker::registGroup(const ModVector<ModUnicodeString>& option_)
{
	try {

		//--- register group list ---
		if ( !_groupstring.get() ) {
			// まだ実体が出来ていないなら作成する
			_groupstring = new GroupString;
		}

		// 同じグループ名はエラーとする
		const ModUnicodeString& gname = option_[OPTION_POS::Option];
		if ( _groupstring->find(gname) != _groupstring->end() ) {
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument,
						 ModErrorLevelError);
		}

		// パターンは品詞列、表記のどちらかにしか許可しない
		const ModUnicodeString* pattern = 0;
		if ( option_.getSize() > OPTION_POS::MarkRule &&
			 option_[OPTION_POS::MarkRule].getLength() > 0 ) {
			pattern = &(option_[OPTION_POS::MarkRule]);
		} else {
			pattern = &(option_[OPTION_POS::POSRule]);
		}

		// register
		_groupstring->insert(gname, *pattern);

	} catch ( ModException& exp ) {
		ModErrorMessage << "group list can't retist...[info:" << exp << "]" << ModEndl;
		throw;
	}
}

//
// FUNCTION pubic
// RuleMaker::registSameType -- 同グループに指定する品詞名を指定する
//
// NOTES
//
// ARGUMENTS
//	const ModVector<ModUnicodeString>& option_
//		option string
//
// RETURN
//
// EXCEPTIONS
//
void
RuleMaker::registSameType(const ModVector<ModUnicodeString>& option_)
{
	// get head value
	if ( option_.getSize() > OPTION_POS::Option ) {

		const ModUnicodeString& tgtstr = option_[OPTION_POS::Option];

		// 文字列の記述をcheck
		if (tgtstr != Local::GroupDelim &&
			tgtstr[tgtstr.getLength()-1] != Local::GroupDelim)
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument,
					 ModErrorLevelError);

		// グループデリミタで囲まれている文字列を取り出す
		ModUnicodeString tgt = ModUnicodeString(&tgtstr[1], tgtstr.getLength()-2);
		ModLanguageSet lang = m_dicSet->getLanguageSet();
		// 言語別に処理するグループを登録する
		if(m_dicSet->getLanguageSet().isContained( ModLanguage::ja ) == ModTrue)
			m_dicSet->registSameType(tgt);
		else
			ModThrow(ModModuleStandard,
					 ModCommonErrorBadArgument,
					 ModErrorLevelError);
	} else {
		// 記述が間違っている
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
}

//=== ルールコマンド ========================================================
//
// FUNCTION pubic
// RuleMaker::getValueName
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getValueName(Rule::RuleCommand value_,
			const ModUnicodeString& option_)
{
	if ( option_.getLength() == 0 )
		return new Data::UnicodeString(Local::DefaultValueName);

	return new Data::UnicodeString(option_);
}

//
// FUNCTION pubic
// RuleMaker::getOperator
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getOperator(Rule::RuleCommand value_,
			const ModUnicodeString& option_)
{
	using namespace Type;
	using namespace Algorithm;

	BitsetOperator::Type optype;
	if(m_dicSet->getLanguageSet().isContained( ModLanguage::ja ) )
	{
		switch ( CalcType::getOperateValue(option_) ) {
		case CalcType::Equal:		optype = BitsetOperator::Equal;	break;
		case CalcType::AndEqual:	optype = BitsetOperator::And;	break;
		case CalcType::OrEqual:		optype = BitsetOperator::Or;	break;
		case CalcType::XorEqual:	optype = BitsetOperator::ExOr;	break;
		default:
			return 0;
		}
	}
	else
	{
		switch ( CalcType::getOperateValue(option_) ) {
		case CalcType::Equal:		optype = BitsetOperator::Equal;	break;
		case CalcType::AndEqual:	optype = BitsetOperator::And;	break;
		default:
			return 0;
		}
	}
	return new Data::BitsetOperator( BitsetOperator(optype) );
}

//
// FUNCTION pubic
// RuleMaker::getOption
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getOption(Rule::RuleCommand value_,
			const ModUnicodeString& option_)
{
	return 0;
}

//
// FUNCTION pubic
// RuleMaker::getPosRule
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getPosRule(Rule::RuleCommand value_,
			const ModUnicodeString& option_)
{
	// 正規表現クラスをcomplie
	try {
		if ( option_.getLength() > 0 ) {
			// pdst_ には RegularExpression* を格納する
			ModAutoPointer<Data::RegularExpression> prx
				= new Data::RegularExpression();

			ModUnicodeString pattern;
	
			m_dicSet->getTypeCodeString(option_,pattern);

			Type::RegularExpression& rx = prx->getData();

			rx._rx->compile(pattern);

			return prx.release();
		}

	} catch ( ModException& ) {
		ModErrorMessage << "regular expression pattern can't compile for type...\""
			<< "\"" << ModEndl;
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}

	return 0;
}

//
// FUNCTION pubic
// RuleMaker::getMarkRule
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getMarkRule(Rule::RuleCommand value_,
			const ModUnicodeString& option_)
{
	try
	{
		// pdst_ は RegularExpression のはず
		ModAutoPointer<Data::RegularExpression> prx
			= new Data::RegularExpression();

		// 正規表現クラスをコンパイル
		Type::RegularExpression& rx = prx->getData();
		rx._rx->compile(option_);

		return prx.release();

	} catch ( ModException& exp) {
		ModErrorMessage << "regular expression pattern can't compile for write...\""
			<< exp << ModEndl;
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}

	return 0;
}

//
// FUNCTION pubic
// RuleMaker::getOperatorForInterchnageability
//
// NOTES
//
// ARGUMENTS
//	Rule::RuleCommand value_
//	const ModUnicodeString& option_
//
// RETURN
//	Data::Object*
//
// EXCEPTIONS
//
Data::Object*
RuleMaker::getOperatorForInterchnageability(Rule::RuleCommand value_,
					 const ModUnicodeString& option_)
{
	switch ( value_ ) {
	case Rule::DeleteFromBlock:
	case Rule::DeleteFromWhole:
	case Rule::PickupFromWhole:
	case Rule::PickupFromBlock:
	case Rule::DeleteTail:
		{
			using namespace Algorithm;

			// argument は演算子のはず
			BitsetOperator::Type type = BitsetOperator::Fin;
			if ( option_ == Local::And || option_.getLength() == 0 ) {
				type = BitsetOperator::And;
			} else if ( option_ == Local::Or ) {
				type = BitsetOperator::Or;
			} else if ( option_ == Local::ExOr ) {
				type = BitsetOperator::ExOr;
			}

			if ( type != BitsetOperator::Fin )
				return new Data::BitsetOperator(BitsetOperator(type));
		}
		break;
	}
	return 0;
}

//
// FUNCTION pubic
// RuleMaker::parse -- parse the rule
//
// NOTES
//	this function is static.
//
// ARGUMENTS
//	RuleScanner*	scanner_
//		lexcal analyzer
//	:RuleMaker*	maker_
//		object pointer of rule maker
//
// RETURN
//	int
//
// EXCEPTIONS
//
int
RuleMaker::parse(RuleScanner* scanner_,
		 RuleMaker*  maker_)
{
	RuleElementSet elemset;
	ModUnicodeString line;
	while ( maker_->getLine(line) )
	{
		// parse it every one line.
		if ( line.getLength() > 0 ) {
#if 1
			elemset._data._string = line;
			elemset._data._strpos = line;
			elemset._data._endpos = elemset._data._strpos + line.getLength();
			elemset._data._tmpstr.clear();
			elemset._data._prestep = Rule::Step::Unknown;
			elemset._data._rule = new Rule();
			elemset._data._strary.clear();

			elemset._object._scanner = scanner_;
			elemset._object._maker = maker_;
#else
			elemset._data._string = line;
			elemset._data._strpos = line;
			elemset._data._endpos = elemset._data._strpos + line.getLength();

			elemset._data._rule = new Rule();

			elemset._object._scanner = scanner_;
			elemset._object._maker = maker_;
#endif
			yyparse(&elemset);
		}
	}
	return 0;
}

//
// Copyright (c) 2004-2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
