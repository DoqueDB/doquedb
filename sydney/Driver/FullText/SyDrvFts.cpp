// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyDrvFts.cpp -- 全文検索ファイルドライバを得る関数
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/FileDriver.h"
#include "LogicalFile/FileDriver.h"

_SYDNEY_USING

//
//	FUNCTION global
//	DBGetFileDriver -- 全文検索ファイルのファイルドライバのインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Logical::FileDriver*
//		全文検索ファイルのファイルドライバ
//
LogicalFile::FileDriver*
DBGetFileDriver()
{
	return new FullText::FileDriver;
}

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
