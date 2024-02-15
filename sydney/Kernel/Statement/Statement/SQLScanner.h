// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLScanner.h -- SQL文トークナイザ
// 
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SQL_SCANNER_H
#define __SYDNEY_STATEMENT_SQL_SCANNER_H

#include "Statement/Module.h"

#include "Common/Object.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Token;

//	CLASS
//		SQLScanner -- 簡易版トークナイザ
//
//	NOTES

class SQLScanner : public Common::Object
{
public:
	// コンストラクタ (1)
	SQLScanner();
	// コンストラクタ (2)
	explicit SQLScanner(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_);

	// 次のトークンを得る
	SYD_STATEMENT_FUNCTION
	Token* getNextToken();
	// すでにEOFか
	SYD_STATEMENT_FUNCTION
	bool isEOF() const;

	// 行番号を得る
	SYD_STATEMENT_FUNCTION
	int getLine() const;

	// Parse結果のSQL文を得るための関数
	// 最後に読んだトークンの先頭位置を返す
	SYD_STATEMENT_FUNCTION
	const ModUnicodeChar* getLastTokenBeginning() const;
#ifndef SYD_COVERAGE
	// 最後から2番目に読んだトークンの末尾位置を返す
	const ModUnicodeChar* get2ndLastTokenEnd() const;
#endif
	// 次のPlaceHolder番号を得る
	SYD_STATEMENT_FUNCTION
	int getNextPlaceHolder() const;

private:
	// コピーコンストラクタは使わない
	SQLScanner(const SQLScanner& cOther_);
	// 代入オペレーターは使わない
	SQLScanner& operator=(const SQLScanner& cOther_);

	// キーワード検索
	bool isKeywordRegisted(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, int& iID);

	// スペース文字判定
	bool isSpace(const ModUnicodeChar suChar);
	
	// メンバ変数
	// 対象となるテキスト
	const ModUnicodeChar* m_pHead;
	const ModUnicodeChar* m_pTail;
	// 次に文字を読む位置
	const ModUnicodeChar* m_pszNext;
	// 最後のトークンの最初
	const ModUnicodeChar* m_pszLastBeg;
	// 最後から2番目のトークンの最後
	const ModUnicodeChar* m_psz2ndLastEnd;
	// すでにeofに達したかどうか
	bool m_bIsEOF;
	// 次の?(placeholder)の番号
	int m_iPlaceHolder;
	// 行番号
	int m_iLine;
	// 行頭か
	bool m_bIsBOL;
};

//	FUNCTION public
//	Statement::SQLScanner::SQLScanner -- コンストラクタ (1)
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
SQLScanner::SQLScanner()
	: m_pHead(0),
	  m_pTail(0),
	  m_pszNext(0),
	  m_pszLastBeg(0),
	  m_psz2ndLastEnd(0),
	  m_bIsEOF(false),
	  m_iPlaceHolder(0),
	  m_iLine(1),
	  m_bIsBOL(true)
{}

//	FUNCTION public
//		Statement::SQLScanner::SQLScanner -- コンストラクタ (2)
//
//	NOTES
//
//	ARGUMENTS
//		const ModUnicodeChar* pHead_
//		const ModUnicodeChar* pTail_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
SQLScanner::SQLScanner(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
	: m_pHead(pHead_),
	  m_pTail(pTail_),
	  m_pszNext(pHead_),
	  m_pszLastBeg(pHead_),
	  m_psz2ndLastEnd(pHead_),
	  m_bIsEOF(false),
	  m_iPlaceHolder(0),
	  m_iLine(1),
	  m_bIsBOL(true)
{}

#ifndef SYD_COVERAGE
//	FUNCTION public
//		Statement::SQLScanner::get2ndLastTokenEnd()
//
//	NOTES
//		最後から2番目に読んだトークンの末尾位置を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeChar* 
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeChar*
SQLScanner::get2ndLastTokenEnd() const
{
	return m_psz2ndLastEnd;
}
#endif

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SQL_SCANNER_H

//
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
