// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- 管理マネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_MANAGER_H
#define __SYDNEY_ADMIN_MANAGER_H

#include "Admin/Module.h"

#include "Schema/Object.h"

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

namespace Manager
{
	// 初期化
	void initialize();

	// 終了処理
	void terminate();
	
	// 管理者用の SQL 文を実際に実行する
	SYD_ADMIN_FUNCTION
	bool
	executeStatement(Trans::Transaction& trans,
					 Server::Session* pSession_,
					 const Statement::Object& stmt,
					 Communication::Connection& connection);
}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif //__SYDNEY_ADMIN_MANAGER_H

//
//	Copyright (c) 2001, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
