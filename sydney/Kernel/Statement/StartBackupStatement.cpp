// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StartBackupStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
#include "Statement/StartBackupStatement.h"
#if 0
#include "Analysis/StartBackupStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

using namespace Statement;

namespace {
}

//
//	FUNCTION public
//	Statement::StartBackupStatement::StartBackupStatement -- コンストラクタ(2)
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
StartBackupStatement::StartBackupStatement(int iType_, int iVersion_)
	: ObjectConnection(ObjectType::StartBackupStatement, f__end_index, Object::System, true)
{
	//【注意】	版を使った読取専用トランザクションでのバックアップは
	//			仕様からなくした
	// 			そのため、常に DISCARD SNAPSHOT となる
	
	iVersion_ = (iVersion_ == UnknownVersion) ? DiscardSnapshot : iVersion_;
	
	setType(iType_);
    setVersion(iVersion_);
}

//
//	FUNCTION public
//	Statement::StartBackupStatement::~StartBackupStatement -- デストラクタ
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
StartBackupStatement::~StartBackupStatement()
{
}

//
//	FUNCTION public
//	Statement::StartBackupStatement::copy -- 自身をコピーする
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
StartBackupStatement::copy() const
{
	return new StartBackupStatement(*this);
}

//
//	FUNCTION public
//	Statement::StartBackupStatement::toSQLStatement -- SQL文で得る
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
StartBackupStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "start backup";
	
	switch (getType())
	{
	case Full:
		s << " full";
		break;
	case Master:
		s << " master";
		break;
	case LogicalLog:
		s << " logicallog";
	}

	switch (getVersion())
	{
	case DiscardSnapshot:
		s << " discard snapshot";
		break;
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::StartBackupStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
StartBackupStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
