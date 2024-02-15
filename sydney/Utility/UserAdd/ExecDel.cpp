// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecDel.cpp --
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
const char moduleName[] = "UserDel";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "UserAdd/ExecDel.h"
#include "Common/UnicodeString.h"
#include "Exception/Message.h"
#include "Exception/NumberUserNotFound.h"
#include "Exception/NumberUserRequired.h"
#include "Common/DataArrayData.h"
#include "Communication/AuthorizeMode.h"
#include "ModUnicodeOstrStream.h"
#include "ModCharTrait.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModTime.h"


#ifdef SYD_OS_WINDOWS
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <iostream>
#include <signal.h>

using namespace std;

_TRMEISTER_USING

namespace {

	//
	//	VARIABLE local
	//
	bool _bCancel = false;

	//
	//	FUNCTION local
	//
	extern "C" void _handler(int sig)
	{
		_bCancel = true;
	}
}
namespace {
#ifdef SYD_OS_WINDOWS
	//
	//	LOCAL
	//	コードページ
	//
	unsigned int _codePage = CP_ACP;
#else
	// Encoding type
	ModKanjiCode::KanjiCodeType _encodingType =	Common::LiteralCode;
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
//	UserDel::ExecDel::ExecDel -- コンストラクタ
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
ExecDel::ExecDel(Client2::DataSource& cDataSource_, const Option& cOption_)
	: Exec(cDataSource_, cOption_)
{
}

//
//	FUNCTION public
//	UserDel::ExecDel::~ExecDel -- デストラクタ
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
ExecDel::~ExecDel()
{
}

//
//	FUNCTION public
//	ExecDel::prompt -- プロンプトを表示する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Session* pSession_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ExecDel::prompt(Client2::Session* pSession_)
{
	for (;;) {
		try {
			while (getUserName() == false) {
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;

			while (getRevoke() == false) {
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
			// ユーザーを削除する
			pSession_->dropUser(m_cstrUserName, m_iRevoke);
			::fputs("Delete a user.\n", stdout);
			break;
		} catch (Exception::Object& e) {
			
			switch (e.getErrorNumber()) {
			case Exception::ErrorNumber::UserNotFound:
			case Exception::ErrorNumber::UserRequired:
				{
					ModUnicodeOstrStream stream;
						stream << e;
						cerr << "[ERROR] "
							 << (const char*)Exec::unicodeToMultiByte(stream.getString())
							 << endl;
					m_cstrUserName.clear();
					m_iRevoke = 0;
					ModOsDriver::Thread::sleep(3000);
					break;
				}
			default:
				{
					throw;
				}
			}
		}
		signal(SIGINT, SIG_IGN);
	}
}
// FUNCTION private
//	UserDel::ExecDel::getRevoke -- すべてのDBに対してRevokeを行うかたずねる
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
ExecDel::
getRevoke()
{
	bool bResult = true;
	if (m_iRevoke == 0) {
		const char* const pszGetBack = "\b \b";
		ModUnicodeString cstrRevoke;
		int c;
		_Password::_disableEchoBack();
		while(cstrRevoke != 'y' && cstrRevoke !='Y' && cstrRevoke != 'n' && cstrRevoke != 'N') {
			cstrRevoke.clear();
			const char* const pszPrompt = "The authority of all DB is deleted automatically.\nOK? [y/n]:";
			::fputs(pszPrompt, stdout);
		while ((c = _Password::_getChar()) != EOF) {
				if (c == '\n' || c == '\r') {
					::fputc('\n', stdout);
					break;
				}
				else if (c >= 0x80) { // Non-ascii -> ignore
					continue;
				}
				else if (c == '\b' || c == 0x7f) {
					if ( cstrRevoke.getLength() > 0) {
						cstrRevoke.truncate(cstrRevoke.getTail() - 1);
						::fputs(pszGetBack, stdout);
					}
					continue;
				}
				else if (c == 0x03 || c == 0x04) { // Ctrl-C
					bResult = false;
					_bCancel = true;
					break;
				}
				else if (c == 0x15) { // Ctrl-U
					::fputs(pszGetBack, stdout);
					cstrRevoke.clear();
					continue;
				}
				else if (cstrRevoke.getLength() > 0 ) {
					continue;
				}
			::fputc(c, stdout);
			cstrRevoke += static_cast<char>(c);
			}
			if (_bCancel) break;
		}
		if ( cstrRevoke == 'y' || cstrRevoke == 'Y' )
			m_iRevoke = 1;
		_Password::_enableEchoBack();

	}
	return bResult;
}
// FUNCTION public
//	UserDel::ExecDel::getUserName -- ユーザー名を入力してもらう
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

ExecDel::
getUserName()
{
	bool bResult = true;
	if (m_cstrUserName.getLength() == 0) {
		const char* const pszPrompt = "delete user:";
		const char* const pszGetBack = "\b \b";
		::fputs(pszPrompt, stdout);
		_Password::_disableEchoBack();
		int c;
		while ((c = _Password::_getChar()) != EOF) {
			if (c == '\n' || c == '\r') {
				::fputc('\n', stdout);
				break;
			}
			else if (c >= 0x80) { // Non-ascii -> ignore
				continue;
			}
			else if (c == '\b' || c == 0x7f) {
				if ( m_cstrUserName.getLength() > 0) {
					m_cstrUserName.truncate(m_cstrUserName.getTail() - 1);
					::fputs(pszGetBack, stdout);
				}
				continue;
			}
			else if (c == 0x03 || c == 0x04) { // Ctrl-C
				bResult = false;
				_bCancel = true;
				break;
			}
			else if (c == 0x15) { // Ctrl-U
				int n = m_cstrUserName.getLength();
				m_cstrUserName.clear();
				while (n-- > 0) {
					::fputs(pszGetBack, stdout);
				}
				continue;
			}
			::fputc(c, stdout);
			m_cstrUserName += static_cast<char>(c);
		}
		_Password::_enableEchoBack();

	}
	return bResult;
}
//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
