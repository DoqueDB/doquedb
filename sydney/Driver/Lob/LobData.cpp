// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobData.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "Lob/LobData.h"
#include "Lob/MessageAll_Class.h"

#include "Os/Memory.h"
#include "Os/Limits.h"

#include "Common/Assert.h"

#include "Exception/BadArgument.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::LobData::LobData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	Lob::BlockPage::PagePointer pBlockPage_
//		ブロックページ
//	Lob::BlockPage::Block* pBlock_
//		ブロック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LobData::LobData(LobFile& cFile_,
				 BlockPage::PagePointer pBlockPage_,
				 BlockPage::Block* pBlock_)
	: m_cFile(cFile_), m_pBlockPage(pBlockPage_), m_pBlock(pBlock_)
{
}

//
//	FUNCTION public
//	Lob::LobData::~LobData -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LobData::~LobData()
{
}

//
//	FUNCTION public
//	Lob::LobData::get -- データを取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		取得する位置
//	ModSize& uiLength_
//		取得するデータ長
//
//	RETURN
//	Lob::AutoPointer<void>
//		取得データ
//
//	EXCEPTIONS
//
Lob::AutoPointer<void>
LobData::get(ModSize uiPosition_, ModSize& uiLength_)
{
	// 長さチェック
	if (uiPosition_ + uiLength_ > m_pBlock->m_uiLength)
	{
		if (m_pBlock->m_uiLength > uiPosition_)
		{
			uiLength_ = m_pBlock->m_uiLength - uiPosition_;
		}
		else
		{
			uiLength_ = 0;
		}
	}

	if (uiLength_ == 0)
		return 0;

	// 領域を確保
	ModSize uiLength = uiLength_;
	AutoPointer<void> pValue(ModDefaultManager::allocate(uiLength_), uiLength);
	char* p = syd_reinterpret_cast<char*>(pValue.get());

	// DATAページをattach
	ModSize uiPrevSize;
	DataPage::PagePointer pDataPage = attachDataPage(uiPosition_, uiPrevSize);
	ModSize position = uiPosition_ % m_cFile.getDataPageDataSize();

	// データ取得
	while (uiLength)
	{
		ModSize length = uiLength;
		if ((pDataPage->getLength() - position) < length)
			length = pDataPage->getLength() - position;
		Os::Memory::copy(p,  pDataPage->getData() + position, length);
		p += length;
		uiLength -= length;
		uiPosition_ += length;
		position = 0;

		if (uiLength)
		{
			// 次のDATAページをattach
			pDataPage = pDataPage->getNextPage();
		}
	}

	return pValue;
}

//
//	FUNCTION public
//	Lob::Lobdata::insert -- 挿入
//
//	NOTES
//
//	ARGUMENTS
//	const void* pBuffer_
//		挿入するデータ
//	ModSize uiLength_
//		挿入するデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::insert(const void* pBuffer_, ModSize uiLength_)
{
	m_pBlockPage->dirty();
	
	// DataPageを得る
	DataPage::PagePointer p
		= m_cFile.allocateDataPage(PhysicalFile::ConstValue::UndefinedPageID,
									PhysicalFile::ConstValue::UndefinedPageID);
	// ブロックを初期化する
	ModOsDriver::Memory::reset(m_pBlock, sizeof(BlockPage::Block));
	m_pBlock->m_cPrevBlock.clear();
	m_pBlock->m_cNextBlock.clear();

	// 初期値を設定する
	m_pBlock->m_uiDirPage = PhysicalFile::ConstValue::UndefinedPageID;
	m_pBlock->m_uiDirLength = 0;
	m_pBlock->setUsedPageNumber(1);
	m_pBlock->unsetExpungeFlag();
	m_pBlock->m_uiLength = 0;
	m_pBlock->m_uiLastPage = p->getID();

	append(pBuffer_, uiLength_);
}

//
//	FUNCTION public
//	Lob::LobData::expunge -- 本当に削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::expunge()
{
	m_pBlockPage->dirty();
	
	if (m_pBlock->m_uiDirPage != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// DIR以下を削除する
		DirPage::PagePointer p = m_cFile.attachDirPage(m_pBlock->m_uiDirPage);
		p->freePage();
		m_cFile.freePage(p.get());
	}

	// 最終バリューページを開放する
	m_cFile.freePage(m_pBlock->m_uiLastPage);
	m_pBlock->clear();
}

//
//	FUNCTION public
//	Lob::LobData::append -- データを追加する
//
//	NOTES
//
//	ARGUMENTS
//	const void* pBuffer_
//		追加するデータ
//	ModSize uiLength_
//		追加するデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::append(const void* pBuffer_, ModSize uiLength_)
{
	// 長さチェック
	if (m_pBlock->m_uiLength + uiLength_
		> static_cast<ModSize>(Os::Limits<int>::getMax()))
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	m_pBlockPage->dirty();
	
	// 最終DATAページを得る
	ModSize uiPrevSize;
	DataPage::PagePointer pDataPage = attachLastDataPage(uiPrevSize);

	// キャスト
	const char* p = syd_reinterpret_cast<const char*>(pBuffer_);

	// 追加する
	m_pBlock->m_uiLength += uiLength_;
	while (uiLength_)
	{
		ModSize length = uiLength_;
		if (uiLength_ > pDataPage->getFreeSize())
			length = pDataPage->getFreeSize();
		pDataPage->append(p, length);
		p += length;
		uiLength_ -= length;

		if (uiLength_)
		{
			// 新しいDATAページを得る
			pDataPage = allocateDataPage(pDataPage);
		}
	}
}

//
//	FUNCTION public
//	Lob::LobData::truncate -- データを縮める
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLength_
//		縮める長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::truncate(ModSize uiLength_)
{
	m_pBlockPage->dirty();
	
	if (uiLength_ > m_pBlock->m_uiLength)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	// 最終DATAページを得る
	ModSize uiPrevSize;
	DataPage::PagePointer pDataPage = attachLastDataPage(uiPrevSize);

	// 縮める
	m_pBlock->m_uiLength -= uiLength_;
	while (uiLength_)
	{
		ModSize length = uiLength_;
		if (length > pDataPage->getLength())
			length = pDataPage->getLength();
		pDataPage->truncate(length);
		uiLength_ -= length;
		if (pDataPage->getLength() == 0)
		{
			// 前のページを得る
			DataPage::PagePointer p = pDataPage->getPrevPage();
			if (p != 0)
				// 前のページの次のページを更新する
				p->setNextPageID(PhysicalFile::ConstValue::UndefinedPageID);
			// ページを開放する
			freeDataPage(pDataPage);

			pDataPage = p;
		}
	}
}

//
//	FUNCTION public
//	Lob::LobData::replace -- データを書き換える
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		書き換える位置
//	const void* pBuffer_
//		書き換えるデータ
//	ModSize uiLength_
//		書き換えるデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::replace(ModSize uiPosition_, const void* pBuffer_, ModSize uiLength_)
{
	if (uiPosition_ + uiLength_ > m_pBlock->m_uiLength)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	const char* p = syd_reinterpret_cast<const char*>(pBuffer_);

	// DATAページをattach
	ModSize uiPrevSize;
	DataPage::PagePointer pDataPage = attachDataPage(uiPosition_,
													 uiPrevSize);
	ModSize position = uiPosition_ % m_cFile.getDataPageDataSize();

	// データ書き換え
	while (uiLength_)
	{
		ModSize length = uiLength_;
		if ((pDataPage->getLength() - position) < length)
			length = pDataPage->getLength() - position;
		pDataPage->replace(position, p, length);
		p += length;
		uiLength_ -= length;
		uiPosition_ += length;
		position = 0;

		if (uiLength_)
		{
			// 次のDATAページをattach
			pDataPage = pDataPage->getNextPage();
		}
	}
}

//
//	FUNCTION public
//	Lob::LobData::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Lob::ObjectID& cObjectID_
//		オブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::verify(const ObjectID& cObjectID_)
{
	ModUInt64 id =
		(static_cast<ModUInt64>(cObjectID_.m_uiPageID) << 32) |
		cObjectID_.m_uiPosition;
	
	// すべてのデータページをattachする
	ModSize uiPosition = 0;
	PhysicalFile::PageID uiPrevPageID
		= PhysicalFile::ConstValue::UndefinedPageID;
	PhysicalFile::PageID uiNextPageID
		= PhysicalFile::ConstValue::UndefinedPageID;
	// m_uiLength=0の場合もあるので do{}while で実行する。
	do
	{
		ModSize uiPrevSize;
		DataPage::PagePointer p = attachDataPage(uiPosition, uiPrevSize);
		if (uiPrevPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			if (uiPrevPageID != p->getPrevPageID())
			{
				// エラー
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::InvalidPageLink(id));
				return;
			}
		}
		if (uiNextPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			if (uiNextPageID != p->getID())
			{
				// エラー
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::InvalidPageLink(id));
				return;
			}
		}
		uiPrevPageID = p->getID();
		uiNextPageID = p->getNextPageID();
		
		uiPosition += p->getLength();
	} while (uiPosition < m_pBlock->m_uiLength);
}

//
//	FUNCTION protected
//	Lob::LobData::attachDataPage -- 指定位置のDATAページを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		ポジション
//	ModSize& uiPrevSize_
//		そのデータページより前の合計サイズ
//
//	RETURN
//	Lob::DataPage::PagePointer
//		DATAページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
LobData::attachDataPage(ModSize uiPosition_,
						ModSize& uiPrevSize_)
{
	// ページIDを得る
	PhysicalFile::PageID uiPageID;
	if (uiPosition_ < m_pBlock->m_uiDirLength)
	{
		// 途中のページ
		uiPageID = getDataPageID(uiPosition_, uiPrevSize_);
	}
	else
	{
		// 最後のページ
		uiPageID = m_pBlock->m_uiLastPage;
		uiPrevSize_ = m_pBlock->m_uiDirLength;
	}

	return m_cFile.attachDataPage(uiPageID);
}

//
//	FUNCTION protected
//	Lob::LobData::attachLastDataPage -- 最終データページを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize& uiPrevSize_
//		このページより前のトータルサイズ
//
//	RETURN
//	Lob::DataPage::PagePointer
//		最終DATAページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
LobData::attachLastDataPage(ModSize& uiPrevSize_)
{
	uiPrevSize_ = m_pBlock->m_uiDirLength;
	return m_cFile.attachDataPage(m_pBlock->m_uiLastPage);
}

//
//	FUNCTION protected
//	Lob::LobData::allocateDataPage -- 新しいDATAページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	DataPage::PagePointer& pPrevPage_
//		前のページ
//
//	RETURN
//	DataPage::PagePointer
//		新しいページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
LobData::allocateDataPage(DataPage::PagePointer& pPrevPage_)
{
	m_pBlockPage->dirty();
	
	// 新しいページを確保する
	DataPage::PagePointer pDataPage
		= m_cFile.allocateDataPage(pPrevPage_->getID(),
									PhysicalFile::ConstValue::UndefinedPageID);
	// 前のページの次のページを設定
	pPrevPage_->setNextPageID(pDataPage->getID());

	// 最終DATAページ
	PhysicalFile::PageID uiLastPageID = m_pBlock->m_uiLastPage;
	// 新しいページを最終ページに設定
	m_pBlock->m_uiLastPage = pDataPage->getID();
	m_pBlock->incrementUsedPageNumber();

	if (uiLastPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// DIRページに設定する
		insertDirPage(uiLastPageID, pPrevPage_->getLength());
	}

	return pDataPage;
}

//
//	FUNCTION protected
//	Lob::LobData::freeDataPage -- DATAページを削除する
//
//	NOTES
//
//	ARGUMENTS
//	DataPage::PagePointer& pDataPage_
//		開放するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::freeDataPage(DataPage::PagePointer& pDataPage_)
{
	m_pBlockPage->dirty();

	// 新しい最後のページを得る
	DataPage::PagePointer p = pDataPage_->getPrevPage();
	// 次のページをクリア
	p->setNextPageID(PhysicalFile::ConstValue::UndefinedPageID);
	// 最終ページを変更
	m_pBlock->m_uiLastPage = p->getID();
	m_pBlock->decrementUsedPageNumber();

	if (m_pBlock->m_uiDirPage != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// DIRページから削除
		deleteDirPage(p->getID(), p->getLength());
	}

	m_cFile.freePage(pDataPage_.get());
}

//
//	FUNCTION private
//	Lob::LobData::getDataPageID -- ページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		開始位置
//	ModSize& uiPrevSize_
//		そのページより前の合計サイズ
//
//	RETURN
//	PhysicalFile::PageID
//		ページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
LobData::getDataPageID(ModSize uiPosition_,
					   ModSize& uiPrevSize_)
{
	PhysicalFile::PageID uiPageID = m_pBlock->m_uiDirPage;
	uiPrevSize_ = 0;

	for (;;)
	{
		// DIRページをattachする
		DirPage::PagePointer pDirPage = m_cFile.attachDirPage(uiPageID);
		// ページIDを得る
		ModSize uiPrevSize;
		uiPageID = pDirPage->getPageID(uiPosition_, uiPrevSize);
		uiPrevSize_ += uiPrevSize;

		if (pDirPage->getStep() == 1)
			break;

		uiPosition_ -= uiPrevSize;
	}

	return uiPageID;
}

//
//	FUNCTION private
//	LobData::insertDirPage -- DIRページの最後にページIDを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		挿入するページID
//	ModSize uiDataSize_
//		挿入するページのデータサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::insertDirPage(PhysicalFile::PageID uiPageID_, ModSize uiDataSize_)
{
	DirPage::PagePointer p;
	
	if (m_pBlock->m_uiDirPage == PhysicalFile::ConstValue::UndefinedPageID)
	{
		m_pBlockPage->dirty();
		// 新しいページを得る
		p = m_cFile.allocateDirPage(1);
		m_pBlock->m_uiDirPage = p->getID();
	}
	else
	{
		p = m_cFile.attachDirPage(m_pBlock->m_uiDirPage);
	}

	PhysicalFile::PageID ret = p->addDataPageID(uiPageID_, uiDataSize_);
	if (ret != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 段数が増える
		DirPage::PagePointer pNew
			= m_cFile.allocateDirPage(p->getStep() + 1);
		pNew->addPageID(p->getID(), m_pBlock->m_uiDirLength);
		pNew->addPageID(ret, uiDataSize_);
		m_pBlockPage->dirty();
		m_pBlock->m_uiDirPage = pNew->getID();
	}

	// DIRページ内のトータルサイズを増やす
	m_pBlock->m_uiDirLength += uiDataSize_;
}

//
//	FUNCTION private
//	LobData::deleteDirPage -- DIRページの最後のページIDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		削除するページID
//	ModSize uiDataSize_
//		削除するページのデータサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobData::deleteDirPage(PhysicalFile::PageID uiPageID_, ModSize uiDataSize_)
{
	; _SYDNEY_ASSERT(m_pBlock->m_uiDirPage
					 != PhysicalFile::ConstValue::UndefinedPageID);
	
	DirPage::PagePointer p = m_cFile.attachDirPage(m_pBlock->m_uiDirPage);
	ModUInt32 count = p->delDataPageID(uiPageID_, uiDataSize_);
	if (count == 0)
	{
		m_pBlockPage->dirty();
		// このページを削除する
		m_cFile.freePage(p.get());
		m_pBlock->m_uiDirPage = PhysicalFile::ConstValue::UndefinedPageID;
	}
	else if (count == 1 && p->getStep() != 1)
	{
		m_pBlockPage->dirty();
		// 先頭ページIDを得る
		m_pBlock->m_uiDirPage = p->getPageID(0);
		// このページを削除する
		m_cFile.freePage(p.get());
	}
	// DIRページ内のトータルサイズを減らす
	m_pBlock->m_uiDirLength -= uiDataSize_;
}

//
//	Copyright (c) 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
