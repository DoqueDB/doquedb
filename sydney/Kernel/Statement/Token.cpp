// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Token.cpp -- スキャナトークン
// 
// Copyright (c) 2000, 2002, 2005, 2012, 2023 Ricoh Company, Ltd.
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
#include "Statement/Token.h"

_SYDNEY_USING

using namespace Statement;

// FUNCTION public
//	Statement::Token::Token -- デフォルトコンストラクタ
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

Token::
Token()
	: m_iTokenID(0),
	  m_pHead(0),
	  m_pTail(0),
	  m_iIntValue(0)
{}

//
//	FUNCTION public
//		Statement::Token::Token -- コンストラクタ(2)
//
//	NOTES
//		コンストラクタ(2)
//
//	ARGUMENTS
//		int iTokenID_
//
//  RETURN
//		なし
//	EXCEPTIONS
//		なし
//
Token::Token(int iTokenID_)
	: Common::Object(),
	  m_iTokenID(iTokenID_),
	  m_pHead(0),
	  m_pTail(0),
	  m_iIntValue(0)
{
}

//
//	FUNCTION public
//		Statement::Token::Token -- コンストラクタ(3)
//
//	NOTES
//		コンストラクタ(3)
//
//	ARGUMENTS
//		int iTokenID_
//		const ModUnicodeChar* pHead_
//		ModSize uLength_
//
//  RETURN
//		なし
//	EXCEPTIONS
//		なし
//
Token::Token(int iTokenID_, const ModUnicodeChar* pHead_, ModSize uLength_)
	: Common::Object(),
	  m_iTokenID(iTokenID_),
	  m_pHead(pHead_),
	  m_pTail(pHead_ + uLength_),
	  m_iIntValue(0)
{
}

//
//	FUNCTION public
//		Statement::Token::Token -- コンストラクタ(4)
//
//	NOTES
//		コンストラクタ(4)
//
//	ARGUMENTS
//		int iTokenID_
//		const ModUnicodeChar* pHead_
//		const ModUnicodeChar* pTail_
//
//  RETURN
//		なし
//	EXCEPTIONS
//		なし
//
Token::Token(int iTokenID_, const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
	: Common::Object(),
	  m_iTokenID(iTokenID_),
	  m_pHead(pHead_),
	  m_pTail(pTail_),
	  m_iIntValue(0)
{
}

//
//	FUNCTION public
//		Statement::Token::Token -- コンストラクタ(5)
//
//	NOTES
//		コンストラクタ(5)
//
//	ARGUMENTS
//		int iTokenID_
//		int iValue_
//
//  RETURN
//		なし
//	EXCEPTIONS
//		なし
//
Token::Token(int iTokenID_, int iValue_)
	: Common::Object(),
	  m_iTokenID(iTokenID_),
	  m_pHead(0),
	  m_pTail(0),
	  m_iIntValue(iValue_)
{
}

// コピーコンストラクター
Token::
Token(const Token& cOther_)
	: m_iTokenID(cOther_.m_iTokenID),
	  m_pHead(cOther_.m_pHead),
	  m_pTail(cOther_.m_pTail),
	  m_iIntValue(cOther_.m_iIntValue)
{}

//
//	FUNCTION public
//		Statement::Token::~Token -- デストラクタ
//
//	NOTES
//		デストラクタ
//
//	ARGUMENTS
//		なし
//
//  RETURN
//		なし
//	EXCEPTIONS
//		なし
//
Token::~Token()
{
}

// 代入オペレーター
Token&
Token::
operator=(const Token& cOther_)
{
	if (this != &cOther_) {
		m_iTokenID = cOther_.m_iTokenID;
		m_pHead = cOther_.m_pHead;
		m_pTail = cOther_.m_pTail;
		m_iIntValue = cOther_.m_iIntValue;
	}
	return *this;
}

// FUNCTION public
//	Statement::Token::serialize -- serialize用
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

void
Token::
serialize(ModArchive& cArchive_)
{
	cArchive_(m_iTokenID);
	cArchive_(m_iIntValue);
}

//
//	Copyright (c) 2000, 2002, 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
