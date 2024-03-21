// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMap.h -- ModMap のクラス定義
// 
// Copyright (c) 1997, 2001, 2002, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModMap_H__
#define __ModMap_H__

#include "ModCommon.h"
#include "ModPair.h"
#include "ModTree.h"

//
// TEMPLATE CLASS
// ModMap -- マップを表現するテンプレート機能クラス
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// STLに代わり、マップを表現するテンプレート機能クラスである。
// 実際にはメモリハンドルを明示したテンプレートクラスModMapを使う。
// 要望に寄せられる最低限の動作だけを提供している。
// tree を使う。
//

// ModPureMapクラスを作成し、継承するメモリハンドル明示クラスを
// 作成したがうまくいかなかったので
// 直接、ModDefaultObjectのサブクラスとして作成する。

template <class KeyType, class MappedType, class Compare >
class ModMap
	: public	ModDefaultObject
{
public:
	typedef ModPair<KeyType, MappedType > ValueType;

	typedef ModTreeIterator<ValueType > Iterator;
	typedef ModTreeConstIterator<ValueType > ConstIterator;

	// デフォルトコンストラクタ
	ModMap();
	// コンストラクタ (1)
	ModMap(const Compare& compare);
	// コンストラクタ (2)
	ModMap(const ValueType* first, const ValueType* last,
		   const Compare& compare = Compare());
	// コンストラクタ (3)
	ModMap(ConstIterator first, ConstIterator last,
		   const Compare& compare = Compare());
	// コピーコンストラクタ
	ModMap(const ModMap<KeyType, MappedType, Compare >& original);
	// デストラクタ
	~ModMap();

	// 代入オペレータ
	ModMap<KeyType, MappedType, Compare >& operator=(const ModMap<KeyType, MappedType, Compare >& original);

	// key にマッチした value を指すイテレータを得る
	ConstIterator find(const KeyType& key) const;
	Iterator find(const KeyType& key);

	// 下限検索
	ConstIterator lowerBound(const KeyType& key) const;
	Iterator lowerBound(const KeyType& key);
	// 上限検索
	ConstIterator upperBound(const KeyType& key) const;
	Iterator upperBound(const KeyType& key);

	// エントリを挿入する
	ModPair<Iterator, ModBoolean > insert(const ValueType& entry);
	ModPair<Iterator, ModBoolean > insert(const KeyType& key,
										  const MappedType& mappedValue);
	Iterator insert(Iterator position, const ValueType& entry);
	void insert(const ValueType* first, const ValueType* last);
	void insert(ConstIterator first, ConstIterator last);
	void insert(Iterator first, Iterator last);

	// key にマッチしたものを消す
	ModSize erase(const KeyType& key);
	// 場所を指定して消す
	void erase(Iterator position);
	// イテレータの範囲を消す
	void erase(Iterator first, Iterator last);

	// key にマッチした value への参照を得る
	MappedType& operator[](const KeyType& key);
	const MappedType& operator[](const KeyType& key) const;

	Iterator begin();
	Iterator end();
	ConstIterator begin() const;
	ConstIterator end() const;

	// アクセサ関数
	ModBoolean	isEmpty() const;
	ModSize getSize() const;

protected:
	// Key-Value ペアを二分木で保持する。
	ModTree<KeyType, ValueType, Compare > tree;
};

//
// TEMPLATE FUNCTION
// ModMap::ModMap -- デフォルトコンストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap のデフォルトコンストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::ModMap() :
	tree(Compare())
{
}

//
// TEMPLATE FUNCTION
// ModMap::ModMap -- 比較関数を指定したコンストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap の Compare を指定したコンストラクタ
//
// ARGUMENTS
// const Compare& compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::ModMap(const Compare& compare) :
	tree(compare)
{
}

//
// TEMPLATE FUNCTION
// ModMap::ModMap -- 初期値をポインターで指定したコンストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap の、初期値と Compare を指定したコンストラクタ
//
// ARGUMENTS
// const ValueType* first
//		初期値に使う ValueType 配列の開始位置を指すポインタ
// const ValueType* last
//		初期値に使う ValueType 配列の終了位置を指すポインタ
// const Compare& compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::ModMap(const ValueType* first,
											  const ValueType* last,
											  const Compare& compare) :
	tree(first, last, compare)
{
}

//
// TEMPLATE FUNCTION
// ModMap::ModMap -- 初期値をイテレータで指定したコンストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap の、初期値と Compare を指定したコンストラクタ
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::ConstIterator first
//		初期値に使う ValueType 配列の開始位置を指すイテレータ
// ModMap<KeyType, MappedType, Compare >::ConstIterator last
//		初期値に使う ValueType 配列の終了位置を指すイテレータ
// const Compare& compare
//		比較関数
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::ModMap(ConstIterator first,
											 ConstIterator last,
											 const Compare& compare)
	: tree(first, last, compare)
{
}

//
// TEMPLATE FUNCTION
// ModMap::ModMap -- コピーコンストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap のコピーコンストラクタ
//
// ARGUMENTS
// const ModMap& original
//		コピー元のデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::ModMap(const ModMap<KeyType, MappedType, Compare >& original)
	: tree(original.tree)
{
}

//
// TEMPLATE FUNCTION
// ModMap::~ModMap -- デストラクタ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap のデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModMap<KeyType, MappedType, Compare >::~ModMap()
{
}

//
// TEMPLATE FUNCTION
// ModMap::operator= -- 代入オペレータ
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// ModMap の代入オペレータ
//
// ARGUMENTS
// const ModMap& original
//		代入元のデータへの参照
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModMap<KeyType, MappedType, Compare >&
ModMap<KeyType, MappedType, Compare >::operator=(const ModMap<KeyType, MappedType, Compare >& original)
{
	this->tree = original.tree;
	return *this;
}

//
// TEMPLATE FUNCTION
// ModMap::find -- キー値による検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() const の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::ConstIterator
ModMap<KeyType, MappedType, Compare >::find(const KeyType& key) const
{
	return this->tree.find(key);
}

//
// TEMPLATE FUNCTION
// ModMap::find -- キー値による検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::find(const KeyType& key)
{
	return this->tree.find(key);
}

//
// TEMPLATE FUNCTION
// ModMap::lowerBound -- キー値による下限検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って
// 下限検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() const の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::ConstIterator
ModMap<KeyType, MappedType, Compare >::lowerBound(const KeyType& key) const
{
	return this->tree.lowerBound(key);
}

//
// TEMPLATE FUNCTION
// ModMap::lowerBound -- キー値による下限検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って
// 下限検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::lowerBound(const KeyType& key)
{
	return this->tree.lowerBound(key);
}

//
// TEMPLATE FUNCTION
// ModMap::upperBound -- キー値による上限検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って
// 上限検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() const の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::ConstIterator
ModMap<KeyType, MappedType, Compare >::upperBound(const KeyType& key) const
{
	return this->tree.upperBound(key);
}

//
// TEMPLATE FUNCTION
// ModMap::upperBound -- キー値による上限検索
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に挿入されているデータをキー値を使って
// 上限検索するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索対象のキー値
//
// RETURN
// 見つかった場合は該当するデータを指すイテレータを返す。
// 見つからなかった場合は end() の返り値と同じものを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::upperBound(const KeyType& key)
{
	return this->tree.upperBound(key);
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- ペアをマップへ挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に key と value のペアで指定したデータを挿入する。
//
// ARGUMENTS
// const ModMap<KeyType, MappedType, Compare >::ValueType& entry
//		挿入するデータへの参照
//
// RETURN
// マップ中に key にマッチするエントリがない場合は entry のコピーを
// マップに挿入してそのイテレータと ModTrue のペアを返す。
// マップ中に key にマッチするエントリがすでにある場合は
// そのエントリを指すイテレータと ModFalse のペアを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModPair<ModTypename ModMap<KeyType, MappedType, Compare >::Iterator, ModBoolean >
ModMap<KeyType, MappedType, Compare >::insert(const ValueType& entry)
{
	return this->tree.insert(entry);
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- キー値とバリュー値をマップへ挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に key と value をそれぞれ指定したデータを
// 挿入するのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		挿入したいデータのキー値
// const MappedType& mappedValue
//		挿入したいデータのバリュー値
//
// RETURN
// マップ中に key にマッチするエントリがない場合は entry のコピーを
// マップに挿入してそのイテレータと ModTrue のペアを返す。
// マップ中に key にマッチするエントリがすでにある場合は
// そのエントリを指すイテレータと ModFalse のペアを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline
ModPair<ModTypename ModMap<KeyType, MappedType, Compare >::Iterator, ModBoolean >
ModMap<KeyType, MappedType, Compare >::
insert(const KeyType& key, const MappedType& mappedValue)
{
	return this->tree.insert(ValueType(key, mappedValue));
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- マップの指定した位置以降にペアを挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は指定した位置以降にキー値とバリュー値のペアで指定したデータを
// 挿入するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::Iterator position
//		データを挿入する位置を探す開始位置指すイテレータ
// ModMap<KeyType, MappedType, Compare >::ValueType& entry
//		挿入するキー値とバリュー値のペア
//
// RETURN
// 挿入した位置を指すイテレータを返す。
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::insert(Iterator position,
											 const ValueType& entry)
{
	return this->tree.insert(position, entry);
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- ポインタで指定した範囲をマップへ挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数はポインタで指定した範囲にあるデータを複数挿入するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::ValueType* first
//		挿入するキー値とバリュー値のペア配列の開始位置
// ModMap<KeyType, MappedType, Compare >::ValueType* last
//		挿入するキー値とバリュー値のペア配列の終了位置
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline void
ModMap<KeyType, MappedType, Compare >::insert(const ValueType* first,
											 const ValueType* last)
{
	this->tree.insert(first, last);
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- イテレータで指定した範囲をマップへ挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数はイテレータで指定した範囲にあるデータを複数挿入するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::ConstIterator first
//		挿入するキー値とバリュー値のペア配列の開始位置を指すイテレータ
// ModMap<KeyType, MappedType, Compare >::ConstIterator last
//		挿入するキー値とバリュー値のペア配列の終了位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline void
ModMap<KeyType, MappedType, Compare >::insert(ConstIterator first,
											  ConstIterator last)
{
	this->tree.insert(first, last);
}

//
// TEMPLATE FUNCTION
// ModMap::insert -- イテレータで指定した範囲をマップへ挿入する
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数はイテレータで指定した範囲にあるデータを複数挿入するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::Iterator first
//		挿入するキー値とバリュー値のペア配列の開始位置を指すイテレータ
// ModMap<KeyType, MappedType, Compare >::Iterator last
//		挿入するキー値とバリュー値のペア配列の終了位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline void
ModMap<KeyType, MappedType, Compare >::insert(Iterator first, Iterator last)
{
	this->tree.insert(first, last);
}

//
// TEMPLATE FUNCTION
// ModMap::erase -- key にマッチしたものをマップから消す
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に保持されているデータからキー値が key に
// 一致するものを削除するために用いる。
//
// ARGUMENTS
// const KeyType& key
//		消去するデータのキー値
//
// RETURN
// key にマッチし、消去されたデータの個数
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModSize
ModMap<KeyType, MappedType, Compare >::erase(const KeyType& key)
{
	return this->tree.erase(key);
}

//
// TEMPLATE FUNCTION
// ModMap::erase -- 指定した場所の要素をマップから消す
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数はイテレータで指定したデータを消去するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::Iterator position
//		消去するデータの位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline void
ModMap<KeyType, MappedType, Compare >::erase(Iterator position)
{
	this->tree.erase(position);
}

//
// TEMPLATE FUNCTION
// ModMap::erase -- イテレータで指定した範囲をマップから消す
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap に保持されているデータのうち、
// イテレータで指定した範囲を消去するのに用いる。
//
// ARGUMENTS
// ModMap<KeyType, MappedType, Compare >::Iterator first
//		消去する範囲の開始位置を指すイテレータ
// ModMap<KeyType, MappedType, Compare >::Iterator last
//		消去する範囲の終了位置を指すイテレータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
template <class KeyType, class MappedType, class Compare >
inline void
ModMap<KeyType, MappedType, Compare >::erase(Iterator first, Iterator last)
{
	this->tree.erase(first, last);
}

//
// TEMPLATE FUNCTION
// ModMap::operator[] -- key にマッチした value への参照を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap の key に合致するデータを得るのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索に使うキー値
//
// RETURN
// キー値が key に合致するものがあればその値を、なければ MappedType() を
// 返す。
//
// EXCEPTIONS
// なし
//

template <class KeyType, class MappedType, class Compare >
inline
MappedType&
ModMap<KeyType, MappedType, Compare >::operator[](const KeyType& key)
{
	return (*this->insert(key, MappedType()).first).second;
}

//
// TEMPLATE FUNCTION
// ModMap::operator[] -- key にマッチした value への参照(const)を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
// class Compare
//		KeyType に対する比較関数を提供するクラス
//
// NOTES
// この関数は ModMap の key に合致するデータを得るのに用いる。
//
// ARGUMENTS
// const KeyType& key
//		検索に使うキー値
//
// RETURN
// キー値が key に合致するものがある場合はそのデータへの参照を返す。
// ない場合は例外を発生する。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		key に合致するデータがない
//

template <class KeyType, class MappedType, class Compare >
inline
const MappedType&
ModMap<KeyType, MappedType, Compare >::operator[](const KeyType& key) const
{
	ConstIterator element(this->find(key));
	if (element == this->end()) {
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
	return (*element).second;
}

//
// TEMPLATE FUNCTION
// ModMap::begin -- マップの要素の開始位置を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
//
// NOTES
// この関数はModMapの要素の開始位置を指すイテレータを得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// マップの要素の開始位置を指すイテレータを返す。
//
// EXCEPTIONS
// その他
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::begin()
{
	return this->tree.begin();
}

//
// TEMPLATE FUNCTION
// ModMap::end -- マップの要素の終了位置を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
//
// NOTES
// この関数はModMapの要素の終了位置を指すイテレータを得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// マップの要素の終了位置を指すイテレータを返す。
//
// EXCEPTIONS
// その他
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::Iterator
ModMap<KeyType, MappedType, Compare >::end()
{
	return this->tree.end();
}

//
// TEMPLATE FUNCTION
// ModMap::begin -- マップの要素の開始位置を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
//
// NOTES
// この関数はModMapの要素の開始位置を指すイテレータを得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// マップの要素の開始位置を指すイテレータを返す。
//
// EXCEPTIONS
// その他
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::ConstIterator
ModMap<KeyType, MappedType, Compare >::begin() const
{
	return this->tree.begin();
}

//
// TEMPLATE FUNCTION
// ModMap::end -- マップの要素の終了位置を得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
//
// NOTES
// この関数はModMapの要素の終了位置を指すイテレータを得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// マップの要素の終了位置を指すイテレータを返す。
//
// EXCEPTIONS
// その他
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModTypename ModMap<KeyType, MappedType, Compare >::ConstIterator
ModMap<KeyType, MappedType, Compare >::end() const
{
	return this->tree.end();
}

//	TEMPLATE FUNCTION public
//	ModMap<KeyType, MappedType, Compare>::isEmpty -- 空か調べる
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			マップに登録する値のキーの型
//		class MappedType
//			マップに登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			空である
//		ModFalse
//			空でない
//
//	EXCEPTIONS
//		なし

template <class KeyType, class MappedType, class Compare >
inline ModBoolean
ModMap<KeyType, MappedType, Compare >::isEmpty() const
{
	return (getSize()) ? ModFalse : ModTrue;
}

//
// TEMPLATE FUNCTION
// ModMap::getSize -- マップのサイズを得る
//
// TEMPLATE ARGUMENTS
// class KeyType
//		マップのキー値に使うクラス
// class MappedType
//		マップのバリュー値に使うクラス
//
// NOTES
// この関数はModMapのprivateメンバ size の値を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// size の値を返す。
//
// EXCEPTIONS
// その他
// なし
//
template <class KeyType, class MappedType, class Compare >
inline ModSize
ModMap<KeyType, MappedType, Compare >::getSize() const
{
	return this->tree.getSize();
}

#endif	// __ModMap_H__

//
// Copyright (c) 1997, 2001, 2002, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
