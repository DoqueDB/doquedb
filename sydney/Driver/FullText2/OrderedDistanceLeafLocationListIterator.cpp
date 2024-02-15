// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OrderedDistanceLeafLocationListIterator.cpp --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OrderedDistanceLeafLocationListIterator.h"
#include "FullText2/OrderedDistanceLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::
//		OrderedDistanceLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	OrderedDistanceLeafNode& cNode_
//		ノード
//	ModSize reserve_
//		m_cVectorの要素をリザーブする数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OrderedDistanceLeafLocationListIterator::
OrderedDistanceLeafLocationListIterator(OrderedDistanceLeafNode& cNode_,
										ModSize reserve_)
	: LocationListIterator(&cNode_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
	if (reserve_ != 0) m_cVector.reserve(reserve_);
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::
//		~OrderedDistanceLeafLocationListIterator
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
OrderedDistanceLeafLocationListIterator::
~OrderedDistanceLeafLocationListIterator()
{
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::find
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
OrderedDistanceLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::lowerBound
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
OrderedDistanceLeafLocationListIterator::lowerBound(ModSize location_,
													int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::release
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
OrderedDistanceLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_cVector.erase(m_cVector.begin(), m_cVector.end());
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::getTermFrequency
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
OrderedDistanceLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafLocationListIterator::pushBack
//		-- 位置情報へのイテレータを追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize pos
//		先頭からの位置
// 	FullText2::LocationListIterator::AutoPointer i
//		位置情報へのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OrderedDistanceLeafLocationListIterator::
pushBack(LocationListIterator::AutoPointer i)
{
	if (i.get()) m_cVector.pushBack(i);
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafLocationListIterator::lowerBoundImpl
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
OrderedDistanceLeafLocationListIterator::lowerBoundImpl(ModSize location_,
														int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 現在位置より前の位置を検索している
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}

	m_uiCurrentLocation = UndefinedLocation;
	m_iCurrentLength = 0;

	LocationVector::Iterator s = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();

	// まずは先頭要素の位置を求める

	int minlen;
	int maxlen;
	m_uiCurrentLocation = (*s)->lowerBound(location_, minlen, maxlen);

	while (m_uiCurrentLocation != UndefinedLocation)
	{
		// 先頭以外のものを順番にチェックする

		ModSize minloc = m_uiCurrentLocation + minlen;
		ModSize maxloc = m_uiCurrentLocation + maxlen;

		LocationVector::Iterator i = s;
		for (++i; i < e; ++i)
		{
			ModSize loc = (*i)->lowerBound(minloc, minlen, maxlen);
			if (loc > maxloc)
			{
				// 次へ
				break;
			}
			
			minloc = loc + minlen;
			maxloc = loc + maxlen;
		}

		if (i == e)
		{
			// 終了
			m_iCurrentLength = static_cast<int>(maxloc - m_uiCurrentLocation);
			length_ = m_iCurrentLength;
			break;
		}

		// 次へ
		m_uiCurrentLocation = (*s)->lowerBound(m_uiCurrentLocation + 1,
											   minlen, maxlen);
	}
	
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafLocationListIterator::resetImpl
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
OrderedDistanceLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;

	// 子ノードをすべてリセットする
	LocationVector::Iterator i = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		(*i)->reset();
	}
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafLocationListIterator::nextImpl
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
OrderedDistanceLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, length_);
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafLocationListIterator::getTermFrequencyImpl
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
OrderedDistanceLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int dummy;
	while (nextImpl(dummy) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
