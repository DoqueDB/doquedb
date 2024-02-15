// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterAreaAction.cpp --
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
#include "Statement/AlterAreaAction.h"
#include "Statement/AreaElementList.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/AlterAreaAction.h"
#endif

namespace
{
	//メンバの m_vecpElements 内での index
	enum
	{
		f_ActionType,
		f_AreaElementList,
		f__end_index
	};
}

_SYDNEY_USING
using namespace Statement;

//
//	FUNCTION public
//	Statement::AlterAreaAction::AlterAreaAction -- コンストラクタ(2)
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
AlterAreaAction::AlterAreaAction(const int iActType_, AreaElementList* pcAreaElem_)
	: Object(ObjectType::AlterAreaAction, f__end_index)
{
	setActionType(iActType_);
	setAreaElementList(pcAreaElem_);
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::~AlterAreaAction -- デストラクタ
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
AlterAreaAction::~AlterAreaAction()
{
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::getActionType
//		-- ActionType を得る
//
//	NOTES
//	ActionType を得る
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
AlterAreaAction::getActionType() const
{
	int iResult = 0;
	Object* pObj  = m_vecpElements[f_ActionType];	
	if (pObj && ObjectType::IntegerValue == pObj->getType()) 
		 iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::setActionType
//
//	NOTES
//	ActionType を設定する
//
//	ARGUMENTS
//	int iActType
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
AlterAreaAction::setActionType(const int iActType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ActionType]);
	if (pIntVal == 0)
	{
		pIntVal = new Statement::IntegerValue;
		m_vecpElements[f_ActionType] = pIntVal;
	}
	pIntVal->setValue(iActType_);
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::getAreaOption
//
//	NOTES
//	AreaOption を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AreaOption*
//		AreaOption
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AreaElementList*
AlterAreaAction::getAreaElementList() const
{
	AreaElementList* pResult = 0;
	Object* pObj = m_vecpElements[f_AreaElementList];
	if ( pObj && ObjectType::AreaElementList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AreaElementList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::setAreaOption
//		-- AreaOption を設定する
//
//	NOTES
//	AreaOption を設定する
//
//	ARGUMENTS
//	AreaOption* pcAreaOpt_
//		AreaOption
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AlterAreaAction::setAreaElementList(AreaElementList* pcAreaOpt_)
{
	m_vecpElements[f_AreaElementList] = pcAreaOpt_;
}

//
//	FUNCTION public
//	Statement::AlterAreaAction::copy -- 自身をコピーする
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
AlterAreaAction::copy() const
{
	return new AlterAreaAction(*this);
}

#if 0
namespace
{
	Analysis::AlterAreaAction _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AlterAreaAction::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
