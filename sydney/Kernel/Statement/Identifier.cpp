// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Identifier.cpp -- Identifier
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/Identifier.h"
#include "Statement/SQLParserL.h"
#include "Statement/StringValue.h"
#include "Statement/Token.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/InvalidIdentifier.h"
#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/Identifier.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f__end_index
	};

	// Identifierに使えない文字
	ModUnicodeString _cstrForbiddenChar("\\/:*?\"<>|");
	bool _isForbidden(const ModUnicodeChar* p_)
	{
		return _cstrForbiddenChar.search(*p_);
	}
}

//
//	FUNCTION public
//		Statement::Identifier::Identifier -- コンストラクタ (1)
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
Identifier::Identifier()
	: Object(ObjectType::Identifier, f__end_index),
	  m_cToken(), m_cstrIdentifier(), m_cstrCopy()
{
}

//
//	FUNCTION public
//		Statement::Identifier::Identifier -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		const Token& cToken_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier::Identifier(const Token& cToken_)
	: Object(ObjectType::Identifier, f__end_index),
	  m_cToken(cToken_), m_cstrIdentifier(), m_cstrCopy()
{
}

// FUNCTION public
//	Statement::Identifier::Identifier -- コピーコンストラクタ
//
// NOTES
//	コピーはSQL文が開放された後の利用のためなので
//	文字列表現を作っておく
//
// ARGUMENTS
//	const Identifier& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Identifier::
Identifier(const Identifier& cOther_)
	: Object(cOther_),
	  m_cToken(cOther_.m_cToken),
	  m_cstrIdentifier(),
	  m_cstrCopy()
{
	setStringFromToken(cOther_.m_cToken);
	setTokenFromString();
}

//
//	FUNCTION public
//		Statement::Identifier::~Identifier -- デストラクタ
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
Identifier::~Identifier()
{
}

//
//	FUNCTION public
//		Statement::Identifier::toSQLStatement -- SQL文を得る
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
Identifier::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	switch (m_cToken.getToken()) {
	case TOKEN__IDENTIFIER_WITH_QUOTE:
		{
			// 前後の「"」を含める
			return ModUnicodeString(m_cToken.getHead() - 1, m_cToken.getLength() + 2);
		}
	}
	if (m_cToken.getLength() == 0)
		return ModUnicodeString();
	else
		return ModUnicodeString(m_cToken.getHead(), m_cToken.getLength());
}

//
//	FUNCTION public
//		Statement::Identifier::setToken -- Tokenを設定する
//
//	NOTES
//		Tokenを設定する
//
//	ARGUMENTS
//		const Token& cToken_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Identifier::setToken(const Token& cToken_)
{
	m_cToken = cToken_;
}

//
//	FUNCTION public
//		Statement::Identifier::getToken -- Tokenを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const Token&
//
//	EXCEPTIONS
//
const Token&
Identifier::getToken() const
{
	return m_cToken;
}

// FUNCTION public
//	Statement::Identifier::getIdentifier -- 名前を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModUnicodeString*
//
// EXCEPTIONS

const ModUnicodeString*
Identifier::
getIdentifier() const
{
	if (m_cstrIdentifier.getLength() == 0 && m_cToken.getLength() > 0) {
		// check forbidden char
		for (const ModUnicodeChar* p = m_cToken.getHead();
			 p != m_cToken.getTail();
			 ++p) {
			if (_isForbidden(p)) {
				_SYDNEY_THROW1(Exception::InvalidIdentifier,
							   ModUnicodeString(*p));
			}
		}
		const_cast<ModUnicodeString&>(m_cstrIdentifier) = ModUnicodeString(m_cToken.getHead(), m_cToken.getLength());
	}
	return &m_cstrIdentifier;
}

//
//	FUNCTION public
//	Statement::Identifier::copy -- 自身をコピーする
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
Identifier::copy() const
{
	return new Identifier(*this);
}

#if 0
namespace
{
	Analysis::Identifier _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
Identifier::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::Identifier::serialize -- 
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
Identifier::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	m_cToken.serialize(cArchive_);
	if (cArchive_.isStore()) {
		// create string to store
		setStringFromToken(m_cToken);
		cArchive_(m_cstrCopy);
	} else {
		// load string
		cArchive_(m_cstrCopy);
		// set token from string
		setTokenFromString();
	}
}

// FUNCTION private
//	Statement::Identifier::setStringFromToken -- 
//
// NOTES
//
// ARGUMENTS
//	const Token& cToken_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Identifier::
setStringFromToken(const Token& cToken_)
{
	// Tokenの種類に応じて前後も入るようにする
	switch (cToken_.getToken()) {
	case TOKEN__IDENTIFIER:
	case TOKEN__IDENTIFIER_WITH_QUOTE:
		{
			// 前後の「"」を含める
			m_cstrCopy = ModUnicodeString(cToken_.getHead() - 1,
										  cToken_.getLength() + 2);
			break;
		}
	default:
		{
			if (cToken_.getLength() == 0)
				m_cstrCopy = ModUnicodeString();
			else
				m_cstrCopy = ModUnicodeString(cToken_.getHead(),
											  cToken_.getLength());
			break;
		}
	}
}

// FUNCTION private
//	Statement::Identifier::setTokenFromString -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Identifier::
setTokenFromString()
{
	// [NOTE]
	//	m_cstrCopy, m_cToken.getToken()には値が入っていること

	switch (m_cToken.getToken()) {
	case TOKEN__IDENTIFIER:
	case TOKEN__IDENTIFIER_WITH_QUOTE:
		{
			// 前後の「"」を含める
			m_cToken = Token(m_cToken.getToken(),
							 static_cast<const ModUnicodeChar*>(m_cstrCopy) + 1,
							 m_cstrCopy.getTail() - 1);
			break;
		}
	default:
		{
			m_cToken = Token(m_cToken.getToken(),
							 static_cast<const ModUnicodeChar*>(m_cstrCopy),
							 m_cstrCopy.getTail());
			break;
		}
	}
}

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
