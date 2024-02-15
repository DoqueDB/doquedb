// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PartitionMap.cpp -- スキーマオブジェクトのマップを表すクラス関連の関数定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/PartitionMap.h"

namespace {

	// PartitionMapのコンストラクターに与えるパラメーター
	ModSize _partitionMapSize = 3;
	ModBoolean _partitionMapEnableLink = ModTrue; // Iterationする
	bool _partitionUseView = false; // 要求されるまでVectorを作らない
	bool _partitionUseCache = false; // Cacheには入れない
}

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::PartitionMap::PartitionMap -- コンストラクター
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

PartitionMap::
PartitionMap()
	: ObjectMap<Partition, PartitionPointer>(_partitionMapSize, _partitionMapEnableLink, _partitionUseView, _partitionUseCache)
{
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
