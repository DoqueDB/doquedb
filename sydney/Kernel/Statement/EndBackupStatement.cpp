// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// EndBackupStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2009, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/EndBackupStatement.h"
#if 0
#include "Analysis/EndBackupStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

using namespace Statement;

namespace {
}

//
//	FUNCTION public
//	Statement::EndBackupStatement::EndBackupStatement -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。
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
EndBackupStatement::EndBackupStatement(int iLogicalLogOption_)
	: ObjectConnection(ObjectType::EndBackupStatement, f__end_index, Object::System)
{
	setLogicalLogOption(iLogicalLogOption_);
}

//
//	FUNCTION public
//	Statement::EndBackupStatement::~EndBackupStatement -- デストラクタ
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
EndBackupStatement::~EndBackupStatement()
{
}

//
//	FUNCTION public
//	Statement::EndBackupStatement::copy -- 自身をコピーする
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
EndBackupStatement::copy() const
{
	return new EndBackupStatement(*this);
}

//
//	FUNCTION public
//	Statement::EndBackupStatement::toSQLStatement -- SQL文で得る
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
EndBackupStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "end backup";

	if (getLogicalLogOption()  == LogicalLogOption::Discard)
	{
		s << " with discard logicallog";
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::EndBackupStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
EndBackupStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2009, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
