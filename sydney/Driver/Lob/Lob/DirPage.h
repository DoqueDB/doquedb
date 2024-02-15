// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirPage.h -- 
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

#ifndef __SYDNEY_LOB_DIRPAGE_H
#define __SYDNEY_LOB_DIRPAGE_H

#include "Lob/Module.h"
#include "Lob/Page.h"
#include "Lob/DataPage.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobFile;

//
//	CLASS
//	Lob::DirPage -- DIRページ
//
//	NOTES
//
class DirPage : public Page
{
public:
	//
	//	TYPEDEF
	//	Lob::DirPage::PagePointer --
	//
	typedef PageObjectPointer<DirPage> PagePointer;

	//
	//	STRUCT
	//	Lob::DirPage::Header --
	//
	struct Header
	{
		ModUInt32	m_uiStep;		// 段数
		ModUInt32	m_uiCount;		// 登録数
	};

	//
	//	STRUCT
	//	Lob::DirPage::Entry --
	//
	struct Entry
	{
		PhysicalFile::PageID	m_uiPageID;		// ページID
		ModSize					m_uiSize;		// そのページ以下の合計サイズ
	};
	
	// コンストラクタ(1)
	DirPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// コンストラクタ(2)
	DirPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
			ModUInt32 uiStep_);
	// デストラクタ
	virtual ~DirPage();

	// 使用サイズを得る
	ModSize getUsedSize()
	{
		return sizeof(Header)
			+ sizeof(Entry) * m_pHeader->m_uiCount;
	}

	// ヘッダーを得る
	Header* getHeader()
	{
		return m_pHeader;
	}

	// 段数を得る
	ModUInt32 getStep()
	{
		return m_pHeader->m_uiStep;
	}

	// 指定番目のページIDを得る
	PhysicalFile::PageID getPageID(ModSize uiNumber_);
	// 指定位置のページIDを得る
	PhysicalFile::PageID getPageID(ModSize uiPosition_, ModSize& uiDataSize_);
	
	// DATAページIDを追加する
	PhysicalFile::PageID addDataPageID(PhysicalFile::PageID uiPageID_,
									   ModSize uiDataSize_);
	// ページIDを追加する
	PhysicalFile::PageID addPageID(PhysicalFile::PageID uiPageID_,
								   ModSize uiDataSize_);

	// DATAページIDを削除する
	ModUInt32 delDataPageID(PhysicalFile::PageID uiPageID_,
							ModSize uiDataSize_);
	// ページIDを削除する
	ModUInt32 delPageID(PhysicalFile::PageID uiPageID_,
						ModSize uiDataSize_);

	// 下位の全ページを開放する
	void freePage();

	// 最大エントリ数を得る
	static ModSize getMaxCount(Os::Memory::Size uiPageSize_);
	
private:
	// ヘッダー
	Header* m_pHeader;
	// エントリ
	Entry* m_pEntry;

	// LobFile
	LobFile& m_cFile;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_DIRPAGE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
