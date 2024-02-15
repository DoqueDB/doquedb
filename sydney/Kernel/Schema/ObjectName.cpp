// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectName.cpp -- オブジェクト名関連の関数定義
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/ObjectName.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

////////////////////////
// Schema::ObjectName //
////////////////////////

//	FUNCTION public
//	Schema::ObjectName::operator== -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const Schema::ObjectName&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致する
//		false..一致しない
//
//	EXCEPTIONS

bool
ObjectName::
operator==(const ObjectName& cName_) const
{
	return equals(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator== -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致する
//		false..一致しない
//
//	EXCEPTIONS

bool
ObjectName::
operator==(const ModUnicodeString& cName_) const
{
	return equals(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator== -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName1_
//			比較するオブジェクト名
//		const Schema::ObjectName&	cName2_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致する
//		false..一致しない
//
//	EXCEPTIONS

bool
operator==(const ModUnicodeString& cName1_, const ObjectName& cName2_)
{
	return ObjectName::equals(cName1_, cName2_);
}

//	FUNCTION public
//	Schema::ObjectName::operator!= -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const Schema::ObjectName&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致しない
//		false..一致する
//
//	EXCEPTIONS

bool
ObjectName::
operator!=(const ObjectName& cName_) const
{
	return !equals(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator!= -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致しない
//		false..一致する
//
//	EXCEPTIONS

bool
ObjectName::
operator!=(const ModUnicodeString& cName_) const
{
	return !equals(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator!= -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName1_
//			比較するオブジェクト名
//		const Schema::ObjectName&	cName2_
//			比較するオブジェクト名
//
//	RETURN
//		true...一致しない
//		false..一致する
//
//	EXCEPTIONS

bool
operator!=(const ModUnicodeString& cName1_, const ObjectName& cName2_)
{
	return !ObjectName::equals(cName1_, cName2_);
}

//	FUNCTION public
//	Schema::ObjectName::operator< -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const Schema::ObjectName&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...thisのほうが小さい
//		false..thisのほうが小さくない
//
//	EXCEPTIONS

bool
ObjectName::
operator<(const ObjectName& cName_) const
{
	return less(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator< -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName_
//			比較するオブジェクト名
//
//	RETURN
//		true...thisのほうが小さい
//		false..thisのほうが小さくない
//
//	EXCEPTIONS

bool
ObjectName::
operator<(const ModUnicodeString& cName_) const
{
	return less(*this, cName_);
}

//	FUNCTION public
//	Schema::ObjectName::operator< -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName1_
//			比較するオブジェクト名
//		const Schema::ObjectName&	cName2_
//			比較するオブジェクト名
//
//	RETURN
//		true...cName1_のほうが小さい
//		false..cName1_のほうが小さくない
//
//	EXCEPTIONS

bool
operator<(const ModUnicodeString& cName1_, const ObjectName& cName2_)
{
	return ObjectName::less(cName1_, cName2_);
}

//	FUNCTION public
//	Schema::ObjectName::equals -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName1_
//		const ModUnicodeString&		cName2_
//			比較する文字列
//
//	RETURN
//		true...一致する
//		false..一致しない
//
//	EXCEPTIONS

//static
bool
ObjectName::
equals(const ModUnicodeString& cName1_, const ModUnicodeString& cName2_)
{
	return cName1_.compare(cName2_, ModFalse) == 0;
}

//	FUNCTION public
//	Schema::ObjectName::less -- オブジェクト名を比較する
//
//	NOTES
//		スキーマオブジェクト名は大文字小文字の違いを無視する
//
//	ARGUMENTS
//		const ModUnicodeString&		cName1_
//		const ModUnicodeString&		cName2_
//			比較する文字列
//
//	RETURN
//		true...cName1_のほうが小さい
//		false..cName1_のほうが小さくない
//
//	EXCEPTIONS

//static
bool
ObjectName::
less(const ModUnicodeString& cName1_, const ModUnicodeString& cName2_)
{
	return cName1_.compare(cName2_, ModFalse) < 0;
}

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
