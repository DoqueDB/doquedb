// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BlockPage.h -- 
// 
// Copyright (c) 2003, 2004, 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_BLOCKPAGE_H
#define __SYDNEY_LOB_BLOCKPAGE_H

#include "Lob/Module.h"
#include "Lob/Page.h"
#include "Lob/ObjectID.h"
#include "Lob/PagePointer.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LobFile;

//
//	CLASS
//	Lob::BlockPage -- 固定長のページ
//
//	NOTES
//
class BlockPage : public Page
{
public:
	//
	//	TYPEDEF
	//	Lob::BlockPage::PagePointer --
	//
	typedef PageObjectPointer<BlockPage> PagePointer;
	
	//
	//	STRUCT
	//	Lob::BlockPage::Block -- 固定部分のブロック
	//
	struct Block
	{
		// 使用バリューページ数
		void setUsedPageNumber(ModUInt32 uiUsedPageNumber_);
		ModUInt32 getUsedPageNumber();
		void incrementUsedPageNumber();
		void decrementUsedPageNumber();

		// 削除フラグ
		bool isExpunge();
		void setExpungeFlag();
		void unsetExpungeFlag();

		// クリアする
		void clear();

		// DIRページ
		PhysicalFile::PageID	m_uiDirPage;
		// DIRページ内のトータルサイズ(byte)
		ModSize					m_uiDirLength;
		// 使用バリューページ数
		ModUInt32				m_uiUsedPageNumber;
		// 全長(byte)
		ModSize					m_uiLength;
		// 最終バリューページ
		PhysicalFile::PageID	m_uiLastPage;
		// 削除したトランザクションID
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy[4];
#endif
		Trans::Transaction::ID::Value
								m_uiTransactionID;
		// 直前のイメージへのリンク
		ObjectID				m_cPrevBlock;
		// リンク先のブロック
		ObjectID				m_cNextBlock;
	};
	
	// コンストラクタ
	BlockPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// デストラクタ
	virtual ~BlockPage();

	// 初期化する
	virtual void initialize() = 0;

	// 新しいブロックを得る
	virtual Block* allocateBlock(ModSize& uiPosition_) = 0;
	// 指定位置のブロックを得る
	Block* getBlock(ModSize uiPosition_);
	// 次のブロックを得る
	Block* getNextBlock(ModSize& uiPosition_);

	// 次のBlockページを設定する
	virtual void setNextBlockPage(PhysicalFile::PageID uiNextPageID_) = 0;
	// 次のBlockページを得る
	virtual PhysicalFile::PageID getNextBlockPage() = 0;

	// エントリ数を得る
	virtual ModSize getEntryCount() = 0;
	// ヘッダーサイズを得る
	virtual ModSize getHeaderSize() = 0;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_BLOCKPAGE_H

//
//	Copyright (c) 2003, 2004, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
