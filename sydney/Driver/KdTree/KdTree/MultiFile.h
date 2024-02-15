// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiFile.h --
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

#ifndef __SYDNEY_KDTREE_MULTIFILE_H
#define __SYDNEY_KDTREE_MULTIFILE_H

#include "KdTree/Module.h"
#include "KdTree/File.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class FileID;

//	CLASS
//	KdTree::MultiFile
//		-- 複数のサブファイルを管理する仮想的なファイル共通の基底クラス
//
//	NOTES
//	単純に複数のサブファイルに同じメソッドを実行すればいいものを実行する
//
class MultiFile : public File
{
public:
	// コンストラクタ
	MultiFile(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~MultiFile();
	
	// ファイルサイズを得る
	ModUInt64 getSize();

	// ファイルを作成する
	void create();
	// ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// ファイルをマウントする
	void mount(const Trans::Transaction& cTransaction_);
	// ファイルをアンマウントする
	void unmount(const Trans::Transaction& cTransaction_);
	
	// ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  Buffer::Page::FixMode::Value eFixMode_);
	//論理ファイルをクローズする
	void close();

	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& incomplete, bool& modified);

	// すべてのページの更新を確定する
	void flushAllPages();
	// すべてのページの更新を破棄する
	void recoverAllPages();

	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const;
	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const;

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

protected:
	// 整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value eTreatment_,
						   Admin::Verification::Progress& cProgress_);
	// 整合性検査を終了する
	void endVerification();

	//
	// サブファイルを格納する配列の領域を確保する
	//
	void reserveSubFile(ModSize size_)
		{ m_vecpSubFile.reserve(size_); }

	//
	// サブファイルを追加する
	//
	//【注意】
	//	追加されたサブファイルのインスタンスの解放はサブクラスで行うこと
	//
	void pushBackSubFile(File* pSubFile_)
		{ m_vecpSubFile.pushBack(pSubFile_); }

	//
	// サブファイルを削除する
	//
	void clearSubFile()
		{ m_vecpSubFile.clear(); }

	// サブファイルを格納する配列
	ModVector<File*> m_vecpSubFile;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_MULTIFILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
