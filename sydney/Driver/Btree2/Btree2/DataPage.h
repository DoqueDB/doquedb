// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.h -- データページ
// 
// Copyright (c) 2003, 2004, 2005, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_DATAPAGE_H
#define __SYDNEY_BTREE2_DATAPAGE_H

#include "Btree2/Module.h"
#include "Btree2/Page.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class BtreeFile;
class Compare;
class Data;

//
//	CLASS
//	Btree2::DataPage -- データページ
//
//	NOTES
//
class DataPage : public Page
{
public:
	//
	//	STRUCT
	//	Btree2::DataPage::Header
	//
	struct Header
	{
		ModUInt32				m_uiCount;		// ページのエントリ数
		PhysicalFile::PageID	m_uiPrevPageID;	// 前のページ
		PhysicalFile::PageID	m_uiNextPageID;	// 後のページ
	};
	
	// コンストラクタ
	DataPage(BtreeFile& cFile_);
	// デストラクタ
	virtual ~DataPage();

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
	
protected:
	// データ領域へのポインタを得る
	ModUInt32* getBuffer();

	// ページのエントリ数を増やす
	void addCount(ModUInt32 uiCount_);
	// ページのエントリ数を減らす
	void subtractCount(ModUInt32 uiCount_);

	// データクラスを得る
	const Data& getDataClass();
	const Data& getKeyDataClass();
	// 比較クラスを得る
	const Compare& getCompareClass(bool onlyKey_ = false);
	
	// 必要ならFixModeを変更する
	void updateMode();

	// ベクターをreloadする
	virtual void reload() = 0;

	// ロック名を得る
	const Lock::FileName& getLockName() const;
	// トランザクションを得る
	const Trans::Transaction& getTransaction() const;

private:
	// ファイル
	BtreeFile& m_cFile;

	// ページヘッダー
	Header* m_pHeader;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_LEAFPAGE_H

//
//	Copyright (c) 2003, 2004, 2005, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
