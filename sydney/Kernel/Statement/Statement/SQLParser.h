// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SQLParser -- SQLパーザ
// 
// Copyright (c) 1999, 2000, 2003, 2005, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SQLPARSER_H
#define __SYDNEY_STATEMENT_SQLPARSER_H

#include "Common/Object.h"
#include "Statement/Object.h"
#include "Statement/SQLScanner.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//	CLASS
//	SQLParser -- SQLパーザ
//
//	NOTES

class SYD_STATEMENT_FUNCTION SQLParser : public Common::Object
{
public:
	// コンストラクタ (1)
	SQLParser();
	// デストラクタ
	~SQLParser();

	// パーズすべき文字列を与える
	void setText(const ModUnicodeString& cstrText_);

	// パーズ結果を得る
	int parse(Statement::Object*& pResult_);
	enum ParserStatus {
		PARSE_PROGRESS = 0,
		PARSE_ACCEPT,
		PARSE_EOF,
		PARSE_NO_TEXT,
		PARSE_SYNTAX_ERROR
	};

	// 状態を設定する
	void setState(int iState_);
	// 状態を得る
	int getState();
	// 結果を設定する
	void setResult(Statement::Object* pObj_);

	// メッセージ
	ModUnicodeOstrStream& getMessage();

#ifdef OBSOLETE
	// 文字列を受け取り、parseして返す。
	static Statement::Object* parseString(const ModUnicodeChar* pszText_);
#endif

	// Syntax Error ディテクト
	void throwSyntaxErrorException( const char* pstrzSrcFile_ ,int iLine_ );

private:
	// コピーコンストラクタは使わない
	SQLParser(const SQLParser& cOther_);
	// 代入オペレーターは使わない
	SQLParser& operator=(const SQLParser& cOther_);

	// メンバ変数
	// スキャナ
	SQLScanner* m_pScanner;
	// lemonパーザ
	void*	m_pLemon;
	// 状態
	int m_iState;
	// 結果
	Statement::Object* m_pResult;
	// エラーメッセージ
	ModUnicodeOstrStream m_cstrErrorMessage;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SQLPARSER_H

//
// Copyright (c) 1999, 2000, 2003, 2005, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
