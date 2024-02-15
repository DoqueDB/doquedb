// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DoubleLinkedList.h --	侵入型双方向リスト関連の
//							テンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_DOUBLELINKEDLIST_H
#define	__TRMEISTER_COMMON_DOUBLELINKEDLIST_H

#include "Common/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

template <class T>
class DoubleLinkedList;

//	TEMPLATE CLASS
//	Common::DoubleLinkedListIterator --
//		侵入型双方向リストの反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES

template <class T, class Ref>
class DoubleLinkedListIterator
{
	friend class DoubleLinkedList<T>;
public:
	// コンストラクター
	DoubleLinkedListIterator(DoubleLinkedList<T>& list, T* v = 0);
	// コピーコンストラクター
	DoubleLinkedListIterator(const DoubleLinkedListIterator& src);
	// デストラクター
	~DoubleLinkedListIterator();

	// = 演算子
	DoubleLinkedListIterator<T, Ref>&	operator =(
		const DoubleLinkedListIterator<T, Ref>& src);

	// ++ 前置演算子
	DoubleLinkedListIterator<T, Ref>&	operator ++();
	// ++ 後置演算子
	DoubleLinkedListIterator<T, Ref>	operator ++(int dummy);
	// -- 前置演算子
	DoubleLinkedListIterator<T, Ref>&	operator --();
	// -- 後置演算子
	DoubleLinkedListIterator<T, Ref>	operator --(int dummy);

	// * 単項演算子
	Ref						operator *() const;

	// == 演算子
	bool					operator ==(
							const DoubleLinkedListIterator<T, Ref>& r) const;
	// != 演算子
	bool					operator !=(
							const DoubleLinkedListIterator<T, Ref>& r) const;
private:
	// 反復子が生成されたリスト
	DoubleLinkedList<T>&	_list;
	// 反復子の指す要素へのポインタ
	T*						_v;
};

//	TEMPLATE CLASS
//	Common::DoubleLinkedList --
//		侵入型双方向リストを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES

template <class T>
class DoubleLinkedList
{
	friend class DoubleLinkedListIterator<T, T&>;
	friend class DoubleLinkedListIterator<T, const T&>;
public:
	//	TYPEDEF
	//	Common::DoubleLinkedList<T>::Iterator -- 反復子を表すクラス
	//
	//	NOTES

	typedef	DoubleLinkedListIterator<T, T&>		Iterator;

	//	TYPEDEF
	//	Common::DoubleLinkedList<T>::ConstIterator --
	//		読み込み専用反復子を表すクラス
	//	
	//	NOTES

	typedef DoubleLinkedListIterator<T, const T&>	ConstIterator;

	//	TYPEDEF
	//	Common::DoubleLinkedList<T>::Size -- リストの長さを表す型
	//
	//	NOTES

	typedef	unsigned int	Size;

	// デフォルトコンストラクター
	DoubleLinkedList();
	// コンストラクター
	DoubleLinkedList(T* T::* prev, T* T::* next);
	// デストラクター
	~DoubleLinkedList();

	// 先頭の要素を指す反復子を得る
	Iterator				begin();
	ConstIterator			begin() const;
	// end を指す反復子を得る
	Iterator				end();
	ConstIterator			end() const;

	// 先頭の要素を得る
	T&						getFront();
	const T&				getFront() const;
	// 末尾の要素を得る
	T&						getBack();
	const T&				getBack() const;

	// 要素を挿入する
	Iterator				insert(const Iterator& position, T& v);
	Iterator				insert(T& r, T& v);
	// 先頭に要素を追加する
	void					pushFront(T& v);
	// 末尾に要素を追加する
	void					pushBack(T& v);

	// 要素を削除する
	void					erase(const Iterator& position);
	void					erase(T& v);
	// 先頭の要素を削除する
	void					popFront();
	// 末尾の要素を削除する
	void					popBack();

	// 空にする
	void					clear();
	// 空にし、前後の要素へのポインタを格納するメンバへのポインタを設定する
	void					reset(T* T::* prev, T* T::* next);

	// あるリストの要素を移動する
	void
	splice(const Iterator& position, DoubleLinkedList<T>& src);
	void
	splice(const Iterator& position, DoubleLinkedList<T>& src,
		   const Iterator& first);
	void
	splice(const Iterator& position, DoubleLinkedList<T>& src,
		   const Iterator& first, const Iterator& last);

	// 要素数を得る
	Size					getSize() const;
	// 空か調べる
	bool					isEmpty() const;

private:
	// 要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
	T* T::*					_prev;
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
//	Common::DoubleLinkedListIterator<T, Ref>::DoubleLinkedListIterator --
//		侵入型双方向リストの反復子を表すテンプレートクラスのコンストラクター
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
//		Common::DoubleLinkedList<T>&	list
//			反復子を生成するリスト
//		T*					v
//			0 以外の値
//				反復子の指す要素へのポインタ
//			0 または指定されないとき
//				反復子はリストの末尾の要素の次(end)を指す
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
DoubleLinkedListIterator<T, Ref>::
DoubleLinkedListIterator(DoubleLinkedList<T>& list, T* v)
	: _list(list),
	  _v(v)
{}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::DoubleLinkedListIterator --
//		侵入型双方向リストの反復子を表すテンプレートクラスの
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
//		Common::DoubleLinkedListIterator<T, Ref>&	src
//			コピーする反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
DoubleLinkedListIterator<T, Ref>::
DoubleLinkedListIterator(const DoubleLinkedListIterator& src)
	: _list(src._list),
	  _v(src._v)
{}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::~DoubleLinkedListIterator --
//		侵入型双方向リストの反復子を表すテンプレートクラスのデストラクター
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
DoubleLinkedListIterator<T, Ref>::~DoubleLinkedListIterator()
{}

//	FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator = -- = 演算子
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
//		Common::DoubleLinkedListIterator<T, Ref>&	src
//			自分自身に代入する反復子
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
DoubleLinkedListIterator<T, Ref>&
DoubleLinkedListIterator<T, Ref>::
operator =(const DoubleLinkedListIterator<T, Ref>& src)
{
	if (this != &src) {
//		_list = src._list;
		_v = src._v;
	}
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator ++ -- ++ 前置演算子
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
DoubleLinkedListIterator<T, Ref>&
DoubleLinkedListIterator<T, Ref>::operator ++()
{
	_v = _v->*(_list._next);
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator ++ -- ++ 後置演算子
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
DoubleLinkedListIterator<T, Ref>
DoubleLinkedListIterator<T, Ref>::operator ++(int dummy)
{
	DoubleLinkedListIterator<T, Ref> tmp = *this;
	++*this;
	return tmp;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator -- -- -- 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		反復子が指している要素の直前の要素を指すようにする
//
//		リストの先頭の要素を指す反復子に対して呼び出した場合、
//		反復子はリストの末尾の次(end)を指す
//
//		リストの末尾の要素の次(end)を指す反復子に対して呼び出した場合、
//		反復子はリストの末尾を指す
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
DoubleLinkedListIterator<T, Ref>&
DoubleLinkedListIterator<T, Ref>::operator --()
{
	_v = (_v) ? _v->*(_list._prev) : _list._back;
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator -- -- -- 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			反復子の指す要素の型
//		class Ref
//			反復子の指す要素のレファレンスの型
//
//	NOTES
//		反復子が指している要素の直前の要素を指すようにする
//
//		リストの先頭の要素を指す反復子に対して呼び出した場合、
//		反復子はリストの末尾の次(end)を指す
//
//		リストの末尾の要素の次(end)を指す反復子に対して呼び出した場合、
//		反復子はリストの末尾を指す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		直前の要素を指す前に自分を複製したもの
//
//	EXCEPTIONS
//		なし

template <class T, class Ref>
inline
DoubleLinkedListIterator<T, Ref>
DoubleLinkedListIterator<T, Ref>::operator --(int dummy)
{
	DoubleLinkedListIterator<T, Ref> tmp = *this;
	--*this;
	return tmp;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator * -- * 単項演算子
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
DoubleLinkedListIterator<T, Ref>::operator *() const
{
	return *_v;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator == -- == 演算子
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
//		DoubleLinkedListIterator<T, Ref>&	r
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
DoubleLinkedListIterator<T, Ref>::
operator ==(const DoubleLinkedListIterator<T, Ref>& r) const
{
	return _v == r._v;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedListIterator<T, Ref>::operator != -- != 演算子
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
//		DoubleLinkedListIterator<T, Ref>&	r
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
DoubleLinkedListIterator<T, Ref>::
operator !=(const DoubleLinkedListIterator<T, Ref>& r) const
{
	return _v != r._v;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::DoubleLinkedList --
//		侵入型双方向リストを表すテンプレートクラスのデフォルトコンストラクター
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
DoubleLinkedList<T>::DoubleLinkedList()
	: _prev(0),
	  _next(0),
	  _front(0),
	  _back(0),
	  _size(0)
{}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::DoubleLinkedList -- 
//		侵入型双方向リストを表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T* T::*				prev
//			要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
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
DoubleLinkedList<T>::DoubleLinkedList(T* T::* prev, T* T::* next)
	: _prev(prev),
	  _next(next),
	  _front(0),
	  _back(0),
	  _size(0)
{}

//	TEMPLATE FUNCTION public
//	Double::DoubleLinkedList<T>::~DoubleLinkedList -- 
//		侵入型双方向リストを表すテンプレートクラスのデストラクター
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
DoubleLinkedList<T>::~DoubleLinkedList()
{}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::begin --
//		侵入型双方向リストの先頭の要素を指す反復子を得る
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
typename DoubleLinkedList<T>::Iterator
DoubleLinkedList<T>::begin()
{
	return Iterator(*this, _front);
}

template <class T>
inline
typename DoubleLinkedList<T>::ConstIterator
DoubleLinkedList<T>::begin() const
{
	return ConstIterator(const_cast<DoubleLinkedList<T>&>(*this), _front);
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::end --
//		侵入型双方向リストの末尾の要素を次を指す反復子を得る
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
typename DoubleLinkedList<T>::Iterator
DoubleLinkedList<T>::end()
{
	return Iterator(*this);
}

template <class T>
inline
typename DoubleLinkedList<T>::ConstIterator
DoubleLinkedList<T>::end() const
{
	return ConstIterator(const_cast<DoubleLinkedList<T>&>(*this));
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::getFront --
//		侵入型双方向リストの先頭の要素を得る
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
DoubleLinkedList<T>::getFront()
{
	return *_front;
}

template <class T>
inline
const T&
DoubleLinkedList<T>::getFront() const
{
	return *_front;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::getBack --
//		侵入型双方向リストの末尾の要素を得る
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
DoubleLinkedList<T>::getBack()
{
	return *_back;
}

template <class T>
inline
const T&
DoubleLinkedList<T>::getBack() const
{
	return *_back;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::insert --
//		侵入型双方向リストのある位置に要素を挿入する
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
//		Common::DoubleLinkedList<T>::Iterator&	position
//			挿入する要素の右の要素を指す反復子
//
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
typename DoubleLinkedList<T>::Iterator
DoubleLinkedList<T>::insert(const Iterator& position, T& v)
{
	if (position == end()) {
		pushBack(v);
		return Iterator(*this, &v);
	}
	return insert(*position, v);
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::insert --
//		侵入型双方向リストのある位置に要素を挿入する
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
//		T&				r
//			挿入する要素の右の要素
//
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
typename DoubleLinkedList<T>::Iterator
DoubleLinkedList<T>::insert(T& r, T& v)
{
	r.*_prev = ((v.*_prev = r.*_prev) ? v.*_prev->*_next : _front) = &v;
	v.*_next = &r;

	++_size;

	return Iterator(*this, &v);
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::pushFront --
//		侵入型双方向リストの先頭に要素を挿入する
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
DoubleLinkedList<T>::pushFront(T& v)
{
	v.*_prev = 0;
	_front = ((v.*_next = _front) ? v.*_next->*_prev : _back) = &v;

	++_size;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::pushBack --
//		侵入型双方向リストの末尾に要素を挿入する
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
DoubleLinkedList<T>::pushBack(T& v)
{
	v.*_next = 0;
	_back = ((v.*_prev = _back) ? v.*_prev->*_next : _front) = &v;

	++_size;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::erase --
//		侵入型双方向リストから要素を削除する
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
DoubleLinkedList<T>::erase(const Iterator& position)
{
	if (position != end())
		erase(*position);
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::erase --
//		侵入型双方向リストから要素を削除する
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
DoubleLinkedList<T>::erase(T& v)
{
	((((v.*_next) ? v.*_next->*_prev : _back) = v.*_prev) ?
	 v.*_prev->*_next : _front) = v.*_next;

	v.*_prev = v.*_next = 0;

	--_size;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::popFront --
//		侵入型双方向リストの先頭の要素を削除する
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
DoubleLinkedList<T>::popFront()
{
	if (getSize()) {
		T& v = *_front;

		v.*_prev = v.*_next =
			((_front = v.*_next) ? _front->*_prev : _back) = 0;

		--_size;
	}
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::popBack --
//		侵入型双方向リストの末尾の要素を削除する
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
DoubleLinkedList<T>::popBack()
{
	if (getSize()) {
		T& v = *_back;

		v.*_prev = v.*_next =
			((_back = v.*_prev) ? _back->*_next : _front) = 0;

		--_size;
	}
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::clear -- 侵入型双方向リストを空にする
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
DoubleLinkedList<T>::clear()
{
	if (_next && _prev) {
		T*	p = _front;
		T*	q;
		for (; p; p = q) {
			q = p->*_next;
			p->*_prev = p->*_next = 0;
		}
	}
	_front = _back = 0;
	_size = 0;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::reset --
//		侵入型双方向リストを空にし、
//		前後の要素へのポインタを格納するメンバへのポインタを設定する
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T* T::*				prev
//			要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
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
DoubleLinkedList<T>::reset(T* T::* prev, T* T::* next)
{
	_prev = prev, _next = next;
	clear();
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::splice --
//		ある侵入型双方向リストのすべての要素をあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		DoubleLinkedList<T>::Iterator&	position
//			指定されたリストのすべての要素を移動する位置の
//			直後の要素を指す反復子
//		DoubleLinkedList<T>&	src
//			すべての要素を移動する侵入型双方向リスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
void
DoubleLinkedList<T>::splice(const Iterator& position, DoubleLinkedList<T>& src)
{
	splice(position, src, src.begin(), src.end());
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::splice --
//		ある侵入型双方向リストのシーケンスをあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		DoubleLinkedList<T>::Iterator&	position
//			指定されたリストのすべての要素を移動する位置の
//			直後の要素を指す反復子
//		DoubleLinkedList<T>&	src
//			シーケンスを移動する侵入型双方向リスト
//		DoubleLinkedList<T>::Iterator&	first
//			移動するシーケンスの先頭を指す反復子
//		DoubleLinkedList<T>::Iterator&	last
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
DoubleLinkedList<T>::splice(const Iterator& position, DoubleLinkedList<T>& src,
							const Iterator& first)
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


		((first._v->*(src._prev)) ?
		 first._v->*(src._prev)->*(src._next) : src._front) =
			first._v->*(src._next);
		((first._v->*(src._next)) ?
		 first._v->*(src._next)->*(src._prev) : src._back) =
			first._v->*(src._prev);

		--src._size;

		T*& z = (position._v) ? position._v->*_prev : _back;
		first._v->*_next = position._v;
		z = ((first._v->*_prev = z) ? z->*_next : _front) = first._v;

		++_size;
	}
}

template <class T>
inline
void
DoubleLinkedList<T>::splice(const Iterator& position, DoubleLinkedList<T>& src,
							const Iterator& first, const Iterator& last)
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

			T*&	x = (last.v) ? last.v->*(src._prev) : src._back;
			T*	y = x;
			((x = first._v->*(src._prev)) ?
			 x->*(src._next) : src._front) = last.v;

			src._size -= n;

			T*&	z = (position._v) ? position._v->*_prev : _back;
			y->*_next = position._v;
			((first._v->*_prev = z) ? z->*_next : _front) = first._v;
			z = y;

			_size += n;
		}
	}
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::getSize -- 侵入型双方向リストの要素数を得る
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
typename DoubleLinkedList<T>::Size
DoubleLinkedList<T>::getSize() const
{
	return _size;
}

//	TEMPLATE FUNCTION public
//	Common::DoubleLinkedList<T>::isEmpty -- 侵入型双方向リストが空か調べる
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
DoubleLinkedList<T>::isEmpty() const
{
	return !_size;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_DOUBLELINKEDLIST_H

//
// Copyright (c) 2000, 2001, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
