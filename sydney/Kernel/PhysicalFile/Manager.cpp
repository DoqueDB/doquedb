// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- 物理ファイルマネージャ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Exception/BadArgument.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"

#include "Version/File.h"

#include "PhysicalFile/Manager.h"
#include "PhysicalFile/AreaManageFile.h"
#include "PhysicalFile/PageManageFile.h"
#include "PhysicalFile/NonManageFile.h"
#include "PhysicalFile/DirectAreaFile.h"
#include "PhysicalFile/PageManageFile2.h"

_SYDNEY_USING

namespace
{

namespace _PhFileManager
{
	Os::CriticalSection	Latch;
}

}

using namespace PhysicalFile;

///////////////////////////////////////////
//
//	public メンバ関数
//
///////////////////////////////////////////
#ifdef OBSOLETE
//
//	VARIABLE public
//	PhysicalFile::Manager::m_Initialize -- 初期化フラグ
//
//	NOTES
//	初期化フラグ。
//
// static
bool
Manager::m_Initialized = false;

//
//	VARIABLE public
//	PhysicalFile::Manager::m_File -- 物理ファイル記述子リスト
//
//	NOTES
//	アタッチ中の物理ファイル記述子のリスト。
//
// static
File*
Manager::m_File = 0;
#endif
//
//	FUNCTION public
//	PhysicalFile::Manager::initialize --
//		物理ファイルマネージャを初期化する
//
//	NOTES
//	物理ファイルマネージャを初期化する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
Manager::initialize()
{
#ifdef OBSOLETE
	Manager::m_Initialized = true;

	Manager::m_File = 0;
#endif
}

//
//	FUNCTION public
//	PhysicalFile::Manager::terminate --
//		物理ファイルマネージャの後処理をする
//
//	NOTES
//	物理ファイルマネージャの後処理をする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
Manager::terminate()
{
#ifdef OBSOLETE
	; _SYDNEY_ASSERT(Manager::m_File == 0);

	Manager::m_Initialized = false;
#endif
}

//
//	FUNCTION public
//	PhysicalFile::Manager::attachFile --
//		物理ファイル記述子を生成する
//
//	NOTES
//	物理ファイル記述子を生成し、返す。
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		物理ファイル格納戦略への参照
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		物理ファイルバッファリング戦略への参照
//
//	RETURN
//	PhysicalFile::File*
//		生成した物理ファイル記述子
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数
//
// static
File*
Manager::attachFile(const File::StorageStrategy&	FileStorageStrategy_,
					const File::BufferingStrategy&	BufferingStrategy_,
					bool batch_)
{
	return Manager::attachFile(FileStorageStrategy_,
							   BufferingStrategy_,
							   0,
							   batch_);
}

//
//	FUNCTION public
//	PhysicalFile::Manager::attachFile --
//		物理ファイル記述子を生成する
//
//	NOTES
//	物理ファイル記述子を生成し、返す。
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		物理ファイル格納戦略への参照
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		物理ファイルバッファリング戦略への参照
//	const Lock::FileName&							LockName_
//		物理ファイルが存在する論理ファイルのロック名への参照
//
//	RETURN
//	PhysicalFile::File*
//		生成した物理ファイル記述子
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数
//
// static
File*
Manager::attachFile(const File::StorageStrategy&	FileStorageStrategy_,
					const File::BufferingStrategy&	BufferingStrategy_,
					const Lock::FileName&			LockName_,
					bool batch_)
{
	return Manager::attachFile(FileStorageStrategy_,
							   BufferingStrategy_,
							   &LockName_,
							   batch_);
}

//
//	FUNCTION public
//	PhysicalFile::Manager::attachFile --
//		物理ファイル記述子を生成する
//
//	NOTES
//	物理ファイル記述子を生成し、返す。
//	実際に参照する物理ファイルの実体は引数SrcFile_と同じもので、
//	異なる記述子を生成する。
//
//	ARGUMENTS
//	const PhysicalFile::File*	SrcFile_
//		物理ファイル記述子
//
//	RETURN
//	PhysicalFile::File*
//		生成した物理ファイル記述子
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
File*
Manager::attachFile(const File*	SrcFile_)
{
	Version::File::StorageStrategy		versionFileStorageStrategy =
		SrcFile_->m_VersionFile->getStorageStrategy();

	File::StorageStrategy	physicalFileStorageStrategy;

	// 物理ファイルタイプ
	physicalFileStorageStrategy.m_PhysicalFileType = SrcFile_->m_Type;
	// 物理ページ内の使用率上限
	if (SrcFile_->m_Type == AreaManageType)
	{
		const AreaManageFile*	srcFile =
			_SYDNEY_DYNAMIC_CAST(const AreaManageFile*, SrcFile_);

		physicalFileStorageStrategy.m_PageUseRate = srcFile->m_PageUseRate;
	}
	// マウントされているかどうか
	physicalFileStorageStrategy.m_VersionFileInfo._mounted =
		versionFileStorageStrategy._mounted;
	// バージョンページサイズ
	physicalFileStorageStrategy.m_VersionFileInfo._pageSize =
		versionFileStorageStrategy._pageSize;
	// マスタデータファイル格納位置
	physicalFileStorageStrategy.m_VersionFileInfo._path._masterData =
		versionFileStorageStrategy._path._masterData;
	// バージョンログファイル格納位置
	physicalFileStorageStrategy.m_VersionFileInfo._path._versionLog =
		versionFileStorageStrategy._path._versionLog;
	// 同期ログファイル格納位置
	physicalFileStorageStrategy.m_VersionFileInfo._path._syncLog =
		versionFileStorageStrategy._path._syncLog;
	// マスタデータファイル最大サイズ
	physicalFileStorageStrategy.m_VersionFileInfo._sizeMax._masterData =
		versionFileStorageStrategy._sizeMax._masterData;
	// バージョンログファイル最大サイズ
	physicalFileStorageStrategy.m_VersionFileInfo._sizeMax._versionLog =
		versionFileStorageStrategy._sizeMax._versionLog;
	// 同期ログファイル最大サイズ
	physicalFileStorageStrategy.m_VersionFileInfo._sizeMax._syncLog =
		versionFileStorageStrategy._sizeMax._syncLog;
	// マスタデータファイルエクステンションサイズ
	physicalFileStorageStrategy.m_VersionFileInfo._extensionSize._masterData =
		versionFileStorageStrategy._extensionSize._masterData;
	// バージョンログファイルエクステンションサイズ
	physicalFileStorageStrategy.m_VersionFileInfo._extensionSize._versionLog =
		versionFileStorageStrategy._extensionSize._versionLog;
	// 同期ログファイルエクステンションサイズ
	physicalFileStorageStrategy.m_VersionFileInfo._extensionSize._syncLog =
		versionFileStorageStrategy._extensionSize._syncLog;
	
	Version::File::BufferingStrategy	versionFileBufferingStrategy =
		SrcFile_->m_VersionFile->getBufferingStrategy();

	File::BufferingStrategy	physicalFileBufferingStrategy;

	// バッファプール種別
	physicalFileBufferingStrategy.m_VersionFileInfo._category =
		versionFileBufferingStrategy._category;

	return Manager::attachFile(physicalFileStorageStrategy,
							   physicalFileBufferingStrategy,
							   SrcFile_->m_VersionFile->getLockName());
}

//
//	FUNCTION public
//	PhysicalFile::Manager::detachFile --
//		物理ファイル記述子を破棄する
//
//	NOTES
//	物理ファイル記述子を破棄する。
//
//	ARGUMENTS
//	PhysicalFile::File*&	File_
//		デタッチする物理ファイルの記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// static
void
Manager::detachFile(File*&	File_)
{
#ifdef OBSOLETE
	Os::AutoCriticalSection	latch(_PhFileManager::Latch);

	File*	top = Manager::m_File;

	File*	file = 0;

	if (File_ == top)
	{
		file = top;
	}
	else
	{
		file = top->m_Next;

		bool	exist = false;

		while (file != top)
		{
			if (file == File_)
			{
				exist = true;

				break;
			}

			file = file->m_Next;
		}

		; _SYDNEY_ASSERT(exist);
	}

	if (file == file->m_Next)
	{
		Manager::m_File = 0;
	}
	else
	{
		file->m_Prev->m_Next = file->m_Next;

		file->m_Next->m_Prev = file->m_Prev;

		Manager::m_File = file->m_Next;
	}
#endif

	delete File_;
	File_ = 0;
}
#ifdef OBSOLETE
//
//	FUNCTION public
//	PhysicalFile::Manager::getLatch --
//		物理ファイル記述子リストの操作の排他制御をするためのラッチを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::CriticalSection&
//		ラッチへの参照
//
//	EXCEPTIONS
//	なし
//
// static
Os::CriticalSection&
Manager::getLatch()
{
	return _PhFileManager::Latch;
}
#endif
///////////////////////////////////////////
//
//	private メンバ関数
//
///////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::Manager::attachFile --
//		物理ファイル記述子を生成する
//
//	NOTES
//	物理ファイル記述子を生成し、返す。
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		物理ファイル格納戦略への参照
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		物理ファイルバッファリング戦略への参照
//	const Lock::FileName*							LockName_
//		物理ファイルが存在する論理ファイルのロック名へのポインタ
//
//	RETURN
//	PhysicalFile::File*
//		生成した物理ファイル記述子
//
//	EXCEPTIONS
//	Exception::BadArgument
//		不正な引数
//
// static
File*
Manager::attachFile(const File::StorageStrategy&	FileStorageStrategy_,
					const File::BufferingStrategy&	BufferingStrategy_,
					const Lock::FileName*			LockName_,
					bool batch_)
{
//	; _SYDNEY_ASSERT(Manager::m_Initialized);

	File*	file = 0;	// 物理ファイル記述子

	switch (FileStorageStrategy_.m_PhysicalFileType)
	{
	case AreaManageType:
		// 空き領域管理機能付き物理ファイルの記述子を生成する
		file = new AreaManageFile(FileStorageStrategy_,
								  BufferingStrategy_,
								  LockName_, batch_);
		break;
	case PageManageType:
		// 物理ページ管理機能付き物理ファイルの記述子を生成する
		file = new PageManageFile(FileStorageStrategy_,
								  BufferingStrategy_,
								  LockName_, batch_);
		break;
	case NonManageType:
		// 管理機能なし物理ファイルの記述子を生成する
		file = new NonManageFile(FileStorageStrategy_,
								 BufferingStrategy_,
								 LockName_, batch_);
		break;
	case DirectAreaType:
		file = new DirectAreaFile(FileStorageStrategy_,
								  BufferingStrategy_,
								  LockName_,
								  batch_);
		break;
	case PageManageType2:
		// 物理ページ管理機能つき物理ファイル PageManageFile2 の記述子を生成する
		file = new PageManageFile2(FileStorageStrategy_,
								   BufferingStrategy_,
								   LockName_,
								   batch_);
		break;
	default:
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
#ifdef OBSOLETE
	Os::AutoCriticalSection	latch(_PhFileManager::Latch);

	if (Manager::m_File == 0)
	{
		Manager::m_File = file;

		Manager::m_File->m_Next = file;

		Manager::m_File->m_Prev = file;
	}
	else
	{
		if (Manager::m_File == Manager::m_File->m_Next)
		{
			file->m_Next =
			file->m_Prev = Manager::m_File;

			Manager::m_File->m_Next =
			Manager::m_File->m_Prev = file;
		}
		else
		{
			file->m_Next = Manager::m_File->m_Next;
			file->m_Prev = Manager::m_File;

			Manager::m_File->m_Next =
			file->m_Next->m_Prev = file;
		}

		Manager::m_File = file;
	}
#endif
	return file;
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
