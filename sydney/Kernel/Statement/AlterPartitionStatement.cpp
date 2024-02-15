// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterPartitionStatement.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Statement/AlterPartitionStatement.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Function,
		f_ColumnList,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::AlterPartitionStatement -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		ルール名
//	Identifier* pFunction_
//		関数
//	ColumnNameList* pColumnList_
//		列リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterPartitionStatement::AlterPartitionStatement(Identifier* pId_,
												 Identifier* pFunction_,
												 ColumnNameList* pColumnList_)
	: Object(ObjectType::AlterPartitionStatement, f__end_index, Object::System)
{
	setPartitionName(pId_);
	setFunction(pFunction_);
	setColumnList(pColumnList_);
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::~AlterPartitionStatement -- デストラクタ
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
AlterPartitionStatement::~AlterPartitionStatement()
{
}

//	FUNCTION public
//	Statement::AlterPartitionStatement::getPartitionName
//		-- ルール名取得
//
//	NOTES
//	ルール名取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		ルール名
//
//	EXCEPTIONS
//	なし

Identifier*
AlterPartitionStatement::getPartitionName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::setPartitionName
//		-- ルール名設定
//
//	NOTES
//	ルール名設定
//
//	ARGUMENTS
//	Identifier* pId_
//		ルール名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterPartitionStatement::setPartitionName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

//	FUNCTION public
//	Statement::AlterPartitionStatement::getFunction
//		-- Function を得る
//
//	NOTES
//	Function を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		Function
//
//	EXCEPTIONS
//	なし

Identifier*
AlterPartitionStatement::getFunction() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Function];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::setFunction
//		-- Function を設定する
//
//	NOTES
//	Function を設定する
//
//	ARGUMENTS
//	Identifier*
//		Function
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterPartitionStatement::setFunction(Identifier* pFunction_)
{
	m_vecpElements[f_Function] = pFunction_;
}

//	FUNCTION public
//	Statement::AlterPartitionStatement::getColumnList
//		-- ColumnList を得る
//
//	NOTES
//	ColumnList を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ColumnNameList*
//		ColumnList
//
//	EXCEPTIONS
//	なし

ColumnNameList*
AlterPartitionStatement::getColumnList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnList];
	if (pObj && ObjectType::ColumnNameList == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::setColumnList
//		-- ColumnList を設定する
//
//	NOTES
//	ColumnList を設定する
//
//	ARGUMENTS
//	ColumnNameList*
//		ColumnList
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterPartitionStatement::setColumnList(ColumnNameList* pColumnList_)
{
	m_vecpElements[f_ColumnList] = pColumnList_;
}

//
//	FUNCTION public
//	Statement::AlterPartitionStatement::copy -- 自身をコピーする
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
AlterPartitionStatement::copy() const
{
	return new AlterPartitionStatement(*this);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
