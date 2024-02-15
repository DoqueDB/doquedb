// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeFile.h -- KD木ファイル
// 
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_KDTREEFILE_H
#define __SYDNEY_KDTREE_KDTREEFILE_H

#include "KdTree/Module.h"
#include "KdTree/MultiFile.h"

#include "KdTree/BtreeDataFile.h"
#include "KdTree/Entry.h"
#include "KdTree/FileID.h"
#include "KdTree/IndexFile.h"
#include "KdTree/InfoFile.h"
#include "KdTree/VectorDataFile.h"

#include "Common/LargeVector.h"
#include "Common/Thread.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class KdTreeIndex;
class KdTreeIndexSet;

//
//	CLASS
//	KdTree::KdTreeFile	-- KD木ファイル
//
//	NOTES
//
class KdTreeFile : public MultiFile
{
public:
	//コンストラクタ
	KdTreeFile(FileID& cFileID_);
	//デストラクタ
	virtual ~KdTreeFile();

	// バッチモードに設定する
	void setBatchMode();
	
	// オーバヘッドコストを得る
	double getOverhead() { return 0.0; }
	// プロセスコストを得る
	double getProcessCost() { return 0.0; }

	// エントリ数を得る
	int getCount() { return m_pDataFile->getCount(); }

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// クローズする
	void close();

	// 挿入する
	void insert(ModUInt32 uiRowID_,	const ModVector<float>& vecValue_);
	// 削除する
	void expunge(ModUInt32 uiRowID_);

	// 最近傍検索する
	void nnsearch(
		const ModVector<ModVector<float> >& vecCondition_,
		Node::TraceType::Value eTraceType_,
		int iMaxCalculateCount_,
		ModSize uiLimit_,
		ModVector<ModVector<ModPair<ModUInt32, double> > >& vecResult_);

	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const
		{ return m_pDataFile->isMounted(trans); }
	// 実態である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const
		{ return m_pDataFile->isAccessible(bForce_); }

	// マージ用のオープン
	void openForMerge(const Trans::Transaction& cTransaction_);
	// マージ用のクローズ
	void closeForMerge();
	// マージする
	void merge(const Common::Thread& cThread_);

private:
	// ファイルをattachする
	void attach();
	// ファイルをdetachする
	void detach();

	// エントリを確保する
	Entry* allocateEntry(ModUInt32 uiRowID_, const ModVector<float>& vecValue_);
	// エントリを解放する
	void freeEntry(Entry* pEntry_);

	// 更新のためにエグゼキュータ側の小索引を得る
	KdTreeIndex* allocateLog(KdTreeIndexSet* pIndexSet_);

	// 検索のためにエグゼキュータ側の小索引を得る
	const KdTreeIndex* attachLog(KdTreeIndexSet* pIndexSet_);
	// 検索のためにマージ側の小索引を得る
	const KdTreeIndex* attachLogForMerge(KdTreeIndexSet* pIndexSet_);

	// エグゼキュータ側の差分データファイルを得る
	BtreeDataFile* attachSmall();
	// マージ側の差分データファイルを得る
	BtreeDataFile* attachSmallForMerge();

	// 削除を反映する
	void reflectDeletedData(const Common::Thread& cThread_);
	// 挿入を反映する
	void reflectInsertedData(const Common::Thread& cThread_);
	// 索引を作成する
	void makeIndex(const Common::Thread& cThread_);
	// マージ側の差分ファイルの中身をクリアする
	void clearSmallFile();

	// 情報ファイル
	InfoFile* m_pInfoFile;
	// データファイル
	VectorDataFile* m_pDataFile;
	// 索引ファイル
	IndexFile* m_pIndexFile;

	// 差分データファイル
	BtreeDataFile* m_pSmallDataFile1;
	BtreeDataFile* m_pSmallDataFile2;

	// バッチモードかどうか
	bool m_bBatch;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_KDTREEFILE_H

//
//	Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
