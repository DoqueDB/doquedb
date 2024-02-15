// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabasePathElement.cpp --
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
#include "Statement/Literal.h"
#include "Statement/DatabasePathElement.h"
#if 0
#include "Analysis/DatabasePathElement.h"
#endif
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING

namespace
{
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::DatabasePathElement::DatabasePathElement -- コンストラクタ(2)
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
DatabasePathElement::DatabasePathElement(PathType iPathType, Literal* pcPath_)
	: ObjectSelection(ObjectType::DatabasePathElement)
{
	setPathType(iPathType);
	setPathName(pcPath_);
}

//
//	FUNCTION public
//	Statement::DatabasePathElement::~DatabasePathElement -- デストラクタ
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
DatabasePathElement::~DatabasePathElement()
{
}

//
//	FUNCTION public
//	Statement::DatabasePathElement::copy -- 自身をコピーする
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
DatabasePathElement::copy() const
{
	return new DatabasePathElement(*this);
}

//
//	FUNCTION public
//	Statement::DatabasePathElement::toSQLStatement
//		-- SQL文で得る
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
DatabasePathElement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;
	Statement::Literal* p = getPathName();
	
	switch (getPathType())
	{
	case Database:
		s << "path ";
		break;
	case LogicalLog:
		s << "logicallog ";
		break;
	case System:
		s << "system ";
		break;
	}

	if (p)
	{
		s << p->toSQLStatement(bForCascade_);
	}
	else
	{
		s << "(null)";
	}

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::DatabasePathElement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DatabasePathElement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
