// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExactWordLeafLocationListIterator.cpp --
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
#include "FullText2/ExactWordLeafLocationListIterator.h"

#include "FullText2/WordLeafNode.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::
//		ExactWordLeafLocationListIterator
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
ExactWordLeafLocationListIterator::
ExactWordLeafLocationListIterator(WordLeafNode& cNode_,
								  ModVector<ModSize>& cWordPosition_)
	: WordLeafLocationListIterator(cNode_, cWordPosition_),
	  m_uiCurrentLocation(0), m_iCurrentLength(0)
{
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::
//		~ExactWordLeafLocationListIterator
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
ExactWordLeafLocationListIterator::~ExactWordLeafLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::find
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
ExactWordLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::lowerBound
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
ExactWordLeafLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::release
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
ExactWordLeafLocationListIterator::release()
{
	m_uiCurrentLocation = 0;
	return WordLeafLocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::ExactWordLeafLocationListIterator::getTermFrequency
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
ExactWordLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION private
//	FullText2::ExactWordLeafLocationListIterator::lowerBoundImpl
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
ExactWordLeafLocationListIterator::
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
			int dummy = 0;
			
			// 単語境界をチェックする
			ModVector<ModSize>::Iterator i = b;
			if (i != e)
			{
				if (m_pSeparator->find(location_ + *i, dummy)
					== false)
				{
					++location_;
					continue;
				}
				++i;
			}
			for (; i < e; ++i)
			{
				if (m_pSeparator->next(dummy) != (location_ + *i))
				{
					break;
				}
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
//	FullText2::ExactWordLeafLocationListIterator::resetImpl
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
ExactWordLeafLocationListIterator::resetImpl()
{
	// 現在位置をクリア
	m_uiCurrentLocation = 0;
	WordLeafLocationListIterator::resetImpl();
}

//
//	FUNCTION private
//	FullText2::ExactWordLeafLocationListIterator::nextImpl
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
ExactWordLeafLocationListIterator::nextImpl(int& length_)
{
	ModSize loc = UndefinedLocation;

	ModVector<ModSize>::Iterator b = m_cWordPosition.begin();
	ModVector<ModSize>::Iterator e = m_cWordPosition.end();
		
	for (;;)
	{
		loc = m_pTerm->next(length_);
		if (loc != UndefinedLocation)
		{
			int dummy = 0;
			
			// 単語境界をチェックする
			ModVector<ModSize>::Iterator i = b;
			if (i != e)
			{
				if (m_pSeparator->find(loc + *i, dummy)	== false)
					continue;

				++i;
			}
			for (; i < e; ++i)
			{
				if (m_pSeparator->next(dummy) != (loc + *i))
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
//	FullText2::ExactWordLeafLocationListIterator::getTermFrequencyImpl
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
ExactWordLeafLocationListIterator::getTermFrequencyImpl()
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
