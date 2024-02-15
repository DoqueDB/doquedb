// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationListItertor.h --
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

#ifndef __SYDNEY_FULLTEXT2_LOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_LOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ListIterator;
class LocationListManager;

//
//	CLASS
//	FullText2::LocationListIterator
//		-- 検索時に位置情報を走査するクラスの基底クラス
//
//	NOTES
//
class LocationListIterator
{
	friend class LocationListManager;
	
public:
	//
	//	CLASS
	//	FullText2::LocationListIterator::AutoPointer -- 自動ポインタ
	//
	//	NOTES
	//
	class AutoPointer
	{
	public:
		// コンストラクタ
		AutoPointer(LocationListIterator* p_ = 0)
			: m_bOwner(p_ ? true : false), m_pPointer(p_) {}
		// コピーコンストラクタ
		AutoPointer(const AutoPointer& c_)
			: m_bOwner(c_.m_bOwner), m_pPointer(c_.release()) {}
		// デストラクタ
		~AutoPointer() { expunge(); }

		// アクセッサ
		LocationListIterator* get() const
			{ return m_pPointer; }
		LocationListIterator* release() const
			{ m_bOwner = false; return m_pPointer; }
		LocationListIterator& operator *() const
			{ return *get(); }
		LocationListIterator* operator ->() const
			{ return get(); }
		operator LocationListIterator* ()
			{ return get(); }
		operator const LocationListIterator* () const
			{ return get(); }

		// 代入演算子
		AutoPointer& operator =(const AutoPointer& c_)
			{
				expunge();
				m_bOwner = c_.m_bOwner;
				m_pPointer = c_.release();
				return *this;
			}
		AutoPointer& operator =(LocationListIterator* p_)
			{
				expunge();
				m_bOwner = p_ ? true : false;
				m_pPointer = p_;
				return *this;
			}
		
	private:
		// 解放する
		void expunge()
			{
				if (m_bOwner == false)
					return;
				if (m_pPointer->release() == false)
					delete m_pPointer;
				
				m_bOwner = false;
				m_pPointer = 0;
			}
		
		mutable bool			m_bOwner;
		LocationListIterator*	m_pPointer;
	};
	
	// コンストラクタ
	LocationListIterator(LocationListManager* pListManager_);
	// デストラクタ
	virtual ~LocationListIterator();

	// 位置情報を検索する
	virtual bool find(ModSize location_, int& length_) = 0;
	virtual ModSize lowerBound(ModSize location_, int& length_) = 0;
	virtual ModSize lowerBound(ModSize location_,
							   int& minLength_, int& maxLength_)
		{
			ModSize pos = lowerBound(location_, maxLength_);
			minLength_ = maxLength_;
			return pos;
		}
	// カーソルを先頭に戻す
	virtual void reset() = 0;
	// 次の値を得る
	virtual ModSize next(int& length_) = 0;

	// 解放する
	//	true:	フリーリストにつなげた
	//	false:	呼び出し側で delete 必要
	virtual bool release();
	// 文書内頻度を得る
	virtual ModSize getTermFrequency() = 0;

protected:
	LocationListManager* m_pListManager;

private:
	// フリーリスト
	LocationListIterator* m_pNext;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
