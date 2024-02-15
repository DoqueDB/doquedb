// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalLocationListIterator.h -- 位置情報リストを操作するイテレータ
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

#ifndef __SYDNEY_FULLTEXT2_NORMALLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_NORMALLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

#include "Common/LargeVector.h"

#include "ModAlgorithm.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::NormalLocationListIterator --
//
//	NOTES
//
class NormalLocationListIterator : public LocationListIterator
{
	typedef Common::LargeVector<ModSize>	Vector;
public:
	// コンストラクタ
	NormalLocationListIterator(const Vector& list_, ModSize length_)
		: LocationListIterator(0),
		  m_cList(list_), m_uiLength(length_)
		{
			m_ite = m_cList.begin();
		}
		
	// デストラクタ
	virtual ~NormalLocationListIterator() {}

	// 位置情報を検索する
	bool find(ModSize location_, int& length_)
		{
			if (lowerBound(location_, length_) == location_)
				return true;
			return false;
		}
	ModSize lowerBound(ModSize location_, int& length_)
		{
			m_ite = ModLowerBound(m_cList.begin(),
								  m_cList.end(),
								  location_,
								  ModLess<ModSize>());
			if (m_ite != m_cList.end())
			{
				location_ = *m_ite;
				length_ = m_uiLength;
			}
			else
			{
				location_ = UndefinedLocation;
			}
			return location_;
		}
		
	// カーソルを先頭に戻す
	void reset()
		{
			m_ite = m_cList.begin();
		}
		
	// 次の値を得る
	ModSize next(int& length_)
		{
			ModSize location = UndefinedLocation;
			if (m_ite != m_cList.end())
			{
				length_ = m_uiLength;
				location = *m_ite;
				++m_ite;
			}
			return location;
		}

	// 文書内頻度を得る
	ModSize getTermFrequency() { return m_cList.getSize(); }
	   
private:
	// ベクター
	const Vector& m_cList;
	// イテレータ
	Vector::ConstIterator m_ite;
	// トークン長
	ModSize m_uiLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NORMALLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
