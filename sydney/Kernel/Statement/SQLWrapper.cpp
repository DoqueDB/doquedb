// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLWrapper.cpp -- SQLWrapper
// 
// Copyright (c) 2001, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#include "Statement/SQLWrapper.h"
#include "Statement/Type.h"
#include "Statement/StringValue.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/SQLWrapper.h"
#endif

_SYDNEY_USING
using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Object,
		f_SQLString,
		f_Lower,
		f_Upper,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::SQLWrapper::SQLWrapper -- コンストラクタ (1)
//
//	NOTES
//		コンストラクタ (1)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
SQLWrapper::SQLWrapper()
	: Object(ObjectType::SQLWrapper, f__end_index)
{
}

//
//	FUNCTION public
//	Statement::SQLWrapper::getReleaseObject
//		-- Object 取得
//
//	NOTES
//	Object 取得
//	戻り値は呼出側が解放する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Object*
//		Object
//
//	EXCEPTIONS
//	なし
//
Object*
SQLWrapper::getReleaseObject()
{
	Object* pObj = m_vecpElements[f_Object];
	m_vecpElements[f_Object] = 0;
	return pObj;
}

//
//	FUNCTION public
//	Statement::SQLWrapper::getObject
//		-- Object 取得
//
//	NOTES
//	Object 取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Object*
//		Object
//
//	EXCEPTIONS
//	なし
//
Object*
SQLWrapper::getObject() const
{
	return m_vecpElements[f_Object];
}

//
//	FUNCTION public
//		Statement::SQLWrapper::setObject -- Object を設定する
//
//	NOTES
//		Objectを設定する
//
//	ARGUMENTS
//		Object* pObject_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SQLWrapper::setObject(Object* pObject_)
{
    m_vecpElements[f_Object] = pObject_;
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//		Statement::SQLWrapper::getSQLString -- SQLString を得る
//
//	NOTES
//		SQLString を得る
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
SQLWrapper::getSQLString() const
{
	const ModUnicodeString* pResult = 0;
	Object* pObj = m_vecpElements[f_SQLString];
	if ( pObj && ObjectType::StringValue == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(StringValue*, pObj)->getValue();
	return pResult;
}

//
//	FUNCTION public
//		Statement::SQLWrapper::setSQLString -- SQLString を設定する
//
//	NOTES
//		SQLString を設定する
//
//	ARGUMENTS
//		const ModUnicodeString& str_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SQLWrapper::setSQLString(const ModUnicodeString& str_)
{
	StringValue* pStrVal = _SYDNEY_DYNAMIC_CAST(StringValue*, m_vecpElements[f_SQLString]);

	if (pStrVal == 0)
	{
		pStrVal = new StringValue;
		m_vecpElements[f_SQLString] = pStrVal;
	}
	pStrVal->setValue(str_);
}

//
//	FUNCTION public
//		Statement::SQLWrapper::getPlaceHolderLower -- PlaceHolderLower を得る
//
//	NOTES
//		PlaceHolderLower を得る
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
SQLWrapper::getPlaceHolderLower() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_Lower];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::SQLWrapper::setPlaceHolderLower -- PlaceHolderLower を設定する
//
//	NOTES
//		PlaceHolderLower を設定する
//
//	ARGUMENTS
//		int iPlaceHolderLower_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SQLWrapper::setPlaceHolderLower(int iPlaceHolderLower_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Lower]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_Lower] = pIntVal;
	}
	pIntVal->setValue(iPlaceHolderLower_);
}

//
//	FUNCTION public
//		Statement::SQLWrapper::getPlaceHolderUpper -- PlaceHolderUpper を得る
//
//	NOTES
//		PlaceHolderUpper を得る
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
SQLWrapper::getPlaceHolderUpper() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_Upper];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::SQLWrapper::setPlaceHolderUpper -- PlaceHolderUpper を設定する
//
//	NOTES
//		PlaceHolderUpper を設定する
//
//	ARGUMENTS
//		int iPlaceHolderUpper_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SQLWrapper::setPlaceHolderUpper(int iPlaceHolderUpper_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Upper]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_Upper] = pIntVal;
	}
	pIntVal->setValue(iPlaceHolderUpper_);
}
#endif

//
//	FUNCTION public
//	Statement::SQLWrapper::copy -- 自身をコピーする
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
SQLWrapper::copy() const
{
	return new SQLWrapper(*this);
}

#if 0
namespace
{
	Analysis::SQLWrapper _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SQLWrapper::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2001, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
