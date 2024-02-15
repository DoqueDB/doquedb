// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeadLeafLocationListIterator.cpp --
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
#include "FullText2/HeadLeafLocationListIterator.h"
#include "FullText2/HeadLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::HeadLeafLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	HeadLeafNode& cNode_
//		ノード
//	ModSize uiLocation_
//		先頭からの位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
HeadLeafLocationListIterator::
HeadLeafLocationListIterator(HeadLeafNode& cNode_,
							 ModSize uiLocation_)
	: LocationListIterator(&cNode_), m_uiLocation(uiLocation_), m_bEnd(false)
{
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::~HeadLeafLocationListIterator
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
HeadLeafLocationListIterator::~HeadLeafLocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::find
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
HeadLeafLocationListIterator::find(ModSize location_, int& length_)
{
	if (lowerBoundImpl(location_, length_) == location_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::lowerBound
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
HeadLeafLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	return lowerBoundImpl(location_, length_);
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::release
//		-- 解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		フリーリストにつなげた場合は true、
//		呼び出し側で delete しなくてはならない場合は false を返す
//
//	EXCEPTIONS
//
bool
HeadLeafLocationListIterator::release()
{
	m_bEnd = false;
	m_pTerm = 0;
	return LocationListIterator::release();
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::getTermFrequency
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
HeadLeafLocationListIterator::getTermFrequency()
{
	return getTermFrequencyImpl();
}

//
//	FUNCTION private
//	FullText2::HeadLeafLocationListIterator::lowerBoundImpl
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
HeadLeafLocationListIterator::lowerBoundImpl(ModSize location_, int& length_)
{
	ModSize loc = m_pTerm->lowerBound(location_, length_);
	if (loc != m_uiLocation)
		loc = UndefinedLocation;
	return loc;
}

//
//	FUNCTION private
//	FullText2::HeadLeafLocationListIterator::resetImpl
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
HeadLeafLocationListIterator::resetImpl()
{
	m_bEnd = false;
	if (m_pTerm.get()) m_pTerm->reset();
}

//
//	FUNCTION public
//	FullText2::HeadLeafLocationListIterator::nextImpl
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
HeadLeafLocationListIterator::nextImpl(int& length_)
{
	if (m_bEnd == true)
		return UndefinedLocation;
	
	ModSize loc = m_pTerm->lowerBound(m_uiLocation, length_);
	if (loc != m_uiLocation)
		loc = UndefinedLocation;

	m_bEnd = true;
		
	return loc;
}

//
//	FUNCTION private
//	FullText2::HeadLeafLocationListIterator::getTermFrequencyImpl
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
HeadLeafLocationListIterator::getTermFrequencyImpl()
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
