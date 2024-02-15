// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h --
// 
// Copyright (c) 2005, 2007, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_FILE_H
#define __SYDNEY_BITMAP_FILE_H

#include "Bitmap/Module.h"
#include "Bitmap/FileID.h"
#include "Bitmap/Page.h"

#include "PhysicalFile/File.h"

#include "Trans/Transaction.h"

#include "Os/Path.h"

#include "FileCommon/OpenOption.h"

#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::File --
//
//	NOTES
//
//
class File
{
	friend class Page;

public:
	//コンストラクタ
	File(const FileID& cFileID_);
	//デストラクタ
	virtual ~File();

	// ファイルサイズを得る
	ModUInt64 getSize() const { return m_pPhysicalFile->getSize(); }

	// ファイルを作成する
	virtual void create();
	// ファイルを破棄する
	void destroy() { destroy(*m_pTransaction); }
	virtual void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	virtual void mount(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->mount(cTransaction_);
	}
	// ファイルをアンマウントする
	virtual void unmount(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->unmount(cTransaction_);
	}
	// ファイルをフラッシュする
	virtual void flush(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->flush(cTransaction_);
	}

	// ファイルのバックアップを開始する
	virtual void startBackup(const Trans::Transaction& cTransaction_,
							 const bool bRestorable_)
	{
		m_pPhysicalFile->startBackup(cTransaction_, bRestorable_);
	}
	// ファイルのバックアップを終了する
	virtual void endBackup(const Trans::Transaction& cTransaction_)
	{
		m_pPhysicalFile->endBackup(cTransaction_);
	}

	// ファイルを障害から回復する
	virtual void recover(const Trans::Transaction& cTransaction_,
						 const Trans::TimeStamp& cPoint_)
	{
		m_pPhysicalFile->recover(cTransaction_, cPoint_);
	}
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	virtual void restore(const Trans::Transaction& cTransaction_,
						 const Trans::TimeStamp& cPoint_)
	{
		m_pPhysicalFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	virtual void open(const Trans::Transaction& cTransaction_,
					  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	//論理ファイルをクローズする
	virtual void close();

	// 同期をとる
	virtual void sync(const Trans::Transaction& cTransaction_,
					  bool& incomplete, bool& modified)
	{
		m_pPhysicalFile->sync(cTransaction_, incomplete, modified);
	}

	//ファイルを移動する
	virtual void move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cFilePath_);

	// 実体である OS ファイルが存在するか調べる
	virtual bool
	isAccessible(bool bForce_ = false) const
	{
		return m_pPhysicalFile->isAccessible(bForce_);
	}
	// マウントされているか調べる
	virtual bool
	isMounted(const Trans::Transaction& trans) const
	{
		return m_pPhysicalFile->isMounted(trans);
	}

	// デタッチされている全ページをフラッシュする
	virtual void flushAllPages();
	// デタッチされている全ページを元に戻す
	virtual void recoverAllPages();

	// ページの使用可能サイズを得る
	ModUInt32 getPageDataSize() const
	{
		return m_pPhysicalFile->getPageDataSize();
	}

	// 1ページを得るコストを得る
	virtual double getCost() const;

	// 物理ファイルの整合性検査を開始する
	virtual void startVerification(
		const Trans::Transaction& cTransaction_,
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_);

	// 物理ファイルの整合性検査を終了する
	virtual void endVerification();

	// 整合性検査実行中かどうか
	bool doVerify()
	{
		return m_bVerification;
	}

	// 整合性検査時に不整合を解消するモードかどうか
	bool isCorrect()
	{
		return m_uiTreatment & Admin::Verification::Treatment::Correct;
	}
	// 整合性検査時に不整合があっても検査を続行するかどうか
	bool isContinue()
	{
		return m_uiTreatment & Admin::Verification::Treatment::Continue;
	}
	// キャンセルリクエストがきているかどうか
	bool isCancel() const
	{
		return const_cast<Trans::Transaction*>(m_pTransaction)
			->isCanceledStatement();
	}

	// 整合性検査の経過を表すクラスを得る
	Admin::Verification::Progress& getProgress() { return *m_pProgress; }
	// パスを得る
	const Os::Path& getPath() { return m_cPath; }

	// 物理ページを新たに確保する
	PhysicalFile::Page* allocatePhysicalPage();
	// 物理ページを開放する
	void freePhysicalPage(PhysicalFile::Page* pPage_);

	// 物理ページをアタッチする
	PhysicalFile::Page*
	attachPhysicalPage(PhysicalFile::PageID uiPageID_,
					   Buffer::Page::FixMode::Value eFixMode_
					   = Buffer::Page::FixMode::Unknown);
	// 物理ページをデタッチする
	void detachPhysicalPage(PhysicalFile::Page* pPage_);

	// 物理ページのFixModeを変更する
	PhysicalFile::Page* changeFixMode(PhysicalFile::Page* pPage_);

	// トランザクションを得る
	const Trans::Transaction& getTransaction() const
	{
		return *m_pTransaction;
	}

	// FixModeを得る
	Buffer::Page::FixMode::Value getFixMode() const { return m_eFixMode; }

protected:
	// ディレクトリを削除する
	void rmdir();

	// 物理ファイル
	PhysicalFile::File* m_pPhysicalFile;

	// FileID
	const FileID& m_cFileID;

private:
	// 物理ファイルをattachする
	void attach(const FileID& cFileID_);
	// 物理ファイルをdetachする
	void detach();

	// 物理ページをattachする
	PhysicalFile::Page* attachPhysicalPageInternal(
		PhysicalFile::PageID uiPageID_,
		Buffer::Page::FixMode::Value eFixMode_);

	typedef ModMap<PhysicalFile::PageID, PhysicalFile::Page*,
				   ModLess<PhysicalFile::PageID> > Map;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;

	// 整合性検査モードかどうか
	bool m_bVerification;
	// 整合性検査の実行モード
	Admin::Verification::Treatment::Value m_uiTreatment;
	// 整合性検査結果を格納する箱
	Admin::Verification::Progress* m_pProgress;
	
	// dirtyなページをキャッシュしておくマップ
	Map m_mapPage;
	// フリーリスト
	ModVector<PhysicalFile::Page*> m_vecpFreeList;

	// パス
	Os::Path m_cPath;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_FILE_H

//
//	Copyright (c) 2005, 2007, 2008, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
