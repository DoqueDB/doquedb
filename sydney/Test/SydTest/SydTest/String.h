// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// String.h -- 文字列クラス
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

#ifndef __SYDNEY_SYDTEST_STRING_H
#define __SYDNEY_SYDTEST_STRING_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "SydTest/Item.h"
#include "ModUnicodeString.h"


_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::String --
//
//	NOTES
//
//
//##ModelId=3A9B474602D5
class  String : public Item
{
public:
	//コンストラクタ
	//##ModelId=3A9B474602EB
	explicit String(const ModUnicodeString& cstrString_);
	//##ModelId=3A9B474602ED
	explicit String(const char* pszString_);
	//デストラクタ
	//##ModelId=3A9B474602EA
	virtual ~String();
	// UCS2文字列を得る
	//##ModelId=3A9B474602E9
	const ModUnicodeString& getUnicodeString() const;
	// マルチバイト文字列を得る
	//##ModelId=3A9B474602E4
	const char* getString(ModKanjiCode::KanjiCodeType code = ModKanjiCode::shiftJis);
	// 文字列の長さを得る
	const int getLength();
private:
	// Unicode文字列
	//##ModelId=3A9B474602E1
	ModUnicodeString m_cstrString;

};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_STRING_H

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
