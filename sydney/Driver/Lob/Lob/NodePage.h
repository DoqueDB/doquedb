// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodePage.h -- 
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

#ifndef __SYDNEY_LOB_NODEPAGE_H
#define __SYDNEY_LOB_NODEPAGE_H

#include "Lob/Module.h"
#include "Lob/BlockPage.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobFile;

//
//	CLASS
//	Lob::NodePage -- 先頭以外のノードページ
//
//	NOTES
//
class NodePage : public BlockPage
{
public:
	//
	//	STRUCT
	//	Lob::NodePage::Header -- ヘッダー
	//
	struct Header
	{
		ModUInt32				m_uiEntryCount;		// ページ内エントリ数
		PhysicalFile::PageID	m_uiNextBlockPage;	// 次のブロックページ
	};
	
	// コンストラクタ
	NodePage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// デストラクタ
	virtual ~NodePage();

	// 初期化する
	void initialize();

	// 新しいブロックを得る
	Block* allocateBlock(ModSize& uiPosition_);

	// ヘッダーを得る
	Header* getHeader()
	{
		return m_pHeader;
	}

	// 次のBlockページを設定する
	void setNextBlockPage(PhysicalFile::PageID uiNextPageID_)
	{
		dirty();
		m_pHeader->m_uiNextBlockPage = uiNextPageID_;
	}
	// 次のBlockページを得る
	PhysicalFile::PageID getNextBlockPage()
	{
		return m_pHeader->m_uiNextBlockPage;
	}
	
	// 使用サイズを得る
	ModSize getUsedSize();

	// ヘッダーサイズを得る
	ModSize getHeaderSize() { return sizeof(Header); }
	// エントリー数を得る
	ModSize getEntryCount() { return m_pHeader->m_uiEntryCount; }
	
private:
	// ヘッダー
	Header* m_pHeader;

	// ファイル
	LobFile& m_cLobFile;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_NODEPAGE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
