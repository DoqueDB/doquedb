// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalBitmapIterator.cpp --
// 
// Copyright (c) 2007, 2017, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/NormalBitmapIterator.h"
#include "Bitmap/BitmapFile.h"

#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::NormalBitmapIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BitmapFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalBitmapIterator::NormalBitmapIterator(BitmapFile& cFile_)
	: BitmapIterator(),
	  m_cFile(cFile_), m_cDirPage(cFile_), m_cBitmapPage(cFile_),
	  m_bEndOfData(true), m_bFirstGet(true), m_pBitset(0), m_uiOffset(0),
	  m_uiBitmapSize(BitmapPage::getBitCount(cFile_.getPageSize()) / (sizeof(ModUInt32) * 8))
{
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::~NormalBitmapIterator -- デストラクタ
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
NormalBitmapIterator::~NormalBitmapIterator()
{
	m_cDirPage.detach();
	m_cBitmapPage.detach();
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::get -- 現在位置のビットマップを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ビットマップ
//
//	EXCEPTIONS
//
ModUInt32
NormalBitmapIterator::get()
{
	if (!isEnd())
	{
		if (m_bFirstGet)
		{
			attachBitmapPage();
		}
		return *m_pBitset;
	}
	return 0;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::getNext
//		-- 現在位置のビットマップを得て、次の位置に進む
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
ModUInt32
NormalBitmapIterator::getNext()
{
	ModUInt32 bitset = 0;
	
	if (!isEnd())
	{
		if (m_bFirstGet)
		{
			attachBitmapPage();
		}
		
		// 現在位置のデータを得る
		if (m_pBitset)
			bitset = *m_pBitset++;

		// 次へ
		++m_uiOffset;
		
		if (m_uiOffset == m_uiBitmapSize)
		{
			// このページには無いので次のページ
			m_cDirPage.next();
			
			if (m_cDirPage.isEnd())
			{
				// 終了
				m_bEndOfData = true;
			}
			else
			{
				// ビットマップページをattachする
				attachBitmapPage();
				m_uiOffset = 0;
			}
		}
	}

	return bitset;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::seek -- 移動する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		ModUInt32単位の先頭からのオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::seek(ModSize offset_)
{
	if (m_cDirPage.isAttached() == false)
	{
		m_bEndOfData = true;
		return;
	}
	
	m_bEndOfData = false;

	// DIRページをシークする
	m_cDirPage.seek(offset_ / m_uiBitmapSize);
	
	// ビットマップページをattachする
	attachBitmapPage();
	
	// ビット位置を移動する
	if (m_pBitset)
		m_pBitset += (offset_ % m_uiBitmapSize);
	m_uiOffset = offset_ % m_uiBitmapSize;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 position_
//		ビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::on(ModUInt32 position_)
{
	m_cDirPage.seek(position_ / (m_uiBitmapSize * sizeof(ModUInt32) * 8), true);
	
	if (m_cDirPage.get() == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ビットマップページがない
		m_cBitmapPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
		m_cBitmapPage.initialize();
		m_cDirPage.set(m_cBitmapPage.getID());
	}
	else if (m_cBitmapPage.isAttached() == false ||
			 m_cBitmapPage.getID() != m_cDirPage.get())
	{
		// ビットマップページをattachする
		attachBitmapPage();
	}

	// ビットをONする
	m_cBitmapPage.on(position_ % (m_uiBitmapSize * sizeof(ModUInt32) * 8));
}
											  
//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 position_
//		ビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::off(ModUInt32 position_)
{
	m_cDirPage.seek(position_ / (m_uiBitmapSize * sizeof(ModUInt32) * 8));
	
	if (m_cDirPage.get() == PhysicalFile::ConstValue::UndefinedPageID)
		// ビットマップページがない
		return;

	if (m_cBitmapPage.isAttached() == false ||
		m_cBitmapPage.getID() != m_cDirPage.get())
	{
		// ビットマップページをattachする
		attachBitmapPage();
	}

	// ビットをOFFする
	m_cBitmapPage.off(position_ % (m_uiBitmapSize * sizeof(ModUInt32) * 8));

	if (m_cBitmapPage.getCount() == 0)
	{
		// もうこのビットマップページにはエントリがない
		m_cBitmapPage.setFree();
		// DirPageのエントリを変更する
		m_cDirPage.set(PhysicalFile::ConstValue::UndefinedPageID);
	}
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::set -- ビットマップ単位でビットを設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		オフセット
//	ModUInt32 b_
//		ビットマップ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::set(ModSize offset_, ModUInt32 b_)
{
	m_cDirPage.seek(offset_ / m_uiBitmapSize, true);
	
	if (m_cDirPage.get() == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ビットマップページがない
		m_cBitmapPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
		m_cBitmapPage.initialize();
		m_cDirPage.set(m_cBitmapPage.getID());
	}
	else if (m_cBitmapPage.isAttached() == false ||
			 m_cBitmapPage.getID() != m_cDirPage.get())
	{
		// ビットマップページをattachする
		attachBitmapPage();
	}

	// ビットを設定する
	m_cBitmapPage.set(offset_ % m_uiBitmapSize, b_);
}
											  
//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::verify -- 整合性検査を行う
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
NormalBitmapIterator::verify()
{
	// The BitmapPage has been attached already in NormalBitmapFile::verify.
	// If NOT detach this here,
	// BitmapPage will NOT be detached in below while-loop,
	// and then memory will be exhausted.
	m_cBitmapPage.detach();
	
	while (m_cDirPage.isEnd() == false)
	{
		PhysicalFile::PageID id = m_cDirPage.get();
		if (id != PhysicalFile::ConstValue::UndefinedPageID)
		{
			m_cBitmapPage.setPhysicalPage(
				m_cFile.attachPhysicalPage(id));
			m_cBitmapPage.verify();
			m_cBitmapPage.detach();
		}
		m_cDirPage.next();

		if (m_cFile.isCancel() == true)
		{
			// 中断
			_SYDNEY_THROW0(Exception::Cancel);
		}
	}
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::setDirPage -- DirPageを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiDirPageID_
//		DIRページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::setDirPage(PhysicalFile::PageID uiDirPageID_)
{
	attachDirPage(uiDirPageID_);
	m_uiOffset = 0;
	m_bFirstGet = true;
	m_bEndOfData = false;
	m_pBitset = 0;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapIterator::unseDirPage -- DIRページを開放する
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
NormalBitmapIterator::unsetDirPage()
{
	m_cDirPage.detach();
	m_cBitmapPage.detach();
	m_bEndOfData = true;
}

//
//	FUNCTION private
//	Bitmap::NormalBitmapIterator::attachDirPage -- DIRページをattachする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		DIRページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapIterator::attachDirPage(PhysicalFile::PageID uiPageID_)
{
	if (uiPageID_ == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 新しいページを確保する
		m_cDirPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
		m_cDirPage.initialize();
	}
	else
	{
		// DIRページをattachする
		m_cDirPage.setPhysicalPage(m_cFile.attachPhysicalPage(
									   uiPageID_,
									   Buffer::Page::FixMode::ReadOnly));
	}
}

//
//	FUNCTION private
//	Bitmap::NormalBitmapIterator::attachBitmapPage
//		-- ビットマップページをattachする
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
NormalBitmapIterator::attachBitmapPage()
{
	m_cBitmapPage.detach();
	if (m_cDirPage.get() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ビットマップページをattachする
		m_cBitmapPage.setPhysicalPage(
			m_cFile.attachPhysicalPage(m_cDirPage.get()));
		m_pBitset = m_cBitmapPage.begin();
	}
	else
	{
		m_pBitset = 0;
	}
	m_bFirstGet = false;
}

//
//	Copyright (c) 2007, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
