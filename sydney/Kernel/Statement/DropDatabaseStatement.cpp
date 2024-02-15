// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropDatabaseStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/DropDatabaseStatement.h"
#include "Statement/Identifier.h"
#if 0
#include "Analysis/DropDatabaseStatement.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_DatabaseName,
		f__end_index
	};

}

using namespace Statement;

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::DropDatabaseStatement
//		-- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pcDbName_
//	bool bIfExists_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DropDatabaseStatement::DropDatabaseStatement(Identifier* pcDbName_, bool bIfExists_)
	: Object(ObjectType::DropDatabaseStatement, f__end_index, Object::System),
	  m_bIfExists(bIfExists_)
{
	setDbName(pcDbName_);
}

// FUNCTION public
//	Statement::DropDatabaseStatement::DropDatabaseStatement -- コンストラクタ(2)
//
// NOTES
//
// ARGUMENTS
//	const DropDatabaseStatement& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DropDatabaseStatement::
DropDatabaseStatement(const DropDatabaseStatement& cOther_)
	: Object(cOther_),
	  m_bIfExists(cOther_.m_bIfExists)
{
}

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::~DropDatabaseStatement -- デストラクタ
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
DropDatabaseStatement::~DropDatabaseStatement()
{
}

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::getDbName
//
//	NOTES
//	Database 名を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		Database 名
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
DropDatabaseStatement::getDbName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_DatabaseName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::
//
//	NOTES
//	Database 名を設定する
//
//	ARGUMENTS
//	Identifier* pcDbName_
//		Database 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
DropDatabaseStatement::setDbName(Identifier* pcDbName_)
{
	m_vecpElements[f_DatabaseName] = pcDbName_;

	return;
}

// FUNCTION public
//	Statement::DropDatabaseStatement::isIfExists -- 
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
DropDatabaseStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::copy -- 自身をコピーする
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
DropDatabaseStatement::copy() const
{
	return new DropDatabaseStatement(*this);
}

//
//	FUNCTION public
//	Statement::DropDatabaseStatement::toSQLStatement -- SQL文で得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列表記
//
//	EXCEPTIONS
//
ModUnicodeString
DropDatabaseStatement::toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream s;

	s << "drop database ";
	if (getDbName())
		s << getDbName()->toSQLStatement(bForCascade_);
	else
		s << "(null)";
	if (m_bIfExists)
		s << " if exists";

	return ModUnicodeString(s.getString());
}

#if 0
namespace
{
	Analysis::DropDatabaseStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DropDatabaseStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DropDatabaseStatement::serialize -- 
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
DropDatabaseStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bIfExists);
}

//
//	Copyright (c) 2000, 2002, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
