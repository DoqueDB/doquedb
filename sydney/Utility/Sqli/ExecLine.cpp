// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecLine.cpp --
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
#include "Sqli/ExecLine.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	Sqli::ExecLine::ExecLine -- コンストラクタ
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
ExecLine::ExecLine(Client2::DataSource& cDataSource_, const Option& cOption_)
: Exec(cDataSource_, cOption_), m_cstrStatement(cOption_.getCommandLine()), m_iCounter(0)
{
}

//
//	FUNCTION public
//	Sqli::ExecLine::~ExecLine -- デストラクタ
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
ExecLine::~ExecLine()
{
}

//
//	FUNCTION public
//	ExecLine::getNext -- 次のSQL文を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrSQL_
//
//	RETURN
//	bool
//		EOFではない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ExecLine::getNext(ModUnicodeString& cstrSQL_)
{
	if (m_iCounter++ == 0)
	{
		cstrSQL_ = m_cstrStatement;
		return true;
	}
	return false;
}

//
//	Copyright (c) 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
