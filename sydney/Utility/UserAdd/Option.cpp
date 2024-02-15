// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.cpp --
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
const char moduleName[] = "Option";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "UserAdd/Option.h"
#include "UserAdd/Exec.h"
#include "Common/UnicodeString.h"
#include "ModCharTrait.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING

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
{
	// 暗号モード初期化(暗号化対応)
	m_eCryptMode = Communication::CryptMode::Unknown;
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
#ifdef SYD_OS_WINDOWS
	ModCharString cstrParentRegPath("HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister");
#else
	ModCharString cstrParentRegPath;
#endif

	ModCharString cstrHostName;
	ModCharString cstrRootUserName;
	ModCharString cstrRootUserPassword;
	ModCharString cstrUserName;
	ModVector<ModPair<ModCharString, ModCharString> > vecReplace;
		
	int i = 1;
	bool bRemote = false;
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
			m_iUserID = -1;
			m_iRevoke = 0;
			if (ModCharTrait::compare(p, "remote", ModFalse) == 0)
			{
				if (i < argc)
					cstrHostName = argv[i++];
				else
					return false;
				if (i < argc)
					m_iPortNumber = ModCharTrait::toInt(argv[i++]);
				else
					return false;
				bRemote = true;
			}
			else if (ModCharTrait::compare(p, "rootname", ModFalse) == 0)
			{
				//スーパーユーザー名
				if (i < argc)
				{
					cstrRootUserName = argv[i++];
				}
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "rootpw", ModFalse) == 0)
			{
				// スーパーユーザパスワード
				if (i < argc)
				{
					cstrRootUserPassword = argv[i++];
				}
				else
					return false;
				m_bPasswordSpecified = true;
			}
			else if (ModCharTrait::compare(p, "user", ModFalse) == 0)
			{
				//ユーザー名
				if (i < argc)
				{
					cstrRootUserName = argv[i++];
				}
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "password", ModFalse) == 0)
			{
				// パスワード
				if (i < argc)
				{
					cstrRootUserPassword = argv[i++];
				}
				else
					return false;
					m_bPasswordSpecified = true;
			}
			else if (ModCharTrait::compare(p, "username", ModFalse) == 0)
			{
				//新規ユーザ名
				if (i < argc)
					cstrUserName = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "chusername", ModFalse) == 0)
			{
				//パスワード変更ユーザ名
				if (i < argc)
					cstrUserName = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "userid", ModFalse) == 0)
			{
				//新規ユーザーID
				if (i < argc)
					m_iUserID = ModCharTrait::toInt(argv[i++]);
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "crypt", ModFalse) == 0)
			{
				return false;
			}
			else if (ModCharTrait::compare(p, "reg", ModFalse) == 0)
			{
				if (i < argc)
					cstrParentRegPath = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "revoke", ModFalse) == 0 )
			{
				m_iRevoke = 1;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	m_cstrParentRegPath = Exec::multiByteToUnicode(cstrParentRegPath);
	m_cstrHostName = Exec::multiByteToUnicode(cstrHostName);
	m_cstrRootUserName = Exec::multiByteToUnicode(cstrRootUserName);
	m_cstrRootUserPassword = Exec::multiByteToUnicode(cstrRootUserPassword);
	m_cstrUserName = Exec::multiByteToUnicode(cstrUserName);

	return bRemote;
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
