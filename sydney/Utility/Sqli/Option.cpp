// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2011, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Sqli";
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

#include "Sqli/Option.h"
#include "Sqli/Exec.h"
#include "Common/UnicodeString.h"
#include "ModCharTrait.h"

using namespace std;

_TRMEISTER_USING

namespace
{
	namespace _Password
	{
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
}

//
//	FUNCTION public
//	Option::Option -- コンストラクタ
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
Option::Option()
	: m_eTarget(Target::Stdin), m_eLocation(Location::Unknown),
	  m_bShutdown(false), m_bVersion(false),
	  m_eFamily(Client2::DataSource::Family::Unspec)
{
	// 暗号モード初期化(暗号化対応)
	m_eCryptMode = Communication::CryptMode::Unknown;
	// プロトコル初期値
	m_iProtocol = Client2::DataSource::Protocol::CurrentVersion;
	
	m_bPasswordSpecified = false;
}

//
//	FUNCTION public
//	Option::~Option -- デストラクタ
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
Option::~Option()
{
}

//
//	FUNCTION public
//	Option::set -- 引数を設定し、解析する
//
//	NOTES
//
//	ARGUMENTS
//	int argc
//		引数の数
//	char* argv[]
//		引数の配列
//
//	RETURN
//	bool
//		正しい引数ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Option::set(int argc, char* argv[])
{
	ModCharString cstrDatabaseName;
#ifdef SYD_OS_WINDOWS
	// [NOTE] 2007/07/25よりレジストリのパスを指すようになった
	//  参照 Common::SystemParameter::setParentPath
	ModCharString cstrParentRegPath("HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister");
#else
	ModCharString cstrParentRegPath;
#endif
	ModCharString cstrHostName;
	ModCharString cstrCommandLine;
	ModCharString cstrUserName;
	ModCharString cstrPassword;
	ModVector<ModPair<ModCharString, ModCharString> > vecReplace;
		
	int i = 1;
	while (i < argc)
	{
		char* p = argv[i++];
#ifdef SYD_OS_WINDOWS
		if (*p == '/' || *p == '-')
#else
		if (*p == '-')
#endif
		{
			//オプション
			p++;
#ifdef SYD_SQLI_INPROCESS
			if (ModCharTrait::compare(p, "install", ModFalse) == 0)
			{
				//インストール
				m_eLocation = Location::Install;
			}
			else
#else
			if (ModCharTrait::compare(p, "database", ModFalse) == 0)
#endif
			{
				//データベース名
				if (i < argc)
					cstrDatabaseName = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "remote", ModFalse) == 0)
			{
				if (m_eLocation != Location::Unknown) return false;
				
				//リモートサーバ
				m_eLocation = Location::Remote;

				if (i < argc)
					cstrHostName = argv[i++];
				else
					return false;
				if (i < argc)
					m_iPortNumber = ModCharTrait::toInt(argv[i++]);
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "sql", ModFalse) == 0)
			{
				//コマンドライン
				if (m_eTarget != Target::Stdin) return false;
				m_eTarget = Target::Line;

				if (i < argc)
				{
					cstrCommandLine = argv[i++];
				}
				else
					return false;
			}
#ifdef SYD_SQLI_INPROCESS
			else if (ModCharTrait::compare(p, "inprocess", ModFalse) == 0)
			{
				if (m_eLocation != Location::Unknown) return false;

				// インプロセスサーバ
				m_eLocation = Location::InProcess;
			}
#endif
			else if (ModCharTrait::compare(p, "shutdown", ModFalse) == 0)
			{
				m_bShutdown = true;
			}
			else if (ModCharTrait::compare(p, "version", ModFalse) == 0)
			{
				m_bVersion = true;
			}
			else if (ModCharTrait::compare(p, "replace", ModFalse) == 0)
			{
				ModPair<ModCharString, ModCharString> p;
				if (i < argc)
				{
					p.first = argv[i++];
					if (i < argc)
					{
						p.second = argv[i++];
						vecReplace.pushBack(p);
					}
					else
						return false;
				}
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "reg", ModFalse) == 0)
			{
				// [NOTE] 2007/07/25よりレジストリのパスを指さすようになった
				//  参照 Common::SystemParameter::setParentPath
				if (i < argc)
					cstrParentRegPath = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "code", ModFalse) == 0)
			{
				if (i < argc)
					m_cstrCode = argv[i++];
				else
					return false;
				if (Exec::setCode(m_cstrCode) == false)
				{
					cout << "Error: code '" << m_cstrCode << "' is not supported." << endl;
					return false;
				}
			}
			else if (ModCharTrait::compare(p, "crypt", ModFalse) == 0)
			{
				return false;
			}
			else if (ModCharTrait::compare(p, "protocol", ModFalse) == 0)
			{
				// プロトコルバージョン
				if (i < argc)
					m_iProtocol = (Client2::DataSource::Protocol::Value)ModCharTrait::toInt(argv[i++]);
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "echo", ModFalse) == 0)
			{
				Exec::setEcho(true);
			}
			else if (ModCharTrait::compare(p, "user", ModFalse) == 0)
			{
				//user name
				if (i < argc)
					cstrUserName = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "password", ModFalse) == 0)
			{
				//password
				if (i < argc)
					cstrPassword = argv[i++];
				else
					return false;
				m_bPasswordSpecified = true;
			}
			else if (ModCharTrait::compare(p, "time", ModFalse) == 0)
			{
				Exec::setTime(true);
			}
			else if (ModCharTrait::compare(p, "ipv4", ModFalse) == 0)
			{
				m_eFamily = Client2::DataSource::Family::IPv4;
			}
			else if (ModCharTrait::compare(p, "ipv6", ModFalse) == 0)
			{
				m_eFamily = Client2::DataSource::Family::IPv6;
			}
			else
			{
				return false;
			}
		}
		else
		{

			if (m_eTarget == Target::Line)
			{
				cstrCommandLine += " ";
				cstrCommandLine += p;
				continue;
			}

			// ファイル名
			if (m_eTarget != Target::Stdin) return false;
			m_eTarget = Target::File;

			m_cstrFileName = p;
		}
	}

	if (m_eLocation == Location::Unknown)
	{
		// エラー
		return false;
	}

	if (m_bShutdown == true && m_eLocation != Location::Remote)
	{
		//エラー
		return false;
	}

	m_cstrDatabaseName = Exec::multiByteToUnicode(cstrDatabaseName);
	// [NOTE] 2007/07/25よりレジストリのパスを指さすようになった
	//  参照 Common::SystemParameter::setParentPath
	m_cstrParentRegPath = Exec::multiByteToUnicode(cstrParentRegPath);
	m_cstrHostName = Exec::multiByteToUnicode(cstrHostName);
	m_cstrCommandLine = Exec::multiByteToUnicode(cstrCommandLine);
	m_cstrUserName = Exec::multiByteToUnicode(cstrUserName);
	m_cstrPassword = Exec::multiByteToUnicode(cstrPassword);
	for (int j = 0; j < static_cast<int>(vecReplace.getSize()); ++j)
	{
		ModPair<ModUnicodeString, ModUnicodeString> p;
		p.first = Exec::multiByteToUnicode(vecReplace[j].first);
		p.second = Exec::multiByteToUnicode(vecReplace[j].second);
		m_vecReplaceString.pushBack(p);
	}

	return true;
}

// FUNCTION public
//	Sqli::Option::inputUserName -- ユーザー名を入力してもらう
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Option::
inputUserName() const
{
	if (m_cstrUserName.getLength() == 0) {
		Option* pThis = const_cast<Option*>(this);
		const char* const pszPrompt = "User:";
		::fputs(pszPrompt, stdout);
		int c;
		while ((c = ::fgetc(stdin)) != EOF) {
			if (c == '\n' || c == '\r') break;
			pThis->m_cstrUserName += static_cast<char>(c);
		}
		return m_cstrUserName.getLength() > 0;
	}
	return true;
}

// FUNCTION public
//	Sqli::Option::inputPassword -- パスワードを入力してもらう
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Option::
inputPassword() const
{
	Option* pThis = const_cast<Option*>(this);
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
					 && m_cstrPassword.getLength() > 0) {
				pThis->m_cstrPassword.truncate(m_cstrPassword.getTail() - 1);
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
				int n = m_cstrPassword.getLength();
				pThis->m_cstrPassword.clear();
				while (n-- > 0) {
					::fputs(pszGetBack, stdout);
				}
				continue;
			}
			::fputc('*', stdout);
			pThis->m_cstrPassword += static_cast<char>(c);
		}
		_Password::_enableEchoBack();
	}
	return bResult;
}

// FUNCTION public
//	Sqli::Option::clearUserPassword -- ユーザー名とパスワードの指定をクリアする
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

void
Option::
clearUserPassword() const
{
	Option* pThis = const_cast<Option*>(this);
	pThis->m_cstrUserName.clear();
	pThis->m_cstrPassword.clear();
	pThis->m_bPasswordSpecified = false;
}

//
//	Copyright (c) 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
