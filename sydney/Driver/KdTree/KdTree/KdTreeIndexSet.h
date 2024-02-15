// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeIndexSet.h -- KD-Tree索引のリスト
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_KDTREEINDEXSET_H
#define __SYDNEY_KDTREE_KDTREEINDEXSET_H

#include "KdTree/Module.h"

#include "KdTree/KdTreeIndex.h"

#include "Common/DoubleLinkedList.h"

#include "Lock/Name.h"

#include "Os/CriticalSection.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class FileID;
class DataFile;
class IndexFile;

//	CLASS
//	KdTree::KdTreeIndexSet --
//
//	NOTES
//
class KdTreeIndexSet
{
public:
	// コンストラクタ
	KdTreeIndexSet();
	// デストラクタ
	virtual ~KdTreeIndexSet();

	// 初期化する
	static void initialize();
	// 後処理する
	static void terminate();
	
	// 索引を得る - 必要に応じてファイルからロードされる
	static KdTreeIndexSet* attach(const Trans::Transaction& cTransaction_,
								  FileID& cFileID_);
	// 索引をメモリ上から削除
	static void drop(FileID& cFileID_);

	// 索引を作成する
	static void create(const Trans::Transaction& cTransaction_,
					   const FileID& cFileID_,
					   DataFile& cDataFile_,
					   IndexFile& cIndexFile_,
					   KdTreeIndex::Direction& cDirection_);

	// 検索用の索引を得る
	const KdTreeIndex* attachMain(const Trans::Transaction& cTransaction_);

	// 検索用の索引を得る
	const KdTreeIndex* attachLog1(const Trans::Transaction& cTransaction_);
	const KdTreeIndex* attachLog2(const Trans::Transaction& cTransaction_);

	// 更新用の索引を得る
	KdTreeIndex* allocateLog1(const Trans::Transaction& cTransaction_);
	KdTreeIndex* allocateLog2(const Trans::Transaction& cTransaction_);

	// 不要な版を削除する
	bool discard();

	// ラッチを得る
	Os::CriticalSection& getLatch() { return m_cLatch; }

	// ロードしたか？
	bool isLoaded() const { return m_bLoaded; }

private:
	// 検索用の索引を得る
	const KdTreeIndex*
		traverseIndex(const Trans::Transaction& cTransaction_,
					  Common::DoubleLinkedList<KdTreeIndex>& cList_);
	// 更新用の索引を得る
	KdTreeIndex* allocateIndex(const Trans::Transaction& cTransaction_,
							   Common::DoubleLinkedList<KdTreeIndex>& cList_);

	// 不要な索引を削除する
	bool discardIndex(const Trans::TimeStamp& cTimeStamp_,
					  Common::DoubleLinkedList<KdTreeIndex>& cList_);
	
	// メイン索引を設定する
	void setIndex(KdTreeIndex* pIndex_);
	
	// 索引をロードする
	void load(const Trans::Transaction& cTransaction_,
			  FileID& cFileID_);
	
	// リストを解放する
	void freeList(Common::DoubleLinkedList<KdTreeIndex>& list);
	
	// 本体のリスト
	Common::DoubleLinkedList<KdTreeIndex> m_cMainList;
	// 小索引
	Common::DoubleLinkedList<KdTreeIndex> m_cSmall1List;
	Common::DoubleLinkedList<KdTreeIndex> m_cSmall2List;
	
	// 排他制御用のラッチ
	Os::CriticalSection m_cLatch;
	
	// ロード済みか否か
	bool m_bLoaded;

	// 次元数
	int m_iDimension;
	// ロック名
	Lock::FileName m_cLockName;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_KDTREEINDEXSET_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
