// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataValue.cpp -- Common::Data値
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/Assert.h"
#include "Statement/DataValue.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"
#include "Common/Data.h"
#include "Common/DataType.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DoubleData.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"

#include "Exception/NotSupported.h"
#if 0
#include "Analysis/DataValue.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::DataValue::DataValue -- コンストラクタ (1)
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
DataValue::DataValue()
	: Object(ObjectType::DataValue, 0),
	  m_pData()
{
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::DataValue::DataValue -- コンストラクタ (2)
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data* pData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

DataValue::DataValue(const Common::Data* pData_)
	: Object(ObjectType::DataValue, 0)
{
	setValue(pData_);
}

//	FUNCTION public
//	Statement::DataValue::DataValue -- コンストラクタ (3)
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//		const ModUnicodeString* pstrValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

DataValue::DataValue(Common::DataType::Type eType_,
					 const ModUnicodeString* pstrValue_)
	: Object(ObjectType::DataValue, 0)
{
	setValue(eType_, pstrValue_);
}
#endif

//
//	FUNCTION public
//		Statement::DataValue::~DataValue -- デストラクタ
//
//	NOTES
//		デストラクタ
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
DataValue::~DataValue()
{
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//		Statement::DataValue::toString -- 文字列化
//
//	NOTES
//		文字列化
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
ModUnicodeString
DataValue::toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "(";
	cStream << getTypeName(getType());

	// メンバの出力
	cStream << " " << m_pData->toString();

	cStream << ")";

	return ModUnicodeString(cStream.getString());
}

//
//	FUNCTION 
//		Statement::DataValue::toString -- LISP形式で出力する
//
//	NOTES
//		LISP形式で出力する
//
//	ARGUMENTS
//		ModOstrStream& cStream_
//		int iIndent_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DataValue::toString(ModUnicodeOstrStream& cStream_, int iIndent_) const
{
	for (int i=0; i<iIndent_; ++i)
		cStream_ << ' ';
	cStream_ << '(';
	cStream_ << getTypeName(getType());

	// メンバの出力
	cStream_ << ' ' << m_pData->toString();

	cStream_ << ')';
}
#endif

//
//	FUNCTION public
//		Statement::DataValue::getValue -- 値を得る
//
//	NOTES
//		値を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Common::Data*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const Common::Data::Pointer&
DataValue::getValue() const
{
	return m_pData;
}

//
//	FUNCTION public
//		Statement::DataValue::setValue -- 値を設定する
//
//	NOTES
//		値を設定する
//
//	ARGUMENTS
//		const Common::Data* cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DataValue::setValue(const Common::Data* pData_)
{
	m_pData = pData_->copy();
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::DataValue::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//		const ModUnicodeString* pstrValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DataValue::setValue(Common::DataType::Type eType_,
					const ModUnicodeString* pstrValue_)
{
	switch(eType_) {
	case Common::DataType::Date:
		m_pData = new Common::DateData(*pstrValue_);
		break;
	case Common::DataType::DateTime:
		m_pData = new Common::DateTimeData(*pstrValue_);
		break;
	case Common::DataType::Integer:
		m_pData = new Common::IntegerData(ModUnicodeCharTrait::toInt(*pstrValue_));
		break;
	case Common::DataType::Double:
		m_pData = new Common::DoubleData(static_cast<double>(ModUnicodeCharTrait::toDouble(*pstrValue_)));
		break;
	case Common::DataType::String:
		m_pData = new Common::StringData(*pstrValue_);
		break;
	default:
		m_pData = Common::Data::Pointer();
		break;
	}
}
#endif

//
//	FUNCTION public
//	Statement::AlterDatabaseAttribute::copy -- 自身をコピーする
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
DataValue::copy() const
{
	return new DataValue(*this);
}

#if 0
namespace
{
	Analysis::DataValue _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DataValue::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DataValue::serialize -- 
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
DataValue::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::CommonData(cArchive_, m_pData);
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
