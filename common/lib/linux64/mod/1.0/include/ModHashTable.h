// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModHashTable.h -- ModHashTable に関係するクラス定義
// 
// Copyright (c) 1997, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModHashTable_H__
#define __ModHashTable_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModException.h"
#include "ModPair.h"

//	CLASS
//	ModHashTableBase -- ハッシュ表を表すクラスの基底クラス
//
//	NOTES

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModHashTableBase
	: public	ModDefaultObject
{
public:
	ModCommonDLL
	static ModSize			verifySize(ModSize n);
												// ハッシュ表のサイズを
												// それ以上の素数のうち、
												// 最小のものにする
private:
	static const ModSize	_primeTable[];		// 素数表 
	static const ModSize	_primeTableSize;	// 素数表のサイズ
};

//	TEMPLATE CLASS
//	ModHashNode -- ハッシュ表の各ノードを表すクラス
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ハッシュ表に登録する値の型
//
//	NOTES

template <class ValueType>
class ModHashNode
	: public	ModDefaultObject
{
public:
	ModHashNode();
	ModHashNode(ValueType& data, ModHashNode<ValueType>* next = 0);
												// コンストラクター

	ValueType				_data;				// 値
	ModHashNode<ValueType>*	_next;				// 次ノード
};

//	TEMPLATE FUNCTION public
//	ModHashNode<ValueType>::ModHashNode --
//		ハッシュ表の各ノードを表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ハッシュ表に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			data
//			指定されたとき
//				ノードの持つ値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//		ModHashNode<ValueType>*	next
//			指定されたとき
//				次のノード
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
ModHashNode<ValueType>::ModHashNode()
	: _next(0)
{ }

template <class ValueType>
inline
ModHashNode<ValueType>::
ModHashNode(ValueType& data, ModHashNode<ValueType>* next)
	: _data(data),
	  _next(next)
{ }

template <class KeyType, class ValueType, class Hasher>
class ModHashTable;

//	TEMPLATE CLASS
//	ModHashTableIterator -- ハッシュ表上の反復子を表すクラス
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES

template <class KeyType, class _ValueType, class Hasher>
class ModHashTableIterator
	: public	ModDefaultObject
{
public:
	typedef	ModHashTable<KeyType, _ValueType, Hasher>	HashTable;
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType									ValueType;
#endif

	ModHashTableIterator(ModHashNode<_ValueType>* node = 0,
						 HashTable* table = 0);
	ModHashTableIterator(
		const ModHashTableIterator<KeyType, _ValueType, Hasher>& src);
												// コンストラクター

	ModHashTableIterator<KeyType, _ValueType, Hasher>&	operator =(
		const ModHashTableIterator<KeyType, _ValueType, Hasher>& src);
												// = 演算子
	_ValueType&				operator *() const;	// * 演算子
	ModHashTableIterator<KeyType, _ValueType, Hasher>&
		operator ++();							// ++ 前置演算子
	ModHashTableIterator<KeyType, _ValueType, Hasher>
		operator ++(int dummy);					// ++ 後置演算子
	ModBoolean				operator ==(
		const ModHashTableIterator<KeyType, _ValueType, Hasher>& r) const;
												// == 演算子
	ModBoolean				operator !=(
		const ModHashTableIterator<KeyType, _ValueType, Hasher>& r) const;
												// != 演算子

	ModHashNode<_ValueType>* getNode() const;	// 反復子の指すノードを得る

private:
	ModHashNode<_ValueType>*	next() const;		// 反復子の指すノードの
												// バケットの後で最初に
												// 登録されているノードを得る

	ModHashNode<_ValueType>*	_node;				// 反復子の指すノード
	HashTable*				_table;				// 反復子の指すノードが
												// 登録されているハッシュ表
};

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::ModHashTableIterator --
//		ハッシュ表の反復子を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashNode<_ValueType>*	node
//			指定されたとき
//				反復子が指すノード
//			指定されないとき
//				0 が指定されたものとみなす
//		ModHashTableIterator<KeyType, _ValueType, Hasher>::HashTable* table
//			指定されたとき
//				反復子が指すノードが登録されているハッシュ表
//			指定されないとき
//				0 が指定されたものとみなす
//		ModHashTableIterator<KeyType, _ValueType, Hasher>& src
//			コピーする反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableIterator<KeyType, _ValueType, Hasher>::
ModHashTableIterator(ModHashNode<_ValueType>* node, HashTable* table)
	: _node(node),
	  _table(table)
{ }

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableIterator<KeyType, _ValueType, Hasher>::
ModHashTableIterator(
	const ModHashTableIterator<KeyType, _ValueType, Hasher>& src)
	: _node(src._node),
	  _table(src._table)
{ }

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableIterator<KeyType, _ValueType, Hasher>&	src
//			自分自身に代入する反復子
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			削除されているノードを指す反復子を代入しようとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableIterator<KeyType, _ValueType, Hasher>&
ModHashTableIterator<KeyType, _ValueType, Hasher>::
operator =(const ModHashTableIterator<KeyType, _ValueType, Hasher>& src)
{
	if (src._node && src._node->_next == src._node)

		// 削除されているノードを指す反復子が与えられた

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	_node = src._node;
	_table = src._table;

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::operator * -- * 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指すノードが持つ値への参照
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
_ValueType&
ModHashTableIterator<KeyType, _ValueType, Hasher>::operator *() const
{
	return (_node) ? _node->_data : _table->buckets[_table->length]->_data;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyTYpe, _ValueType, Hasher>::operator ++ --
//		++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次のノードを指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorOutOfRange
//			end を指しているのに次を指そうとした
//		ModCommonErrorBadArgument
//			削除されているノードを指しているのに次を指そうとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableIterator<KeyType, _ValueType, Hasher>&
ModHashTableIterator<KeyType, _ValueType, Hasher>::operator ++()
{
	if (_node == 0)

		// end を指す反復子を先に進めようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange, ModErrorLevelError);

	if (_node->_next == _node)

		// 削除されているノードを指す反復子を操作しようとした

		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);

	_node = (_node->_next) ? _node->_next : this->next();

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::operator ++ --
//		++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		int					dummy
//			後置演算子であることを表す引数
//
//	RETURN
//		次のノードを指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorOutOfRange
//			end を指しているのに次を指そうとした
//		ModCommonErrorBadArgument
//			削除されているノードを指しているのに次を指そうとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableIterator<KeyType, _ValueType, Hasher>
ModHashTableIterator<KeyType, _ValueType, Hasher>::operator ++(int dummy)
{
	if (_node == 0)

		// end を指す反復子を先に進めようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange, ModErrorLevelError);

	if (_node->_next == _node)

		// 削除されているノードを指す反復子を操作しようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	ModHashTableIterator<KeyType, _ValueType, Hasher> saved(*this);

	_node = (_node->_next) ? _node->_next : this->next();

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableIterator<KeyType, _ValueType, Hasher>&	r
//			自分自身と比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子と自分自身は同じノードを指している
//		ModFalse
//			与えられた反復子と自分自身は違うノードを指している
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModBoolean
ModHashTableIterator<KeyType, _ValueType, Hasher>::
operator ==(const ModHashTableIterator<KeyType, _ValueType, Hasher>& r) const
{
	return (_node == r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableIterator<KeyType, _ValueType, Hasher>&	r
//			自分自身と比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子と自分自身は違うノードを指している
//		ModFalse
//			与えられた反復子と自分自身は同じノードを指している
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModBoolean
ModHashTableIterator<KeyType, _ValueType, Hasher>::
operator !=(const ModHashTableIterator<KeyType, _ValueType, Hasher>& r) const
{
	return (_node != r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::getNode --
//		反復子の指すノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指すノードを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashNode<_ValueType>* 
ModHashTableIterator<KeyType, _ValueType, Hasher>::getNode() const
{
	return _node;
}

//	TEMPLATE FUNCTION private
//	ModHashTableIterator<KeyType, _ValueType, Hasher>::next --
//		今のバケットの後で最初に登録されているノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		反復子が指しているノードが登録されているバケットより後のバケットで、
//		最初に登録されているノードを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外
//			得られたノードを格納する領域の先頭アドレス
//		0
//			存在しない
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
ModHashNode<_ValueType>*
ModHashTableIterator<KeyType, _ValueType, Hasher>::next() const
{
	ModSize i = (_table->hasher(_node->_data.first) % _table->length) + 1;
	if (_table->links == 0) {

		// なめて探す

		for (; i < _table->length; i++)
			if (_table->buckets[i])
				return _table->buckets[i];

	} else if (_table->links[i])

		// リンク情報を使う
		// リンク情報にはバケット + 1 で格納されている

		return _table->buckets[_table->links[i] - 1];

	return 0;
}

//	TEMPLATE CLASS
//	ModHashTableConstIterator -- ハッシュ表上の読み出し専用反復子を表すクラス
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES

template <class KeyType, class _ValueType, class Hasher>
class ModHashTableConstIterator
	: public	ModDefaultObject
{
public:
	typedef	ModHashTable<KeyType, _ValueType, Hasher>	HashTable;
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType									ValueType;
#endif

	ModHashTableConstIterator(const ModHashNode<_ValueType>* node = 0,
							  const HashTable* table = 0);
	ModHashTableConstIterator(
		const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& src);
												// コンストラクター

	ModHashTableConstIterator<KeyType, _ValueType, Hasher>&	operator =(
		const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& src);
												// = 演算子
	const _ValueType&		operator *() const;	// * 演算子
	ModHashTableConstIterator<KeyType, _ValueType, Hasher>&
		operator ++();							// ++ 前置演算子
	ModHashTableConstIterator<KeyType, _ValueType, Hasher>
		operator ++(int dummy);					// ++ 後置演算子
	ModBoolean				operator ==(
		const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& r) const;
												// == 演算子
	ModBoolean				operator !=(
		const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& r) const;
												// != 演算子

	const ModHashNode<_ValueType>* getNode() const;
												// 反復子の指すノードを得る
private:
	const ModHashNode<_ValueType>* next() const;	// 反復子の指すノードの
												// バケットの後で最初に
												// 登録されているノードを得る

	const ModHashNode<_ValueType>*	_node;		// 反復子の指すノード
	const HashTable*		_table;				// 反復子の指すノードが
												// 登録されているハッシュ表
};

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::ModHashTableConstIterator --
//		ハッシュ表の読み出し専用反復子を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashNode<_ValueType>*	node
//			指定されたとき
//				反復子が指すノード
//			指定されないとき
//				0 が指定されたものとみなす
//		ModHashTableIterator<KeyType, _ValueType, Hasher>::HashTable* table
//			指定されたとき
//				反復子が指すノードが登録されているハッシュ表
//			指定されないとき
//				0 が指定されたものとみなす
//		ModHashTableConstIterator<KeyType, _ValueType, Hasher>&	src
//			コピーする反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::
ModHashTableConstIterator(const ModHashNode<_ValueType>* node,
						  const HashTable* table)
	: _node(node),
	  _table(table)
{ }

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::
ModHashTableConstIterator(
	const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& src)
	: _node(src._node),
	  _table(src._table)
{ }

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator = --
//		= 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableConstIterator<KeyType, _ValueType, Hasher>&	src
//			自分自身に代入する反復子
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			削除されているノードを指す反復子を代入しようとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableConstIterator<KeyType, _ValueType, Hasher>&
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::
operator =(const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& src)
{
	if (src._node && src._node->_next == src._node)

		// 削除されているノードを指す反復子が与えられた

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	_node = src._node;
	_table = src._table;

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator * --
//		* 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指すノードが持つ値への参照
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
const _ValueType&
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator *() const
{
	return (_node) ? _node->_data : _table->buckets[_table->length]->_data;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator ++ --
//		++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次のノードを指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorOutOfRange
//			end を指しているのに次を指そうとした
//		ModCommonErrorBadArgument
//			削除されているノードを指しているのに次を指そうとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableConstIterator<KeyType, _ValueType, Hasher>&
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator ++()
{
	if (_node == 0)

		// end を指す反復子を先に進めようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange, ModErrorLevelError);

	if (_node->_next == _node)

		// 削除されているノードを指す反復子を操作しようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	_node = (_node->_next) ? _node->_next : this->next();

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator ++ --
//		++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		int					dummy
//			後置演算子であることを表す引数
//
//	RETURN
//		次のノードを指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorOutOfRange
//			end を指しているのに次を指そうとした
//		ModCommonErrorBadArgument
//			削除されているノードを指しているのに次を指そうとした

template <class KeyType, class _ValueType, class Hasher>
inline
ModHashTableConstIterator<KeyType, _ValueType, Hasher>
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator ++(int dummy)
{
	if (_node == 0)

		// end を指す反復子を先に進めようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorOutOfRange, ModErrorLevelError);

	if (_node->_next == _node)

		// 削除されているノードを指す反復子を操作しようとした

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	ModHashTableConstIterator<KeyType, _ValueType, Hasher> saved(*this);

	_node = (_node->_next) ? _node->_next : this->next();

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator == --
//		== 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableConstIterator<KeyType, _ValueType, Hasher>&	r
//			自分自身と比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子と自分自身は同じノードを指している
//		ModFalse
//			与えられた反復子と自分自身は違うノードを指している
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModBoolean
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::
operator ==(
	const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& r) const
{
	return (_node == r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::operator != --
//		!= 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableConstIterator<KeyType, _ValueType, Hasher>&	r
//			自分自身と比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子と自分自身は違うノードを指している
//		ModFalse
//			与えられた反復子と自分自身は同じノードを指している
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
ModBoolean
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::
operator !=(
	const ModHashTableConstIterator<KeyType, _ValueType, Hasher>& r) const
{
	return (_node != r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::getNode --
//		反復子の指すノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指すノードを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
inline
const ModHashNode<_ValueType>* 
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::getNode() const
{
	return _node;
}

//	TEMPLATE FUNCTION private
//	ModHashTableConstIterator<KeyType, _ValueType, Hasher>::next --
//		今のバケットの後で最初に登録されているノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class _ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		反復子が指しているノードが登録されているバケットより後のバケットで、
//		最初に登録されているノードを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外
//			得られたノードを格納する領域の先頭アドレス
//		0
//			存在しない
//
//	EXCEPTIONS
//		なし

template <class KeyType, class _ValueType, class Hasher>
const ModHashNode<_ValueType>*
ModHashTableConstIterator<KeyType, _ValueType, Hasher>::next() const
{
	ModSize i = (_table->hasher(_node->_data.first) % _table->length) + 1;
	if (_table->links == 0) {

		// なめて探す

		for (; i < _table->length; i++)
			if (_table->buckets[i])
				return _table->buckets[i];

	} else if (_table->links[i])

		// リンク配列を使う
		// リンク情報にはバケット + 1 で格納されている

		return _table->buckets[_table->links[i] - 1];

	return 0;
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
//	TEMPLATE FUNCTION
//	ModValueType -- 反復子の指すノードが持つ値の型を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTableIterator<KeyType, ValueType, Hasher>&	dummy
//			調べる反復子
//		ModHashTableConstIterator<KeyType, ValueType, Hasher>&	dummy
//			調べる反復子
//
//	RETURN
//		求めた型へのポインターの 0
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
ValueType*
ModValueType(const ModHashTableIterator<KeyType, ValueType, Hasher>& dummy)
{
	return static_cast<ValueType*>(0);
}

template <class KeyType, class ValueType, class Hasher>
inline
ValueType*
ModValueType(const ModHashTableConstIterator<KeyType, ValueType, Hasher>& dummy)
{
	return static_cast<ValueType*>(0);
}
#endif

//	CONST
//	ModHashTableSizeDefault -- ハッシュ表のデフォルトの大きさ
//
//	NOTES

const ModSize				ModHashTableSizeDefault = 203;

//	TEMPLATE CLASS
//	ModHashTable -- ハッシュ表を表すクラス
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES

template <class KeyType, class ValueType, class Hasher>
class ModHashTable
	: public	ModHashTableBase
{
	friend class ModHashTableIterator<KeyType, ValueType, Hasher>;
	friend class ModHashTableConstIterator<KeyType, ValueType, Hasher>;
public:
	typedef ModHashTableIterator<KeyType, ValueType, Hasher> Iterator;
	typedef ModHashTableConstIterator<KeyType, ValueType, Hasher> ConstIterator;

	ModHashTable(ModSize tableSize = ModHashTableSizeDefault,
				 const Hasher& hasher_ = Hasher(),
				 ModBoolean enableLink = ModTrue);
	ModHashTable(Iterator first, const Iterator& last,
				 ModSize tableSize = ModHashTableSizeDefault,
				 const Hasher& hasher_ = Hasher(),
				 ModBoolean enableLink = ModTrue);
	ModHashTable(ConstIterator first, const ConstIterator& last,
				 ModSize tableSize = ModHashTableSizeDefault,
				 const Hasher& hasher_ = Hasher(),
				 ModBoolean enableLink = ModTrue);
	ModHashTable(const ValueType* first, const ValueType* last,
				 ModSize tableSize = ModHashTableSizeDefault,
				 const Hasher& hasher_ = Hasher(),
				 ModBoolean enableLink = ModTrue);
	ModHashTable(const ModHashTable<KeyType, ValueType, Hasher>& src);
												// コンストラクター
	~ModHashTable();							// デストラクター

	ModHashTable<KeyType, ValueType, Hasher>&
		operator =(const ModHashTable<KeyType, ValueType, Hasher>& src);
												// = 演算子

	ModBoolean				isEmpty() const;	// ハッシュ表は空か

	ModPair<Iterator, ModBoolean>
							insert(const ValueType& value,
								   ModBoolean notUnique = ModFalse);
	void					insert(const ValueType* first,
								   const ValueType* last,
								   ModBoolean notUnique = ModFalse);
	void					insert(Iterator first, const Iterator& last,
								   ModBoolean notUnique = ModFalse);
	void					insert(ConstIterator first,
								   const ConstIterator& last,
								   ModBoolean notUnique = ModFalse);
												// 挿入する

	void					erase(Iterator position);
	ModSize					erase(const KeyType& key);
	void					erase(Iterator first, const Iterator& last);
												// 削除する

	void					popFront();			// 先頭の値を削除する

	void					clear();			// ハッシュ表を空にする
	void					resize(ModSize newLen);
												// ハッシュ表の大きさを変更する

	void				splice(ModHashTable<KeyType, ValueType, Hasher>& src,
							   const Iterator& first);
	void				splice(ModHashTable<KeyType, ValueType, Hasher>& src,
							   const Iterator& first, const Iterator& last);
												// あるハッシュ表の
												// 要素を移動する
	// 以下、近日中に廃止します
	static void		move(ModHashTable<KeyType, ValueType, Hasher>& fromTable,
						 Iterator& from,
						 ModHashTable<KeyType, ValueType, Hasher>& toTable);
												// 反復子の指す要素を
												// 他のハッシュ表へ移動する
	// 以上、近日中に廃止します

	Iterator				find(const KeyType& key);
	ConstIterator			find(const KeyType& key) const;
												// 探す

	ValueType&				getFront();
	const ValueType&		getFront() const;	// 先頭の値を得る

	Iterator				begin();
	ConstIterator			begin() const;		// 先頭の値を指す反復子を得る
	const Iterator&			end();
	const ConstIterator&	end() const;		// end を指す反復子を得る

	ModSize					getSize() const;	// ハッシュ表中の値数を得る

	Hasher					getHasher() const;	// ハッシュ関数を得る

	void					printHist() const;	// バケット分布を出力する

private:
	ModHashNode<ValueType>*	first() const;		// 先頭の値を持つノードを得る

	ModBoolean				keyEqual(ModSize hashValue, const KeyType& key,
									 const ModHashNode<ValueType>* node) const;
												// ノードの持つ値が
												// キーにマッチするか

	void			copy(const ModHashTable<KeyType, ValueType, Hasher>& src);
												// ハッシュ表を複製する

	void					allocateNodePointer(
											ModBoolean enableLink = ModTrue);
												// バケット管理領域の確保
	void					freeNodePointer();	// バケット管理領域の破棄

	void					validateLinkForInsert(ModSize probe);
												// 挿入のためのリンク情報の更新
	void					validateLinkForErase(ModSize probe);
												// 削除のためのリンク情報の更新

	ModHashNode<ValueType>** buckets;			// バケットの先頭アドレス
	ModSize					length;				// ハッシュ表の大きさ
	Hasher					hasher;				// ハッシュ関数
	ModSize					size;				// ハッシュ表中の値の数
	ModSize*				links;				// リンク情報

	ModHashNode<ValueType>	nilNode;			// end を表すノード
	Iterator				endIterator;		// end を指す反復子
	ConstIterator			endConstIterator;	// end を指す読み出し専用反復子
};

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::ModHashTable --
//		初期値を指定しないハッシュ表を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		与えられたハッシュ表の大きさは、
//		ModHashTableBase::verifySize により適当な値に変更される
//
//	ARGUMENTS
//		ModSize				tableSize
//			指定されたとき
//				ハッシュ表の最初の大きさ
//			指定されないとき
//				ModHashTableSizeDefault が指定されたものとみなす
//		Hasher&				hasher_
//			指定されたとき
//				ハッシュ関数
//			指定されないとき
//				Hasher() が指定されたものとみなす
//		ModBoolean			enableLink
//			ModTrue または指定されないとき
//				反復子の前後の移動を高速化するための配列を確保、維持する
//			ModFalse
//				反復子の前後の移動を高速化するための配列を確保、維持しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::
ModHashTable(ModSize tableSize, const Hasher& hasher_, ModBoolean enableLink)
	: buckets(0),
	  length(ModHashTableBase::verifySize(tableSize)),
	  hasher(hasher_),
	  size(0),
	  links(0)
{
	this->allocateNodePointer(enableLink);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::ModHashTable --
//		複数の値を初期値として与えるハッシュ表を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		与えられたハッシュ表の大きさは、
//		ModHashTableBase::verifySize により適当な値に変更される
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator	first
//			初期値として最初に挿入する値を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::ConstIterator	first
//			初期値として最初に挿入する値を指す反復子
//		ModHashNode<ValueType>*					first
//			初期値として最初に挿入する値を持つノードの
//			格納されている先頭アドレス
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	last
//			初期値として最後に挿入するものの 1 つ後の値を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::ConstIterator&	last
//			初期値として最後に挿入するものの 1 つ後の値を指す反復子
//		ModHashNode<ValueType>*					last
//			初期値として最後に挿入する値を持つノードの
//			次のノードが格納されている先頭アドレス
//		ModSize				tableSize
//			指定されたとき
//				ハッシュ表の最初の大きさ
//			指定されないとき
//				ModHashTableSizeDefault が指定されたものとみなす
//		Hasher&				hasher_
//			指定されたとき
//				ハッシュ関数
//			指定されないとき
//				Hasher() が指定されたものとみなす
//		ModBoolean			enableLink
//			ModTrue または指定されないとき
//				反復子の前後の移動を高速化するための配列を確保、維持する
//			ModFalse
//				反復子の前後の移動を高速化するための配列を確保、維持しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::
ModHashTable(Iterator first, const Iterator& last,
			 ModSize tableSize, const Hasher& hasher_, ModBoolean enableLink)
	: buckets(0),
	  length(ModHashTableBase::verifySize(tableSize)),
	  hasher(hasher_),
	  size(0),
	  links(0)
{
	this->allocateNodePointer(enableLink);

	// すでに存在するか検査せずに挿入する

	this->insert(first, last, ModTrue);
}

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::
ModHashTable(ConstIterator first, const ConstIterator& last,
			 ModSize tableSize, const Hasher& hasher_, ModBoolean enableLink)
	: buckets(0),
	  length(ModHashTableBase::verifySize(tableSize)),
	  hasher(hasher_),
	  size(0),
	  links(0)
{
	this->allocateNodePointer(enableLink);

	// すでに存在するか検査せずに挿入する

	this->insert(first, last, ModTrue);
}

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::
ModHashTable(const ValueType* first, const ValueType* last,
			 ModSize tableSize, const Hasher& hasher_, ModBoolean enableLink)
	: buckets(0),
	  length(ModHashTableBase::verifySize(tableSize)),
	  hasher(hasher_),
	  size(0),
	  links(0)
{
	this->allocateNodePointer(enableLink);

	// すでに存在するか検査せずに挿入する

	this->insert(first, last, ModTrue);
}

// TEMPLATE FUNCTION
//	ModHashTable<KeyType, ValueType, Hasher>::ModHashTable --
//		他のハッシュ表を複製するハッシュ表を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>&	src
//			複製するハッシュ表
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::
ModHashTable(const ModHashTable<KeyType, ValueType, Hasher>& src)
	: buckets(0),
	  length(src.length),
	  hasher(src.hasher),
	  size(0),
	  links(0)
{
	this->copy(src);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::~ModHashTable --
//		ハッシュ表を表すクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
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

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>::~ModHashTable()
{
	this->clear();
	this->freeNodePointer();
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>&	src
//			自分自身に代入するハッシュ表
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModHashTable<KeyType, ValueType, Hasher>&
ModHashTable<KeyType, ValueType, Hasher>::
operator =(const ModHashTable<KeyType, ValueType, Hasher>& src)
{
	if (this != &src)
		this->copy(src);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::isEmpty -- ハッシュ表が空か調べる
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ハッシュ表は空である
//		ModFalse
//			ハッシュ表は空でない
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
ModBoolean
ModHashTable<KeyType, ValueType, Hasher>::isEmpty() const
{
	return (this->getSize()) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::insert --
//		ハッシュ表へ 1 つの値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			挿入する値
//		ModBoolean			notUnique
//			ModTrue
//				挿入する値がすでにハッシュ表に存在するか調べずに挿入する
//			ModFalse または指定されないとき
//				挿入する値がすでにハッシュ表に存在するか調べる
//				存在すれば、挿入しない
//
//	RETURN
//		ハッシュ表に値を挿入したとき
//			挿入した値を持つノードを指す反復子と ModTrue の組
//		ハッシュ表に値を挿入しなかったとき
//			挿入しようとした値を持つノードを指す反復子と ModFalse の組
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
// inline
ModPair<ModTypename ModHashTable<KeyType, ValueType, Hasher>::Iterator, ModBoolean>
ModHashTable<KeyType, ValueType, Hasher>::
insert(const ValueType& value, ModBoolean notUnique)
{
	ModSize	newLen = (this->getSize() + 1) / 2;
	if (this->length < newLen)

		// 挿入後の要素数がハッシュ表の大きさの 2 倍を超えれば、
		// ハッシュ表を拡張する

		this->resize(newLen);

	ModSize	hashValue = this->hasher(value.first);
	ModSize	probe = hashValue % this->length;

	ModHashNode<ValueType>* node;

	if (notUnique == ModFalse)

		// 挿入する値がハッシュ表に存在するか調べる

		for (node = this->buckets[probe]; node; node = node->_next)
			if (this->keyEqual(hashValue, value.first, node))

				// 存在したので、それを指す反復子と
				// 挿入しなかったことを表す ModFalse の組を返す

				return ModPair<Iterator, ModBoolean>
								(Iterator(node, this), ModFalse);

	// バケットへ初めて挿入するノードならば、リンク情報を更新する

	this->validateLinkForInsert(probe);

	// 挿入する値を格納するノードを生成する

	this->buckets[probe] = node =
		new ModHashNode<ValueType>(const_cast<ValueType&>(value),
								   this->buckets[probe])
	; ModAssert(node);

	// ハッシュ表に格納されている値の数を 1 増やす

	this->size++;

	// 挿入した値を持つノードを指す反復子と
	// 挿入したことを表す ModTrue の組を返す

	return ModPair<Iterator, ModBoolean>(Iterator(node, this), ModTrue);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::insert --
//		ハッシュ表へ複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator	first
//			最初に挿入する値を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::ConstIterator	first
//			最初に挿入する値を指す反復子
//		ModHashNode<ValueType>*					first
//			最初に挿入する値を持つノードの格納されている先頭アドレス
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	last
//			最後に挿入するものの 1 つ後の値を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::ConstIterator&	last
//			最後に挿入するものの 1 つ後の値を指す反復子
//		ModHashNode<ValueType>*					last
//			最後に挿入する値を持つノードの
//			次のノードが格納されている先頭アドレス
//		ModBoolean			notUnique
//			ModTrue
//				挿入する値がすでにハッシュ表に存在するか調べる
//				存在すれば、挿入しない
//			ModFalse または指定されないとき
//				挿入する値がすでにハッシュ表に存在するか調べずに挿入する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::
insert(Iterator first, const Iterator& last, ModBoolean notUnique)
{
	for (; first != last; ++first)
		this->insert(*first, notUnique);
}

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::
insert(ConstIterator first, const ConstIterator& last, ModBoolean notUnique)
{
	for (; first != last; ++first)
		this->insert(*first, notUnique);
}

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::
insert(const ValueType* first, const ValueType* last, ModBoolean notUnique)
{
	// 必要であれば、ハッシュ表の大きさを拡張する
	//
	//【注意】	要素ごとにちまちま拡張されることを防ぐために
	//			ここで必要であれば一度に拡張する

	ModSize	newLen= (this->getSize() + (last - first)) / 2;
	if (this->length < newLen)

		// 挿入後の要素数がハッシュ表の大きさの 2 倍を超えれば、
		// ハッシュ表を拡張する
		//
		//【注意】	要素ごとにちまちま拡張されることを防ぐために
		//			ここで必要であれば一度に拡張する

		this->resize(newLen);

	for (; first != last; ++first)
		this->insert(*first, notUnique);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::erase -- 反復子の指す値を削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator		position
//			削除する値を持つノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			end または削除されているノードを指す反復子が与えられた

template <class KeyType, class ValueType, class Hasher>
void
ModHashTable<KeyType, ValueType, Hasher>::erase(Iterator position)
{
	ModHashNode<ValueType>*	node = position.getNode();

	if (node == 0 || node == node->_next)

		// 削除されているノードを指す反復子が与えられた

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	ModSize	probe = this->hasher(node->_data.first) % this->length;

	if (this->buckets[probe] != node) {

		// 削除するノードの前後のノードを接続する

		ModHashNode<ValueType>* prev = this->buckets[probe];
		for (; prev->_next != node; prev = prev->_next) ;
		prev->_next = node->_next;
	} else {

		// バケットに格納されている先頭のノードを削除すると、
		// そのバケットにはもうノードが存在しなくなるとき、
		// リンク情報からこのバケットを削除する

		this->buckets[probe] = node->_next;
		this->validateLinkForErase(probe);
	}

	// 削除するノードの次を自分自身にして、
	// 不正なノードにしてから、削除する

	node->_next = node;
	delete node;

	// ハッシュ表に格納されている値の数を 1 減らす

	this->size--;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::erase --
//		キー値にマッチする値を削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			削除する値を探すためのキー値
//
//	RETURN
//		削除した値の個数
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
ModSize
ModHashTable<KeyType, ValueType, Hasher>::erase(const KeyType& key)
{
	ModSize		hashValue = this->hasher(key);
	ModSize		probe = hashValue % this->length;

	ModHashNode<ValueType>*	node = this->buckets[probe];
	ModHashNode<ValueType>*	prev = 0;

	for (; node; prev = node, node = node->_next)
		if (this->keyEqual(hashValue, key, node))
			goto found;

	return 0;

found:
	ModSize	n = 0;
	ModHashNode<ValueType>*	next;

	do {
		next = node->_next;

		// 削除するノードの次を自分自身にして、
		// 不正なノードにしてから、削除する

		node->_next = node;
		delete node;

		// 削除したノードの数を 1 増やす

		n++;

	} while ((node = next) && this->keyEqual(hashValue, key, node));

	if (prev)

		// 削除された複数のノードの前後のノードを接続する

		prev->_next = node;
	else {

		// バケットに格納されているノードを削除した結果、
		// そのバケットにはもうノードが存在しないので、
		// リンク情報からこのバケットを削除する

		this->buckets[probe] = node;
		this->validateLinkForErase(probe);
	}

	// ハッシュ表に格納されている値の数を削除したぶん減らす

	this->size -= n;

	return n;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::erase --
//		反復子で指定された範囲の値を削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator	first
//			最初に削除する値を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	last
//			最後に削除するものの 1 つ後の値を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::
erase(Iterator first, const Iterator& last)
{
	while (first != last)
		this->erase(first++);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::splice --
//		あるハッシュ表のシーケンスをあるハッシュ表へ移動する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>&	src
//			移動するシーケンスがあるハッシュ表
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	first
//			移動するシーケンスの先頭を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	last
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

template <class KeyType, class ValueType, class Hasher>
// inline
void
ModHashTable<KeyType, ValueType, Hasher>::
splice(ModHashTable<KeyType, ValueType, Hasher>& src, const Iterator& first)
{
	if (this != &src && first != src.end()) {
		ModHashNode<ValueType>*		node = first.getNode();
		; ModAssert(node);

		ModSize	probe = src.hasher(node->_data.first) % src.length;
		; ModAssert(src.buckets);
		if (src.buckets[probe] == node) {
			src.buckets[probe] = node->_next;
			src.validateLinkForErase(probe);
		} else {
			ModHashNode<ValueType>*	prev = src.buckets[probe];
			for (; prev->_next != node; prev = prev->_next) ;
			prev->_next = node->_next;
		}
		src.size--;

		ModSize	newLen = (this->getSize() + 1) / 2;
		if (this->length < newLen)

			// 挿入後の要素数がハッシュ表の大きさの 2 倍を超えれば、
			// ハッシュ表を拡張する

			this->resize(newLen);

		probe = this->hasher(node->_data.first) % this->length;
		; ModAssert(this->buckets);

		this->validateLinkForInsert(probe);

		node->_next = this->buckets[probe];
		this->buckets[probe] = node;
		this->size++;
	}
}

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::
splice(ModHashTable<KeyType, ValueType, Hasher>& src,
	   const Iterator& first, const Iterator& last)
{
	while (first != last)
		this->splice(src, first++);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::move --
//		あるハッシュ表の反復子の指す要素を他のハッシュ表へ移動する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable<KeyType, ValueType, Hasher>&	fromTable
//			移動する要素を持つハッシュ表
//		ModHashTable<KeyType, ValueType, Hasher>::Iterator&	from
//			移動する要素を指す反復子
//		ModHashTable<KeyType, ValueType, Hasher>&	toTable
//			要素を移動するハッシュ表
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			移動する要素を指す反復子が end を指している

// static
template <class KeyType, class ValueType, class Hasher>
void
ModHashTable<KeyType, ValueType, Hasher>::move(
						ModHashTable<KeyType, ValueType, Hasher>& fromTable,
						Iterator& from,
						ModHashTable<KeyType, ValueType, Hasher>& toTable)
{
	if (from == fromTable.end())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	if (&fromTable != &toTable) {
		ModHashNode<ValueType>*		node = from.getNode();
		; ModAssert(node);

		ModSize	probe =	fromTable.hasher(node->_data.first) % fromTable.length;
		; ModAssert(fromTable.buckets);
		if (fromTable.buckets[probe] == node) {
			fromTable.buckets[probe] = node->_next;
			fromTable.validateLinkForErase(probe);
		} else {
			ModHashNode<ValueType>*	prev = fromTable.buckets[probe];
			for (; prev->_next != node; prev = prev->_next) ;
			prev->_next = node->_next;
		}
		fromTable.size--;

		ModSize	newLen = (toTable.getSize() + 1) / 2;
		if (toTable.length < newLen)
			toTable.resize(newLen);

		probe = toTable.hasher(node->_data.first) % toTable.length;
		; ModAssert(toTable.buckets);

		toTable.validateLinkForInsert(probe);

		node->_next = toTable.buckets[probe];
		toTable.buckets[probe] = node;
		toTable.size++;
	}
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::find --
//		キー値にマッチする値を探す
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			値を探すためのキー値
//
//	RETURN
//		this->end() 以外
//			キーにマッチする値を指す反復子
//		this->end()
//			キーにマッチする値は見つからなかった
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
// inline
ModTypename ModHashTable<KeyType, ValueType, Hasher>::Iterator
ModHashTable<KeyType, ValueType, Hasher>::find(const KeyType& key)
{
	ModSize		hashValue = this->hasher(key);
	ModSize		probe = hashValue % this->length;

	ModHashNode<ValueType>* node = this->buckets[probe];
	ModHashNode<ValueType>* prev = 0;

	for (; node; prev = node, node = node->_next)
		if (this->keyEqual(hashValue, key, node)) {
			if (prev) {

				// 見つかったノードはそのバケットの先頭へ移動する

				prev->_next = node->_next;
				node->_next = this->buckets[probe];
				this->buckets[probe] = node;
			}
			return Iterator(node, this);
		}

	return this->end();
}

template <class KeyType, class ValueType, class Hasher>
// inline
ModTypename ModHashTable<KeyType, ValueType, Hasher>::ConstIterator
ModHashTable<KeyType, ValueType, Hasher>::find(const KeyType& key) const
{
	ModSize		hashValue = this->hasher(key);
	ModSize		probe = hashValue % this->length;

	ModHashNode<ValueType>* node = this->buckets[probe];

	for (; node; node = node->_next)
		if (this->keyEqual(hashValue, key, node))

			// 見つかった
			//
			//【注意】	const なので、
			//			見つかったノードをバケットの先頭へ移動しない

			return ConstIterator(node, this);

	return this->end();
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::getFront -- 先頭の値を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ表の先頭の値
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ValueType&
ModHashTable<KeyType, ValueType, Hasher>::getFront()
{
	return *this->begin();
}

template <class KeyType, class ValueType, class Hasher>
inline
const ValueType&
ModHashTable<KeyType, ValueType, Hasher>::getFront() const
{
	return *this->begin();
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::popFront -- 先頭の値を削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
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

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::popFront()
{
	this->erase(this->begin());
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::begin --
//		ハッシュ表の先頭の値を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
// 		なし
//
//	RETURN
//		ハッシュ表の先頭の値を指す反復子
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModTypename ModHashTable<KeyType, ValueType, Hasher>::Iterator
ModHashTable<KeyType, ValueType, Hasher>::begin()
{
	return Iterator(this->first(), this);
}

template <class KeyType, class ValueType, class Hasher>
inline
ModTypename ModHashTable<KeyType, ValueType, Hasher>::ConstIterator
ModHashTable<KeyType, ValueType, Hasher>::begin() const
{
	return ConstIterator(this->first(), this);
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::end --
//		ハッシュ表の最後の値(end)を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
// 		なし
//
//	RETURN
//		ハッシュ表の最後の値(end)を指す反復子
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
const ModTypename ModHashTable<KeyType, ValueType, Hasher>::Iterator&
ModHashTable<KeyType, ValueType, Hasher>::end()
{
	return this->endIterator;
}

template <class KeyType, class ValueType, class Hasher>
inline
const ModTypename ModHashTable<KeyType, ValueType, Hasher>::ConstIterator&
ModHashTable<KeyType, ValueType, Hasher>::end() const
{
	return this->endConstIterator;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::getSize --
//		ハッシュ表中の値の数を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ表中の値の数
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
ModSize
ModHashTable<KeyType, ValueType, Hasher>::getSize() const
{
	return this->size;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::getHasher --
//		ハッシュ表のハッシュ関数を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ表のハッシュ関数
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
Hasher
ModHashTable<KeyType, ValueType, Hasher>::getHasher() const
{
	return this->hasher;
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::clear -- ハッシュ表を空にする
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
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

template <class KeyType, class ValueType, class Hasher>
// inline
void
ModHashTable<KeyType, ValueType, Hasher>::clear()
{
	if (this->getSize()) {

		// すべてのノードを削除する

		for (ModSize i = 0; i < this->length; i++) {
			ModHashNode<ValueType>*	node = this->buckets[i];
			ModHashNode<ValueType>*	next;
			for (; node; node = next) {
				next = node->_next;

				// 削除するノードの次を自分自身にして、
				// 不正なノードにしてから、削除する

				node->_next = node;
				delete node;
			}
		}

		// バケットおよび必要であればリンク情報を初期化する

		ModSize	allocateSize =
			sizeof(ModHashNode<ValueType>*) * (this->length + 1);
		if (this->links)
			allocateSize += sizeof(ModSize) * (this->length + 1) * 2;
		ModOsDriver::Memory::reset(this->buckets, allocateSize);

		// バケットに nilNode を記録しなおす

		this->buckets[this->length] = &this->nilNode;

		// ハッシュ表中の値の数を 0 にする

		this->size = 0;
	}
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::resize --
//		ハッシュ表の大きさを拡張する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				newLen
//			新しいハッシュ表の大きさ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
// inline
void
ModHashTable<KeyType, ValueType, Hasher>::resize(ModSize newLen)
{
	ModSize	oldLen = this->length;
	if (newLen > oldLen) {

		// 与えられた大きさを適当な素数にする

		newLen = ModHashTableBase::verifySize(newLen);

		if (newLen > oldLen) {
			ModHashNode<ValueType>**	oldBuckets = this->buckets;
			; ModAssert(oldBuckets != 0);

			// 新しいサイズのバケットを生成する

			this->length = newLen;
			this->allocateNodePointer((this->links) ? ModTrue : ModFalse);

			// 既存のノードをすべて新しく確保したバケットへ登録する

			for (ModSize i = 0; i < oldLen; i++) {
				ModHashNode<ValueType>*	node = oldBuckets[i];
				ModHashNode<ValueType>* next;

				for (; node; node = next) {

					ModSize probe =
						this->hasher(node->_data.first) % this->length;

					this->validateLinkForInsert(probe);

					next = node->_next;
					node->_next = this->buckets[probe];
					this->buckets[probe] = node;
				}
			}

			// 古いバケットを破棄する

			ModSize	oldSize = sizeof(ModHashNode<ValueType>*) * (oldLen + 1);
			if (this->links)
				oldSize += sizeof(ModSize) * (oldLen + 1) * 2;

			ModStandardManager::free(oldBuckets, oldSize);
		}
	}
}

//	TEMPLATE FUNCTION public
//	ModHashTable<KeyType, ValueType, Hasher>::printHist --
//		ハッシュ表のバケットごとのノード数分布を出力する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
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

#include "ModMessage.h"

template <class KeyType, class ValueType, class Hasher>
void
ModHashTable<KeyType, ValueType, Hasher>::printHist() const
{
	ModSize		prev = ModSizeMax;
	ModBoolean	continued = ModFalse;

	ModMessage << "bucket#(0-" << this->length << ")" << ModEndl;

	for (ModSize i = 0; i < this->length; i++) {
		ModSize	n = 0;
		for (ModHashNode<ValueType>* node = this->buckets[i];
			 node; node = node->_next, n++) ;

		if (n != prev) {
			ModMessage << ModSetW(5) << ModSetFill('0') << i
					   << ' ' << n << ModEndl;
			prev = n;
			continued = ModFalse;

		} else if (continued == ModFalse) {
			ModMessage << ":     :" << ModEndl;
			continued = ModTrue;
		}
	}
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::first --
//		ハッシュ表の先頭の値を持つノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた先頭の値を持つノード
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
ModHashNode<ValueType>*
ModHashTable<KeyType, ValueType, Hasher>::first() const
{
	if (this->getSize() > 0)
		if (this->links == 0) {

			// なめて探す

			for (ModSize i = 0; i < this->length; i++)
				if (this->buckets[i])
					return this->buckets[i];

		} else if (this->links[0])

			// リンク情報を使う
			// リンク情報にはバケット + 1 で格納されている

			return this->buckets[this->links[0] - 1];

	return 0;
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::keyEqual --
//		ノードの持つ値がキーにマッチするか
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				hashValue
//			ハッシュ値
//		KeyType&			key
//			キー値
//		ModHashNode*		node
//			調べるノードが格納された領域の先頭アドレス
//
//	RETURN
//		ModTrue
//			マッチする
//		ModFalse
//			マッチしない
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
inline
ModBoolean
ModHashTable<KeyType, ValueType, Hasher>::
keyEqual(ModSize hashValue, const KeyType& key,
		 const ModHashNode<ValueType>* node) const
{
	return (this->hasher(node->_data.first) == hashValue &&
			key == node->_data.first) ?
		ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::copy --
//		自分を与えられたハッシュ表と同じ物にする
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModHashTable&		src
//			複製するハッシュ表
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
void
ModHashTable<KeyType, ValueType, Hasher>::
copy(const ModHashTable<KeyType, ValueType, Hasher>& src)
{
	ModBoolean	enableLink = (src.links) ? ModTrue : ModFalse;

	// 自分自身を空にする

	this->clear();

	if (this->length != src.length) {
		this->freeNodePointer();
		this->length = src.length;
		this->allocateNodePointer(enableLink);
	} else if (((this->links) ? ModTrue : ModFalse) != enableLink) {
		this->freeNodePointer();
		this->allocateNodePointer(enableLink);
	} else if (this->buckets == 0)
		this->allocateNodePointer(enableLink);

	this->size = src.size;
	this->hasher = src.hasher;

	// まず、バケットを複製する
	//
	//【注意】	バケットの末尾の nilNode は
	//			すでに確保されているので複製することはない

	for (ModSize i = 0; i < this->length; i++) {
		ModHashNode<ValueType>*	srcNode = src.buckets[i];
		ModHashNode<ValueType>*	prev = 0;

		for (; srcNode; srcNode = srcNode->_next) {
			ModHashNode<ValueType>*	dstNode =
				new ModHashNode<ValueType>(srcNode->_data);
			; ModAssert(dstNode);

			if (prev)
				prev->_next = dstNode;
			else
				this->buckets[i] = dstNode;

			prev = dstNode;
		}
	}

	if (enableLink)

		// 反復子の前後移動を高速化するための配列を複製する

		ModOsDriver::Memory::copy((char*) this->links, (char*) src.links,
								  sizeof(ModSize) * (this->length + 1) * 2);
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::allocateNodePointer --
//		バケット管理用の領域を確保する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			enableLink
//			ModTrue または指定されないとき
//				反復子の前後移動を高速化するための配列を確保する
//			ModFalse
//				反復子の前後移動を高速化するための配列を確保しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Hasher>
// inline
void
ModHashTable<KeyType, ValueType, Hasher>::
allocateNodePointer(ModBoolean enableLink)
{
	try {
		// nilNode を末尾に記録するため、
		// 指定されたサイズ + 1 でバケットを確保する

		ModSize	allocateSize =
			sizeof(ModHashNode<ValueType>*) * (this->length + 1);

		if (enableLink) {

			// バケットと共に、反復子の前後移動を高速化するための
			// リンク情報を格納する配列を指定されたサイズ + 1 で確保する

			ModSize	bucketSize = allocateSize;
			ModSize linkSize = sizeof(ModSize) * (this->length + 1);
			allocateSize += linkSize * 2;

			this->buckets =	(ModHashNode<ValueType>**)
				ModStandardManager::allocate(allocateSize);
			; ModAssert(this->buckets);

			this->links = (ModSize*) ((char*) this->buckets + bucketSize);
		} else {

			// バケットを確保する

			this->buckets = (ModHashNode<ValueType>**)
				ModStandardManager::allocate(allocateSize);
			; ModAssert(this->buckets);
		}

		// バケットを初期化する

		ModOsDriver::Memory::reset(this->buckets, allocateSize);

		// バケットに nilNode を記録する

		this->buckets[this->length] = &this->nilNode;

		// 確保したバケットおよびリンク情報を使って
		// end を指す反復子を生成しなおす

		this->endIterator =	Iterator(0, this);
		this->endConstIterator = ConstIterator(0, this);

	} catch (ModException& exception) {
		ModHashTable<KeyType, ValueType, Hasher>::freeNodePointer();
		ModRethrow(exception);
	} catch (...) {
		ModHashTable<KeyType, ValueType, Hasher>::freeNodePointer();
		ModUnexpectedThrow(ModModuleStandard);
	}
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::freeNodePointer --
//		バケット管理用の領域を破棄する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
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

template <class KeyType, class ValueType, class Hasher>
// inline
void
ModHashTable<KeyType, ValueType, Hasher>::freeNodePointer()
{
	if (this->buckets) {
		ModSize	allocateSize =
			sizeof(ModHashNode<ValueType>*) * (this->length + 1);
		if (this->links) {
			allocateSize += sizeof(ModSize) * (this->length + 1) * 2;
			this->links = 0;
		}

		ModStandardManager::free(this->buckets, allocateSize);
		this->buckets = 0;
	}
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::validateLinkForInsert --
//		値を挿入する前に必要ならば、リンク情報を更新する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		値の挿入時に、実際に値をバケットへ登録する前に呼び出す必要がある
//
//	ARGUMENTS
//		ModSize				probe
//			値を挿入するバケット
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
inline
void
ModHashTable<KeyType, ValueType, Hasher>::validateLinkForInsert(ModSize probe)
{
	if (this->buckets[probe] == 0 && this->links) {

		// バケットへ初めて挿入するノードなので、リンク情報を更新する

		ModSize&	prev = this->links[this->length + 1];

		this->links[this->length + 1 + probe + 1] = prev;
		prev = this->links[prev] = probe + 1;
	}
}

//	TEMPLATE FUNCTION private
//	ModHashTable<KeyType, ValueType, Hasher>::validateLinkForErase --
//		値を削除した後に必要ならば、リンク情報を更新する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			ハッシュキーの型
//		class ValueType
//			ハッシュ表に登録する値の型
//		class Hasher
//			ハッシュ関数を提供するクラス
//
//	NOTES
//		値の削除時に、実際に値をバケットから削除した後に呼び出す必要がある
//
//	ARGUMENTS
//		ModSize				probe
//			値を削除したバケット
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Hasher>
void
ModHashTable<KeyType, ValueType, Hasher>::validateLinkForErase(ModSize probe)
{
	if (this->buckets[probe] == 0 && this->links) {

		// バケットに格納されている先頭のノードを削除した結果、
		// そのバケットにはもうノードが存在しないので、
		// リンク情報からこのバケットを削除する

		ModSize&	next = this->links[probe + 1];
		ModSize&	prev = this->links[this->length + 1 + probe + 1];

		this->links[prev] = next;
		this->links[this->length + 1 + next] = prev;
		next = prev = 0;
	}
}

#endif	// __ModHashTable_H__

//
// Copyright (c) 1997, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
