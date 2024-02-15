// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MainFile.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_MAINFILE_H
#define __SYDNEY_FULLTEXT2_MAINFILE_H

#include "FullText2/Module.h"
#include "FullText2/File.h"
#include "FullText2/Page.h"
#include "FullText2/FileID.h"

#include "Common/Object.h"
#include "Common/DoubleLinkedList.h"
#include "Common/VectorMap.h"
#include "PhysicalFile/File.h"
#include "Trans/Transaction.h"
#include "Os/Path.h"
#include "FileCommon/OpenOption.h"

#include "ModOstream.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::MainFile --
//
//	NOTES
//	転置索引を構成する主要なファイルである
//		Ｂ木ファイル
//		リーフファイル
//		オーバーフローファイル
//	共通の基底クラス
//
class MainFile : public File
{
	friend class Page;

public:
	// ファイル種別
	struct Type
	{
		enum Value
		{
			Leaf,				// リーフファイル
			Overflow,			// オーバーフローファイル
			Btree				// B木ファイル
		};
	};

	// コンストラクタ
	MainFile(Type::Value eType_, int iCacheCount_);
	// デストラクタ
	virtual ~MainFile();

	// ファイルタイプを得る
	Type::Value getType() const { return m_eType; }

	// 物理ファイルをアタッチする
	void attach(const FileID& cFileID_, int iPageSize_,
				const Os::Path& cFilePath_, const Os::Path& cPart_,
				bool batch_);
	// 物理ファイルをデタッチする
	void detach();

	// ファイルサイズを得る
	ModUInt64 getSize() const { return m_pPhysicalFile->getSize(); }
	// 使用ファイルサイズを得る
	virtual ModUInt64 getUsedSize(const Trans::Transaction& cTransaction_)
	{
		return m_pPhysicalFile->getUsedSize(cTransaction_);
	}

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy() { destroy(*m_pTransaction); }
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
				 const Trans::TimeStamp& cPoint_)
	{
		m_pPhysicalFile->recover(cTransaction_, cPoint_);
	}
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
	{
		m_pPhysicalFile->restore(cTransaction_, cPoint_);
	}

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);
	//論理ファイルをクローズする
	void close();

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
	virtual bool isMounted() const
	{
		return isMounted(*m_pTransaction);
	}

	// ファイルをクリアする
	virtual void clear(const Trans::Transaction& cTransaction_,	bool bForce_);

	// 物理ページを確保する
	virtual PhysicalFile::Page* allocatePage()
	{
		return m_pPhysicalFile->allocatePage2(*m_pTransaction, m_eFixMode);
	}

	// 物理ページを解放する
	void freePage(Page* pPage_);

	// デタッチされている全ページをフラッシュする
	void flushAllPages();
	// デタッチされている全ページを元に戻す
	void recoverAllPages();
	// デタッチされている全ページ内容を保存する
	virtual void saveAllPages();

	// ページの使用可能サイズを得る
	ModUInt32 getPageDataSize() const
	{
		return m_pPhysicalFile->getPageDataSize();
	}

	// 1ページを得るコストを得る
	static double getOverhead(ModSize uiPageSize_);

	// ディレクトリのみ削除する
	virtual void rmdir();

	// 物理ファイルの整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_);

	// 物理ファイルの整合性検査を終了する
	void endVerification();

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
	bool isCancel()
	{
		return m_pTransaction->isCanceledStatement();
	}

	// 整合性検査の経過を表すクラスを得る
	Admin::Verification::Progress& getProgress() { return *m_pProgress; }

	// ルートパスを得る
	const Os::Path& getRootPath() const { return m_cRootPath; }

protected:
	// 物理ページをアタッチする
	virtual PhysicalFile::Page* attachPhysicalPage(
		PhysicalFile::PageID uiPageID_,
		Buffer::ReplacementPriority::Value eReplacementPriority_
			= Buffer::ReplacementPriority::Low);
	// 物理ページの内容を確定する
	virtual void detachPhysicalPage(PhysicalFile::Page*& pPhysicalPage)
	{
		PhysicalFile::Page::UnfixMode::Value mode
			= (pPhysicalPage->getUnfixMode()
			   == PhysicalFile::Page::UnfixMode::Dirty) ?
			PhysicalFile::Page::UnfixMode::Dirty :
			PhysicalFile::Page::UnfixMode::NotDirty;
		m_pPhysicalFile->detachPage(pPhysicalPage, mode);
	}
		
	// 物理ページの内容を破棄する
	virtual void recoverPhysicalPage(PhysicalFile::Page*& pPhysicalPage)
	{
		m_pPhysicalFile->recoverPage(pPhysicalPage);
	}
		

	// マップを検索して、ページを取り出す
	Page* findMap(PhysicalFile::PageID uiPageID_);

	// フリーされているページを１つ取り出す
	Page* getFreePage()
	{
		Page* pPage = m_pFreeList;
		if (pPage)
		{
			m_pFreeList = pPage->m_pNext;
			pPage->m_bFree = false;
		}
		return pPage;
	}

	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cFilePath_,
			  const Os::Path& cDirectory_);

	// FixModeを得る
	Buffer::Page::FixMode::Value getFixMode() const { return m_eFixMode; }

	// ページをアタッチする
	void attachPage(Page* pPage_);

	// ページをデタッチする
	void detachPage(Page* pPage_);

	// ページを保存する
	void savePage(Page* pPage_);

	// オープンされているか
	bool isOpened() const { return (m_pTransaction == 0) ? false : true; }

	// インスタンスリストからpopする
	Page* popInstanceList();

#ifndef SYD_COVERAGE
	// ファイル状態を出力する
	virtual void reportFile(const Trans::Transaction& cTransaction_,
							ModOstream& stream_) {}

	// 先頭の物理ページIDを得る
	PhysicalFile::PageID getTopPageID(const Trans::Transaction& cTransaction_)
	{
		return m_pPhysicalFile->getTopPageID(cTransaction_);
	}

	// 次の物理ページIDを得る
	PhysicalFile::PageID getNextPageID(const Trans::Transaction& cTransaction_,
									   PhysicalFile::PageID uiPrevPageID_)
	{
		return m_pPhysicalFile->getNextPageID(cTransaction_, uiPrevPageID_);
	}

	// ページ数を得る
	ModSize getUsedPageNum(const Trans::Transaction& cTransaction_)
	{
		return m_pPhysicalFile->getUsedPageNum(cTransaction_);
	}
#endif

private:
	// LRUリスト
	typedef Common::DoubleLinkedList<Page>	PageList;
	// MAP
	typedef Common::VectorMap<PhysicalFile::PageID, Page*,
							  ModLess<PhysicalFile::PageID> > PageMap;

	// インスタンスリストにpushする
	void pushInstanceList(Page* pPage_, PhysicalFile::PageID id_);
	// 不要なページを削除する
	void detachNoDirtyPage();
	
	// LRUリスト
	PageList m_cPageList;
	// MAP
	PageMap m_cPageMap;

	// 物理ファイル
	PhysicalFile::File* m_pPhysicalFile;
	// ファイルタイプ
	Type::Value m_eType;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;

	// パス
	Os::Path m_cPath;
	// ルートパス
	Os::Path m_cRootPath;

	// 整合性検査モードかどうか
	bool m_bVerification;
	// 整合性検査結果を格納する箱
	Admin::Verification::Progress* m_pProgress;
	// 整合性検査の実行モード
	Admin::Verification::Treatment::Value m_uiTreatment;

	// フリーリスト
	Page* m_pFreeList;
	// インスタンスリスト
	Page* m_pInstanceList;

	// キャッシュしておく数
	int m_iCacheCount;
	// 現在のキャッシュ数
	int m_iCurrentCacheCount;
	// 現在の不要インスタンス数
	int m_iFreeInstanceCount;

	// attachカウンター
	int m_iAttachCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MAINFILE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
