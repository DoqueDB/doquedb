// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.h -- トランザクションの回復関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_TRANSACTION_H
#define	__SYDNEY_ADMIN_TRANSACTION_H

#include "Admin/Module.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;

	namespace Log
	{
		class Data;
	}
}

_SYDNEY_ADMIN_BEGIN
_SYDNEY_ADMIN_RECOVERY_BEGIN

class Database;

//	CLASS
//	Admin::Recovery::Transaction -- トランザクションの回復処理を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class Transaction
{
	friend class Database;
	friend class Trans::Transaction;
private:
	// タプルに対する更新操作を UNDO する
	SYD_ADMIN_FUNCTION
	static void
	undoTuple(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  const ModUnicodeString& dbName);
	static void
	undoTuple(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  Schema::Database& database);
	SYD_ADMIN_FUNCTION
	static void
	undoTuple(Trans::Transaction& trans, const Common::DataArrayData& undoLog,
			  const ModUnicodeString& dbName);
	static void
	undoTuple(Trans::Transaction& trans, const Common::DataArrayData& undoLog,
			  Schema::Database& database);

	// タプルに対する操作を REDO する
#ifdef OBSOLETE // データベース名でREDOすることはない
	static void
	redoTuple(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  const ModUnicodeString& dbName);
#endif
	static void
	redoTuple(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  Schema::Database& database);

	// スキーマに対する操作を UNDO する
	static void
	undoReorg(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  const ModUnicodeString& dbName, bool redone,
			  bool rollforward = false);
	static void
	undoReorg(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  Schema::Database& database, bool redone,
			  bool rollforward = false);

	// スキーマに対する操作を REDO する
	static Database*
	redoReorg(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  const ModUnicodeString& dbName, bool rollforward = false);
	static Database*
	redoReorg(Trans::Transaction& trans, const Trans::Log::Data& logData,
			  Schema::Database& database, bool rollforward = false);

	// ドライバーに対する操作を UNDO する
	static void
	undoDriver(Trans::Transaction& trans, const Trans::Log::Data& logData,
			   Schema::Database& database);

	// ドライバーに対する操作を REDO する
	static void
	redoDriver(Trans::Transaction& trans, const Trans::Log::Data& logData,
			   Schema::Database& database);
	
};

_SYDNEY_ADMIN_RECOVERY_END
_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_TRANSACTION_H

//
// Copyright (c) 2001, 2002, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
