// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.cpp --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "TRBackup";
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

#include "TRBackup/Option.h"
#include "Common/UnicodeString.h"
#include "ModCharTrait.h"

#include "ModCharString.h"

using namespace std;

_TRMEISTER_USING

namespace
{
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
	: m_iPortNumber(0)
{
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
	ModCharString cstrHostName;
	ModCharString cstrUserName;
	ModCharString cstrPassword;
		
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
			if (ModCharTrait::compare(p, "database", ModFalse) == 0)
			{
				//データベース名
				if (i < argc)
					cstrDatabaseName = argv[i++];
				else
					return false;
			}
			else if (ModCharTrait::compare(p, "remote", ModFalse) == 0)
			{
				if (i < argc)
					cstrHostName = argv[i++];
				else
					return false;
				if (i < argc)
					m_iPortNumber = ModCharTrait::toInt(argv[i++]);
				else
					return false;
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

	m_cstrDatabaseName = ModUnicodeString(cstrDatabaseName);
	m_cstrHostName = ModUnicodeString(cstrHostName);
	m_cstrUserName = ModUnicodeString(cstrUserName);
	m_cstrPassword = ModUnicodeString(cstrPassword);

	return true;
}

//
//	Copyright (c) 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
