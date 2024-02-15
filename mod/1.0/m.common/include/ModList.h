// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModList.h -- リストに関するクラス定義
// 
// Copyright (c) 1997, 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModList_H__
#define __ModList_H__

#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModException.h"
#include "ModAlgorithm.h"

template <class _ValueType>	class ModListIterator;
template <class _ValueType>	class ModListConstIterator;
template <class ValueType>	class ModList;

//	TEMPLATE CLASS
//	ModListNode -- リストの各要素を表すクラス
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES

template <class ValueType>
class ModListNode
	: public	ModDefaultObject
{
	friend class ModListIterator<ValueType>;
	friend class ModListConstIterator<ValueType>;
	friend class ModList<ValueType>;
public:
	ModListNode();
	ModListNode(const ValueType& value,
				ModListNode<ValueType>* left, ModListNode<ValueType>* right);
												// コンストラクター

	ModListNode<ValueType>&
		operator =(const ModListNode<ValueType>& src);
												// = 演算子
private:
	ModListNode<ValueType>*		_left;			// 前の要素
	ModListNode<ValueType>*		_right;			// 次の要素
	ValueType					_value;			// 値
};

//	TEMPLATE FUNCTION public
//	ModListNode<ValueType>::ModListNode --
//		リストの要素を表すクラスのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		前後の要素が自分自身の要素が生成される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModListNode<ValueType>::ModListNode()
{
	//【注意】	メンバーの初期化リストでは this を使えない

	_left = _right = this;
}

//	TEMPLATE FUNCTION public
//	ModListNode<ValueType>::ModListNode --
//		リストの要素を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			要素の値
//		ModListNode<ValueType>*	left
//			前の要素を格納する領域の先頭アドレス
//		ModListNode<ValueType>*	right
//			次の要素を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
ModListNode<ValueType>::ModListNode(const ValueType& value,
									ModListNode<ValueType>* left,
									ModListNode<ValueType>* right)
	: _left(left),
	  _right(right),
	  _value(value)
{ }

//	TEMPLATE FUNCTION public
//	ModListNode<ValueType>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListNode<ValueType>&	src
//			自分自身に代入する要素
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

template <class ValueType>
inline
ModListNode<ValueType>&
ModListNode<ValueType>::operator =(const ModListNode<ValueType>& src)
{
	if (this != src)
		_left = src._left, _right = src._right, _value = src._value;
}

//	TEMPLATE CLASS
//	ModListIterator -- リストの反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES

template <class _ValueType>
class ModListIterator
	: public	ModDefaultObject
{
	friend class ModList<_ValueType>;
public:
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType	ValueType;
#endif
	
	ModListIterator(ModListNode<_ValueType>*	node = 0);
												// コンストラクター

	ModListIterator<_ValueType>&	operator ++();	// ++ 前置演算子
	ModListIterator<_ValueType>		operator ++(int dummy);
												// ++ 後置演算子
	ModListIterator<_ValueType>&	operator +=(ModOffset n);
												// += 演算子
	ModListIterator<_ValueType>		operator +(ModOffset n) const;
												// + 演算子

	ModListIterator<_ValueType>&	operator --();	// -- 前置演算子
	ModListIterator<_ValueType>		operator --(int dummy);
												// -- 後置演算子
	ModListIterator<_ValueType>&	operator -=(ModOffset n);
												// -= 演算子
	ModListIterator<_ValueType>		operator -(ModOffset n) const;
												// - 演算子

	_ValueType&						operator *() const;
												// * 単項演算子

	ModBoolean
		operator ==(const ModListIterator<_ValueType>& r) const;
												// == 演算子
	ModBoolean
		operator !=(const ModListIterator<_ValueType>& r) const;
												// != 演算子
private:
	ModListNode<_ValueType>*		_node;			// 指している要素
};

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::ModListIterator --
//		リストの反復子を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListNode<_ValueType>*	node
//			指定されたとき
//				反復子の指すノードの格納されている領域の先頭アドレス
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModListIterator<_ValueType>::
ModListIterator(ModListNode<_ValueType>* node)
	: _node(node)
{ }

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator ++ -- ++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を次の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListIterator<_ValueType>&
ModListIterator<_ValueType>::operator ++()
{
	if (_node)
		_node = _node->_right;
	else
		//【注意】	指している要素が 0 ならば、
		//			削除された要素を指す反復子であるとみなす

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator ++ -- ++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を次の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の要素を指す前の自分自身と等しい反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListIterator<_ValueType>
ModListIterator<_ValueType>::operator ++(int dummy)
{
	ModListIterator<_ValueType> saved(*this);

	if (_node)
		_node = _node->_right;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator += -- += 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を指定された数ぶん次の要素を指すようにする
//
//	ARGUMENTS
//		ModOffset			n
//			いくつ次へ進めるか
//
//	RETURN
//		指定された数ぶん次の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListIterator<_ValueType>::operator ++ より)

template <class _ValueType>
inline
ModListIterator<_ValueType>&
ModListIterator<_ValueType>::operator +=(ModOffset n)
{
	if (n < 0)
		return *this -= -n;

	while (n--)
		++(*this);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator + -- + 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素から指定された数ぶん次の要素を指す反復子を得る
//
//	ARGUMENTS
//		ModOffset			n
//			いくつぶん次を指す反復子を得るか
//
//	RETURN
//		指定された数ぶん次の要素を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListIterator<_ValueType>::operator += より)

template <class _ValueType>
inline
ModListIterator<_ValueType>
ModListIterator<_ValueType>::operator +(ModOffset n) const
{
	return ModListIterator<_ValueType>(*this) += n;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator -- -- -- 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を前の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListIterator<_ValueType>&
ModListIterator<_ValueType>::operator --()
{
	if (_node)
		_node = _node->_left;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator -- -- -- 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を前の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前の要素を指す前の自分自身と等しい反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListIterator<_ValueType>
ModListIterator<_ValueType>::operator --(int dummy)
{
	ModListIterator<_ValueType> saved(*this);

	if (_node)
		_node = _node->_left;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator -= -- -= 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を指定された数ぶん前の要素を指すようにする
//
//	ARGUMENTS
//		ModOffset			n
//			いくつ前へ進めるか
//
//	RETURN
//		指定された数ぶん前の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListIterator<_ValueType>::operator -- より)

template <class _ValueType>
inline
ModListIterator<_ValueType>&
ModListIterator<_ValueType>::operator -=(ModOffset n)
{
	if (n < 0)
		return *this += -n;

	while (n--)
		--(*this);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator - -- - 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素から指定された数ぶん前の要素を指す反復子を得る
//
//	ARGUMENTS
//		ModOffset			n
//			いくつぶん前を指す反復子を得るか
//
//	RETURN
//		指定された数ぶん前の要素を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListIterator<_ValueType>::operator += より)

template <class _ValueType>
inline
ModListIterator<_ValueType>
ModListIterator<_ValueType>::operator -(ModOffset n) const
{
	return ModListIterator<_ValueType>(*this) -= n;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator * -- * 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素の値を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身の指す要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
_ValueType&
ModListIterator<_ValueType>::operator *() const
{
	if (_node == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return _node->_value;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<_ValueType>&	r
//			比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子の指す要素は自分自身の指すものと等しい
//		ModFalse
//			自分自身の指すものと等しくない
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModListIterator<_ValueType>::
operator ==(const ModListIterator<_ValueType>& r) const
{
	return (_node == r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModListIterator<_ValueType>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<_ValueType>&	r
//			比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子の指す要素は自分自身の指すものと等しくない
//		ModFalse
//			自分自身の指すものと等しい
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModListIterator<_ValueType>::
operator !=(const ModListIterator<_ValueType>& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
//	TEMPLATE FUNCTION
//	ModValueType<ValueType> --
//		リストの反復子の指す要素の値の型を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<ValueType>&	dummy
//			指す要素の値の型を得たいリストの反復子
//
//	RETURN
//		要素の値の型へのポインターにキャストされた 0
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType*
ModValueType(const ModListIterator<ValueType>& dummy)
{
	return static_cast<ValueType*>(0);
}
#endif

//	TEMPLATE CLASS
//	ModListConstIterator -- リストの読み出し専用反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES

template <class _ValueType>
class ModListConstIterator
	: public	ModDefaultObject
{
	friend class ModList<_ValueType>;
public:
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType	ValueType;
#endif
	
	ModListConstIterator(ModListNode<_ValueType>* node = 0);
												// コンストラクター

	ModListConstIterator<_ValueType>&	operator ++();
												// ++ 前置演算子
	ModListConstIterator<_ValueType>	operator ++(int dummy);
												// ++ 後置演算子
	ModListConstIterator<_ValueType>&	operator +=(ModOffset n);
												// += 演算子
	ModListConstIterator<_ValueType>	operator +(ModOffset n) const;
												// + 演算子

	ModListConstIterator<_ValueType>&	operator --();
												// -- 前置演算子
	ModListConstIterator<_ValueType>	operator --(int dummy);
												// -- 後置演算子
	ModListConstIterator<_ValueType>&	operator -=(ModOffset n);
												// -= 演算子
	ModListConstIterator<_ValueType>	operator -(ModOffset n) const;
												// - 演算子

	const _ValueType&					operator *() const;
												// * 単項演算子

	ModBoolean
		operator ==(const ModListConstIterator<_ValueType>& r) const;
												// == 演算子
	ModBoolean
		operator !=(const ModListConstIterator<_ValueType>& r) const;
												// != 演算子
private:
	ModListNode<_ValueType>*			_node;	// 指している要素
};

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::ModListConstIterator --
//		リストの読み出し専用反復子を表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListNode<_ValueType>*	node
//			指定されたとき
//				反復子の指すノードの格納されている領域の先頭アドレス
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModListConstIterator<_ValueType>::
ModListConstIterator(ModListNode<_ValueType>* node)
	: _node(node)
{ }

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator ++ -- ++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を次の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListConstIterator<_ValueType>&
ModListConstIterator<_ValueType>::operator ++()
{
	if (_node)
		_node = _node->_right;
	else
		//【注意】	以前は指しているノードが 0 ならば、
		//			削除された要素を指す反復子であるとみなす
		//
		//			ただし、読み取り専用反復子なので、
		//			削除された要素を指していることはないはず

		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator ++ -- ++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を次の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次の要素を指す前の自分自身と等しい反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListConstIterator<_ValueType>
ModListConstIterator<_ValueType>::operator ++(int dummy)
{
	ModListConstIterator<_ValueType> saved(*this);

	if (_node)
		_node = _node->_right;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator += -- += 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を指定された数ぶん次の要素を指すようにする
//
//	ARGUMENTS
//		ModOffset			n
//			いくつ次へ進めるか
//
//	RETURN
//		指定された数ぶん次の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListConstIterator<_ValueType>::operator ++ より)

template <class _ValueType>
inline
ModListConstIterator<_ValueType>&
ModListConstIterator<_ValueType>::operator +=(ModOffset n)
{
	if (n < 0)
		return *this -= -n;

	while (n--)
		++(*this);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator + -- + 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素から指定された数ぶん次の要素を指す反復子を得る
//
//	ARGUMENTS
//		ModOffset			n
//			いくつぶん次を指す反復子を得るか
//
//	RETURN
//		指定された数ぶん次の要素を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListConstIterator<_ValueType>::operator += より)

template <class _ValueType>
inline
ModListConstIterator<_ValueType>
ModListConstIterator<_ValueType>::operator +(ModOffset n) const
{
	return ModListConstIterator<_ValueType>(*this) += n;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator -- -- -- 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を前の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListConstIterator<_ValueType>&
ModListConstIterator<_ValueType>::operator --()
{
	if (_node)
		_node = _node->_left;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator -- -- -- 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を前の要素を指すようにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前の要素を指す前の自分自身と等しい反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
ModListConstIterator<_ValueType>
ModListConstIterator<_ValueType>::operator --(int dummy)
{
	ModListConstIterator<_ValueType> saved(*this);

	if (_node)
		_node = _node->_left;
	else
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return saved;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator -= -- -= 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身を指定された数ぶん前の要素を指すようにする
//
//	ARGUMENTS
//		ModOffset			n
//			いくつ前へ進めるか
//
//	RETURN
//		指定された数ぶん前の要素を指す自分自身
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListConstIterator<_ValueType>::operator -- より)

template <class _ValueType>
inline
ModListConstIterator<_ValueType>&
ModListConstIterator<_ValueType>::operator -=(ModOffset n)
{
	if (n < 0)
		return *this += -n;

	while (n--)
		--(*this);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator - -- - 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素から指定された数ぶん前の要素を指す反復子を得る
//
//	ARGUMENTS
//		ModOffset			n
//			いくつぶん前を指す反復子を得るか
//
//	RETURN
//		指定された数ぶん前の要素を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である
//			(ModListConstIterator<_ValueType>::operator += より)

template <class _ValueType>
inline
ModListConstIterator<_ValueType>
ModListConstIterator<_ValueType>::operator -(ModOffset n) const
{
	return ModListConstIterator<_ValueType>(*this) -= n;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator * -- * 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の指す要素の値を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身の指す要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			自分は削除された要素を指す反復子である

template <class _ValueType>
inline
const _ValueType&
ModListConstIterator<_ValueType>::operator *() const
{
	if (_node == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return _node->_value;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListConstIterator<_ValueType>&	r
//			比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子の指す要素は自分自身の指すものと等しい
//		ModFalse
//			自分自身の指すものと等しくない
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModListConstIterator<_ValueType>::
operator ==(const ModListConstIterator<_ValueType>& r) const
{
	return (_node == r._node) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModListConstIterator<_ValueType>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListConstIterator<_ValueType>&	r
//			比較する反復子
//
//	RETURN
//		ModTrue
//			与えられた反復子の指す要素は自分自身の指すものと等しくない
//		ModFalse
//			自分自身の指すものと等しい
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModListConstIterator<_ValueType>::
operator !=(const ModListConstIterator<_ValueType>& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

//	TEMPLATE CLASS
//	ModList -- リストを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		リストは、型 ValueType の値を持つ要素からなる可変長のシーケンスである
//		シーケンスは、型 ValueType の値を持つ要素の双方向リストにより実装される

template <class ValueType>
class ModList
	: public	ModDefaultObject
{
public:
	typedef ModListIterator<ValueType>		Iterator;
	typedef ModListConstIterator<ValueType>	ConstIterator;

	ModList();
	ModList(ModSize n, const ValueType& value = ValueType());
	ModList(const ModList<ValueType>& src);		// コンストラクター
	~ModList();									// デストラクター

	ModList<ValueType>&		operator =(const ModList<ValueType>& src);
												// = 演算子

	void					assign(ConstIterator first,
								   const ConstIterator& last);
	void					assign(ModSize n,
								   const ValueType& value = ValueType());
												// 値を割り当てる

	ModBoolean				isEmpty() const;	// 空か調べる

	Iterator				insert(Iterator position,
								   const ValueType& value = ValueType());
	void					insert(Iterator position,
								   ModSize n, const ValueType& value);
	void					insert(Iterator position,
								   ConstIterator first,
								   const ConstIterator& last);
	void					insert(Iterator position,
								   Iterator first, const Iterator& last);
												// ある位置へ値を挿入する

	void					pushFront(const ValueType& value);
												// 先頭に値を挿入する
	void					pushBack(const ValueType& value);
												// 末尾に値を挿入する
	
	Iterator				erase(Iterator position);
	Iterator				erase(Iterator first, const Iterator& last);
												// 削除する

	void					popFront();			// 先頭の値を削除する
	void					popBack();			// 末尾の値を削除する

	void					clear();			// 全要素を削除する

	void					swap(ModList<ValueType>& r);
												// 全要素を入れ替える

	void					splice(const Iterator& position,
								   ModList<ValueType>& src);
	void					splice(const Iterator& position,
								   ModList<ValueType>& src,
								   const Iterator& first);
	void					splice(const Iterator& position,
								   ModList<ValueType>& src,
								   const Iterator& first,
								   const Iterator& last);
												// あるリストの要素を移動する

	// 以下、近日中に廃止します
	void					move(Iterator from, Iterator to);
	static void				move(ModList<ValueType>& fromList, Iterator from,
								 ModList<ValueType>& toList, Iterator to);
												// あるノードを移動する

	void					moveFront(Iterator from);
												// 反復子の指す要素を
												// 先頭に移動する
	void					moveBack(Iterator from);
												// 反復子の指す要素を
												// 末尾へ移動する
	// 以上、近日中に廃止します

	void					reverse();			// 要素の並びを逆にする

	Iterator				find(const ValueType& key);
	ConstIterator			find(const ValueType& key) const;
												// 等しい値を持つ要素を
												// 先頭から探す

	ValueType&				getFront();
	const ValueType&		getFront() const;	// 先頭の要素の値を得る
	ValueType&				getBack();
	const ValueType&		getBack() const;	// 末尾の要素の値を得る

	Iterator				begin();
	ConstIterator			begin() const;		// 先頭の値を指す反復子を得る
	Iterator&				end();
	const ConstIterator&	end() const;		// end を指す反復子を得る

	ModSize					getSize() const;	// リスト中の値の数を得る

private:
	ModSize					_length;			// リスト中の値の数
	ModListNode<ValueType>	_endNode;			// end を表すノード
	Iterator				_endIterator;		// end を指す反復子
	ConstIterator			_endConstIterator;	// end を指す読み出し専用反復子
};

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::ModList -- リストのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
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

template <class ValueType>
inline
ModList<ValueType>::ModList()
	: _length(0),
	  _endIterator(&_endNode),
	  _endConstIterator(&_endNode)
{ }

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::ModList -- ある値を複数個持つリストのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				n
//			生成されるリストの要素数
//		ValueType&			value
//			指定されたとき
//				生成されるリストの要素の値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
ModList<ValueType>::ModList(ModSize n, const ValueType& value)
	: _length(0),
	  _endIterator(&_endNode),
	  _endConstIterator(&_endNode)
{
	// 指定された値を複数個挿入する

	this->insert(this->begin(), n, value);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::ModList -- リストのコピーコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>&		src
//			生成されるリストに割り当てる要素を持つリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
ModList<ValueType>::ModList(const ModList<ValueType>& src)
	: _length(0),
	  _endIterator(&_endNode),
	  _endConstIterator(&_endNode)
{
	// 指定されたリストの要素をすべて挿入する

	this->insert(this->begin(), src.begin(), src.end());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::~ModList -- リストのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
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

template <class ValueType>
inline
ModList<ValueType>::~ModList()
{
	// 全要素を削除する

	this->clear();
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>&		src
//			自分自身へ代入するリスト
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

template <class ValueType>
ModList<ValueType>&
ModList<ValueType>::operator =(const ModList<ValueType>& src)
{
	if (this != &src)
		this->assign(src.begin(), src.end());

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::assign -- 指定された範囲の複数の値を割り当てる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の持つ要素はすべて削除され、
//		与えられた複数の値が新しい要素になる
//	
//	ARGUMENTS
//		ModList<ValueType>::ConstIterator	first
//			最初に割り当てる値を持つ要素を指す反復子
//		ModList<ValueType>::ConstIterator&	last
//			最後に割り当てる値を持つ要素の 1 つ後を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された反復子が自分は削除された要素を指している

template <class ValueType>
void
ModList<ValueType>::assign(ConstIterator first, const ConstIterator& last)
{
	Iterator		iterator(this->begin());
	const Iterator&	end = this->end();

	for (; iterator != end && first != last; ++iterator, ++first)
		*iterator = *first;

	if (iterator == end)
		this->insert(end, first, last);
	else
		(void) this->erase(iterator, end);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::assign -- ある値を複数個を割り当てる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		自分自身の持つ要素はすべて削除され、
//		与えられた複数の値が新しい要素になる
//	
//	ARGUMENTS
//		ModSize			n
//			割り当てる値の数
//		ValueType&		value
//			指定されたとき
//				割り当てる値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
void
ModList<ValueType>::assign(ModSize n, const ValueType& value)
{
	Iterator		iterator(this->begin());
	const Iterator&	end = this->end();

	for (; iterator != end && n; ++iterator, n--)
		*iterator = value;

	if (iterator == end)
		this->insert(end, n, value);
	else
		(void) this->erase(iterator, end);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::isEmpty -- リストに要素がないか調べる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			リストに要素はない
//		ModFalse
//			リストに要素はある
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModBoolean
ModList<ValueType>::isEmpty() const
{
	return (this->getSize()) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::insert -- ある値を 1 つ挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator	position
//			値を挿入する位置を指す反復子
//		ValueType&			value
//			指定されたとき
//				挿入する値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//
//	RETURN
//		挿入した要素を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された反復子の指す挿入する位置の要素は削除されたものだった

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator
ModList<ValueType>::insert(Iterator position, const ValueType& value)
{
	//【注意】	指している要素が 0 ならば、
	//			削除された要素を指す反復子であるとみなす

	if (position._node == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	// 挿入する値を格納する要素を確保する

	ModListNode<ValueType>*	node =
		new ModListNode<ValueType>(value,
								   position._node->_left, position._node);

	// 指定された反復子の指す位置へ確保した要素をつなぐ

	position._node->_left->_right = node;
	position._node->_left = node;
	_length++;

	// 挿入された要素を指す反復子を返す

	return Iterator(node);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::insert -- ある値を複数個挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator	position
//			値を挿入する位置を指す反復子
//		ModSize				n
//			挿入する値の数
//		ValueType&			value
//			挿入する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された反復子の指す挿入する位置の要素は削除されたものだった

template <class ValueType>
inline
void
ModList<ValueType>::insert(Iterator position,
						   ModSize n, const ValueType& value)
{
	while (n--)
		(void) this->insert(position, value);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::insert -- 指定された範囲の複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator	position
//			値を挿入する位置を指す反復子
//		ModList<ValueType>::Iterator	first
//			最初に挿入する値を持つ要素を指す反復子
//		ModList<ValueType>::ConstIterator	first
//			最初に挿入する値を持つ要素を指す読み出し専用反復子
//		ModList<ValueType>::Iterator&	last
//			最後に挿入する値を持つ要素の 1 つ後を指す反復子
//		ModList<ValueType>::ConstIterator&	last
//			最後に挿入する値を持つ要素の 1 つ後を指す読み出し専用反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された反復子の指す挿入する位置の要素は削除されたものだった

template <class ValueType>
inline
void
ModList<ValueType>::insert(Iterator position,
						   ConstIterator first, const ConstIterator& last)
{
	for (; first != last; ++first)
		(void) this->insert(position, *first);
}

template <class ValueType>
inline
void
ModList<ValueType>::insert(Iterator position,
						   Iterator first, const Iterator& last)
{
	for (; first != last; ++first)
		(void) this->insert(position, *first);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::pushFront -- 与えられた値をリストの先頭へ挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			先頭へ挿入する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
void
ModList<ValueType>::pushFront(const ValueType& value)
{
	(void) this->insert(this->begin(), value);
}


//	TEMPLATE FUNCTION public
//	ModList<ValueType>::pushBack -- 与えられた値をリストの末尾へ挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			末尾へ挿入する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
void
ModList<ValueType>::pushBack(const ValueType& value)
{
	(void) this->insert(this->end(), value);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::erase -- 指定された範囲の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator	first
//			最初に削除する要素を指す反復子
//		ModList<ValueType>::Iterator&	last
//			最後に削除する要素の 1 つ後を指す反復子
//
//	RETURN
//		最後に削除した要素の 1 つ後を指す反復子
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator
ModList<ValueType>::erase(Iterator first, const Iterator& last)
{
	for (; first != last; first = this->erase(first)) ;
	return first;
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::erase -- 指定された反復子の指す要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator	position
//			削除する要素を指す反復子
//
//	RETURN
//		削除した要素の 1 つ後を指す反復子
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			削除された要素を指す反復子が指定された

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator
ModList<ValueType>::erase(Iterator position)
{
	if (this->getSize() == 0)
		return this->end();

	//【注意】	指している要素が 0 ならば、
	//			削除された要素を指す反復子であるとみなす

	if (position._node == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	ModListNode<ValueType>*	right = position._node->_right;
	position._node->_left->_right = right;
	right->_left = position._node->_left;
	_length--;

	// 反復子の指すノードを 0 にして、不正なノードを指していることを表す

	delete position._node, position._node = 0;

	// 削除した要素の 1 つ次の要素を指す反復子を返す

	return Iterator(right);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::popFront -- 先頭の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
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

template <class ValueType>
inline
void
ModList<ValueType>::popFront()
{
	(void) this->erase(this->begin());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::popBack -- 末尾の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
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

template <class ValueType>
inline
void
ModList<ValueType>::popBack()
{
	(void) this->erase(this->end() - 1);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::clear -- 全要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		erase(begin(), end()) と同じである
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
void
ModList<ValueType>::clear()
{
	(void) this->erase(this->begin(), this->end());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::swap -- 全要素を入れ替える
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//	
//	ARGUMENTS
//		ModList<ValueType>&		r
//			自分自身と全要素を交換するリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
void
ModList<ValueType>::swap(ModList<ValueType>& r)
{
	ModSwap(_length, r._length);
	ModSwap(_endNode, r._endNode);
}

// ModSwap の ModList<ValueType> の特別バージョンを定義する

template <class ValueType>
inline
void
ModSwap(ModList<ValueType>& l, ModList<ValueType>& r)
{
	l.swap(r);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::splice --
//		あるリスト中のすべての要素をあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator&	position
//			あるリストのすべての要素を移動する位置を指す反復子
//			反復子の指す要素の前にあるリストのすべての要素は移動される
//		ModList<ValueType>&		src
//			すべての要素を移動するリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
void
ModList<ValueType>::splice(const Iterator& position, ModList<ValueType>& src)
{
	this->splice(position, src, src.begin(), src.end());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::splice --
//		あるリストのシーケンスをあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator&	position
//			シーケンスを移動する位置を指す反復子
//			反復子の指す要素の前にシーケンスは移動される
//		ModList<ValueType>&		src
//			移動するシーケンスがあるリスト
//		ModList<ValueType>::Iterator&	first
//			移動するシーケンスの先頭を指す反復子
//		ModList<ValueType>::Iterator&	last
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

template <class ValueType>
inline
void
ModList<ValueType>::splice(const Iterator& position,
						   ModList<ValueType>& src, const Iterator& first)
{
	if (position != first && first != src.end() && position != first + 1) {

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

		first._node->_left->_right = first._node->_right;
		first._node->_right->_left = first._node->_left;

		first._node->_left = position._node->_left;
		first._node->_right = position._node;

		position._node->_left->_right = first._node;
		position._node->_left = first._node;

		// リストの長さを 1 増減する

		--src._length;
		++_length;
	}
}

template <class ValueType>
void
ModList<ValueType>::splice(const Iterator& position,
						   ModList<ValueType>& src,
						   const Iterator& first, const Iterator& last)
{
	if (position != first && first != src.end() &&
		position != last && first != last) {

		// 移動する要素の数を求める

		ModSize	n = 0;
		for (Iterator iterator = first; iterator != last; ++iterator, ++n);

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

			last._node->_left->_right = position._node;
			first._node->_left->_right = last._node;
			position._node->_left->_right = first._node;

			ModListNode<ValueType>* tmp = position._node->_left;
			position._node->_left = last._node->_left;
			last._node->_left = first._node->_left;
			first._node->_left = tmp;

			// 移動した要素数ぶん、リストの長さを増減する

			src._length -= n;
			_length += n;
		}
	}
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::move -- 反復子の指すノードをある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<ValueType>	from
//			移動するノードを指す反復子
//		ModListIterator<ValueType>	to
//			ノードを移動する位置を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			移動するノードを指す反復子が end を指している

template <class ValueType>
inline
void
ModList<ValueType>::move(Iterator from, Iterator to)
{
	ModList<ValueType>::move(*this, from, *this, to);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::move --
//		あるリストの反復子の指すノードをあるリストのある位置へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModList<ValueType>&		fromList
//			移動するノードを持つリスト
//		ModListIterator<ValueType>	from
//			移動するノードを指す反復子
//		ModList<ValueType>&		toList
//			ノードを移動するリスト
//		ModListIterator<ValueType>	to
//			ノードを移動する位置を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			移動するノードを指す反復子が end を指している

template <class ValueType>
void
ModList<ValueType>::move(ModList<ValueType>& fromList, Iterator from,
						 ModList<ValueType>& toList, Iterator to)
{
	if (from == fromList.end())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	if (&fromList != &toList || from != to) {
		from._node->_left->_right = from._node->_right;
		from._node->_right->_left = from._node->_left;

		fromList._length--;

		from._node->_left = to._node->_left;
		from._node->_right = to._node;

		to._node->_left->_right = from._node;
		to._node->_left = from._node;

		toList._length++;
	}
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::moveFront -- 反復子の指す要素を先頭へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<ValueType>	from
//			先頭へ移動するノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			移動するノードを指す反復子が end を指している
//			(ModList<ValueType>::move より)

template <class ValueType>
inline
void
ModList<ValueType>::moveFront(Iterator from)
{
	this->move(from, this->begin());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::moveBack -- 反復子の指す要素を末尾へ移動する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModListIterator<ValueType>	from
//			末尾へ移動するノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			移動するノードを指す反復子が end を指している
//			(ModList<ValueType>::move より)

template <class ValueType>
inline
void
ModList<ValueType>::moveBack(Iterator from)
{
	this->move(from, this->end());
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::reverse -- リストの要素の並びを逆にする
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
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

template <class ValueType>
inline
void
ModList<ValueType>::reverse()
{
	if (this->getSize() > 1) {
		Iterator	first(this->begin());
		while (++first != this->end())
			this->splice(this->begin(), *this, first, first + 1);
	}
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::find -- ある値と等しい値を持つ要素を先頭から探す
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		指定された値と等しい値を持つ要素を先頭から探し、
//		最初に見つかった要素を指す反復子を返す
//		見つからなかったとき、end を指す反復子を返す
//
//	ARGUMENTS
//		ValueType&				key
//			リスト中を探す値
//
//	RETURN
//		見つかったとき
//			等しい値を持つ要素を指す反復子
//		見つからないとき
//			end を指す反復子
//
//	EXCEPTIONS

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator
ModList<ValueType>::find(const ValueType& key)
{
	const Iterator&	end = this->end();

	if (this->isEmpty() == ModFalse) {
		Iterator	iterator(this->begin());
		for (; iterator != end; ++iterator)
			if (*iterator == key)
				return iterator;		// 見つかった
	}

	return end;
}

template <class ValueType>
inline
ModTypename ModList<ValueType>::ConstIterator
ModList<ValueType>::find(const ValueType& key) const
{
	const ConstIterator&	end = this->end();

	if (this->isEmpty() == ModFalse) {
		ConstIterator	iterator(this->begin());
		for (; iterator != end; ++iterator)
			if (*iterator == key)
				return iterator;		// 見つかった
	}

	return end;
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::getFront -- 先頭のノードの値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		リストが空のときは ValueType() を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたリストの先頭のノードの値
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType&
ModList<ValueType>::getFront()
{
	return *this->begin();
}

template <class ValueType>
inline
const ValueType&
ModList<ValueType>::getFront() const
{
	return *this->begin();
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::getBack -- 末尾のノードの値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		リストが空のときは ValueType() を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたリストの末尾のノードの値
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType&
ModList<ValueType>::getBack()
{
	return *(this->end() - 1);
}

template <class ValueType>
inline
const ValueType&
ModList<ValueType>::getBack() const
{
	return *(this->end() - 1);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::begin -- 先頭のノードを指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた先頭のノードを指す反復子
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator
ModList<ValueType>::begin()
{
	return Iterator(_endNode._right);
}

template <class ValueType>
inline
ModTypename ModList<ValueType>::ConstIterator
ModList<ValueType>::begin() const
{
	return ConstIterator(_endNode._right);
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::end -- end を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた end を指す反復子
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModTypename ModList<ValueType>::Iterator&
ModList<ValueType>::end()
{
	return _endIterator;
}

template <class ValueType>
inline
const ModTypename ModList<ValueType>::ConstIterator&
ModList<ValueType>::end() const
{
	return _endConstIterator;
}

//	TEMPLATE FUNCTION public
//	ModList<ValueType>::getSize -- リストの長さを得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		リストに格納されている要素数
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModSize
ModList<ValueType>::getSize() const
{
	return _length;
}

#ifdef OBSOLETE
//	TEMPLATE FUNCTION private
//	ModList<ValueType>::debugSize -- リストの長さが実際の長さと等しいか検査する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		デバッグ用関数
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorAssert
//			検査の結果、異常が見つかった

template <class ValueType>
inline
void
ModList<ValueType>::debugSize() const
{
	ModSize	n;

	ConstIterator			iterator(this->begin());
	const ConstIterator&	end = this->end();

	for (n = 0; iterator != end && n < this->getSize(); ++iterator, ++n) ;
	; ModAssert(iterator == end && this->getSize() == n);

	iterator = end - 1;
	for (n = 0; iterator != end && n < this->getSize(); --iterator, ++n) ;
	; ModAssert(iterator == end && this->getSize() == n);
}

//	TEMPLATE FUNCTION private
//	ModList<ValueType>::debugIterator --
//		反復子がリストの要素を本当に指しているか調べる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			リストに登録する値の型
//
//	NOTES
//		デバッグ用関数
//
//	ARGUMENTS
//		ModList<ValueType>::Iterator&	checked
//			検査する反復子
//		ModList<ValueType>::ConstIterator&	checked
//			検査する読み出し専用反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorAssert
//			検査の結果、異常が見つかった

template <class ValueType>
inline
void
ModList<ValueType>::debugIterator(const Iterator& checked) const
{
	this->debugIterator(ConstIterator(checked._node));
}

template <class ValueType>
inline
void
ModList<ValueType>::debugIterator(const ConstIterator& checked) const
{
	const ConstIterator&	end = this->end();
	if (end != checked) {
		ConstIterator	iterator(this->begin());
		for (; iterator != end && iterator != checked; ++iterator) ;
		; ModAssert(iterator != end);

		iterator = end - 1;
		for (; iterator != end && iterator != checked; --iterator) ;
		; ModAssert(iterator != end);
	}
}
#endif	// OBSOLETE

#endif	// __ModList_H__

//
// Copyright (c) 1997, 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
