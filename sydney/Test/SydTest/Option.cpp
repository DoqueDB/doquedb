// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.cpp -- オプション名を表すクラス
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/Option.h"
#include "ModCriticalSection.h"
#include "ModAutoMutex.h"
#include <stdlib.h>
#include <iostream>
_SYDNEY_USING

using namespace SydTest;

namespace {

	struct OptionItem
	{
		const char* m_pszOption;
		Option::OptionType::Value m_eType;
	};

	OptionItem _pOptionItem[] = {
		{"CascadeID",      Option::OptionType::CascadeID},
		{"ConnectionID",   Option::OptionType::ConnectionID},
		{"SessionID",      Option::OptionType::SessionID},
		{"DBName",         Option::OptionType::DBName},
		{0,                Option::OptionType::Other}
	};
}

_SYDNEY_USING

using namespace SydTest;


// static変数の宣言
std::map<std::string, Option::OptionType::Value>
Option::m_mapOptionItem;

ModCriticalSection
Option::m_cCriticalSection;

//
//	FUNCTION public
//	SydTest::Option::Option -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	char* pszString_
//		文字列 ただし末端がコロン(:)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Option::Option(const char* pszString_)
	: Item(Type::Option), 
	  m_cstrOptionName(pszString_, 0, ModOs::Process::getEncodingType())
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	if (m_mapOptionItem.size() == 0)
	{
		OptionItem* pItem = _pOptionItem;
		while (pItem->m_pszOption != 0)
		{
			m_mapOptionItem.insert(std::map<std::string, Option::OptionType::Value>::value_type(pItem->m_pszOption, pItem->m_eType));
			pItem++;
		}
	}

	std::map<std::string, Option::OptionType::Value>::iterator i = m_mapOptionItem.find(pszString_);
	if (i == m_mapOptionItem.end())
	{
		m_eOptionType = OptionType::Other;
	}
	else
	{
		m_eOptionType = (*i).second;
	}
}

//
//	FUNCTION public
//	SydTest::Option::~Option -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Option::~Option()
{
}


//
//	FUNCTION public
//	SydTest::Option::getOption -- コマンド文字列を取り出す
//
//	NOTES
//	コマンドを表す文字列を取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString&
//		コマンドを表す文字列
//
//	EXCEPTIONS
//	なし
//
//
const ModUnicodeString&
Option::getOptionName() const
{
	return m_cstrOptionName;
}

//
//	FUNCTION public
//	SydTest::Option::getOptionName -- オプション名を得る
//
//	NOTES
//	現在のオブジェクトが示すオプション名を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Option::OptionType
//		オプション名
//
//	EXCEPTIONS
//	なし
//

Option::OptionType
Option::getOptionType() const
{
	return m_eOptionType;
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
