// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortLeafLocationListIterator.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/ShortLeafLocationListIterator.h"

#include "FullText2/ShortLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafLocationListIterator::ShortLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	LeafNode& cNode_
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
ShortLeafLocationListIterator::
ShortLeafLocationListIterator(LeafNode& cNode_,
							  int length_,
							  ModSize reserve_)
	: LocationListIterator(&cNode_),
	  m_iLength(length_), m_uiCurrentLocation(0),
	  m_iCurrentMinLength(0), m_iCurrentMaxLength(0)
{
	if (reserve_ != 0) m_cVector.reserve(reserve_);
}

//
//	FUNCTION public
//	FullText2::ShortLeafLocationListIterator::~ShortLeafLocationListIterator
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
ShortLeafLocationListIterator::~ShortLeafLocationListIterator()
{
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::ShortLeafLocationListIterator::release
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
ShortLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	m_cVector.erase(m_cVector.begin(), m_cVector.end());
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::ShortLeafLocationListIterator::pushBack
//		-- 位置情報へのイテレータを追加する
//
//	NOTES
//
//	ARGUMENTS
// 	FullText2::LocationListIterator::AutoPointer i
//		位置情報へのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortLeafLocationListIterator::pushBack(LocationListIterator::AutoPointer i)
{
	if (i.get()) m_cVector.pushBack(LocationPair(Data(), i));
}

//
//	FUNCTION private
//	FullText2::ShortLeafLocationListIterator::lowerBoundImpl
//		-- 位置情報を検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& minLength_
//		検索語長(最短)
//	int& maxLength_
//		検索語長(最長)
//
//	RETURN
//	ModSize
//		位置
//
//	EXCEPTIONS
//
ModSize
ShortLeafLocationListIterator::lowerBoundImpl(ModSize location_,
											  int& minLength_, int& maxLength_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 前の位置を検索しているので、現在値を返す
		minLength_ = m_iCurrentMinLength;
		maxLength_ = m_iCurrentMaxLength;
		return m_uiCurrentLocation;
	}
	
	m_uiCurrentLocation = UndefinedLocation;	// 最大値
	minLength_ = 0;
	maxLength_ = 0;

	// 最小の位置を探す
	LocationVector::Iterator b = m_cVector.begin();
	LocationVector::Iterator e = m_cVector.end();
	LocationVector::Iterator i = b;
	for (; i < e; ++i)
	{
		if ((*i).first.loc < location_)
		{
			int len = 0;
			ModSize loc = (*i).second->lowerBound(location_, len);
			(*i).first.loc = loc;
			(*i).first.len = len;
		}

		if ((*i).first.loc == UndefinedLocation)
			// ヒットしていない
			continue;
		
		if ((*i).first.loc < m_uiCurrentLocation)
		{
			// 新しい位置
	
			m_uiCurrentLocation = (*i).first.loc;
			minLength_ = (*i).first.len;
			maxLength_ = (*i).first.len;
		}
		else if (m_iLength == -1 &&
				 (*i).first.loc == m_uiCurrentLocation)
		{
			// 同じ位置なので、長さを確認する

			if (minLength_ > (*i).first.len)
				minLength_ = (*i).first.len;
			else if (maxLength_ < (*i).first.len)
				maxLength_ = (*i).first.len;
		}
	}

	if (m_iLength != -1)
	{
		minLength_ = m_iLength;
		maxLength_ = m_iLength;
	}
	
	m_iCurrentMinLength = minLength_;
	m_iCurrentMaxLength = maxLength_;
			
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::ShortLeafLocationListIterator::resetImpl
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
ShortLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;

	// 子ノードをすべてリセットする
	LocationVector::Iterator i = m_cVector.begin();
	for (; i != m_cVector.end(); ++i)
		(*i).second->reset();
}

//
//	FUNCTION public
//	FullText2::ShortLeafLocationListIterator::nextImpl
//		-- 次の位置情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRN
//	ModSize
//		次の位置情報
//
//	EXCEPTIONS
//
ModSize
ShortLeafLocationListIterator::nextImpl(int& minLength_, int& maxLength_)
{
	if (m_uiCurrentLocation == UndefinedLocation)
		return UndefinedLocation;
	return lowerBoundImpl(m_uiCurrentLocation + 1, minLength_, maxLength_);
}

//
//	FUNCTION private
//	FullText2::ShortLeafLocationListIterator::getTermFrequencyImpl
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
ShortLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int len1 = 0;
	int len2 = 0;
	while (nextImpl(len1, len2) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
