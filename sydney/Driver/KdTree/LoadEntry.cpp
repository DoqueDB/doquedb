// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadEntry.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/LoadEntry.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::LoadEntry::LoadEntry -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::DataFile& cDataFile_
//		エントリデータ格納ファイル
//	KdTree::Allocator& cAllocator_
//		エントリ格納領域を確保するためのアロケータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LoadEntry::LoadEntry(DataFile& cDataFile_,
					 Allocator& cAllocator_)
	: m_cDataFile(cDataFile_), m_cAllocator(cAllocator_),
	  m_uiCurrentPageID(0), m_iParallelCount(0)
{
}

//
//	FUNCTION public
//	KdTree::LoadEntry::~LoadEntry -- デストラクタ
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
LoadEntry::~LoadEntry()
{
}

//
//	FUNCTION public
//	KdTree::LoadEntry::prepare -- 準備する
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
LoadEntry::prepare()
{
	// スレッド数分のDataFileを確保する
	//
	//【注意】	DataFile のオープンモードが ReadOnly の場合は並列実行するが、
	//			そうではない場合、1スレッドでの実行となる
	//
	//			なぜなら、
	//			並列実行時に DataFile のインスタンスはスレッド分コピーされるが、
	//			Version::Page は共有される
	//			Version::Page は ReadWrite アクセスの場合、
	//			複数のスレッドから同時にアクセスされることを想定していない
	
	m_iParallelCount = getThreadSize();
	m_vecpDataFile.reserve(m_iParallelCount);
	
	for (int i = 0; i < m_iParallelCount; ++i)
	{
		if (m_cDataFile.getFixMode() == Buffer::Page::FixMode::ReadOnly)
		{
			m_vecpDataFile.pushBack(m_cDataFile.copy());
		}
		else
		{
			if (i == 0)
				m_vecpDataFile.pushBack(&m_cDataFile);
			else
				m_vecpDataFile.pushBack(0);
		}
	}

	// 読み込んだエントリのポインターを格納する領域を確保する
	m_vecpEntry.reserve(m_cDataFile.getCount());
}

//
//	FUNCTION public
//	KdTree::LoadEntry::parallel -- 平行実行する
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
LoadEntry::parallel()
{
	// 自スレッド用のDataFile
	DataFile* pDataFile = m_vecpDataFile[getThreadNum()];
	if (pDataFile == 0)
		return;

	// 1ページ分のエントリのポインタを格納する配列
	Common::LargeVector<Entry*> vecpPageEntry;
	vecpPageEntry.reserve(pDataFile->getCountPerPage());

	DataFile::PageID pageID;
	while ((pageID = getNextPageID()) != DataFile::IllegalPageID)
	{
		vecpPageEntry.erase(vecpPageEntry.begin(), vecpPageEntry.end());

		pDataFile->getPageData(pageID, m_cAllocator,
							   vecpPageEntry);

		{
			// マージする
			
			Os::AutoCriticalSection cAuto(m_cLatch);
			m_vecpEntry.insert(m_vecpEntry.end(),
							   vecpPageEntry.begin(), vecpPageEntry.end());
		}
	}

	if (m_cDataFile.getFixMode() == Buffer::Page::FixMode::ReadOnly)
		// ページを解放する
		pDataFile->flushAllPages();
}

//
//	FUNCTION public
//	KdTree::LoadEntry::dispose -- 後処理する
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
LoadEntry::dispose()
{
	if (m_cDataFile.getFixMode() == Buffer::Page::FixMode::ReadOnly)
	{
		ModVector<DataFile*>::Iterator i = m_vecpDataFile.begin();
		for (; i != m_vecpDataFile.end(); ++i)
		{
			delete (*i);
		}
	}
	m_vecpDataFile.clear();
}

//
//	FUNCTION private
//	KdTree::LoadEntry::getNextPageID
//		-- 次に読み込むべきページのページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::DataFile::PageID
//		次に読み込むべきページのページID
//
//	EXCEPTIONS
//
DataFile::PageID
LoadEntry::getNextPageID()
{
	Os::AutoCriticalSection cAuto(m_cPageLatch);

	if (m_uiCurrentPageID != DataFile::IllegalPageID)
		m_uiCurrentPageID = m_cDataFile.getNextPageID(m_uiCurrentPageID);
	
	return m_uiCurrentPageID;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
