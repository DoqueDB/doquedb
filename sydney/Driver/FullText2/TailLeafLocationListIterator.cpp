// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TailLeafLocationListIterator.cpp --
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
#include "FullText2/TailLeafLocationListIterator.h"

#include "FullText2/TailLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::TailLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	TailLeafNode& cNode_
//		ノード
//	ModSize uiLocation_
//		先頭からの位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TailLeafLocationListIterator::
TailLeafLocationListIterator(TailLeafNode& cNode_,
							 ModSize uiLocation_)
	: LocationListIterator(&cNode_), m_uiLocation(uiLocation_), m_bEnd(false)
{
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::~TailLeafLocationListIterator
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
TailLeafLocationListIterator::~TailLeafLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::find
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
TailLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::lowerBound
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
TailLeafLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::release
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
TailLeafLocationListIterator::release()
{
	m_bEnd = false;
	m_pTerm = 0;
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::getTermFrequency
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
TailLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION private
//	FullText2::TailLeafLocationListIterator::lowerBoundImpl
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
TailLeafLocationListIterator::lowerBoundImpl(ModSize location_, int& length_)
{
	ModSize loc;
	while ((loc = m_pTerm->lowerBound(location_, length_)) != UndefinedLocation)
	{
		if (loc < (m_uiDocumentLength - length_ - m_uiLocation + 1))
		{
			++location_;
			continue;
		}
		if (loc !=  (m_uiDocumentLength - length_ - m_uiLocation + 1))
		{
			loc = UndefinedLocation;
		}
		break;
	}
	return loc;
}

//
//	FUNCTION private
//	FullText2::TailLeafLocationListIterator::resetImpl
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
TailLeafLocationListIterator::resetImpl()
{
	m_bEnd = false;
	if (m_pTerm.get()) m_pTerm->reset();
}

//
//	FUNCTION public
//	FullText2::TailLeafLocationListIterator::nextImpl
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
TailLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_bEnd == true)
		return UndefinedLocation;

	ModSize loc;
	while ((loc = m_pTerm->next(length_)) != UndefinedLocation)
	{
		if (loc < (m_uiDocumentLength - length_ - m_uiLocation + 1))
			continue;
		if (loc !=  (m_uiDocumentLength - length_ - m_uiLocation + 1))
		{
			loc = UndefinedLocation;
		}
		break;
	}

	m_bEnd = true;
		
	return loc;
}

//
//	FUNCTION private
//	FullText2::TailLeafLocationListIterator::getTermFrequencyImpl
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
TailLeafLocationListIterator::getTermFrequencyImpl()
{
	resetImpl();
	ModSize count = 0;
	int length = 0;
	while (nextImpl(length) != UndefinedLocation)
		++count;
	return count;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
