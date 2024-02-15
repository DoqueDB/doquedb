// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropIndexStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2006, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/DropIndexStatement.h"
#include "Statement/Identifier.h"
#if 0
#include "Analysis/DropIndexStatement.h"
#endif

#include "Common/UnicodeString.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_IndexName,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::DropIndexStatement::DropIndexStatement
//		-- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pcIdxName_
//	bool bIfExists_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DropIndexStatement::DropIndexStatement(Identifier* pcIdxName_, bool bIfExists_)
	: Object(ObjectType::DropIndexStatement, f__end_index, Object::Reorganize),
	  m_bIfExists(bIfExists_)
{
	// If name begins from '#', object type become special type.
	if (pcIdxName_->getToken().getLength() > 0
		&& *(pcIdxName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::DropTemporaryIndexStatement);
	}
	setIndexName(pcIdxName_);
}

// FUNCTION public
//	Statement::DropIndexStatement::DropIndexStatement -- コンストラクタ(2)
//
// NOTES
//
// ARGUMENTS
//	const DropIndexStatement& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DropIndexStatement::
DropIndexStatement(const DropIndexStatement& cOther_)
	: Object(cOther_),
	  m_bIfExists(cOther_.m_bIfExists)
{
}

//
//	FUNCTION public
//	Statement::DropIndexStatement::~DropIndexStatement -- デストラクタ
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
DropIndexStatement::~DropIndexStatement()
{
}

//
//	FUNCTION public
//	Statement::DropIndexStatement::
//		-- Index 名取得
//
//	NOTES
//	Index 名取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		Index 名
//
//	EXCEPTIONS
//	なし
//
Identifier*
DropIndexStatement::getIndexName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_IndexName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DropIndexStatement::
//		-- Index 名設定
//
//	NOTES
//	Index 名設定
//
//	ARGUMENTS
//	Identifier* pcIdxName_
//		Index 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DropIndexStatement::setIndexName(Identifier* pcIdxName_)
{
	m_vecpElements[f_IndexName] = pcIdxName_;
}

// FUNCTION public
//	Statement::DropIndexStatement::isIfExists -- 
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
DropIndexStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropIndexStatement::copy -- 自身をコピーする
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
DropIndexStatement::copy() const
{
	return new DropIndexStatement(*this);
}

// FUNCTION public
//	Statement::DropIndexStatement::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	bool bForCascade_ /* = false */
//	
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
DropIndexStatement::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "drop index " << getIndexName()->toSQLStatement(bForCascade_);
	if (isIfExists()) {
		stream << " if exists";
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::DropIndexStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DropIndexStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DropIndexStatement::serialize -- 
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
DropIndexStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bIfExists);
}

//
//	Copyright (c) 2000, 2002, 2006, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
