// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h --
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

#ifndef __SYDNEY_KDTREE_FILE_H
#define __SYDNEY_KDTREE_FILE_H

#include "KdTree/Module.h"
#include "KdTree/FileID.h"

#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"

#include "Admin/Verification.h"

#include "Buffer/Page.h"

#include "Lock/Name.h"

#include "Os/CriticalSection.h"
#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::File -- すべてのファイル共通の基底クラス
//
//	NOTES
//
//
class File
{
public:
	// コンストラクタ
	File(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~File();
	
	// ファイルサイズを得る
	virtual ModUInt64 getSize() = 0;

	// 1ページを得るコストを得る
	static double getOverhead(ModSize uiPageSize_);

	// ファイルを作成する
	virtual void create() = 0;
	// ファイルを破棄する
	virtual void destroy() { destroy(*m_pTransaction); }
	virtual void destroy(const Trans::Transaction& cTransaction_) = 0;

	// ファイルをマウントする
	virtual void mount(const Trans::Transaction& cTransaction_) = 0;
	// ファイルをアンマウントする
	virtual void unmount(const Trans::Transaction& cTransaction_) = 0;
	
	// ファイルをフラッシュする
	virtual void flush(const Trans::Transaction& cTransaction_) = 0;

	// ファイルのバックアップを開始する
	virtual void startBackup(const Trans::Transaction& cTransaction_,
							 const bool bRestorable_) = 0;
	// ファイルのバックアップを終了する
	virtual void endBackup(const Trans::Transaction& cTransaction_) = 0;

	// ファイルを障害から回復する
	virtual void recover(const Trans::Transaction& cTransaction_,
						 const Trans::TimeStamp& cPoint_) = 0;

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	virtual void restore(const Trans::Transaction& cTransaction_,
						 const Trans::TimeStamp& cPoint_) = 0;

	// 論理ファイルをオープンする
	virtual void open(const Trans::Transaction& cTransaction_,
					  Buffer::Page::FixMode::Value eFixMode_);
	// 論理ファイルをクローズする
	virtual void close();

	// 同期をとる
	virtual void sync(const Trans::Transaction& cTransaction_,
					  bool& incomplete, bool& modified) = 0;

	//ファイルを移動する
	virtual void move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cPath_) = 0;

	// すべての更新を確定する
	virtual void flushAllPages() = 0;
	// すべての更新を破棄する
	virtual void recoverAllPages() = 0;

	// マウントされているか調べる
	virtual bool isMounted(const Trans::Transaction& trans) const = 0;
	// 実体である OS ファイルが存在するか調べる
	virtual bool isAccessible(bool bForce_ = false) const = 0;

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

	// 排他制御用のCriticalSectionを得る
	Os::CriticalSection& getLatch() { return m_cLatch; }

	// FixModeを得る
	Buffer::Page::FixMode::Value getFixMode() const
		{ return m_eFixMode; }

	// 物理ファイルの整合性検査を開始する
	virtual void startVerification(
		const Trans::Transaction& cTransaction_,
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_);

	// 物理ファイルの整合性検査を終了する
	virtual void endVerification();

	// パスを得る
	const Os::Path& getPath() const { return m_cPath; }

protected:
	// ディレクトリを削除する
	void rmdir(const Os::Path& cPath_);

	// LockNameを得る
	const Lock::FileName& getLockName() const
		{ return m_cFileID.getLockName(); }
	
	// オープンされているか
	bool isOpened() const { return (m_pTransaction == 0) ? false : true; }

	// パスを設定する
	void setPath(const Os::Path& cNewPath_) { m_cPath = cNewPath_; }
		
	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FixMode
	Buffer::Page::FixMode::Value m_eFixMode;

	// 整合性検査モードかどうか
	bool m_bVerification;
	// 整合性検査結果を格納する箱
	Admin::Verification::Progress* m_pProgress;
	// 整合性検査の実行モード
	Admin::Verification::Treatment::Value m_uiTreatment;
	
	// FileID
	FileID& m_cFileID;
	// パス
	Os::Path m_cPath;

	// 排他制御用のCiriticalSection
	Os::CriticalSection m_cLatch;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_FILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
