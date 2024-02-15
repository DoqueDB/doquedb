// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLParser.cpp -- SQLパーザ
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#undef STATEMENT_TRACE

namespace {
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Statement/SQLParser.h"
#include "Statement/Object.h"
#include "Statement/SQLScanner.h"
#include "Statement/SQLWrapper.h"
#include "Statement/Token.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/SystemParameter.h"
#include "Exception/BadArgument.h"
#include "Exception/SQLSyntaxError.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModOsDriver.h"

#ifdef STATEMENT_TRACE
#include <stdio.h>
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

#ifndef SYD_COVERAGE
namespace {
//
//	FUNCTION local
//	$$::_isParameterKeep() -- パラメータを出力するかどうかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		出力する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool _isParameterKeep()
{
	return true;	// 常にtrue
}

}
#endif

void *SQLParserEngineAlloc(void *(*mallocProc)(ModSize,const char*,int));
void SQLParserEngineFree(void *p, void (*freeProc)(void*,const char*,int));
#ifdef STATEMENT_TRACE
void SQLParserEngineTrace(FILE *TraceFILE, char *zTracePrompt);
#endif
void SQLParserEngine(
  void *yyp,				   /* The parser */
  int yymajor,				   /* The major token code number */
  Token* yyminor,			   /* The value for the token */
  SQLParser* pParser
);

namespace {

void *Alloc(ModSize size_, const char* file_, int line_);
void Free(void *pMem_, const char* file_, int line_);

//
//	FUNCTION global
//		$$::Alloc -- メモリalloc
//
//	NOTES
//		alloc
//
//	ARGUMENTS
//		ModSize size_
//			領域の大きさ
//		const char* file_
//			呼び出し側の__FILE__
//		int line_
//			呼び出し側の__LINE__
//
//	RETURN
//		void*
//			allocしたメモリ
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//

void*
Alloc(ModSize size_, const char* file_, int line_)
{
	return ModOsDriver::Memory::alloc(size_);
}

//
//	FUNCTION global
//		$$::Free -- メモリalloc
//
//	NOTES
//		alloc
//
//	ARGUMENTS
//		void* pMem_
//			領域
//		const char* file_
//			呼び出し側の__FILE__
//		int line_
//			呼び出し側の__LINE__
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外を再送
//

void
Free(void* pMem_, const char* file_, int line_)
{
	ModOsDriver::Memory::free(pMem_);
}

} // end of namespace

_SYDNEY_USING

//
//	FUNCTION public
//		Statement::SQLParser::SQLParser -- コンストラクタ (1)
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
//		下位の例外を再送
//
SQLParser::SQLParser()
	: m_pScanner(0),
	  m_iState(SQLParser::PARSE_NO_TEXT),
	  m_pLemon(SQLParserEngineAlloc(Alloc)),
	  m_pResult(0)
{
#ifdef STATEMENT_TRACE
	SQLParserEngineTrace(stdout, "");
#endif
}

//
//	FUNCTION public
//		Statement::SQLParser::~SQLParser -- デストラクタ
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
//		下位の例外を再送
//
SQLParser::~SQLParser()
{
	delete m_pScanner;
	if (m_pLemon) {
		SQLParserEngineFree(m_pLemon, Free);
	}
}

//
//	FUNCTION public
//		Statement::SQLParser::setText -- パーズすべき文字列を与える
//
//	NOTES
//		パーズすべき文字列を与える
//
//	ARGUMENTS
//		ModString& cstrText_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		下位の例外を再送
//
void
SQLParser::setText(const ModUnicodeString& cstrText_)
{
	m_pScanner = new SQLScanner(cstrText_, cstrText_.getTail());
}

//
//	FUNCTION public
//		Statement::SQLParser::parse -- パーズ結果を得る
//
//	NOTES
//		パーズ結果を得る
//
//	ARGUMENTS
//		Statement::Object*& pResult_
//
//	RETURN
//		int
//			パーザの状態
//
//	EXCEPTIONS
//	SQLSyntaxErrorException
//		SQL文の文法エラー
//	その他
//		下位の例外を再送
//
int
SQLParser::parse(Statement::Object*& pResult_)
{
	pResult_ = 0;
	m_pResult = 0;
	if (m_pScanner == 0) return PARSE_NO_TEXT;
	if (m_pScanner->isEOF()) return PARSE_EOF;

	const ModUnicodeChar* pszStart = 0;
	int iPlaceHolderStart = 0;
	try{
		pszStart = m_pScanner->getLastTokenBeginning();
		iPlaceHolderStart = m_pScanner->getNextPlaceHolder();
		do {
			Token* pToken = m_pScanner->getNextToken();
			if (pToken == 0) {
				SQLParserEngine(m_pLemon, 0, 0, this);
			} else {
				setState(PARSE_PROGRESS);
				SQLParserEngine(m_pLemon, pToken->getToken(), pToken, this);
			}
		} while (m_iState == PARSE_PROGRESS);
	} catch (Exception::SQLSyntaxError&) {
		delete m_pResult;
		throw;
	} catch (Exception::Object&) {
		delete m_pResult;
		_SYDNEY_RETHROW;
	} catch (...) {
		delete m_pResult;
		if (m_iState == PARSE_SYNTAX_ERROR) {
			m_cstrErrorMessage << " at line " << m_pScanner->getLine();
			throw Exception::SQLSyntaxError(moduleName
				                           ,srcFile
										   ,__LINE__
										   ,m_cstrErrorMessage.getString());
		}
		_SYDNEY_RETHROW;
	}
	if (m_iState == PARSE_NO_TEXT) {
		delete m_pResult;
		setState(PARSE_EOF);
	}
	if (m_iState == PARSE_SYNTAX_ERROR) {
		delete m_pResult;
		m_cstrErrorMessage << " at line " << m_pScanner->getLine();
		throw Exception::SQLSyntaxError(moduleName, srcFile, __LINE__,
										m_cstrErrorMessage.getString());
	}

	if (m_iState == PARSE_ACCEPT) {
		pResult_ = m_pResult;
		if (m_pResult == 0 && m_pScanner->isEOF()) {
			setState(PARSE_EOF);
		}
		if (m_pResult)
		{
			// UnsignedInteger -> Integer変換
			m_pResult->convertToSignedInteger();

			SQLWrapper* pWrapper = new SQLWrapper();
			pWrapper->setObject(m_pResult);
			pResult_ = m_pResult = pWrapper;

#ifndef SYD_COVERAGE
			if (_isParameterKeep()) {
				const ModUnicodeChar* pszEnd = m_pScanner->get2ndLastTokenEnd();
				if (pszStart && pszEnd) {
					ModUnicodeString cStmtStr(pszStart, (ModSize)(pszEnd - pszStart));
					int iPlaceHolderEnd = m_pScanner->getNextPlaceHolder();
					// placeholder(?)によって文が始まることはないので、
					// getLastPlaceHolder()は必要ない

					pWrapper->setSQLString(cStmtStr);
					pWrapper->setPlaceHolderLower(iPlaceHolderStart);
					pWrapper->setPlaceHolderUpper(iPlaceHolderEnd);
				}
			}
#endif
		}
	}

	return m_iState;
}

//
//	FUNCTION public
//		Statement::SQLParser::setState -- 状態を設定する
//
//	NOTES
//		状態を設定する
//
//	ARGUMENTS
//		int iState_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
SQLParser::setState(int iState_)
{
	m_iState = iState_;
}

// FUNCTION public
//	Statement::SQLParser::getState -- 状態を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SQLParser::getState()
{
	return m_iState;
}

//
//	FUNCTION public
//		Statement::SQLParser::setResult -- 結果を設定する
//
//	NOTES
//		結果を設定する
//
//	ARGUMENTS
//		Statement::Object* pObj_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
SQLParser::setResult(Statement::Object* pObj_)
{
	m_pResult = pObj_;
}

//
//	FUNCTION public
//		Statement::SQLParser::getMessage -- メッセージ
//
//	NOTES
//		メッセージ用のModOstrStreamを返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModOstrStream&
//
//	EXCEPTIONS
//		なし
//
ModUnicodeOstrStream&
SQLParser::getMessage()
{
	return m_cstrErrorMessage;
}

#ifdef OBSOLETE
//	FUNCTION public static
//		Statement::SQLParser::parseString -- 主としてデバッグ用
//
//	NOTES
//		主としてデバッグ用
//		与えられた文字列の最初の1文をparseし返す
//
//	ARGUMENTS
//		char* pszText_
//			parseしたい文字列
//
//	RETURN
//		Statement::Object* pObj
//			parse結果のStatement::Object。
//			使い終ったら呼出側で pObj->destruct() または delete pObj すること。
//			何らかのエラーがあった場合は例外を起こす
//
//	EXCEPTIONS
//	SQLSyntaxErrorException
//		syntax error
//	その他
//		下位の例外を再送
//
Object*
SQLParser::parseString(const ModUnicodeChar* pszText_)
{
	SQLParser* pParser = 0;
	Object* pObj = 0;
	try {
		pParser = new SQLParser();
		pParser->setText(pszText_);
		int rc = pParser->parse(pObj);
	} catch (Exception::SQLSyntaxError&) {
		delete pParser;
		if (pObj) pObj->destruct();
		throw;
	} catch (...) {
//		ModDebugMessage << "Parse error" << ModEndl;
		delete pParser;
		if (pObj) pObj->destruct();
		_SYDNEY_RETHROW;
	}
	delete pParser;
	return pObj;
}
#endif

//
//	FUNCTION public static
//		Statement::SQLParser::throwSyntaxErrorException -- 実行時の文法エラー検出
//
//	NOTES
//		実行時の文法エラー検出
//		BNF で記述できない文法エラーの部分（ユーザープログラム部分で検出）の
//		ための例外発生メソッド
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	SQLSyntaxErrorException
//		syntax error
//	その他
//		下位の例外を再送
//
void
SQLParser::throwSyntaxErrorException( const char* pstrzSrcFile_ ,int iLine_ )
{
	m_cstrErrorMessage << " at line " << m_pScanner->getLine();
	throw Exception::SQLSyntaxError(moduleName, pstrzSrcFile_, iLine_, m_cstrErrorMessage.getString());
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
