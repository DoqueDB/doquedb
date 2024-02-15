// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Number.cpp -- 数値要素を表すクラス
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
#include "SydTest/Number.h"

#include <stdlib.h>

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::Number::Number -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	char* pszString_
//		文字列 ただし数字のみを含む
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Number::Number(const char* pszString_)
: Item(Type::Number)
{
	m_iNumber = atoi(pszString_);
}

//
//	FUNCTION public
//	SydTest::Number::~Number -- デストラクタ
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
Number::~Number()
{
}

//
//	FUNCTION public
//	SydTest::Number::getNumber -- 整数を得る
//
//	NOTES
//	現在のオブジェクトが示す数値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		整数値
//
//	EXCEPTIONS
//	なし
//
int
Number::getNumber() const
{
	return m_iNumber;
}
//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
