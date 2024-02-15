// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation.h --	操作が可能かチェックするための定義
// 
// Copyright (c) 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SCHEMA_OPERATION_H
#define __SYDNEY_SCHEMA_OPERATION_H

#include "Schema/Module.h"
#include "Server/Worker.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}
namespace Schema
{
	class Database;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class Object;
}

_SYDNEY_SCHEMA_BEGIN

namespace Operation {

//	FUNCTION static
//	Schema::Operation::isApplicatble --
//		操作が可能かチェックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		const Schema::Database& cDatabase_
//			操作対象となるデータベースを表すオブジェクト
//		const Statement::Object* pStatement_
//			操作をするSQL文を表すオブジェクト
//		Server::Session* pSession_
//			セッション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
isApplicable(Trans::Transaction& cTrans_,
			 const Schema::Database& cDatabase_,
			 const Statement::Object* pStatement_,
			 Server::Session* pSession_)
{
	Server::Worker::isOperationApplicable(cTrans_, cDatabase_, pStatement_, pSession_);
}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_OPERATION_H

//
//	Copyright (c) 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
