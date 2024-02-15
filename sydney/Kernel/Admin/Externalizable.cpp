// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.cpp -- シリアル化可能なオブジェクト関連の関数定義
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Admin/Externalizable.h"
#include "Admin/LogData.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{
}

//
//	FUNCTION public
//	Admin::Externalizable::getClassInstance
//		-- 指定された種別のシリアル化可能なオブジェクトを確保する
//
//	NOTES
//
//	ARGUMENTS
//	int classID
//		確保するオブジェクトの種別を表す値
//
//	RETURN
//	確保したシリアル化可能なオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//
// static
Common::Externalizable*
Externalizable::getClassInstance(int classID)
{
	; _SYDNEY_ASSERT(classID >= Common::Externalizable::AdminClasses);

	Common::Externalizable*	object = 0;

	Externalizable::Category::Value	category =
		static_cast<Externalizable::Category::Value>(
			classID - Common::Externalizable::AdminClasses);
		
	switch (category) {
	case Category::ReplicationEndLogData:
		object = new Log::ReplicationEndData();			break;
	default:
		; _SYDNEY_ASSERT(false);
	}

	return object;
}

//
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
