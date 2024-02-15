// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.h -- B木部分のページ
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_BTREEPAGE_H
#define __SYDNEY_BITMAP_BTREEPAGE_H

#include "Bitmap/Module.h"
#include "Bitmap/NodePage.h"
#include "Bitmap/PagePointer.h"
#include "Bitmap/Data.h"
#include "Bitmap/Compare.h"
#include "Bitmap/AutoPointer.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BtreeFile;

//
//	CLASS
//	Bitmap::BtreePage -- B木部分のページ
//
//	NOTES
//
class BtreePage : public NodePage
{
	friend class BtreeFile;
	
public:
	//
	//	STRUCT
	//	Bitmap::BtreePage::Header
	//
	struct Header
	{
		ModUInt32				m_uiCount;		// ページのエントリ数
		PhysicalFile::PageID	m_uiPrevPageID;	// 前のページ
		PhysicalFile::PageID	m_uiNextPageID;	// 後のページ
	};
	
	//
	//	TYPEDEF
	//	Bitmap::BtreePage::PagePointer
	//
	typedef PageObjectPointer<BtreePage> PagePointer;
	
	//
	//	TYPEDEF
	//	Bitmap::BtreePage::Iterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::Iterator Iterator;

	//
	//	TYPEDEF
	//	Bitmap::BtreePage::ConstIterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::ConstIterator ConstIterator;
	
	// コンストラクタ
	BtreePage(BtreeFile& cFile_);
	// デストラクタ
	virtual ~BtreePage();

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);
	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
						 PhysicalFile::PageID uiPrevPageID_,
						 PhysicalFile::PageID uiNextPageID_);

	// ページサイズを得る(UNIT単位)
	ModSize getPageSize();
	// 使用サイズを得る(UNIT単位)
	ModSize getUsedSize();
	// 空きサイズを得る(UNIT単位)
	ModSize getFreeSize();

	// 親を設定する
	void setParentPage(const BtreePage::PagePointer& pParent_)
	{
		m_pParentPage = pParent_;
	}

	// 検索する
	Iterator lowerBound(const ModUInt32* pValue_, const Compare& cCompare_);
	Iterator upperBound(const ModUInt32* pValue_, const Compare& cCompare_);
	Iterator find(const ModUInt32* pValue_, const Compare& cCompare_);

	// 挿入する
	bool insertEntry(const ModUInt32* pBuffer_, ModSize uiSize_ = 0);
	// 削除する
	bool expungeEntry(const ModUInt32* pBuffer_, ModSize uiSize_ = 0,
					  bool isReduce_ = true);
	// 更新する
	bool updateEntry(const ModUInt32* pBuffer_, ModSize uiSize_ = 0);

	// 指定位置に挿入
	void insert(Iterator i_, const ModUInt32* pBuffer_, ModSize uiSize_);
	void insert(Iterator i_, ConstIterator start_, ConstIterator end_);

	// 指定位置を削除
	void expunge(Iterator i_);
	void expunge(Iterator start_, Iterator end_);

	// 先頭を得る
	Iterator begin() { return m_vecpEntry.begin(); }
	// 最後を得る
	Iterator end() { return m_vecpEntry.end() - 1; }

	// 整合性検査
	void verify();

	// ルートページか
	bool isRoot() const;
	// リーフページか
	bool isLeaf() const;
	// リーフページに設定する
	void setLeaf();

	// 前のページ
	void setPrevPageID(PhysicalFile::PageID uiPageID_)
	{
		dirty();
		m_pHeader->m_uiPrevPageID = uiPageID_;
	}
	PhysicalFile::PageID getPrevPageID() const
		{ return m_pHeader->m_uiPrevPageID; }
	
	// 後のページ
	void setNextPageID(PhysicalFile::PageID uiPageID_)
	{
		dirty();
		m_pHeader->m_uiNextPageID = uiPageID_;
	}
	PhysicalFile::PageID getNextPageID() const
		{ return m_pHeader->m_uiNextPageID; }

	// ページのエントリ数を得る
	ModUInt32 getCount();
	
	// デタッチする
	void detach();
	// 物理ファイルをdetachする
	void detachPhysicalPage();

	// 参照を増やす
	void incrementReference() { m_iReference++; }
	// 参照を減らす
	int decrementReference() { return --m_iReference; }
	// 参照を得る
	int getReference() { return m_iReference; }

protected:
	// データ領域へのポインタを得る
	ModUInt32* getBuffer();

	// ページのエントリ数を増やす
	void addCount(ModUInt32 uiCount_);
	// ページのエントリ数を減らす
	void subtractCount(ModUInt32 uiCount_);

	// 物理ページが変更されたので読み直す
	void reload();

private:
	// 初期化
	void initialize();
	// ロードする
	void load(ModUInt32 count_);

	// 拡張 -- ページ分割か再配分
	PagePointer expand(const ModUInt32* pBuffer_, ModSize uiSize_);
	// 縮小 -- ページ連結か再配分
	void reduce();

	// ページ分割
	PagePointer split(PagePointer pPrevPage_, const ModUInt32* pBuffer_);
	// ページ連結
	void concatenate(PagePointer pPrevPage_);
	// 再配分
	PagePointer redistribute(PagePointer pPrevPage_,
							 const ModUInt32* pBuffer_ = 0);

	// ノードエントリを作成する
	void makeNodeEntry(ModUInt32* pEntry_,
					   ModUInt32* pBuffer_, ModSize& uiSize_);

	// データクラスを得る
	const Data& getDataClass();

	// モードを変更する

	// ファイル
	BtreeFile& m_cFile;
	
	// ページヘッダー
	Header* m_pHeader;
	
	// 親ページへのポインタ
	PagePointer m_pParentPage;

	ModVector<ModUInt32*> m_vecpEntry;
	
	// 参照カウンタ(参照する変数によって増減する)
	int m_iReference;

	// リスト用
	BtreePage* m_pNext;
	BtreePage* m_pPrev;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_BTREEPAGE_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
