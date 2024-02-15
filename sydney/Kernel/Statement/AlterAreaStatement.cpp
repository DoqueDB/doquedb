// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterAreaStatement.cpp --
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
#include "Statement/AlterAreaStatement.h"
#include "Statement/AlterAreaAction.h"
#include "Statement/Identifier.h"
#if 0
#include "Analysis/AlterAreaStatement.h"
#endif

_SYDNEY_USING

namespace
{
	//メンバーの m_vecpElements 内での index
	enum
	{
		f_AreaName,
		f_AlterAction,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterAreaStatement::AlterAreaStatement -- コンストラクタ
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
AlterAreaStatement::AlterAreaStatement(Identifier* pcIdent_,
									   AlterAreaAction* pcAction_)
	: Object(ObjectType::AlterAreaStatement, f__end_index, Object::Reorganize, true)
{
	setAreaName(pcIdent_);
	setAlterAreaAction(pcAction_);
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::~AlterAreaStatement -- デストラクタ
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
AlterAreaStatement::~AlterAreaStatement()
{
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::getAreaName
//		-- エリア名を得る
//
//	NOTES
//	エリア名を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		エリア名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
AlterAreaStatement::getAreaName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_AreaName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::setAreaName
//		-- エリア名を設定する
//
//	NOTES
//	エリア名を設定する
//
//	ARGUMENTS
//	Identifier* pcIdent_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterAreaStatement::setAreaName(Identifier* pcIdent_)
{
	m_vecpElements[f_AreaName] = pcIdent_;
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::getAlterAreaAction
//		-- AlterAreaAction を得る
//
//	NOTES
//	AlterAreaAction を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AlterAreaAction*
//		AlterAreaAction
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
//AlterAreaAction アクセサ
AlterAreaAction*
AlterAreaStatement::getAlterAreaAction() const
{
	AlterAreaAction* pResult = 0;
	Object* pObj = m_vecpElements[f_AlterAction];
	if ( pObj && ObjectType::AlterAreaAction == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AlterAreaAction*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::setAlterAreaAction
//		-- setAlterAreaAction を設定する
//
//	NOTES
//	setAlterAreaAction を設定する
//
//	ARGUMENTS
//	AlterAreaAction* pcAction_
//		AlterAreaAction
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void AlterAreaStatement::setAlterAreaAction(AlterAreaAction* pcAction_)
{
	m_vecpElements[f_AlterAction] = pcAction_;
}

//
//	FUNCTION public
//	Statement::AlterAreaStatement::copy -- 自身をコピーする
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
AlterAreaStatement::copy() const
{
	return new AlterAreaStatement(*this);
}

#if 0
namespace
{
	Analysis::AlterAreaStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterAreaStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
