// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ItemReference.cpp -- ItemReference
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ItemReference.h"
#include "Statement/Type.h"
#include "Statement/Identifier.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/ItemReference_Qualifier.h"
#include "Analysis/ItemReference_NoQualifier.h"
#endif
#include "Analysis/Value/ItemReference.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_ItemQualifier,
		f_ItemName,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::ItemReference::ItemReference -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pItemQualifier_
//		Identifier* pItemName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ItemReference::ItemReference(Identifier* pItemQualifier_, Identifier* pItemName_)
	: Object(ObjectType::ItemReference, f__end_index)
{
	// ItemQualifier を設定する
	setItemQualifier(pItemQualifier_);
	// ItemName を設定する
	setItemName(pItemName_);
}

//
//	FUNCTION public
//		Statement::ItemReference::toSQLStatement -- SQL文を得る
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
ItemReference::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	Identifier* pQualifier = getItemQualifier();
	Identifier* pItem = getItemName();
	return pQualifier ? pQualifier->toSQLStatement(bForCascade_).append(".").append(pItem->toSQLStatement(bForCascade_))
		: pItem->toSQLStatement(bForCascade_);
}

//
//	FUNCTION public
//		Statement::ItemReference::getItemQualifier -- ItemQualifier を得る
//
//	NOTES
//		ItemQualifier を得る
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
ItemReference::getItemQualifier() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_ItemQualifier];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ItemReference::setItemQualifier -- ItemQualifier を設定する
//
//	NOTES
//		ItemQualifier を設定する
//
//	ARGUMENTS
//		Identifier* pItemQualifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ItemReference::setItemQualifier(Identifier* pItemQualifier_)
{
	m_vecpElements[f_ItemQualifier] = pItemQualifier_;
}

//
//	FUNCTION public
//		Statement::ItemReference::getItemQualifierString
//			-- ItemQualifier を ModString で得る
//
//	NOTES
//		ItemQualifier を ModUnicodeString で得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModString*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const ModUnicodeString*
ItemReference::getItemQualifierString() const
{
	Identifier* pIdentifier = getItemQualifier();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

//
//	FUNCTION public
//		Statement::ItemReference::getItemName -- ItemName を得る
//
//	NOTES
//		ItemName を得る
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
ItemReference::getItemName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_ItemName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ItemReference::setItemName -- ItemName を設定する
//
//	NOTES
//		ItemName を設定する
//
//	ARGUMENTS
//		Identifier* pItemName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ItemReference::setItemName(Identifier* pItemName_)
{
	m_vecpElements[f_ItemName] = pItemName_;
}

//
//	FUNCTION public
//		Statement::ItemReference::getItemNameString -- ItemName を ModString で得る
//
//	NOTES
//		ItemName を ModString で得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModString*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const ModUnicodeString*
ItemReference::getItemNameString() const
{
	Identifier* pIdentifier = getItemName();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

//
//	FUNCTION public
//	Statement::ItemReference::copy -- 自身をコピーする
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
ItemReference::copy() const
{
	return new ItemReference(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::ItemReference_Qualifier _analyzerQualifier;
	Analysis::ItemReference_NoQualifier _analyzerNoQualifier;
}

// FUNCTION public
//	Statement::ItemReference::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
ItemReference::
getAnalyzer() const
{
	if (m_vecpElements[f_ItemQualifier])
		return &_analyzerQualifier;
	else
		return &_analyzerNoQualifier;
}
#endif

// FUNCTION public
//	Statement::ItemReference::getAnalyzer2 -- 
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
ItemReference::
getAnalyzer2() const
{
	return Analysis::Value::ItemReference::create(this);
}

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
