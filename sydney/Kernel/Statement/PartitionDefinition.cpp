// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PartitionDefinition.cpp --
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

#include "Statement/PartitionDefinition.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Function,
		f_ColumnList,
		f_Category,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::PartitionDefinition -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		子サーバー名
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
PartitionDefinition::PartitionDefinition(Identifier* pId_,
										 Identifier* pFunction_,
										 ColumnNameList* pColumnList_,
										 Category::Value eCategory_)
	: Object(ObjectType::PartitionDefinition, f__end_index, Object::Reorganize)
{
	setTableName(pId_);
	setFunction(pFunction_);
	setColumnList(pColumnList_);
	setCategory(eCategory_);
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::~PartitionDefinition -- デストラクタ
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
PartitionDefinition::~PartitionDefinition()
{
}

//	FUNCTION public
//	Statement::PartitionDefinition::getTableName
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
PartitionDefinition::getTableName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::setTableName
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
PartitionDefinition::setTableName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

//	FUNCTION public
//	Statement::PartitionDefinition::getFunction
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
//		関数名
//
//	EXCEPTIONS
//	なし

Identifier*
PartitionDefinition::getFunction() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Function];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::setFunction
//		-- Function を設定する
//
//	NOTES
//	Function を設定する
//
//	ARGUMENTS
//	Identifier*
//		関数名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
PartitionDefinition::setFunction(Identifier* pFunction_)
{
	m_vecpElements[f_Function] = pFunction_;
}

//	FUNCTION public
//	Statement::PartitionDefinition::getColumnList
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
//		列リスト
//
//	EXCEPTIONS
//	なし

ColumnNameList*
PartitionDefinition::getColumnList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnList];
	if (pObj && ObjectType::ColumnNameList == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::setColumnList
//		-- ColumnList を設定する
//
//	NOTES
//	ColumnList を設定する
//
//	ARGUMENTS
//	ColumnNameList*
//		列リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
PartitionDefinition::setColumnList(ColumnNameList* pColumnList_)
{
	m_vecpElements[f_ColumnList] = pColumnList_;
}

// FUNCTION public
//	Statement::PartitionDefinition::getCategory -- カテゴリーを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	PartitionDefinition::Category::Value
//
// EXCEPTIONS

PartitionDefinition::Category::Value
PartitionDefinition::
getCategory() const
{
	IntegerValue* pResult = 0;
	Object* pObj = m_vecpElements[f_Category];
	if (pObj && ObjectType::IntegerValue == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj);
	if (pResult) {
		return static_cast<Category::Value>(pResult->getValue());
	}
	return Category::Normal;
}

// FUNCTION public
//	Statement::PartitionDefinition::setCategory -- カテゴリーを設定する
//
// NOTES
//
// ARGUMENTS
//	Category::Value eCategory_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
PartitionDefinition::
setCategory(Category::Value eCategory_)
{
	m_vecpElements[f_Category] = new IntegerValue(static_cast<int>(eCategory_));
}

//
//	FUNCTION public
//	Statement::PartitionDefinition::copy -- 自身をコピーする
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
PartitionDefinition::copy() const
{
	return new PartitionDefinition(*this);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
