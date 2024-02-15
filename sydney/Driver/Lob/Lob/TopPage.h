// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TopPage.h -- 
// 
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_TOPPAGE_H
#define __SYDNEY_LOB_TOPPAGE_H

#include "Lob/Module.h"
#include "Lob/PagePointer.h"
#include "Lob/BlockPage.h"
#include "Lob/ObjectID.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobFile;

//
//	CLASS
//	Lob::TopPage -- 先頭ページ
//
//	NOTES
//
class TopPage : public BlockPage
{
public:
	//
	//	STRUCT
	//	Lob::TopPage::Header -- ヘッダー
	//
	struct Header
	{
		ModUInt32				m_uiEntryCount;			// ページ内エントリ数
		PhysicalFile::PageID	m_uiNextBlockPage;		// 次のブロックページ
		ModUInt32				m_uiTotalBlockCount;	// 全ブロック数
		ModUInt32				m_uiTotalEntryCount;	// 全エントリ数
		PhysicalFile::PageID	m_uiLastBlockPage;		// 最終ブロックページ
		ObjectID				m_cFreeBlock;			// 先頭空き領域
		ObjectID				m_cExpungeBlock;		// 先頭削除領域
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy[4];
#endif
		Trans::Transaction::ID::Value
								m_uiTransactionID;		// 本当の削除を実行する
														// トランザクションID
		ObjectID				m_cPrevFreeBlock;		// 次に開放するブロック
														// の１つ前のブロック
	};
	
	// コンストラクタ
	TopPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// デストラクタ
	virtual ~TopPage();

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
	
	// 最終のBlockページを設定する
	void setLastBlockPage(PhysicalFile::PageID uiLastPageID_)
	{
		dirty();
		m_pHeader->m_uiLastBlockPage = uiLastPageID_;
	}

	// ブロック数を増やす
	void incrementBlockCount()
	{
		dirty();
		m_pHeader->m_uiTotalBlockCount++;
	}

	// エントリ数を得る
	ModUInt32 getTotalEntryCount() { return m_pHeader->m_uiTotalEntryCount; }
	// エントリ数を増やす
	void incrementEntryCount()
	{
		dirty();
		m_pHeader->m_uiTotalEntryCount++;
	}
	// エントリ数を減らす
	void decrementEntryCount()
	{
		dirty();
		m_pHeader->m_uiTotalEntryCount--;
	}

	// 最終BLOCKページIDを得る
	PhysicalFile::PageID getLastBlockPageID()
	{
		return m_pHeader->m_uiLastBlockPage;
	}
	// 最終BLOCKページIDを設定する
	void setLastBlockPageID(PhysicalFile::PageID uiPageID)
	{
		dirty();
		m_pHeader->m_uiLastBlockPage = uiPageID;
	}

	// 先頭空き領域を得る
	ObjectID getFreeBlock() { return m_pHeader->m_cFreeBlock; }
	// 先頭空き領域を設定する
	void setFreeBlock(const ObjectID& cFreeBlock_)
	{
		dirty();
		m_pHeader->m_cFreeBlock = cFreeBlock_;
	}

	// 先頭削除領域を得る
	ObjectID getExpungeBlock() { return m_pHeader->m_cExpungeBlock; }
	// 先頭空き領域を設定する
	void setExpungeBlock(const ObjectID& cExpungeBlock_)
	{
		dirty();
		m_pHeader->m_cExpungeBlock = cExpungeBlock_;
	}

	// 削除トランザクションIDを得る
	Trans::Transaction::ID::Value getTransactionID()
	{
		return m_pHeader->m_uiTransactionID;
	}
	// 削除トランザクションIDを設定する
	void setTransactionID(Trans::Transaction::ID::Value uiTransactionID_)
	{
		dirty();
		m_pHeader->m_uiTransactionID = uiTransactionID_;
	}

	// 次に開放するブロックの１つ前を得る
	ObjectID getPrevFreeBlock() { return m_pHeader->m_cPrevFreeBlock; }
	// 次に開放するブロックの１つ前を設定する
	void setPrevFreeBlock(const ObjectID& cPrevFreeBlock_)
	{
		dirty();
		m_pHeader->m_cPrevFreeBlock = cPrevFreeBlock_;
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
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_TOPPAGE_H

//
//	Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
