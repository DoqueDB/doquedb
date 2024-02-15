// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExecDel.h --
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

#ifndef __TRMEISTER_UserAdd_ExecDel_H
#define __TRMEISTER_UserAdd_ExecDel_H

#include "Common/Common.h"
#include "UserAdd/Exec.h"
#include "UserAdd/Option.h"
#include "Client2/Session.h"
#include <stdio.h>

_TRMEISTER_USING

//
//	CLASS
//	UserDel::ExecDel --
//
//	NOTES
//
//
class ExecDel : public Exec
{
public:
	//コンストラクタ
	ExecDel(Client2::DataSource& cDataSource_, const Option& cOption_);
	//デストラクタ
	virtual ~ExecDel();

	// プロンプトを表示する
	void prompt(Client2::Session* pSession_);

	// 権限を削除してまわるか聞く
	bool getRevoke();
	bool getUserName();
protected:
};

#endif //__TRMEISTER_UserAdd_ExecDel_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
