// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.h --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_BTREEFILE_H
#define __SYDNEY_BTREE2_BTREEFILE_H

#include "Btree2/Module.h"
#include "Btree2/File.h"
#include "Btree2/FileID.h"
#include "Btree2/Data.h"
#include "Btree2/Compare.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/HeaderPage.h"

#include "LogicalFile/OpenOption.h"

#include "Common/Data.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"

#include "Trans/Transaction.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class Condition;

//
//	CLASS
//	Btree2::BtreeFile --
//
//	NOTES
//
//
class BtreeFile : public File
{
	friend class Page;

public:
	//コンストラクタ
	BtreeFile(const FileID& cFileID_, const Os::Path& cSubPath_);
	//デストラクタ
	virtual ~BtreeFile();

	// ファイル作成
	void create();

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOpenMode_);
	// クローズする
	void close();

	// 検索結果件数を見積もる
	ModUInt32 getEstimateCount(Condition* pCondition_);

	// 検索
	virtual void search(Condition* pCondition_, bool isReverse_) = 0;
	// 取得 -- 配列で
	virtual bool get(unsigned char ucBitSet_,
					 Common::DataArrayData& cTuple_,
					 unsigned int& uiTupleID_) = 0;
	// 取得 -- ビットセットで
	virtual void getByBitSet(ModVector<Condition*>& vecpCondition_,
							 int iFieldID_, Common::BitSet& cBitSet_) = 0;

	// 挿入
	virtual void insert(const Common::DataArrayData& cValue_) = 0;
	// 削除
	virtual void expunge(const Common::DataArrayData& cKey_) = 0;

	// 整合性チェック
	virtual void verify() = 0;

	// マーク
	virtual bool mark() = 0;
	// リワインド
	virtual bool rewind() = 0;

	// 不要なデータを削除する
	virtual void compact(const Trans::Transaction& cTransaction_,
						 bool& incomplete, bool& modified) = 0;

	// 全ページをフラッシュする
	virtual void flushAllPages();
	// 全ページを元に戻す
	virtual void recoverAllPages();

	// 検索ページをdetachする
	virtual void detachSearchPage() = 0;

	// ヘッダーページをdirtyにする
	void dirtyHeaderPage();

	// 物理ファイルの整合性検査を開始する
	void startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_)
	{
		m_uiVerifyCount = 0;
		File::startVerification(cTransaction_, uiTreatment_, cProgress_);
	}

	// データクラスを得る
	const Data& getKeyData()
	{
		return m_cKeyData;
	}
	const Data& getNodeData()
	{
		return m_cNodeData;
	}
	const Data& getLeafData()
	{
		return m_cLeafData;
	}

	// 比較用のクラスを得る
	const Compare& getCompare(bool onlyKey_ = false)
	{
		return onlyKey_ ? m_cKeyCompare : m_cCompare;
	}

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

	// ロック名を得る
	const Lock::FileName& getLockName() const
		{ return m_cFileID.getLockName(); }
	
protected:
	// 検索での結果件数の見積り
	virtual ModUInt32 getEstimateCountForSearch(Condition* pCondition_,
												ModUInt32 count_) = 0;
	// フェッチでの検索件数の見積り
	virtual ModUInt32 getEstimateCountForFetch(Condition* pCondition_,
											   ModUInt32 count_) = 0;
	
	// ロック情報を登録する
	void insertLock(PhysicalFile::PageID uiPageID_);
	// ロック情報を削除する
	void eraseLock(PhysicalFile::PageID uiPageID_);
	// ロック情報が存在しているか
	bool checkLock(PhysicalFile::PageID uiPageID_);

	// ページをデタッチする
	void detachPage(Page* pPage_);

	// 可能なら、制約ロックのためのエントリを削除する
	void expungeConstraintLockEntry(Trans::Transaction& cTrans_,
									const Lock::FileName& name_,
									bool& modified_);

	// すべてのエントリの行ロックを試みる
	bool tryLockAllEntry(Trans::Transaction& cTrans_,
						 const Lock::FileName& name_);
	// ファイルの中身をクリアする (サブファイルのみ)
	void clear(Trans::Transaction& cTrans_,
			   const Lock::FileName& name_);

	// FileID
	const FileID& m_cFileID;

	// 整合性検査時のリーフページのエントリ数
	ModSize m_uiVerifyCount;

private:
	// ヘッダーページを初期化する
	void initializeHeaderPage();
	
	// データクラスを作成する
	void createData();
	// 比較クラスを作成する
	void createCompare();

	// キーのサイズを得る
	ModSize getKeySize();
	// ノードのサイズを得る
	ModSize getNodeSize();
	// リーフのサイズを得る
	ModSize getLeafSize();

	// ヘッダーページ
	HeaderPage::PagePointer m_pHeaderPage;
	// ヘッダーページのポインタ
	HeaderPage* m_pHeaderPageInstance;

	// データクラス
	Data m_cKeyData;
	Data m_cNodeData;
	Data m_cLeafData;

	// 比較クラス
	Compare m_cCompare;
	Compare m_cKeyCompare;

	// 更新が確定したページID
	ModVector<PhysicalFile::PageID> m_vecPageID;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_FILE_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
