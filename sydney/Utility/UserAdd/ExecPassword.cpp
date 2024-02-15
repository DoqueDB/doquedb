// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecPasswd.cpp --
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
const char moduleName[] = "ExecPassword";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "UserAdd/ExecPassword.h"
#include "Client2/ResultSet.h"
#include "Common/DataArrayData.h"
#include "Common/UnicodeString.h"

#include "Communication/AuthorizeMode.h"

#include "Exception/Message.h"
#include "Exception/NumberUserNotFound.h"
#include "Exception/NumberUserRequired.h"
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
//	Passwd::ExecPasswd::ExecDel -- コンストラクタ
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
ExecPassword::ExecPassword(Client2::DataSource& cDataSource_, const Option& cOption_)
	: Exec(cDataSource_, cOption_)
{
}

//
//	FUNCTION public
//	Passwd::ExecPasswd::~ExecPasswd -- デストラクタ
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
ExecPassword::~ExecPassword()
{
}

//
//	FUNCTION public
//	ExecPasswd::prompt -- プロンプトを表示する
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
ExecPassword::prompt(Client2::Session* pSession_)
{
	ModUnicodeString cstrNewPassword;
	ModUnicodeString cstrRetypeNewPassword;

	for (;;) {
		try {
			//スーパーユーザーの場合
			if ( isRootUser(pSession_) )
			{
				// 変更するユーザー名
				while (getUserName() == false) {
					signal(SIGINT, _handler);
					if ( _bCancel ) break;
				}
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
			// 新しいパスワード
			while (getPassword(cstrNewPassword, "New password:") == false) {
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
			// 確認のためもう一度
			while (getPassword(cstrRetypeNewPassword, "Retype new password:") == false) {
				signal(SIGINT, _handler);
				if ( _bCancel ) break;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
			if ( cstrNewPassword != cstrRetypeNewPassword )
			{
				cstrNewPassword.clear();
				cstrRetypeNewPassword.clear();
				::fputs("New passwords mismatch.\n", stdout);
				continue;
			}
			signal(SIGINT, _handler);
			if ( _bCancel ) break;
	
			// 自分のパスワードを変更
			if ( m_cstrUserName.getLength() == 0 || m_cstrUserName == m_cstrRootUserName )
				pSession_->changeOwnPassword(cstrNewPassword);
			// 他のユーザーのパスワードを変更
			else
				pSession_->changePassword(m_cstrUserName, cstrNewPassword);
			::fputs("Changed a password.\n", stdout);
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
					cstrNewPassword.clear();
					cstrRetypeNewPassword.clear();
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
//	Passwd::ExecPasswd::getPassword -- パスワードを入力してもらう
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeString *cstrPassword_
//  const char* const pszPrompt_
//
// RETURN
//	bool	true ... 正常終了
//
// EXCEPTIONS

bool
ExecPassword::
 getPassword(ModUnicodeString& cstrPassword_, const char* const pszPrompt_)
{
	bool bResult = true;
	if (cstrPassword_.getLength() == 0) {
		const char* const pszGetBack = "\b \b";
		::fputs(pszPrompt_, stdout);
		_Password::_disableEchoBack();
		int c;
		while ((c = _Password::_getChar()) != EOF) {

			if (c == '\n' || c == '\r') {
				::fputc('\n', stdout);
				break;
			}
			else if ((c == '\b' || c == 0x7f)
					 && cstrPassword_.getLength() > 0) {
				cstrPassword_.truncate(cstrPassword_.getTail() - 1);
				::fputs(pszGetBack, stdout);
				continue;
			}
			else if (c >= 0x80) { // Non-ascii -> ignore
				continue;
			}
			else if (c == 0x03 || c == 0x04) { // Ctrl-C
				bResult = false;
				_bCancel = true;
				break;
			}
			else if (c == 0x15) { // Ctrl-U
				int n = cstrPassword_.getLength();
				cstrPassword_.clear();
				while (n-- > 0) {
					::fputs(pszGetBack, stdout);
				}
				continue;
			}
			::fputc('*', stdout);
			cstrPassword_ += static_cast<char>(c);
		}
		_Password::_enableEchoBack();
	}
	return bResult;
}

// FUNCTION private
//	Passwd::ExecPasswd::getUserName -- ユーザー名を入力してもらう
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
ExecPassword::
getUserName()
{
	bool bResult = true;
	if (m_cstrUserName.getLength() == 0) {
		const char* const pszPrompt = "user name to change a password:";
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

// FUNCTION private
//	Passwd::ExecPasswd::isRootUser -- スーパーユーザーか調べる
//
// NOTES
//
// ARGUMENTS
//	Client2::Session* pSession_
//
// RETURN
//	bool	true ... スーパーユーザー
//
// EXCEPTIONS

bool
ExecPassword::
isRootUser(Client2::Session* pSession_)
{
	ModUnicodeString cstrSQL;
	Client2::ResultSet* pResultSet = 0;
	bool bType = false;
	/***
	try
	{
		cstrSQL = "select Type from SYSTEM_USER where Name='" + m_cstrRootUserName + "';";
		pSession_->executeStatement(cstrSQL);

		//実行結果を得る
		for (;;)
		{
			ModAutoPointer<Common::DataArrayData> pTuple = new Common::DataArrayData();
			Client2::ResultSet::Status::Value eStatus = pResultSet->getNextTuple(pTuple.get());

			if (eStatus == Client2::ResultSet::Status::Data)
			{
				//タプルデータ
				ModUnicodeString cstrTuple = pTuple->getString();

				const char *cType = (const char*)unicodeToMultiByte(cstrTuple);
				::fputs(cType, stdout);
				if ( !strcmp(cType, "0") )
					bType = true;
			}
			else if (eStatus == Client2::ResultSet::Status::EndOfData)
			{
			}
			else if (eStatus == Client2::ResultSet::Status::Success)
			{
				break;
			}
			else if (eStatus == Client2::ResultSet::Status::Canceled)
			{
				break;
			}
			else if (eStatus == Client2::ResultSet::Status::MetaData)
			{
			}
		}
	}
	catch (...)
	{
		pResultSet->close();
		pResultSet->release();
		throw;
	}
	pResultSet->close();
	pResultSet->release();
	***/
	if ( m_cstrRootUserName == "root" )
		bType = true;
	return bType;
}
//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
