// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterIndexStatement.cpp --
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
#include "Statement/AlterIndexStatement.h"
#include "Statement/Identifier.h"
#include "Statement/AlterIndexAction.h"
#if 0
#include "Analysis/AlterIndexStatement.h"
#endif

_SYDNEY_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum {
		f_IndexName,
		f_AlterAction,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterIndexStatement::AlterIndexStatement -- コンストラクタ(2)
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
AlterIndexStatement::AlterIndexStatement(Identifier* pcIdent_,
										 AlterIndexAction* pcAction_)
	: Object(ObjectType::AlterIndexStatement, f__end_index, Object::Reorganize, true)
{
	setIndexName(pcIdent_);
	setAlterIndexAction(pcAction_);
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::~AlterIndexStatement -- デストラクタ
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
AlterIndexStatement::~AlterIndexStatement()
{
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::getAreaName
//		-- Index 名を取得する
//
//	NOTES
//	Index 名を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		Index 名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
AlterIndexStatement::getIndexName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_IndexName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::setAreaName
//		-- Index 名を設定する
//
//	NOTES
//	Index 名を設定する
//
//	ARGUMENTS
//	Ideintifier* pcIdent_
//		Index 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterIndexStatement::setIndexName(Identifier* pcIdent_)
{
	m_vecpElements[f_IndexName] = pcIdent_;
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::getAlterIndexAction
//		AlterIndexAction を取得する
//
//	NOTES
//	AlterIndexAction を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AlterIndexAction*
//		AlterIndexAction
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AlterIndexAction*
AlterIndexStatement::getAlterIndexAction() const
{
	AlterIndexAction* pResult = 0;
	Object* pObj = m_vecpElements[f_AlterAction];
	if ( pObj && ObjectType::AlterIndexAction == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AlterIndexAction*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::setAlterIndexAction
//		-- AlterIndexAction を設定する
//
//	NOTES
//	AlterIndexAction を設定する
//
//	ARGUMENTS
//	AlterIndexAction* pcAction_
//		AlterIndexAction
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterIndexStatement::setAlterIndexAction(AlterIndexAction* pcAction_)
{
	m_vecpElements[f_AlterAction] = pcAction_;
}

//
//	FUNCTION public
//	Statement::AlterIndexStatement::copy -- 自身をコピーする
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
AlterIndexStatement::copy() const
{
	return new AlterIndexStatement(*this);
}

#if 0
namespace
{
	Analysis::AlterIndexStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterIndexStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
