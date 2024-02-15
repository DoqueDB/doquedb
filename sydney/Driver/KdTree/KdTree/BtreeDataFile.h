// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeDataFile.h -- キー値である多次元ベクトルを格納するファイル
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

#ifndef __SYDNEY_KDTREE_BTREEDATAFILE_H
#define __SYDNEY_KDTREE_BTREEDATAFILE_H

#include "KdTree/Module.h"
#include "KdTree/DataFile.h"

#include "KdTree/Entry.h"
#include "KdTree/FileID.h"
#include "KdTree/BtreeFile.h"

#include "Common/BitSet.h"
#include "Common/LargeVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class AreaFile;
class Allocator;
class IDVectorFile;

//	CLASS
//	KdTree::BtreeDataFile	-- 多次元ベクトルを格納するファイル
//
//	NOTES
//
class BtreeDataFile : public DataFile
{
public:
	// コンストラクタ
	BtreeDataFile(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~BtreeDataFile();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);
	
	// ファイルに挿入されているタプル数を得る
	ModUInt32 getCount() { return m_pBtreeFile->getCount(); }
	
	// 挿入する
	void insert(const Entry& cEntry_);
	// 削除する
	void expunge(ModUInt32 uiRowID_);
	// 取得する
	bool get(ModUInt32 uiRowiD_, Entry& cEntry_);

	// 全数検索する
	void getAll(Common::BitSet& cBitSet_);

	// 削除フラグから削除する
	void undoExpungedEntry(ModUInt32 uiRowID_);
	// 削除済みエントリの一覧を取得する
	void getExpungedEntry(Common::BitSet& cBitSet_);
	// 削除済みエントリ数を得る
	ModUInt32 getExpungedEntryCount();

	// 指定されたページIDに格納されているエントリを得る
	void getPageData(PageID uiPageID_,
					 Allocator& allocator_,
					 Common::LargeVector<Entry*>& vecpEntry_);

	// 次のページIDを得る
	PageID getNextPageID(PageID uiCurrentPageID_);

	// 1ページに格納できるエントリ数を得る
	ModSize getCountPerPage() { return m_pBtreeFile->getCountPerPage(); }

	// コピーを得る
	DataFile* copy();

	// ファイルの内容をクリアする
	void clear();

private:
	// ファイルをattachする
	void attach();
	// ファイルをdetachする
	void detach();
	
	// データファイルx
	AreaFile* m_pAreaFile;
	// Ｂ木ファイル
	BtreeFile* m_pBtreeFile;
	// 削除ファイル
	IDVectorFile* m_pExpungeFile;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_BTREEDATAFILE_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
