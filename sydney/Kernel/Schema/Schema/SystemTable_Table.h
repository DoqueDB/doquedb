// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_Table.h -- システム表関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_TABLE_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_TABLE_H

#include "Schema/SystemTable.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::Table -- 
	//		「表」表のうち、あるデータベースに関する部分を表すクラス
	//
	//	NOTES
	//		「表」表は、データベースごとに分割され、
	//		あるデータベースに関する部分「表」表には、
	//		そのデータベースに定義されている表の情報が記録される

	class Table
		: public	Base<Schema::Table, Schema::Table::Pointer, Schema::Database>
	{
	public:
		SYD_SCHEMA_FUNCTION
		explicit Table(Schema::Database& database);
												// コンストラクター

		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_, bool bRecovery_ = false);
												// 表から読み出す
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::TablePointer& pTable_,
								  bool continuously = false,
								  bool bNeedToErase_ = true);
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::Table& pTable_,
								  bool continuously = false,
								  bool bNeedToErase_ = true);
												// 表へある表の情報を書き込む
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_TABLE_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
