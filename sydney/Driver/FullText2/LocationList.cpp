// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationList.cpp
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
#include "FullText2/LocationList.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::LocationList::LocationList -- コンストラクタ(1)
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
LocationList::LocationList()
	: m_uiCount(0), m_bNoLocation(false), m_uiLength(0)
{
}

//
//	FUNCTION public
//	FullText2::LocationList::LocationList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	ModSize length_
//		トークンの長さ
//	bool bNoLocation_
//		位置情報を格納しないかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LocationList::LocationList(ModSize length_, bool bNoLocation_)
	: m_uiCount(0), m_bNoLocation(bNoLocation_), m_uiLength(length_)
{
}

//
//	FUNCTION public
//	FullText2::LocationList::~LocationList -- デストラクタ
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
LocationList::~LocationList()
{
}

//
//	FUNCTION public
//	FullText2::LocationList::LocationList -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	const FullText2::LocationList& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LocationList::LocationList(const LocationList& src_)
	: m_uiCount(src_.m_uiCount),
	  m_bNoLocation(src_.m_bNoLocation),
	  m_uiLength(src_.m_uiLength)
{
}

//
//	FUNCTION public
//	FullText2::LocationList::operator = -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
// 	const FullText2::LocationList& src_
//		代入元
//
//	RETURN
//	FullText2::LocationList&
//		自分自身への参照
//
//	EXCEPTIONS
//
LocationList&
LocationList::operator = (const LocationList& src_)
{
	m_uiCount = src_.m_uiCount;
	m_bNoLocation = src_.m_bNoLocation;
	m_uiLength = src_.m_uiLength;

	return *this;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
