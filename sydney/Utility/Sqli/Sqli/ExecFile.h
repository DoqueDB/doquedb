// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecFile.h --
// 
// Copyright (c) 2002, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_SQLI_EXECFILE_H
#define __TRMEISTER_SQLI_EXECFILE_H

#include "Common/Common.h"
#include "Sqli/ExecStdin.h"
#include "ModCharString.h"

_TRMEISTER_USING

//
//	CLASS
//	Sqli::ExecFile --
//
//	NOTES
//
//
class ExecFile : public ExecStdin
{
public:
	//コンストラクタ
	ExecFile(Client2::DataSource& cDataSource_, const Option& cOption_);
	//デストラクタ
	virtual ~ExecFile();

	//初期化
	void initialize();
	//終了処理
	void terminate();

	// プロンプトを表示する
	void prompt() {}

private:
	//ファイル名
	ModCharString m_cstrFileName;
};

#endif //__TRMEISTER_SQLI_EXECFILE_H

//
//	Copyright (c) 2002, 2004, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
