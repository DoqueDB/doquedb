// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Version.h -- Ｂ＋木ファイルバージョン関連のヘッダファイル
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_VERSION_H
#define __SYDNEY_BTREE_VERSION_H

#include "Common/Common.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	STRUCT
//	Btree::FileVersion -- Ｂ＋木ファイルバージョンのための構造体
//
//	NOTES
//	Ｂ＋木ファイルバージョンのための構造体。
//
struct FileVersion
{
	//
	//	ENUM
	//	Btree::FileVersion::Value -- Ｂ＋木ファイルバージョン
	//
	//	NOTES
	//	Ｂ＋木ファイルバージョン。
	//
	enum Value
	{
		Version1 = 0,                   // 初期バージョン
		VersionNum,                     // 現在のバージョン数
		CurrentVersion = VersionNum - 1 // 現在のバージョン
	};

}; // end of struct Btree::FileVersion

} // end of namespace Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_VERSION_H

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
