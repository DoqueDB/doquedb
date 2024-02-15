// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBitmapIterator.cpp -- 
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
#include "Bitmap/ShortBitmapIterator.h"
#include "Bitmap/CompressedBitmapFile.h"
#include "Bitmap/LongBitmapIterator.h"
#include "Bitmap/MiddleBitmapIterator.h"
#include "Bitmap/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::Area::getSize -- サイズを得る
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
ShortBitmapIterator::Area::getSize() const
{
	return static_cast<ModSize>(m_cArea.getSize());
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::Area::getLastRowID -- 最終ROWIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		最終ROWID
//
//	EXCEPTIONS
//
ModUInt32
ShortBitmapIterator::Area::getLastRowID() const
{
	const Header* h
		= syd_reinterpret_cast<const Header*>(m_cArea.operator const void*());
	return h->m_uiLastRowID;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::Area::setLastRowID -- 最終ROWIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastRowID_
//		最終ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBitmapIterator::Area::setLastRowID(ModUInt32 uiLastRowID_)
{
	m_cArea.dirty();
	Header* h = syd_reinterpret_cast<Header*>(m_cArea.operator void*());
	h->m_uiLastRowID = uiLastRowID_;
}

//	FUNCTION public
//	Bitmap::ShortBitmapIterator::Area::begin -- 先頭のIDBlockを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	IDBlock*
//	const IDBlock*
//		先頭のIDBlock
//
//	EXCEPTIONS
//
IDBlock*
ShortBitmapIterator::Area::begin()
{
	return syd_reinterpret_cast<IDBlock*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*())
		+ sizeof(Header));
}
const IDBlock*
ShortBitmapIterator::Area::begin() const
{
	return syd_reinterpret_cast<const IDBlock*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ sizeof(Header));
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::Area::end -- 終端のIDBlockを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	IDBlock*
//	const IDBlock*
//		終端のIDBlock
//
//	EXCEPTIONS
//
IDBlock*
ShortBitmapIterator::Area::end()
{
	return syd_reinterpret_cast<IDBlock*>(
		syd_reinterpret_cast<char*>(m_cArea.operator void*()) + getSize());
}
const IDBlock*
ShortBitmapIterator::Area::end() const
{
	return syd_reinterpret_cast<const IDBlock*>(
		syd_reinterpret_cast<const char*>(m_cArea.operator const void*())
		+ getSize());
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::ShortBitmapIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::CompressedBitmapFile& cFile_
//		圧縮ビットマップファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortBitmapIterator::ShortBitmapIterator(CompressedBitmapFile& cFile_)
	: CompressedBitmapIterator(cFile_),
	  m_uiBitmap(0), m_uiCurrentMax(0),
	  m_pIDBlock(0), m_uiLastRowID(0), m_uiOffset(0)
{
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::~ShortBitmapIterator -- デストラクタ
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
ShortBitmapIterator::~ShortBitmapIterator()
{
	m_cArea.m_cArea.detach();
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::getNext
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
ShortBitmapIterator::getNext()
{
	if (m_pIDBlock == 0)
	{
		// 初めて
		next();
	}
	
	ModUInt32 bitset = m_uiBitmap;
	next();	// 次へ
	
	return bitset;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::seek -- 移動する(ModUInt32単位)
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
ShortBitmapIterator::seek(ModSize offset_)
{
	const Area& cArea = m_cArea;
	
	// ModUInt32単位なので、32ビット単位のオフセットとなる
	// よって、offset_に32をかけると必要なROWIDとなる
	m_uiCurrentMax = offset_ * sizeof(ModUInt32) * 8;

	// 最終ROWIDと比較する
	if (m_uiCurrentMax > cArea.getLastRowID())
	{
		// 最終ROWIDよりも大きいのでこのAREAには存在しない
		m_uiBitmap = 0;
		m_pEndIDBlock = m_pIDBlock = cArea.end();
		
		return;
	}

	// upper_boundの1つ前を検索
	m_pEndIDBlock = cArea.end();
	m_pIDBlock = ModUpperBound(cArea.begin(), m_pEndIDBlock,
							   IDBlock(m_uiCurrentMax), IDBlock::Less());
	if (m_pIDBlock != cArea.begin())
		--m_pIDBlock;

	// 必要なROWIDの位置まで読み進める
	m_uiLastRowID = m_pIDBlock->getFirstID();
	m_uiOffset = 0;
	while (m_uiLastRowID < m_uiCurrentMax)
	{
		ModUInt32 prev = m_uiLastRowID;
		m_uiLastRowID = m_pIDBlock->getID(prev, m_uiOffset);
		if (m_uiLastRowID == prev)
		{
			// このブロックは終わり、次へ
			++m_pIDBlock;
			if (m_pIDBlock == m_pEndIDBlock) return;
			m_uiLastRowID = m_pIDBlock->getFirstID();
			m_uiOffset = 0;
		}
	}

	// 次へ進むことにより、ビットが設定される
	next();
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::initialize -- 初期化する
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
ShortBitmapIterator::initialize(ModUInt32 uiRowID_)
{
	// エリアを確保する
	m_cArea.m_cArea
		= m_cFile.allocateArea(sizeof(Area::Header) + sizeof(IDBlock));
	*(syd_reinterpret_cast<ModUInt32*>(m_cArea.m_cArea.operator void*()))
		= Type::Short;
	m_cArea.setLastRowID(uiRowID_);
	m_cArea.dirty();
	
	// IDブロックを初期化する
	IDBlock* pBlock = m_cArea.begin();
	pBlock->initialize(uiRowID_);
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::setArea -- エリアを設定する
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
ShortBitmapIterator::setArea(const PhysicalFile::DirectArea& cArea_)
{
	m_cArea.m_cArea = cArea_;
	m_uiBitmap = 0;
	m_uiCurrentMax = 0;
	m_pIDBlock = 0;
	m_uiLastRowID = 0;
	m_uiOffset = 0;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::on -- ビットをONする
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
ShortBitmapIterator::on(ModUInt32 uiRowID_)
{
	if (uiRowID_ > m_cArea.getLastRowID())
	{
		// 最後なので、appendする
		return append(uiRowID_);
	}

	Result::Value r = Result::Success;
	
	// upper_boundの1つ前を検索
	IDBlock* pIDBlock = ModUpperBound(m_cArea.begin(), m_cArea.end(),
									  IDBlock(uiRowID_), IDBlock::Less());
	if (pIDBlock != m_cArea.begin())
		--pIDBlock;

	// 挿入してみる
	while (pIDBlock->insert(uiRowID_) == false)
	{
		// このIDBlockでは領域が不足している -> splitする
		if ((pIDBlock = split(pIDBlock, uiRowID_)) == 0)
		{
			// ミドルリストへの変換が必要
			return Result::NeedConvert;
		}

		// Areaが変更された
		r = Result::Modify;
	}
	
	m_cArea.dirty();
	
	return r;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::off -- ビットをOFFする
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
ShortBitmapIterator::off(ModUInt32 uiRowID_)
{
	Result::Value r = Result::Success;
	
	// upper_boundの1つ前を検索
	IDBlock* pIDBlock = ModUpperBound(m_cArea.begin(), m_cArea.end(),
									  IDBlock(uiRowID_), IDBlock::Less());
	if (pIDBlock != m_cArea.begin())
		--pIDBlock;

	// 削除する
	ModUInt32 uiPrevID = uiRowID_;
	pIDBlock->erase(uiRowID_, uiPrevID);
	m_cArea.dirty();

	// サイズをチェックする
	if (pIDBlock != (m_cArea.end() - 1) &&
		pIDBlock->getBitLength() < (IDBlock::getMaxBitLength() / 3) &&
		(pIDBlock + 1)->getBitLength() < (IDBlock::getMaxBitLength() / 3))
	{
		// ブロックが最後のブロックではなく、
		// 自身のビット長がキャパシティーの1/3未満であり、
		// となりのビット長がキャパシティーの1/3未満である

		// マージを試みる
		if (pIDBlock->merge(pIDBlock + 1) == true)
		{
			// マージできたので、エリアを縮める
			int n = static_cast<int>(pIDBlock - m_cArea.begin());
			reduce(pIDBlock + 1);	// ここでエリアが置き換わる
			pIDBlock = m_cArea.begin() + n;

			r = Result::Modify;
		}
	}

	// カウントをチェックする
	if (pIDBlock->getCount() == 0)
	{
		if ((m_cArea.end() - m_cArea.begin()) == 1)
		{
			// これしかIDBlockがない -> エリア自体を消す
			m_cArea.m_cArea.expunge(getTransaction());

			return Result::Deleted;
		}
		else
		{
			// このIDBlockのカウントが0になった -> これを消す
			reduce(pIDBlock);

			r = Result::Modify;
		}
	}

	// 最終ROWIDをチェックする
	if (m_cArea.getLastRowID() == uiRowID_)
	{
		// 最終ROWIDが変更された

		if (uiPrevID != uiRowID_)
		{
			// 最終RowIDを設定する
			m_cArea.setLastRowID(uiPrevID);
		}
		else
		{
			// 最終IDBlockの先頭ROWIDが削除された
			IDBlock* p = m_cArea.begin();
			for (; p != (m_cArea.end() - 1); ++p);

			// 現在の最終IDBlockの最後を読み出す
			uiPrevID = p->getFirstID();
			ModUInt32 uiNextID = uiPrevID;
			ModSize offset = 0;
			while ((uiNextID = p->getID(uiPrevID, offset)) != uiPrevID)
			{
				uiPrevID = uiNextID;
			}

			// 最終ROWIDを設定する
			m_cArea.setLastRowID(uiPrevID);
		}
	}

	return r;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::convert -- ミドルリストへ変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		登録するROWID
//
//	RETURN
//	Bitmap::CompressedBitmapIterator*
///		ミドルリストまたは、ロングリスト
//
//	EXCEPTIONS
//
CompressedBitmapIterator*
ShortBitmapIterator::convert(ModUInt32 uiRowID_)
{
	ModAutoPointer<MiddleBitmapIterator> i
		= new MiddleBitmapIterator(m_cFile);
	ModAutoPointer<LongBitmapIterator> j;

	CompressedBitmapIterator* ret = 0;

	// 最大ROWID
	ModUInt32 uiMaxRowID
		= (uiRowID_ > getLastRowID()) ? uiRowID_ : getLastRowID();
	
	// ミドルリストの範囲内か？
	if (i->isMiddleRange(uiMaxRowID))
	{
		// ミドルリスト
		
		i->initialize(this);
		// 新しいROWIDを挿入する
		i->on(uiRowID_);

		ret = i.release();
	}
	else
	{
		// いきなりロングリスト

		j = new LongBitmapIterator(m_cFile);
		j->initializeArea(uiMaxRowID);
		j->insert(this);
		// 新しいROWIDを挿入する
		j->on(uiRowID_);

		ret = j.release();
	}
			
	// 自身の不要なものを削除する
	m_cArea.m_cArea.expunge(getTransaction());
	
	return ret;
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::split -- 分割する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::CompressedBitmapIterator*
//		分割した後ろ半分のショートリスト
//
//	EXCEPTIONS
//
CompressedBitmapIterator*
ShortBitmapIterator::split()
{
	// 半分にする

	const Area& cArea = m_cArea;
	
	const IDBlock* b = cArea.begin();
	const IDBlock* e = cArea.end();
	const IDBlock* h = b + ((e - b) / 2);	// 中間位置

	// エリアを確保する
	PhysicalFile::DirectArea area1 = m_cFile.allocateArea(
		static_cast<ModSize>((h - b) * sizeof(IDBlock) + sizeof(Area::Header)));
	PhysicalFile::DirectArea area2 = m_cFile.allocateArea(
		static_cast<ModSize>((e - h) * sizeof(IDBlock) + sizeof(Area::Header)));

	// エリアを確保すると、このエリアのポインタが移動するので、
	// ポインタを設定し直す

	b = cArea.begin();
	e = cArea.end();
	h = b + ((e - b) / 2);	// 中間位置
	
	// 前半部分を自身の新しいエリアにコピーする
	Os::Memory::copy(area1.operator void*(),
					 m_cArea.m_cArea.operator const void*(),
					 static_cast<ModSize>(
						 (h - b) * sizeof(IDBlock) + sizeof(Area::Header)));
	ModUInt32 last1 = (h - 1)->getLastID();

	// 後半部分を新しいイテレータのエリアにコピーする
	*(syd_reinterpret_cast<ModUInt32*>(area2.operator void*())) = Type::Short;
	ModAutoPointer<ShortBitmapIterator> i = new ShortBitmapIterator(m_cFile);
	(*i).setArea(area2);
	(*i).m_cArea.setLastRowID((e - 1)->getLastID());
	Os::Memory::copy((*i).m_cArea.begin(), h,
					 static_cast<ModSize>((e - h) * sizeof(IDBlock)));

	// 自身のエリアを解放し、自身のメンバー変数を設定し直す
	m_cArea.m_cArea.expunge(getTransaction());
	m_cArea.m_cArea = area1;
	m_cArea.setLastRowID(last1);

	return i.release();
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::verify -- 整合性検査
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
ShortBitmapIterator::verify()
{
	// 読み出してみる
	ModSize uiOffset = 0;
	ModUInt32 bitset;
	while (isEnd() == false)
	{
		bitset = getNext();
		uiOffset++;
	}

	// 最終ROWIDをチェックする
	int last = -1;
	for (int i = 0; i < sizeof(ModUInt32)*8; ++i)
	{
		if (bitset & (1 << i))
		{
			last = i;
		}
	}
	
	ModUInt32 l = sizeof(ModUInt32) * 8 * (uiOffset - 1) + last;
	if (m_cArea.getLastRowID() != l)
	{
		_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
									m_cFile.getPath(),
									Message::IllegalLastRowID(
										l, m_cArea.getLastRowID()));
	}
}

//
//	FUNCTION public
//	Bitmap::ShortBitmapIterator::getFirstRowID -- 先頭のROWIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		このイテレータの先頭のROWID
//
//	EXCEPTIONS
//
ModUInt32
ShortBitmapIterator::getFirstRowID()
{
	const Area& cArea = m_cArea;
	return cArea.begin()->getFirstID();
}

//
//	FUNCTION private
//	Bitmap::ShortBitmapIterator::next -- 次へ
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
ShortBitmapIterator::next()
{
	const Area& cArea = m_cArea;
	
	ModUInt32 currentMin = m_uiCurrentMax;
	m_uiCurrentMax += sizeof(ModUInt32) * 8;
	m_uiBitmap = 0;
	
	if (m_pIDBlock == 0)
	{
		m_pIDBlock = cArea.begin();
		m_pEndIDBlock = cArea.end();
		if (m_pIDBlock == m_pEndIDBlock) return;
		m_uiLastRowID = m_pIDBlock->getFirstID();
		m_uiOffset = 0;
	}

	if (m_pIDBlock == m_pEndIDBlock) return;
	
	while (m_uiLastRowID < m_uiCurrentMax)
	{
		// ビットを立てる
		m_uiBitmap |= (1 << (m_uiLastRowID - currentMin));

		// 次の値を得る
		ModSize prev = m_uiLastRowID;
		m_uiLastRowID = m_pIDBlock->getID(prev, m_uiOffset);
		if (m_uiLastRowID == prev)
		{
			// このブロックは終わり、次へ
			++m_pIDBlock;
			if (m_pIDBlock == m_pEndIDBlock) break;
			m_uiLastRowID = m_pIDBlock->getFirstID();
			m_uiOffset = 0;
		}
	}
}

//
//	FUNCTION private
//	Bitmap::ShortBitmapIterator::append -- 最後のIDBlockにappendする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		追加するROWID
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
ShortBitmapIterator::append(ModUInt32 uiRowID_)
{
	; _TRMEISTER_ASSERT(uiRowID_ > m_cArea.getLastRowID());

	Result::Value r = Result::Success;

	IDBlock* pIDBlock = m_cArea.end();
	--pIDBlock;

	if (pIDBlock->append(m_cArea.getLastRowID(), uiRowID_) == false)
	{
		// このブロックには入りきらない -> エリアを拡張する
		if (expand() == false)
		{
			// ミドルリストへの変換が必要
			return Result::NeedConvert;
		}

		// データを設定する
		pIDBlock = m_cArea.end();
		--pIDBlock;
		pIDBlock->initialize(uiRowID_);

		r = Result::Modify;
	}

	// 最終ROWIDを更新する
	m_cArea.setLastRowID(uiRowID_);
	m_cArea.dirty();
	
	return r;
}

//
//	FUNCTION private
//	Bitmap::ShortBitmapIterator::split --
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::IDBlock* pIDBlock_
//		分割対象のブロック
//	ModUInt32 uiRowID_
//		挿入するROWID
//
//	RETURN
//	Bitmap::IDBlock*
//		挿入対象のブロック
//
//	EXCEPTIONS
//
IDBlock*
ShortBitmapIterator::split(IDBlock* pIDBlock_, ModUInt32 uiRowID_)
{
	// 挿入対象のブロックの位置とこのブロックより後ろのサイズを記憶する
	int n = static_cast<int>(pIDBlock_ - m_cArea.begin());
										// このブロックの位置
	Os::Memory::Size s = static_cast<Os::Memory::Size>(
		m_cArea.end() - (pIDBlock_ + 1)) * sizeof(IDBlock);
										// このブロック以降のサイズ
	
	// エリアを拡張する
	if (expand() == false)
	{
		// ミドルリストへの変換が必要
		return 0;
	}

	IDBlock* b = m_cArea.begin();
	
	if (s != 0)
	{
		// splitするための領域を確保する
		Os::Memory::move(b + n + 2,	b + n + 1, s);
	}

	// ブロックの内容を分ける
	pIDBlock_ = b + n;
	pIDBlock_->split(pIDBlock_ + 1);

	if ((pIDBlock_ + 1)->getFirstID() < uiRowID_)
	{
		++pIDBlock_;
	}

	return pIDBlock_;
}

//
//	FUNCTION private
//	Bitmap::ShortBitmapIterator::expand -- エリアを拡張する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		エリアの最大サイズを超えた場合はfalse、成功した場合はtrue
//
//	EXCEPTIONS
//
bool
ShortBitmapIterator::expand()
{
	ModSize newSize = m_cArea.getSize() + sizeof(IDBlock);
	
	// 最大のエリアサイズを越えていないかチェック
	if (newSize > getMaxStorableAreaSize())
		return false;

	// 新しいAreaを確保する
	PhysicalFile::DirectArea newArea = m_cFile.allocateArea(newSize);

	// リセットする
	Os::Memory::reset(newArea.operator void*(),
					  newArea.getSize());

	// 内容をコピーする
	Os::Memory::copy(newArea.operator void*(),
					 m_cArea.m_cArea.operator const void*(),
					 m_cArea.getSize());

	// 古いエリアを開放する
	m_cArea.m_cArea.expunge(getTransaction());

	// 新しいエリアを設定する
	m_cArea.m_cArea = newArea;

	return true;
}

//
//	FUNCTION private
//	Bitmap::ShortBitmapIterator::reduce -- エリアを縮める
//
//	NOTES
//
//	ARGUMENTS
//	IDBlock* pPosition_
//		削除するIDBlock
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBitmapIterator::reduce(IDBlock* pPosition_)
{
	Os::Memory::Size s = static_cast<Os::Memory::Size>(
		m_cArea.end() - (pPosition_ + 1));
	if (s)
	{
		Os::Memory::move(pPosition_, pPosition_ + 1, s * sizeof(IDBlock));
	}

	// 新しいサイズ
	ModSize newSize = m_cArea.getSize() - sizeof(IDBlock);

	// 新しいエリアを得る
	PhysicalFile::DirectArea newArea = m_cFile.allocateArea(newSize);

	// 新しいエリアに移動する
	Os::Memory::copy(newArea.operator void*(),
					 m_cArea.m_cArea.operator const void*(),
					 newSize);

	// 古いエリアを開放する
	m_cArea.m_cArea.expunge(getTransaction());

	// 新しいエリアを設定する
	m_cArea.m_cArea = newArea;
}

//
//	Copyright (c) 2007, 2009, 2012, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
