// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_AreaContent.h -- システム表関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_AREACONTENT_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_AREACONTENT_H

#include "Schema/SystemTable.h"
#include "Schema/AreaContent.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::AreaContent -- 「エリア格納関係」表を表すクラス
	//
	//	NOTES

	class AreaContent
		: public	Base<Schema::AreaContent, Schema::AreaContent::Pointer, Schema::Area>
	{
	public:
		SYD_SCHEMA_FUNCTION
		explicit AreaContent(Schema::Database& database);// コンストラクター

		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_,
								 Schema::Area& cArea_,
								 bool bRecovery_ = false);
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_);
												// システム表へ書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::Area& cArea_,
 								  bool continuously = false);
												// システム表へあるエリア
												// を対象とするすべての格納関係
												// の情報を書き込む
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::AreaContentPointer& pAreaContent_,
								  bool continuously = false);
												// 表へあるエリア格納関係の
												// 情報を書き込む
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_AREACONTENT_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
