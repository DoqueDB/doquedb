// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorMap.h -- 順序つきVectorを用いたMap
// 
// Copyright (c) 2004, 2005, 2006, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_VECTORMAP_H
#define __TRMEISTER_COMMON_VECTORMAP_H

#include "Common/Object.h"
#include "Common/LargeVector.h"
#include "Exception/BadArgument.h"
#include "ModAlgorithm.h"
#include "ModPair.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	TEMPLATE CLASS
//	Common::VectorMap -- 順序つきVectorを用いたMap
//
//	TEMPLATE ARGUMENTS
//	class _K_
//		Mapのキーの型
//	class _V_
//		Mapのバリューの型
//	class _C_
//		キーに対する比較関数を表すオブジェクト
//
//	NOTES
//	指定された比較関数で指定される順序で並んでいるVectorに対して
//	二分探索によりMap操作を行う
//
//	これまでModVectorにValueTypeをnewしたものを格納していたが、LargeVectorが
//	高速化されたので、LargeVectorにValueTypeの実体を格納するように修正した
//
template <class _K_, class _V_, class _C_>
class VectorMap
	: private LargeVector< ModPair<_K_, _V_> >
{
public:
	typedef ModPair<_K_, _V_> Entry;
	typedef LargeVector<Entry> Super;
	typedef VectorMap<_K_, _V_, _C_> This;

	typedef _K_ KeyType;
	typedef _V_ MappedValueType;
	typedef Entry ValueType;
	typedef _C_ Comparator;

	typedef typename Super::Iterator Iterator;
	typedef typename Super::ConstIterator ConstIterator;

	// 親クラスの実装をそのまま使うメソッドの宣言
	using Super::isEmpty;
	using Super::getSize;
	using Super::reserve;

	// コンストラクター
	VectorMap();
	VectorMap(const This& cOther_);
	// デストラクター
	~VectorMap();

	Iterator begin() {return Iterator(Super::begin());}
	ConstIterator begin() const {return ConstIterator(Super::begin());}
	Iterator end() {return Iterator(Super::end());}
	ConstIterator end() const {return ConstIterator(Super::end());}

	// キーにマッチしたバリューを指すイテレーターを得る
	ConstIterator find(const _K_& cKey_) const;
	Iterator find(const _K_& cKey_);
	
	// 下限検索
	ConstIterator lowerBound(const _K_& cKey_) const;
	Iterator lowerBound(const _K_& cKey_);
	// 上限検索
	ConstIterator upperBound(const _K_& cKey_) const;
	Iterator upperBound(const _K_& cKey_);

	// エントリーを挿入する
	ModPair<Iterator, ModBoolean> insert(const ValueType& cValue_);
	ModPair<Iterator, ModBoolean> insert(const _K_& cKey_, const _V_& cValue_);
	void insert(ConstIterator first, const ConstIterator& last);
	void insert(Iterator first, const Iterator& last);

	// 指定した位置のオブジェクトを消す
	Iterator erase(const Iterator& position);
	Iterator erase(const Iterator& first, const Iterator& last);
	// キーにマッチしたものを消す
	int erase(const _K_& cKey_);

	// キーにマッチしたバリューへの参照を得る
	_V_& operator[](const _K_& cKey_);
	const _V_& operator[](const _K_& cKey_) const;

	// clearの実装を上書きする
	void clear();

	// operator=の実装を上書きする
	This& operator=(const This& cOther_);

	class Compare
	{
	public:
		Compare() : comp() {}

		bool operator()(const Entry& cEntry1_, const Entry& cEntry2_) const
		{
			return comp(cEntry1_.first, cEntry2_.first);
		}
		bool operator()(const _K_& cKey_, const Entry& cEntry_) const
		{
			return comp(cKey_, cEntry_.first);
		}
		bool operator()(const Entry& cEntry_, const _K_& cKey_) const
		{
			return comp(cEntry_.first, cKey_);
		}
		bool operator()(const _K_& cKey1_, const _K_& cKey2_) const
		{
			return comp(cKey1_, cKey2_);
		}
	private:
		_C_ comp;
	};
private:
};

// コンストラクター
template <class _K_, class _V_, class _C_>
inline
VectorMap<_K_, _V_, _C_>::
VectorMap()
	: Super()
{}

template <class _K_, class _V_, class _C_>
inline
VectorMap<_K_, _V_, _C_>::
VectorMap(const This& cOther_)
	: Super()
{
	*this = cOther_;
}

// デストラクター
template <class _K_, class _V_, class _C_>
inline
VectorMap<_K_, _V_, _C_>::
~VectorMap()
{
	clear();
}

// キーにマッチしたバリューを指すイテレーターを得る
template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::ConstIterator
VectorMap<_K_, _V_, _C_>::
find(const _K_& cKey_) const
{
	ConstIterator e = end();
	Compare comp;
	ConstIterator iterator =
		ModLowerBound(begin(), e, cKey_, comp);
	if (iterator != e && !comp(cKey_, *iterator))
		return iterator;
	return e;
}

template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::Iterator
VectorMap<_K_, _V_, _C_>::
find(const _K_& cKey_)
{
	Iterator e = end();
	Compare comp;
	Iterator iterator =
		ModLowerBound(begin(), e, cKey_, comp);
	if (iterator != e && !comp(cKey_, *iterator))
		return iterator;
	return e;
}

// 下限検索
template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::ConstIterator
VectorMap<_K_, _V_, _C_>::
lowerBound(const _K_& cKey_) const
{
	Compare comp;
	return ModLowerBound(begin(), end(), cKey_, comp);
}

template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::Iterator
VectorMap<_K_, _V_, _C_>::
lowerBound(const _K_& cKey_)
{
	Compare comp;
	return ModLowerBound(begin(), end(), cKey_, comp);
}

// 上限検索
template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::ConstIterator
VectorMap<_K_, _V_, _C_>::
upperBound(const _K_& cKey_) const
{
	Compare comp;
	return ModUpperBound(begin(), end(), cKey_, comp);
}

template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::Iterator
VectorMap<_K_, _V_, _C_>::
upperBound(const _K_& cKey_)
{
	Compare comp;
	return ModUpperBound(begin(), end(), cKey_, comp);
}

// エントリーを挿入する
template <class _K_, class _V_, class _C_>
inline
ModPair<typename VectorMap<_K_, _V_, _C_>::Iterator, ModBoolean>
VectorMap<_K_, _V_, _C_>::
insert(const ValueType& cValue_)
{
	Iterator e = end();
	Compare comp;
	Iterator iterator =
		ModLowerBound(begin(), e, cValue_.first, comp);
	if (iterator == e || comp(cValue_.first, *iterator))
		return ModPair<Iterator, ModBoolean>(Iterator(Super::insert(iterator, cValue_)), ModTrue);
	return ModPair<Iterator, ModBoolean>(iterator, ModFalse);
}

template <class _K_, class _V_, class _C_>
inline
ModPair<typename VectorMap<_K_, _V_, _C_>::Iterator, ModBoolean>
VectorMap<_K_, _V_, _C_>::
insert(const _K_& cKey_, const _V_& cValue_)
{
	Iterator e = end();
	Compare comp;
	Iterator iterator =	ModLowerBound(begin(), e, cKey_, comp);
	if (iterator == e || comp(cKey_, *iterator))
		return ModPair<Iterator, ModBoolean>(Iterator(Super::insert(iterator, Entry(cKey_, cValue_))), ModTrue);
	return ModPair<Iterator, ModBoolean>(iterator, ModFalse);
}

template <class _K_, class _V_, class _C_>
inline
void
VectorMap<_K_, _V_, _C_>::
insert(ConstIterator first, const ConstIterator& last)
{
	ModSize size = getSize();
	ModSize n = last - first;
	if (size + n > Super::getCapacity())
		reserve(size + ModMax(size, n));

	for(; first != last; ++first)
		insert(*first);
}

template <class _K_, class _V_, class _C_>
inline
void
VectorMap<_K_, _V_, _C_>::
insert(Iterator first, const Iterator& last)
{
	ModSize size = getSize();
	ModSize n = last - first;
	if (size + n > Super::getCapacity())
		reserve(size + ModMax(size, n));

	for(; first != last; ++first)
		insert(*first);
}

template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::Iterator
VectorMap<_K_, _V_, _C_>::
erase(const Iterator& position)
{
	return Iterator(Super::erase(position));
}

template <class _K_, class _V_, class _C_>
inline
typename VectorMap<_K_, _V_, _C_>::Iterator
VectorMap<_K_, _V_, _C_>::
erase(const Iterator& first, const Iterator& last)
{
	return Iterator(Super::erase(first, last));
}

// キーにマッチしたものを消す
template <class _K_, class _V_, class _C_>
inline
int
VectorMap<_K_, _V_, _C_>::
erase(const _K_& cKey_)
{
	Iterator e = end();
	Compare comp;
	Iterator iterator =
		ModLowerBound(begin(), e, cKey_, comp);
	if (iterator != e && !comp(cKey_, *iterator)) {
		erase(iterator);
		return 1;
	}
	return 0;
}

// キーにマッチしたバリューへの参照を得る
template <class _K_, class _V_, class _C_>
inline
_V_&
VectorMap<_K_, _V_, _C_>::
operator[](const _K_& cKey_)
{
	return (*insert(cKey_, _V_()).first).second;
}

// キーにマッチしたバリューへの参照を得る(const)
// [NOTES]
// cKey_'s validity should be confirmed by the caller
template <class _K_, class _V_, class _C_>
inline
const _V_&
VectorMap<_K_, _V_, _C_>::
operator[](const _K_& cKey_) const
{
	const ConstIterator& iterator = find(cKey_);
	return (*iterator).second;
}

// clearの実装を上書きする
template <class _K_, class _V_, class _C_>
inline
void
VectorMap<_K_, _V_, _C_>::
clear()
{
	Super::clear();
}

// operator=の実装を上書きする
template <class _K_, class _V_, class _C_>
VectorMap<_K_, _V_, _C_>&
VectorMap<_K_, _V_, _C_>::
operator=(const This& cOther_)
{
	Super::operator = (cOther_);
	return *this;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END


#endif //__TRMEISTER_COMMON_VECTORMAP_H

//
//	Copyright (c) 2004, 2005, 2006, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

