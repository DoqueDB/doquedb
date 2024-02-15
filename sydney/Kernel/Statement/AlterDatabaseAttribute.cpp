// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterDatabaseAttribute.cpp --
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
#include "Statement/IntegerValue.h"
#include "Statement/AlterDatabaseAttribute.h"
#include "Statement/DatabaseCreateOption.h"
#if 0
#include "Analysis/AlterDatabaseAttribute.h"
#endif

_SYDNEY_USING

namespace
{
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterDatabaseAttribute::AlterDatabaseAttribute -- コンストラクタ(2)
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
AlterDatabaseAttribute::AlterDatabaseAttribute(AttributeType iAttributeType_, int iAttribute_)
	: ObjectSelection(ObjectType::AlterDatabaseAttribute)
{
	setAttributeType(iAttributeType_);
	setAttribute(iAttribute_);
}

//
//	FUNCTION public
//	Statement::AlterDatabaseAttribute::~AlterDatabaseAttribute -- デストラクタ
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
AlterDatabaseAttribute::~AlterDatabaseAttribute()
{
}

//
//	FUNCTION public
//	Statement::AlterDatabaseAttribute::copy -- 自身をコピーする
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
AlterDatabaseAttribute::copy() const
{
	return new AlterDatabaseAttribute(*this);
}

//
//	FUNCTION public
//	Statement::AlterDatabaseAttribute::toSQLStatement
//		-- SQL文で値を得る
//
//	NOTES
//
//	ARGUMNENTS
//	bool bForCascade_ = false
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
AlterDatabaseAttribute::toSQLStatement(bool bForCascade_ /* = false */) const
{
	DatabaseCreateOption::OptionType type = DatabaseCreateOption::Unknown;
	
	switch (getAttributeType())
	{
	case ReadWrite:
		type = DatabaseCreateOption::ReadWriteOption;
		break;
	case Online:
		type = DatabaseCreateOption::OnlineOption;
		break;
	case RecoveryFull:
		type = DatabaseCreateOption::RecoveryOption;
		break;
	case SuperUserMode:
		type = DatabaseCreateOption::UserModeOption;
		break;
	}

	return DatabaseCreateOption::getOptionName(type, getAttribute());
}

#if 0
namespace
{
	Analysis::AlterDatabaseAttribute _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterDatabaseAttribute::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
