// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Type.cpp -- クラスIDと文字列との変換
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Statement/Object.h"
#include "Statement/Type.h"

_SYDNEY_USING
using namespace Statement;

namespace {
    //
    //	CONST
    //		cstrName --
    //
    //	NOTES
    //		Statementのタイプに対応する文字列を保持する
    //
    const ModString cstrName[ObjectType::TotalNumber] =
    {
#define TypeDef(name)	ModString(#name),
		TypeDef(Object)
#include "Statement/TypeList.h"
#undef TypeDef
    };

	const ModString cstrUnknown("(Object)");
}

//
//	FUNCTION global
//	Statement::getTypeName -- typeIDからクラス名を得る
//
//	NOTES
//	型IDから文字列へのマッピング
//
//	ARGUMENTS
//	int iType_
//		タイプ番号
//
//	RETURN
//	ModString
//		文字列
//
//	EXCEPTIONS
//		なし
//
const ModString&
Statement::getTypeName(int iType_)
{
    if (iType_ < 0 || iType_ >= ObjectType::TotalNumber)
		return cstrUnknown;
    return cstrName[iType_];
}

#ifndef SYD_COVERAGE

//
//	FUNCTION global
//	Statement::getTypeIDFromName -- クラス名からtypeIDを得る
//
//	NOTES
//	文字列から型IDへのマッピング
//
//	ARGUMENTS
//	const ModString& cstrString_
//		文字列
//
//	RETURN
//	int 
//		タイプ番号
//		見つからなければ-1を返す。
//
//	EXCEPTIONS
//		なし
//
int
Statement::getTypeIDFromName(const ModString& cstrString_)
{
	for (int i = 0; i < ObjectType::TotalNumber; i++) {
		if (cstrName[i] == cstrString_)
			return i;
	}
    return -1;
}

//
//	FUNCTION global
//	Statement::getTypeIDFromName -- クラス名からtypeIDを得る
//
//	NOTES
//	文字列から型IDへのマッピング
//
//	ARGUMENTS
//	const char* pszString_
//		文字列
//
//	RETURN
//	int 
//		タイプ番号
//		見つからなければ-1を返す。
//
//	EXCEPTIONS
//		なし
//
int
Statement::getTypeIDFromName(const char* pszString_)
{
	ModString cstrString(pszString_);
	return getTypeIDFromName(cstrString);
}

#endif

//
//	Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
