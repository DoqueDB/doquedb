// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaCategory.h -- エリア種別関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_AREACATEGORY_H
#define	__SYDNEY_SCHEMA_AREACATEGORY_H

#include "Schema/Module.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace AreaCategory {

	//	ENUM
	//	Schema::AreaCategory::Value -- エリアを指定する対象の種類
	//
	//	NOTES

	enum Value
	{
		Default = 0,						// デフォルト
		LogicalLog,							// 論理ログファイル(非使用)
		PhysicalLog,						// 物理ログファイル(非使用)
		//--------
		// 以下はエリアを指定できるファイルの種類が増えたら追加される
		//--------
		FileMin,							// これ以降はファイル
		Heap,								// ヒープファイル
		Index,								// B+木索引ファイル
		FullText,							// 全文索引ファイル
		ValueNum
	};

	// あるカテゴリーで指定がないときに代わりに使うべきカテゴリーを得る
	Value getSuperValue(Value eValue_);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_AREACATEGORY_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
