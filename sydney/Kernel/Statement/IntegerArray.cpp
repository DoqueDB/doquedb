// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerArray.cpp --
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
#include "Statement/IntegerValue.h"
#include "Statement/IntegerArray.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/IntegerArray.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
   	// メンバの m_vecpElements 内でのindex
	enum {
        f__end_index
	};
}

//
//	FUNCTION public
//	Statement::IntegerArray::IntegerArray -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
IntegerArray::IntegerArray()
        : Object(ObjectType::IntegerArray, f__end_index)
{
}

//
//	FUNCTION public
//	Statement::IntegerArray::~IntegerArray -- デストラクタ
//
//	NOTES
//	デストラクタ
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
IntegerArray::~IntegerArray()
{
}

//	FUNCTION public
//	Statement::VerifyStatement::getValue -- 値を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const int iIdx_
//		  	インデックス値
//
//	RETURN
//		int
//		スキーマ種別
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送

int
IntegerArray::getValue(const int iIdx_) const
{
    const IntegerValue* pInt = _SYDNEY_DYNAMIC_CAST(
		const IntegerValue*, m_vecpElements[iIdx_]);
    return pInt->getValue();
}
    
#ifdef OBSOLETE
//	FUNCTION public
//		Statement::VerifyStatement::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const int iIdx_
//			配列のインデックス
//		const int iValue_
//			格納する値
//
//	RETURN
//
//	EXCEPTIONS

void
IntegerArray::setValue(const int iIdx_, const int iValue_)
{
	while (static_cast<int>(m_vecpElements.getSize()) <= iIdx_)
		m_vecpElements.pushBack(new IntegerValue);
	IntegerValue* pInt = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[iIdx_]);
	pInt->setValue(iValue_);
}
#endif

//
//	FUNCTION public
//		Statement::VerifyStatement::pushBack -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const int iValue_
//			格納する値
//
//	RETURN
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IntegerArray::pushBack(const int iValue_)
{
    m_vecpElements.pushBack(new IntegerValue(iValue_));
}

//
//	FUNCTION public
//		Statement::VerifyStatement::getSize -- 配列のサイズを取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int 
IntegerArray::getSize() const
{
    return m_vecpElements.getSize();
}

#ifdef OBSOLETE
//
//	FUNCTION public
//		Statement::VerifyStatement::
//
//	NOTES
//
//	ARGUMENTS
//		const int iIdx_
//			配列のインデックス
//
//	RETURN
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
IntegerArray::operator[](int iIdx_)
{
    return getValue(iIdx_);
}

const int
IntegerArray::operator[](const int iIdx_) const
{
    return getValue(iIdx_);
}
#endif

#if 0
namespace
{
	Analysis::IntegerArray _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
IntegerArray::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
