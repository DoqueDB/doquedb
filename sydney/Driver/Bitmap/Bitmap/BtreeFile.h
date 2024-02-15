// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.h --
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_BTREEFILE_H
#define __SYDNEY_BITMAP_BTREEFILE_H

#include "Bitmap/Module.h"
#include "Bitmap/File.h"
#include "Bitmap/BtreePage.h"
#include "Bitmap/HeaderPage.h"

#include "Common/DoubleLinkedList.h"

#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class Condition;

//
//	CLASS
//	Bitmap::BtreeFile -- BitmapのB木部分を管理するファイル 
//
//	NOTES
//
class BtreeFile : public File
{
	friend class BtreePage;
	
public:
	// コンストラクタ
	BtreeFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~BtreeFile();

	// 挿入する
	void insert(const Common::Data& cKey_,
				const Common::Data& cValue_,
				bool isNull_ = false);
	// 削除する
	void expunge(const Common::Data& cKey_,
				 const Common::Data& cValue_,
				 bool isNull_ = false);
	// 更新する
	void update(const Common::Data& cKey_,
				const Common::Data& cValue_,
				bool isNull_ = false);

	// 検索
	void search(Condition* pCondition_);
	// 取得
	bool get(Common::Data& cValue_);
	bool get(Common::Data& cKey_, Common::Data& cValue_);
	bool getWithLatch(Common::Data& cKey_, Common::Data& cValue_);
	bool get(const Common::Data& cKey_, bool isNull_,
			 Common::Data& cValue_);

	// 全ページをフラッシュする
	void flushAllPages();
	// 全ページを元に戻す
	void recoverAllPages();

	// ページを開放する
	void freePage(BtreePage* pPage_);

	// データクラスを得る
	const Data& getKeyData()
	{
		return m_cKeyData;
	}
	const Data& getNodeData()
	{
		return m_cNodeData;
	}
	// データクラスを得る
	const Data& getLeafData()
	{
		return m_cLeafData;
	}
	// 比較用のクラスを得る
	const Compare& getCompare()
	{
		return m_cCompare;
	}

	// ヘッダーページを得る
	virtual HeaderPage& getHeaderPage() = 0;

	// 整合性検査する
	void verify();

private:	
	// ページを得る
	BtreePage::PagePointer attachPage(PhysicalFile::PageID uiPageID_,
									  const BtreePage::PagePointer& pParent_,
									  Buffer::Page::FixMode::Value eFixMode_
									  = Buffer::Page::FixMode::Unknown);
	// 新しいページを得る
	BtreePage::PagePointer allocatePage(PhysicalFile::PageID uiPrevPageID_,
										PhysicalFile::PageID uiNextPageID_,
										const BtreePage::PagePointer& pParent_);

	// 親ページを検索する
	BtreePage::PagePointer findPage(const ModUInt32* pValue_,
									PhysicalFile::PageID uiChildPageID_);

	// データを作成する
	void makeData(const Common::Data& cKey_,
				  const Common::Data& cValue_,
				  const Data& cData_,
				  ModUInt32* pBuffer_,
				  ModSize& uiSize_);

	// リーフページを得る
	BtreePage::PagePointer getLeafPage(const ModUInt32* pBuffer_,
									   const Compare& cCompare_,
									   bool isLower_ = true);

	// ルートページを得る
	BtreePage::PagePointer getRootPage();

	// 検索前準備を行う
	void preSearch();
	// 次の検索結果を設定する
	bool next(Common::Data& cValue_, Common::Data* pKey_);

	// データクラスを作成する
	void createData();
	// 比較クラスを作成する
	void createCompare();

	// すべてのページをdetachする
	void detachAll();

	// データクラス
	Data m_cKeyData;
	Data m_cNodeData;
	Data m_cLeafData;
	// 比較クラス
	Compare m_cCompare;
	
	// 検索しているページ
	BtreePage::PagePointer m_pSearchPage;
	// 検索しているページでの位置
	int m_iSearchEntryPosition;
	// is null か？
	bool m_bIsNull;
	bool m_bIsNull_All;

	// 検索条件
	Condition* m_pCondition;

	// attachしたページをキャッシュするためのリスト
	typedef Common::DoubleLinkedList<BtreePage> PageList;
	PageList m_cList;

	// OpenMPのためのラッチ
	Os::CriticalSection m_cLatch;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_BTREEFILE_H

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
