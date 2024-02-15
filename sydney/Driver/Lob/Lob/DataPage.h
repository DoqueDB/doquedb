// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.h -- 
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_DATAPAGE_H
#define __SYDNEY_LOB_DATAPAGE_H

#include "Lob/Module.h"
#include "Lob/Page.h"
#include "Lob/PagePointer.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobFile;

//
//	CLASS
//	Lob::DataPage -- DATAページ
//
//	NOTES
//
class DataPage : public Page
{
public:
	//
	//	TYPEDEF
	//	Lob::DataPage::PagePointer --
	//
	typedef PageObjectPointer<DataPage> PagePointer;

	//
	//	STRUCT
	//	Lob::DataPage::Header -- ヘッダー
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiPrevPageID;	// 前のページ
		PhysicalFile::PageID	m_uiNextPageID;	// 次のページ
		ModSize					m_uiLength;		// データサイズ
	};
	
	// コンストラクタ(1)
	DataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// コンストラクタ(2)
	DataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
			  PhysicalFile::PageID uiPrevPageID_,
			  PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~DataPage();

	// 前のページIDを得る
	PhysicalFile::PageID getPrevPageID()
	{
		return m_pHeader->m_uiPrevPageID;
	}
	// 後のページIDを得る
	PhysicalFile::PageID getNextPageID()
	{
		return m_pHeader->m_uiNextPageID;
	}

	// 前のページを得る
	PagePointer getPrevPage();
	// 後のページを得る
	PagePointer getNextPage();

	// 次のページを設定する
	void setNextPageID(PhysicalFile::PageID uiNextPageID_)
	{
		dirty();
		m_pHeader->m_uiNextPageID = uiNextPageID_;
	}

	// データを得る
	virtual char* getData()
	{
		return getBuffer() + sizeof(Header);
	}
	// データサイズを得る
	ModSize getLength()
	{
		return m_pHeader->m_uiLength;
	}

	// データを追加する
	virtual bool append(const char* pBuffer_, ModSize uiLength_);
	// データを削除する
	virtual void truncate(ModSize uiLength_);
	// データを書き換える
	virtual void replace(ModSize uiPosition_,
						 const char* pBuffer_, ModSize uiLength_);

	// データサイズを得る
	ModSize getUsedSize()
	{
		return sizeof(Header) + m_pHeader->m_uiLength;
	}

	// 最大データサイズを得る
	static ModSize getPageDataSize(Os::Memory::Size uiPageSize_)
	{
		return Page::getPageSize(uiPageSize_) - sizeof(Header);
	}

private:
	// ヘッダー
	Header* m_pHeader;

	// ファイル
	LobFile& m_cFile;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_DATAPAGE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
