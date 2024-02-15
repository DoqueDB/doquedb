// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterTableAction.cpp --
// 
// Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
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
#include "Statement/AlterTableAction.h"
#include "Statement/AreaOption.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/AlterTableAction.h"
#endif

_SYDNEY_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_ActionType,
		f_Action,
		f__end_index
	};

}

using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterTableAction::AlterTableAction -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	ActionType eActType_
//		動作識別子
//
//	Action* pcAreaOpt_
//		エリアオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterTableAction::AlterTableAction(const ActionType eActType_, Object* pcAction_)
	: Object(ObjectType::AlterTableAction, f__end_index)
{
	setActionType(eActType_);
	setAction(pcAction_);
}

//
//	FUNCTION public
//	Statement::AlterTableAction::~AlterTableAction -- デストラクタ
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
AlterTableAction::~AlterTableAction()
{
}

//
//	FUNCTION public
//	Statement::AlterTableAction::getActionType
//		-- 動作識別子を得る
//
//	NOTES
//	動作識別子を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ActionType
//		動作識別子
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AlterTableAction::ActionType
AlterTableAction::getActionType() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_ActionType];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return static_cast<ActionType>(iResult);
}

//
//	FUNCTION public
//	Statement::AlterTableAction::
//		-- 動作識別子を設定する
//
//	NOTES
//	動作識別子を設定する
//
//	ARGUMENTS
//	ActionType eActionType_
//		動作識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterTableAction::setActionType(const ActionType eActionType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ActionType]);
	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
	 	m_vecpElements[f_ActionType] = pIntVal;
	}
	pIntVal->setValue(static_cast<int>(eActionType_));
}

//
//	FUNCTION public
//	Statement::AlterTableAction::
//		-- Action を得る
//
//	NOTES
//	Action を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Object*
//		Action
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Object*
AlterTableAction::getAction() const
{
	return m_vecpElements[f_Action];
}

//
//	FUNCTION public
//	Statement::AlterTableAction::
//		-- Action を得る
//
//	NOTES
//	Action を得る
//
//	ARGUMENTS
//	Object* pcAction_
//		Action
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterTableAction::setAction(Object* pcAction_)
{
	m_vecpElements[f_Action] = pcAction_;
}

//
//	FUNCTION public
//	Statement::AlterTableAction::copy -- 自身をコピーする
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
AlterTableAction::copy() const
{
	return new AlterTableAction(*this);
}

#if 0
namespace
{
	Analysis::AlterTableAction _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterTableAction::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
