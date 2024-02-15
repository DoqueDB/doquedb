// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseDefinition.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/Literal.h"
#include "Statement/DatabasePathElement.h"
#include "Statement/DatabasePathElementList.h"
#include "Statement/DatabaseDefinition.h"
#include "Statement/Identifier.h"
#include "Statement/Hint.h"
#include "Statement/DatabaseCreateOption.h"
#include "Statement/DatabaseCreateOptionList.h"
#if 0
#include "Analysis/DatabaseDefinition.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Hint,
		f_OptionList,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::DatabaseDefinition -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		データベース名
//	DatabaseCreateOptionList* pOption_
//		エリアオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DatabaseDefinition::DatabaseDefinition(Identifier* pId_,
									   Hint* pHint_,
									   DatabaseCreateOptionList* pOption_)
	: Object(ObjectType::DatabaseDefinition, f__end_index, Object::System)
{
	setDatabaseName(pId_);
	setHint(pHint_);
	setOptionList(pOption_);
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::~DatabaseDefinition -- デストラクタ
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
DatabaseDefinition::~DatabaseDefinition()
{
}

//	FUNCTION public
//	Statement::DatabaseDefinition::getDatabaseName
//		-- データベース名取得
//
//	NOTES
//	データベース名取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		データベース名
//
//	EXCEPTIONS
//	なし

Identifier*
DatabaseDefinition::getDatabaseName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::setDatabaseName
//		-- データベース名設定
//
//	NOTES
//	データベース名設定
//
//	ARGUMENTS
//	Identifier* pId_
//		データベース名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DatabaseDefinition::setDatabaseName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

//	FUNCTION public
//	Statement::DatabaseDefinition::getHint
//		-- Hint を得る
//
//	NOTES
//	Hint を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Hint*
//		ヒント
//
//	EXCEPTIONS
//	なし

Hint*
DatabaseDefinition::getHint() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_Hint];
	if (pObj && ObjectType::Hint == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::setHint
//		-- Hint を設定する
//
//	NOTES
//	Hint を設定する
//
//	ARGUMENTS
//	Hint*
//		ヒント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DatabaseDefinition::setHint(Hint* pHint_)
{
	m_vecpElements[f_Hint] = pHint_;
}

//	FUNCTION public
//	Statement::DatabaseDefinition::getOptionList
//		-- オプションリスト取得
//
//	NOTES
//	オプションリスト取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	DatabaseCreateOptionList*
//		オプションリスト
//
//	EXCEPTIONS
//	なし

DatabaseCreateOptionList*
DatabaseDefinition::getOptionList() const
{
	DatabaseCreateOptionList* pResult = 0;
	Object* pObj = m_vecpElements[f_OptionList];
	if (pObj && ObjectType::DatabaseCreateOptionList == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(DatabaseCreateOptionList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::setOptionList
//		-- オプションリスト設定
//
//	NOTES
//	オプションリスト設定
//
//	ARGUMENTS
//	DatabaseCreateOptionList* pOption_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DatabaseDefinition::setOptionList(DatabaseCreateOptionList* pOption_)
{
	m_vecpElements[f_OptionList] = pOption_;
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::copy -- 自身をコピーする
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
DatabaseDefinition::copy() const
{
	return new DatabaseDefinition(*this);
}

//
//	FUNCTION public
//	Statement::DatabaseDefinition::toSQLStatement -- SQL文で得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
///	RERTURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
DatabaseDefinition::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "create database ";
	if (getDatabaseName())
		s << getDatabaseName()->toSQLStatement(bForCascade_);
	else
		s << "(null)";
	if (getHint())
		s << " " << getHint()->toSQLStatement(bForCascade_);
	if (getOptionList())
		s << " " << getOptionList()->toSQLStatement(bForCascade_);

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::DatabaseDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DatabaseDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
