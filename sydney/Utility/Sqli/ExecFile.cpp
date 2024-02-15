// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecFile.cpp --
// 
// Copyright (c) 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
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
#include "Sqli/ExecFile.h"
#include "Exception/FileNotFound.h"
#include "Common/UnicodeString.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Sqli::ExecFile::ExecFile -- コンストラクタ
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
ExecFile::ExecFile(Client2::DataSource& cDataSource_, const Option& cOption_)
: ExecStdin(cDataSource_, cOption_), m_cstrFileName(cOption_.getFileName())
{
}

//
//	FUNCTION public
//	Sqli::ExecFile::~ExecFile -- デストラクタ
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
ExecFile::~ExecFile()
{
}

//
//	FUNCTION public
//	ExecFile::initialize -- ファイルをオープンする
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
ExecFile::initialize()
{
	m_pFp = ::fopen(m_cstrFileName, "rb");
	if (m_pFp == 0)
		_TRMEISTER_THROW1(Exception::FileNotFound,
					   Exec::multiByteToUnicode(m_cstrFileName));
}

//
//	FUNCTION public
//	ExecFile::terminate -- ファイルをクローズする
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
ExecFile::terminate()
{
	::fclose(m_pFp);
}

//
//	Copyright (c) 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
