// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.h -- ヘッダーページ
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

#ifndef __SYDNEY_BTREE2_HEADERPAGE_H
#define __SYDNEY_BTREE2_HEADERPAGE_H

#include "Btree2/Module.h"
#include "Btree2/Page.h"
#include "Btree2/PagePointer.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class BtreeFile;

//
//	CLASS
//	Btree2::HeaderPage -- ヘッダーページ
//
//	NOTES
//
class HeaderPage : public Page
{
public:
	//
	//	STRUCT
	//	Btree2::HeaderPage::Header
	//
	struct Header
	{
		//【注意】	Windows(vc)とLinux(gcc)でパディングの動きが違うが、
		//			この構造体はどちらも40バイトで問題ない
		
		PhysicalFile::PageID	m_uiRootPageID;			// ルートページ
		PhysicalFile::PageID	m_uiLeftLeafPageID;		// 一番左のリーフページ
		PhysicalFile::PageID	m_uiRightLeafPageID;	// 一番右のリーフページ
		ModUInt32				m_uiCount;				// エントリ数
		ModUInt32				m_uiStepCount;			// B木の段数
		ModUInt32				m_uiInsertCount;		// 総挿入数
		ModUInt32				m_uiMaxValueCount;		// 最大値挿入数
		char					m_pTimeStamp[8];		// 最終更新日時
		ModUInt32				m_uiExpungeFlag;		// 削除フラグエントリ数
	};
	
	//
	//	TYPEDEF
	//	Btree2::HeaderPage::Pointer
	//
	typedef PageObjectPointer<HeaderPage> PagePointer;
	
	// コンストラクタ
	HeaderPage(BtreeFile& cFile_);
	// デストラクタ
	virtual ~HeaderPage();

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);

	// 使用サイズを得る
	ModSize getUsedSize() { return sizeof(Header) / sizeof(ModUInt32); }

	// 初期化する
	void initialize();

	// ルートページを得る
	PhysicalFile::PageID getRootPageID() const
		{ return m_cHeader.m_uiRootPageID; }
	// ルートページを設定する
	void setRootPageID(PhysicalFile::PageID uiPageID_)
	{
		m_cHeader.m_uiRootPageID = uiPageID_;
	}

	// 一番左のリーフページを得る
	PhysicalFile::PageID getLeftLeafPageID() const
		{ return m_cHeader.m_uiLeftLeafPageID; }
	// 一番左のリーフページを設定する
	void setLeftLeafPageID(PhysicalFile::PageID uiPageID_)
	{
		m_cHeader.m_uiLeftLeafPageID = uiPageID_;
	}

	// 一番右のリーフページを得る
	PhysicalFile::PageID getRightLeafPageID() const
		{ return m_cHeader.m_uiRightLeafPageID; }
	// 一番右のリーフページを設定する
	void setRightLeafPageID(PhysicalFile::PageID uiPageID_)
	{
		m_cHeader.m_uiRightLeafPageID = uiPageID_;
	}

	// エントリ数を増やす(同時に最終更新日時と総挿入数も更新される)
	void incrementCount();
	// エントリ数を減らす(同時に最終更新日時も更新される)
	void decrementCount();
	// エントリ数を得る
	ModSize getCount() const { return m_cHeader.m_uiCount; }

	// B木の段数を増やす
	void incrementStepCount()
	{
		m_cHeader.m_uiStepCount++;
	}
	// B木の段数を減らす
	void decrementStepCount()
	{
		m_cHeader.m_uiStepCount--;
	}
	// B木の段数を得る
	ModSize getStepCount() { return m_cHeader.m_uiStepCount; }

	// 最大値挿入数を増やす
	void incrementMaxValueCount()
	{
		m_cHeader.m_uiMaxValueCount++;
	}
	// 最大値挿入数を減らす
	void decrementMaxValueCount()
	{
		if (m_cHeader.m_uiMaxValueCount > 0)
		{
			m_cHeader.m_uiMaxValueCount--;
		}
	}

	// 削除フラグエントリ数を増やす
	void incrementExpungeFlagCount()
	{
		m_cHeader.m_uiExpungeFlag++;
	}
	// 削除フラグエントリ数を減らす
	void decrementExpungeFlagCount()
	{
		if (m_cHeader.m_uiExpungeFlag != 0)
			m_cHeader.m_uiExpungeFlag--;
	}
	// 削除フラグエントリ数を得る
	ModSize getExpungeFlagCount() const { return m_cHeader.m_uiExpungeFlag; }

	// ページ分割時の前ページに残すエントリの割合(%)を得る
	int getSplitRatio() const;

	// flush前処理
	void preFlush();

private:
	// 最終更新日時を設定する
	void setLastModificationTime();
	
	// ヘッダー
	Header m_cHeader;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_HEADERPAGE_H

//
//	Copyright (c) 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
