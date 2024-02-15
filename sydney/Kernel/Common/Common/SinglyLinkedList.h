// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SinglyLinkedList.h --	侵入型単方向リスト関連の
//							テンプレートクラス定義、関数宣言
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

#ifndef	__TRMEISTER_COMMON_SINGLYLINKEDLIST_H
#define	__TRMEISTER_COMMON_SINGLYLINKEDLIST_H

#include "Common/Module.h"
#include "Common/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

template <class T>
class SinglyLinkedList;

//	TEMPLATE CLASS
//	Common::SinglyLinkedListIterator --
//		侵入型単方向リストの反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES

template <class T, class Ref>
class SinglyLinkedListIterator
	: public	Object
{
	friend class SinglyLinkedList<T>;
public:
	// コンストラクター
	SinglyLinkedListIterator(SinglyLinkedList<T>& list);
	SinglyLinkedListIterator(SinglyLinkedList<T>& list, T* prev, T* v);
	// コピーコンストラクター
	SinglyLinkedListIterator(const SinglyLinkedListIterator& src);
	// デストラクター
	~SinglyLinkedListIterator();

	// = 演算子
	SinglyLinkedListIterator<T, Ref>&
	operator =(const SinglyLinkedListIterator<T, Ref>& src);

	// ++ 前置演算子
	SinglyLinkedListIterator<T, Ref>&
	operator ++();
	// ++ 後置演算子
	SinglyLinkedListIterator<T, Ref>
	operator ++(int dummy);

	// * 単項演算子
	Ref
	operator *() const;

	// == 演算子
	bool
	operator ==(const SinglyLinkedListIterator<T, Ref>& r) const;
	// != 演算子
	bool
	operator !=(const SinglyLinkedListIterator<T, Ref>& r) const;

private:
	// 反復子が生成されたリスト
	SinglyLinkedList<T>&	_list;
	// 反復子の指す要素の直前の要素へのポインタ
	T*						_prev;
	// 反復子の指す要素へのポインタ
	T*						_v;
};

//	TEMPLATE CLASS
//	Common::SinglyLinkedList --
//		侵入型単方向リストを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES

template <class T>
class SinglyLinkedList
	: public	Object
{
	friend class SinglyLinkedListIterator<T, T&>;
	friend class SinglyLinkedListIterator<T, const T&>;
public:
	//	TYPEDEF
	//	Common::SinglyLinkedList<T>::Iterator -- 反復子を表すクラス
	//
	//	NOTES

	typedef	SinglyLinkedListIterator<T, T&>		Iterator;

	//	TYPEDEF
	//	Common::SinglyLinkedList<T>::ConstIterator --
	//		読み込み専用反復子を表すクラス
	//	
	//	NOTES

	typedef SinglyLinkedListIterator<T, const T&>	ConstIterator;

	//	TYPEDEF
	//	Common::SinglyLinkedList<T>::Size -- リストの長さを表す型
	//
	//	NOTES

	typedef	unsigned int	Size;

	// デフォルトコンストラクター
	SinglyLinkedList();
	// コンストラクター
	SinglyLinkedList(T* T::* next);
	// デストラクター
	~SinglyLinkedList();

	// 先頭の要素を指す反復子を得る
	Iterator
	begin();
	ConstIterator
	begin() const;
	// end を指す反復子を得る
	Iterator
	end();
	ConstIterator
	end() const;

	// 先頭の要素を得る
	T&
	getFront();
	const T&
	getFront() const;
	// 末尾の要素を得る
	T&
	getBack();
	const T&
	getBack() const;

	// 要素を挿入する
	Iterator
	insert(Iterator& position, T& v);
	Iterator
	insert(T& l, T& r, T& v);
	// 先頭に要素を追加する
	void
	pushFront(T& v);
	// 末尾に要素を追加する
	void
	pushBack(T& v);

	// 要素を削除する
	void
	erase(Iterator& position);
	void
	erase(T& prev, T& v);
	// 先頭の要素を削除する
	void
	popFront();
	// 末尾の要素を削除する
	void
	popBack();

	// 空にする
	void
	clear();
	// 空にし、前後の要素へのポインタを格納するメンバへのポインタを設定する
	void
	reset(T* T::* next);

	// あるリストの要素を移動する
	void
	splice(Iterator& position, SinglyLinkedList<T>& src);
	void
	splice(Iterator& position, SinglyLinkedList<T>& src, Iterator& first);
	void
	splice(Iterator& position,
		   SinglyLinkedList<T>& src, Iterator& first, Iterator& last);

	// 要素数を得る
	Size
	getSize() const;
	// 空か調べる
	bool
	isEmpty() const;

private:
	// 要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
	T* T::*					_next;
	// 先頭の要素へのポインタ
	T*						_front;
	// 末尾の要素へのポインタ
	T*						_back;
	// 登録されている要素数
	Size					_size;
};

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::SinglyLinkedListIterator --
//		侵入型単方向リストの反復子を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//
//	ARGUMENTS
//		Common::SinglyLinkedList<T>&	list
//			反復子を生成するリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>::
SinglyLinkedListIterator(SinglyLinkedList<T>& list)
	: _list(list),
	  _prev(0),
	  _v(0)
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::SinglyLinkedListIterator --
//		侵入型単方向リストの反復子を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//
//	ARGUMENTS
//		Common::SinglyLinkedList<T>&	list
//			反復子を生成するリスト
//		T*					prev
//			反復子の指す要素の直前の要素へのポインタ
//		T*					v
//			反復子の指す要素へのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>::
SinglyLinkedListIterator(SinglyLinkedList<T>& list, T* prev, T* v)
	: _list(list),
	  _prev(prev),
	  _v(v)
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::SinglyLinkedListIterator --
//		侵入型単方向リストの反復子を表すテンプレートクラスの
//		コピーコンストラクター
//	
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//
//	ARGUMENTS
//		Common::SinglyLinkedListIterator<T, Ref>&	src
//			コピーする反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>::
SinglyLinkedListIterator(const SinglyLinkedListIterator& src)
	: _list(src._list),
	  _prev(src._prev),
	  _v(src._v)
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::~SinglyLinkedListIterator --
//		侵入型単方向リストの反復子を表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>::~SinglyLinkedListIterator()
{}

//	FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//
//	ARGUMENTS
//		Common::SinglyLinkedListIterator<T, Ref>&	src
//			自分自身に代入する反復子
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>&
SinglyLinkedListIterator<T, Ref>::
operator =(const SinglyLinkedListIterator<T, Ref>& src)
{
	if (this != &src) {
		_list = src._list;
		_prev = src._prev;
		_v = src._v;
	}
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator ++ -- ++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		反復子が指している要素の直後の要素を指すようにする
//
//		リストの末尾の要素の次(end)を指す反復子に対して呼び出した場合、
//		動作を保証しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の要素を指す自分自身
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>&
SinglyLinkedListIterator<T, Ref>::operator ++()
{
	_v = _v->*(_list._next);
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator ++ -- ++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		反復子が指している要素の直後の要素を指すようにする
//
//		リストの末尾の要素の次(end)を指す反復子に対して呼び出した場合、
//		動作を保証しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直後の要素を指す前に自分を複製したもの
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
SinglyLinkedListIterator<T, Ref>
SinglyLinkedListIterator<T, Ref>::operator ++(int dummy)
{
	SinglyLinkedListIterator<T, Ref> tmp = *this;
	++*this;
	return tmp;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator * -- * 単項演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		反復子が指している要素を得る
//
//		リストの末尾の要素の次(end)を指す反復子に対して呼び出した場合、
//		動作を保証しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指す要素
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
Ref
SinglyLinkedListIterator<T, Ref>::operator *() const
{
	return *_v;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		与えられた反復子と自分が同じ要素を指しているか調べる
//
//		自分と与えられた反復子は同じリストから
//		生成されたものであることが前提になっている
//
//	ARGUMENTS
//		SinglyLinkedListIterator<T, Ref>&	r
//			調べる反復子
//
//	RETURN
//		true
//			同じ要素を指している
//		false
//			同じ要素を指していない
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
bool
SinglyLinkedListIterator<T, Ref>::
operator ==(const SinglyLinkedListIterator<T, Ref>& r) const
{
	return _v == r._v;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedListIterator<T, Ref>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		与えられた反復子と自分が同じ要素を指していないことを調べる
//
//		自分と与えられた反復子は同じリストから
//		生成されたものであることが前提になっている
//
//	ARGUMENTS
//		SinglyLinkedListIterator<T, Ref>&	r
//			調べる反復子
//
//	RETURN
//		true
//			同じ要素を指していない
//		false
//			同じ要素を指している
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
bool
SinglyLinkedListIterator<T, Ref>::
operator !=(const SinglyLinkedListIterator<T, Ref>& r) const
{
	return _v != r._v;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::SinglyLinkedList --
//		侵入型単方向リストを表すテンプレートクラスのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		生成されたリストをそのまま使用したときの動作は保証されない
//		が、リストの配列を生成するために必要である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::SinglyLinkedList()
	: _next(0),
	  _front(0),
	  _back(0),
	  _size(0)
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::SinglyLinkedList -- 
//		侵入型単方向リストを表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T* T::*				next
//			要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::SinglyLinkedList(T* T::* next)
	: _next(next),
	  _front(0),
	  _back(0),
	  _size(0)
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::~SinglyLinkedList -- 
//		侵入型単方向リストを表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::~SinglyLinkedList()
{}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::begin --
//		侵入型単方向リストの先頭の要素を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		ひとつもリストに要素がなければ、
//		end を指す反復子が得られる
//		
//	ARGUMENTS
//		なし
//
//	RETURN
//		先頭の要素を指す反復子
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::Iterator
SinglyLinkedList<T>::begin()
{
	return Iterator(*this, 0, _front);
}

template <class T>
inline
SinglyLinkedList<T>::ConstIterator
SinglyLinkedList<T>::begin() const
{
	return ConstIterator(const_cast<SinglyLinkedList<T>&>(*this), 0, _front);
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::end --
//		侵入型単方向リストの末尾の要素を次を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		
//	ARGUMENTS
//		なし
//
//	RETURN
//		末尾の要素の次を指す反復子
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::Iterator
SinglyLinkedList<T>::end()
{
	return Iterator(*this);
}

template <class T>
inline
SinglyLinkedList<T>::ConstIterator
SinglyLinkedList<T>::end() const
{
	return ConstIterator(const_cast<SinglyLinkedList<T>&>(*this));
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::getFront --
//		侵入型単方向リストの先頭の要素を得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		リストが空の場合に呼び出した場合、動作を保証しない
//		
//	ARGUMENTS
//		なし
//
//	RETURN
//		先頭の要素
//
//	EXCEPTIONS
//		なし

template <class T>
inline
T&
SinglyLinkedList<T>::getFront()
{
	return *_front;
}

template <class T>
inline
const T&
SinglyLinkedList<T>::getFront() const
{
	return *_front;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::getBack --
//		侵入型単方向リストの末尾の要素を得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		リストが空の場合に呼び出した場合、動作を保証しない
//		
//	ARGUMENTS
//		なし
//
//	RETURN
//		末尾の要素
//
//	EXCEPTIONS
//		なし

template <class T>
inline
T&
SinglyLinkedList<T>::getBack()
{
	return *_back;
}

template <class T>
inline
const T&
SinglyLinkedList<T>::getBack() const
{
	return *_back;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::insert --
//		侵入型単方向リストのある位置に要素を挿入する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		指定された反復子の指す要素の左の要素として、
//		指定された値を挿入する
//
//		異なるリストの要素を指す反復子が指定されたとき、
//		動作を保証しない
//
//	ARGUMENTS
//		Common::SinglyLinkedList<T>::Iterator&	position
//			挿入する要素の右の要素を指す反復子
//		T&				v
//			要素として挿入するオブジェクト
//
//	RETURN
//		挿入した要素を指す反復子
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::Iterator
SinglyLinkedList<T>::insert(Iterator& position, T& v)
{
	if (!_back) {
		pushFront(v);
		return Iterator(*this, 0, &v);
	} else if (position == end()) {
		T* prev = _back;
		pushBack(v);
		return Iterator(*this, prev, &v);
	}
	return insert(getBack(), *position, v);
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::insert --
//		侵入型単方向リストのある位置に要素を挿入する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		指定された反復子の指す要素の左の要素として、
//		指定された値を挿入する
//
//		異なるリストの要素を指す反復子が指定されたとき、
//		動作を保証しない
//
//	ARGUMENTS
//		T&				l
//			挿入する要素の左の要素
//		T&				r
//			挿入する要素の右の要素
//		T&				v
//			要素として挿入するオブジェクト
//
//	RETURN
//		挿入した要素を指す反復子
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::Iterator
SinglyLinkedList<T>::insert(T& l, T& r, T& v)
{
	l.*_next = &v;
	v.*_next = &r;

	++_size;

	return Iterator(*this, &l, &v);
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::pushFront --
//		侵入型単方向リストの先頭に要素を挿入する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T&					v
//			リストの先頭の要素として挿入するオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::pushFront(T& v)
{
	if (!(v.*_next = _front))
		_back = &v;
	_front = &v;

	++_size;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::pushBack --
//		侵入型単方向リストの末尾に要素を挿入する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T&					v
//			リストの末尾の要素として挿入するオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::pushBack(T& v)
{
	v.*_next = 0;
	_back = ((_back) ? _back->*_next : _front) = &v;

	++_size;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::erase --
//		侵入型単方向リストから要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		異なるリストの要素を指す反復子が指定されたとき、
//		動作を保証しない
//
//	ARGUMENTS
//		Iterator&			position
//			リストから削除する要素を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::erase(Iterator& position)
{
	if (position != end())
		if (posision._prev)
			erase(*position._prev, *position);
		else
			popFront();
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::erase --
//		侵入型単方向リストから要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		異なるリストの要素を指す反復子が指定されたとき、
//		動作を保証しない
//
//	ARGUMENTS
//		T&					prev
//			リストから削除する要素の直前の要素
//		T&					v
//			リストから削除する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::erase(T& prev, T& v)
{
	if (!(prev.*_next = v.*_next))
		_back = prev;
	
	v.*_next = 0;

	--_size;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::popFront --
//		侵入型単方向リストの先頭の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::popFront()
{
	if (getSize()) {
		T& v = *_front;
		if (!(_front = v.*_next))
			_back = 0;

		v.*_next = 0;

		--_size;
	}
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::popBack --
//		侵入型単方向リストの末尾の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::popBack()
{
	if (getSize()) {

		// 削除する末尾の要素の直前の要素を求める

		T* prev = 0;
		for (T* p = _front; p != _back; prev = p, p = p->*_next) ;

		T& v = *_back;
		((_back = prev) ? prev->*_next : _front) = 0;

		v.*_next = 0;

		--_size;
	}
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::clear -- 侵入型単方向リストを空にする
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::clear()
{
	if (_next) {
		T* p = _front;
		T* next;
		for (; p; p = next) {
			next = p->*_next;
			p->*_next = 0;
		}
	}
	_front = _back = 0;
	_size = 0;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::reset --
//		侵入型単方向リストを空にし、
//		後の要素へのポインタを格納するメンバへのポインタを設定する
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T* T::*				next
//			要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::reset(T* T::* next)
{
	clear();
	_next = next;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::splice --
//		ある侵入型単方向リストのすべての要素をあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		SinglyLinkedList<T>::Iterator&	position
//			指定されたリストのすべての要素を移動する位置の
//			直後の要素を指す反復子
//		SinglyLinkedList<T>&	src
//			すべての要素を移動する侵入型単方向リスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::splice(Iterator& position, SinglyLinkedList<T>& src)
{
	splice(position, src, src.begin(), src.end());
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::splice --
//		ある侵入型単方向リストのシーケンスをあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		SinglyLinkedList<T>::Iterator&	position
//			指定されたリストのすべての要素を移動する位置の
//			直後の要素を指す反復子
//		SinglyLinkedList<T>&	src
//			シーケンスを移動する侵入型単方向リスト
//		SinglyLinkedList<T>::Iterator&	first
//			移動するシーケンスの先頭を指す反復子
//		SinglyLinkedList<T>::Iterator&	last
//			指定されたとき
//				移動するシーケンスの末尾のひとつ後の要素を指す反復子
//			指定されないとき
//				first の指す要素の次の要素を指す反復子が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
SinglyLinkedList<T>::splice(Iterator& position, SinglyLinkedList<T>& src,
							Iterator& first)
{
	if (position != first && first != src.end() &&
		(position == end() || position._v != first._v->*(src._next))) {

		// 反復子 first の指す要素を
		// 反復子 position の指す要素の前に移動する
		//
		//	                    first
		//	                     ↓
		//	src :	[  ] … [  ][  ] …
		//
		//			            position
		//		                 ↓ 
		//	*this : [  ] … [  ][  ] …

		if (!(((first._prev) ? first._prev->*(src._next) : src._front) =
			  first._v->*(src._next)))
			src._back = first._prev;

		--src._size;

		T*& z = (position._prev) ? position._prev : _back;
		first._v->*_next = position._v;
		z = ((z) ? z->*_next : _front) = first._v;

		++_size;
	}
}

template <class T>
inline
void
SinglyLinkedList<T>::splice(Iterator& position, SinglyLinkedList<T>& src,
							Iterator& first, Iterator& last)
{
	if (position != first && first != src.end() &&
		position != last && first != last) {

		// 移動する要素の数を求める

		Size n = 0;
		for (ConstIterator ite = first; ite != last; ++ite, ++n) ;

		if (n) {

			// 反復子 first の指す要素から
			// 反復子 last の指す要素の前の要素までを
			// 反復子 position の指す要素の前に移動する
			//
			//	                    first       last
			//	                     ↓          ↓ 
			//	src :	[  ] … [  ][  ] … [  ][  ] …
			//
			//			            position
			//		                 ↓ 
			//	*this : [  ] … [  ][  ] …

			T*&	x = (last._prev) ? last._prev : src._back;
			T*	y = x;
			((x = first._prev) ? x->*(src._next) : src._front) = last.v;

			src._size -= n;

			T*&	z = (position._prev) ? position._prev : _back;
			y->*_next = position._v;
			((first._prev = z) ? z->*_next : _front) = first._v;
			z = y;

			_size += n;
		}
	}
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::getSize -- 侵入型単方向リストの要素数を得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた要素数
//
//	EXCEPTIONS
//		なし

template <class T>
inline
SinglyLinkedList<T>::Size
SinglyLinkedList<T>::getSize() const
{
	return _size;
}

//	TEMPLATE FUNCTION public
//	Common::SinglyLinkedList<T>::isEmpty -- 侵入型単方向リストが空か調べる
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			空である
//		false
//			空でない
//
//	EXCEPTIONS
//		なし

template <class T>
inline
bool
SinglyLinkedList<T>::isEmpty() const
{
	return !_size;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_DOUBLELINKEDLIST_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
