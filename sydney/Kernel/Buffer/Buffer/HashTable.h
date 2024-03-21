// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HashTable.h --	バッファプールで管理しているオブジェクトを
//					管理するためのハッシュ表関連の
//					テンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2010, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_HASHTABLE_H
#define	__SYDNEY_BUFFER_HASHTABLE_H

#include "Buffer/Module.h"
#include "Buffer/List.h"

#include "Os/CriticalSection.h"

#include "Common/HashTable.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	TEMPLATE CLASS
//	Buffer::HashTable --
//		バッファプールで管理しているオブジェクトを
//		管理するためのハッシュ表を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES

template <class T>
class HashTable
	: public	Common::HashTable<List<T>, T>
{
public:
	// コンストラクター
	HashTable(unsigned int size, T* T::* prev, T* T::* next);
	// デストラクター
	~HashTable();

	using Common::HashTable<List<T>, T>::getLength;
	using Common::HashTable<List<T>, T>::getBucket;

private:
	// バケットを排他するためラッチ
	// 以前は List クラスのメンバーだったが、ハッシュ表のサイズが大きくなり、
	// 大量のリソースを消費してしまうので、ここでインスタンスを確保し、
	// ポインターを List クラスに割り当てる

	Os::CriticalSection* _latchArray;

	unsigned int _arraySize;
};

//	TEMPLATE FUNCTION public
//	Buffer::HashTable<T>::HashTable -- 
//		バッファプールで管理しているオブジェクトを管理する
//		ハッシュ表を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		size
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

template <class T>
inline
HashTable<T>::HashTable(unsigned int size, T* T::* prev, T* T::* next)
	: Common::HashTable<List<T>, T>(size, prev, next), _latchArray(0)
{
	// 排他制御用のラッチ数 - 今は 1024 固定

	_arraySize = 1024;
	
	// 排他制御用のラッチを確保する

	_latchArray = new Os::CriticalSection[_arraySize];

	// バケットにポインターを割り当てる
	//
	//【注意】
	// for (Length i = 0; ...) としたいが、gccでコンパイルエラー(undef)に
	// なるので、unsigned int とする

	for (unsigned int i = 0; i < getLength(); ++i)
		getBucket(i)._latch = &_latchArray[i % _arraySize];
}

//	TEMPLATE FUNCTION public
//	Buffer::HashTable<T>::~HashTable -- 
//		バッファプールで管理しているオブジェクトを管理する
//		ハッシュ表を表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
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

template <class T>
inline
HashTable<T>::~HashTable()
{
	delete[] _latchArray;
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_HASHTABLE_H

//
// Copyright (c) 2000, 2010, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
