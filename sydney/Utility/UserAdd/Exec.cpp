// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exec.cpp --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Exec";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#ifdef SYD_OS_WINDOWS
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include "UserAdd/Exec.h"
#include "Client2/Session.h"
#include "Common/DataArrayData.h"
#include "Common/UnicodeString.h"

#include "Communication/AuthorizeMode.h"

#include "Exception/Message.h"
#include "Exception/NumberAuthorizationFailed.h"
#include "Exception/NumberUserNotFound.h"
#include "Exception/NumberUserRequired.h"


#include "ModUnicodeOstrStream.h"
#include "ModCharTrait.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModTime.h"

using namespace std;

_TRMEISTER_USING

namespace {

#ifdef SYD_OS_WINDOWS
	//
	//	LOCAL
	//	コードページ
	//
	unsigned int _codePage = CP_ACP;
#else
	// Encoding type
	ModKanjiCode::KanjiCodeType _encodingType =
									Common::LiteralCode;
#endif

	// エコーバックするか
	bool _doEcho = false;

	namespace _Password
	{
		// retry count
		const int _iRetryMax = 3;
#ifndef SYD_OS_WINDOWS
		struct termios _cTCSave;
#endif

		// disable echo back
		void _disableEchoBack()
		{
#ifdef SYD_OS_WINDOWS
			; // do nothing
#else
			struct termios cTmp;
			int iStdinFileNo = ::fileno(stdin);
			::tcgetattr(iStdinFileNo, &_cTCSave);
			cTmp = _cTCSave;
			cTmp.c_lflag &= ~(ECHO | ISIG | ICANON); // disable echo
			cTmp.c_cc[VMIN] = 1;
			cTmp.c_cc[VTIME] = 0;
			::tcsetattr(iStdinFileNo, TCSANOW, &cTmp);
#endif
		}

		// revert termio setting
		void _enableEchoBack()
		{
#ifdef SYD_OS_WINDOWS
			; // do nothing
#else
			::tcsetattr(::fileno(stdin), TCSANOW, &_cTCSave);
#endif
		}

		// read one character
		int _getChar()
		{
#ifdef SYD_OS_WINDOWS
			return ::_getch();
#else
			return ::fgetc(stdin);
#endif
		}
	}

	// UserAdd の終了ステータス
	int _exitStatus = 0;

}
//
//	FUNCTION public
//	Exec::Exec -- コンストラクタ
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
//	EXCEPTIONS
//	なし
//
Exec::Exec(Client2::DataSource& cDataSource_, const Option& cOption_)
: m_cDataSource(cDataSource_)
{
	m_bPasswordSpecified = cOption_.isPasswordSpecified();
	m_cstrRootUserName = cOption_.getRootUserName();
	m_cstrRootUserPassword = cOption_.getRootUserPassword();
	m_cstrUserName = cOption_.getUserName();
	m_iUserID = cOption_.getUserID();
	m_iRevoke = cOption_.getRevoke();
}

//
//	FUNCTION public
//	Exec::~Exec -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
Exec::~Exec()
{
}

//
//	FUNCTION public
//	Exec::execute -- 実行する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Exec::execute()
{
	//セッションを作成する
		Client2::Session* pSession = createSession();
	if (pSession == 0) {
		return;
	}
	try 
	{
		// prompt表示
		prompt(pSession);
	}
	catch (...)
	{
		pSession->close();
		pSession->release();
		throw;
	}

	pSession->close();
	pSession->release();
}

//
//	FUNCTION public
//	Exec::prompt -- プロンプトを表示する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Session* pSession
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Exec::prompt(Client2::Session* pSession)
{
}

//	FUNCTION public static
//	Exec::setEcho -- エコーバックするか設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			true
//				エコーバックする
//			false
//				エコーバックしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Exec::setEcho(bool v)
{
	_doEcho = v;
}

// FUNCTION private
//	Exec::createSession -- Sessionを新たに作る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Client2::Session*
//
// EXCEPTIONS

Client2::Session*
Exec::
createSession()
{
	if (m_cDataSource.getAuthorization() == Communication::AuthorizeMode::None) {
		// connecting to old version server
		return m_cDataSource.createSession("$$SystemDB");
	}

	// retry until valid user and password is specified
	int iRetry = (m_cstrRootUserName.getLength() > 0 && m_cstrRootUserPassword.getLength() > 0)
		? 1
		: _Password::_iRetryMax;

	Client2::Session* pSession = 0;
	for (;;) {
		try {
			while (getRootUserName() == false) {
				if (--iRetry > 0) {
					continue;
				}
				break;
			}
			if (getRootPassword() == false) break;
			pSession = m_cDataSource.createSession("$$SystemDB", m_cstrRootUserName, m_cstrRootUserPassword);
			break;
		} catch (Exception::Object& e) {
			switch (e.getErrorNumber()) {
			case Exception::ErrorNumber::AuthorizationFailed:
			case Exception::ErrorNumber::UserNotFound:
			case Exception::ErrorNumber::UserRequired:
				{
					ModOsDriver::Thread::sleep(3000);
					if (--iRetry > 0) {
						m_cstrRootUserName.clear();
						m_cstrRootUserPassword.clear();
						m_bPasswordSpecified = false;
						break;
					}
					throw;
				}
			default:
				{
					throw;
				}
			}
		}
	}
	return pSession;
}

// FUNCTION public
//	Exec::getUserName -- ユーザー名を入力してもらう
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool	true ... 正常終了
//
// EXCEPTIONS

bool
Exec::
getUserName()
{
	return true;
}

// FUNCTION private
//	Exec::getRootUserName -- スーパーユーザー名を入力してもらう
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool	true ... 正常終了
//
// EXCEPTIONS

bool
Exec::
getRootUserName()
{
	if (m_cstrRootUserName.getLength() == 0) {
		const char* const pszPrompt = "User:";
		::fputs(pszPrompt, stdout);
		int c;
		while ((c = ::fgetc(stdin)) != EOF) {
			if (c == '\n' || c == '\r') break;
			m_cstrRootUserName += static_cast<char>(c);
		}
		return m_cstrRootUserName.getLength() > 0;
	}
	return true;
}
// FUNCTION private
//	Exec::getRootPassword -- パスワードを入力してもらう
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool	true ... 正常終了
//
// EXCEPTIONS

bool
Exec::
getRootPassword()
{
	bool bResult = true;
	if (!m_bPasswordSpecified && m_cstrPassword.getLength() == 0) {
		const char* const pszPrompt = "Password:";
		const char* const pszGetBack = "\b \b";
		::fputs(pszPrompt, stdout);

		_Password::_disableEchoBack();
		int c;
		while ((c = _Password::_getChar()) != EOF) {
			if (c == '\n' || c == '\r') {
				::fputc('\n', stdout);
				break;
			}
			else if ((c == '\b' || c == 0x7f)
					 && m_cstrRootUserPassword.getLength() > 0) {
				m_cstrRootUserPassword.truncate(m_cstrRootUserPassword.getTail() - 1);
				::fputs(pszGetBack, stdout);
				continue;
			}
			else if (c >= 0x80) { // Non-ascii -> ignore
				continue;
			}
			else if (c == 0x03) { // Ctrl-C
				bResult = false;
				break;
			}
			else if (c == 0x15) { // Ctrl-U
				int n = m_cstrRootUserPassword.getLength();
				m_cstrPassword.clear();
				while (n-- > 0) {
					::fputs(pszGetBack, stdout);
				}
				continue;
			}
			::fputc('*', stdout);
			m_cstrRootUserPassword += static_cast<char>(c);
		}
		_Password::_enableEchoBack();
	}
	return bResult;
}

//
//	FUNCTION public static
//	Exec::getExitStatus -- adduserの終了ステータスを得る
//
int
Exec::getExitStatus()
{
	return _exitStatus;
}

//
//	FUNCTION pubklic static
//	Exec::setExitStatus -- adduserの終了ステータスを設定する
//
void
Exec::setExitStatus(int status_)
{
	_exitStatus = status_;
}

//
//	FUNCTION public static
//	Exec::unicodeToMultiByte -- Unicode文字でつをマルチバイト文字列に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrBuffer_
//		変換するUnicode文字列
//
//	RETURN
//	ModCharString
//		変換したマルチバイト文字列
//
//	EXCEPTIONS
//
ModCharString
Exec::unicodeToMultiByte(const ModUnicodeString& cstrBuffer_)
{
#ifdef SYD_OS_WINDOWS
	int len = ::WideCharToMultiByte(_codePage, 0, cstrBuffer_, -1, 0, 0, 0, 0);
	char* p = new char[len];
	::WideCharToMultiByte(_codePage, 0, cstrBuffer_, -1, p, len, 0, 0);
	ModCharString str(p);
	delete [] p;
	return str;
#else
	ModUnicodeString tmp(cstrBuffer_);
	return ModCharString(tmp.getString(_encodingType));
#endif
}

//
//	FUNCTION public static
//	Exec::multiByteToUnicode -- マルチバイト文字列をUnicode文字列に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const char* pszBuffer_
//		変換するマルチバイト文字列
//
//	RETURN
//	ModUnicodeString
//		変換したUnicode文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Exec::multiByteToUnicode(const char* pszBuffer_)
{
#ifdef SYD_OS_WINDOWS
	int len = ::MultiByteToWideChar(_codePage, 0, pszBuffer_, -1, 0, 0);
	ModUnicodeChar* p = new ModUnicodeChar[len];
	::MultiByteToWideChar(_codePage, 0, pszBuffer_, -1, p, len*sizeof(ModUnicodeChar));
	ModUnicodeString str(p);
	delete [] p;
	return str;
#else
	return ModUnicodeString(pszBuffer_, _encodingType);
#endif
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
