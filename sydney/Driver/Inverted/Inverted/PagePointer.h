// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PagePointer.h --
// 
// Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_PAGEPOINTER_H
#define __SYDNEY_INVERTED_PAGEPOINTER_H

#include "Inverted/Module.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	TEMPLATE CLASS
//	Inverted::PageObjectPointer
//
//	NOTES
//	ページの参照カウンタを管理するポインタクラス
//
template <class PAGE>
class PageObjectPointer
{
public:
	// コンストラクタ
	PageObjectPointer(PAGE* pPage_ = 0)
		: m_pPage(pPage_)
	{
		increment();
	}

	// コピーコンストラクタ
	PageObjectPointer(const PageObjectPointer& cPointer_)
		: m_pPage(cPointer_.get())
	{
		increment();
	}

	// デストラクタ
	~PageObjectPointer()
	{
		decrement();
	}

	// 代入演算子
	PageObjectPointer& operator = (const PageObjectPointer& cPointer_)
	{
		if (get() != cPointer_.get())
		{
			decrement();
			m_pPage = cPointer_.get();
			increment();
		}
		return *this;
	}
	PageObjectPointer& operator = (PAGE* pPage_)
	{
		if (get() != pPage_)
		{
			decrement();
			m_pPage = pPage_;
			increment();
		}
		return *this;
	}

	// ページを得る
	PAGE* get() const { return const_cast<PAGE*>(m_pPage); }

	// オペレータ
	operator bool () const { return get() != 0; }
	operator PAGE* () { return get(); }
	PAGE* operator -> () { return get(); }
	PAGE& operator * () { return *get(); }
	bool operator == (int i) { return get() == syd_reinterpret_cast<PAGE*>(i); }
	bool operator == (PAGE* pPage_) { return get() == pPage_; }
	bool operator == (const PageObjectPointer& cPointer_) { return get() == cPointer_.get(); }
	bool operator != (int i) { return get() != syd_reinterpret_cast<PAGE*>(i); }
	bool operator != (PAGE* pPage_) { return get() != pPage_; }
	bool operator != (const PageObjectPointer& cPointer_) { return get() != cPointer_.get(); }

private:
	// 参照カウンタを増やす
	void increment()
	{
		if (m_pPage) m_pPage->incrementReference();
	}

	// 参照カウンタを減らす
	void decrement()
	{
		if (m_pPage)
		{
			int ref = m_pPage->decrementReference();
			if (ref == 0)
				m_pPage->detach();
		}
	}

	// ページ
	PAGE* m_pPage;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_PAGEPOINTER_H

//
//	Copyright (c) 2000, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
