// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessLobFile.cpp -- 再構成や整合性検査で論理ファイルを読み書きするためのクラスの定義
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"

#include "Schema/AccessLobFile.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::AccessLobFile::isGetData -- フィールドを取得するデータとして扱うか
//
//	NOTES
//		LOBファイルは常にデータを取得しない
//
//	ARGUMENTS
//		Schema::Field* pField
//			検査対象のフィールド
//
//	RETURN
//		true ... 取得するデータとして扱う
//		false... 取得するデータとして扱わない
//
//	EXCEPTIONS
//		なし

bool
AccessLobFile::
isGetData(Field* pField_) const
{
	return false;
}

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
