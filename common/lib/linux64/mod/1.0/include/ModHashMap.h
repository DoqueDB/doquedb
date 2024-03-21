// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModHashMap.h -- ハッシュマップに関するクラス定義
// 
// Copyright (c) 1997, 1999, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModHashMap_H__
#define __ModHashMap_H__

#include "ModCommon.h"
#include "ModPair.h"
#include "ModHashTable.h"
#include "ModHasher.h"

//	CONST
//	ModHashMapSizeDefault -- ハッシュマップのデフォルトの大きさ
//
//	NOTES

const ModSize				ModHashMapSizeDefault = ModHashTableSizeDefault;

//	TEMPLATE CLASS
//	ModHashMap -- ハッシュによる探索が可能なマップを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES

template <class KeyType, class MappedType, class Hasher>
class ModHashMap
	: public	ModHashTable<KeyType, ModPair<KeyType, MappedType>, Hasher>
{
public:
	typedef ModPair<KeyType, MappedType>				ValueType;
	typedef ModHashTableIterator<KeyType, ValueType, Hasher>	Iterator;
	typedef ModHashTableConstIterator<KeyType, ValueType, Hasher> ConstIterator;
	typedef	ModHashMap<KeyType, MappedType, Hasher>		HashMap;
	typedef ModHashTable<KeyType, ValueType, Hasher>	HashTable;

	ModHashMap(ModSize tableSize = ModHashMapSizeDefault,
			   const Hasher& func = Hasher(),
			   ModBoolean enableLink = ModTrue);
	ModHashMap(ModSize tableSize, ModBoolean enableLink);
	ModHashMap(Iterator first, const Iterator& last,
			   ModSize tableSize = ModHashMapSizeDefault,
			   const Hasher& func = Hasher(),
			   ModBoolean enableLink = ModTrue);
	ModHashMap(ConstIterator first, const ConstIterator& last,
			   ModSize tableSize = ModHashMapSizeDefault,
			   const Hasher& func = Hasher(),
			   ModBoolean enableLink = ModTrue);
	ModHashMap(const ValueType* first, const ValueType* last,
			   ModSize tableSize = ModHashMapSizeDefault,
			   const Hasher& func = Hasher(),
			   ModBoolean enableLink = ModTrue);
												// コンストラクター
//	ModHashTable::
//	ModBoolean				isEmpty() const;	// ひとつも登録されていないか

	ModPair<Iterator, ModBoolean>
							insert(const ValueType& value,
								   ModBoolean noCheckFlag = ModFalse);
	void					insert(Iterator first, const Iterator& last,
								   ModBoolean noCheckFlag = ModFalse);
	void					insert(ConstIterator first,
								   const ConstIterator& last,
								   ModBoolean noCheckFlag = ModFalse);
	void					insert(const ValueType* first,
								   const ValueType* last,
								   ModBoolean noCheckFlag = ModFalse);
	ModPair<Iterator, ModBoolean>
							insert(const KeyType& key,
								   const MappedType& mappedValue,
								   ModBoolean noCheckFlag = ModFalse);
	Iterator				insert(Iterator position,
								   const ValueType& value,
								   ModBoolean noCheckFlag = ModFalse);
												// 要素を挿入する
//	ModHashTable::
//	ModSize					erase(const KeyType& key);
//	void					erase(Iterator position);
//	void					erase(Iterator first, const Iterator& last);
//												// 要素を削除する
//
//	void					popFront();			// 先頭の要素を削除する
//
//	void					clear();			// ハッシュマップを空にする
//
//	void					splice(HashMap& src, Iterator& first);
//	void					splice(HashMap& src,
//								   Iterator& first, const Iterator& last);
//												// あるハッシュ表の
//												// 要素を移動する
//	// 以下、近日中に廃止します
//	static void				move(HashMap& fromMap,
//								 Iterator& from, HashMap& toMap);
//												// 他のハッシュマップへ
//												// 要素を移動する
//	// 以上、近日中に廃止します
//
//	Iterator				find(const KeyType& key);
//	ConstIterator			find(const KeyType& key) const;
//												// 探索する

	MappedType&				operator [](const KeyType& key);
	const MappedType&		operator [](const KeyType& key) const;
												// キーに対応するバリューを得る

	static MappedType&		getValue(const Iterator& iterator);
	static const MappedType& getValue(const ConstIterator& iterator);
												// 反復子の指す値の
												// バリューを得る
	const MappedType&		getFront() const;	// 先頭のマップ値を得る

//	ModHashTable::
//	Iterator				begin();
//	ConstIterator			begin() const;		// 先頭の要素を指す反復子を得る
//	const Iterator&			end();
//	const ConstIterator&	end() const;		// 末尾の要素を指す反復子を得る
//
//	ModSize					getSize() const;	// 要素数を得る
//	Hasher					getHasher() const;	// ハッシュ関数を得る
//
//	void					printHist() const;	// ハッシュ表の
//												// バケット分布を出力する
};

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::ModHashMap --
//		ハッシュマップを表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				tableSize
//			指定されたとき
//				ハッシュ表のサイズ
//			指定されないとき
//				ModHashMapSizeDefault が指定されたものとみなす
//		Hasher&				func
//			指定されたとき
//				ハッシュ表での格納位置を求めるためのハッシュ関数
//			指定されないとき
//				Hasher::Hasher() が指定されたものとみなす
//		ModBoolean			enableLink
//			ModTrue または指定されないとき
//				ハッシュ表中のバケット操作を高速化するための配列を確保する
//			ModFalse
//				ハッシュ表中のバケット操作を高速化するための配列を確保しない
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator	first
//			初期値として使用する最初の要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ConstIterator	first
//			初期値として使用する最初の要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType*	first
//			初期値として使用する最初のデータが格納されている領域の先頭アドレス
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator	last
//			初期値として使用する最後の要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ConstIterator	last
//			初期値として使用する最後の要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType*	last
//			初期値として使用する最後のデータが格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
ModHashMap<KeyType, MappedType, Hasher>::
ModHashMap(ModSize tableSize, const Hasher& func, ModBoolean enableLink)
	: ModHashTable<KeyType, ValueType, Hasher>(tableSize, func, enableLink)
{ }

template <class KeyType, class MappedType, class Hasher>
inline
ModHashMap<KeyType, MappedType, Hasher>::
ModHashMap(ModSize tableSize, ModBoolean enableLink)
	: ModHashTable<KeyType, ValueType, Hasher>(tableSize, Hasher(), enableLink)
{ }

template <class KeyType, class MappedType, class Hasher>
inline
ModHashMap<KeyType, MappedType, Hasher>::
ModHashMap(Iterator first, const Iterator& last,
		   ModSize tableSize, const Hasher& func, ModBoolean enableLink)
	: ModHashTable<KeyType, ValueType, Hasher>(first, last,
											   tableSize, func, enableLink)
{ }

template <class KeyType, class MappedType, class Hasher>
inline
ModHashMap<KeyType, MappedType, Hasher>::
ModHashMap(ConstIterator first, const ConstIterator& last,
		   ModSize tableSize, const Hasher& func, ModBoolean enableLink)
	: ModHashTable<KeyType, ValueType, Hasher>(first, last,
											   tableSize, func, enableLink)
{ }

template <class KeyType, class MappedType, class Hasher>
inline
ModHashMap<KeyType, MappedType, Hasher>::
ModHashMap(const ValueType* first, const ValueType* last,
		   ModSize tableSize, const Hasher& func, ModBoolean enableLink)
	: ModHashTable<KeyType, ValueType, Hasher>(first, last,
											   tableSize, func, enableLink)
{ }

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::insert -- 新しい値を挿入する(1)
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType&	value
//			挿入する値
//		KeyType&			key
//			挿入する値のキー
//		MappedType&			mappedValue
//			挿入する値のバリュー
//		ModBoolean			noCheckFlag
//			ModTrue
//				与えられたキーの値が存在するか調べない
//			ModFalse または指定しないとき
//				与えられたキーの値が存在するか調べる
//
//	RETURN
//		与えられたキーの値が存在を調べないとき、または存在しないとき
//			挿入された値を指す反復子と ModTrue の組
//		与えられたキーの値が存在するとき
//			存在する値を指す反復子と ModFalse の組
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
ModPair<ModTypename ModHashMap<KeyType, MappedType, Hasher>::Iterator, ModBoolean>
ModHashMap<KeyType, MappedType, Hasher>::
insert(const ValueType& value, ModBoolean noCheckFlag)
{
	return HashTable::insert(value, noCheckFlag);
}

template <class KeyType, class MappedType, class Hasher>
inline
ModPair<ModTypename ModHashMap<KeyType, MappedType, Hasher>::Iterator, ModBoolean>
ModHashMap<KeyType, MappedType, Hasher>::
insert(const KeyType& key,
	   const MappedType& mappedValue, ModBoolean noCheckFlag)
{
	return HashTable::insert(ValueType(key, mappedValue), noCheckFlag);
}

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::insert -- 新しい値を挿入する(2)
//	
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator	first
//			最初に挿入する要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ConstIterator	first
//			最初に挿入する要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType*	first
//			最初に挿入するデータが格納されている領域の先頭アドレス
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator&	last
//			最後に挿入する要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ConstIterator&	last
//			最後に挿入する要素を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType*	last
//			最後に挿入するデータが格納されている領域の先頭アドレス
//		ModBoolean			noCheckFlag
//			ModTrue
//				与えられたキーの値が存在するか調べない
//			ModFalse または指定しないとき
//				与えられたキーの値が存在するか調べる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
void
ModHashMap<KeyType, MappedType, Hasher>::
insert(Iterator first, const Iterator& last, ModBoolean noCheckFlag)
{
	HashTable::insert(first, last, noCheckFlag);
}

template <class KeyType, class MappedType, class Hasher>
inline
void
ModHashMap<KeyType, MappedType, Hasher>::
insert(ConstIterator first, const ConstIterator& last, ModBoolean noCheckFlag)
{
	HashTable::insert(first, last, noCheckFlag);
}

template <class KeyType, class MappedType, class Hasher>
inline
void
ModHashMap<KeyType, MappedType, Hasher>::
insert(const ValueType* first, const ValueType* last, ModBoolean noCheckFlag)
{
	HashTable::insert(first, last, noCheckFlag);
}

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::insert -- 新しい値を挿入する(3)
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator	position
//			値の挿入位置を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ValueType&	value
//			挿入する値
//		ModBoolean			noCheckFlag
//			ModTrue
//				与えられたキーの値が存在するか調べない
//			ModFalse または指定しないとき
//				与えられたキーの値が存在するか調べる
//
//	RETURN
//		与えられたキーの値が存在を調べないとき、または存在しないとき
//			挿入された値を指す反復子
//		与えられたキーの値が存在するとき
//			存在する値を指す反復子
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
ModTypename ModHashMap<KeyType, MappedType, Hasher>::Iterator
ModHashMap<KeyType, MappedType, Hasher>::
insert(Iterator position, const ValueType& value, ModBoolean noCheckFlag)
{
	return HashTable::insert(value, noCheckFlag).first;
}

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::operator [] -- [] 演算子
//		キーのバリューを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			バリューを得るキー
//
//	RETURN
//		与えられたキーの値が存在しないとき
//			バリューのデフォルト値
//		与えられたキーの値が存在するとき
//			存在する値のバリュー
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
MappedType&
ModHashMap<KeyType, MappedType, Hasher>::operator [](const KeyType& key)
{
	const ModPair<Iterator, ModBoolean>& pair =
		HashTable::insert(ValueType(key, MappedType()));
	return getValue(pair.first);
}

template <class KeyType, class MappedType, class Hasher>
inline
const MappedType&
ModHashMap<KeyType, MappedType, Hasher>::operator [](const KeyType& key) const
{
	ConstIterator	iterator = this->find(key);
	if (iterator == HashTable::end()) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
	return getValue(iterator);
}

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::getValue --
//		反復子の指す値のバリューを取り出す
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashMap<KeyType, MappedType, Hasher>::Iterator&	iterator
//			バリューを取り出す値を指す反復子
//		ModHashMap<KeyType, MappedType, Hasher>::ConstIterator&	iterator
//			バリューを取り出す値を指す定数反復子
//
//	RETURN
//		取り出した反復子の指す値のバリュー
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
MappedType&
ModHashMap<KeyType, MappedType, Hasher>::getValue(const Iterator& iterator)
{
	//【注意】	end を指す反復子の値も取り出せる

	return (*iterator).second;
}

template <class KeyType, class MappedType, class Hasher>
inline
const MappedType&
ModHashMap<KeyType, MappedType, Hasher>::
getValue(const ConstIterator& iterator)
{
	//【注意】	end を指す反復子の値も取り出せる

	return (*iterator).second;
}

//	TEMPLATE FUNCTION public
//	ModHashMap<KeyType, MappedType, Hasher>::getFront --
//		先頭の値のバリューを取り出す
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取り出した先頭の値のバリュー
//
//	EXCEPTIONS

template <class KeyType, class MappedType, class Hasher>
inline
const MappedType&
ModHashMap<KeyType, MappedType, Hasher>::getFront() const
{
	return getValue(this->begin());
}

#endif	// __ModHashMap_H__

//
// Copyright (c) 1997, 1999, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
