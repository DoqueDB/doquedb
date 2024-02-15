// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableMap.cpp -- スキーマオブジェクトのマップを表すクラス関連の関数定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/TableMap.h"
#include "Schema/Recovery.h"

namespace {

	// TableMapのコンストラクターに与えるパラメーター
	ModSize _tableMapSize = 23;
	ModBoolean _tableMapEnableLink = ModTrue; // Iterationする
	bool _tableUseView = false; // 要求されるまでVectorを作らない
	bool _tableUseCache = false; // Cacheには入れない
}

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::TableMap::TableMap -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

TableMap::
TableMap()
	: ObjectMap<Table, TablePointer>(_tableMapSize, _tableMapEnableLink, _tableUseView, _tableUseCache)
{
}

//	FUNCTION public
//	Schema::TableMap::findValid -- 外部に見せてよいオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table* pTable_
//			比較対象のオブジェクト
//		const Schema::Object::Name& cDatabaseName_
//		bool bInternal_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
bool
TableMap::
findValid(Table* pTable_, const Object::Name& cDatabaseName_, bool bInternal_)
{
	return (bInternal_ || Manager::RecoveryUtility::Undo::isValidTable(cDatabaseName_, pTable_->getID()));
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
