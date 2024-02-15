// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- 例外クラス
// 
// Copyright (c) 1999, 2000, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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
}


#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Exception/Object.h"
#include "Exception/ErrorMessage.h"
#include "Exception/AllNumberFiles.h"
#include "Exception/AllExceptionFiles.h"

#include "Common/MessageStream.h"

#include "ModKanjiCode.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"
#include "ModMessage.h"
							
_TRMEISTER_USING

namespace {

	const ModUnicodeChar _ucsEndOfString	= ModUnicodeCharTrait::null();
	const ModUnicodeChar _ucsSlash			= 0x002f;		// '/'
	const ModUnicodeChar _ucsBackSlash		= 0x005c;		// '\\'
#ifdef	OS_WINDOWSNT4_0
	const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::shiftJis;
#else
	const ModKanjiCode::KanjiCodeType	_eLiteralCode = ModKanjiCode::euc;
#endif
	const int _iStateCodeLength = 5;

	//
	//	FUNCTION local
	//	_GetBaseName -- ファイル名からディレクトリ部分を除く
	//
	//	NOTES
	//	ファイル名からディレクトリ部分を除く。
	//
	//	ARGUMENTS
	//	const ModUnicodeChar* pszSrcName_
	//		取除く対象の文字列
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		ディレクトリ等を除いた部分の先頭のポインタ
	//
	//	EXCEPTIONS
	//	なし
	//
	const ModUnicodeChar*
	_GetBaseName(const ModUnicodeChar* pszSrcName_)
	{
		const ModUnicodeChar* p = pszSrcName_;
		if (p)
		{
			const ModUnicodeChar* cp = 0;
			for (; *p != _ucsEndOfString; p++)
			{
				switch (*p)
				{
				case _ucsSlash: 		// '/'
				case _ucsBackSlash:	// '\\'
					cp = p;
					break;
				default:
					;
				}
			}
			if (cp)
			{
				p = cp+1;
			}
			else
			{
				p = pszSrcName_;
			}
		}
		return p;
	}
	
	//
	//	FUNCTION local
	//	_GetBaseName -- ファイル名からディレクトリ部分を除く
	//
	//	NOTES
	//	ファイル名からディレクトリ部分を除く。
	//
	//	ARGUMENTS
	//	const char* pszSrcName_
	//		取除く対象の文字列
	//
	//	RETURN
	//	const char*
	//		ディレクトリ等を除いた部分の先頭のポインタ
	//
	//	EXCEPTIONS
	//	なし
	//
	const char*
	_GetBaseName(const char* pszSrcName_)
	{
		const char* p = pszSrcName_;
		if (p)
		{
			const char* cp = 0;
			for (; *p != '\0'; p++)
			{
				switch (*p)
				{
				case '/':
				case '\\':
					cp = p;
					break;
				default:
					;
				}
			}
			if (cp)
			{
				p = cp+1;
			}
			else
			{
				p = pszSrcName_;
			}
		}
		return p;
	}

}

_TRMEISTER_EXCEPTION_USING

//
//	FUNCTION public
//	Exception::Object::Object -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
Object::
Object()
: m_uiErrorNumber(0)
{
}

//
//	FUNCTION public
//	Exception::Object::Object -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Exception::ErrorNumber::Type uiErrorNumber_
//		エラー番号
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
Object::
Object(ErrorNumber::Type uiErrorNumber_)
: m_uiErrorNumber(uiErrorNumber_)
{
}

//
//	FUNCTION public
//	Exception::Object::Object -- コンストラクタ(3)
//
//	NOTES
//	コンストラクタ。引数をデータメンバーに設定する。
//
//	ARGUMENTS
//	unsigned int uiErrorNumber_
//		エラー番号(メッセージ番号)
//	const char* pszModuleName_
//		モジュール名
//	const char* pszFileName_
//		例外が発生したファイル名
//	int iLineNumber_
//		例外が発生した場所
//	const char* pszStateCode_
//		SQLSTATEコード
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
Object::
Object(unsigned int uiErrorNumber_,
	   const char* pszModuleName_,
	   const char* pszFileName_, int iLineNumber_,
	   const char* pszStateCode_)
: m_uiErrorNumber(uiErrorNumber_), m_iLineNumber(iLineNumber_)
{
	//メッセージ引数を初期化する
	setErrorMessageArgument(&_ucsEndOfString);
	//モジュール名をコピーする
	ModUnicodeCharTrait::copy(m_pszModuleName, ModUnicodeString(pszModuleName_, _eLiteralCode));
	//ファイル名からディレクトリー部分を除く
	const char* pFname = _GetBaseName(pszFileName_);
	//ファイル名をコピーする
	ModUnicodeString strTmp(pFname, _eLiteralCode);
	ModUnicodeCharTrait::copy(m_pszFileName, strTmp);
	// STATEコードをコピーする
	ModCharTrait::copy(m_pszStateCode, pszStateCode_, _iStateCodeLength);
	m_pszStateCode[_iStateCodeLength] = '\0';
}

//
//	FUNCTION public
//	Exception::Object::Object -- コンストラクタ(4)
//
//	NOTES
//	コンストラクタ。引数をデータメンバーに設定する。
//
//	ARGUMENTS
//	unsigned int uiErrorNumber_
//		エラー番号(メッセージ番号)
//	const ModUnicodeChar* pszModuleName_
//		モジュール名
//	const ModUnicodeChar* pszFileName_
//		例外が発生したファイル名
//	int iLineNumber_
//		例外が発生した場所
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
Object::
Object(unsigned int uiErrorNumber_,
	   const ModUnicodeChar* pszModuleName_,
	   const ModUnicodeChar* pszFileName_, int iLineNumber_,
	   const char* pszStateCode_)
: m_uiErrorNumber(uiErrorNumber_), m_iLineNumber(iLineNumber_)
{
	//メッセージ引数を初期化する
	setErrorMessageArgument(&_ucsEndOfString);
	//モジュール名をコピーする
	ModUnicodeCharTrait::copy(m_pszModuleName, pszModuleName_);
	//ファイル名からディレクトリー部分を除く
	const ModUnicodeChar* pFname = _GetBaseName(pszModuleName_);
	//ファイル名をコピーする
	ModUnicodeCharTrait::copy(m_pszFileName, pFname);
	// STATEコードをコピーする
	ModCharTrait::copy(m_pszStateCode, pszStateCode_, _iStateCodeLength);
	m_pszStateCode[_iStateCodeLength] = '\0';
}

//
//	FUNCTION public
//	Exception::Object::Object -- コンストラクタ(5)
//
//	NOTES
//
//	ARGUMENTS
//	const Exception::Object& cObject_
//		例外クラス
//	const char* pszStateCode_
//		STATEコード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Object::Object(const Object& cObject_, const char* pszStateCode_)
	: m_uiErrorNumber(cObject_.m_uiErrorNumber),
	  m_iLineNumber(cObject_.m_iLineNumber)
{
	this->operator = (cObject_);
	ModOsDriver::Memory::copy(m_pszStateCode,
							  pszStateCode_,
							  sizeof(m_pszStateCode));
}

//
//	FUNCTION public
//	Exception::Object::serialize -- シリアル化を行う
//
//	NOTES
//	シリアル化を行う
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	OBJECTS
//	その他
//		下位の例外はそのまま再送
//
void
Object::
serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//
		//	書き込み
		//

		// エラー番号
		cArchiver_.writeArchive(m_uiErrorNumber);

		// 引数の数
		int iArgNumber = ErrorMessage::getArgumentNumber(m_uiErrorNumber);
		if (iArgNumber == -1) iArgNumber = 0;
		cArchiver_.writeArchive(iArgNumber);

		// 引数
		int iCount = 0;
		const ModUnicodeChar* p = m_pszErrorMessageArgument;
		for (int i = 0; i < iArgNumber; ++i)
		{
			ModUnicodeString cstrArgument = p;
			cArchiver_ << cstrArgument;
			while (*p++);
		}
		
		// モジュール名
		ModUnicodeString cstrData = m_pszModuleName;
		cArchiver_ << cstrData;

		// ファイル名
		cstrData = m_pszFileName;
		cArchiver_ << cstrData;
		
		// 行番号
		cArchiver_.writeArchive(m_iLineNumber);
	}
	else
	{
		//
		//	読み込み
		//

		// エラー番号
		cArchiver_.readArchive(m_uiErrorNumber);

		// 引数の数
		int iArgNumber;
		cArchiver_.readArchive(iArgNumber);

		// 引数
		ModUnicodeChar* p = m_pszErrorMessageArgument;
		for (int i = 0; i < iArgNumber; i++)
		{
			ModUnicodeString cstrData;
			cArchiver_ >> cstrData;
			ModUnicodeCharTrait::copy(p, cstrData);
			while (*p++);
		}

		// モジュール名
		ModUnicodeString cstrData;
		cArchiver_ >> cstrData;
		ModUnicodeCharTrait::copy(m_pszModuleName, cstrData);

		// ファイル名
		cArchiver_ >> cstrData;
		ModUnicodeCharTrait::copy(m_pszFileName, cstrData);

		// 行番号
		cArchiver_.readArchive(m_iLineNumber);
	}
}

//
//	FUNCTION protected
//	Exception::Object::setErrorMessageArgument
//								-- エラーメッセージ引数を設定する
//
//	NOTES
//	メッセージ引数をメンバー変数 m_pszErrorMessageArgument へコピーする。
//	引数とデータメンバーm_pszErrorMessageArgumentが同じアドレスの
//	場合は何もしない。
//
//	ARGUMENTS
//	const ModUnicodeChar* pszErrorMessageArgument_
//		メッセージ引数
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
void
Object::
setErrorMessageArgument(const ModUnicodeChar* pszErrorMessageArgument_)
{
	const ModUnicodeChar* p = pszErrorMessageArgument_;
	ModUnicodeChar* d = &m_pszErrorMessageArgument[0];
	if (p != d)
	{
		do
		{
			while(*p)
			{
				*d++ = *p++;
			}
			*d++ = *p++;
		}
		while (*p);
		*d = _ucsEndOfString;
	}
}

//	FUNCTION
//	Exception::throwClassInstance -- 例外クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Exception::Object& cObject_
//		基底クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
Exception::throwClassInstance(const Object& cObject_)
{
	switch(cObject_.getErrorNumber()) {
#include "Exception/ThrowClassInstance.h"
	default:
		throw UnknownException(cObject_.getModuleName(), cObject_.getFileName(),
							   cObject_.getLineNumber(), cObject_.getErrorNumber());
	}
}

//	FUNCTION
//	Exception::rethrowLog -- 例外再送のログを出力する
//
//	NOTES
//
//	ARGUMENTS
//	マクロ _TRMEISTER_RETHROW から呼び出される。
//	例外を再送したことをINFOログに出力する。
//
//	ARGUMENTS
//	const char* pszModuleName_
//		モジュール名
//	const char* pszFileName_
//		ファイル名
//	int iLineNumber_
//		行番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
void
Exception::rethrowLog(const char* pszModuleName_,
					  const char* pszFileName_,
					  int iLineNumber_)
{
	using namespace Common;
	
	MessageStreamSelection::getInstance(
		pszModuleName_,
		pszFileName_,
		iLineNumber_,
		"Common_MessageOutputInfo",
		MessageStreamBuffer::LEVEL_INFO) << "rethrow exception"
										 << ModEndl;
}

//
//	Copyright (c) 1999, 2000, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
