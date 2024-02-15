// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FunctionDefinition.cpp --
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

#include "Statement/FunctionDefinition.h"
#include "Statement/Identifier.h"
#include "Statement/ParameterDeclarationList.h"
#include "Statement/ReturnsClause.h"
#include "Statement/RoutineBody.h"

#include "Analysis/Procedure/Function.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Param,
		f_Returns,
		f_Routine,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::FunctionDefinition -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		関数名
//	ParameterDeclarationList* pParam_
//		パラメーターリスト
//	ReturnsClause* pReturns_
//		Returns
//	RoutineBody* pRoutine_
//		RoutineBody
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FunctionDefinition::FunctionDefinition(Identifier* pId_,
									   ParameterDeclarationList* pParam_,
									   ReturnsClause* pReturns_,
									   RoutineBody* pRoutine_)
	: Object(ObjectType::FunctionDefinition, f__end_index, Object::Reorganize)
{
	setFunctionName(pId_);
	setParam(pParam_);
	setReturns(pReturns_);
	setRoutine(pRoutine_);
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::~FunctionDefinition -- デストラクタ
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
FunctionDefinition::~FunctionDefinition()
{
}

//	FUNCTION public
//	Statement::FunctionDefinition::getFunctionName
//		-- 関数名取得
//
//	NOTES
//	関数名取得
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
FunctionDefinition::getFunctionName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::setFunctionName
//		-- 関数名設定
//
//	NOTES
//	関数名設定
//
//	ARGUMENTS
//	Identifier* pId_
//		関数名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FunctionDefinition::setFunctionName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

//	FUNCTION public
//	Statement::FunctionDefinition::getParam
//		-- Param を得る
//
//	NOTES
//	Param を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ParameterDeclarationList*
//		Param
//
//	EXCEPTIONS
//	なし

ParameterDeclarationList*
FunctionDefinition::getParam() const
{
	ParameterDeclarationList* pResult = 0;
	Object* pObj = m_vecpElements[f_Param];
	if (pObj && ObjectType::ParameterDeclarationList == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ParameterDeclarationList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::setParam
//		-- Param を設定する
//
//	NOTES
//	Param を設定する
//
//	ARGUMENTS
//	ParameterDeclarationList*
//		Param
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FunctionDefinition::setParam(ParameterDeclarationList* pParam_)
{
	m_vecpElements[f_Param] = pParam_;
}

//	FUNCTION public
//	Statement::FunctionDefinition::getReturns
//		-- Returns を得る
//
//	NOTES
//	Returns を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ReturnsClause*
//		Returns
//
//	EXCEPTIONS
//	なし

ReturnsClause*
FunctionDefinition::getReturns() const
{
	ReturnsClause* pResult = 0;
	Object* pObj = m_vecpElements[f_Returns];
	if (pObj && ObjectType::ReturnsClause == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ReturnsClause*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::setReturns
//		-- Returns を設定する
//
//	NOTES
//	Returns を設定する
//
//	ARGUMENTS
//	ReturnsClause*
//		Returns
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FunctionDefinition::setReturns(ReturnsClause* pReturns_)
{
	m_vecpElements[f_Returns] = pReturns_;
}

//	FUNCTION public
//	Statement::FunctionDefinition::getRoutine
//		-- Routine を得る
//
//	NOTES
//	Routine を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	RoutineBody*
//		Routine
//
//	EXCEPTIONS
//	なし

RoutineBody*
FunctionDefinition::getRoutine() const
{
	RoutineBody* pResult = 0;
	Object* pObj = m_vecpElements[f_Routine];
	if (pObj && ObjectType::RoutineBody == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(RoutineBody*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::setRoutine
//		-- Routine を設定する
//
//	NOTES
//	Routine を設定する
//
//	ARGUMENTS
//	RoutineBody*
//		Routine
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FunctionDefinition::setRoutine(RoutineBody* pRoutine_)
{
	m_vecpElements[f_Routine] = pRoutine_;
}

//
//	FUNCTION public
//	Statement::FunctionDefinition::copy -- 自身をコピーする
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
FunctionDefinition::copy() const
{
	return new FunctionDefinition(*this);
}

// FUNCTION public
//	Statement::FunctionDefinition::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
FunctionDefinition::
getAnalyzer2() const
{
	return Analysis::Procedure::Function::create(this);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
