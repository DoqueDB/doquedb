// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalShortLeafLocationListIterator.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/NormalShortLeafLocationListIterator.h"
#include "FullText2/NormalShortLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::
//		NormalShortLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	NormalShortLeafNode& cNode_
//		ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalShortLeafLocationListIterator::
NormalShortLeafLocationListIterator(NormalShortLeafNode& cNode_)
	: LocationListIterator(&cNode_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::
//		~NormalShortLeafLocationListIterator
//		-- デストラクタ
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
NormalShortLeafLocationListIterator::
~NormalShortLeafLocationListIterator()
{
	m_pNormal = 0;
	m_pShort = 0;
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::find
//		-- 位置情報を検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& length_
//		長さ
//
//	RETURN
//	bool
//		ヒットした場合にはtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
NormalShortLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::lowerBound
//		-- 位置情報を検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& length_
//		長さ
//
//	RETURN
//	ModSize
//		検索された位置
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafLocationListIterator::lowerBound(ModSize location_,
												int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::release
//		-- 解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	bool
//		フリーリストにつなげた場合は true、
//		呼び出し側で delete しなくてはならない場合は false を返す
//
//	EXCEPTIONS
//
bool
NormalShortLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_pNormal = 0;
	m_pShort = 0;
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafLocationListIterator::getTermFrequency
//		-- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION private
//	FullText2::NormalShortLeafLocationListIterator::lowerBoundImpl
//		-- 位置情報を検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& length_
//		長さ
//
//	RETURN
//	ModSize
//		位置
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafLocationListIterator::lowerBoundImpl(ModSize location_,
													int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 現在位置より前の位置を検索している
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}

	m_uiCurrentLocation = location_;
	m_iCurrentLength = 0;

	while (m_uiCurrentLocation != UndefinedLocation)
	{
		// まずはNormalLefNodeの位置を求める

		int len;
		m_uiCurrentLocation = m_pNormal->lowerBound(m_uiCurrentLocation, len);

		if (m_uiCurrentLocation != UndefinedLocation)
		{
			// 次にShortLeafNodeの位置を求める

			ModSize loc = m_uiCurrentLocation + m_uiPos;
			ModSize l = m_pShort->lowerBound(loc, len);

			if (l == UndefinedLocation)
			{
				m_uiCurrentLocation = UndefinedLocation;
				break;
			}

			if (l != loc)
			{
				// 次へ
				m_uiCurrentLocation = l - m_uiPos;
				continue;
			}

			// 終了
			m_iCurrentLength = static_cast<int>(m_uiPos + len);
			length_ = m_iCurrentLength;
			break;
		}
	}
	
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::NormalShortLeafLocationListIterator::resetImpl
//		-- カーソルをリセットする
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
NormalShortLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;

	// 子ノードをすべてリセットする
	m_pNormal->reset();
	m_pShort->reset();
}

//
//	FUNCTION private
//	FullText2::NormalShortLeafLocationListIterator::nextImpl
//		-- 次の位置情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	int& length_
//		長さ
//
//	RETRN
//	ModSize
//		次の位置情報
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, length_);
}

//
//	FUNCTION private
//	FullText2::NormalShortLeafLocationListIterator::getTermFrequencyImpl
//		-- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int dummy;
	while (nextImpl(dummy) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
