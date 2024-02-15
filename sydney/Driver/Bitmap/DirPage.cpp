// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirPage.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2009, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/DirPage.h"
#include "Bitmap/BitmapFile.h"
#ifdef DEBUG
#include "Bitmap/Parameter.h"
#endif

#include "PhysicalFile/File.h"

#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
#ifdef DEBUG
	//	ページサイズ -- デバッグ用に利用する
	ParameterInteger _cDebugPageSize("Bitmap_DebugPageSize", 0);
#endif
}

//
//	FUNCTION public
//	Bitmap::DirPage::DirPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BitmapFiel& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DirPage::DirPage(BitmapFile& cFile_)
	: NodePage(cFile_), m_cFile(cFile_), m_pHeader(0), m_pBegin(0), m_pEnd(0),
	  m_pCurrent(0), m_pDirPage(0), m_bEndOfData(false)
{
}

//
//	FUNCTION public
//	Bitmap::DirPage::~DirPage -- デストラクタ
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
	detach();
}

//
//	FUNCTION public
//	Bitmap::DirPage::next -- 次に移動する
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
DirPage::next()
{
	if (isEnd())
		// すでに終端
		return;
	
	if (m_pHeader->m_iStepCount > 0)
	{
		// 下位がある
		m_pDirPage->next();
		if (m_pDirPage->isEnd())
		{
			// 下位が終了したので、ここを次にする
			++m_pCurrent;
			if (m_pCurrent == m_pEnd ||
				(*m_pCurrent) == PhysicalFile::ConstValue::UndefinedPageID)
			{
				// 終了
				m_bEndOfData = true;
				return;
			}
			m_pDirPage->detach();
			m_pDirPage->setPhysicalPage(
				m_cFile.attachPhysicalPage(*m_pCurrent,
										   Buffer::Page::FixMode::ReadOnly));
		}
	}
	else
	{
		// 下位がない
		++m_pCurrent;
		if (m_pCurrent == m_pEnd)
		{
			// 終了
			m_bEndOfData = true;
			return;
		}
	}
}

//
//	FUNCTION public
//	Bitmap::DirPage::seek -- 指定位置に移動する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		先頭からのオフセット値
//	bool isUpdate_
//		更新モードかどうか。trueの場合、指定された位置にページが
//		存在しない場合は、新しいページを確保する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirPage::seek(ModSize offset_, bool isUpdate_)
{
	m_bEndOfData = false;
	
	// まず、与えられたオフセット値が表現できるDIRページの段数を求める
	// 今は最高で2段なので、それを想定したコードにする
	
	ModSize off = offset_;
	ModSize size = static_cast<ModSize>(m_pEnd - m_pBegin);
	int iStepCount = (off / size) ? 1 : 0;

	// 現在の段数でいいかチェックする
	if (m_pHeader->m_iStepCount < iStepCount)
	{
		if (isUpdate_)
		{
			dirty();
		
			// 現在の段数では表現できないので、段数を増やす
			DirPage cNewPage(m_cFile);
			cNewPage.setPhysicalPage(m_cFile.allocatePhysicalPage());

			// 自分の内容をコピーする
			cNewPage.setStepCount(m_pHeader->m_iStepCount);
			Os::Memory::copy(cNewPage.m_pBegin, m_pBegin,
							 size * sizeof(PhysicalFile::PageID));

			// 自分を初期化する
			initialize();
			setStepCount(iStepCount);

			PhysicalFile::PageID* p = m_pBegin;
			*p++ = cNewPage.getID();
			cNewPage.detach();

			// メモリーを確保する
			m_pDirPage = new DirPage(m_cFile);
		}
		else
		{
			m_bEndOfData = true;
			return;
		}
	}

	if (m_pHeader->m_iStepCount)
	{
		// 下位がある
		m_pCurrent = m_pBegin;
		for (ModSize i = 0; i < (off/size) + 1; ++i, ++m_pCurrent)
		{
			if (*m_pCurrent == PhysicalFile::ConstValue::UndefinedPageID)
			{
				if (isUpdate_)
				{
					dirty();
				
					// まだこの下位のDIRページは確保されていない
					m_pDirPage->detach();
					m_pDirPage->setPhysicalPage(m_cFile.allocatePhysicalPage());
					m_pDirPage->initialize();

					// エントリを変更する
					*m_pCurrent = m_pDirPage->getID();
				}
				else
				{
					m_bEndOfData = true;
					return;
				}
			}
		}
		--m_pCurrent;

		// まずはこのページの直下のページを確保する
		if (!m_pDirPage->getPhysicalPage() ||
			m_pDirPage->getID() != *m_pCurrent)
		{
			m_pDirPage->detach();
			m_pDirPage->setPhysicalPage(
				m_cFile.attachPhysicalPage(*m_pCurrent,
										   Buffer::Page::FixMode::ReadOnly));
		}
		// 下位ページをseekする
		m_pDirPage->seek(off % size);
	}
	else
	{
		m_pCurrent = m_pBegin + off;
	}
}

//
//	FUNCTION public
//	Bitmap::DirPage::get -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		ページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
DirPage::get()
{
	PhysicalFile::PageID uiPageID = PhysicalFile::ConstValue::UndefinedPageID;
	if (!isEnd())
	{
		if (m_pHeader->m_iStepCount > 0)
			// 下位がある
			uiPageID = m_pDirPage->get();
		else
			// 下位がない
			uiPageID = *m_pCurrent;
	}
	return uiPageID;
}

//
//	FUNCTION public
//	Bitmap::DirPage::set -- 現在位置にページIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		設定するページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirPage::set(PhysicalFile::PageID uiPageID_)
{
	if (m_pHeader->m_iStepCount > 0)
	{
		// 下位がある
		m_pDirPage->set(uiPageID_);
	}
	else
	{
		dirty();
		*m_pCurrent = uiPageID_;
	}
}

//
//	FUNCTION public
//	Bitmap::DirPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DirPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	NodePage::setPhysicalPage(pPhysicalPage_);
	load();

	// 下位があったら下位もattachする
	if (m_pHeader->m_iStepCount)
	{
		m_pDirPage = new DirPage(m_cFile);
		m_pDirPage->setPhysicalPage(
			m_cFile.attachPhysicalPage(*m_pBegin,
									   Buffer::Page::FixMode::ReadOnly));
	}
}

//
//	FUNCTION public
//	Bitmap::DirPage::initialize -- 初期化する
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
DirPage::initialize()
{
	dirty();
	m_pHeader->m_iStepCount = 0;
	PhysicalFile::PageID* i = m_pBegin;
	for (; i != m_pEnd; ++i)
	{
		(*i) = PhysicalFile::ConstValue::UndefinedPageID;
	}
}

//
//	FUNCTION public
//	Bitmap::DirPage::detach -- ページをdetachする
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
DirPage::detach()
{
	if (m_pDirPage)
	{
		m_pDirPage->detach();
		delete m_pDirPage, m_pDirPage = 0;
	}
	NodePage::detach();
	m_pHeader = 0;
	m_pBegin = m_pEnd = 0;
}

//
//	FUNCTION public static
//	Bitmap::DirPage::getMaxCount -- 格納できるエントリの最大数を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPageSize_
//		ページサイズ
//
//	RETURN
//	ModSize
//		格納できるエントリの最大数
//
//	EXCEPTIONS
//
ModSize
DirPage::getMaxCount(Os::Memory::Size uiPageSize_)
{
#ifdef DEBUG
	if (_cDebugPageSize.get() != 0)
		return (_cDebugPageSize.get() - sizeof(Header))
			/ sizeof(PhysicalFile::PageID);
#endif
	return (PhysicalFile::File::getPageDataSize(
				PhysicalFile::PageManageType, uiPageSize_) - sizeof(Header))
		/ sizeof(PhysicalFile::PageID);
}

//
//	FUNCTION private
//	Bitmap::DirPage::load -- ページの内容を読み込む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRURN
//	なし
//
//	EXCEPTIONS
//
void
DirPage::load()
{
	PhysicalFile::PageID* p
		= syd_reinterpret_cast<PhysicalFile::PageID*>(getBuffer());
	m_pHeader = syd_reinterpret_cast<Header*>(p);
	m_pBegin = p + sizeof(Header)/sizeof(ModUInt32);
	m_pEnd = p + getPageSize();
	m_pCurrent = m_pBegin;
	m_bEndOfData = false;
}

//
//	FUNCTION private
//	Bitmap::DirPage::reload -- ページが更新されたので読み直す
//
//	NOTES
//	これはページの中身は同じだが、メモリーだけが変更された場合に
//	呼び出される
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
DirPage::reload()
{
	int off = static_cast<int>(m_pCurrent - m_pBegin);
	
	PhysicalFile::PageID* p
		= syd_reinterpret_cast<PhysicalFile::PageID*>(getBuffer());
	m_pHeader = syd_reinterpret_cast<Header*>(p);
	m_pBegin = p + sizeof(Header)/sizeof(ModUInt32);
	m_pEnd = p + getPageSize();
	m_pCurrent = m_pBegin + off;
}

//
//	Copyright (c) 2005, 2006, 2007, 2009, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
