// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenMode.h -- 論理ファイルのオープンモードのヘッダーファイル
// 
// Copyright (c) 1999, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FILECOMMON_OPENMODE_H
#define __SYDNEY_FILECOMMON_OPENMODE_H

#include "Common/Common.h"
#include "Common/Internal.h"

_SYDNEY_BEGIN

namespace FileCommon
{

namespace OpenMode
{
	//
	//	ENUM
	//	FileCommon::OpenMode::Mode -- 論理ファイルオープンモード
	//
	//	NOTES
	//	論理ファイルオープンモードを示す列挙子
	//	LogicalFile::OpenOption::OpenMode::Value や
	//	FileCommon::OpenOption::OpenMode とは値が違うので注意！
	//
	enum Mode
	{
		Read = 0,
		Search,
		Update,
		Initialize,
		Batch,
		// insert, update, delete は Mode::Update を使用する
		// システム初期化時は、Mode::Initializeを使用する
		// バッチモードのときは Mode::Batch を使用する
		Unknown
	};

} // end of namespace OpenMode

namespace ReadSubMode // 2000-12-13追加
{
	//
	//	ENUM
	//	FileCommon::ReadSubMode::Mode -- オブジェクト取得サブモード列挙子
	//
	//	NOTES
	//	オブジェクト取得サブモード列挙子。
	//	LogicalFile::OpenOption::ReadSubMode::Value や
	//	FileCommon::OpenOption::ReadSubMode とは値が違うので注意！
	//
	enum Mode {
		ScanRead = 0,
		FetchRead,
		Unknown
	};
} // end of namespace ReadSubMode

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_OPENMODE_H

//
//	Copyright (c) 1999, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
