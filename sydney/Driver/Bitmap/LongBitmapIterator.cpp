// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LongBitmapIterator.cpp -- 
// 
// Copyright (c) 2007, 2009, 2012, 2017, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/LongBitmapIterator.h"
#include "Bitmap/CompressedBitmapFile.h"
#include "Bitmap/MiddleBitmapIterator.h"
#include "Bitmap/ShortBitmapIterator.h"
#include "Bitmap/DirPage.h"
#include "Bitmap/BitmapPage.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"
#include "Exception/NotSupported.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::Area::getSize -- サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		エリアのサイズ
//
//	EXCEPTIONS
//
ModSize
LongBitmapIterator::Area::getSize() const
{
	return static_cast<ModSize>(m_cArea.getSize());
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::Area::begin -- 先頭のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//	const PhysicalFile::PageID*
//		先頭のページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID*
LongBitmapIterator::Area::begin()
{
	return syd_reinterpret_cast<PhysicalFile::PageID*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*())
		+ sizeof(Header));
}
const PhysicalFile::PageID*
LongBitmapIterator::Area::begin() const
{
	return syd_reinterpret_cast<const PhysicalFile::PageID*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ sizeof(Header));
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::Area::end -- 終端のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ページID*
//	const ページID*
//		終端のページID
//
//	EXCEPTIONS
//
PhysicalFile::PageID*
LongBitmapIterator::Area::end()
{
	return syd_reinterpret_cast<PhysicalFile::PageID*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*()) + getSize());
}
const PhysicalFile::PageID*
LongBitmapIterator::Area::end() const
{
	return syd_reinterpret_cast<const PhysicalFile::PageID*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ getSize());
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::LongBitmapIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::File* pDirectFile_
//		ダイレクトエリアファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LongBitmapIterator::LongBitmapIterator(CompressedBitmapFile& cFile_)
	: CompressedBitmapIterator(cFile_),
	  m_cIterator(cFile_),
	  m_pPageID(0),
	  m_bEndOfData(false), m_bFirstGet(true),
	  m_uiOffset(0),
	  m_uiBitmapSize(BitmapPage::getBitCount(
						 cFile_.getPageSize()) / (sizeof(ModUInt32) * 8)),
	  m_uiDirCount(DirPage::getMaxCount(cFile_.getPageSize()))
{
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::~LongBitmapIterator -- デストラクタ
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
LongBitmapIterator::~LongBitmapIterator()
{
	m_cArea.m_cArea.detach();
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::get -- 現在位置のビットマップを得る
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
LongBitmapIterator::get()
{
	if (!isEnd())
	{
		if (m_bFirstGet)
		{
			if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
			{
				m_cIterator.setDirPage(*m_pPageID);
				m_bFirstGet = false;
			}
		}
		return m_cIterator.get();
	}
	return 0;
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::getNext
//		-- 現在位置のビットマップを得て、次の位置に進む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		現在位置のビットマップ
//
//	EXCEPTIONS
//
ModUInt32
LongBitmapIterator::getNext()
{
	ModUInt32 bitset = 0;
	
	if (!isEnd())
	{
		if (m_bFirstGet)
		{
			if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
			{
				m_cIterator.setDirPage(*m_pPageID);
				m_bFirstGet = false;
			}
		}
		
		// 現在位置のデータを得る
		bitset = m_cIterator.getNext();

		// 次へ
		++m_uiOffset;
		
		if (m_uiOffset == (m_uiBitmapSize * m_uiDirCount))
		{
			// このページには無いので次のページ
			++m_pPageID;

			// イテレータを初期化する
			m_cIterator.unsetDirPage();
			m_uiOffset = 0;
			
			if (m_pPageID == const_cast<const Area&>(m_cArea).end())
			{
				// 終了
				m_bEndOfData = true;
			}
			else if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
			{
				m_cIterator.setDirPage(*m_pPageID);
			}
		}
	}

	return bitset;
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::seek -- 移動する(ModUInt32単位)
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		オフセット
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
void
LongBitmapIterator::seek(ModSize offset_)
{
	(void) seek(offset_, false);
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		最初のROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LongBitmapIterator::initialize(ModUInt32 uiRowID_)
{
	// 呼ばれない
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::setArea -- エリアを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea& cArea_
//		エリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LongBitmapIterator::setArea(const PhysicalFile::DirectArea& cArea_)
{
	m_cArea.m_cArea = cArea_;
	m_pPageID = const_cast<const Area&>(m_cArea).begin();
	m_bEndOfData = false;
	m_bFirstGet = true;
	m_uiOffset = 0;
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ビット立てるROWID
//
//	RETURN
//	Bitmap::CompressedBitmapIterator::Result::Value
//		Success		そのまま追加できた
//		Modify		AreaIDが変更された
//		NeedConvert	ミドルリストへのコンバートが必要
//
//	EXCEPTIONS
//
CompressedBitmapIterator::Result::Value
LongBitmapIterator::on(ModUInt32 uiRowID_)
{
	Result::Value r = Result::Success;

	// ビットをONする位置まで進める
	r = seek(uiRowID_ / (sizeof(ModUInt32) * 8), true);
	if (r == Result::NeedConvert)
		// ロングリストでは格納できない
		return r;

	// ビットをONする
	m_cIterator.on(uiRowID_ %
				   (m_uiBitmapSize * m_uiDirCount * sizeof(ModUInt32) * 8));
	
	return r;
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ビットをOFFするROWID
//
//	RETURN
//	Bitmap::CompressedBitmapIterator::Result::Value
//		Success		そのまま削除できた
//		Modify		AreaIDが変更された
//		Deleted		エリア内の件数が0になり削除した
//
//	EXCEPTIONS
//
CompressedBitmapIterator::Result::Value
LongBitmapIterator::off(ModUInt32 uiRowID_)
{
	Result::Value r = Result::Success;

	// ビットをOFFする位置まで移動する
	seek(uiRowID_ / (sizeof(ModUInt32) * 8));

	if (*m_pPageID == PhysicalFile::ConstValue::UndefinedPageID)
		// DIRページがない
		return r;

	// ビットをOFFする
	m_cIterator.off(uiRowID_ %
					(m_uiBitmapSize * m_uiDirCount * sizeof(ModUInt32) * 8));

	return r;
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::convert -- 超ロングリストへコンバートする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		挿入するROWID
//
//	RETURN
//	CompressedBitmapIterator*
//		ロングリスト
//
//	EXCEPTIONS
//
CompressedBitmapIterator*
LongBitmapIterator::convert(ModUInt32 uiRowID_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Bitmap::LongBitmapIterator::verify -- 整合性を検査する
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
LongBitmapIterator::verify()
{
	m_cIterator.unsetDirPage();

	while (m_pPageID != const_cast<const Area&>(m_cArea).end())
	{
		if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			m_cIterator.setDirPage(*m_pPageID);
			m_cIterator.verify();
			m_cIterator.unsetDirPage();
		}

		++m_pPageID;
		
		if (m_cFile.isCancel() == true)
		{
			// 中断
			_SYDNEY_THROW0(Exception::Cancel);
		}
	}
}

//
//	FUNCTION private
//	Bitmap::LongBitmapIterator::seek -- 移動する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiOffset_
//		ModUInt32単位のオフセット
//	bool bUpdate_
//		更新かどうか
//
//	RETURN
//	Bitmap::CompressedBitmapIterator::Result::Value
//		Success		そのまま移動できた
//		Modify		AreaIDが変更された
//
//	EXCEPTIONS
//
CompressedBitmapIterator::Result::Value
LongBitmapIterator::seek(ModSize uiOffset_, bool bUpdate_)
{
	m_cIterator.unsetDirPage();
	
	Result::Value r = Result::Success;
	m_bEndOfData = false;
	m_uiOffset = uiOffset_ % (m_uiBitmapSize * m_uiDirCount);

	const Area& cArea = m_cArea;

	// ビットマップページの位置
	ModSize pos = uiOffset_ / m_uiBitmapSize / m_uiDirCount;
	if ((cArea.end() - cArea.begin()) < static_cast<int>(pos + 1))
	{
		// 今の大きさを越えている
		if (bUpdate_ == false)
		{
			m_bEndOfData = true;
			return r;
		}

		// エリアを拡張する
		ModSize s = pos + 1 - static_cast<ModSize>(cArea.end() - cArea.begin());
		ModSize newSize = cArea.getSize() + s * sizeof(PhysicalFile::PageID);
		if (newSize > getMaxStorableAreaSize())
		{
			// エリアの最大サイズを超えてしまう
			return Result::NeedConvert;
		}
		PhysicalFile::DirectArea cNewArea = m_cFile.allocateArea(newSize);
		Os::Memory::copy(cNewArea.operator void*(),
						 cArea.m_cArea.operator const void*(),
						 cArea.getSize());
		m_cArea.m_cArea.expunge(getTransaction());
		m_cArea.m_cArea = cNewArea;
		PhysicalFile::PageID* pPageID = (m_cArea.end() - s);
		for (; pPageID != m_cArea.end(); ++pPageID)
			*pPageID = PhysicalFile::ConstValue::UndefinedPageID;

		r = Result::Modify;
	}

	m_pPageID = cArea.begin() + pos;
	if (bUpdate_)
	{
		// 更新のためならとにかくDIRページを設定する
		// NormalBitmapIterator内でDIRページが確保される
		
		m_cIterator.setDirPage(*m_pPageID);
		m_bFirstGet = false;
		if (*m_pPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			*(m_cArea.begin() + pos) = m_cIterator.getDirPageID();
			m_cArea.dirty();
			m_pPageID = cArea.begin() + pos;
		}
	}
	else if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ページがあるので設定
		
		m_cIterator.setDirPage(*m_pPageID);
		m_bFirstGet = false;
	}

	return r;
}

//
//	FUNCTION private
//	Bitmap::LongBitmapIterator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::MiddleBitmapIterator* i
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LongBitmapIterator::initialize(MiddleBitmapIterator* i)
{
	// DIRページに物理ページを割り当てる
	DirPage cDirPage(m_cFile);
	cDirPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
	// 初期化する
	cDirPage.initialize();

	// ページをコピーする
	PhysicalFile::PageID* pBegin = i->m_cArea.begin();
	PhysicalFile::PageID* pEnd = i->m_cArea.end();
	Os::Memory::copy(cDirPage.m_pBegin, pBegin,
					 static_cast<Os::Memory::Size>(pEnd - pBegin)
					 * sizeof(PhysicalFile::PageID));

	// エリアを得る
	m_cArea.m_cArea = m_cFile.allocateArea(sizeof(Area::Header) +
										   sizeof(PhysicalFile::PageID) * 2);
	m_cArea.dirty();
	*(syd_reinterpret_cast<ModUInt32*>(m_cArea.m_cArea.operator void*()))
		= Type::Long;
	PhysicalFile::PageID* p = m_cArea.begin();
	for (; p != m_cArea.end(); ++p)
		*p = PhysicalFile::ConstValue::UndefinedPageID;
	*(m_cArea.begin()) = cDirPage.getID();
}

//
//	FUNCTION private
//	Bitmap::LongBitmapIterator::initializeArea -- エリアを初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiMaxRowID_
//		最大ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LongBitmapIterator::initializeArea(ModUInt32 uiMaxRowID_)
{
	ModSize s
		= uiMaxRowID_ / (m_uiBitmapSize * m_uiDirCount * sizeof(ModUInt32) * 8);
	
	// エリアを得る
	m_cArea.m_cArea
		= m_cFile.allocateArea(sizeof(Area::Header) +
							   sizeof(PhysicalFile::PageID) * (s + 1));
	m_cArea.dirty();
	*(syd_reinterpret_cast<ModUInt32*>(m_cArea.m_cArea.operator void*()))
		= Type::Long;
	PhysicalFile::PageID* p = m_cArea.begin();
	for (; p != m_cArea.end(); ++p)
		*p = PhysicalFile::ConstValue::UndefinedPageID;
}

//
//	FUNCTION private
//	Bitmap::LongBitmapIterator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::ShortBitmapIterator* i
//		ショートリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LongBitmapIterator::insert(ShortBitmapIterator* i)
{
	ModSize offset = 0;
	while (i->isEnd() == false)
	{
		ModUInt32 b = i->getNext();
		if (b != 0)
		{
			set(offset, b);
		}
		++offset;
	}
}

//
//	FUNCTION private
//	Bitmap::LongBitmapIterator::set
//		-- ビットマップ単位(ModUInt32単位)でビットを設定する
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
LongBitmapIterator::set(ModSize offset_, ModUInt32 b_)
{
	// ビットマップの位置まで移動する
	seek(offset_, true);

	// NormalBitmapIteratorにビットセット設定する
	// m_uiOffset は seek 内で設定される
	m_cIterator.set(m_uiOffset, b_);
}

//
//	Copyright (c) 2007, 2009, 2012, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
