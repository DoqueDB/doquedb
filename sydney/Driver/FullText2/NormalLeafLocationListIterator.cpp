// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalLeafLocationListIterator.cpp --
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
#include "FullText2/NormalLeafLocationListIterator.h"
#include "FullText2/NormalLeafNode.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::NormalLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	NormalLeafNode& cNode_
//		ノード
//	ModSize reserve_
//		m_cVectorの要素をリザーブする数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalLeafLocationListIterator::
NormalLeafLocationListIterator(NormalLeafNode& cNode_,
							   ModSize reserve_)
	: LocationListIterator(&cNode_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
	if (reserve_ != 0) m_cVector.reserve(reserve_);
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::~NormalLeafLocationListIterator
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
NormalLeafLocationListIterator::~NormalLeafLocationListIterator()
{
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::find
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
NormalLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::lowerBound
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
NormalLeafLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::release
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
NormalLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_cVector.erase(m_cVector.begin(), m_cVector.end());
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::getTermFrequency
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
NormalLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::pushBack
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
NormalLeafLocationListIterator::pushBack(ModSize pos,
										 LocationListIterator::AutoPointer i)
{
	if (i.get()) m_cVector.pushBack(LocationPair(pos, i));
}

//
//	FUNCTION private
//	FullText2::NormalLeafLocationListIterator::lowerBoundImpl
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
NormalLeafLocationListIterator::lowerBoundImpl(ModSize location_, int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 前の位置を検索しているので、現在値を返す
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}

	m_uiCurrentLocation = location_;
	m_iCurrentLength = 0;
	ModSize endLoc = 0;
	LocationVector::Iterator b = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	LocationVector::Iterator i = b;
	while (i < e)
	{
		int len;
		ModSize pos = (*i).first;
		ModSize loc = (*i).second->lowerBound(m_uiCurrentLocation + pos,
											  len);
		if (loc == UndefinedLocation)
		{
			// 最後に達した
			m_uiCurrentLocation = UndefinedLocation;
			return m_uiCurrentLocation;
		}

		if (endLoc < (loc + len)) endLoc = (loc + len);
		loc -= pos;
			
		if (loc != m_uiCurrentLocation)
		{
			// 新しい
			m_uiCurrentLocation = loc;

			if (i != b)
			{
				// 位置情報が変わったので初めから
				//
				//【注意】
				// m_cVector内は文書頻度の少ない順にソートされている
				// おそらく文書内頻度も少ないと思われるので、
				// 続きから確認するより、最初に戻った方がたぶん効率的である

				i = b;
				continue;
			}
		}
			
		++i;
	}

	m_iCurrentLength = static_cast<int>(endLoc - m_uiCurrentLocation);
	length_ = m_iCurrentLength;
			
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::NormalLeafLocationListIterator::resetImpl
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
NormalLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;

	// 子ノードをすべてリセットする
	LocationVector::Iterator i = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
		(*i).second->reset();
}

//
//	FUNCTION public
//	FullText2::NormalLeafLocationListIterator::nextImpl
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
NormalLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, length_);
}

//
//	FUNCTION private
//	FullText2::NormalLeafLocationListIterator::getTermFrequencyImpl
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
NormalLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int dummy;
	while (nextImpl(dummy) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
