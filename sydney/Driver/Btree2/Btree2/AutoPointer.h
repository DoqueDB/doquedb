// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoPointer.h --
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_AUTOPOINTER_H
#define __SYDNEY_BTREE2_AUTOPOINTER_H

#include "Btree2/Module.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	TEMPLATE CLASS
//	Btree2::AutoPointer
//
//	NOTES
//	Os::Memory::allocate で確保されたメモリーを解放するauto_pointer
//
template <class TYPE>
class AutoPointer
{
public:
	// コンストラクタ
	AutoPointer(TYPE* pPointer_ = 0)
		: m_bOwner(pPointer_ ? true : false), m_pPointer(pPointer_)
	{
	}

	// コピーコンストラクタ
	AutoPointer(const AutoPointer<TYPE>& cAutoPointer_)
		: m_bOwner(cAutoPointer_.m_bOwner), m_pPointer(cAutoPointer_.release())
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
	TYPE& operator *() const { return *get(); }
//	TYPE* operator ->() const { return get(); }
	operator TYPE* () { return get(); }
	operator const TYPE* () const { return get(); }

	// 代入演算子
	AutoPointer<TYPE>& operator =(const AutoPointer& cAutoPointer_)
	{
		freeMemory();
		m_bOwner = cAutoPointer_.m_bOwner;
		m_pPointer = cAutoPointer_.release();
		return *this;
	}
	AutoPointer<TYPE>& operator =(TYPE* pPointer_)
	{
		freeMemory();
		m_bOwner = pPointer_ ? true : false;
		m_pPointer = pPointer_;
		return *this;
	}

private:
	void freeMemory()
	{
		if (m_bOwner)
		{
			void* p = m_pPointer;
			Os::Memory::free(p);
		}
	}

	mutable bool m_bOwner;
	TYPE* m_pPointer;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_AUTOPOINTER_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
