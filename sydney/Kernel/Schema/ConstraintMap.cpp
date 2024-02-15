// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConstraintMap.cpp -- スキーマオブジェクトのマップを表すクラス関連の関数定義
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

#include "Schema/ConstraintMap.h"

namespace {

	// ConstraintMapのコンストラクターに与えるパラメーター
	ModSize _constraintMapSize = 1; // 現在は主キーの制約のみ
	ModBoolean _constraintMapEnableLink = ModFalse; // Iterationするが小さいので不要
	bool _constraintUseView = false; // 要求されるまでVectorを作らない
}

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::ConstraintMap::ConstraintMap -- コンストラクター
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

ConstraintMap::
ConstraintMap()
	: ObjectMap<Constraint, ConstraintPointer>(_constraintMapSize, _constraintMapEnableLink, _constraintUseView)
{
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
