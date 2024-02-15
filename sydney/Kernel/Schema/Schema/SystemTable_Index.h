// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_Index.h -- システム表関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_INDEX_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_INDEX_H

#include "Schema/SystemTable.h"
#include "Schema/Index.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::Index --
	//		「索引」表のうち、あるデータベースに関する部分を表すクラス
	//
	//	NOTES
	//		「索引」表は、データベースごとに分割され、
	//		あるデータベースに関する部分「索引」表には、
	//		そのデータベースに定義されている索引の情報が記録される

	class Index
		: public	Base<Schema::Index, Schema::Index::Pointer, Schema::Table>
	{
	public:
		SYD_SCHEMA_FUNCTION
		explicit Index(Schema::Database& database);
												// コンストラクター

		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_,
								 Schema::Table& cTable_,
								 bool bRecovery_ = false);
												// ある表についているすべての
												// 索引をシステム表から読み出す
		Schema::Index*		load(Trans::Transaction& cTrans_,
								 Schema::Object::ID::Value iID_);
												// あるオブジェクトIDを持つ
												// 索引の情報をシステム表から
												// 読み出す
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  Schema::Table& table,
								  bool continuously = false);
												// ある表についている
												// 索引の情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::IndexPointer& index,
								  bool continuously = false,
								  bool bNeedToErase_ = true);
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::Index& index,
								  bool continuously = false,
								  bool bNeedToErase_ = true);
												// 表へある索引の情報を書き込む
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_INDEX_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
