// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorDataFile.h -- キー値である多次元ベクトルを格納するファイル
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

#ifndef __SYDNEY_KDTREE_VECTORDATAFILE_H
#define __SYDNEY_KDTREE_VECTORDATAFILE_H

#include "KdTree/Module.h"
#include "KdTree/DataFile.h"

#include "KdTree/Entry.h"
#include "KdTree/FileID.h"
#include "KdTree/AreaVectorFile.h"

#include "Common/BitSet.h"
#include "Common/LargeVector.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class AreaFile;
class Allocator;

//	CLASS
//	KdTree::VectorDataFile	-- 多次元ベクトルを格納するファイル
//
//	NOTES
//
class VectorDataFile : public DataFile
{
public:
	//コンストラクタ
	VectorDataFile(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~VectorDataFile();

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);
	
	// ファイルに挿入されているタプル数を得る
	ModUInt32 getCount() { return m_pVectorFile->getCount(); }
	
	// 挿入する
	void insert(const Entry& cEntry_);
	// 削除する
	void expunge(ModUInt32 uiRowID_);
	// 取得する
	bool get(ModUInt32 uiRowiD_, Entry& cEntry_);

	// 挿入されているか？
	bool test(ModUInt32 uiRowID_);

	// 全数検索する
	void getAll(Common::BitSet& cBitSet_);

	// 指定されたページIDに格納されているエントリを得る
	void getPageData(PageID uiPageID_,
					 Allocator& allocator_,
					 Common::LargeVector<Entry*>& vecpEntry_);

	// 次のページIDを得る
	PageID getNextPageID(PageID uiCurrentPageID_);

	// 1ページに格納できるエントリ数を得る
	ModSize getCountPerPage() { return m_pVectorFile->getCountPerPage(); }

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
	// ベクターファイル
	AreaVectorFile* m_pVectorFile;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_VECTORDATAFILE_H

//
//	Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
