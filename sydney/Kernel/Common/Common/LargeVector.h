// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LargeVector.h -- 大量のデータが挿入されても大丈夫なvector
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_LARGEVECTOR_H
#define __TRMEISTER_COMMON_LARGEVECTOR_H

#include "Common/Module.h"

#include "ModOsDriver.h"
#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	TEMPLATE CLASS
//	Common::LargeVector -- ベクターを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//	class _V_
//		ベクターに登録する値の型
//
//	NOTES
//	このベクターは数千万エントリを登録しても大丈夫なように手を加えてある。
//	連続領域で確保しないので、ポインタで取り出すことはできない。
//	ModVectorのインターフェース仕様を踏襲している。
//
template <class _V_>
class LargeVector
{
public:
	enum { MAX_SIZE = 64 << 10 };	// 64KB
	enum { MAX = MAX_SIZE / sizeof(_V_) };

	class Iterator;
	
	//
	//	CLASS
	//	Common::LargeVector<_V_>::ConstIterator -- 反復子
	//
	class ConstIterator
	{
		friend class LargeVector<_V_>;
		friend class Iterator;
		
	public:
		typedef _V_ ValueType;
		typedef int DifferenceType;
		typedef _V_* Pointer;
		typedef const _V_* ConstPointer;
		typedef _V_& Reference;
		typedef const _V_& ConstReference;
		typedef LargeVector<_V_> Container;

		// コンストラクタ(1)
		ConstIterator()
			: m_ppArray(0), m_iElement(0), m_pCur(0), m_pEnd(0) {}
		// コンストラクタ(2)
		ConstIterator(_V_** ppArray_, ModSize uiPosition_)
			: m_ppArray(ppArray_)
			{
				setPosition(static_cast<int>(uiPosition_));
			}
		// コピーコンストラクタ
		ConstIterator(const ConstIterator& other)
			: m_ppArray(other.m_ppArray), m_iElement(other.m_iElement)
			{
				m_pCur = other.m_pCur;
				m_pEnd = other.m_pEnd;
			}
		// デストラクタ
		~ConstIterator() {}

		// * 単項演算子
		ConstReference operator *() const
		{
			return *m_pCur;
		}

		// ++ 前置演算子
		ConstIterator& operator ++ ()
		{
			++m_pCur;
			if (m_pCur == m_pEnd)
			{
				m_pCur = m_ppArray[++m_iElement];
				m_pEnd = m_pCur + MAX;
			}
			return *this;
		}
		// ++ 後置演算子
		ConstIterator operator ++ (int dummy)
		{
			ConstIterator save(*this);
			operator ++();
			return save;
		}
		// -- 前置演算子
		ConstIterator& operator -- ()
		{
			if (m_pCur == (m_pEnd - Container::MAX))
			{
				m_pCur = m_ppArray[--m_iElement] + Container::MAX;
				m_pEnd = m_pCur;
			}
			--m_pCur;
			return *this;
		}
		// -- 後置演算子
		ConstIterator operator -- (int dummy)
		{
			ConstIterator save(*this);
			operator --();
			return save;
		}

		// = 演算子
		ConstIterator& operator = (const ConstIterator& other)
		{
			m_ppArray = other.m_ppArray;
			m_iElement = other.m_iElement;
			m_pCur = other.m_pCur;
			m_pEnd = other.m_pEnd;
			return *this;
		}

		// + 演算子
		DifferenceType operator + (const ConstIterator& last) const
		{
			return getPosition() + last.getPosition();
		}
		ConstIterator operator + (DifferenceType distance) const
		{
			ConstIterator save(*this);
			save.operator +=(distance);
			return save;
		}
		// - 演算子
		DifferenceType operator - (const ConstIterator& last) const
		{
			return getPosition() - last.getPosition();
		}
		ConstIterator operator - (DifferenceType distance) const
		{
			ConstIterator save(*this);
			save.operator -= (distance);
			return save;
		}

		// += 演算子
		ConstIterator& operator += (DifferenceType distance)
		{
			setPosition(getPosition() + distance);
			return *this;
		}
		// -= 演算子
		ConstIterator& operator -= (DifferenceType distance)
		{
			setPosition(getPosition() - distance);
			return *this;
		}

		// == 演算子
		bool operator == (const ConstIterator& other) const
		{
			return m_pCur == other.m_pCur;
		}
		// != 演算子
		bool operator != (const ConstIterator& other) const
		{
			return m_pCur != other.m_pCur;
		}

		// < 演算子
		bool operator < (const ConstIterator& other) const
		{
			return (m_iElement < other.m_iElement ||
					m_iElement == other.m_iElement && m_pCur < other.m_pCur);
		}
		// < 演算子
		bool operator <= (const ConstIterator& other) const
		{
			return (m_iElement < other.m_iElement ||
					m_iElement == other.m_iElement && m_pCur <= other.m_pCur);
		}
		// > 演算子
		bool operator > (const ConstIterator& other) const
		{
			return (m_iElement > other.m_iElement ||
					m_iElement == other.m_iElement && m_pCur > other.m_pCur);
		}
		// > 演算子
		bool operator >= (const ConstIterator& other) const
		{
			return (m_iElement > other.m_iElement ||
					m_iElement == other.m_iElement && m_pCur >= other.m_pCur);
		}

		// 有効かどうか
		bool isValid() const { return m_ppArray; }
		
	private:
		// 設定する
		void setPosition(int pos)
			{
				m_iElement = pos / Container::MAX;
				if (m_ppArray)
				{
					m_pCur = m_ppArray[m_iElement] + (pos % Container::MAX);
					m_pEnd = m_ppArray[m_iElement] + Container::MAX;
				}
				else
				{
					m_pCur = m_pEnd = 0;
				}
			}
		// 位置を得る
		int getPosition() const
			{
				int p = m_iElement * Container::MAX;
				if (m_pCur)
				{
					p += static_cast<int>(m_pCur - m_ppArray[m_iElement]);
				}
				return p;
			}
		
		// メモリ
		_V_** m_ppArray;
		// 今参照している要素番号
		int m_iElement;
		// 今参照している要素
		_V_* m_pCur;
		// 今参照している要素の終端
		_V_* m_pEnd;
	};

	//
	//	CLASS
	//	Common::LargeVector<_V_>::Iterator -- 反復子
	//
	class Iterator
		: public ConstIterator
	{
		typedef ConstIterator	Super;
		
	public:
		// コンストラクタ(1)
		Iterator() {}
		// コンストラクタ(2)
		Iterator(_V_** ppArray_, ModSize uiPosition)
			: ConstIterator(ppArray_, uiPosition) {}
		// コピーコンストラクタ
		Iterator(const Iterator& other)
			: ConstIterator(other) {}
		// デストラクタ
		~Iterator() {}

		// * 単項演算子
		typename Super::Reference operator *() const
		{
			return *Super::m_pCur;
		}

		// ++ 前置演算子
		Iterator& operator ++ ()
		{
			Super::operator ++ ();
			return *this;
		}
		// ++ 後置演算子
		Iterator operator ++ (int dummy)
		{
			Iterator save(*this);
			Super::operator ++ ();
			return save;
		}
		// -- 前置演算子
		Iterator& operator -- ()
		{
			Super::operator -- ();
			return *this;
		}
		// -- 後置演算子
		Iterator operator -- (int dummy)
		{
			Iterator save(*this);
			Super::operator -- ();
			return save;
		}

		// = 演算子
		Iterator& operator = (const Iterator& other)
		{
			Super::operator = (other);
			return *this;
		}

		// + 演算子
		typename Super::DifferenceType operator + (const Iterator& last) const
		{
			return Super::operator + (last);
		}
		Iterator operator + (typename Super::DifferenceType distance) const
		{
			Iterator save(*this);
			save.operator +=(distance);
			return save;
		}
		// - 演算子
		typename Super::DifferenceType operator - (const Iterator& last) const
		{
			return Super::operator - (last);
		}
		Iterator operator - (typename Super::DifferenceType distance) const
		{
			Iterator save(*this);
			save.operator -= (distance);
			return save;
		}

		// += 演算子
		Iterator& operator += (typename Super::DifferenceType distance)
		{
			Super::operator += (distance);
			return *this;
		}
		// -= 演算子
		Iterator& operator -= (typename Super::DifferenceType distance)
		{
			Super::operator -= (distance);
			return *this;
		}
	};

	// コンストラクタ
	LargeVector()
		: m_ppArray(0), m_uiArraySize(0),
		  m_uiSize(0), m_uiCapacity(0) {}
		
	LargeVector(ModSize n, const _V_& value = _V_())
		: m_ppArray(0), m_uiArraySize(0),
		  m_uiSize(0), m_uiCapacity(0)
	{
		assign(n, value);
	}
	
	// コピーコンストラクタ
	LargeVector(const LargeVector<_V_>& src)
		: m_ppArray(0), m_uiArraySize(0),
		  m_uiSize(0), m_uiCapacity(0)
	{
		assign(src.begin(), src.end());
	}

	// デストラクタ
	virtual ~LargeVector()
	{
		clear();
	}

	// 代入演算子
	LargeVector<_V_>& operator = (const LargeVector<_V_>& src)
	{
		assign(src.begin(), src.end());
		return *this;
	}

	// 値を割り当てる
	void assign(const ConstIterator& first, const ConstIterator& last)
	{
		if (!isEmpty())
			erase(begin(), end());
		insert(end(), first, last);
	}
	void assign(ModSize n, const _V_& value = _V_())
	{
		if (!isEmpty())
			erase(begin(), end());
		reserve(n);
		int i = 0;
		ModSize save_n = n;
		while (n)
		{
			ModSize len = (n < MAX) ? n : MAX;
			uninitializedCopy(value, len, m_ppArray[i++]);
			n -= len;
		}
		m_uiSize = save_n;
	}

	// 空か調べる
	bool isEmpty() const { return m_uiSize == 0; }

	// ある位置へ値を挿入する
	Iterator insert(const Iterator& position, const _V_& value = _V_())
	{
		ModSize p = position.getPosition();
		
		reserve(m_uiSize + 1);
		// 挿入位置以降を後ろへ移動する
		moveBackward(p, 1);
		// コピーする
		_V_* d = m_ppArray[p / MAX] + (p % MAX);
		uninitializedCopy(value, d);
		// サイズを増やす
		m_uiSize += 1;

		// メモリが再確保されているかも知れない
		return Iterator(m_ppArray, p);
	}
	void insert(const Iterator& position,
				const ConstIterator& first, const ConstIterator& last)
	{
		ModSize p = position.getPosition();
		
		ModSize len = last - first;
		reserve(m_uiSize + len);
		// 挿入位置以降を後ろへ移動する
		moveBackward(p, len);
		// コピーする
		uninitializedCopy_range(first, last, p);
		// サイズを増やす
		m_uiSize += len;
	}

	// 先頭に値を挿入する
	void pushFront(const _V_& value)
	{
		insert(begin(), value);
	}
	// 末尾に値を挿入する
	void pushBack(const _V_& value)
	{
		reserve(m_uiSize + 1);
		// コピーする
		uninitializedCopy(value,
						  m_ppArray[m_uiSize / MAX] + (m_uiSize % MAX));
		// サイズを増やす
		m_uiSize += 1;
	}

	// 格納領域を確保する
	void reserve(ModSize n)
	{
		if (m_uiCapacity < n)
		{
			if (n < MAX)
			{
				// MAXサイズ未満の場合、領域は倍々に増やしていく

				if (m_uiCapacity == 0) m_uiCapacity = 1;
				while (m_uiCapacity < n)
					m_uiCapacity <<= 1;	// 2倍にする
				if (m_uiCapacity > MAX) m_uiCapacity = MAX;

				if (m_uiArraySize == 0)
				{
					// 常に1つ余計に確保する
					m_ppArray = new _V_* [2];
					ModOsDriver::Memory::reset(m_ppArray, sizeof(_V_*) * 2);
					m_uiArraySize = 1;
				}
				// 領域を再確保する
				m_ppArray[0] = reallocate(m_uiCapacity, m_ppArray[0], m_uiSize);
			}
			else
			{
				// MAXサイズ以上の場合は、MAXの倍数にする

				// 配列格納領域を再確保する
				// 常に1つ余計に確保する
				ModSize s = ((n - 1) / MAX) + 2;
				_V_** p = new _V_* [s];
				ModOsDriver::Memory::reset(p, sizeof(_V_*) * s);
				if (m_uiArraySize)
				{
					ModOsDriver::Memory::copy(p, m_ppArray,
											  m_uiArraySize * sizeof(_V_*));
					delete [] m_ppArray;
				}

				m_ppArray = p;
				m_uiArraySize = (s - 1);
				
				if (m_uiCapacity < MAX)
				{
					// 領域を再確保する
					m_ppArray[0] = reallocate(MAX, m_ppArray[0], m_uiSize);
					m_uiCapacity = MAX;
				}
				// 格納領域を確保する
				for (ModSize i = (m_uiCapacity / MAX); i < m_uiArraySize; ++i)
					m_ppArray[i] = allocate(MAX);
				
				m_uiCapacity = m_uiArraySize * MAX;
			}
		}
	}

	// 削除する
	Iterator erase(const Iterator& position)
	{
		ModSize p = position.getPosition();
		destruct(m_ppArray[p / MAX] + (p % MAX));
		// 削除位置以降を前へ移動する
		moveForward(p + 1, 1);
		m_uiSize -= 1;
				
		return position;
	}
	Iterator erase(const Iterator& first, const Iterator& last)
	{
		Iterator i = first;
		for (; i != last; ++i)
			destruct(&(*i));
		// 削除位置以降を前へ移動する
		ModSize d = last - first;
		moveForward(last.getPosition(), d);
		m_uiSize -= d;

		return first;
	}

	// 先頭の要素を削除する
	void popFront()
	{
		erase(begin());
	}
	// 末尾の要素を削除する
	void popBack()
	{
		erase(end()-1);
	}

	// 全要素とメモリー領域を破棄する
	void clear()
	{
		for (ModSize i = 0; i < m_uiArraySize; ++i)
		{
			if (m_uiSize < MAX)
			{
				destruct_range(m_ppArray[i], m_uiSize);
				m_uiSize = 0;
			}
			else
			{
				destruct_range(m_ppArray[i], MAX);
				m_uiSize -= MAX;
			}
			deallocate(m_ppArray[i]);
		}
		
		m_uiSize = 0;
		m_uiCapacity = 0;
		m_uiArraySize = 0;
		delete [] m_ppArray;
		m_ppArray = 0;
	}

	// 先頭の要素の値を得る
	_V_& getFront() { return at(0); }
	const _V_& getFront() const	{ return at(0); }
	// 最後の要素の値を得る
	_V_& getBack() { return at(m_uiSize - 1); }
	const _V_& getBack() const { return at(m_uiSize - 1); }

	// 先頭から指定された数番目の要素の値を得る
	_V_& at(ModSize i)
	{
		return *(m_ppArray[i / MAX] + (i % MAX));
	}
	const _V_& at(ModSize i) const
	{
		return *(m_ppArray[i / MAX] + (i % MAX));
	}

	// []演算子
	_V_& operator [](ModSize i) { return at(i); }
	const _V_& operator [](ModSize i) const { return at(i); }

	// 先頭の要素を指す反復子を得る
	Iterator begin() { return Iterator(m_ppArray, 0); }
	ConstIterator begin() const { return ConstIterator(m_ppArray, 0); }
	// 終端を指す反復子を得る
	Iterator end() { return Iterator(m_ppArray, m_uiSize); }
	ConstIterator end() const { return ConstIterator(m_ppArray, m_uiSize); }

	// 引数と一致する要素を指す反復子を得る
	Iterator find(const _V_& value_)
	{
		Iterator iterator = begin();
		const Iterator last = end();
		for (; iterator != last; ++iterator) {
			if ((*iterator) == value_) return iterator;
		}
		return last;
	}
	ConstIterator find(const _V_& value_) const
	{
		ConstIterator iterator = begin();
		const ConstIterator last = end();
		for (; iterator != last; ++iterator) {
			if ((*iterator) == value_) return iterator;
		}
		return last;
	}

	// 要素数を得る
	ModSize getSize() const { return m_uiSize; }
	// 格納可能な要素数を得る
	ModSize getCapacity() const { return m_uiCapacity; }

private:
	// メモリーを後ろに移動する
	void moveBackward(ModSize position, ModSize distance)
	{
		//	----****** を ---------- とかに移動する
		//	***-------    ------****
		//	----------    *****-----

		ModSize sf = position;
		ModSize sl = m_uiSize;
		ModSize df = sf + distance;
		ModSize dl = sl + distance;

		while (sf != sl)
		{
			ModSize s = sl % MAX;
			if (s == 0) s = MAX;
			ModSize d = dl % MAX;
			if (d == 0) d = MAX;
		
			ModSize len = (s < d) ?
				(s - ((sf / MAX == (sl - s) / MAX) ?
					  (sf % MAX) : 0)) :
				(d - ((df / MAX == (dl - d) / MAX) ?
					  (df % MAX) : 0));

			sl -= len;
			dl -= len;
		
			ModOsDriver::Memory::move(
				m_ppArray[dl / MAX] + dl % MAX,
				m_ppArray[sl / MAX] + sl % MAX,
				len * sizeof(_V_));
		}
	}
	
	// メモリーを前に移動する
	void moveForward(ModSize position, ModSize distance)
	{
		//	---------- を ----****** とかに移動する
		//	------****    ***-------
		//	*****-----    ----------

		ModSize sf = position;
		ModSize sl = m_uiSize;
		ModSize df = sf - distance;
		ModSize dl = sl - distance;

		while (sf != sl)
		{
			ModSize s = sf % MAX;
			ModSize d = df % MAX;
	
			ModSize len = (s > d) ?
				((((sl / MAX) == (sf / MAX)) ? (sl % MAX) : MAX)
				 - s) :
				((((dl / MAX) == (df / MAX)) ? (dl % MAX) : MAX)
				 - d);
			
			ModOsDriver::Memory::move(
				m_ppArray[df / MAX] + df % MAX,
				m_ppArray[sf / MAX] + sf % MAX,
				len * sizeof(_V_));

			sf += len;
			df += len;
		}
	}
	
	// 初期化されていない領域にコピーする
	void uninitializedCopy_range(ConstIterator first,
								 const ConstIterator& last,
								 ModSize position)
	{
		Iterator dist = begin();
		dist += position;
		Iterator next = dist;
		try	{
			for (; first != last; ++dist, ++first)
				construct(&(*dist), *first);
		} catch (...) {
			for (; next != dist; ++next)
				destruct(&(*next));
			throw;
		}
	}
	void uninitializedCopy(const _V_& src, ModSize n, _V_* dist)
	{
		_V_* next = dist;
		try {
			for (ModSize i = 0; i != n; ++dist, ++i)
				construct(dist, src);
		} catch (...) {
			for (; next != dist; ++next)
				destruct(next);
			throw;
		}
	}
	void uninitializedCopy(const _V_& src, _V_* dist)
	{
		construct(dist, src);
	}

	// コンストラクタを呼び出す
	void construct(_V_* p, const _V_& src)
	{
		::new (p) _V_(src);
	}
	// デストラクタを呼び出す
	void destruct(_V_* p)
	{
		p->~_V_();
	}

	// ある領域のデストラクタを呼び出す
	void destruct_range(_V_* p, ModSize len)
	{
		for (ModSize i = 0; i < len; ++i, ++p)
			destruct(p);
	}

	// 領域を再確保する
	_V_* reallocate(ModSize count, _V_* src, ModSize scount)
	{
		_V_* p = allocate(count);
		if (src)
		{
			ModOsDriver::Memory::copy(p, src, scount * sizeof(_V_));
			deallocate(src);
		}
		return p;
	}
	
	// 領域を確保する
	_V_* allocate(ModSize count)
	{
		return (_V_*)(operator new (sizeof(_V_) * count));
	}
	// 領域を開放する
	void deallocate(_V_* p)
	{
		operator delete (p);
	}
	
	// 要素配列の配列(常に+1個確保する)
	_V_** m_ppArray;
	// 要素配列の配列の長さ
	ModSize m_uiArraySize;
	
	// トータルの格納領域(要素数)
	ModSize m_uiSize;
	// トータルの格納可能領域(要素数)
	ModSize m_uiCapacity;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION

//	ModSortとかに必要
template <class _V_>
inline
_V_*
ModValueType(
	typename _TRMEISTER::Common::LargeVector<_V_>::Iterator& dummy)
{
	return static_cast<_V_*>(0);
}

#endif

#endif //__TRMEISTER_COMMON_LARGEVECTOR_H

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
