// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Number.h -- 数値クラス
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

#ifndef __SYDNEY_SYDTEST_NUMBER_H
#define __SYDNEY_SYDTEST_NUMBER_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "Item.h"


_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Number --
//
//	NOTES
//
//
//##ModelId=3A9B474603B2
class  Number :  public Item
{
public:
	//コンストラクタ
	//##ModelId=3A9B474603C6
	explicit Number(const char* pszString_);
	//デストラクタ
	//##ModelId=3A9B474603BF
	virtual ~Number();
	// 数値の値を得る
	//##ModelId=3A9B474603BE
	int getNumber() const;

private:
	// 整数値
	//##ModelId=3A9B474603BD
	int m_iNumber;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_NUMBER_H

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
