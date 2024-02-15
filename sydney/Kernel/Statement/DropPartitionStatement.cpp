// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropPartitionStatement.cpp --
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

#include "Statement/DropPartitionStatement.h"
#include "Statement/ColumnNameList.h"
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
//	Statement::DropPartitionStatement::DropPartitionStatement -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		ルール名
//	bool bIfExists_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DropPartitionStatement::DropPartitionStatement(Identifier* pId_, bool bIfExists_)
	: Object(ObjectType::DropPartitionStatement, f__end_index, Object::System),
	  m_bIfExists(bIfExists_)
{
	setPartitionName(pId_);
}

// FUNCTION public
//	Statement::DropPartitionStatement::DropPartitionStatement -- 
//
// NOTES
//
// ARGUMENTS
//	const DropPartitionStatement& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DropPartitionStatement::
DropPartitionStatement(const DropPartitionStatement& cOther_)
	: Object(cOther_),
	  m_bIfExists(cOther_.m_bIfExists)
{}

//
//	FUNCTION public
//	Statement::DropPartitionStatement::~DropPartitionStatement -- デストラクタ
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
DropPartitionStatement::~DropPartitionStatement()
{
}

//	FUNCTION public
//	Statement::DropPartitionStatement::getPartitionName
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
DropPartitionStatement::getPartitionName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if (pObj && ObjectType::Identifier == pObj->getType())
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DropPartitionStatement::setPartitionName
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
DropPartitionStatement::setPartitionName(Identifier* pId_)
{
	m_vecpElements[f_Name] = pId_;
}

// FUNCTION public
//	Statement::DropPartitionStatement::isIfExists -- 
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
DropPartitionStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropPartitionStatement::copy -- 自身をコピーする
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
DropPartitionStatement::copy() const
{
	return new DropPartitionStatement(*this);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
