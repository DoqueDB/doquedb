// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchList.cpp --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Inverted/BatchList.h"
#include "Inverted/BatchListMap.h"
#include "Inverted/BatchListIterator.h"

#include "ModList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::BatchList::BatchList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	Inverted::BatchListMap& cBatchListMap_
//		バッチリストマップ
//	const ModUnicodeChar* pKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchList::BatchList(InvertedUnit& cInvertedUnit_,
					 BatchListMap& cBatchListMap_,
					 const ModUnicodeChar* pszKey_)
: BatchBaseList(cInvertedUnit_, cBatchListMap_, pszKey_),
  m_pMap(&cBatchListMap_), m_pArea(0)
{
	m_pArea = LeafPage::Area::allocateArea(getKey(), getAllocateUnitSize());

	// 大きさを求める
	ModSize size = m_pArea->getUnitSize() * sizeof(ModUInt32);
	size += sizeof(BatchList);			// BatchList自体の大きさ
	size += getKey().getBufferSize();	// ModUnicodeStringの大きさ
	size += sizeof(ModListNode<BatchList*>);		// ListNodeの大きさ

	m_pMap->addListSize(size);
}

//
//	FUNCTION public
//	Inverted::BatchList::BatchList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	LeafPage::Area* pArea_
//		リーフページのエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchList::BatchList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_)
	: BatchBaseList(cInvertedUnit_, pArea_),
	  m_pMap(0), m_pArea(pArea_)
{
}

//
//	FUNCTION public
//	Inverted::BatchList::~BatchList -- デストラクタ
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
//	なし
//
BatchList::~BatchList()
{
}

//
//	FUNCTION public
//	Inverted::BatchList::begin -- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::InvertedItertor*
//		イテレータ
//
//	EXCEPTIONS
//
InvertedIterator*
BatchList::begin() const
{
	return new BatchListIterator(const_cast<BatchList&>(*this));
}

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
