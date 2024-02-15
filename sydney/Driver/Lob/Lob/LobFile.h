// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_LOBFILE_H
#define __SYDNEY_LOB_LOBFILE_H

#include "Lob/Module.h"
#include "Lob/File.h"
#include "Lob/Page.h"
#include "Lob/FileID.h"
#include "Lob/DataPage.h"
#include "Lob/DirPage.h"
#include "Lob/BlockPage.h"
#include "Lob/TopPage.h"
#include "Lob/AutoPointer.h"

#include "Common/Object.h"
#include "PhysicalFile/File.h"
#include "LogicalFile/File.h"
#include "Trans/Transaction.h"
#include "Os/Path.h"
#include "FileCommon/OpenOption.h"

#include "ModHashMap.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobData;

//
//	CLASS
//	Lob::LobFile -- 各型用のLobFileの共通基底クラス
//
//	NOTES
//
//
class LobFile : public File
{
public:
	//コンストラクタ
	LobFile(const FileID& cFileID_);
	//デストラクタ
	virtual ~LobFile();

	// オブジェクトの総数を得る
	ModInt64 getCount() const;

	// 存在をチェックする
	bool check(const ObjectID& cObjectID_);

	// 内容を取り出す
	AutoPointer<void> get(const ObjectID& cObjectID_,
						  ModSize uiPosition_, ModSize& uiLength_,
						  bool& isNull_);
	// データ長を得る
	ModSize getDataSize(const ObjectID& cObjectID_);

	// 挿入する
	ObjectID insert(const void* pBuffer_, ModSize uiLength_);
	// 削除する
	void expunge(const ObjectID& cObjectID_);
	// 削除を取り消す
	void undoExpunge(const ObjectID& cObjectID_);
	// 更新する
	void update(const ObjectID& cObjectID_,
				const void* pBuffer_, ModSize uiLength_);
	// 更新を取り消す
	void undoUpdate(const ObjectID& cObjectID_);
	
	// 追加
	void append(const ObjectID& cObjectID_,
				const void* pBuffer_, ModSize uiLength_);
	// 縮める
	void truncate(const ObjectID& cObjectID_, ModSize uiLength_);
	// 内容を取り替える
	void replace(const ObjectID& cObjectID_, ModSize uiPosition_,
				 const void* pBuffer_, ModSize uiLength_);

	// 削除するべきデータがあるかないか
	bool isExistExpungeData();
	// 削除データを１つ開放する
	bool compact();
	
	// ファイルを作成する
	void create();
	// 整合性検査を行う
	void verify();
	// 同期をとる
	void sync(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_);

	// 内容を確定する
	void flushAllPages();
	// 内容を破棄する
	void recoverAllPages();

	// オーバーヘッドコストを得る
	double getOverhead() const;
	// エントリ数を得る
	ModSize getEntryCount() const;

	// ブロックページを得る
	BlockPage::PagePointer attachBlockPage(PhysicalFile::PageID uiPageID_);
	BlockPage::PagePointer allocateBlockPage();
	
	// Dirページを得る
	DirPage::PagePointer attachDirPage(PhysicalFile::PageID uiPageID_);
	DirPage::PagePointer allocateDirPage(ModUInt32 uiStep_);
	
	// DATAページを得る
	DataPage::PagePointer attachDataPage(PhysicalFile::PageID uiPageID_);
	DataPage::PagePointer allocateDataPage(
									PhysicalFile::PageID uiPrevPageID_,
									PhysicalFile::PageID uiNextPageID_);

	// DATAページの格納可能データサイズを得る
	ModSize getDataPageDataSize()
	{
		return DataPage::getPageDataSize(m_cFileID.getPageSize());
	}

	// DIRページの最大エントリ数を得る
	ModSize getDirMaxCount()
	{
		return DirPage::getMaxCount(m_cFileID.getPageSize());
	}

private:
	// トップページを初期化する
	void initializeTopPage();
	// トップページを得る
	TopPage* getTopPage(PhysicalFile::Page* pPage = 0);

	// 新しいBlockを得る
	BlockPage::Block* allocateBlock(BlockPage::PagePointer& pBlockPage_,
									ObjectID& cObjectID_);

	// 削除リストから取り除く
	void undoExpungeList(const ObjectID& cObjectID_);

	// LobDataを得る
	LobData* allocateLobData(BlockPage::PagePointer pBlockPage_,
							 BlockPage::Block* pBlock_);

	// FileID
	const FileID& m_cFileID;

	// トップページ
	TopPage* m_pTopPage;
	TopPage::PagePointer m_pTopPagePointer;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_LOBFILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
