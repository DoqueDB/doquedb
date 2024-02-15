// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleIndexFile.cpp --
// 
// Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/SimpleIndexFile.h"
#include "Inverted/IndexFileSet.h"



_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

//
//	FUNCTION public
//	FullText::SimpleIndexFile::SimpleIndexFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleIndexFile::SimpleIndexFile(FullText::FileID& cFileID_)
	: IndexFile(cFileID_, cFileID_.getPath())
{
}

//
//	FUNCTION public
//	FullText::SimpleIndexFile::~SimpleIndexFile -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleIndexFile::~SimpleIndexFile()
{
}
//
//	Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
