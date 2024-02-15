// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoPointer.h --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_AUTOPOINTER_H
#define __SYDNEY_LOB_AUTOPOINTER_H

#include "Lob/Module.h"
#include "ModDefaultManager.h"
#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	TEMPLATE CLASS
//	Lob::AutoPointer
//
//	NOTES
//	ModDefaultManager::allocateで確保されたメモリーを解放するauto_pointer
//
template <class TYPE>
class AutoPointer
{
public:
	// コンストラクタ
	AutoPointer(TYPE* pPointer_ = 0, ModSize uiSize_ = 0)
		: m_bOwner(pPointer_ ? true : false), m_pPointer(pPointer_), m_uiSize(uiSize_)
	{
	}

	// コピーコンストラクタ
	AutoPointer(const AutoPointer& cAutoPointer_)
		: m_bOwner(cAutoPointer_.m_bOwner), m_pPointer(cAutoPointer_.release()),
		  m_uiSize(cAutoPointer_.m_uiSize)
	{
	}

	// デストラクタ
	~AutoPointer()
	{
		freeMemory();
	}

	// サクセッサ
	TYPE* get() const { return m_pPointer; }
	TYPE* release() const { m_bOwner = false; return m_pPointer; }
	operator TYPE* () { return get(); }
	operator const TYPE* () const { return get(); }

	// 代入演算子たち
	// 本当はクラス内テンプレート関数を使用したいが、VC6はバグっているので、
	// 使用するすべての型の代入演算子を定義する。
	AutoPointer<TYPE>&
	operator =(const AutoPointer<char>& cAutoPointer_)
	{
		freeMemory();
		m_bOwner = cAutoPointer_.m_bOwner;
		m_pPointer = syd_reinterpret_cast<TYPE*>(cAutoPointer_.release());
		m_uiSize = cAutoPointer_.m_uiSize;
		return *this;
	}
	
	AutoPointer<TYPE>&
	operator =(const AutoPointer<void>& cAutoPointer_)
	{
		freeMemory();
		m_bOwner = cAutoPointer_.m_bOwner;
		m_pPointer = syd_reinterpret_cast<TYPE*>(cAutoPointer_.release());
		m_uiSize = cAutoPointer_.m_uiSize;
		return *this;
	}
	
	AutoPointer<TYPE>&
	operator =(const AutoPointer<ModUnicodeChar>& cAutoPointer_)
	{
		freeMemory();
		m_bOwner = cAutoPointer_.m_bOwner;
		m_pPointer = syd_reinterpret_cast<TYPE*>(cAutoPointer_.release());
		m_uiSize = cAutoPointer_.m_uiSize;
		return *this;
	}
	
	mutable bool m_bOwner;
	TYPE* m_pPointer;
	ModSize m_uiSize;

private:
	void freeMemory()
	{
		if (m_bOwner)
		{
			ModDefaultManager::free(m_pPointer, m_uiSize);
		}
	}
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_AUTOPOINTER_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
