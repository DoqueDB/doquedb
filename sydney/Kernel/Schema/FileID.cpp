// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h --	論理ファイルに関する定義
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
#include "Schema/FileID.h"
#include "LogicalFile/FileID.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION
//	Schema::FileID::setMode -- ファイルIDで選択肢から選ぶオプションをセットする
//
//	NOTES

void
FileID::
setMode(LogicalFile::FileID& cOption_,
		const KeyType& cKey_, const ModeType& cValue_)
{
	cOption_.setInteger(cKey_, cValue_);
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
