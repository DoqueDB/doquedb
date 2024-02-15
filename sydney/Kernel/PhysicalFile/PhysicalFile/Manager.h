// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- 物理ファイルマネージャ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_MANAGER_H
#define __SYDNEY_PHYSICALFILE_MANAGER_H

#include "PhysicalFile/Module.h"
#include "PhysicalFile/File.h"

_SYDNEY_BEGIN

namespace PhysicalFile
{

//
//	CLASS
//	PhysicalFile::Manager --
//		物理ファイルマネージャクラス
//
//	NOTES
//	物理ファイルマネージャクラス。
//
class SYD_PHYSICALFILE_FUNCTION Manager : public Common::Object
{
public:

	//
	// メンバ関数
	//

	// 物理ファイルマネージャを初期化する
	static void initialize();

	// 物理ファイルマネージャの後処理をする
	static void terminate();

	// 物理ファイル記述子を生成する
	static File* attachFile(
		const File::StorageStrategy&	FileStorageStrategy_,
		const File::BufferingStrategy&	BufferingStrategy_,
		bool batch_ = false);

	// 物理ファイル記述子を生成する
	static File* attachFile(
		const File::StorageStrategy&	FileStorageStrategy_,
		const File::BufferingStrategy&	BufferingStrategy_,
		const Lock::FileName&			LockName_,
		bool batch_ = false);

	// 物理ファイル記述子を生成する
	static File* attachFile(const File*	SrcFile_);

	// 物理ファイル記述子を破棄する
	static void detachFile(File*&	File_);

#ifdef OBSOLETE

	// 物理ファイル記述子リストの操作の排他制御をするためのラッチを返す
	static Os::CriticalSection& getLatch();

	//
	// データメンバ
	//

	// 初期化フラグ
	static bool		m_Initialized;

	// 物理ファイル記述子リスト
	static File*	m_File;

#endif

private:

	// 物理ファイル記述子を生成する
	static File* attachFile(
		const File::StorageStrategy&	FileStorageStrategy_,
		const File::BufferingStrategy&	BufferingStrategy_,
		const Lock::FileName*			LockName_,
		bool batch_);
};

}

_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_MANAGER_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
