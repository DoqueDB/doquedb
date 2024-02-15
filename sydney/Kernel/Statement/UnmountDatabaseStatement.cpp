// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnmountDatabaseStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Statement/Type.h"
#include "Statement/Identifier.h"
#include "Statement/UnmountDatabaseStatement.h"
#if 0
#include "Analysis/UnmountDatabaseStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

using namespace Statement;

namespace {
}

//
//	FUNCTION public
//	Statement::UnmountDatabaseStatement::UnmountDatabaseStatement -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		データベース名
//	AreaOption* pAreaOpt_
//		エリアオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
UnmountDatabaseStatement::UnmountDatabaseStatement(Identifier* pId_)
	: ObjectConnection(ObjectType::UnmountDatabaseStatement, f__end_index, Object::System)
{
	setDatabaseName(pId_);
}

//
//	FUNCTION public
//	Statement::UnmountDatabaseStatement::~UnmountDatabaseStatement -- デストラクタ
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
UnmountDatabaseStatement::~UnmountDatabaseStatement()
{
}

//
//	FUNCTION public
//	Statement::UnmountDatabaseStatement::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
UnmountDatabaseStatement::copy() const
{
	return new UnmountDatabaseStatement(*this);
}

//
//	FUNCTION public
//	Statement::UnmountDatabaseStatement::toSQLStatement -- SQL文で得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
UnmountDatabaseStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "unmount ";
	if (getDatabaseName())
		s << getDatabaseName()->toSQLStatement(bForCascade_);
	else
		s << "(null)";

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::UnmountDatabaseStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
UnmountDatabaseStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
