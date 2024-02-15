// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.h --
// 
// Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_BTREEFILE_H
#define __SYDNEY_KDTREE_BTREEFILE_H

#include "KdTree/Module.h"
#include "KdTree/SubFile.h"

#include "KdTree/BtreePage.h"
#include "KdTree/FileID.h"
#include "KdTree/HeaderPage.h"

#include "Common/BitSet.h"
#include "Common/VectorMap.h"

#include "PhysicalFile/File.h"

#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::BtreeFile --
//
//	NOTES
//	
//
class BtreeFile : public SubFile
{
	friend class BtreePage;
	
public:
	// コンストラクタ
	BtreeFile(FileID& cFileID_, const Os::Path& cPath_);
	// デストラクタ
	virtual ~BtreeFile();

	// エントリ数を得る
	ModUInt32 getCount();

	// 挿入
	void insert(ModUInt32 uiRowID_, const PhysicalFile::DirectArea::ID& id_);
	// 削除
	void expunge(ModUInt32 uiRowID_);

	// 取得
	bool get(ModUInt32 uiRowID_, PhysicalFile::DirectArea::ID& id_);
	// 全件取得
	void getAll(Common::BitSet& cBitSet_);

	// 指定ページ内のキーとバリューを得る
	void getPageData(PhysicalFile::PageID pageID_,
					 Common::LargeVector<
					 	ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >&
					 vecData_);

	// 中身を空にする
	void clear();

	// 次のリーフページのページIDを得る
	PhysicalFile::PageID
	getNextLeafPageID(PhysicalFile::PageID uiCurrentPageID);

	// ファイルを作成する
	void create();

	// クローズする
	void close();

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// ページを確保する
	BtreePage::PagePointer allocatePage(PhysicalFile::PageID uiPrevPageID_,
										PhysicalFile::PageID uiNextPageID_,
										const BtreePage::PagePointer& pParentPage_);
	// ページを解放する
	void freePage(BtreePage* pPage_);
	
	// ページをアタッチする
	BtreePage::PagePointer attachPage(PhysicalFile::PageID uiPageID_,
									  Buffer::Page::FixMode::Value eFixMode_,
									  const BtreePage::PagePointer& pParentPage_);
	// ページをデタッチする
	void detachPage(Page* pPage_);

	// fixモードを変更する
	PhysicalFile::Page* changeFixMode(PhysicalFile::Page* pPage_);

	// デタッチされている全ページをフラッシュする
	void flushAllPages();
	// デタッチされている全ページを元に戻す
	void recoverAllPages();

	// 1ページに格納できるエントリ数を得る
	ModSize getCountPerPage();

	// 親ページを検索する
	BtreePage::PagePointer findPage(const BtreePage::Entry* pEntry_,
									PhysicalFile::PageID uiChildPageID_);
	
private:
	// リーフページを得る
	BtreePage::PagePointer getLeafPage(const BtreePage::Entry* pEntry_);
	// ルートページを得る
	BtreePage::PagePointer getRootPage();

	// ヘッダーページを得る
	HeaderPage* getHeaderPage();

	// MAP
	typedef Common::VectorMap<PhysicalFile::PageID, BtreePage*,
							  ModLess<PhysicalFile::PageID> > PageMap;

	// ヘッダーページ
	HeaderPage* m_pHeaderPage;
	
	// ページマップ
	PageMap m_cPageMap;
	// フリーリスト
	BtreePage* m_pFree;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_BTREEFILE_H

//
//	Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
