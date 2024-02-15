// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecAdd.cpp --
// 
// Copyright (c) 2008, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "UserAdd";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "UserAdd/ExecAdd.h"
#include "Exception/Message.h"
#include "Exception/NumberDuplicateUser.h"
#include "Exception/NumberDuplicateUserID.h"
#include "Exception/NumberInvalidUserName.h"
#include "Exception/NumberUserIDOutOfRange.h"
#include "Common/DataArrayData.h"
#include "Common/UnicodeString.h"

#include "ModUnicodeOstrStream.h"

#ifdef SYD_OS_WINDOWS
#include <conio.h>
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <iostream>
#include <stdio.h>
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
//	UserAdd::ExecAdd::ExecAdd -- コンストラクタ
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
ExecAdd::ExecAdd(Client2::DataSource& cDataSource_, const Option& cOption_)
	: Exec(cDataSource_, cOption_)
{
}

//
//	FUNCTION public
//	UserAdd::ExecAdd::~ExecAdd -- デストラクタ
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
ExecAdd::~ExecAdd()
{
}

//
//	FUNCTION public
//	ExecAdd::prompt -- プロンプトを表示する
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
ExecAdd::prompt(Client2::Session* pSession_)
{
	for (;;) {
		try {
			while (getUserName() == false) {
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
			while (getAddUserID() == false ) {
				signal(SIGINT, _handler);
				if (_bCancel ) break;
			}
			signal(SIGINT, _handler);
			if (_bCancel ) break;
			// ユーザーを作成する
			pSession_->createUser(m_cstrUserName, "", m_iUserID);
			::fputs("Create a user.\n", stdout);
			break;
		} catch (Exception::Object& e) {
			
			switch (e.getErrorNumber()) {
			case Exception::ErrorNumber::DuplicateUser:
			case Exception::ErrorNumber::InvalidUserName:
				{
					ModUnicodeOstrStream stream;
						stream << e;
						cerr << "[ERROR] "
							 << (const char*)Exec::unicodeToMultiByte(stream.getString())
							 << endl;
					m_cstrUserName.clear();
					ModOsDriver::Thread::sleep(3000);
					break;
				}
			case Exception::ErrorNumber::DuplicateUserID:
			case Exception::ErrorNumber::UserIDOutOfRange:
				{
					ModUnicodeOstrStream stream;
						stream << e;
						cerr << "[ERROR] "
							 << (const char*)Exec::unicodeToMultiByte(stream.getString())
							 << endl;
					ModOsDriver::Thread::sleep(3000);
					m_iUserID = -1;
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
//	UserAdd::ExecAdd::getAddUserID -- ユーザーIDを入力してもらう
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
ExecAdd::
getAddUserID()
{
	bool bResult = true;
	if (m_iUserID == -1) {
		const char* const pszPrompt = "user id:";
		const char* const pszGetBack = "\b \b";
		::fputs(pszPrompt, stdout);
		char cUserID[256];
		memset(cUserID, '\0', 256);
		int i = 0;
		int c;
		while ((c = ::fgetc(stdin)) != EOF) {
			if (c == '\n' || c == '\r') {
				::fputc('\n', stdout);
				break;
			}
			else if (c >= 0x80) { // Non-ascii -> ignore
				continue;
			}
			else if (c == '\b' || c == 0x7f) {
				if ( strlen(cUserID) > 0) {
					cUserID[i] = '\0';
					i--;
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
				int n = strlen(cUserID);
				memset(cUserID, '\0', 256);
				while (n-- > 0) {
					::fputs(pszGetBack, stdout);
				}
				continue;
			}

			cUserID[i] = static_cast<char>(c);
			i++;
		}
		if ( strlen(cUserID) )
			m_iUserID = ModCharTrait::toInt(cUserID);

	}
	return bResult;
}
// FUNCTION public
//	UserAdd::ExecAdd::getUserName -- ユーザー名を入力してもらう
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
ExecAdd::
getUserName()
{
	bool bResult = true;
	if (m_cstrUserName.getLength() == 0) {
		const char* const pszPrompt = "add user:";
		const char* const pszGetBack = "\b \b";
		::fputs(pszPrompt, stdout);
		int c;
		while ((c = ::fgetc(stdin)) != EOF) {

			if (c == '\n' || c == '\r') {
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
			m_cstrUserName += static_cast<char>(c);
		}

	}
	return bResult;
}

//
//	Copyright (c) 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
