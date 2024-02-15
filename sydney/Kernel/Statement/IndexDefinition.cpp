// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IndexDefinition.cpp --
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Statement/AreaOption.h"
#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/IndexDefinition.h"
#include "Statement/Type.h"
#if 0
#include "Analysis/IndexDefinition.h"
#endif

#include "Common/UnicodeString.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_TableName,
		f_ColumnNameList,
		f_IndexType,
		f_LanguageColumnName,
		f_ScoreColumnName,
		f_Hint,
		f_AreaOpt,
		f__end_index
	};
}

//	FUNCTION public
//	Statement::IndexDefinition::IndexDefinition -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Identifier* pName_
//		Identifier* pTableName_
//		ColumnNameList* pColumnNameList_
//		Statement::ColumnName*	columnName
//			0 以外の値
//				設定する列の名前を表すクラスを格納する領域の先頭アドレス
//			0
//				言語指定を得るための列の名前が索引定義に指定されなかった
//		int iIndexType_
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

IndexDefinition::IndexDefinition(
	Identifier* pName_, Identifier* pTableName_,
	ColumnNameList* pColumnNameList_, int iIndexType_,
	ColumnName* languageColumnName,
	ColumnName* scoreColumnName,
	Hint* pHint_, AreaOption* pAreaOpt_)
	: Object(ObjectType::IndexDefinition, f__end_index, Object::Reorganize)
{
	// If name begins from '#', object type become special type.
	if (pName_->getToken().getLength() > 0
		&& *(pName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryIndexDefinition);
	}
	// Name を設定する
	setName(pName_);
	// TableName を設定する
	setTableName(pTableName_);
	// ColumnNameList を設定する
	setColumnNameList(pColumnNameList_);
	// IndexType を設定する
	setIndexType(iIndexType_);
	// 言語指定を得るための列を設定する
	setLanguageColumnName(languageColumnName);
	// スコア修正指定を得るための列を設定する
	setScoreColumnName(scoreColumnName);
	// Hint を設定する
	setHint(pHint_);
	// AreaOption を設定する
	setAreaOption(pAreaOpt_);
}

//
//	FUNCTION public
//		Statement::IndexDefinition::getName -- Name を得る
//
//	NOTES
//		Name を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier*
IndexDefinition::getName() const
{
	return _SYDNEY_DYNAMIC_CAST(
		Identifier*, getElement(f_Name, ObjectType::Identifier));
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setName -- Name を設定する
//
//	NOTES
//		Name を設定する
//
//	ARGUMENTS
//		Identifier* pName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setName(Identifier* pName_)
{
	setElement(f_Name, pName_);
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::IndexDefinition::getNameString
//			-- Name を ModUnicodeString で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS

const ModUnicodeString*
IndexDefinition::getNameString() const
{
	Identifier* pIdentifier = getName();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

//
//	FUNCTION public
//		Statement::IndexDefinition::getTableName -- TableName を得る
//
//	NOTES
//		TableName を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier*
IndexDefinition::getTableName() const
{
	return _SYDNEY_DYNAMIC_CAST(
		Identifier*, getElement(f_TableName, ObjectType::Identifier));
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setTableName -- TableName を設定する
//
//	NOTES
//		TableName を設定する
//
//	ARGUMENTS
//		Identifier* pTableName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setTableName(Identifier* pTableName_)
{
	setElement(f_TableName, pTableName_);
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::IndexDefinition::getTableNameString
//			-- TableName を ModUnicodeString で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS

const ModUnicodeString*
IndexDefinition::getTableNameString() const
{
	Identifier* pIdentifier = getTableName();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

//
//	FUNCTION public
//		Statement::IndexDefinition::getColumnNameList -- ColumnNameList を得る
//
//	NOTES
//		ColumnNameList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ColumnNameList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnNameList*
IndexDefinition::getColumnNameList() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ColumnNameList*, getElement(f_ColumnNameList,
									ObjectType::ColumnNameList));
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setColumnNameList -- ColumnNameList を設定する
//
//	NOTES
//		ColumnNameList を設定する
//
//	ARGUMENTS
//		ColumnNameList* pColumnNameList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setColumnNameList(ColumnNameList* pColumnNameList_)
{
	setElement(f_ColumnNameList, pColumnNameList_);
}

//
//	FUNCTION public
//		Statement::IndexDefinition::getIndexType -- IndexType を得る
//
//	NOTES
//		IndexType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
IndexDefinition::getIndexType() const
{
	return getIntegerElement(f_IndexType);
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setIndexType -- IndexType を設定する
//
//	NOTES
//		IndexType を設定する
//
//	ARGUMENTS
//		int iIndexType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setIndexType(int iIndexType_)
{
	setIntegerElement(f_IndexType, iIndexType_);
}

//	FUNCTION public
//	Statement::IndexDefinition::getLanguageColumnName --
//		言語指定を得るための列の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られた列の名前を表すクラスを格納する領域の先頭アドレス
//		0
//			言語指定を得るための列の名前が索引定義に指定されなかった
//
//	EXCEPTIONS
//		なし

ColumnName*
IndexDefinition::getLanguageColumnName() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ColumnName*, getElement(f_LanguageColumnName, ObjectType::ColumnName));
}

//	FUNCTION public
//	Statement::IndexDefinition::setLanguageColumnName --
//		言語指定を得るための列の名前を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ColumnName*	columnName
//			0 以外の値
//				設定する列の名前を表すクラスを格納する領域の先頭アドレス
//			0
//				言語指定を得るための列の名前が索引定義に指定されなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
IndexDefinition::setLanguageColumnName(ColumnName* columnName)
{
	setElement(f_LanguageColumnName, columnName);
}

//	FUNCTION public
//	Statement::IndexDefinition::getScoreColumnName --
//		スコア修正指定を得るための列の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られた列の名前を表すクラスを格納する領域の先頭アドレス
//		0
//			スコア修正指定を得るための列の名前が索引定義に指定されなかった
//
//	EXCEPTIONS
//		なし

ColumnName*
IndexDefinition::getScoreColumnName() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ColumnName*, getElement(f_ScoreColumnName, ObjectType::ColumnName));
}

//	FUNCTION public
//	Statement::IndexDefinition::setScoreColumnName --
//		スコア修正指定を得るための列の名前を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ColumnName*	columnName
//			0 以外の値
//				設定する列の名前を表すクラスを格納する領域の先頭アドレス
//			0
//				スコア修正指定を得るための列の名前が索引定義に指定されなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
IndexDefinition::setScoreColumnName(ColumnName* columnName)
{
	setElement(f_ScoreColumnName, columnName);
}

//
//	FUNCTION public
//		Statement::IndexDefinition::getHint -- Hint を得る
//
//	NOTES
//		Hint を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Hint*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Hint*
IndexDefinition::getHint() const
{
	return _SYDNEY_DYNAMIC_CAST(Hint*, getElement(f_Hint, ObjectType::Hint));
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setHint -- Hint を設定する
//
//	NOTES
//		Hint を設定する
//
//	ARGUMENTS
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setHint(Hint* pHint_)
{
	setElement(f_Hint, pHint_);
}

//
//	FUNCTION public
//		Statement::IndexDefinition::getHint -- AreaOption を得る
//
//	NOTES
//		AreaOption を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		AreaOption*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
AreaOption* 
IndexDefinition::getAreaOption() const
{
	return _SYDNEY_DYNAMIC_CAST(
		AreaOption*, getElement(f_AreaOpt, ObjectType::AreaOption));
}

//
//	FUNCTION public
//		Statement::IndexDefinition::setHint -- AreaOption を設定する
//
//	NOTES
//		AreaOption を設定する
//
//	ARGUMENTS
//		AreaOption* pAreaOpt_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IndexDefinition::setAreaOption(AreaOption* pAreaOpt_)
{
	setElement(f_AreaOpt, pAreaOpt_);
}

//
//	FUNCTION public
//	Statement::IndexDefinition::copy -- 自身をコピーする
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
IndexDefinition::copy() const
{
	return new IndexDefinition(*this);
}

namespace
{
	const char* const _pszTypeName[] =
	{
		"",
		" clustered",
		" nonclustered",
		"",
		" fulltext",
		" bitmap",
		" unique",
		" array",
		" all rows",
		" kdtree",
	};
}

// FUNCTION public
//	Statement::IndexDefinition::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
IndexDefinition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "create" << _pszTypeName[getIndexType()] << " index "
		   << getName()->toSQLStatement(bForCascade_) << " on "
		   << getTableName()->toSQLStatement(bForCascade_) << "("
		   << getColumnNameList()->toSQLStatement(bForCascade_) << ")";
	if (getLanguageColumnName()) {
		stream << " language column " << getLanguageColumnName()->toSQLStatement(bForCascade_);
	}
	if (getScoreColumnName()) {
		stream << " score column " << getScoreColumnName()->toSQLStatement(bForCascade_);
	}
	if (getHint()) {
		stream << " " << getHint()->toSQLStatement(bForCascade_);
	}
	if (getAreaOption()) {
		stream << " " << getAreaOption()->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::IndexDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
IndexDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
