// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintElement.cpp -- HintElement
// 
// Copyright (c) 1999, 2000, 2003, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/HintElement.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/HintElement.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_HintPrimary,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::HintElement::HintElement -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Statement::Object* pHintPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HintElement::HintElement(Object* pHintPrimary_)
	: Object(ObjectType::HintElement, 0)
{
	// HintPrimary を加える
	append(pHintPrimary_);
}

//
//	FUNCTION public
//		Statement::HintElement::getHintPrimaryAt -- HintPrimary を得る
//
//	NOTES
//		HintPrimary を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		Statement::Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
HintElement::getHintPrimaryAt(int iAt_) const
{
	return m_vecpElements[iAt_];
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::HintElement::setHintPrimaryAt -- HintPrimary を設定する
//
//	NOTES
//
//	ARGUMENTS
//		int iAt_
//		Statement::Object* pHintPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
HintElement::setHintPrimaryAt(int iAt_, Object* pHintPrimary_)
{
	while (static_cast<int>(m_vecpElements.getSize()) <= iAt_)
		m_vecpElements.pushBack(0);
	m_vecpElements[iAt_] = pHintPrimary_;
}
#endif

//
//	FUNCTION public
//		Statement::HintElement::getHintPrimaryCount -- HintPrimary の個数を得る
//
//	NOTES
//		HintPrimary の個数を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
HintElement::getHintPrimaryCount() const
{
	return m_vecpElements.getSize();
}

//
//	FUNCTION public
//		Statement::HintElement::append -- HintPrimary を加える
//
//	NOTES
//		HintPrimary を加える
//
//	ARGUMENTS
//		Statement::Object* pHintPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
HintElement::append(Object* pHintPrimary_)
{
	m_vecpElements.pushBack(pHintPrimary_);
}

//
//	FUNCTION public
//	Statement::HintElement::copy -- 自身をコピーする
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
HintElement::copy() const
{
	return new HintElement(*this);
}

// FUNCTION public
//	Statement::HintElement::toSQLStatement -- get SQL statement
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
HintElement::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	int n = getHintPrimaryCount();
	if (n) {
		for (int i = 0; i < n; ++i) {
			cStream << " " << getHintPrimaryAt(i)->toSQLStatement(bForCascade_);
		}
	}
	return cStream.getString();
}

#if 0
namespace
{
	Analysis::HintElement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
HintElement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2000, 2003, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
