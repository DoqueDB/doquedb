// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SmartLocationList.cpp
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
#include "FullText2/SmartLocationList.h"
#include "FullText2/SmartLocationListIterator.h"
#include "Os/Memory.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::SmartLocationList -- コンストラクタ
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
SmartLocationList::SmartLocationList()
	: LocationList(),
	  m_pLocationCoder(0),
	  m_pBuffer(0), m_uiBufferSize(0)
{
	clear();
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::SmartLocationList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedCoder* pLocationCoder_
//		圧縮器
//	ModSize length_
//		トークン長
//	bool bNoLocation_
//		位置情報を格納しない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SmartLocationList::SmartLocationList(ModInvertedCoder* pLocationCoder_,
									 ModSize length_,
									 bool bNoLocation_)
	: LocationList(length_, bNoLocation_),
	  m_pLocationCoder(pLocationCoder_),
	  m_pBuffer(0), m_uiBufferSize(0)
{
	clear();
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::SmartLocationList -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const SmartLocationList& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SmartLocationList::SmartLocationList(const SmartLocationList& src_)
	: LocationList(src_),
	  m_pLocationCoder(src_.m_pLocationCoder),
	  m_uiLastLocation(src_.m_uiLastLocation),
	  m_uiBitOffset(src_.m_uiBitOffset),
	  m_pBuffer(0),
	  m_uiBufferSize(src_.m_uiBufferSize)
{
	if (m_uiBufferSize)
	{
		m_pBuffer = new ModUInt32[m_uiBufferSize];
		Os::Memory::copy(m_pBuffer, src_.m_pBuffer,
						 sizeof(ModUInt32) * m_uiBufferSize);
	}
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::~SmartLocationList -- デストラクタ
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
SmartLocationList::~SmartLocationList()
{
	if (m_pBuffer) delete[] m_pBuffer;
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::operator = -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//	const SmartLocationList& src_
//		代入元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SmartLocationList&
SmartLocationList::operator = (const SmartLocationList& src_)
{
	LocationList::operator = (src_);
	m_pLocationCoder = src_.m_pLocationCoder;
	m_uiLastLocation = src_.m_uiLastLocation;
	m_uiBitOffset = src_.m_uiBitOffset;
	if (m_pBuffer) delete[] m_pBuffer, m_pBuffer = 0;
	m_uiBufferSize = src_.m_uiBufferSize;
	
	if (m_uiBufferSize)
	{
		m_pBuffer = new ModUInt32[m_uiBufferSize];
		Os::Memory::copy(m_pBuffer, src_.m_pBuffer,
						 sizeof(ModUInt32) * m_uiBufferSize);
	}

	return *this;
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::getIterator -- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator::AutoPointer
//		イテレータ
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
SmartLocationList::getIterator() const
{
	if (m_bNoLocation == true)
		return 0;
	
	return LocationListIterator::AutoPointer(
		new SmartLocationListIterator(m_pBuffer, m_uiBitOffset, m_uiCount,
									  m_pLocationCoder, m_uiLength));
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::pushBack -- 位置情報を追加する
//
//	NOTES
//	追加する位置情報は単調増加することを想定しており、それ以外の場合の動作は不定
//
//	ARGUMENTS
//	ModSize uiLocation_
//		位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SmartLocationList::pushBack(ModSize uiLocation_)
{
	if (m_bNoLocation == false)
	{
		; _SYDNEY_ASSERT(uiLocation_ > m_uiLastLocation);
		
		
		// まずは、追加する位置情報のサイズを確認して、現在のバッファで
		// 足りるかどうか確認する
	
		ModSize len
			= m_pLocationCoder->getBitLength(uiLocation_ - m_uiLastLocation);
		if ((m_uiBitOffset + len) > (m_uiBufferSize * 8 * sizeof(ModUInt32)))
		{
			// バッファが不足しているので、倍のサイズの新しいバッファを確保し、
			// 内容をコピーする

			ModSize s = (m_uiBufferSize != 0) ? m_uiBufferSize * 2 : 1;
			while ((s * 8 * sizeof(ModUInt32)) < (m_uiBitOffset + len))
				s *= 2;
			ModUInt32* p = new ModUInt32[s];
			Os::Memory::reset(p, s * sizeof(ModUInt32));
			if (m_pBuffer)
			{
				Os::Memory::copy(p, m_pBuffer,
								 m_uiBufferSize * sizeof(ModUInt32));
				delete[] m_pBuffer;
			}
			m_pBuffer = p;
			m_uiBufferSize = s;
		}

#ifdef DEBUG
		ModSize uiSave = m_uiBitOffset;
#endif
		m_pLocationCoder->append(uiLocation_ - m_uiLastLocation,
								 m_pBuffer, m_uiBitOffset);
#ifdef DEBUG
		ModSize uiGap;
		m_pLocationCoder->get(uiGap, m_pBuffer, m_uiBitOffset, uiSave);
		; _SYDNEY_ASSERT(uiLocation_ - m_uiLastLocation == uiGap);
#endif
		m_uiLastLocation = uiLocation_;
	}

	++m_uiCount;
}

//
//	FUNCTION public
//	FullText2::SmartLocationList::clear -- クリアする
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
SmartLocationList::clear()
{
	LocationList::clear();
	
	m_uiLastLocation = 0;
	m_uiBitOffset = 0;
	
	if (m_pBuffer)
		// 解放はせずに、リセットするのみ
		Os::Memory::reset(m_pBuffer, m_uiBufferSize * sizeof(ModUInt32));
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
