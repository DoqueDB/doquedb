// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StringValue.cpp -- 文字列値
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"
#include "Statement/StringValue.h"
#include "Statement/Type.h"

#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"
#if 0
#include "Analysis/StringValue.h"
#endif

#include "ModHasher.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::StringValue::StringValue -- コンストラクタ (1)
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
StringValue::StringValue()
	: Object(ObjectType::StringValue)
{
}

//
//	FUNCTION public
//		Statement::StringValue::StringValue -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ModUnicodeString cstrValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
StringValue::StringValue(const ModUnicodeString& cstrValue_)
	: Object(ObjectType::StringValue), m_cstrValue(cstrValue_)
{
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::StringValue::StringValue -- コンストラクタ (3)
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*	pszValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

StringValue::StringValue(const ModUnicodeChar* pszValue_)
	: Object(ObjectType::StringValue), m_cstrValue(pszValue_)
{}
#endif

// コピーコンストラクタ
StringValue::
StringValue(const StringValue& cOther_)
	: Object(cOther_),
	  m_cstrValue(cOther_.m_cstrValue)
{}

//
//	FUNCTION public
//		Statement::StringValue::~StringValue -- デストラクタ
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
StringValue::~StringValue()
{
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//		Statement::StringValue::toString -- 文字列化
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
StringValue::toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "(";
	cStream << getTypeName(getType());

	// メンバの出力
	cStream << " " << m_cstrValue;

	cStream << ")";

	return ModUnicodeString(cStream.getString());
}

//
//	FUNCTION 
//		Statement::StringValue::toString -- LISP形式で出力する
//
//	NOTES
//		LISP形式で出力する
//
//	ARGUMENTS
//		ModUnicodeOstrStream& cStream_
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
StringValue::toString(ModUnicodeOstrStream& cStream_,
								 int iIndent_) const
{
	for (int i=0; i<iIndent_; ++i)
		cStream_ << ' ';
	cStream_ << '(';
	cStream_ << getTypeName(getType());

	// メンバの出力
	cStream_ << " '" << m_cstrValue;

	cStream_ << "')";
}
#endif

//
//	FUNCTION public
//		Statement::StringValue::getValue -- 値を得る
//
//	NOTES
//		値を得る
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
const ModUnicodeString*
StringValue::getValue() const
{
	return &m_cstrValue;
}

//
//	FUNCTION public
//		Statement::StringValue::setValue -- 値を設定する
//
//	NOTES
//		値を設定する
//
//	ARGUMENTS
//		const ModUnicodeString& cstrValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
StringValue::setValue(const ModUnicodeString& cstrValue_)
{
	m_cstrValue = cstrValue_;
}

//
//	FUNCTION public
//	Statement::StringValue::copy -- 自身をコピーする
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
StringValue::copy() const
{
	return new StringValue(*this);
}

// FUNCTION public
//	Statement::StringValue::toSQLStatement -- SQL文で値を得る
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
StringValue::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	if (m_cstrValue.compare("ORDER", ModFalse) == 0) {
		return ModUnicodeString("IN ORDER");
	}
	return m_cstrValue;
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
// ハッシュコードを計算する
//virtual
ModSize
StringValue::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 4;
	value += ModUnicodeStringHasher()(m_cstrValue);
	return value;
}

// 同じ型のオブジェクト同士でless比較する
//virtual
bool
StringValue::
compare(const Object& cObj_) const
{
	return Super::compare(cObj_)
		|| m_cstrValue < _SYDNEY_DYNAMIC_CAST(const StringValue&, cObj_).m_cstrValue;
}
#endif

#if 0
namespace
{
	Analysis::StringValue _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
StringValue::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::StringValue::serialize -- 
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
StringValue::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_cstrValue);
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
