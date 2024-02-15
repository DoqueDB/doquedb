// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::Token -- スキャナトークン
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2009, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TOKEN_H
#define __SYDNEY_STATEMENT_TOKEN_H

#include "Statement/Module.h"

#include "Common/Object.h"

#include "ModUnicodeChar.h"

class ModArchive;
class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN
	
//	CLASS
//	Statement::Token -- スキャナトークン
//
//	NOTES
//		SQLParserに渡されたModUnicodeStringオブジェクトが破棄されるまで有効

class Token : public Common::Object
{
public:
	// デフォルトコンストラクタ
	SYD_STATEMENT_FUNCTION
	Token();
	// コンストラクタ (2)
	SYD_STATEMENT_FUNCTION
	Token(int iTokenID_);
	// コンストラクタ (3)
	SYD_STATEMENT_FUNCTION
	Token(int iTokenID_, const ModUnicodeChar* pHead_, ModSize uLength_);
	// コンストラクタ (4)
	SYD_STATEMENT_FUNCTION
	Token(int iTokenID_, const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);
	// コンストラクタ (5)
	SYD_STATEMENT_FUNCTION
	Token(int iTokenID_, int iValue_);
	// コピーコンストラクター
	SYD_STATEMENT_FUNCTION
	Token(const Token& cOther_);
	// デストラクタ
	SYD_STATEMENT_FUNCTION
	~Token();

	// アクセサ
	int getToken() const;
	// アクセサ
	const ModUnicodeChar* getHead() const;
	const ModUnicodeChar* getTail() const;
	ModSize getLength() const;
	// アクセサ
	int getInteger() const;

	// 代入オペレーター
	Token& operator=(const Token& cOther_);

	// serialize用
	void serialize(ModArchive& cArchive_);

private:
	// タイプ
	int m_iTokenID;
	// 値
	const ModUnicodeChar* m_pHead;
	const ModUnicodeChar* m_pTail;
	int m_iIntValue;
};

_SYDNEY_STATEMENT_END

//
//	FUNCTION public
//		Statement::Token::getToken -- 型を得る
//
//	NOTES
//		型を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//		なし
//
inline
int
Statement::Token::getToken() const
{
	return m_iTokenID;
}

//
//	FUNCTION public
//		Statement::Token::getHead -- トークン文字列の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeChar*
//
//	EXCEPTIONS
//		なし
//
inline
const ModUnicodeChar*
Statement::Token::getHead() const
{
	return m_pHead;
}

//
//	FUNCTION public
//		Statement::Token::getTail -- トークン文字列の終端アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeChar*
//
//	EXCEPTIONS
//		なし
//
inline
const ModUnicodeChar*
Statement::Token::getTail() const
{
	return m_pTail;
}

// FUNCTION public
//	Statement::Statement::Token::getLength -- トークンの長さを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

inline
ModSize
Statement::Token::
getLength() const
{
	return static_cast<ModSize>(m_pTail - m_pHead);
}

//
//	FUNCTION public
//		Statement::Token::getInteger -- 整数値を得る
//
//	NOTES
//		テキストを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//		なし
//
inline
int
Statement::Token::getInteger() const
{
	return m_iIntValue;
}

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TOKEN_H

//
// Copyright (c) 1999, 2002, 2003, 2005, 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
