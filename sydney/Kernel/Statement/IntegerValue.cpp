// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerValue.cpp -- 整数値/enum
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/IntegerValue.h"
#include "Statement/Type.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/IntegerValue.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::IntegerValue::IntegerValue -- コンストラクタ (1)
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
IntegerValue::IntegerValue()
	: Object(ObjectType::IntegerValue), m_iValue(0)
{
}

//
//	FUNCTION public
//		Statement::IntegerValue::IntegerValue -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
IntegerValue::IntegerValue(int iValue_)
	: Object(ObjectType::IntegerValue), m_iValue(iValue_)
{
}

// コピーコンストラクタ
IntegerValue::
IntegerValue(const IntegerValue& cOther_)
	: Object(cOther_),
	  m_iValue(cOther_.m_iValue)
{}

//
//	FUNCTION public
//		Statement::IntegerValue::~IntegerValue -- デストラクタ
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
IntegerValue::~IntegerValue()
{
}

//
//	FUNCTION public
//		Statement::IntegerValue::toSQLStatement -- SQL文を得る
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
IntegerValue::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << m_iValue;
	return cStream.getString();
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//		Statement::IntegerValue::toString -- 文字列化
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
IntegerValue::toString() const
{
	ModUnicodeOstrStream cStream;
	cStream << "(";
	cStream << Statement::getTypeName(getType());

	// メンバの出力
	cStream << " " << m_iValue;

	cStream << ")";

	return ModUnicodeString(cStream.getString());
}

//
//	FUNCTION 
//		Statement::IntegerValue::toString -- LISP形式で出力する
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
IntegerValue::toString(ModUnicodeOstrStream& cStream_,
					   int iIndent_) const
{
	for (int i=0; i<iIndent_; ++i)
		cStream_ << ' ';
	cStream_ << '(';
	cStream_ << Statement::getTypeName(getType());

	// メンバの出力
	cStream_ << ' ' << m_iValue;

	cStream_ << ')';
}
#endif

//
//	FUNCTION public
//		Statement::IntegerValue::getValue -- 値を得る
//
//	NOTES
//		値を得る
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
IntegerValue::getValue() const
{
	return m_iValue;
}

//
//	FUNCTION public
//		Statement::IntegerValue::setValue -- 値を設定する
//
//	NOTES
//		値を設定する
//
//	ARGUMENTS
//		const iValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
IntegerValue::setValue(const int iValue_)
{
	m_iValue = iValue_;
}

//
//	FUNCTION public
//	Statement::IntegerValue::copy -- 自身をコピーする
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
IntegerValue::copy() const
{
	return new IntegerValue(*this);
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
// ハッシュコードを計算する
//virtual
ModSize
IntegerValue::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 4;
	value += m_iValue;
	return value;
}

// 同じ型のオブジェクト同士でless比較する
//virtual
bool
IntegerValue::
compare(const Object& cObj_) const
{
	return Super::compare(cObj_)
		|| m_iValue < _SYDNEY_DYNAMIC_CAST(const IntegerValue&, cObj_).m_iValue;
}
#endif

#if 0
namespace
{
	Analysis::IntegerValue _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
IntegerValue::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::IntegerValue::serialize -- 
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
IntegerValue::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_iValue);
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
