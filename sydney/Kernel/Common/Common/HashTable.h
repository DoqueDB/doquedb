// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HashTable.h --	侵入型双方向リストをバケットとする
//					ハッシュ表関連のテンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_HASHTABLE_H
#define	__TRMEISTER_COMMON_HASHTABLE_H

#include "Common/Object.h"

#include "ModHashTable.h"

_TRMEISTER_BEGIN

namespace Common
{

//	TEMPLATE CLASS
//	Common::HashTable --
//		侵入型双方向リストをバケットとする
//		ハッシュ表を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//		バケットを表す侵入型双方向リスト L は、
//		ハッシュ表に格納する要素 T を格納しなければならない

template <class L, class T>
class HashTable
	: public	Common::Object
{
public:
	//	TYPEDEF
	//	Common::HashTable<L, T>::Bucket -- バケットを表すクラス
	//
	//	NOTES

	typedef	L				Bucket;

	//	TYPEDEF
	//	Common::HashTable<L, T>::Length -- ハッシュ表のバケット数を表す型
	//
	//	NOTES

	typedef unsigned int	Length;

	//	TYPEDEF
	//	Common::HashTable<L, T>::Size -- ハッシュ表の要素数を表す型
	//
	//	NOTES

	typedef	unsigned int	Size;

	// コンストラクター
	HashTable(unsigned int length, T* T::* prev, T* T::* next);
	// デストラクター
	~HashTable();

	// バケット数を得る
	Length					getLength() const;
	// バケットを得る
	Bucket&					getBucket(unsigned int addr);
	const Bucket&			getBucket(unsigned int addr) const;

	// 要素数を得る
	Size					getSize() const;
	// 空か調べる
	bool					isEmpty() const;

private:
	// バケット数
	Length					_length;
	// バケット表
	Bucket*					_buckets;
};

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::HashTable -- 
//		侵入型双方向リストをバケットとする
//		ハッシュ表を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//		与えられたバケット数からそれ以上の適当な素数を求めて、
//		その数ぶんのバケットを持つハッシュ表を生成する
//
//	ARGUMENTS
//		unsigned int		length
//			バケット数
//		T* T::*				prev
//			要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
//		T* T::*				next
//			要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class L, class T>
inline
HashTable<L, T>::HashTable(unsigned int length, T* T::* prev, T* T::* next)
	: _length(ModHashTableBase::verifySize(length)),
	  _buckets(0)
{
	// 与えられたバケット数を
	// 元にして求めた素数ぶんのバケットを確保する

	_buckets = new Bucket[_length];
	for (unsigned int i = 0; i < _length; ++i)
		_buckets[i].reset(prev, next);
}

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::~HashTable --
//		侵入型双方向リストをバケットとする
//		ハッシュ表を表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
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

template <class L, class T>
inline
HashTable<L, T>::~HashTable()
{
	delete[] _buckets, _buckets = 0;
}

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::getLength -- バケット数を得る
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバケット数
//
//	EXCEPTIONS
//		なし

template <class L, class T> 
inline
typename HashTable<L, T>::Length
HashTable<L, T>::getLength() const
{
	return _length;
}

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::getBucket -- バケットを得る
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		addr
//			バケットを取得したいバケットアドレス
//
//	RETURN
//		得られたバケット
//
//	EXCEPTIONS
//		なし

template <class L, class T>
inline
typename HashTable<L, T>::Bucket&
HashTable<L, T>::getBucket(unsigned int addr)
{
	return _buckets[addr];
}

template <class L, class T>
inline
const typename HashTable<L, T>::Bucket&
HashTable<L, T>::getBucket(unsigned int addr) const
{
	return _buckets[addr];
}

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::getSize -- 要素数を得る
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//		すべてのバケットの要素数の総和を求めるので、かなり遅い
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた要素数
//
//	EXCEPTIONS
//		なし

template <class L, class T>
inline
typename HashTable<L, T>::Size
HashTable<L, T>::getSize() const
{
	Size n = 0;
	for (Length i = 0; i < getLength(); ++i)
		n += getBucket(i).getSize();

	return n;
}

//	TEMPLATE FUNCTION public
//	Common::HashTable<L, T>::isEmpty -- 空か調べる
//
//	TEMPLATE ARGUMENTS
//		class L
//			バケットを表す侵入型双方向リストの型
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//		すべてのバケットが空であるか調べるので、かなり遅い
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

template <class L, class T>
inline
bool
HashTable<L, T>::isEmpty() const
{
	for (Length i = 0; i < getLength(); ++i)
		if (getBucket(i).getSize())
			return false;

	return true;
}

}

_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_HASHTABLE_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
