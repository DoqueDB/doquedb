// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MoveDatabaseStatement.cpp --
// 
// Copyright (c) 2000, 2001, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/Literal.h"
#include "Statement/DatabasePathElement.h"
#include "Statement/DatabasePathElementList.h"
#include "Statement/MoveDatabaseStatement.h"
#if 0
#include "Analysis/MoveDatabaseStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

using namespace Statement;

namespace {
}

//
//	FUNCTION public
//	Statement::MoveDatabaseStatement::MoveDatabaseStatement -- コンストラクタ(2)
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
MoveDatabaseStatement::MoveDatabaseStatement(Identifier* pId_
									   ,DatabasePathElementList* pPath_
									   ,int iType_)
	: Statement::ObjectConnection(Statement::ObjectType::MoveDatabaseStatement
	                             ,f__end_index
								 ,Statement::Object::System
								 ,true)
{
	setDatabaseName(pId_);
	setPathList(pPath_);
	setType(iType_);
}

//
//	FUNCTION public
//	Statement::MoveDatabaseStatement::~MoveDatabaseStatement -- デストラクタ
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
MoveDatabaseStatement::~MoveDatabaseStatement()
{
}

//
//	FUNCTION public
//	Statement::MoveDatabaseStatement::copy -- 自身をコピーする
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
MoveDatabaseStatement::copy() const
{
	return new MoveDatabaseStatement(*this);
}

//
//	FUNCTION public
//	Statement::MoveDatabaseStatement::toSQLStatement
//		-- SQL文で値を得る
//
//	NOTES
//
//	ARGUMNENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
MoveDatabaseStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "alter database ";
	if (getDatabaseName())
		s << getDatabaseName()->toSQLStatement(bForCascade_);
	else
		s << "(null)";
	if (getType() == Set)
		s << " set ";
	else
		s << " drop ";
	if (getPathList())
		s << getPathList()->toSQLStatement(bForCascade_);
	else
		s << "(null)";

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::MoveDatabaseStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
MoveDatabaseStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2001, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
