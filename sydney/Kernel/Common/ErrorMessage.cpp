// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessage.cpp -- エラーメッセージを作成するクラス
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2011,2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ErrorMessage.h"
#include "Common/ErrorMessageManager.h"
#include "Common/UnicodeString.h"

#include "Exception/ErrorMessage.h"
#include "Exception/Object.h"
#include "Os/Library.h"
#include "Os/Unicode.h"

#ifndef SYD_DLL
#include "Message/Message.h"
#endif

#include "ModUnicodeCharTrait.h"
#include "ModOstrStream.h"

_TRMEISTER_USING

namespace {

//
//	VARIABLE local
//	pszFunctionName -- メッセージフォーマットを得る関数名
//
//	NOTES
//	メッセージフォーマットを得るライブラリの関数名
//
const ModUnicodeString& strFunctionName =
	_TRMEISTER_U_STRING("DBGetMessageFormat");

}

//
//	FUNCTION public
//	Common::ErrorMessage::ErrorMessage -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::ErrorMessage::ErrorMessage(const ModCharString & pszLibrary_)
: m_pGetFormatFunction(0)
{
	m_cstrLibraryName = "SyMes";
	m_cstrLibraryName += pszLibrary_;
}

//
//	FUNCTION public
//	Common::ErrorMessage::~ErrorMessage -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::ErrorMessage::~ErrorMessage()
{
}

//
//	FUNCTION public
//	Common::ErrorMessage::initialize -- 初期化を行う
//
//	NOTES
//	初期化を行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Common::ErrorMessage::initialize()
{
#ifdef SYD_DLL
	const Os::UnicodeString libName(m_cstrLibraryName);

	//ライブラリをロードする
	Os::Library::load(libName);

	//関数を設定する
	//	void* -> 関数へのキャストは
	//	static_cast も syd_reinterpret_cast もエラーになるので、
	//	旧スタイルのキャストを用いる。
	m_pGetFormatFunction = (const ModUnicodeChar*(*)(unsigned int))
		Os::Library::getFunction(libName, strFunctionName);
#else
	m_pGetFormatFunction = DBGetMessageFormat;
#endif
}

//
//	FUNCTION public
//	Common::ErrorMessage::terminate -- 終了処理を行う
//
//	NOTES
//	終了処理を行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Common::ErrorMessage::terminate()
{
}

//
//	FUNCTION public
//	Common::ErrorMessage::makeMessage -- エラーメッセージを作成する
//
//	NOTES
//	エラーメッセージを作成する。
//
//	ARGUMENTS
//	ModUnicodeChar* pszBuffer_
//		エラーメッセージを格納するバッファ
//	unsigned int uiMessageNumber_
//		エラーメッセージのメッセージ番号
//	const ModUnicodeChar* pszArgument_
//		エラーメッセージに埋め込む引数
//
//	RETURN
//	pszBuffer_の先頭のポインタ
//
//	EXCEPTIONS
//	なし
//
ModUnicodeChar*
Common::ErrorMessage::makeMessage(ModUnicodeChar* pszBuffer_,
								  unsigned int uiMessageNumber_,
								  const ModUnicodeChar* pszArgument_) const
{
	const ModUnicodeChar* p = getFormat(uiMessageNumber_);
	ModUnicodeChar* d = pszBuffer_;

	ModUnicodeChar chDelimt = UnicodeChar::usPercent;
	
	if (p)
	{
		while (*p)
		{
			if (*p == chDelimt)
			{
				p++;
				if (ModUnicodeCharTrait::isDigit(*p) == ModTrue)
				{
					//引数を取出す
					int n = ModUnicodeCharTrait::toInt(ModUnicodeString(*p)) - 1;
					const ModUnicodeChar* a =
						Exception::ErrorMessage::getMessageArgumentElement(
															pszArgument_, n);
					while (*a)
					{
						*d++ = *a++;
					}
					p++;
				}
				else
				{
					*d++ = *p++;
				}
			}
			else
			{
				*d++ = *p++;
			}
		}
	}
	*d = ModUnicodeCharTrait::null();
	
	return pszBuffer_;
}

//
//	FUNCTION private
//	Common::ErrorMessage::getFormat -- メッセージフォーマットを得る
//
//	NOTES
//	エラーメッセージフォーマットを得る。
//
//	ARGUMENTS
//	unsigned int iMessageNumber_
//		メッセージ番号
//
//	RETURN
//	const ModUnicodeChar*
//		メッセージフォーマット 得られない場合は0を返す
//
//	EXCEPTION
//	なし
//
const ModUnicodeChar*
Common::ErrorMessage::getFormat(unsigned int uiMessageNumber_) const
{
	const ModUnicodeChar* p = 0;
	if (m_pGetFormatFunction)
	{
		p = (*m_pGetFormatFunction)(uiMessageNumber_);
	}
	return p;
}

//
//	FUNCTION global
//	operator<< -- 例外をModMessageに書出す
//
//	NOTES
//	例外をModMessageに書出す(for Debug)
//
//	ARGUMENTS
//	ModMessageStream& cstream_
//		メッセージストリーム
//	Exception::Object& cObject_
//		例外クラス
//
//	RETURN
//	ModMessageStream&
//		メッセージストリームへの参照
//
//	OBJECTS
//	その他
//		下位の例外はそのまま再送
//
ModMessageStream&
operator<<(ModMessageStream& cStream_,
		   const Exception::Object& cObject_)
{
	// ModHexとかModDecとかはMessageStreamで使用してはいけない
	// よって、エラー番号を16進で出力するために、
	// 一時変数のModOstrStreamに書き出し、それをMessageStreamに書くことにする

	ModOstrStream o;
	o << ModHex << cObject_.getErrorNumber();
	
	ModUnicodeChar buf[2048];
	cStream_ << "Object No=0x"
			 << o.getString()
			 << " ("
			 << cObject_.getModuleName()
			 << "::"
			 << cObject_.getFileName()
			 << " "
			 << cObject_.getLineNumber()
			 << ") "
			 << Common::ErrorMessageManager::makeErrorMessage(
				 buf,
				 cObject_.getErrorNumber(),
				 cObject_.getErrorMessageArgument());

	return cStream_;
}

//
//	FUNCTION global
//	Exception::Object::operator<< -- 例外をModMessageに書出す
//
//	NOTES
//	例外をModMessageに書出す(for Debug)
//
//	ARGUMENTS
//	MessageStream& cstream_
//		メッセージストリーム
//	Exception::Object& cObject_
//		例外クラス
//
//	RETURN
//	MessageStream&
//		メッセージストリームへの参照
//
//	OBJECTS
//	その他
//		下位の例外はそのまま再送
//
ModOstream&
operator<<(ModOstream& cStream_,
		   const Exception::Object& cObject_)
{
	ModUnicodeChar buf[2048];
	cStream_ << "Object No=0x"
			 << ModHex
		     << cObject_.getErrorNumber()
			 << ModDec
			 << " ("
			 << cObject_.getModuleName()
			 << "::"
			 << cObject_.getFileName()
			 << " "
			 << cObject_.getLineNumber()
			 << ") "
			 << Common::ErrorMessageManager::makeErrorMessage(
				 buf,
				 cObject_.getErrorNumber(),
				 cObject_.getErrorMessageArgument());
	return cStream_;
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2011,2023 Ricoh Company, Ltd.
//	All rights reserved.
//
