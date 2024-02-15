// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	Rule.cpp -- ルールクラスの実装ファイル
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

#include "LibUna/Rule.h"
#include "LibUna/Data.h"
#include "LibUna/UnicodeChar.h"
#include "ModUnicodeOstrStream.h"

_UNA_USING

//
// FUNCTION public
//	Rule::Rule
//		-- Rule クラスのコンストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Rule::Rule()
	 : _command(0)
{
}

#ifdef OBSOLETE
Rule::Rule(Rule::Command command_)
	 : _command(command_)
{
}
#endif

//
// FUNCTION public
//	Rule::~Rule
//		-- Rule クラスのデストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Rule::~Rule()
{
}

#ifdef OBSOLETE
//
// FUNCTION public
//	Rule::operator = 
//		--  コピー演算子
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Rule&
Rule::operator = (const Rule& src_)
{
	if ( this != &src_ ) {
		_command = src_._command;
		_option = src_._option;
	}
	return *this;
}
#endif

//
// FUNCTION public
//	Rule::getCommand
//		::setCommand
//		-- コマンド識別子の設定
//
// NOTES
//
// ARGUMENTS
//		Command com_
//			コマンド識別子
//
// RETURN
//		Command
//			コマンド識別子
//
// EXCEPTIONS
//
Rule::Command
Rule::getCommand() const
{
	return _command;
}

void
Rule::setCommand(Command com_)
{
	_command = com_;
}

//
//	FUNCTION pubic
//	Rule::getOption -- コマンドオプションの設定
//		::setOption
//
//	NOTES
//
//	ARGUMENTS
//		const CommandOption& option_
//
//	RETURN
//		const CommandOption& option_
//
//	EXCEPTIONS
//
const Rule::CommandOption&
Rule::getOption() const
{
	return _option;
}

#ifdef OBSOLETE
void
Rule::setOption(const Rule::CommandOption& option_)
{
	_option = option_;
}	
#endif

//
//	FUNCTION pubic
//	Rule::pushBackOption -- コマンドオプションの追加
//
//	NOTES
//
//	ARGUMENTS
//		const SmartPointer<Data>
//			追加するオプション
//
//	RETURN
//		void
//
//	EXCEPTIONS
//
void
Rule::pushBackOption(Data::Object* option_)
{
	try {
		_option.pushBack(option_);
	} catch ( ModException& exp ) {
		ModErrorMessage << "rule can't add option...[info:"
			<< exp << "]" << ModEndl;
		throw;
	}
}

#ifdef OBSOLETE
//
//	FUNCTION pubic
//	Rule::getMessage 
//
//	NOTES
//		文字列の取得(デバック、エラー出力用)
//		本当は純粋仮想にしたいが RuleHolder が
//		Rule クラスを持つ形なので仮想関数にしておく
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		void
//
//	EXCEPTIONS
//
ModUnicodeString
Rule::getMessage() const
{
	ModUnicodeOstrStream str;
	str << "(" << _command << ")";

	CommandOption::ConstIterator it  = _option.begin();
	CommandOption::ConstIterator fin = _option.end();
//- for ( ; it != fin; ++it )
//-	str << "[" << (!it ? (*it)->getMessage() : "no data") << "]";
	for ( ; it != fin; ++it ) {
		str << "[";
		if(!it)
			str << (*it)->getMessage();
		else
			str << "no data";
		str << "]";
	}

	return str.getString();
}
#endif

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
