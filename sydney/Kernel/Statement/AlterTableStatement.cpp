// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterTableStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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
#include "Statement/AlterTableStatement.h"
#include "Statement/Identifier.h"
#include "Statement/AlterTableAction.h"
#if 0
#include "Analysis/AlterTableStatement.h"
#endif

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_TableName,
		f_AlterAction,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterTableStatement::AlterTableStatement(2)
//		-- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pcIdent_
//		操作するテーブル名
//	AlterTableAction* pcAction_
//		動作
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterTableStatement::AlterTableStatement(Identifier* pcIdent_, AlterTableAction* pcAction_)
	: Object(ObjectType::AlterTableStatement, f__end_index, Object::Undefine, true)
{
	setTableName(pcIdent_);
	setAlterTableAction(pcAction_);
}

//
//	FUNCTION public
//	Statement::AlterTableStatement::~AlterTableStatement
//		-- デストラクタ
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
AlterTableStatement::~AlterTableStatement()
{
}

//
//	FUNCTION public
//	AlterTableStatement::
//		-- テーブル名を得る
//
//	NOTES
//	テーブル名を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		対象となるテーブル名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
AlterTableStatement::getTableName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_TableName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}


//
//	FUNCTION public
//	AlterTableStatement::
//		-- テーブル名を設定する
//
//	NOTES
//	テーブル名を設定する
//
//	ARGUMENTS
//	Identifier* pcIdentifier_
//		対象となるテーブル名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterTableStatement::setTableName(Identifier* pcIdentifier_)
{
	m_vecpElements[f_TableName] = pcIdentifier_;
}

//
//	FUNCTION public
//	AlterTableStatement::
//		-- AlterTableAction を得る
//
//	NOTES
//	AlterTableAction を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AlterTableAction*
//		動作クラス
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AlterTableAction*
AlterTableStatement::getAlterTableAction() const
{
	AlterTableAction* pResult = 0;
	Object* pObj = m_vecpElements[f_AlterAction];
	if ( pObj && ObjectType::AlterTableAction == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AlterTableAction*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	AlterTableStatement::
//		-- AlterTableAction を設定する
//
//	NOTES
//	AlterTableAction を設定する
//
//	ARGUMENTS
//	AlterTableAction* pcAction_
//		動作クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterTableStatement::setAlterTableAction(AlterTableAction* pcAction_)
{
	m_vecpElements[f_AlterAction] = pcAction_;
}

//
//	FUNCTION public
//	Statement::AlterTableStatement::copy -- 自身をコピーする
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
AlterTableStatement::copy() const
{
	return new AlterTableStatement(*this);
}

#if 0
namespace
{
	Analysis::AlterTableStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterTableStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
