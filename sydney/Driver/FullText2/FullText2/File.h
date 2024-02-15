// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_FILE_H
#define __SYDNEY_FULLTEXT2_FILE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

#include "Trans/Transaction.h"
#include "Trans/TimeStamp.h"

#include "Admin/Verification.h"

#include "Buffer/Page.h"

#include "Lock/Name.h"

#include "Os/Path.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//	CLASS
//	FullText2::File -- 全文索引のすべてのファイル共通の基底クラス
//
//	NOTES
//
//
class File
{
public:
	// コンストラクタ
	File();
	// デストラクタ
	virtual ~File();
	
	// ファイルサイズを得る
	virtual ModUInt64 getSize() = 0;
	// 利用サイズを得る
	virtual ModUInt64 getUsedSize(const Trans::Transaction& cTransaction_);

	// 1ページを得るコストを得る
	static double getOverhead(ModSize uiPageSize_);

	// ファイルを作成する
	virtual void create() = 0;
	// ファイルを破棄する
	virtual void destroy() = 0;
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

	//論理ファイルをオープンする
	virtual void open(const Trans::Transaction& cTransaction_,
					  Buffer::Page::FixMode::Value eFixMode_) = 0;
	//論理ファイルをクローズする
	virtual void close() = 0;

	// 同期をとる
	virtual void sync(const Trans::Transaction& cTransaction_,
					  bool& incomplete, bool& modified) = 0;

	//ファイルを移動する
	virtual void move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cPath_) = 0;

	// 整合性検査を行う
	virtual void verify(const Trans::Transaction& cTransaction_,
						const Admin::Verification::Treatment::Value eTreatment_,
						Admin::Verification::Progress& cProgress_) = 0;

	// すべてのページの更新を確定する
	virtual void flushAllPages() = 0;
	// すべてのページの更新を破棄する
	virtual void recoverAllPages() = 0;

	// マウントされているか調べる
	virtual bool isMounted(const Trans::Transaction& trans) const = 0;
	// 実体である OS ファイルが存在するか調べる
	virtual bool isAccessible(bool bForce_ = false) const = 0;

protected:
	// ディレクトリを削除する
	void rmdir(const Os::Path& cPath_);

	// ロック名を得る
	virtual const Lock::FileName& getLockName() const = 0;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FILE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
