// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.h
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

#ifndef __SYDNEY_LOGICALFILE_EXTERNALIZABLE_H
#define __SYDNEY_LOGICALFILE_EXTERNALIZABLE_H

#include "LogicalFile/Module.h"
#include "Common/Externalizable.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

//	CLASS
//	LogicalFile::Externalizable -- 
//
//	NOTES

class SYD_LOGICALFILE_FUNCTION Externalizable
	: public Common::Externalizable
{
public:
	struct Category
	{
		enum Value
		{
			Unknown = 0,
			FileID,
			OpenOption,
			
			FullTextMergeLog,	// 全文索引のマージを表すログ
			KdTreeMergeLog,		// KD-Tree索引のマージを表すログ
			
			ValueNum
		};
	};

	static Common::Externalizable* getClassInstance(int classID);
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_EXTERNALIZABLE_H

//
//	Copyright (c) 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
