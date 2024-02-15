// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Item.h -- コマンド要素のクラス
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

#ifndef __SYDNEY_SYDTEST_ITEM_H
#define __SYDNEY_SYDTEST_ITEM_H

#include "Common/Common.h"
#include "Common/Internal.h"


_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Item --
//
//	NOTES
//
//
//##ModelId=3A9B474601E9
class Item 
{
public:
	// 要素タイプ
	struct Type
	{
		enum Value
		{
			Command,
			String,
			Number,
			Parameter,
			Option
		};

		operator Value()
		{
			return m_eType;
		}

		Type& operator=(Value eType_)
		{
			m_eType = eType_;
			return *this;
		}

		Type::Type(Value eType)
			: m_eType(eType) {}

		Value m_eType;
	};

	//コンストラクタ
	//##ModelId=3A9B47460211
	explicit Item(Type eType_);
	//デストラクタ
	//##ModelId=3A9B47460210
	virtual ~Item();
	// 次の要素を設定する
	//##ModelId=3A9B4746020E
	void setNext(Item* pNext_);
	// 次の要素を得る
	//##ModelId=3A9B4746020D
	Item* getNext() const;
	// 要素タイプを得る
	//##ModelId=3A9B47460209
	Type getType() const;
	// 破棄
	//##ModelId=3A9B47460208
	void release();

private:
	// 次の要素
	//##ModelId=3A9B47460205
	Item* m_pNext;
	// 要素タイプ
	//##ModelId=3A9B474601FB
	Type m_eType;

};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_ITEM_H

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
