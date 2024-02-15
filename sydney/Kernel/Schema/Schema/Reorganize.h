// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Reorganize.h -- Managerに属する関数のうち再構成関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2005, 2006, 2007, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_H
#define	__SYDNEY_SCHEMA_REORGANIZE_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Object.h"

#include "Lock/Mode.h"
#include "Lock/Duration.h"

_SYDNEY_BEGIN

namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
	namespace Log
	{
		class File;
	}
}

_SYDNEY_SCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeUtility
		{
			Result::Value createDatabase(Trans::Transaction& cTrans_,
										 Server::Session* pSession_,
										 const Schema::Object::Name& cDatabase_,
										 const Statement::Object* pStatement_);
			Result::Value dropDatabase(Trans::Transaction& cTrans_,
									   Server::Session* pSession_,
									   const Schema::Object::Name& cDatabase_,
									   const Statement::Object* pStatement_);
			Result::Value moveDatabase(Trans::Transaction& cTrans_,
									   Server::Session* pSession_,
									   const Schema::Object::Name& cDatabase_,
									   const Statement::Object* pStatement_);
			Result::Value alterDatabase(Trans::Transaction& cTrans_,
										Server::Session* pSession_,
										const Schema::Object::Name& cDatabase_,
										const Statement::Object* pStatement_);

			Result::Value createArea(Trans::Transaction& cTrans_,
									 Server::Session* pSession_,
									 const Schema::Object::Name& cDatabase_,
									 const Statement::Object* pStatement_);
			Result::Value dropArea(Trans::Transaction& cTrans_,
								   Server::Session* pSession_,
								   const Schema::Object::Name& cDatabase_,
								   const Statement::Object* pStatement_);
			Result::Value alterArea(Trans::Transaction& cTrans_,
									Server::Session* pSession_,
									const Schema::Object::Name& cDatabase_,
									const Statement::Object* pStatement_);

			Result::Value createTable(Trans::Transaction& cTrans_,
									  Server::Session* pSession_,
									  const Schema::Object::Name& cDatabase_,
									  const Statement::Object* pStatement_);
			Result::Value dropTable(Trans::Transaction& cTrans_,
									Server::Session* pSession_,
									const Schema::Object::Name& cDatabase_,
									const Statement::Object* pStatement_);
			Result::Value alterTable(Trans::Transaction& cTrans_,
									 Server::Session* pSession_,
									 const Schema::Object::Name& cDatabase_,
									 const Statement::Object* pStatement_);
			Result::Value createIndex(Trans::Transaction& cTrans_,
									  Server::Session* pSession_,
									  const Schema::Object::Name& cDatabase_,
									  const Statement::Object* pStatement_);
			Result::Value dropIndex(Trans::Transaction& cTrans_,
									Server::Session* pSession_,
									const Schema::Object::Name& cDatabase_,
									const Statement::Object* pStatement_);
			Result::Value alterIndex(Trans::Transaction& cTrans_,
									 Server::Session* pSession_,
									 const Schema::Object::Name& cDatabase_,
									 const Statement::Object* pStatement_);

			Result::Value grant(Trans::Transaction& cTrans_,
								Server::Session* pSession_,
								const Schema::Object::Name& cDatabase_,
								const Statement::Object* pStatement_);
			Result::Value revoke(Trans::Transaction& cTrans_,
								 Server::Session* pSession_,
								 const Schema::Object::Name& cDatabase_,
								 const Statement::Object* pStatement_);

			Result::Value createCascade(Trans::Transaction& cTrans_,
										 Server::Session* pSession_,
										 const Schema::Object::Name& cDatabaseName_,
										 const Statement::Object* pStatement_);
			Result::Value dropCascade(Trans::Transaction& cTrans_,
									  Server::Session* pSession_,
									  const Schema::Object::Name& cDatabaseName_,
									  const Statement::Object* pStatement_);
			Result::Value alterCascade(Trans::Transaction& cTrans_,
									   Server::Session* pSession_,
									   const Schema::Object::Name& cDatabaseName_,
									   const Statement::Object* pStatement_);

			Result::Value createPartition(Trans::Transaction& cTrans_,
										  Server::Session* pSession_,
										  const Schema::Object::Name& cDatabaseName_,
										  const Statement::Object* pStatement_);
			Result::Value dropPartition(Trans::Transaction& cTrans_,
										Server::Session* pSession_,
										const Schema::Object::Name& cDatabaseName_,
										const Statement::Object* pStatement_);
			Result::Value alterPartition(Trans::Transaction& cTrans_,
										 Server::Session* pSession_,
										 const Schema::Object::Name& cDatabaseName_,
										 const Statement::Object* pStatement_);

			Result::Value createFunction(Trans::Transaction& cTrans_,
										 Server::Session* pSession_,
										 const Schema::Object::Name& cDatabaseName_,
										 const Statement::Object* pStatement_);
			Result::Value dropFunction(Trans::Transaction& cTrans_,
									   Server::Session* pSession_,
									   const Schema::Object::Name& cDatabaseName_,
									   const Statement::Object* pStatement_);
		} // end of namespace ReorganizeUtility
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_H

//
// Copyright (c) 2001, 2005, 2006, 2007, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
