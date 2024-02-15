// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.cpp
//		-- LogicalFileモジュール内のExternalizableオブジェクトに関する定義
// 
// Copyright (c) 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Common/Assert.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/LogData.h"

_SYDNEY_USING
using namespace LogicalFile;

//	FUNCTION
//	LogicalFile::Externalizable::getClassInstance --
//		LogicalFileに属するExternalizableオブジェクトを得る
//
//	NOTES

Common::Externalizable*
Externalizable::
getClassInstance(int classID)
{
	; _SYDNEY_ASSERT(classID >= Common::Externalizable::LogicalFileClasses);

	Common::Externalizable*	object = 0;

	Externalizable::Category::Value category =
		static_cast<Externalizable::Category::Value>(
			 classID - Common::Externalizable::LogicalFileClasses);

	switch (category) {
	case Category::FileID:
		object = new FileID();
		break;
	case Category::OpenOption:
		object = new OpenOption();
		break;
	case Category::FullTextMergeLog:
		object = new FullTextMergeLog();
		break;
	case Category::KdTreeMergeLog:
		object = new KdTreeMergeLog();
		break;
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
	return object;
}

//
// Copyright (c) 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
