// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayFile.h --
// 
// Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_ARRAYFILE_H
#define __SYDNEY_ARRAY_ARRAYFILE_H

#include "Array/Module.h"

#include "Array/Data.h"
#include "Array/DataPage.h"
#include "Array/DataTree.h"
#include "Array/File.h"
#include "Array/HeaderPage.h"
#include "Array/NullArrayTree.h"
#include "Array/NullDataTree.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
	class BitSet;
	class DataArrayData;
}

_SYDNEY_ARRAY_BEGIN

class Condition;

//
//	CLASS
//	Array::ArrayFile --
//
//	NOTES
//
//
class ArrayFile : public File
{
	friend class Page;

public:
	//コンストラクタ
	ArrayFile(const FileID& cFileID_);
	//デストラクタ
	virtual ~ArrayFile();

	// ファイル作成
	void create();

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	// クローズする
	void close();

	// Get the count of all the tuples.
	ModSize getTupleCount() { return getHeaderPage()->getTupleCount(); }

	// 検索結果件数を見積もる
	ModUInt32 getEstimateCount(Condition* pCondition_);

	// Search the page in which the searched entry may be included.
	void search(Condition* pCondition_);
	// 取得 -- 配列で
	bool get(Common::DataArrayData& cData_, unsigned int& uiTupleID_);
	// 取得 -- ビットセットで
	void getByBitSet(Common::BitSet& cBitSet_);

	// Check whether the entry which satisfies the conditions exists.
	bool checkEntry(Condition* pCondition_,
					unsigned int uiRowID_, unsigned int index_);
	
	// 挿入
	void insert(const Common::DataArrayData& cTuple_);
	// Update
	void update(const Common::DataArrayData& cTuple_,
				const Common::DataArrayData& cKey_);
	// 削除
	void expunge(const Common::DataArrayData& cTuple_);

	// 整合性チェック
	void verify();

	// マーク
	bool mark();
	// リワインド
	bool rewind();

	// 全ページをフラッシュする
	void flushAllPages();
	// 全ページを元に戻す
	void recoverAllPages();

	// 検索ページをdetachする
	void detachSearchPage() { m_pSearchPage = 0; }

	// ヘッダーページをdirtyにする
	void dirtyHeaderPage();

	// 更新が確定したPageIDを登録する
	void pushPageID(PhysicalFile::PageID uiPageID_)
	{
		m_vecPageID.pushBack(uiPageID_);
	}

	// ヘッダーページを得る
	HeaderPage::PagePointer getHeaderPage(PhysicalFile::Page* pPage = 0);

	// 整合性検査時にリーフページのエントリ数を加える
	void addVerifyEntryCount(ModSize uiCount_)
	{
		m_uiVerifyCount += uiCount_;
	}
	
	// ページを得る
	DataPage::PagePointer	attachPage(Tree* pTree_,
									   PhysicalFile::PageID uiPageID_,
									   const DataPage::PagePointer& pParent_,
									   Buffer::Page::FixMode::Value eFixMode_
									   = Buffer::Page::FixMode::Unknown);

	// 新しいページを得る
	DataPage::PagePointer allocatePage(Tree* pTree_,
									   PhysicalFile::PageID uiPrevPageID_,
									   PhysicalFile::PageID uiNextPageID_,
									   const DataPage::PagePointer& pParent_);

	// 親ページを検索する
	DataPage::PagePointer findParentPage(Tree* pTree_,
										 const ModUInt32* pBeginEntry_,
										 PhysicalFile::PageID uiChildPageID_);

private:
	// Get Tree.
	Tree* getTree(Tree::Type::Value);

	// Insert one data.
	void insertOneData(Tree* pTree_, const Common::DataArrayData& cData_);
	// Expunge one data.
	void expungeOneData(Tree* pTree_, const Common::DataArrayData& cData_);

	// 整合性チェック
	void verifyOneTree(Tree* pTree_);

	// ロック情報を登録する
	void insertLock(PhysicalFile::PageID uiPageID_);
	// ロック情報を削除する
	void eraseLock(PhysicalFile::PageID uiPageID_);
	// ロック情報が存在しているか
	bool checkLock(PhysicalFile::PageID uiPageID_);

	// ページをデタッチする
	void detachPage(Page* pPage_);

	// ヘッダーページを初期化する
	void initializeHeaderPage();

	// Get leaf.
	DataPage::PagePointer getLeafPage(
		Tree* pTree_,
		const ModUInt32* pBuffer_,
		const Compare& cCompare_,
		LogicalFile::TreeNodeInterface::Type eType_
		= LogicalFile::TreeNodeInterface::GreaterThanEquals);
	
	// Get root.
	DataPage::PagePointer getRootPage(Tree* pTree_);
	// Create root.
	DataPage::PagePointer createRootPage(Tree* pTree_);

	// 次の検索結果を設定する
	void next(Tree* pTree_);

	// Attach the previous searched Page.
	DataPage::PagePointer attachPreviousSearchPage(Tree* pTree_);
	// Get the searched entry.
	DataPage::Iterator getSearchEntry(Tree* pTree_, int& iSearchEntryPosition_);
	// Attach the previous searched Page.
	DataPage::PagePointer attachNextPage(Tree* pTree_);

	// リーフエントリをコピーする
	void copyEntry(ModUInt32* dst, DataPage::Iterator i_);

	// 検索結果数を見積もる(for 検索)
	ModUInt32 getEstimateCountForSearch(Tree* pTree_,
										Condition* pCondition_);
	// 検索結果数を見積もる(for フェッチ)
	ModUInt32 getEstimateCountForFetch(Tree* pTree_,
									   Condition* pCondition_);

	// Check the tuple.
	bool checkTuple(const Common::DataArrayData& cTuple_) const;
	// Check the tuple.
	bool checkKey(const Common::DataArrayData& cKey_) const;

	// Get the RowID from Tuple's DataArrayData.
	unsigned int getRowID(const Common::DataArrayData& cTuple_,
						  int iPosition_) const;
	// Set the RowID to Tuple's DataArrayData.
	void setRowID(Common::DataArrayData& cTuple_,
				  int iPosition_,
				  unsigned int uiRowID);
	
	// Get the Value's array from Tuple's DataArrayData.
	const Common::DataArrayData&
	getArray(const Common::DataArrayData& cTuple_,
			 int iPosition_) const;
	// Set the Value's array to Tuple's DataArrayData.
	void setArray(Common::DataArrayData& cTuple_,
				  int iPosition_,
				  const Common::Data& cArray_);

	// Initialize DataArrayData.
	void initializeDataArrayData(Common::DataArrayData& cData_,
								 const Tree* pTree_,
								 unsigned int uiRowID_) const;
	// Set DataArrayData.
	void setDataArrayData(Common::DataArrayData& cData_,
						  const Tree* pTree_,
						  const Common::Data::Pointer pElement_,
						  unsigned int uiIndex_) const;
	void setDataArrayData(Common::DataArrayData*& pTemp_,
						  Tree*& pTree_,
						  Common::DataArrayData& cData_,
						  Common::DataArrayData& cNullData_,
						  const Common::DataArrayData& cArray_,
						  unsigned int uiIndex_);

	// ページ内のヒット件数を確認する
	ModSize getHitCount(DataPage::PagePointer pPage_,
						Condition* pCondition_,
						const Compare& cCompare_,
						DataPage::Iterator& l_,
						DataPage::Iterator& u_,
						bool isLower_ = false);

	// FileID
	const FileID& m_cFileID;

	// Tree
	DataTree m_cDataTree;
	NullDataTree m_cNullDataTree;
	NullArrayTree m_cNullArrayTree;

	// is Tree initialized?
	bool m_bTreeInitialize;

	// HeaderPage
	HeaderPage::PagePointer m_pHeaderPage;
	// Pointer to HeaderPage
	// This instance is cached.
	HeaderPage* m_pHeaderPageInstance;

	// 更新が確定したページID
	ModVector<PhysicalFile::PageID> m_vecPageID;

	// 整合性検査時のリーフページのエントリ数
	ModSize m_uiVerifyCount;

	// 検索しているページ
	DataPage::PagePointer m_pSearchPage;
	
	// 直前にgetした情報
	PhysicalFile::PageID m_uiSearchPageID;		// ページID
	int m_iSearchEntryPosition;					// エントリ位置
	ModUInt32 m_pSearchEntryBuffer[FileID::MAX_SIZE];	// エントリ内容

	// マークした情報
	PhysicalFile::PageID m_uiMarkPageID;		// ページID
	int m_iMarkEntryPosition;					// エントリ位置
	ModUInt32 m_pMarkEntryBuffer[FileID::MAX_SIZE];	// エントリ内容
	
	// 検索条件
	Condition* m_pCondition;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_ARRAYFILE_H

//
//	Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
