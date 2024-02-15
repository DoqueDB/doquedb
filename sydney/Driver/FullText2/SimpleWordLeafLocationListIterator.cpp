// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleWordLeafLocationListIterator.cpp --
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
#include "FullText2/SimpleWordLeafLocationListIterator.h"

#include "FullText2/WordLeafNode.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::
//		SimpleWordLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	WordLeafNode& cNode_
//		ノード
//	ModVector<ModSize>& cWordPosition_
//		単語境界を確認する位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleWordLeafLocationListIterator::
SimpleWordLeafLocationListIterator(WordLeafNode& cNode_,
								   ModVector<ModSize>& cWordPosition_)
	: WordLeafLocationListIterator(cNode_, cWordPosition_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::
//		~SimpleWordLeafLocationListIterator
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
SimpleWordLeafLocationListIterator::~SimpleWordLeafLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::find
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
SimpleWordLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::lowerBound
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
SimpleWordLeafLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::release
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
SimpleWordLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	return WordLeafLocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::SimpleWordLeafLocationListIterator::getTermFrequency
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
SimpleWordLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION private
//	FullText2::SimpleWordLeafLocationListIterator::lowerBoundImpl
//		-- 位置情報を検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize location_
//		検索する位置
//	int& length_
//		検索語長
//
//	RETURN
//	ModSize
//		位置
//
//	EXCEPTIONS
//
ModSize
SimpleWordLeafLocationListIterator::
lowerBoundImpl(ModSize location_, int& length_)
{
	if (location_ <= m_uiCurrentLocation)
	{
		// 現在位置より前の位置を検索している
		length_ = m_iCurrentLength;
		return m_uiCurrentLocation;
	}
	
	ModVector<ModSize>::Iterator b = m_cWordPosition.begin();
	ModVector<ModSize>::Iterator e = m_cWordPosition.end();
		
	for (;;)
	{
		location_ = m_pTerm->lowerBound(location_, length_);
		if (location_ != UndefinedLocation)
		{
			// 単語境界をチェックする
			ModVector<ModSize>::Iterator i = b;
			for (; i < e; ++i)
			{
				int dummy = 0;
				if (m_pSeparator->find(location_ + *i, dummy)
					== false)
					break;
			}

			if (i != e)
			{
				++location_;
				continue;
			}
		}

		break;
	}

	m_iCurrentLength = length_;
	m_uiCurrentLocation = location_;

	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::SimpleWordLeafLocationListIterator::resetImpl
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
SimpleWordLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;
	WordLeafLocationListIterator::resetImpl();
}

//
//	FUNCTION private
//	FullText2::SimpleWordLeafLocationListIterator::nextImpl
//		-- 次の位置情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	int& length_
//		検索語長
//
//	RETRN
//	ModSize
//		次の位置情報
//
//	EXCEPTIONS
//
ModSize
SimpleWordLeafLocationListIterator::nextImpl(int& length_)
{
	ModSize loc = UndefinedLocation;

	ModVector<ModSize>::Iterator b = m_cWordPosition.begin();
	ModVector<ModSize>::Iterator e = m_cWordPosition.end();
		
	for (;;)
	{
		loc = m_pTerm->next(length_);
		if (loc != UndefinedLocation)
		{
			// 単語境界をチェックする
			ModVector<ModSize>::Iterator i = b;
			for (; i < e; ++i)
			{
				int dummy = 0;
				if (m_pSeparator->find(loc + *i, dummy) == false)
					break;
			}

			if (i != e)
				continue;
		}

		break;
	}

	m_uiCurrentLocation = loc;
	m_iCurrentLength = length_;
				
	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::SimpleWordLeafLocationListIterator::getTermFrequencyImpl
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
SimpleWordLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int length = 0;
	while (nextImpl(length) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
