// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedLobData.cpp --
// 
// Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
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
#include "Lob/CompressedLobData.h"
#include "Lob/CompressedDataPage.h"
#include "Lob/MessageAll_Class.h"

#include "Os/Memory.h"
#include "Os/Limits.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::CompressedLobData::CompressedLobData -- コンストラクタ
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
CompressedLobData::CompressedLobData(LobFile& cFile_,
									 BlockPage::PagePointer pBlockPage_,
									 BlockPage::Block* pBlock_)
	: LobData(cFile_, pBlockPage_, pBlock_)
{
}

//
//	FUNCTION public
//	Lob::CompressedLobData::~CompressedLobData -- デストラクタ
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
CompressedLobData::~CompressedLobData()
{
}

//
//	FUNCTION public
//	Lob::CompressedLobData::get -- データを取得する
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
CompressedLobData::get(ModSize uiPosition_, ModSize& uiLength_)
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
	ModSize position = uiPosition_ - uiPrevSize;
	CompressedDataPage* pCompressedDataPage
		= static_cast<CompressedDataPage*>(pDataPage.get());

	// ブロックを得る
	CompressedDataPage::Iterator i
		= pCompressedDataPage->getData(position, uiPrevSize);
	position -= uiPrevSize;

	// 伸長用バッファ
	char buff[CompressedDataPage::BlockSize];

	// データ取得
	while (uiLength)
	{
		ModSize size = CompressedDataPage::BlockSize;
		pCompressedDataPage->uncompress(buff, size, i);

		ModSize length = uiLength;
		if ((size - position) < length)
			length = size - position;
		Os::Memory::copy(p,  buff + position, length);
		p += length;
		uiLength -= length;
		position = 0;

		if (uiLength)
		{
			++i;
			if (i == pCompressedDataPage->end())
			{
				// 次のDATAページをattach
				pDataPage = pDataPage->getNextPage();
				pCompressedDataPage
					= static_cast<CompressedDataPage*>(pDataPage.get());
				i = pCompressedDataPage->begin();
			}
		}
	}

	return pValue;
}

//
//	FUNCTION public
//	Lob::CompressedLobData::append -- データを追加する
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
CompressedLobData::append(const void* pBuffer_, ModSize uiLength_)
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
	CompressedDataPage* pCompressedDataPage
		= static_cast<CompressedDataPage*>(pDataPage.get());

	AutoPointer<char> pTmp;
	const char* p = 0;
	CompressedDataPage::Iterator i = pCompressedDataPage->end();
	if (i != pCompressedDataPage->begin()
		&& pCompressedDataPage->getLength(--i) != CompressedDataPage::BlockSize)
	{
		// 半端があるので半端を削除する
		ModSize length = pCompressedDataPage->getLength(i);
		ModSize size = uiLength_ + length;
		char* buf = syd_reinterpret_cast<char*>(ModDefaultManager::allocate(size));
		pTmp = AutoPointer<char>(buf, size);
		pCompressedDataPage->uncompress(buf, length, i);
		ModOsDriver::Memory::copy(&buf[length], pBuffer_, uiLength_); 
		pCompressedDataPage->truncate(length);
		m_pBlock->m_uiLength -= length;
		uiLength_ += length;
		p = buf;
	}
	else
	{
		// キャスト
		p = syd_reinterpret_cast<const char*>(pBuffer_);
	}

	// 追加する
	m_pBlock->m_uiLength += uiLength_;
	while (uiLength_)
	{
		ModSize length = uiLength_;
		if (uiLength_ > CompressedDataPage::BlockSize)
			length = CompressedDataPage::BlockSize;
		if (pDataPage->append(p, length) == false)
		{
			// このページには入りきらないので新しいページ
			pDataPage = allocateDataPage(pDataPage);
			pDataPage->append(p, length);
		}
		p += length;
		uiLength_ -= length;
	}
}

//
//	FUNCTION public
//	Lob::CompressedLobData::replace -- データを書き換える
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
CompressedLobData::replace(ModSize uiPosition_,
						   const void* pBuffer_, ModSize uiLength_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Lob::CompressedLobData::verify -- 整合性検査を行う
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
CompressedLobData::verify(const ObjectID& cObjectID_)
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
	while (uiPosition < m_pBlock->m_uiLength)
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
		
		if (uiPosition != uiPrevSize)
		{
			// エラー
			_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
										m_cFile.getPath(),
										Message::InconsistentDataSize(id));
			return;
		}

		// ページ内すべてを伸長してサイズをチェックする
		CompressedDataPage* pCompressedDataPage
			= static_cast<CompressedDataPage*>(p.get());
		CompressedDataPage::Iterator i = pCompressedDataPage->begin();
		ModSize size = 0;
		for (; i != pCompressedDataPage->end(); ++i)
		{
			char buff[CompressedDataPage::BlockSize];
			ModSize s = CompressedDataPage::BlockSize;
			pCompressedDataPage->uncompress(buff, s, i);
			if (s != pCompressedDataPage->getLength(i))
			{
				// エラー
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::InconsistentDataSize(id));
				return;
			}
			size += s;
		}
		if (p->getLength() != size)
		{
			// エラー
			_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
										m_cFile.getPath(),
										Message::InconsistentDataSize(id));
			return;
		}
		uiPosition += p->getLength();
	}
}

//
//	Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
