// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.h -- データページ
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_DATAPAGE_H
#define __SYDNEY_ARRAY_DATAPAGE_H

#include "Array/Module.h"

#include "Array/AutoPointer.h"
#include "Array/Page.h"
#include "Array/PagePointer.h"		//PageObjectPointer

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

class ArrayFile;
class Compare;
class Data;
class Tree;

//
//	CLASS
//	Array::DataPage -- データページ
//
//	NOTES
//
class DataPage : public Page
{
public:
	//
	//	STRUCT
	//	Array::DataPage::Header
	//
	struct Header
	{
		ModUInt32				m_uiCount;		// ページのエントリ数
		PhysicalFile::PageID	m_uiPrevPageID;	// 前のページ
		PhysicalFile::PageID	m_uiNextPageID;	// 後のページ
	};
	
	//
	//	TYPEDEF
	//	Array::DataPage::PagePointer
	//
	typedef PageObjectPointer<DataPage> PagePointer;
	
	//
	//	TYPEDEF
	//	Array::DataPage::Iterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::Iterator Iterator;

	//
	//	TYPEDEF
	//	Array::DataPage::ConstIterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::ConstIterator ConstIterator;

	// コンストラクタ
	DataPage(ArrayFile& cFile_);
	// デストラクタ
	virtual ~DataPage();

	// Treeを設定する
	void setTree(Tree* pTree_) { m_pTree = pTree_; }

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);
	// 物理ページを設定する
	virtual void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
								 PhysicalFile::PageID uiPrevPageID_,
								 PhysicalFile::PageID uiNextPageID_);

	// ページサイズを得る(UNIT単位)
	ModSize getPageSize();

	// ルートページか
	bool isRoot() const;
	// リーフページか
	bool isLeaf() const;
	// リーフページに設定する
	void setLeaf();

	// 確定前処理
	void preFlush();

	// 前のページ
	void setPrevPageID(PhysicalFile::PageID uiPageID_)
	{
		updateMode();
		dirty();
		m_pHeader->m_uiPrevPageID = uiPageID_;
	}
	PhysicalFile::PageID getPrevPageID() const
		{ return m_pHeader->m_uiPrevPageID; }
	
	// 後のページ
	void setNextPageID(PhysicalFile::PageID uiPageID_)
	{
		updateMode();
		dirty();
		m_pHeader->m_uiNextPageID = uiPageID_;
	}
	PhysicalFile::PageID getNextPageID() const
		{ return m_pHeader->m_uiNextPageID; }

	// ページのエントリ数を得る
	ModUInt32 getCount();
	
	// 使用サイズを得る(UNIT単位)
	ModSize getUsedSize();

	// 親を設定する
	void setParentPage(const DataPage::PagePointer& pParent_)
	{
		m_pParentPage = pParent_;
	}

	// Get the iterator toward the Leaf's lowerBound one.
	Iterator getIteratorTowardLeafLowerBound(const ModUInt32* pValue_,
											 const Compare& cCompare_);
	// Get the iterator toward the Leaf's upperBound one.
	Iterator getIteratorTowardLeafUpperBound(const ModUInt32* pValue_,
											 const Compare& cCompare_);
	// 検索する
	Iterator find(const ModUInt32* pValue_, const Compare& cCompare_);

	// 挿入する
	bool insertEntry(const ModUInt32* pBuffer_, ModSize uiSize_ = 0);
	// 削除する
	bool expungeEntry(const ModUInt32* pBuffer_, ModSize uiSize_ = 0,
					  bool isReduce_ = true);

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

	// デタッチする
	void detach();

private:
	// データ領域へのポインタを得る
	ModUInt32* getBuffer();

	// ページのエントリ数を増やす
	void addCount(ModUInt32 uiCount_);
	// ページのエントリ数を減らす
	void subtractCount(ModUInt32 uiCount_);

	// データクラスを得る
	const Data& getDataClass() const;
	
	// 必要ならFixModeを変更する
	void updateMode();

	// 物理ページが変更されたので読み直す
	void reload();

	// 初期化
	void initialize();
	// ロードする
	void load(ModUInt32 count_);

	// 拡張 -- ページ分割か再配分
	PagePointer expand(const ModUInt32* pBuffer_);
	void createNewRoot();
	PagePointer getNewRightMostPage();
	
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
	AutoPointer<ModUInt32> makeNodeEntry(ModUInt32* pEntry_, ModSize& uiSize_);

	// ファイル
	ArrayFile& m_cFile;

	// 親ページへのポインタ
	PagePointer m_pParentPage;

	// The pointer of the entries in the page.
	// Last entry points 0 which is a simbole of the end of the entries.
	// So, in initialize(), this is assigned with the number of count+1.
	// And the last entry is always used in this module.
	// So, in end(), return the m_vecpEntry.end()-1.
	ModVector<ModUInt32*> m_vecpEntry;

	// ページヘッダー
	Header* m_pHeader;

	// Tree
	Tree* m_pTree;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_LEAFPAGE_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
