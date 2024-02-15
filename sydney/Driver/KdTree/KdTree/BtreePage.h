// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.h --
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

#ifndef __SYDNEY_KDTREE_BTREEPAGE_H
#define __SYDNEY_KDTREE_BTREEPAGE_H

#include "KdTree/Module.h"
#include "KdTree/Page.h"

#include "KdTree/PagePointer.h"

#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class BtreeFile;

//
//	CLASS
//	KdTree::BtreePage --
//
//	NOTES
//
class BtreePage : public Page
{
	friend class BtreeFile;
	
public:
	//
	//	STRUCT
	//	KdTree::BtreePage::Header
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiPrevPageID;		// 前方のページID
		PhysicalFile::PageID	m_uiNextPageID;		// 後方のページID
		ModUInt32				m_uiCount;			// ページ内のエントリ数
										// 最上位ビットはNodeかLeafかのフラグ
	};

	//
	//	STRUCT
	//	KdTree::BtreePage::Entry
	//
	struct Entry
	{
		// コンストラクタ(1)
		Entry()
			: m_uiKey(0), m_uiPageID(0), m_uiAreaID(0) {}
		// コンストラクタ(2)
		Entry(ModUInt32 uiKey_, ModUInt32 uiPageID_, ModUInt32 uiAreaID_ = 0)
			: m_uiKey(uiKey_), m_uiPageID(uiPageID_), m_uiAreaID(uiAreaID_) {}

		ModUInt32			m_uiKey;		// キー
		ModUInt32			m_uiPageID;		// ページID
		ModUInt32			m_uiAreaID;		// エリアID
	};

	//
	//	CLASS
	//	KdTree::BtreePage::Compare -- 比較クラス
	//
	class Compare
	{
	public:
		Compare() {}
		~Compare() {}

		// 比較する
		//
		//	-1 : p1 < p2
		//	 0 : p1 == p2
		//  +1 : p1 > p2
		//
		int operator () (const BtreePage::Entry& e1,
						 const BtreePage::Entry& e2) const
			{
				return (e1.m_uiKey < e2.m_uiKey) ? -1 :
					((e1.m_uiKey > e2.m_uiKey) ? 1 : 0);
			}
	};

	//
	//	TYPEDEF
	//	KdTree::BtreePage::Iterator
	//
	typedef Entry* Iterator;

	//
	//	TYPDEF
	//	KdTree::BtreePage::ConstIterator
	//
	typedef const Entry* ConstIterator;

	//
	//	TYPEDEF
	//	KdTree::BtreePage::PagePointer
	//
	typedef PageObjectPointer<BtreePage> PagePointer;

	// コンストラクタ(1)
	BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_);
	// コンストラクタ(2)
	BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_,
			  PhysicalFile::PageID uiPrevPageID_,
			  PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~BtreePage();

	// 親を設定する
	void setParentPage(const BtreePage::PagePointer& pParent_)
		{ m_pParentPage = pParent_; }

	// 検索する
	Iterator lowerBound(const Entry* pEntry_);
	Iterator upperBound(const Entry* pEntry_);
	Iterator find(const Entry* pEntry_);

	// 挿入する
	bool insertEntry(const Entry* pEntry_);
	// 削除する
	bool expungeEntry(const Entry* pEntry_, bool isReduce_ = true);

	// 指定位置に挿入
	void insert(Iterator i_, const Entry* pEntry_);
	void insert(Iterator i_, ConstIterator start_, ConstIterator end_);

	// 指定位置を削除
	void expunge(Iterator i_);
	void expunge(Iterator start_, Iterator end_);

	// 先頭を得る
	Iterator begin() { return m_pEntry; }
	// 終端を得る
	Iterator end() { return m_pEntry + getCount(); }

	// 整合性検査
	void verify();

	// リーフか
	bool isLeaf() const
		{ return m_pHeader->m_uiCount & 0x80000000; }
	// リーフにする
	void setLeaf();

	// ルートか
	bool isRoot() const
	{
		return getPrevPageID() == PhysicalFile::ConstValue::UndefinedPageID
			&& getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID;
	}

	// 前方のページIDを得る
	PhysicalFile::PageID getPrevPageID() const
		{ return m_pHeader->m_uiPrevPageID; }
	// 後方のページIDを得る
	PhysicalFile::PageID getNextPageID() const
		{ return m_pHeader->m_uiNextPageID; }

	// 前方のページIDを設定する
	void setPrevPageID(PhysicalFile::PageID uiPrevPageID_);
	// 後方のページIDを設定する
	void setNextPageID(PhysicalFile::PageID uiNextPageID_);

	// エントリ数を得る
	int getCount() const
		{ return static_cast<int>(m_pHeader->m_uiCount & 0x7fffffff); }
	
	// 最大格納数を得る
	int getMaxCount() const
		{
			return (getPageDataSize() - sizeof(Header))	/ sizeof(Entry);
		}
	static int getMaxCount(int iPageSize_)
		{
			return (getPageDataSize(iPageSize_) - sizeof(Header))
				/ sizeof(Entry);
		}

	// 空き数を得る
	int getFreeCount() const
		{ return getMaxCount() - getCount(); }

	// ページをdetachする
	void detach();

private:
	// ページを読み込む
	void load();

	// 拡張 -- ページ分割か再配分
	PagePointer expand(const Entry* pEntry_);
	// 縮小 -- ページ連結か再配分
	void reduce();

	// ページ分割
	PagePointer split(PagePointer pPrevPage_, const Entry* pEntry_);
	// ページ連結
	void concatenate(PagePointer pPrevPage_);
	// 再配分
	PagePointer redistribute(PagePointer pPrevPage_, const Entry* pEntry_ = 0);

	// 件数を増減する
	void changeCount(int changeCount_);

	// 親ページへのポインタ
	PagePointer m_pParentPage;

	// ヘッダー
	Header* m_pHeader;
	// エントリへのポインタ
	Entry* m_pEntry;
	// フリーリスト
	BtreePage* m_pNext;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_BTREEPAGE_H

//
//	Copyright (c) 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
