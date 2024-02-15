// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_FILE_H
#define __SYDNEY_BTREE2_FILE_H

#include "Btree2/Module.h"
#include "Btree2/FileID.h"
#include "Btree2/Page.h"
#include "Common/Object.h"
#include "PhysicalFile/File.h"
#include "Trans/Transaction.h"
#include "Os/CriticalSection.h"
#include "Os/Path.h"
#include "FileCommon/OpenOption.h"
#include "ModMap.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class DataPage;

//
//	CLASS
//	Btree2::File --
//
//	NOTES
//
//
class File : public Common::Object
{
	friend class Page;
	friend class DataPage;

public:
	//コンストラクタ
	File(int iCacheCount_);
	//デストラクタ
	virtual ~File();

	// 物理ファイルをアタッチする
	void attach(const FileID& cFileID_, int iPageSize_,
				const Os::Path& cSubPath_);
	// 物理ファイルをデタッチする
	void detach();

	// ファイルサイズを得る
	virtual ModUInt64 getSize() const { return m_pPhysicalFile->getSize(); }

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

	// 物理ページを確保する
	virtual PhysicalFile::Page* allocatePage()
	{
		return m_pPhysicalFile->allocatePage2(*m_pTransaction,
											  m_eFixMode);
	}

	// 物理ページを解放する
	void freePage(Page* pPage_);

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

	// ディレクトリを削除する
	void rmdir();

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
	const Os::Path& getPath() const { return m_cPath; }

	// dirtyなページ数を得る
	ModSize getDirtyPageCount() const
		{ return m_mapPage.getSize() - m_iCurrentCacheCount; }

	// 排他制御用のクリティカルセクションを得る
	Os::CriticalSection& getLatch() const { return m_cLatch; }

protected:
	// 物理ページをアタッチする
	PhysicalFile::Page*	attachPhysicalPage(
		PhysicalFile::PageID uiPageID_,
		Buffer::Page::FixMode::Value eFixMode_
			= Buffer::Page::FixMode::Unknown);

	// 物理ページのFixModeを変更する
	PhysicalFile::Page* changeFixMode(PhysicalFile::Page* pPage_);

	// マップを検索して、ページを取り出す
	Page* findMap(PhysicalFile::PageID uiPageID_);

	// フリーされているページを１つ取り出す
	PhysicalFile::Page* getFreePage()
	{
		PhysicalFile::Page* pPhysicalPage = 0;
		Page* pPage = m_pFreeList;
		if (pPage)
		{
			m_pFreeList = pPage->m_pNext;
			pPhysicalPage = pPage->m_pPhysicalPage;
			delete pPage;
		}
		return pPhysicalPage;
	}

	// FixModeを得る
	Buffer::Page::FixMode::Value getFixMode() const { return m_eFixMode; }

	// ページをアタッチする
	virtual void attachPage(Page* pPage_);

	// ページをデタッチする
	virtual void detachPage(Page* pPage_);

	// トランザクションを得る
	const Trans::Transaction& getTransaction() const
	{
		return *m_pTransaction;
	}

	// インスタンスを得る
	Page* popPage();
	// インスタンスをつなげる
	void pushPage(Page* pPage_);

	// ファイルの中身をクリアする
	virtual void clear()
	{
		m_pPhysicalFile->clear(*m_pTransaction, false);
	}

	// 物理ファイル
	PhysicalFile::File* m_pPhysicalFile;

	// サブファイルかどうか
	bool m_bSubFile;
	
private:
	// デタッチされているページの内、dirtyじゃないページのみ開放する
	void detachNoDirtyPage();

	typedef ModMap<PhysicalFile::PageID, Page*,
				   ModLess<PhysicalFile::PageID> > Map;

	// デタッチしたページをキャッシュしておくマップ
	Map m_mapPage;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;

	// パス
	Os::Path m_cPath;

	// 整合性検査モードかどうか
	bool m_bVerification;
	// 整合性検査結果を格納する箱
	Admin::Verification::Progress* m_pProgress;
	// 整合性検査の実行モード
	Admin::Verification::Treatment::Value m_uiTreatment;
	// フリーリスト
	Page* m_pFreeList;

	// インスタンスフリーリスト
	Page* m_pInstanceFreeList;

	// キャッシュしておく数
	int m_iCacheCount;
	// 現在のキャッシュ数
	int m_iCurrentCacheCount;

	// 排他制御用
	mutable Os::CriticalSection m_cLatch;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_FILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
