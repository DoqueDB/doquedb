// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropFunctionStatement.cpp --
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

#include "Statement/DropFunctionStatement.h"
#include "Statement/Identifier.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::DropFunctionStatement::DropFunctionStatement -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		関数名
//	bool bIfExists_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DropFunctionStatement::DropFunctionStatement(Identifier* pId_,
											 bool bIfExists_)
	: Object(ObjectType::DropFunctionStatement, f__end_index, Object::Reorganize),
	  m_bIfExists(bIfExists_)
{
	setFunctionName(pId_);
}

//
//	FUNCTION public
//	Statement::DropFunctionStatement::~DropFunctionStatement -- デストラクタ
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
DropFunctionStatement::~DropFunctionStatement()
{
}

//	FUNCTION public
//	Statement::DropFunctionStatement::getFunctionName
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
DropFunctionStatement::getFunctionName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DropFunctionStatement::setFunctionName
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
DropFunctionStatement::setFunctionName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

// FUNCTION public
//	Statement::DropFunctionStatement::isIfExists -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DropFunctionStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropFunctionStatement::copy -- 自身をコピーする
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
DropFunctionStatement::copy() const
{
	return new DropFunctionStatement(*this);
}

// FUNCTION public
//	Statement::DropFunctionStatement::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DropFunctionStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bIfExists);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
