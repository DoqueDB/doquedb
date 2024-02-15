// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_Field.h -- システム表関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_FIELD_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_FIELD_H

#include "Schema/SystemTable.h"
#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::Field --
	//		「フィールド」表のうち、あるデータベースに関する部分を表すクラス
	//
	//	NOTES
	//		「フィールド」表は、データベースごとに分割され、
	//		あるデータベースに関する部分「フィールド」表には、
	//		そのデータベースに定義されているフィールドの情報が記録される

	class Field
		: public	Base<Schema::Field, Schema::Field::Pointer, Schema::File>
	{
	public:
		SYD_SCHEMA_FUNCTION
		explicit Field(Schema::Database& database);
												// コンストラクター

		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_,
								 Schema::File& cFile_,
								 bool bRecovery_ = false);
												// あるファイルに属する
												// すべてのフィールドの情報を
												// 表から読み出す
		Schema::Field*		load(Trans::Transaction& cTrans_,
								 Schema::Object::ID::Value iID_);
												// あるオブジェクトIDを持つ
												// フィールドの情報を
												// システム表から読み出す
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  Schema::Table& table,
								  bool continuously = false);
												// 表へある表を構成する
												// ファイルのすべての
												// フィールドの情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  Schema::Index& index,
								  bool continuously = false);
												// 表へある索引を構成する
												// ファイルのすべての
												// フィールドの情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  Schema::Constraint& cConstraint_,
								  bool continuously = false);
												// 表へある制約に対応する
												// ファイルのすべての
												// フィールドの情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  Schema::File& file,
								  bool continuously = false);
												// 表へあるファイルのすべての
												// フィールドの情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::FieldPointer& field,
								  bool continuously = false,
								  bool bNeedToErase_ = true);
												// 表へあるフィールドの情報を
												// 書き込む
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_FIELD_H

//
// Copyright (c) 2002, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
