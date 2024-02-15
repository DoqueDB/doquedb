// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// String.cpp -- 文字列を表すクラス
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "SydTest/String.h"

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::String::String -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	ModUnicodeString cstrString_
//		オブジェクトに格納する文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
String::String(const ModUnicodeString& cstrString_)
: Item(Type::String), m_cstrString(cstrString_)
{
}

//
//	FUNCTION public
//	SydTest::String::String -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	char* pszString_
//		オブジェクトに格納する文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
String::String(const char* pszString_)
: Item(Type::String)
{
	m_cstrString = ModUnicodeString(pszString_, ModOs::Process::getEncodingType());
}

//
//	FUNCTION public
//	SydTest::String::~String -- デストラクタ
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
String::~String()
{
}

//
//	FUNCTION public
//	SydTest::String::getUnicodeString -- 文字列を得る
//
//	NOTES
//	格納したUCS2文字列を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		UCS2文字列
//
//	EXCEPTIONS
//	なし
//
const char*
String::getString(ModKanjiCode::KanjiCodeType code)
{
	return m_cstrString.getString(code);
}

//
//	FUNCTION public
//	SydTest::String::getUnicodeString -- 文字列を得る
//
//	NOTES
//	格納したUCS2文字列を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		UCS2文字列
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeString&
String::getUnicodeString() const
{
	return m_cstrString;
}

//
//	FUNCTION public
//	SydTest::String::getLength -- 文字列の長さを得る
//
//	NOTES
//	格納したUCS2文字列の長さを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		UCS2文字列の長さ
//
//	EXCEPTIONS
//	なし
//
const int
String::getLength()
{
	return m_cstrString.getLength();
}

//
//	Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
