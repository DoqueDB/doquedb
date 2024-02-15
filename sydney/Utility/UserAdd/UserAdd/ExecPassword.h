// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecPasswd.h --
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

#ifndef __TRMEISTER_UserAdd_ExecPassword_H
#define __TRMEISTER_UserAdd_ExecPassword_H

#include "Common/Common.h"
#include "UserAdd/Exec.h"
#include "UserAdd/Option.h"
#include "Client2/Session.h"

#include <stdio.h>

_TRMEISTER_USING

//
//	CLASS
//	Passwd::ExecPassword --
//
//	NOTES
//
//
class ExecPassword : public Exec
{
public:
	//コンストラクタ
	ExecPassword(Client2::DataSource& cDataSource_, const Option& cOption_);
	//デストラクタ
	virtual ~ExecPassword();
	// プロンプトを表示する
	void prompt(Client2::Session* pSession_);

private:
	bool getUserName();
	bool getPassword(ModUnicodeString& cstrPassword_, const char* const pszPrompt_);
	bool isRootUser(Client2::Session* pSession_);

};

#endif //__TRMEISTER_UserAdd_ExecPassword_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
