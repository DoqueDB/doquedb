// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OverflowPage.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/OverflowPage.h"

#include "FullText2/InvertedList.h"
#include "FullText2/MiddleBaseListIterator.h"
#include "FullText2/OverflowFile.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace 
{
	//
	//	CONST
	//	_$$::_CONTINUE_MASK
	//
	//	NOTES
	//	ロックブロックが次につづくかどうかをあらわすフラグのマスク
	//
	const ModUInt32 _CONTINUE_MASK = 0x80000000;

	//
	//	CONST
	//	_$$::_DATAUNIT_MASK
	//
	//	NOTES
	//	ロックブロックのデータユニット数を得るためのマスク
	//
	const ModUInt32 _DATAUNIT_MASK = 0x7ffc0000;

	//
	//	CONST
	//	_$$::_DATAUNIT_OFFSET
	//
	//	NOTES
	//	ロックブロックのデータユニット数のオフセット
	//
	const int _DATAUNIT_OFFSET = 18;

	//
	//	CONST
	//	_$$::_BITLENGTH_MASK
	//
	//	NOTES
	//	ロックブロックのデータのビット長を得るためのマスク
	//
	const ModUInt32 _BITLENGTH_MASK = 0x0003ffff;

	//
	//	CONST
	//	_$$::_START_LOCBLOCK_OFFSET
	//
	//	NOTES
	//	先頭ロックブロックの先頭位置
	//
	const unsigned short _START_LOCBLOCK_OFFSET
	= sizeof(OverflowPage::LocHeader)/sizeof(ModUInt32) + 1;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::IDBlock::Header::getFirstDocumentID
//		-- 先頭文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		先頭文書ID
//
//	EXCEPTIONS
//
ModUInt32
OverflowPage::IDBlock::Header::getFirstDocumentID() const
{
	return m_uiFirstDocumentID & DocumentIdMask;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::IDBlock::Header::setLocBlockOffset
//		-- LOCブロックのオフセットを設定する
//
//	NOTES
//
//	ARGUMENTS
//	unsigned short usLocBlockOffset_
//		LOCブロックのオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::IDBlock::Header::
setLocBlockOffset(unsigned short usLocBlockOffset_)
{
	m_uiLocBlockOffset &= 0x0000ffff;
	m_uiLocBlockOffset |= static_cast<ModUInt32>(usLocBlockOffset_) << 16;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::IDBlock::Header::isExpunge
//		-- 削除されたか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除された場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OverflowPage::IDBlock::Header::isExpunge() const
{
	return m_uiFirstDocumentID & UndefinedDocumentID;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::IDBlock::Header::setExpunge
//		-- 削除フラグを設定する
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
OverflowPage::IDBlock::Header::setExpunge()
{
	m_uiFirstDocumentID |= UndefinedDocumentID;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::isContinue -- 次のブロックに続くかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		続く場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
OverflowPage::LocBlock::isContinue() const
{
	return *m_pBuffer & _CONTINUE_MASK;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::getDataUnitSize
//		-- データ部のユニット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		データ部のユニット長
//
//	EXCEPTIONS
//	なし
//
ModSize
OverflowPage::LocBlock::getDataUnitSize() const
{
	return (*m_pBuffer & _DATAUNIT_MASK) >> _DATAUNIT_OFFSET;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::getDataBitLength
//		-- データ部のビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		データ部のビット長
//
//	EXCEPTIONS
//	なし
//
ModSize
OverflowPage::LocBlock::getDataBitLength() const
{
	return *m_pBuffer & _BITLENGTH_MASK;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::setDataBitLength
//		-- データ部のビット長を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBitLength_
//		設定するビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::LocBlock::setDataBitLength(ModSize uiBitLength_)
{
	*m_pBuffer &= ~_BITLENGTH_MASK;
	*m_pBuffer |= uiBitLength_;

	// ページをdirtyにする
	m_pPage->dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::expandUnitSize -- ユニット数を増やす
//
//	NOTES
//	指定されたユニット数分のユニット数を増やす
//
//	ARGUMENTS
//	ModSize iUnitSize_
//		増やすユニット数
//
//	RETURN
//	bool
//		増やせた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	
bool
OverflowPage::LocBlock::expandUnitSize(ModSize iUnitSize_)
{
	;_SYDNEY_ASSERT(m_pBuffer);

	// ページの使用ユニット数を変更する
	if (m_pPage->addUsedUnitSize(iUnitSize_) == false)
		return false;

	ModSize iSize = getDataUnitSize();

	ModOsDriver::Memory::reset(getBuffer() + iSize,
							   iUnitSize_ * sizeof(ModUInt32));

	iSize += iUnitSize_;
	*m_pBuffer &= ~_DATAUNIT_MASK;
	*m_pBuffer |= iSize << _DATAUNIT_OFFSET;

	return true;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::expandBitLength
//		-- 使用ビット長を増減させる
//
//	NOTES
//	指定されたビット長分の使用ビット長を増減させる
//
//	ARGUMENTS
//	ModSize iBitLength_
//		増減させるビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::LocBlock::expandBitLength(ModSize iBitLength_)
{
	;_SYDNEY_ASSERT(m_pBuffer);

	ModSize iLength = getDataBitLength();
	iLength += iBitLength_;
	*m_pBuffer &= ~_BITLENGTH_MASK;
	*m_pBuffer |= iLength;

	// ページをdirtyにする
	m_pPage->dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::setContinueFlag
//		-- 次のブロックにつづくブロックに設定する
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
//	なし
//
void
OverflowPage::LocBlock::setContinueFlag()
{
	;_SYDNEY_ASSERT(m_pBuffer);

	*m_pBuffer |= _CONTINUE_MASK;

	// ページをdirtyにする
	m_pPage->dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::LocBlock::unsetContinueFlag
//		-- 次につづくブロックのフラグをクリアする
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
//	なし
//
void
OverflowPage::LocBlock::unsetContinueFlag()
{
	;_SYDNEY_ASSERT(m_pBuffer);

	*m_pBuffer &= ~_CONTINUE_MASK;

	// ページをdirtyにする
	m_pPage->dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::OverflowPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowFile& cFile_
//		オーバーフローファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OverflowPage::OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_uiUsedUnitSize(0),
	  m_pIDHeader(0), m_pLocHeader(0)
{
	// ページ種別を読む
	readType();
	if (m_eType & Type::ID)
	{
		loadIDHeader();
		loadIDData();
	}
	if (m_eType & Type::LOC)
	{
		loadLocHeader();
	}
}

//
//	FUNCTION public
//	FullText2::OverflowPage::OverflowPage -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowFile& cFile_
//		オーバーフローファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OverflowPage::OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
						   ModSize uiBlockSize_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_uiUsedUnitSize(0),
	  m_pIDHeader(0), m_pLocHeader(0)
{
	// ページ種別を設定する
	setType(Type::ID);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::OverflowPage -- コンストラクタ(3)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowFile& cFile_
//		オーバーフローファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OverflowPage::OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
						   PhysicalFile::PageID uiPrevPageID_,
						   PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_uiUsedUnitSize(0),
	  m_pIDHeader(0), m_pLocHeader(0)
{
	// ページ種別を設定する
	setType(Type::LOC);

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::OverflowPage -- コンストラクタ(4)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowFile& cFile_
//		オーバーフローファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OverflowPage::OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
						   ModSize uiBlockSize_,
						   PhysicalFile::PageID uiPrevPageID_,
						   PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_uiUsedUnitSize(0),
	  m_pIDHeader(0), m_pLocHeader(0)
{
	// ページ種別を設定する
	setType(Type::IDLOC);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::~OverflowPage -- デストラクタ
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
//	なし
//
OverflowPage::~OverflowPage()
{
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset(ModSize uiBlockSize_)
{
	// ページ種別を設定する
	setType(Type::ID);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset(ModSize uiBlockSize_,
					PhysicalFile::PageID uiPrevPageID_,
					PhysicalFile::PageID uiNextPageID_)
{
	// ページ種別を設定する
	setType(Type::IDLOC);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset(PhysicalFile::PageID uiPrevPageID_,
					PhysicalFile::PageID uiNextPageID_)
{
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	
	// ページ種別を設定する
	setType(Type::LOC);

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset2 -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset2(PhysicalFile::Page* pPage_)
{
	Page::reset(pPage_);
	m_uiUsedUnitSize = 0;
	m_pIDHeader = 0;
	m_pLocHeader = 0;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	
	// ページ種別を読む
	readType();
	if (m_eType & Type::ID)
	{
		loadIDHeader();
		loadIDData();
	}
	if (m_eType & Type::LOC)
	{
		loadLocHeader();
	}
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset2 -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset2(PhysicalFile::Page* pPage_, ModSize uiBlockSize_)
{
	Page::reset(pPage_);
	m_uiUsedUnitSize = 0;
	m_pIDHeader = 0;
	m_pLocHeader = 0;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	
	// ページ種別を設定する
	setType(Type::ID);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset2 -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//	ModSize uiBlockSize_
//		IDブロックのユニットサイズ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset2(PhysicalFile::Page* pPage_, ModSize uiBlockSize_,
					 PhysicalFile::PageID uiPrevPageID_,
					 PhysicalFile::PageID uiNextPageID_)
{
	Page::reset(pPage_);
	m_uiUsedUnitSize = 0;
	m_pIDHeader = 0;
	m_pLocHeader = 0;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	
	// ページ種別を設定する
	setType(Type::IDLOC);

	loadIDHeader();

	m_pIDHeader->m_uiBlockCount = 0;
	m_pIDHeader->m_uiBlockSize = uiBlockSize_;

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::reset2 -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::reset2(PhysicalFile::Page* pPage_,
					 PhysicalFile::PageID uiPrevPageID_,
					 PhysicalFile::PageID uiNextPageID_)
{
	Page::reset(pPage_);
	m_uiUsedUnitSize = 0;
	m_pIDHeader = 0;
	m_pLocHeader = 0;
	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	
	// ページ種別を設定する
	setType(Type::LOC);

	loadLocHeader();

	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;
	m_pLocHeader->m_usBlockCount = 0;
	m_pLocHeader->m_usOffset = 0;

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::setType -- ページ種別を設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowPage::Type::Value eType_
//		ページ種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::setType(Type::Value eType_)
{
	// メンバーに設定する
	m_eType = eType_;

	ModUInt32* p = Page::getBuffer();
	*p = static_cast<ModUInt32>(eType_);

	if (!(m_eType & Type::LOC))
	{
		m_pLocHeader = 0;
	}
	if (!(m_eType & Type::ID))
	{
		m_pIDHeader = 0;
		m_vecIDBlockHeader.clear();
	}

	m_uiUsedUnitSize = 0;

	// Dirty
	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::allocateIDBlock -- 新しいIDブロックを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OverflowPage::IDBlock
//		新しく確保したIDブロック。確保できなかった場合はinvalidなブロック
//
//	EXCEPTIONS
//
OverflowPage::IDBlock
OverflowPage::allocateIDBlock()
{
	// 容量チェック
	if (m_pIDHeader == 0 || getFreeUnitSize() < m_pIDHeader->m_uiBlockSize)
		return IDBlock();

	// IDブロックを１つ増やす

	m_pIDHeader->m_uiBlockCount++;

	// IDブロックは後ろにしか追加しない

	// ページの最後に移動する
	ModUInt32* p = syd_reinterpret_cast<ModUInt32*>(m_pIDHeader);
	p -= m_pIDHeader->m_uiBlockSize * m_pIDHeader->m_uiBlockCount;
	IDBlock::Header* pHeader = syd_reinterpret_cast<IDBlock::Header*>(p);
	m_vecIDBlockHeader.pushBack(pHeader);

	IDBlock cIDBlock(pHeader, m_pIDHeader->m_uiBlockSize);
	cIDBlock.clear();

	// 使用容量を増やす
	addUsedUnitSize(m_pIDHeader->m_uiBlockSize);

	dirty();

	return cIDBlock;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::freeIDBlock -- 指定した位置のIDブロックを削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::freeIDBlock(ModSize uiPosition_)
{
	// 最後じゃなかったら前の方を1ブロック移動する
	if (uiPosition_ < getIDBlockCount() - 1)
	{
		ModSize size = getIDBlockSize() *
			(getIDBlockCount() - uiPosition_ - 1) * sizeof(ModUInt32);
		void* pSrc = m_vecIDBlockHeader[getIDBlockCount() - 1];
		void* pDst = syd_reinterpret_cast<ModUInt32*>(pSrc) + getIDBlockSize();
		ModOsDriver::Memory::move(pDst, pSrc, size);
	}

	m_uiUsedUnitSize = 0;	// 初期化

	m_pIDHeader->m_uiBlockCount--;
	loadIDData();

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getIDBlock -- 指定した位置のIDブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		指定した位置
//
//	RETURN
//	FullText2::OverflowPage::IDBlock
//		IDブロック
//
//	EXCEPTIONS
//
OverflowPage::IDBlock
OverflowPage::getIDBlock(ModSize uiPosition_)
{
	return IDBlock(m_vecIDBlockHeader[uiPosition_], m_pIDHeader->m_uiBlockSize);
}

//
//	FUNCTION public
//	FullText2::OverflowPage::lowerBoundIDBlock
//		-- IDブロックをlower_boundで検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		指定した文書ID
//	ModSize& uiPosition_
//		ページ内での位置
//	bool bUndo_
//		削除のUndo処理中かどうか
//
//	RETURN
//	FullText2::OverflowPage::IDBlock
//		検索でヒットしたIDブロック
//		すべて削除されている場合はinvalidなIDブロック
//
//	EXCEPTIONS
//
OverflowPage::IDBlock
OverflowPage::lowerBoundIDBlock(ModUInt32 uiDocumentID_,
								ModSize& uiPosition_, bool bUndo_)
{
	// 最小値が格納されているので、upper_boundで検索して1つ前
	IDBlock::Header cHeader;
	cHeader.m_uiFirstDocumentID = uiDocumentID_;
	ModVector<IDBlock::Header*>::Iterator i
		= ModUpperBound(m_vecIDBlockHeader.begin(), m_vecIDBlockHeader.end(),
								&cHeader, IDBlock::Less());
	if (i != m_vecIDBlockHeader.begin())
		--i;
	
	for (; i != m_vecIDBlockHeader.end(); ++i)
	{
		if (bUndo_ == true || (*i)->isExpunge() == false)
		{
			uiPosition_ = i - m_vecIDBlockHeader.begin();
			return IDBlock(*i, m_pIDHeader->m_uiBlockSize);
		}
	}
	return IDBlock();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::moveIDBlock
//		-- 他のページのIDブロックをすべて自分に移動する
//		   このページの空き容量は十分にあることが想定されている
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowPage::PagePointer pIdPage_
//		供給元のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::moveIDBlock(PagePointer pIdPage_)
{
	ModSize size = pIdPage_->getIDBlockCount();
	for (ModSize i = 0; i < size; ++i)
	{
		IDBlock dst = allocateIDBlock();
		dst.copy(pIdPage_->getIDBlock(i));
	}
	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getPrevDocumentID -- 直前のIDブロックの文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		IDブロックの位置
//
//	RETURN
//	ModUInt32
//		直前のIDブロックの文書ID
//		直前のブロックが存在しない場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
OverflowPage::getPrevDocumentID(ModSize uiPosition_)
{
	ModUInt32 uiPrevID = UndefinedDocumentID;
	if (uiPosition_ > 0 && uiPosition_ <= getIDBlockCount())
	{
		uiPrevID = m_vecIDBlockHeader[--uiPosition_]->getFirstDocumentID();
	}
	return uiPrevID;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getNextDocumentID -- 直後のIDブロックの文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		IDブロックの位置
//
//	RETURN
//	ModUInt32
//		直後のIDブロックの文書ID
//		直後のブロックが存在しない場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
OverflowPage::getNextDocumentID(ModSize uiPosition_)
{
	ModUInt32 uiNextID = UndefinedDocumentID;
	if (uiPosition_ < getIDBlockCount() - 1)
	{
		uiNextID = m_vecIDBlockHeader[++uiPosition_]->getFirstDocumentID();
	}
	return uiNextID;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::setPrevPageID -- 前方のページIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		設定する前方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::setPrevPageID(PhysicalFile::PageID uiPrevPageID_)
{
	m_pLocHeader->m_uiPrevPageID = uiPrevPageID_;	// 前のページID

	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::setNextPageID -- 後方のページIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiNextPageID_
//		設定する後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OverflowPage::setNextPageID(PhysicalFile::PageID uiNextPageID_)
{
	m_pLocHeader->m_uiNextPageID = uiNextPageID_;

	// ページをdirtyにする
	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::allocateLocBlock -- 新しくロックブロックを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OverflowPage::LocBlock
//		新しく確保されたロックブロック
//
//	EXCEPTIONS
//	
OverflowPage::LocBlock
OverflowPage::allocateLocBlock()
{
	ModUInt32* p = getBuffer();

	if (getFreeUnitSize() < 4)
	{
		// 空きが4ユニット未満なので、新しいLOCブロックは作成できない
		return LocBlock();
	}

	if (m_pLocHeader->m_usOffset == 0)
	{
		// このページに初めてLOCブロックが確保される
		m_pLocHeader->m_usOffset = _START_LOCBLOCK_OFFSET;
	}
	else
	{
		// 最後のブロックを得る
		LocBlock cLast(this, m_pLocHeader->m_usOffset, p);

		// ユニット数を得る
		ModSize uiUnitSize = cLast.getUnitSize();

		// ヘッダーを更新する
		m_pLocHeader->m_usOffset += static_cast<unsigned short>(uiUnitSize);
	}

	// 新しいLOCブロックのヘッダー部分をクリアする
	p += m_pLocHeader->m_usOffset;
	*p = 0;

	// ロックブロック数を増やす
	m_pLocHeader->m_usBlockCount++;

	addUsedUnitSize(1);

	dirty();

	return LocBlock(this, m_pLocHeader->m_usOffset, getBuffer());
}

//
//	FUNCTION public
//	FullText2::OverflowPage::freeLocBlock -- LOCブロックを1つ開放する
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
OverflowPage::freeLocBlock()
{
	m_pLocHeader->m_usBlockCount--;
	dirty();
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getLocBlock -- LOCブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	unsigned short usOffset_
//		ページ内のオフセット
//
//	RETURN
//	FullText2::OverflowPage::LocBlock
//		指定されたオフセットのLOCブロック
//
//	EXCEPTIONS
//
OverflowPage::LocBlock
OverflowPage::getLocBlock(unsigned short usOffset_)
{
	if (usOffset_ == 0)
		return getLocBlock();
	return LocBlock(this, usOffset_, getBuffer());
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getLocBlock -- 先頭のLOCブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OverflowPage::LocBlock
//		先頭のLOCブロック
//
//	EXCEPTIONS
//
OverflowPage::LocBlock
OverflowPage::getLocBlock()
{
	return LocBlock(this, _START_LOCBLOCK_OFFSET, getBuffer());
}

//
//	FUNCTION public
//	FullText2::OverflowPage::addUsedUnitSize -- 使用ユニット数を増加させる
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiUnitSize_
//		増加するユニット数
//
//	RETURN
//	bool
//		増加できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OverflowPage::addUsedUnitSize(ModSize uiUnitSize_)
{
	if (getFreeUnitSize() < uiUnitSize_)
		return false;
	m_uiUsedUnitSize += uiUnitSize_;
	dirty();
	return true;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::getUsedUnitSize -- 使用ユニット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		使用ユニット数
//
//	EXCEPTIONS
//
ModSize
OverflowPage::getUsedUnitSize() const
{
	if (m_uiUsedUnitSize != 0) return m_uiUsedUnitSize;

	if (m_eType & Type::LOC)
	{
		if (m_pLocHeader->m_usOffset == 0)
		{
			// まだこのページにはLOCブロックが存在していない
			m_uiUsedUnitSize = _START_LOCBLOCK_OFFSET;
		}
		else
		{
			ModUInt32* p = getBuffer();

			// 最後のブロックを得る
			LocBlock cLast(const_cast<OverflowPage*>(this),
						   m_pLocHeader->m_usOffset, p);

			// ユニット数を得る
			ModSize uiUnitSize = cLast.getUnitSize();

			// 使用しているユニット数は、
			// 最後のブロックのオフセット値+ユニット数である
			m_uiUsedUnitSize = uiUnitSize + m_pLocHeader->m_usOffset;
		}
	}
	else
	{
		m_uiUsedUnitSize = 1;	// ページ種別分
	}

	if (m_eType & Type::ID)
	{
		m_uiUsedUnitSize += sizeof(IDHeader)/sizeof(ModUInt32)
				+ m_pIDHeader->m_uiBlockSize * m_pIDHeader->m_uiBlockCount;
	}

	return m_uiUsedUnitSize;
}

//
//	FUNCTION public
//	FullText2::OverflowPage::enterDeleteIdBlock
//		-- 削除ずみのIDブロックを登録する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MiddleBaseListIterator& cListIterator_
//		ミドルリスト
//
//	RETURN
//	bool
//		すべてのIDブロックが削除ずみならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OverflowPage::enterDeleteIdBlock(MiddleBaseListIterator& cListIterator_)
{
	bool result = true;
	ModVector<IDBlock::Header*>::Iterator i = m_vecIDBlockHeader.begin();
	for (; i != m_vecIDBlockHeader.end(); ++i)
	{
		if ((*i)->isExpunge())
			cListIterator_.enterDeleteIdBlock((*i)->getFirstDocumentID());
		else
			result = false;
	}
	return result;
}

//
//	FUNCTION private
//	FullText2::OverflowPage::readType -- ページ種別を読む
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
OverflowPage::readType()
{
	ModUInt32* p = Page::getBuffer();
	m_eType = static_cast<Type::Value>(*p);
}

//
//	FUNCTION private
//	FullText2::OverflowPage::loadIDHeader -- ID情報を読み込む
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
OverflowPage::loadIDHeader()
{
	// ページの最後に移動する
	ModUInt32* p = getBuffer() + getPageUnitSize()
		- sizeof(IDHeader) / sizeof(ModUInt32);
	// ヘッダーを設定する
	m_pIDHeader = syd_reinterpret_cast<IDHeader*>(p);
}

//
//	FUNCTION public
//	FullText2::OverflowPage::loadIDData -- ID情報を読み込む
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
OverflowPage::loadIDData()
{
	if (m_pIDHeader->m_uiBlockCount == 0) return;

	// IDブロックを読み込む

	m_vecIDBlockHeader.erase(m_vecIDBlockHeader.begin(),
							 m_vecIDBlockHeader.end());
	ModUInt32* pBuffer = syd_reinterpret_cast<ModUInt32*>(m_pIDHeader);
	if (m_vecIDBlockHeader.getCapacity() < m_pIDHeader->m_uiBlockCount)
		m_vecIDBlockHeader.reserve(m_pIDHeader->m_uiBlockCount);

	for (int i = 0; i < static_cast<int>(m_pIDHeader->m_uiBlockCount); ++i)
	{
		pBuffer -= m_pIDHeader->m_uiBlockSize;
		m_vecIDBlockHeader.pushBack(
			syd_reinterpret_cast<IDBlock::Header*>(pBuffer));
	}
}

//
//	FUNCTION private
//	FullText2::OverflowPage::loadLocHeader -- LOCヘッダーを設定する
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
OverflowPage::loadLocHeader()
{
	m_pLocHeader = syd_reinterpret_cast<LocHeader*>(getBuffer() + 1);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
