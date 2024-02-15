// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Literal.cpp -- Literal
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Statement/Literal.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/DataValue.h"
#include "Statement/SQLParserL.h"
#include "Statement/Utility.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Literal.h"
#endif
#include "Analysis/Value/Literal.h"

#include "ModAutoPointer.h"
#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f__end_index
	};
}

// FUNCTION public
//	Statement::Literal::Literal -- コンストラクタ
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

Literal::Literal(const Token& cToken_)
	: Object(ObjectType::Literal, f__end_index),
	  m_cToken(cToken_),
	  m_cstrLiteral(),
	  m_bSeparated(false)
{
}

//	FUNCTION 
//	Statement::Literal::Literal -- コピーコンストラクタ
//
//	NOTES
//	コピーはSQL文が開放された後の利用のためなので
//	文字列表現を作っておく
//
//	ARGUMENTS
//		const Literal& cOther_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Literal::Literal(const Literal& cOther_)
	: Object(cOther_),
	  m_cToken(cOther_.m_cToken),
	  m_cstrLiteral(),
	  m_bSeparated(cOther_.m_bSeparated)
{
	setLiteralFromToken(cOther_.m_cToken);
	setTokenFromLiteral();
}

//	FUNCTION public
//	Statement::Literal::toSQLStatement -- SQL 文を得る
//
//	NOTES
//		SQL 文の文字列を得る。ただし、完全に SQL 文を再構成するわけではない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた SQL 文の文字列
//
//	EXCEPTIONS

// virtual
ModUnicodeString
Literal::toSQLStatement(bool bForCascade_ /* = false */) const
{
	if (!m_cstrLiteral.getLength())
		switch (m_cToken.getToken()) {
		case TOKEN__STRING:
		case TOKEN__STRING_WITH_QUOTE:

			// 生成済でない場合はトークンの前後に
			// クォート(' と ')を入れたものを生成する

			m_cstrLiteral.allocateCopy(getToken().getHead() - 1,
									   getToken().getLength() + 2);
			break;

		case TOKEN__BINARY_STRING:

			// 生成済でない場合はトークンの前後に
			// クォート(X' と ')を入れたものを生成する

			m_cstrLiteral.append(Common::UnicodeChar::usLargeX);
			m_cstrLiteral.append(getToken().getHead() - 1,
								 getToken().getLength() + 2);
			break;

		default:
			if (getToken().getLength())
				m_cstrLiteral.allocateCopy(getToken().getHead(),
										   getToken().getLength());
		}

	return m_cstrLiteral;
}

// FUNCTION public
//	Statement::Literal::isStringLiteral -- 文字列Literalか
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Literal::
isStringLiteral() const
{
	switch (m_cToken.getToken()) {
	case TOKEN__STRING:
	case TOKEN__STRING_WITH_QUOTE:
		return true;
	}
	return false;
}

// FUNCTION public
//	Statement::Literal::getToken -- Token を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Token&
//
// EXCEPTIONS

const Token&
Literal::
getToken() const
{
	return m_cToken;
}

// FUNCTION public
//	Statement::Literal::setToken -- Token をセットする
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
Literal::
setToken(const Token& cToken_)
{
	m_cToken = cToken_;
}

// FUNCTION public
//	Statement::Literal::createData -- Token が表すデータを作る
//
// NOTES
//
// ARGUMENTS
//	Common::DataType::Type eType = Common::DataType::Undefined
//		作成するデータの型、Undefinedなら内部で勝手に決める
//	bool bForAssign_ = false
//		trueの場合代入のための処理とみなす
//
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

Common::Data::Pointer
Literal::
createData(Common::DataType::Type eType_ /* = Common::DataType::Undefined */,
		   bool bForAssign_ /* = true */) const
{
	switch (m_cToken.getToken()) {
	case TOKEN__FLOAT_LITERAL:
		{
			return Common::StringData::getFloat(m_cToken.getHead(), m_cToken.getTail(), eType_, bForAssign_);
		}
	case TOKEN__INTEGER_LITERAL:
		{
			return Common::StringData::getInteger(m_cToken.getHead(), m_cToken.getTail(), eType_, bForAssign_);
		}
	case TOKEN__DATE:
		{
			return new Common::DateData(m_cToken.getHead(), m_cToken.getTail());
		}
	case TOKEN__TIMESTAMP:
		{
			return new Common::DateTimeData(m_cToken.getHead(), m_cToken.getTail());
		}
	case TOKEN__STRING:
		{
			if (m_cToken.getLength() == 0)
				return new Common::StringData;
			else
				return new Common::StringData(m_cToken.getHead(), m_cToken.getLength());
		}
	case TOKEN__STRING_WITH_QUOTE:
		{
			ModUnicodeString cstrString;
			Utility::replaceDuplicateCharacter(m_cToken.getHead(), m_cToken.getTail(),
											   Common::UnicodeChar::usQuate,
											   cstrString);
			return new Common::StringData(cstrString);
		}
	case TOKEN__BINARY_STRING:
	{
		ModAutoPointer<Common::BinaryData> p(new Common::BinaryData);
		p->decodeString(getToken().getHead(), getToken().getLength(), 16);
		return p.release();
	}
                   	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

// FUNCTION public
//	Statement::Literal::setSeparated -- set a value expression as included in parenthesis
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
Literal::
setSeparated()
{
	m_bSeparated = true;
}

// FUNCTION public
//	Statement::Literal::isSeparated -- get whether a value expression as included in parenthesis
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Literal::
isSeparated() const
{
	return m_bSeparated;
}

//
//	FUNCTION public
//	Statement::Literal::copy -- 自身をコピーする
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
Literal::copy() const
{
	return new Literal(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::Literal _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
Literal::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::Literal::getAnalyzer2 -- 
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
Literal::
getAnalyzer2() const
{
	return Analysis::Value::Literal::create(this);
}

// FUNCTION public
//	Statement::Literal::serialize -- 
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
Literal::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	m_cToken.serialize(cArchive_);
	if (cArchive_.isStore()) {
		// create string to store
		setLiteralFromToken(m_cToken);
		cArchive_(m_cstrLiteral);
	} else {
		// load string
		cArchive_(m_cstrLiteral);
		// set token from string
		setTokenFromLiteral();
	}
	cArchive_(m_bSeparated);
}

// FUNCTION private
//	Statement::Literal::setLiteralFromToken -- Tokenから文字列を作る
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
Literal::
setLiteralFromToken(const Token& cToken_)
{
	switch (cToken_.getToken()) {
	case TOKEN__STRING:
	case TOKEN__STRING_WITH_QUOTE:
		{
			//前後の「'」を含める
			m_cstrLiteral.allocateCopy(cToken_.getHead() - 1,
									   cToken_.getLength() + 2);
			break;
		}
	case TOKEN__BINARY_STRING:
		{
			// トークンの前後に
			// クォート(X' と ')を入れたものを生成する

			m_cstrLiteral.clear();
			m_cstrLiteral.append(Common::UnicodeChar::usLargeX);
			m_cstrLiteral.append(cToken_.getHead() - 1,
								 cToken_.getLength() + 2);
			break;
		}
	default:
		{
			if (cToken_.getLength() == 0)
				m_cstrLiteral = ModUnicodeString();
			else
				m_cstrLiteral = ModUnicodeString(cToken_.getHead(),
												 cToken_.getLength());
			break;
		}
	}
}

// FUNCTION private
//	Statement::Literal::setTokenFromLiteral -- 文字列からTokenを作る
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
Literal::
setTokenFromLiteral()
{
	switch (m_cToken.getToken()) {
	case TOKEN__STRING:
	case TOKEN__STRING_WITH_QUOTE:
		{
			m_cToken = Token(m_cToken.getToken(),
							 static_cast<const ModUnicodeChar*>(m_cstrLiteral) + 1,
							 m_cstrLiteral.getTail() - 1);
			break;
		}
	case TOKEN__BINARY_STRING:
		{
			m_cToken = Token(m_cToken.getToken(),
							 static_cast<const ModUnicodeChar*>(m_cstrLiteral) + 2,
							 m_cstrLiteral.getTail() - 1);
			break;
		}
	default:
		{
			m_cToken = Token(m_cToken.getToken(),
							 static_cast<const ModUnicodeChar*>(m_cstrLiteral),
							 m_cstrLiteral.getTail());
			break;
		}
	}
}

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
