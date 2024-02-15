// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataFile.h -- キー値である多次元ベクトルを格納するファイル
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

#ifndef __SYDNEY_KDTREE_DATAFILE_H
#define __SYDNEY_KDTREE_DATAFILE_H

#include "KdTree/Module.h"
#include "KdTree/MultiFile.h"

#include "KdTree/Entry.h"
#include "KdTree/FileID.h"

#include "PhysicalFile/Types.h"

#include "Common/BitSet.h"
#include "Common/LargeVector.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class AreaFile;
class Allocator;
class VectorFile;

//
//	CLASS
//	KdTree::DataFile -- 多次元ベクトルを格納するファイル
//
//	NOTES
//
class DataFile : public MultiFile
{
public:
	// ページID
	typedef PhysicalFile::PageID PageID;
	
	// 無効なページID
	enum {
		IllegalPageID = PhysicalFile::ConstValue::UndefinedPageID
	};
	
	// コンストラクタ
	DataFile(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~DataFile();

	// ファイルに挿入されているタプル数を得る
	virtual ModUInt32 getCount() = 0;
	
	// 挿入する
	virtual void insert(const Entry& cEntry_) = 0;
	// 削除する
	virtual void expunge(ModUInt32 uiRowID_) = 0;
	// 取得する
	virtual bool get(ModUInt32 uiRowID_, Entry& cEntry_) = 0;

	// 全数検索する
	virtual void getAll(Common::BitSet& cBitSet_) = 0;

	// 指定されたベクターのページIDに格納されているエントリを得る
	virtual void getPageData(PageID uiPageID_,
							 Allocator& allocator_,
							 Common::LargeVector<Entry*>& vecpEntry_) = 0;

	// 次のページIDを得る
	virtual PageID getNextPageID(PageID uiCurrentPageID_) = 0;

	// 1ページに格納できるエントリ数を得る
	virtual ModSize getCountPerPage() = 0;

	// コピーを得る
	virtual DataFile* copy() = 0;

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

	// ファイルの内容をクリアする
	virtual void clear() = 0;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_DATAFILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
