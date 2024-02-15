// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriverTable.h -- 各論理ファイルドライバのIDと名前を管理する
// 
// Copyright (c) 1999, 2000, 2003, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_FILEDRIVERTABLE_H
#define __SYDNEY_LOGICALFILE_FILEDRIVERTABLE_H

#include "LogicalFile/Module.h"
#include "Os/CriticalSection.h"
#include "ModString.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

namespace FileDriverID
{

//	ENUM
//	LogicalFile::FileDriverID::Value -- ファイルドライバー ID の値を表す列挙型
//
//	NOTES

enum Value
{
	Unknown = 0,
	Record,
	Btree,
	Inverted,
	FullText,
	Vector,
	Lob,
	Bitmap,
	Array,
	KdTree
};
}

namespace FileDriverName
{

//
//	VARIABLE
//	LogicalFile::FileDriverName -- ファイルドライバ名
//
//	NOTES
//	ファイルドライバ名
//
const char* const Record = "Record";
const char* const Btree = "Btree";
const char* const Inverted = "Inverted";
const char* const FullText = "FullText";
const char* const Vector = "Vector";
const char* const Lob = "Lob";
const char* const Bitmap = "Bitmap";
const char* const Array = "Array";
const char* const KdTree = "KdTree";

}

//
//	CLASS
//	FileDriverTable -- ファイルドライバIDと名前とライブラリ名のテーブル
//
//	NOTES
//	ファイルドライバIDとファイルドライバ名とライブラリ名とのテーブル
//
class FileDriverTable
{
public:
	//ファイルドライバIDからライブラリ名を得る
	static ModString getLibraryName(int iDriverID_);

#ifndef SYD_COVERAGE
	//ファイルドライバ名からライブラリ名を得る
	static ModString getLibraryName(const ModString& cstrDriverName_);
#endif

private:
	// 初期化
	static void initialize();
	
	//ドライバIDとライブラリ名とのマップ
	static ModMap<int, ModString, ModLess<int> > m_mapLibraryByID;
	//ドライバ名とライブラリ名とのマップ
	static ModMap<ModString, ModString, ModLess<ModString> > m_mapLibraryByName;

	//排他制御用のクリティカルセクション
	static Os::CriticalSection m_cCriticalSection;
	//初期化したかどうか
	static bool m_bInitialized;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_FILEDRIVERTABLE_H

//
//	Copyright (c) 1999, 2000, 2003, 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
