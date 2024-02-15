// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.h -- オプション名を表すクラス
// 
// Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_OPTION_H
#define __SYDNEY_SYDTEST_OPTION_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModCriticalSection.h"
#include "ModUnicodeString.h"
#include "Item.h"
#include <map>
#include <string>

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Option -- オプション名を表すクラス
//
//	NOTES
//
//
class  Option :  public Item
{
public:
	//オプションタイプ
	struct OptionType
	{
		enum Value
		{
			Other,
			ConnectionID,
			SessionID,
			CascadeID,
			DBName
		};

		operator Value()
		{
			return m_eOptionType;
		}

		OptionType& operator=(Value eType_)
		{
			m_eOptionType = eType_;
			return *this;
		}

		Value m_eOptionType;
	};

	//コンストラクタ
	explicit Option(const char* pszString_);
	//デストラクタ
	virtual ~Option();
	//オプション名の文字列を得る
	const ModUnicodeString& getOptionName() const;
	// オプションタイプを得る
	OptionType getOptionType() const;

private:
	// オプション名文字列
	ModUnicodeString m_cstrOptionName;
	// オプションタイプ
	OptionType m_eOptionType;

	// オプションを格納するマップ
	static std::map<std::string, OptionType::Value> m_mapOptionItem;

	static ModCriticalSection m_cCriticalSection;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_OPTION_H

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
