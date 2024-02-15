// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecLine.h --
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

#ifndef __TRMEISTER_SQLI_EXECLINE_H
#define __TRMEISTER_SQLI_EXECLINE_H

#include "Common/Common.h"
#include "Sqli/Exec.h"
#include "ModUnicodeString.h"

_TRMEISTER_USING

//
//	CLASS
//	Sqli::ExecLine --
//
//	NOTES
//
//
class ExecLine : public Exec
{
public:
	//コンストラクタ
	ExecLine(Client2::DataSource& cDataSource_, const Option& cOption_);
	//デストラクタ
	virtual ~ExecLine();

	//次のSQL文を取り出す
	bool getNext(ModUnicodeString& cstrSQL_);

private:
	//SQL文
	ModUnicodeString m_cstrStatement;
	//カウンタ
	int m_iCounter;
};

#endif //__TRMEISTER_SQLI_EXECLINE_H

//
//	Copyright (c) 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
