// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterCascadeStatement.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/AlterCascadeStatement.h"
#include "Statement/Identifier.h"
#include "Statement/ValueExpression.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Host,
		f_Port,
		f_Database,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::AlterCascadeStatement -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		子サーバー名
//	ValueExpression* pHost_
//		ホスト
//	ValueExpression* pPort_
//		ポート
//	Identifier* pDatabase_
//		子サーバーにおけるDB名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AlterCascadeStatement::AlterCascadeStatement(Identifier* pId_,
											 ValueExpression* pHost_,
											 ValueExpression* pPort_,
											 Identifier* pDatabase_)
	: Object(ObjectType::AlterCascadeStatement, f__end_index, Object::Reorganize)
{
	setCascadeName(pId_);
	setHost(pHost_);
	setPort(pPort_);
	setDatabase(pDatabase_);
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::~AlterCascadeStatement -- デストラクタ
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
AlterCascadeStatement::~AlterCascadeStatement()
{
}

//	FUNCTION public
//	Statement::AlterCascadeStatement::getCascadeName
//		-- 子サーバー名取得
//
//	NOTES
//	子サーバー名取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		子サーバー名
//
//	EXCEPTIONS
//	なし

Identifier*
AlterCascadeStatement::getCascadeName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::setCascadeName
//		-- 子サーバー名設定
//
//	NOTES
//	子サーバー名設定
//
//	ARGUMENTS
//	Identifier* pId_
//		子サーバー名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterCascadeStatement::setCascadeName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

//	FUNCTION public
//	Statement::AlterCascadeStatement::getHost
//		-- Host を得る
//
//	NOTES
//	Host を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ValueExpression*
//		Host
//
//	EXCEPTIONS
//	なし

ValueExpression*
AlterCascadeStatement::getHost() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Host];
	if (pObj && ObjectType::ValueExpression == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::setHost
//		-- Host を設定する
//
//	NOTES
//	Host を設定する
//
//	ARGUMENTS
//	ValueExpression*
//		Host
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterCascadeStatement::setHost(ValueExpression* pHost_)
{
	m_vecpElements[f_Host] = pHost_;
}

//	FUNCTION public
//	Statement::AlterCascadeStatement::getPort
//		-- Port を得る
//
//	NOTES
//	Port を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ValueExpression*
//		Port
//
//	EXCEPTIONS
//	なし

ValueExpression*
AlterCascadeStatement::getPort() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Port];
	if (pObj && ObjectType::ValueExpression == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::setPort
//		-- Port を設定する
//
//	NOTES
//	Port を設定する
//
//	ARGUMENTS
//	ValueExpression*
//		Port
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AlterCascadeStatement::setPort(ValueExpression* pPort_)
{
	m_vecpElements[f_Port] = pPort_;
}

// FUNCTION public
//	Statement::AlterCascadeStatement::getDatabase -- データベース名を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Identifier*
//
// EXCEPTIONS

Identifier*
AlterCascadeStatement::
getDatabase() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Database];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

// FUNCTION public
//	Statement::AlterCascadeStatement::setDatabase -- データベース名を設定する
//
// NOTES
//
// ARGUMENTS
//	Identifier* pDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
AlterCascadeStatement::
setDatabase(Identifier* pDatabase_)
{
	m_vecpElements[f_Database] = pDatabase_;
}

//
//	FUNCTION public
//	Statement::AlterCascadeStatement::copy -- 自身をコピーする
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
AlterCascadeStatement::copy() const
{
	return new AlterCascadeStatement(*this);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
