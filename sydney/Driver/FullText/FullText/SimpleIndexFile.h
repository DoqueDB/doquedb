// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleIndexFile.h --
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

#ifndef __SYDNEY_FULLTEXT_SIMPLEINDEXFILE_H
#define __SYDNEY_FULLTEXT_SIMPLEINDEXFILE_H

#include "FullText/Module.h"
#include "FullText/FileID.h"
#include "FullText/IndexFile.h"

#include "LogicalFile/OpenOption.h"

#include "Inverted/SearchCapsule.h"
#include "Inverted/GetLocationCapsule.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	CLASS
//	FullText::SimpleIndexFile --
//
//	NOTES
//
class SimpleIndexFile : public IndexFile
{
public:
	// コンストラクタ
	SimpleIndexFile(FileID& cFileID_);
	// デストラクタ
	virtual ~SimpleIndexFile();
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_INDEXFILE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
