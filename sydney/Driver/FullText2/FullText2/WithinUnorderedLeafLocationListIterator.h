// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinUnorderedLeafLocationListItertor.h --
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

#ifndef __SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class WithinUnorderedLeafNode;

//
//	CLASS
//	FullText2::WithinUnorderedLeafLocationListIterator
//		-- 検索時に位置情報を走査するクラス
//
//	NOTES
//
class WithinUnorderedLeafLocationListIterator : public LocationListIterator
{
	friend class WithinUnorderedLeafNode;
	
public:
	// コンストラクタ
	WithinUnorderedLeafLocationListIterator(WithinUnorderedLeafNode& cNode_,
											ModSize uiLower_,
											ModSize uiUpper_,
											ModSize reserve_);
	// デストラクタ
	virtual ~WithinUnorderedLeafLocationListIterator();

	// 位置情報を検索する
	bool find(ModSize location_, int& length_);
	ModSize lowerBound(ModSize location_, int& length_);
	// カーソルを先頭に戻す
	void reset() { resetImpl(); }
	// 次の値を得る
	ModSize next(int& length_)
		{
			return nextImpl(length_);
		}

	// 解放する
	bool release();
	// 文書内頻度を得る
	ModSize getTermFrequency();

	// 位置情報へのイテレータを追加する
	void pushBack(LocationListIterator::AutoPointer i);

private:
	struct Data {
		Data() : location(0), length(0) {}
		Data(ModSize loc, int len) : location(loc), length(len) {}
		
		ModSize	location;	// 位置
		int		length;		// 長さ
	};
	typedef ModPair<Data, LocationListIterator::AutoPointer>	LocationPair;
	typedef ModVector<LocationPair>								LocationVector;

	// 位置情報を検索する
	ModSize lowerBoundImpl(ModSize location_, int& length_);
	// カーソルをリセットする
	void resetImpl();
	// 次の値を得る
	ModSize nextImpl(int& length_);
	// 文書内頻度を得る
	ModSize getTermFrequencyImpl();
	
	// 位置情報と位置の配列
	LocationVector	m_cVector;
	// 下限
	ModSize			m_uiLower;
	// 上限
	ModSize			m_uiUpper;

	// 現在位置
	ModSize			m_uiCurrentLocation;
	// 現在の長さ
	int				m_iCurrentLength;
	// 現在の最小位置と長さ
	Data			m_cMinLocation;
	// 現在の最大位置と長さ
	Data			m_cMaxLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
