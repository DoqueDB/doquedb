// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.h -- ヘッダーページ
// 
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_HEADERPAGE_H
#define __SYDNEY_BITMAP_HEADERPAGE_H

#include "Bitmap/Module.h"
#include "Bitmap/Page.h"
#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;

//
//	CLASS
//	Bitmap::HeaderPage -- ヘッダーページ
//
//	NOTES
//
class HeaderPage : public Page
{
public:
	//
	//	STRUCT
	//	Bitmap::HeaderPage::Header
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiRootPageID;			// ルートページ
		PhysicalFile::PageID	m_uiLeftLeafPageID;		// 一番左のリーフページ
		PhysicalFile::PageID	m_uiRightLeafPageID;	// 一番右のリーフページ
		ModUInt32				m_uiCount;				// エントリ数
		ModUInt32				m_uiStepCount;			// B木の段数
		ModUInt32				m_uiMaxRowID;			// 最大ROWID
		PhysicalFile::PageID	m_uiNullPageID;			// null用のバリュー
		PhysicalFile::PageID	m_uiAllNullPageID;		// null用のバリュー
		unsigned short			m_usNullAreaID;			// null用のAreaID
		unsigned short			m_usAllNullAreaID;		// null用のAreaID
		char					m_pTimeStamp[8];		// 最終更新日時
	};
	
	// コンストラクタ
	HeaderPage(BitmapFile& cFile_);
	// デストラクタ
	virtual ~HeaderPage();

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);

	// 初期化する
	void initialize();

	// ルートページを得る
	PhysicalFile::PageID getRootPageID() const
		{ return m_pHeader->m_uiRootPageID; }
	// ルートページを設定する
	void setRootPageID(PhysicalFile::PageID uiPageID_)
	{
		dirty();
		m_pHeader->m_uiRootPageID = uiPageID_;
	}

	// 一番左のリーフページを得る
	PhysicalFile::PageID getLeftLeafPageID() const
		{ return m_pHeader->m_uiLeftLeafPageID; }
	// 一番左のリーフページを設定する
	void setLeftLeafPageID(PhysicalFile::PageID uiPageID_)
	{
		dirty();
		m_pHeader->m_uiLeftLeafPageID = uiPageID_;
	}

	// 一番右のリーフページを得る
	PhysicalFile::PageID getRightLeafPageID() const
		{ return m_pHeader->m_uiRightLeafPageID; }
	// 一番右のリーフページを設定する
	void setRightLeafPageID(PhysicalFile::PageID uiPageID_)
	{
		dirty();
		m_pHeader->m_uiRightLeafPageID = uiPageID_;
	}

	// null用のIDを得る
	bool getNullID(Common::Data& cID_) const;
	// null用のIDを設定する
	void setNullID(const Common::Data& cID_);
	// null用のIDをクリアする
	void clearNullID();

	// null用のIDを得る
	bool getAllNullID(Common::Data& cID_) const;
	// null用のIDを設定する
	void setAllNullID(const Common::Data& cID_);
	// null用のIDをクリアする
	void clearAllNullID();

	// エントリ数を増やす(同時に最終更新日時と総挿入数も更新される)
	void incrementCount();
	// エントリ数を減らす(同時に最終更新日時も更新される)
	void decrementCount();
	// エントリ数を得る
	ModSize getCount() const { return m_pHeader->m_uiCount; }

	// B木の段数を増やす
	void incrementStepCount()
	{
		dirty();
		m_pHeader->m_uiStepCount++;
	}
	// B木の段数を減らす
	void decrementStepCount()
	{
		dirty();
		m_pHeader->m_uiStepCount--;
	}
	// B木の段数を得る
	ModSize getStepCount() { return m_pHeader->m_uiStepCount; }

	// 最大ROWIDを更新する
	void setMaxRowID(ModUInt32 uiRowID_)
	{
		if (m_pHeader->m_uiMaxRowID < uiRowID_)
		{
			dirty();
			m_pHeader->m_uiMaxRowID = uiRowID_;
		}
	}
	// 最大ROWIDを得る
	ModUInt32 getMaxRowID() { return m_pHeader->m_uiMaxRowID; }

	// ページ分割時の前ページに残すエントリの割合(%)を得る
	int getSplitRatio() const;

private:
	// 最終更新日時を設定する
	void setLastModificationTime();
	
	// ヘッダー
	Header* m_pHeader;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_HEADERPAGE_H

//
//	Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
