// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropTableStatement.cpp -- DropTableStatement
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/DropTableStatement.h"
#include "Statement/Type.h"
#include "Statement/Identifier.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#if 0
#include "Analysis/DropTableStatement.h"
#endif

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
//		Statement::DropTableStatement::DropTableStatement -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Identifier* pName_
//		bool bIfExists_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		その他
//		下位の例外を再送
//
DropTableStatement::DropTableStatement( Identifier* pName_, bool bIfExists_ )
	: Object(ObjectType::DropTableStatement, f__end_index, Object::Reorganize),
	  m_bIfExists(bIfExists_)
{
	// If name begins from '#', object type become special type.
	if (pName_->getToken().getLength() > 0
		&& *(pName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::DropTemporaryTableStatement);
	}
	setName(pName_);
}

// FUNCTION public
//	Statement::DropTableStatement::DropTableStatement -- 
//
// NOTES
//
// ARGUMENTS
//	const DropTableStatement& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DropTableStatement::
DropTableStatement(const DropTableStatement& cOther_)
	: Object(cOther_),
	  m_bIfExists(cOther_.m_bIfExists)
{
}

//
//	FUNCTION public
//		Statement::DropTableStatement::getName -- Name を得る
//
//	NOTES
//		Name を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//		その他
//		下位の例外を再送
//
Identifier*
DropTableStatement::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DropTableStatement::setName -- Name を設定する
//
//	NOTES
//		Name を設定する
//
//	ARGUMENTS
//		Identifier* pName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		その他
//		下位の例外を再送
//
void
DropTableStatement::setName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::DropTableStatement::getNameString -- Name を ModString で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS

const ModUnicodeString*
DropTableStatement::getNameString() const
{
	Identifier* pIdentifier = getName();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

// FUNCTION public
//	Statement::DropTableStatement::isIfExists -- If exists?
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
DropTableStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropTableStatement::copy -- 自身をコピーする
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
DropTableStatement::copy() const
{
	return new DropTableStatement(*this);
}

// FUNCTION public
//	Statement::DropTableStatement::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
DropTableStatement::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "drop table " << getName()->toSQLStatement(bForCascade_);
	if (isIfExists()) {
		stream << " if exists";
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::DropTableStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DropTableStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DropTableStatement::serialize -- 
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
DropTableStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bIfExists);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
