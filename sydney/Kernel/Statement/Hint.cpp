// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hint.cpp -- Hint
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/Hint.h"
#include "Statement/Type.h"
#include "Statement/HintElement.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/Hint.h"
#endif
#include "Analysis/Value/Hint.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_HintElement,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::Hint::Hint -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		HintElement* pHintElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Hint::Hint(HintElement* pHintElement_)
	: Object(ObjectType::Hint, 0)
{
	// HintElement を加える
	append(pHintElement_);
}

//
//	FUNCTION public
//		Statement::Hint::getHintElementAt -- HintElement を得る
//
//	NOTES
//		HintElement を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		HintElement*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HintElement*
Hint::getHintElementAt(int iAt_) const
{
	HintElement* pResult = 0;
	Object* pObj = m_vecpElements[iAt_];
	if ( pObj && ObjectType::HintElement == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(HintElement*, pObj);
	return pResult;
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::Hint::setHintElementAt -- HintElement を設定する
//
//	NOTES
//
//	ARGUMENTS
//		int iAt_
//		HintElement* pHintElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Hint::setHintElementAt(int iAt_, HintElement* pHintElement_)
{
	while (static_cast<int>(m_vecpElements.getSize()) <= iAt_)
		m_vecpElements.pushBack(0);
	m_vecpElements[iAt_] = pHintElement_;
}
#endif

//
//	FUNCTION public
//		Statement::Hint::getHintElementCount -- HintElement の個数を得る
//
//	NOTES
//		HintElement の個数を得る
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
Hint::getHintElementCount() const
{
	return m_vecpElements.getSize();
}

//
//	FUNCTION public
//		Statement::Hint::append -- HintElement を加える
//
//	NOTES
//		HintElement を加える
//
//	ARGUMENTS
//		HintElement* pHintElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Hint::append(HintElement* pHintElement_)
{
	m_vecpElements.pushBack(pHintElement_);
}

//
//	FUNCTION public
//	Statement::Hint::copy -- 自身をコピーする
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
Hint::copy() const
{
	return new Hint(*this);
}

// FUNCTION public
//	Statement::Hint::toSQLStatement -- get SQL statement
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
Hint::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	int n = getHintElementCount();
	if (n) {
		cStream << "HINT";
		for (int i = 0; i < n; ++i) {
			cStream << " " << getHintElementAt(i)->toSQLStatement(bForCascade_);
		}
	}
	return cStream.getString();
}

#if 0
namespace
{
	Analysis::Hint _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
Hint::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::Hint::getAnalyzer2 -- 
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
Hint::
getAnalyzer2() const
{
	return Analysis::Value::Hint::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
