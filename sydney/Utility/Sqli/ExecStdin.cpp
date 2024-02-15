// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecStdin.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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
#include "Sqli/ExecStdin.h"
#include "Common/UnicodeString.h"
#ifdef SYD_OS_WINDOWS
#include <io.h>
#else
#include <stdio.h>
#include <unistd.h>
#endif
#include <iostream>
using namespace std;

_TRMEISTER_USING

//
//	FUNCTION public
//	Sqli::ExecStdin::ExecStdin -- コンストラクタ
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
ExecStdin::ExecStdin(Client2::DataSource& cDataSource_, const Option& cOption_)
	: Exec(cDataSource_, cOption_), isStdin(-1)
{
}

//
//	FUNCTION public
//	Sqli::ExecStdin::~ExecStdin -- デストラクタ
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
ExecStdin::~ExecStdin()
{
}

//
//	FUNCTION public
//	ExecStdin::getNext -- 次のSQL文を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUncodeString& cstrSQL_
//
//	RETURN
//	bool
//		EOFではない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ExecStdin::getNext(ModUnicodeString& cstrSQL_)
{
	bool bLiteral = false;

	ModCharString cstrBuffer;
	int c;
	while ((c = ::fgetc(m_pFp)) != EOF)
	{
		if (c == '\n')
		{
			if (cstrBuffer.getLength() == 0)
			{
				prompt();
				continue;
			}
		}
		else if (c == ' ' || c == '\t' || c == '\r')
		{
			if (cstrBuffer.getLength() == 0) continue;
		}
		else if (c == '\'')
		{
			if (bLiteral == false) bLiteral = true;
			else bLiteral = false;
		}

		if (bLiteral == false && c == ';')
		{
			if (cstrBuffer.getLength())
				break;
		}
		else
		{
			cstrBuffer += (char)c;
		}
	}

	if (cstrBuffer.getLength())
	{
		cstrSQL_ = Exec::multiByteToUnicode(cstrBuffer);
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	ExecStdin::initialize -- 初期化を行う
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
ExecStdin::initialize()
{
	m_pFp = stdin;
}

//
//	FUNCTION public
//	ExecStdin::prompt -- プロンプトを表示する
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
//	なし
//
void
ExecStdin::prompt()
{
	if (isStdin == -1)
	{
		isStdin = 1;
#ifdef SYD_OS_WINDOWS
		if (_isatty(_fileno(m_pFp)) == false)
#else
		if (isatty(fileno(m_pFp)) == false)
#endif
			isStdin = 0;
	}

	if (isStdin)
		cout << "SQL>" << flush;
}

//
//	Copyright (c) 2002, 2003, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
