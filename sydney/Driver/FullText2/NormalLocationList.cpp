// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalLocationList.cpp
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
#include "FullText2/NormalLocationList.h"
#include "FullText2/NormalLocationListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::NormalLocationList -- コンストラクタ(1)
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
NormalLocationList::NormalLocationList()
	: LocationList()
{
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::NormalLocationList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	ModSize length_
//		トークン長
//	bool bNoLocation_
//		位置情報を格納しないかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalLocationList::NormalLocationList(ModSize length_, bool bNoLocation_)
	: LocationList(length_, bNoLocation_)
{
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::~NormalLocationList -- デストラクタ
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
NormalLocationList::~NormalLocationList()
{
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::NormalLocationList -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::NormalLocationList& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalLocationList::NormalLocationList(const NormalLocationList& src_)
	: LocationList(src_),
	  m_vecLocation(src_.m_vecLocation)
{
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::getIterator -- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator::AutoPointer
//		イテレータ
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
NormalLocationList::getIterator() const
{
	if (m_bNoLocation == true)
		return 0;
	
	return LocationListIterator::AutoPointer(
		new NormalLocationListIterator(m_vecLocation, m_uiLength));
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::pushBack -- 位置情報を追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLocation_;
//		位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalLocationList::pushBack(ModSize uiLocation_)
{
	if (m_bNoLocation == false)
		m_vecLocation.pushBack(uiLocation_);
	LocationList::pushBack(uiLocation_);
}

//
//	FUNCTION public
//	FullText2::NormalLocationList::clear -- 中身をクリアする
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
NormalLocationList::clear()
{
	LocationList::clear();
	
	// メモリは解放しない
	m_vecLocation.erase(m_vecLocation.begin(), m_vecLocation.end());
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
