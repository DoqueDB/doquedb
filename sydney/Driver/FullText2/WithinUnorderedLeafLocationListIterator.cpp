// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinUnorderedLeafLocationListIterator.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/WithinUnorderedLeafLocationListIterator.h"
#include "FullText2/WithinUnorderedLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::
//		WithinUnorderedLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	WithinUnorderedLeafNode& cNode_
//		ノード
//	int length_
//		全長
//	ModSize reserve_
//		m_cVectorの要素をリザーブする数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WithinUnorderedLeafLocationListIterator::
WithinUnorderedLeafLocationListIterator(WithinUnorderedLeafNode& cNode_,
										ModSize uiLower_,
										ModSize uiUpper_,
										ModSize reserve_)
	: LocationListIterator(&cNode_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
	if (reserve_ != 0) m_cVector.reserve(reserve_);
	
	m_uiLower = (uiLower_ == 0) ? 0 : (uiLower_ - 1);
	m_uiUpper = (uiUpper_ == 0) ? 0 : (uiUpper_ - 1);
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::
//		~WithinUnorderedLeafLocationListIterator
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
WithinUnorderedLeafLocationListIterator::
~WithinUnorderedLeafLocationListIterator()
{
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::find
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
WithinUnorderedLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::lowerBound
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
WithinUnorderedLeafLocationListIterator::lowerBound(ModSize location_,
													int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::release
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
WithinUnorderedLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_cVector.erase(m_cVector.begin(), m_cVector.end());
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::getTermFrequency
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
WithinUnorderedLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafLocationListIterator::pushBack
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
WithinUnorderedLeafLocationListIterator::
pushBack(LocationListIterator::AutoPointer i)
{
	if (i.get()) m_cVector.pushBack(LocationPair(Data(), i));
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafLocationListIterator::lowerBoundImpl
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
WithinUnorderedLeafLocationListIterator::lowerBoundImpl(ModSize location_,
														int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 現在位置より前の位置を検索している
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}

	m_uiCurrentLocation = UndefinedLocation;
	m_cMinLocation.location = location_;
	bool nextUpper = false;

	LocationVector::Iterator b = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	
	for (;;)
	{
		Data minLoc(UndefinedLocation, 0);
		Data maxLoc(0, 0);

		for (LocationVector::Iterator i = b; i < e; ++i)
		{
			if ((*i).first.location < m_cMinLocation.location)
			{
				// 保存されている最小値以下のものをすべて次にすすめる
				
				(*i).first.location
					= (*i).second->lowerBound(m_cMinLocation.location,
											  (*i).first.length);
			}
			else if (nextUpper &&
					 (*i).first.location == m_cMaxLocation.location)
			{
				// 最大値のものを1つだけ次に進める

				(*i).first.location
					= (*i).second->next((*i).first.length);
				nextUpper = false;
			}
			
			if ((*i).first.location == UndefinedLocation)
			{
				// 終わり
				return UndefinedLocation;
			}
			
			// 最小値または最大値が更新されたら記憶する
			
			if ((*i).first.location < minLoc.location)
			{
				minLoc.location = (*i).first.location;
				minLoc.length = (*i).first.length;
			}
			if ((*i).first.location > maxLoc.location)
			{
				maxLoc.location = (*i).first.location;
				maxLoc.length = (*i).first.length;
			}
		}

		nextUpper = false;

		// 今の最小値と最大値を記憶する
		
		m_cMinLocation.location = minLoc.location;
		m_cMinLocation.length = minLoc.length;
		m_cMaxLocation.location = maxLoc.location;
		m_cMaxLocation.length = maxLoc.length;

		// 現在のものが条件に一致しているか確認する

		if ((maxLoc.location - (minLoc.location + minLoc.length))
			>= m_uiLower &&
			(maxLoc.location - (minLoc.location + minLoc.length))
			<= m_uiUpper)
		{
			// 一致している

			m_uiCurrentLocation = minLoc.location;
			m_iCurrentLength
				= maxLoc.location + maxLoc.length - minLoc.location;
			length_ = m_iCurrentLength;

			break;
		}
		else if ((maxLoc.location - (minLoc.location + minLoc.length))
				 > m_uiUpper)
		{
			// 上限値が超えている
			//
			//【注意】
			// minLoc.length が固定長なら、
			// maxLoc.location - m_uiUpper - minLo.length にできるが、
			// minLoc.length がいくつになるのかわからないので、
			// １つづつ調べるしかない
			
			m_cMinLocation.location += 1;
		}
		else
		{
			// 下限値が超えている
			nextUpper = true;
		}
	}

	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafLocationListIterator::resetImpl
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
WithinUnorderedLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;
	m_iCurrentLength = 0;
	m_cMinLocation.location = 0;
	m_cMinLocation.length = 0;
	m_cMaxLocation.location = 0;
	m_cMaxLocation.length = 0;

	// 子ノードをすべてリセットする
	LocationVector::Iterator i = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		(*i).first.location = 0;
		(*i).first.length = 0;
		(*i).second->reset();
	}
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafLocationListIterator::nextImpl
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
WithinUnorderedLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, length_);
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafLocationListIterator::getTermFrequencyImpl
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
WithinUnorderedLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int dummy;
	while (nextImpl(dummy) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
