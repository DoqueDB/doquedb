// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapIterator.h -- バリュー部分を得るイテレータ
// 
// Copyright (c) 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_BITMAPITERATOR_H
#define __SYDNEY_BITMAP_BITMAPITERATOR_H

#include "Bitmap/Module.h"
#include "Common/BitSet.h"
#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::BitmapIterator -- バリュー部分を得るイテレータ
//
//	NOTES
//
class BitmapIterator
{
public:
	// コンストラクタ
	BitmapIterator();
	// デストラクタ
	virtual ~BitmapIterator();

	// 現在位置のビットマップを得て、次の位置に進む
	virtual ModUInt32 getNext() = 0;
	// 現在位置のビットマップを得る
	virtual ModUInt32 get() = 0;

	// 移動する(ModUInt32単位)
	virtual void seek(ModSize offset_) = 0;

	// 終端か
	virtual bool isEnd() = 0;

	// Common::BitSet::UnitType ごとのインターフェース

	// 現在位置のビットマップを得て、次の位置に進む
	Common::BitSet::UnitType getNextUnitType()
	{
		Common::BitSet::UnitType unit;
		int n = Common::BitSet::UNIT_SIZE/sizeof(ModUInt32);
		for (int i = 0; i < n; ++i)
		{
			unit[i] = getNext();
		}
		return unit;
	}
	
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_ITERATOR_H

//
//	Copyright (c) 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
