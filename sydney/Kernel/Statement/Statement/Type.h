// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Type.h -- 構文要素の種類
// 
// Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TYPE_H
#define __SYDNEY_STATEMENT_TYPE_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "ModString.h"

_SYDNEY_BEGIN

namespace Statement
{

namespace ObjectType
{
//
//	ENUM
//	ObjectType::Type -- 構文要素の種類
//
//	NOTES
//	データ型をあらわす列挙子
//
enum Type
{
	Object		= 0,
#define TypeDef(name)	name,
#include "Statement/TypeList.h"
#undef TypeDef

	TotalNumber					// 最大値
};

} // namespace ObjectType

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
const ModString& getTypeName(int iType_);

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
int getTypeIDFromName(const ModString& cstrString_);

//
//	FUNCTION global
//	Statement::getTypeIDFromName -- クラス名からtypeIDを得る
//
//	NOTES
//	文字列から型IDへのマッピング
//
//	ARGUMENTS
//	char* pszString_
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
int getTypeIDFromName(const char* pszString_);
#endif
} // namespace Statement

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_TYPE_H

//
//	Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
