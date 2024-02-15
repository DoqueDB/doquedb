// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortLeafLocationListItertor.h --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SHORTLEAFLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_SHORTLEAFLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class LeafNode;
class ShortLeafNode;
class ShortLeafNodeCompatible;
class AtomicOrLeafNode;

//
//	CLASS
//	FullText2::ShortLeafLocationListIterator
//		-- 検索時に位置情報を走査するクラス
//
//	NOTES
//
class ShortLeafLocationListIterator : public LocationListIterator
{
	friend class ShortLeafNode;
	friend class ShortLeafNodeCompatible;
	friend class AtomicOrLeafNode;
	
public:
	// コンストラクタ
	ShortLeafLocationListIterator(LeafNode& cNode_,
								  int length_, ModSize reserve_);
	// デストラクタ
	virtual ~ShortLeafLocationListIterator();

	// 位置情報を検索する
	bool find(ModSize location_, int& length_)
		{
			int dummy;
			if (lowerBoundImpl(location_, dummy, length_) == location_)
				return true;
			return false;
		}
	ModSize lowerBound(ModSize location_, int& length_)
		{
			int dummy;
			return lowerBoundImpl(location_, dummy, length_);
		}
	ModSize lowerBound(ModSize location_, int& minLength_, int& maxLength_)
		{
			return lowerBoundImpl(location_, minLength_, maxLength_);
		}
	// カーソルを先頭に戻す
	void reset() { resetImpl(); }
	// 次の値を得る
	ModSize next(int& length_)
		{
			int dummy;
			return nextImpl(dummy, length_);
		}
	ModSize next(int& minLength_, int& maxLength_)
		{
			return nextImpl(minLength_, maxLength_);
		}

	// 解放する
	bool release();
	// 文書内頻度を得る
	ModSize getTermFrequency()
		{
			return getTermFrequencyImpl();
		}

	// 位置情報へのイテレータを追加する
	void pushBack(LocationListIterator::AutoPointer i);

private:
	struct Data
	{
		Data() : loc(0), len(0) {}
		
		ModSize	loc;	// 位置情報
		int		len;	// 検索語長
	};

	typedef ModPair<Data, LocationListIterator::AutoPointer>	LocationPair;
	typedef ModVector<LocationPair>								LocationVector;

	// 位置情報を検索する
	ModSize lowerBoundImpl(ModSize location_, int& minLength_, int& maxLength_);
	// カーソルをリセットする
	void resetImpl();
	// 次の値を得る
	ModSize nextImpl(int& minLength_, int& maxLength_);
	// 文書内頻度を得る
	ModSize getTermFrequencyImpl();
	
	// 位置情報と位置の配列
	LocationVector	m_cVector;
	// 全長
	int				m_iLength;

	// 現在位置
	ModSize			m_uiCurrentLocation;
	// 現在の長さ
	int				m_iCurrentMinLength;
	int				m_iCurrentMaxLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTLEAFLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
