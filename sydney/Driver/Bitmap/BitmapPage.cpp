// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapPage.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2016, 2017, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/BitmapPage.h"
#include "Bitmap/BitmapFile.h"
#ifdef DEBUG
#include "Bitmap/Parameter.h"
#endif
#include "Bitmap/MessageAll_Class.h"

#include "Exception/VerifyAborted.h"

#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace {

	//
	//	CONST
	//	_$$::_BitCount --
	//		1 バイト中の1が立っているビットの数を引くテーブル
	//
	//	NOTES
	//
	const unsigned char _BitCount[] =
	{
		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};

	//
	//	FUNCTION local
	//	_$$::_getCount -- 何個ビットが立っているかを得る
	//
	ModSize _getCount(ModUInt32 b_)
	{
		ModSize c = 0;
		
		const unsigned char* p
			= syd_reinterpret_cast<const unsigned char*>(&b_);
		for (int i = 0; i < static_cast<int>(sizeof(ModUInt32)); ++i, ++p)
			c += static_cast<ModSize>(_BitCount[*p]);
		
		return c;
	}

#ifdef DEBUG
	//	ページサイズ -- デバッグ用に利用する
	ParameterInteger _cDebugPageSize("Bitmap_DebugPageSize", 0);
#endif

}

//
//	FUNCTION public
//	Bitmap::BitmapPage::BitmapPage -- コンストラクタ
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
BitmapPage::BitmapPage(BitmapFile& cFile_)
	: Page(cFile_), m_cFile(cFile_), m_pHeader(0), m_pValue(0)
{
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::~BitmapPage -- デストラクタ
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
BitmapPage::~BitmapPage()
{
	detach();
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::setPhysicalPage -- 物理ページを設定する
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
BitmapPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	ModUInt32* p = getBuffer();
	m_pHeader = syd_reinterpret_cast<Header*>(p);
	m_pValue = p + sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::initialize -- 初期化する
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
BitmapPage::initialize()
{
	dirty();
	m_pHeader->m_uiCount = 0;
	Os::Memory::reset(m_pValue, getPageSize() * sizeof(ModUInt32));
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	ModSize position_
//		このページでのビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitmapPage::on(ModSize position_)
{
	ModUInt32* p = m_pValue + position_ / (sizeof(ModUInt32) * 8);
	ModUInt32 b = (1 << position_ % (sizeof(ModUInt32) * 8));
	
	if ((*p & b) == 0)
	{
		dirty();
		*p |= b;
		m_pHeader->m_uiCount++;
	}
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	ModSize position_
//		このページでのビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitmapPage::off(ModSize position_)
{
	ModUInt32* p = m_pValue + position_ / (sizeof(ModUInt32) * 8);
	ModUInt32 b = (1 << position_ % (sizeof(ModUInt32) * 8));
	
	if ((*p & b) != 0)
	{
		dirty();
		*p &= ~b;
		m_pHeader->m_uiCount--;
	}
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::set -- ビットを設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		このページでのModUInt32単位のオフセット
//	ModUInt32 b_
//		ビットマップ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitmapPage::set(ModSize offset_, ModUInt32 b_)
{
	// 既存のビットマップを上書きする
	
	dirty();

	ModUInt32* p = m_pValue + offset_;
	m_pHeader->m_uiCount -= _getCount(*p);

	*p = b_;
	m_pHeader->m_uiCount += _getCount(b_);
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::detach -- ページをdetachする
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
BitmapPage::detach()
{
	Page::detach();
	m_pHeader = 0;
	m_pValue = 0;
}

//
//	FUNCTION public
//	Bitmap::BitmapPage::verify -- 整合性検査
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
BitmapPage::verify()
{
	//
	//	BitmapPageの整合性検査はヘッダーのカウントと実際に立っている
	//	ビットの数とを比較することにより行う。
	//
	
	int size = getPageSize() * sizeof(ModUInt32);
	const unsigned char* p
		= syd_reinterpret_cast<const unsigned char*>(m_pValue);

	ModSize count = 0;

	for (int i = 0; i < size; ++i, ++p)
		count += static_cast<ModSize>(_BitCount[*p]);

	if (count != m_pHeader->m_uiCount)
	{
		// 数が違っている
		_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
									m_cFile.getPath(),
									Message::DiscordBitNum(getID(),
														   m_pHeader->m_uiCount,
														   count));
		if (!m_cFile.isContinue())
			_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//	FUNCTION public static
//	Bitmap::BitmapPage::getBitCount -- 1ページに格納できるビット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	Os::Memory::Size uiPageSize_
//		ページサイズ
//
//	RETURN
//	ModSize
//		ビット数
//
//	EXCEPTIONS
//
ModSize
BitmapPage::getBitCount(Os::Memory::Size uiPageSize_)
{
#ifdef DEBUG
	if (_cDebugPageSize.get() != 0)
		return (_cDebugPageSize.get() - sizeof(Header))
			/ sizeof(ModUInt32) * sizeof(ModUInt32) * 8;
#endif
	return (PhysicalFile::File::getPageDataSize(
				PhysicalFile::PageManageType, uiPageSize_) - sizeof(Header))
		/ sizeof(ModUInt32) * sizeof(ModUInt32) * 8;
}

//
//	FUNCTION public static
//	Bitmap::BitmapPage::getCount -- ビット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiBitset
//		ビットセット
//
//	RETURN
//	ModSize
//		ビット数
//
//	EXCEPTIONS
//
ModSize
BitmapPage::getCount(ModUInt32 uiBitSet_)
{
	const unsigned char* p
		= syd_reinterpret_cast<const unsigned char*>(&uiBitSet_);
	ModSize c = 0;
	for (int i = 0; i < sizeof(ModUInt32); ++i, ++p)
		c += static_cast<ModSize>(_BitCount[*p]);
	return c;
}

//
//	Copyright (c) 2005, 2006, 2007, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
