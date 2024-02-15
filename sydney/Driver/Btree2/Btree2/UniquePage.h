// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UniquePage.h -- ユニーク制約用
// 
// Copyright (c) 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_UNIQUEPAGE_H
#define __SYDNEY_BTREE2_UNIQUEPAGE_H

#include "Btree2/Module.h"
#include "Btree2/DataPage.h"
#include "Btree2/PagePointer.h"
#include "Btree2/Data.h"
#include "Btree2/Compare.h"
#include "Btree2/AutoPointer.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class UniqueFile;

//
//	CLASS
//	Btree2::UniquePage -- 要素にnullがくることは無いページ
//
//	NOTES
//
class UniquePage : public DataPage
{
public:
	//
	//	TYPEDEF
	//	Btree2::UniquePage::PagePointer
	//
	typedef PageObjectPointer<UniquePage> PagePointer;
	
	//
	//	TYPEDEF
	//	Btree2::UniquePage::Iterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::Iterator Iterator;

	//
	//	TYPEDEF
	//	Btree2::UniquePage::ConstIterator -- イテレータ
	//
	typedef ModVector<ModUInt32*>::ConstIterator ConstIterator;
	
	// コンストラクタ
	UniquePage(UniqueFile& cFile_);
	// デストラクタ
	virtual ~UniquePage();

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);
	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
						 PhysicalFile::PageID uiPrevPageID_,
						 PhysicalFile::PageID uiNextPageID_);

	// 使用サイズを得る(UNIT単位)
	ModSize getUsedSize();

	// 親を設定する
	void setParentPage(const UniquePage::PagePointer& pParent_)
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

	// ページ内の削除可能な削除フラグの付いたエントリを追加する
	void addVacuumEntry(
		ModVector<ModPair<AutoPointer<ModUInt32>, ModSize> >& vecEntry_);
						

protected:
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

	// 削除済みエントリの削除
	void vacuum();

	// ノードエントリを作成する
	AutoPointer<ModUInt32> makeNodeEntry(ModUInt32* pEntry_, ModSize& uiSize_);

	// ロックできるか
	bool isLock(ModUInt32* pEntry_);

	// ファイル
	UniqueFile& m_cFile;
	
	// 親ページへのポインタ
	PagePointer m_pParentPage;

	ModVector<ModUInt32*> m_vecpEntry;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_UNIQUEPAGE_H

//
//	Copyright (c) 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
