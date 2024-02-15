// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterIndexAction.cpp --
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

#include "Common/Assert.h"

#include "Statement/Type.h"
#include "Statement/AlterIndexAction.h"
#include "Statement/AlterIndexStatement.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/AreaOption.h"
#if 0
#include "Analysis/AlterIndexAction.h"
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
//	Statement::AlterIndexAction::AlterIndexAction -- コンストラクタ(2)
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
AlterIndexAction::AlterIndexAction(const int iActType_, Object* pcAction_)
	: Object(ObjectType::AlterIndexAction, f__end_index)
{
	setActionType(iActType_);
	setAction(pcAction_);
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::~AlterIndexAction -- デストラクタ
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
AlterIndexAction::~AlterIndexAction()
{
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::getActionType
//		-- ActionType を取得する
//
//	NOTES
//	ActionType を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ActionType
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
int
AlterIndexAction::getActionType() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_ActionType];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::setActionType
//		-- ActionType を設定する
//
//	NOTES
//	ActionType を設定する
//
//	ARGUMENTS
//	int iActType_
//		ActionType
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterIndexAction::setActionType(const int iActType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ActionType]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_ActionType] = pIntVal;
	}

	pIntVal->setValue(iActType_);
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::getAction
//		-- Action を取得する
//
//	NOTES
//	AreaOperation を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Action*
//		AreaOperation
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Object*
AlterIndexAction::getAction() const
{
	return m_vecpElements[f_Action];
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::setAction
//		-- Action を設定する
//
//	NOTES
//	Action を設定する
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
AlterIndexAction::setAction(Object* pcAction_)
{
	m_vecpElements[f_Action] = pcAction_;
}

//
//	FUNCTION public
//	Statement::AlterIndexAction::copy -- 自身をコピーする
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
AlterIndexAction::copy() const
{
	return new AlterIndexAction(*this);
}

#if 0
namespace
{
	Analysis::AlterIndexAction _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterIndexAction::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
