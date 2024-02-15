// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalLocationList.h -- 位置情報リストをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_NORMALLOCATIONLIST_H
#define __SYDNEY_FULLTEXT2_NORMALLOCATIONLIST_H

#include "FullText2/Module.h"
#include "FullText2/LocationList.h"
#include "FullText2/LocationListIterator.h"

#include "Common/LargeVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::NormalLocationList --
//
//	NOTES
//	位置情報を圧縮しないで配列で保持する
//
class NormalLocationList : public LocationList
{
public:
	// コンストラクタ(1) -- ModMap用
	NormalLocationList();
	// コンストラクタ(2)
	NormalLocationList(ModSize length_, bool bNoLocation_ = false);
	// デストラクタ
	virtual ~NormalLocationList();
	// コピーコンストラクタ -- ModMap用
	NormalLocationList(const NormalLocationList& src_);

	// イテレータを得る
	LocationListIterator::AutoPointer getIterator() const;

	// 位置情報を追加する
	void pushBack(ModSize uiLocation_);
	// 中身をclearする
	void clear();

private:
	// 位置情報
	Common::LargeVector<ModSize> m_vecLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NORMALLOCATIONLIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
