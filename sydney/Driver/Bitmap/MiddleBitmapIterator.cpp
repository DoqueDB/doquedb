// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBitmapIterator.cpp -- 
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
#include "Bitmap/MiddleBitmapIterator.h"
#include "Bitmap/CompressedBitmapFile.h"
#include "Bitmap/ShortBitmapIterator.h"
#include "Bitmap/LongBitmapIterator.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::Area::getSize -- サイズを得る
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
MiddleBitmapIterator::Area::getSize() const
{
	return static_cast<ModSize>(m_cArea.getSize());
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::Area::begin -- 先頭のページIDを得る
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
MiddleBitmapIterator::Area::begin()
{
	return syd_reinterpret_cast<PhysicalFile::PageID*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*())
		+ sizeof(Header));
}
const PhysicalFile::PageID*
MiddleBitmapIterator::Area::begin() const
{
	return syd_reinterpret_cast<const PhysicalFile::PageID*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ sizeof(Header));
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::Area::end -- 終端のページIDを得る
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
MiddleBitmapIterator::Area::end()
{
	return syd_reinterpret_cast<PhysicalFile::PageID*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*()) + getSize());
}
const PhysicalFile::PageID*
MiddleBitmapIterator::Area::end() const
{
	return syd_reinterpret_cast<const PhysicalFile::PageID*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ getSize());
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::MiddleBitmapIterator -- コンストラクタ
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
MiddleBitmapIterator::MiddleBitmapIterator(CompressedBitmapFile& cFile_)
	: CompressedBitmapIterator(cFile_),
	  m_pPageID(0), m_cBitmapPage(cFile_),
	  m_bEndOfData(false), m_bFirstGet(true),
	  m_pBitset(0), m_uiOffset(0),
	  m_uiBitmapSize(BitmapPage::getBitCount(
						 cFile_.getPageSize()) / (sizeof(ModUInt32) * 8))
{
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::~MiddleBitmapIterator -- デストラクタ
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
MiddleBitmapIterator::~MiddleBitmapIterator()
{
	m_cArea.m_cArea.detach();
	m_cBitmapPage.detach();
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::get -- 現在位置のビットマップを得る
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
MiddleBitmapIterator::get()
{
	if (!isEnd())
	{
		if (m_bFirstGet)
		{
			attachBitmapPage();
		}
		if (m_pBitset)
			return *m_pBitset;
	}
	return 0;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::getNext
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
MiddleBitmapIterator::getNext()
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
			++m_pPageID;
			
			if (m_pPageID == const_cast<const Area&>(m_cArea).end())
			{
				// 終了
				m_bEndOfData = true;
			}
			else
			{
				// ビットマップページをattachする
				m_uiOffset = 0;
				attachBitmapPage();
			}
		}
	}

	return bitset;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::seek -- 移動する(ModUInt32単位)
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
MiddleBitmapIterator::seek(ModSize offset_)
{
	(void) seek(offset_, false);
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::initialize -- 初期化する
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
MiddleBitmapIterator::initialize(ModUInt32 uiRowID_)
{
	// 呼ばれない
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::setArea -- エリアを設定する
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
MiddleBitmapIterator::setArea(const PhysicalFile::DirectArea& cArea_)
{
	m_cArea.m_cArea = cArea_;
	m_pPageID = const_cast<const Area&>(m_cArea).begin();
	m_bEndOfData = false;
	m_bFirstGet = true;
	m_pBitset = 0;
	m_uiOffset = 0;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::on -- ビットをONする
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
MiddleBitmapIterator::on(ModUInt32 uiRowID_)
{
	Result::Value r = Result::Success;
	m_uiOffset = 0;

	// ビットをONする位置まで移動する
	r = seek(uiRowID_ / (sizeof(ModUInt32) * 8), true);
	if (r == Result::NeedConvert)
		// ミドルリストでは格納できない
		return r;

	// ビットをONする
	m_cBitmapPage.on(uiRowID_ % (m_uiBitmapSize * sizeof(ModUInt32) * 8));

	return r;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::off -- ビットをOFFする
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
MiddleBitmapIterator::off(ModUInt32 uiRowID_)
{
	m_uiOffset = 0;
	Result::Value r = Result::Success;

	// ビットをOFFする位置まで移動する
	seek(uiRowID_ / (sizeof(ModUInt32) * 8));

	if (isEnd() || *m_pPageID == PhysicalFile::ConstValue::UndefinedPageID)
		// ビットマップページがない
		return r;

	if (m_cBitmapPage.isAttached() == false ||
		m_cBitmapPage.getID() != *m_pPageID)
	{
		// ビットマップページをattachする
		attachBitmapPage();
	}

	// ビットをOFFする
	m_cBitmapPage.off(uiRowID_ % (m_uiBitmapSize * sizeof(ModUInt32) * 8));

	if (m_cBitmapPage.getCount() == 0)
	{
		// もうこのビットマップページにはエントリがない
		m_cBitmapPage.setFree();
		// エントリを変更する
		*(m_cArea.begin()
		  + (m_pPageID - const_cast<const Area&>(m_cArea).begin()))
			= PhysicalFile::ConstValue::UndefinedPageID;
		m_cArea.dirty();
	}

	return r;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::convert -- ロングリストへコンバートする
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
MiddleBitmapIterator::convert(ModUInt32 uiRowID_)
{
	LongBitmapIterator* i = new LongBitmapIterator(m_cFile);
	i->initialize(this);

	// 自身の不要なものを削除する
	m_cArea.m_cArea.expunge(getTransaction());

	// 新しいROWIDを挿入する
	i->on(uiRowID_);

	return i;
}

//
//	FUNCTION public
//	Bitmap::MiddleBitmapIterator::verify -- 整合性を検査する
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
MiddleBitmapIterator::verify()
{
	m_cBitmapPage.detach();

	while (m_pPageID != const_cast<const Area&>(m_cArea).end())
	{
		if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			m_cBitmapPage.setPhysicalPage(
				m_cFile.attachPhysicalPage(*m_pPageID));
			m_cBitmapPage.verify();
			m_cBitmapPage.detach();
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
//	Bitmap::MiddleBitmapIterator::seek -- 移動する
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
MiddleBitmapIterator::seek(ModSize uiOffset_, bool bUpdate_)
{
	Result::Value r = Result::Success;
	m_bEndOfData = false;
	m_uiOffset = uiOffset_ % m_uiBitmapSize;

	const Area& cArea = m_cArea;

	// ビットマップページの位置
	ModSize pos = uiOffset_ / m_uiBitmapSize;
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
	if (bUpdate_ && *m_pPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ビットマップページがない
		m_cBitmapPage.detach();
		m_cBitmapPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
		m_cBitmapPage.initialize();
		*(m_cArea.begin() + pos) = m_cBitmapPage.getID();
		m_cArea.dirty();
		m_pPageID = cArea.begin() + pos;
	}

	// ビットマップページをattachする
	attachBitmapPage();
	
	return r;
}

//
//	FUNCTION private
//	Bitmap::MiddleBitmapIterator::attachBitmapPage
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
MiddleBitmapIterator::attachBitmapPage()
{
	; _TRMEISTER_ASSERT(isEnd() == false);
	
	m_cBitmapPage.detach();
	if (*m_pPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ビットマップページをattachする
		m_cBitmapPage.setPhysicalPage(
			m_cFile.attachPhysicalPage(*m_pPageID));
		m_pBitset = const_cast<const BitmapPage&>(m_cBitmapPage).begin();
		// ビット位置を移動する
		m_pBitset += m_uiOffset;
	}
	else
	{
		m_pBitset = 0;
	}
	m_bFirstGet = false;
}

//
//	FUNCTION private
//	Bitmap::MiddleBitmapIterator::initialize -- 初期化する
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
MiddleBitmapIterator::initialize(ShortBitmapIterator* i)
{
	// 最大ROWIDを得る
	ModUInt32 uiMaxID = i->getLastRowID();

	// エリアを確保する
	ModSize s = (uiMaxID / (m_uiBitmapSize * sizeof(ModUInt32) * 8)) + 1;
	m_cArea.m_cArea = m_cFile.allocateArea(sizeof(Area::Header) +
										   s * sizeof(PhysicalFile::PageID));
	m_cArea.dirty();
	*(syd_reinterpret_cast<ModUInt32*>(m_cArea.m_cArea.operator void*()))
		= Type::Middle;
	PhysicalFile::PageID* p = m_cArea.begin();
	for (; p != m_cArea.end(); ++p)
		*p = PhysicalFile::ConstValue::UndefinedPageID;

	// ビットをコピーする
	m_uiOffset = 0;
	PhysicalFile::PageID* pPageID = m_cArea.begin();
	while (i->isEnd() == false)
	{
		; _TRMEISTER_ASSERT(pPageID <= m_cArea.end());
		
		ModUInt32 b = i->getNext();
		if (b != 0)
		{
			ModSize c = BitmapPage::getCount(b);
			
			if (*pPageID == PhysicalFile::ConstValue::UndefinedPageID)
			{
				m_cBitmapPage.detach();
				m_cBitmapPage.setPhysicalPage(m_cFile.allocatePhysicalPage());
				m_cBitmapPage.initialize();
				*pPageID = m_cBitmapPage.getID();
				m_cArea.dirty();
			}

			// ビットを設定する
			*(m_cBitmapPage.begin() + m_uiOffset) = b;
			m_cBitmapPage.addCount(c);
			m_cBitmapPage.dirty();
		}

		++m_uiOffset;
		if ((m_uiOffset % m_uiBitmapSize) == 0)
		{
			// 次のビットマップ
			++pPageID;
			m_uiOffset = 0;
		}
	}
}

//
//	FUNCTION private
//	Bitmap::MiddleBitmapIterator::isMiddleRange -- ミドルリストの範囲内か否か
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiMaxRowID_
//		最大ROWID
//
//	RETURN
//	bool
//		ミドルリストの範囲内であればtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleBitmapIterator::isMiddleRange(ModUInt32 uiMaxRowID_)
{
	ModSize s = uiMaxRowID_ / (m_uiBitmapSize * sizeof(ModUInt32) * 8) + 1;
	
	if ((sizeof(Area::Header) + s * sizeof(PhysicalFile::PageID))
		> getMaxStorableAreaSize())
	{
		// 超えているので範囲外
		return false;
	}

	return true;
}

//
//	Copyright (c) 2007, 2009, 2012, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
