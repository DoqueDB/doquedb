// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Vector.h -- nullビットマップ用のベクター
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_VECTOR_H
#define __SYDNEY_ARRAY_VECTOR_H

#include "Array/Module.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

//
//	TEMPLATE CLASS
//	Array::ReverseIterator -- nullビットマップ用ベクターのiterator
//
//	NOTES
//
template <class TYPE>
class ReverseIterator
{
	typedef ReverseIterator<TYPE> self;
public:
	// コンストラクタ
	ReverseIterator(TYPE* current_ = 0) : _current(current_) {}
	// デストラクタ
	~ReverseIterator() {}

	// アクセス
	const TYPE& operator * () const
	{
		const TYPE* tmp = _current; return *--tmp;
	}
	TYPE& operator * ()
	{
		TYPE* tmp = _current; return *--tmp;
	}
	const TYPE& operator [] (int n_) const { return *(_current - n_ - 1); }
	TYPE& operator [] (int n_) { return *(_current - n_ - 1); }

	// 移動
	self& operator ++ () { --_current; return *this; }
	self& operator -- () { ++_current; return *this; }
	self operator + (int distance_) const
	{
		self tmp(_current - distance_); return tmp;
	}
	self& operator += (int distance_) { _current -= distance_; return *this; }
	self operator - (int distance_) const
	{
		self tmp(_current + distance_); return tmp;
	}
	self& operator -= (int distance_) { _current += distance_; return *this; }

	// 距離
	int operator - (const self& i_) const
	{
		return i_._current - _current;
	}

	// 比較
	bool operator == (const self& i_) const { return i_._current == _current; }
	bool operator != (const self& i_) const { return !(*this == i_); }
	bool operator < (const self& i_) const { return i_._current < _current; }
	bool operator <= (const self& i_) const { return !(i_ < *this); }
	bool operator > (const self& i_) const { return i_ < *this; }
	bool operator >= (const self& i_) const { return !(*this < i_); }

	// ポインタを得る
	const TYPE* base () const { return _current; }

private:
	TYPE* _current;
};

//
//	TEMPLATE CLASS
//	Array::Vector -- nullビットマップ用のベクター
//
//	NOTES
//	逆方向専用のベクター。おまけに領域も確保しない。
//
template <class TYPE>
class Vector
{
public:
	// イテレータ
	typedef ReverseIterator<TYPE>	Iterator;

	// コンストラクタ(1)
	Vector()
		: _begin(0), _end(0)
	{
	}
	// コンストラクタ(2)
	Vector(TYPE* begin_, ModSize uiLength_)
	{
		assign(begin_, uiLength_);
	}
	// デストラクタ
	~Vector() {}

	// データを設定する
	void assign(TYPE* begin_, ModSize uiLength_)
	{
		_begin = begin_;
		_end = begin_ - uiLength_;
	}

	// イテレータを得る
	Iterator begin()
	{
		return Iterator(_begin);
	}
	Iterator end()
	{
		return Iterator(_end);
	}

	// サイズを得る
	ModSize getSize()
	{
		return static_cast<ModSize>(_begin - _end);
	}

	// 挿入する
	void insert(Iterator& position_, TYPE value_)
	{
		Iterator e(_end);
		int size = (e - position_) * sizeof(TYPE);
		TYPE* tmp = _end;
		_end -= 1;
		if (size)
			Os::Memory::move(_end, tmp, size);
		*position_ = value_;
	}
	void insert(Iterator position_,
				const Iterator& begin_, const Iterator& end_)
	{
		Iterator e(_end);
		int size = (e - position_) * sizeof(TYPE);
		TYPE* tmp = _end;
		_end -= (end_ - begin_);
		if (size)
			Os::Memory::move(_end, tmp, size);
		Os::Memory::copy(_end + size, end_.base(), end_ - begin_);
	}

	// 削除する
	void expunge(Iterator& position_)
	{
		Iterator e(_end);
		int size = (e - position_ - 1) * sizeof(TYPE);
		TYPE* tmp = _end;
		_end += 1;
		if (size)
			Os::Memory::move(_end, tmp, size);
	}
	void expunge(Iterator& begin_, Iterator& end_)
	{
		Iterator e(_end);
		int size = (e - end_) * sizeof(TYPE);
		TYPE* tmp = _end;
		_end += (end_ - begin_);
		if (size)
			Os::Memory::move(_end, tmp, size);
	}

	// アクセス
	const TYPE& operator [] (int n_) const { return *(_begin - n_ - 1); }
	TYPE& operator [] (int n_) { return *(_begin - n_ - 1); }

private:
	TYPE* _begin;
	TYPE* _end;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_VECTOR_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
