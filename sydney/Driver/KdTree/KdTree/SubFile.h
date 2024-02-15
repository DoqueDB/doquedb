// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_SUBFILE_H
#define __SYDNEY_KDTREE_SUBFILE_H

#include "KdTree/Module.h"
#include "KdTree/File.h"
#include "KdTree/FileID.h"

#include "PhysicalFile/File.h"
#include "Trans/Transaction.h"
#include "Os/CriticalSection.h"
#include "Os/Path.h"
#include "FileCommon/OpenOption.h"

#include "ModOstream.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::SubFile --
//
//	NOTES
//	PhysicalFileモジュールを利用するファイルの基底クラス
//	PhysicalFile共通の操作はここに実装する
//
class SubFile : public File
{
public:
	// コンストラクタ
	SubFile(FileID& cFileID_,
			PhysicalFile::Type eType_,
			int iPageSize_,
			const Os::Path& cPath_);
	// デストラクタ
	virtual ~SubFile();

	// ファイルサイズを得る
	ModUInt64 getSize() { return m_pPhysicalFile->getSize(); }

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->mount(cTransaction_);
	}
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->unmount(cTransaction_);
	}
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_)
	{
		m_pPhysicalFile->startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_pPhysicalFile->restore(cTransaction_, cPoint_);
	}

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified)
	{
		m_pPhysicalFile->sync(cTransaction_, incomplete, modified);
	}

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
	{
		return m_pPhysicalFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
	{
		return m_pPhysicalFile->isMounted(trans);
	}
	bool isMounted() const
	{
		return isMounted(*m_pTransaction);
	}

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cRootPath_);

protected:
	// 物理ファイルの整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_);

	// 物理ファイルの整合性検査を終了する
	void endVerification();

	// 物理ページをattachする
	PhysicalFile::Page*
	attachPhysicalPage(PhysicalFile::PageID uiPageID_,
					   Buffer::Page::FixMode::Value eFixMode_);
	
	// 物理ファイル
	PhysicalFile::File* m_pPhysicalFile;
	// ファイルタイプ
	PhysicalFile::Type m_eType;

private:
	// 物理ファイルをattachする
	void attach(int iPageSize_);
	// 物理ファイルをdetachする
	void detach();
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_SUBFILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
