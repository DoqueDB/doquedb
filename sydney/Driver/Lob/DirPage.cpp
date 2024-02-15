// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirPage.cpp --
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
#include "Lob/DirPage.h"
#include "Lob/LobFile.h"

#include "Exception/BadArgument.h"

#include "Common/Assert.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::DirPage::DirPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DirPage::DirPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_)
{
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
	m_pEntry = syd_reinterpret_cast<Entry*>(getBuffer() + sizeof(Header));
}

//
//	FUNCTION public
//	Lob::DirPage::DirPage -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	ModUInt32 uiStep_
//		段数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DirPage::DirPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
					 ModUInt32 uiStep_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_)
{
	dirty();
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
	m_pEntry = syd_reinterpret_cast<Entry*>(getBuffer() + sizeof(Header));
	m_pHeader->m_uiStep = uiStep_;
	m_pHeader->m_uiCount = 0;
}

//
//	FUNCTION public
//	Lob::DirPage::~DirPage -- デストラクタ
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
DirPage::~DirPage()
{
}

//
//	FUNCTION public
//	Lob::DirPage::getPageID -- 指定番目のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiNumber_
//		何番目か
//
//	RETURN
//	PhysicalFile::PageID
//		物理ページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
DirPage::getPageID(ModSize uiNumber_)
{
	return (m_pEntry + uiNumber_)->m_uiPageID;
}

//
//	FUNCTION public
//	Lob::DirPage::getPageID -- 指定位置のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		取得するデータの先頭位置
//	ModSize& uiDataSize_
//		返されたページの先頭位置
//
//	RETURN
//	PhysicalFile::PageID
//		物理ページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
DirPage::getPageID(ModSize uiPosition_, ModSize& uiDataSize_)
{
	Entry* pEntry = m_pEntry;
	uiDataSize_ = pEntry->m_uiSize;
	while (uiDataSize_ <= uiPosition_)
	{
		++pEntry;
		if (static_cast<ModSize>(pEntry - m_pEntry) >= m_pHeader->m_uiCount)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		uiDataSize_ += pEntry->m_uiSize;
	}
	uiDataSize_ -= pEntry->m_uiSize;
	return pEntry->m_uiPageID;
}

//
//	FUNCTION public
//	Lob::DirPage::addDataPageID -- DATAページIDを追加する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		追加するDATAページのページID
//	ModSize uiDataSize_
//		追加するDATAページ内のサイズ
//
//	RETURN
//	PhysicalFile::PageID
//		入りきれなかった場合に新しくallocateしたページID。入りきった場合は
//		PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	EXCEPTIONS
//
PhysicalFile::PageID
DirPage::addDataPageID(PhysicalFile::PageID uiPageID_,
					   ModSize uiDataSize_)
{
	PhysicalFile::PageID ret = PhysicalFile::ConstValue::UndefinedPageID;
	
	if (m_pHeader->m_uiStep == 1)
	{
		ret = addPageID(uiPageID_, uiDataSize_);
	}
	else
	{
		dirty();
		// 最後のエントリを得る
		Entry* pEntry = m_pEntry + m_pHeader->m_uiCount - 1;
		DirPage::PagePointer p = m_cFile.attachDirPage(pEntry->m_uiPageID);
		PhysicalFile::PageID id = p->addDataPageID(uiPageID_, uiDataSize_);
		if (id != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// ページが増えたので、ここにも加える
			ret = addPageID(id, uiDataSize_);
		}
		else
		{
			// 最後のエントリのサイズを増やす
			pEntry->m_uiSize += uiDataSize_;
		}
	}

	return ret;
}

//
//	FUNCTION public
//	Lob::DirPage::addPageID -- ページIDを追加する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		追加するページID
//	ModSize uiDataSize_
//		追加するページ内のサイズ
//
//	RETURN
//	PhysicalFile::PageID
//		入りきれなかった場合に新しくallocateしたページID。入りきった場合は
//		PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	EXCEPTIONS
//
PhysicalFile::PageID
DirPage::addPageID(PhysicalFile::PageID uiPageID_,
				   ModSize uiDataSize_)
{
	PhysicalFile::PageID ret = PhysicalFile::ConstValue::UndefinedPageID;
	
	if (getFreeSize() < sizeof(Entry))
	{
		// ここには加えられない -> 新しいページに加える
		DirPage::PagePointer p = m_cFile.allocateDirPage(m_pHeader->m_uiStep);
		ret = p->getID();
		p->addPageID(uiPageID_, uiDataSize_);
	}
	else
	{
		// ここに入る
		dirty();
		(m_pEntry + m_pHeader->m_uiCount)->m_uiPageID = uiPageID_;
		(m_pEntry + m_pHeader->m_uiCount)->m_uiSize = uiDataSize_;
		m_pHeader->m_uiCount++;
	}

	return ret;
}

//
//	FUNCTION public
//	Lob::DirPage::delDataPageID -- DATAページIDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		削除するDATAページのページID
//	ModSize uiDataSize_
//		削除するDATAページのデータサイズ
//
//	RETURN
//	ModUInt32
//		残ったエントリ数
//
//	EXCEPTIONS
//
ModUInt32
DirPage::delDataPageID(PhysicalFile::PageID uiPageID_,
					   ModSize uiDataSize_)
{
	ModUInt32 count = m_pHeader->m_uiCount;
	
	if (m_pHeader->m_uiStep == 1)
	{
		count = delPageID(uiPageID_, uiDataSize_);
	}
	else
	{
		dirty();
		// 最後のエントリを得る
		Entry* pEntry = m_pEntry + m_pHeader->m_uiCount - 1;
		DirPage::PagePointer p = m_cFile.attachDirPage(pEntry->m_uiPageID);
		if (p->delDataPageID(uiPageID_, uiDataSize_) == 0)
		{
			PhysicalFile::PageID id = p->getID();
			// 空になったのでこのページを削除する
			m_cFile.freePage(p.get());
			// ここのエントリを削除する
			count = delPageID(id, uiDataSize_);
		}
		else
		{
			// 最後のエントリのサイズを変更する
			pEntry->m_uiSize -= uiDataSize_;
		}
	}
	return count;
}

//
//	FUNCTION public
//	Lob::DirPage::delPageID -- ページIDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		削除するページID
//	ModSize uiDataSize_
//		削除するDATAページのデータサイズ
//
//	RETURN
//	ModUInt32
//		残ったエントリ数
//
//	EXCEPTIONS
//
ModUInt32
DirPage::delPageID(PhysicalFile::PageID uiPageID_, ModSize uiDataSize_)
{
	// 最後のエントリを削除する
	dirty();
	; _SYDNEY_ASSERT((m_pEntry + m_pHeader->m_uiCount - 1)->m_uiPageID
					 == uiPageID_);
	m_pHeader->m_uiCount--;

	return m_pHeader->m_uiCount;
}

//
//	FUNCTION public
//	Lob::DirPage::freePage -- 下位の全ページを開放する
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
DirPage::freePage()
{
	Entry* pEntry = m_pEntry;
	
	if (m_pHeader->m_uiStep == 1)
	{
		for (int i = 0; i < static_cast<int>(m_pHeader->m_uiCount); ++i)
		{
			m_cFile.freePage(pEntry->m_uiPageID);
			++pEntry;
		}
	}
	else
	{
		for (int i = 0; i < static_cast<int>(m_pHeader->m_uiCount); ++i)
		{
			DirPage::PagePointer p = m_cFile.attachDirPage(pEntry->m_uiPageID);
			p->freePage();
			m_cFile.freePage(p.get());
			++pEntry;
		}
	}
}

//
//	FUNCTION public static
//	Lob::DirPage::getMaxCount -- 最大エントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	Os::Memory::Size uiPageSize_
//		ページサイズ
//
//	RETURN
//	ModSize
//		最大エントリ数
//
//	EXCEPTIONS
//
ModSize
DirPage::getMaxCount(Os::Memory::Size uiPageSize_)
{
	return (getPageSize(uiPageSize_) - sizeof(Header)) / sizeof(Entry);
}

//
//	Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
