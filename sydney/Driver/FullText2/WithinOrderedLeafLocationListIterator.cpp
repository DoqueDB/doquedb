// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinOrderedLeafLocationListIterator.cpp --
// 
// Copyright (c) 2010, 2014, 2023 Ricoh Company, Ltd.
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
#include "FullText2/WithinOrderedLeafLocationListIterator.h"
#include "FullText2/WithinOrderedLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::
//		WithinOrderedLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	WithinOrderedLeafNode& cNode_
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
WithinOrderedLeafLocationListIterator::
WithinOrderedLeafLocationListIterator(WithinOrderedLeafNode& cNode_,
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
//	FullText2::WithinOrderedLeafLocationListIterator::
//		~WithinOrderedLeafLocationListIterator
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
WithinOrderedLeafLocationListIterator::~WithinOrderedLeafLocationListIterator()
{
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::find
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
WithinOrderedLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::lowerBound
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
WithinOrderedLeafLocationListIterator::lowerBound(ModSize location_,
												  int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::release
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
WithinOrderedLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_cVector.erase(m_cVector.begin(), m_cVector.end());
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::getTermFrequency
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
WithinOrderedLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION public
//	FullText2::WithinOrderedLeafLocationListIterator::pushBack
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
WithinOrderedLeafLocationListIterator::
pushBack(LocationListIterator::AutoPointer i)
{
	if (i.get()) m_cVector.pushBack(LocationPair(Data(), i));
}

//
//	FUNCTION private
//	FullText2::WithinOrderedLeafLocationListIterator::lowerBoundImpl
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
WithinOrderedLeafLocationListIterator::lowerBoundImpl(ModSize location_,
													  int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 現在位置より前の位置を検索している
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}

	m_uiCurrentLocation = UndefinedLocation;

	// まずは、先頭の要素と最後の要素が距離の条件を満たしている場所を探す

	LocationVector::Iterator s = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	--e;

	// 要求位置以降で最初の位置まで先頭要素を進める
	(*s).first.location = (*s).second->lowerBound(location_, (*s).first.length);
	if ((*s).first.location == UndefinedLocation)
		// 先頭要素がないので、終わり
		return m_uiCurrentLocation;

	// 先頭要素の位置から その長さ + m_uiLower 分後方まで最後の要素を進める
	ModSize endLoc = (*s).first.location + (*s).first.length + m_uiLower;
	(*e).first.location = (*e).second->lowerBound(endLoc, (*e).first.length);
	endLoc = (*e).first.location;

	while ((*s).first.location != UndefinedLocation &&
		   (*e).first.location != UndefinedLocation)
	{
		if ((*e).first.location <=
			((*s).first.location + (*s).first.length + m_uiUpper) &&
			(*e).first.location >=
			((*s).first.location + (*s).first.length + m_uiLower))
		{
			// とりあえず、先頭と最後は条件を満たしているので、
			// 中間要素が満たしているか確認する

			ModSize loc = (*s).first.location + (*s).first.length;
			LocationVector::Iterator i = s;
			++i;
			for (; i < e; ++i)
			{
				(*i).first.location = (*i).second->lowerBound(
					loc,
					(*i).first.length);

				if ((*i).first.location == UndefinedLocation)
					// 見つからなかった
					return m_uiCurrentLocation;
					
				if (((*i).first.location + (*i).first.length) >
					(*e).first.location)
				{
					// 最後の条件を越したので、次へ
					// 少なくとも、今の位置より最後の要素は後ろ
					endLoc = (*i).first.location + (*i).first.length;
					break;
				}

				loc = (*i).first.location + (*i).first.length;
			}

			if (i == e)
			{
				// すべての条件を満たした
				m_uiCurrentLocation = (*s).first.location;
				m_iCurrentLength = (*e).first.location + (*e).first.length
					- (*s).first.location;
				length_ = m_iCurrentLength;
				break;
			}
			else
			{
				// 条件を満たせなかったので、最後の要素のみ次へ
				(*e).first.location
					= (*e).second->lowerBound(endLoc, (*e).first.length);
				endLoc = (*e).first.location;
				continue;
			}
		}

		// 先頭要素を次へ
		(*s).first.location = (*s).second->next((*s).first.length);
		if ((*s).first.location == UndefinedLocation)
			// もうなかった
			break;

		// 最後の要素を次へ
		//
		// 前回 lowerBound した時より endLoc が大きくないと、lowerBound した
		// 結果は同じになる
		if (endLoc < (*s).first.location + (*s).first.length + m_uiLower)
			endLoc = (*s).first.location + (*s).first.length + m_uiLower;
		(*e).first.location
			= (*e).second->lowerBound(endLoc, (*e).first.length);
		endLoc = (*e).first.location;
	}
			
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::WithinOrderedLeafLocationListIterator::resetImpl
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
WithinOrderedLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;

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
//	FullText2::WithinOrderedLeafLocationListIterator::nextImpl
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
WithinOrderedLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, length_);
}

//
//	FUNCTION private
//	FullText2::WithinOrderedLeafLocationListIterator::getTermFrequencyImpl
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
WithinOrderedLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int dummy;
	while (nextImpl(dummy) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
