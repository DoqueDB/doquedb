// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_Partition.h -- システム表関連のクラス定義、関数宣言
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_PARTITION_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_PARTITION_H

#include "Schema/SystemTable.h"
#include "Schema/Partition.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::Partition -- 「ルール」表を表すクラス
	//
	//	NOTES

	class Partition
		: public	Base<Schema::Partition, Schema::Partition::Pointer, Schema::Database>
	{
	public:
		SYD_SCHEMA_FUNCTION
		explicit Partition(Schema::Database& database); // コンストラクター

		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_, bool bRecovery_ = false);
												// 表から読み出す
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::PartitionPointer& pPartition_,
								  bool continuously = false);
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::Partition& cPartition_,
								  bool continuously = false);
												// 表へあるエリアの情報を
												// 書き込む
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_PARTITION_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
