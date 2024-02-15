// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationList.h -- 位置情報リストをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_LOCATIONLIST_H
#define __SYDNEY_FULLTEXT2_LOCATIONLIST_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::LocationList --
//
//	NOTES
//
class LocationList
{
public:
	// コンストラクタ(1) -- ModMap用
	LocationList();
	// コンストラクタ(2)
	LocationList(ModSize length_, bool bNoLocation_ = false);
	// デストラクタ
	virtual ~LocationList();
	// コピーコンストラクタ -- ModMap用
	LocationList(const LocationList& src_);
	// 代入演算子 -- ModMap用
	LocationList& operator = (const LocationList& src_);

	// イテレータを得る
	virtual LocationListIterator::AutoPointer getIterator() const = 0;

	// 位置情報を追加する
	virtual void pushBack(ModSize uiLocation_) { ++m_uiCount; }
	// 中身をclearする
	virtual void clear() { m_uiCount = 0; }
	// 頻度を得る
	ModSize getCount() const { return m_uiCount; }

protected:
	// 頻度
	ModSize m_uiCount;
	// 位置情報を格納しない
	bool m_bNoLocation;
	// トークンの長さ
	ModSize m_uiLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LOCATIONLIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
