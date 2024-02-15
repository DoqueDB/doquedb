// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnName.cpp -- ColumnName
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ColumnName.h"
#include "Statement/Type.h"
#include "Statement/Identifier.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#if 0
#include "Analysis/ColumnName.h"
#endif
#include "Analysis/Value/ColumnName.h"


_SYDNEY_USING

using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Identifier,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ColumnName::ColumnName -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pIdentifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnName::ColumnName(Identifier* pIdentifier_)
	: Object(ObjectType::ColumnName, f__end_index)
{
	// Identifier を設定する
	setIdentifier(pIdentifier_);
}

//
//	FUNCTION public
//		Statement::ColumnName::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
ColumnName::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	Identifier* pIdentifier = getIdentifier();
	return pIdentifier ? getIdentifier()->toSQLStatement(bForCascade_) : Object::toSQLStatement(bForCascade_);
}

//
//	FUNCTION public
//		Statement::ColumnName::getIdentifier -- Identifier を得る
//
//	NOTES
//		Identifier を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier*
ColumnName::getIdentifier() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Identifier];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnName::setIdentifier -- Identifier を設定する
//
//	NOTES
//		Identifier を設定する
//
//	ARGUMENTS
//		Identifier* pIdentifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnName::setIdentifier(Identifier* pIdentifier_)
{
	m_vecpElements[f_Identifier] = pIdentifier_;
}

//
//	FUNCTION public
//		Statement::ColumnName::getIdentifierString
//			-- Identifier を ModUnicodeString で得る
//
//	NOTES
//		Identifier を ModUnicodeString で得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const ModUnicodeString*
ColumnName::getIdentifierString() const
{
	Identifier* pIdentifier = getIdentifier();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

//
//	FUNCTION public
//	Statement::ColumnName::copy -- 自身をコピーする
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
ColumnName::copy() const
{
	return new ColumnName(*this);
}

#if 0
namespace
{
	Analysis::ColumnName _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ColumnName::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::ColumnName::getAnalyzer2 -- 
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
ColumnName::
getAnalyzer2() const
{
	return Analysis::Value::ColumnName::create(this);
}

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
