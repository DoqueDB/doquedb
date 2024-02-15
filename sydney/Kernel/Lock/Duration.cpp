// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Duration.cpp -- ロック持続期間関連の関数定義
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Lock/Duration.h"

#include "ModMessage.h"
#include "ModOstream.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{
	const char* const _pszDurationName[Lock::Duration::ValueNum+1] =
	{
		"Pulse",
		"Statement",
		"Cursor",
		"Inside",
		"User",
		"--",
	};
}

// Durationをメッセージに出力する
ModMessageStream& operator<<(ModMessageStream& cStream_, Lock::Duration::Value eValue_)
{
	return cStream_ << _pszDurationName[eValue_];
}

ModOstream& operator<<(ModOstream& cStream_, Lock::Duration::Value eValue_)
{
	return cStream_ << _pszDurationName[eValue_];
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
