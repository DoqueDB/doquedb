// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTree.h -- Red-Black 木に関するクラス定義
// 
// Copyright (c) 1997, 2002, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModTree_H__
#define __ModTree_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModPair.h"

//	CLASS
//	ModTreeNode -- Red-Black 木のノードを表す基底クラス
//
//	NOTES
//		ModPureTreeNode などの機能クラスを用意し、
//		そのサブクラスでメモリハンドルを明示したクラスを作る方法を試したが、
//		うまくいかなかったのでどのクラスも
//		直接 ModDefaultObject のサブクラスとして作成する

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModTreeNode
	: public	ModDefaultObject
{
public:
	enum ColorType
	{
		red,					// 仮ノードへの枝の色
		black					// 実ノードへの枝の色
	};

	// コンストラクター
	ModTreeNode();

	// 色を得る
	ColorType					getColor() const;
	// 色を設定する
	void						setColor(ColorType v);
	// 親ノードを得る
	ModTreeNode*&				getParent();
	const ModTreeNode*			getParent() const;
	// 親ノードを設定する
	void						setParent(ModTreeNode* v);
	// 右の子ノードを得る
	ModTreeNode*&				getRight();
	const ModTreeNode*			getRight() const;
	// 右の子ノードを設定する
	void						setRight(ModTreeNode* v);
	// 左の子ノードを得る
	ModTreeNode*&				getLeft();
	const ModTreeNode*			getLeft() const;
	// 左の子ノードを設定する
	void						setLeft(ModTreeNode* v);

private:
	ColorType					_color;			// このノードへの枝の色
	ModTreeNode*				_parent;		// 親ノード
	ModTreeNode*				_left;			// 子ノード(左)
	ModTreeNode*				_right;			// 子ノード(右)
};

//	FUNCTION public
//	ModTreeNode::ModTreeNode --
//		Red-Black 木のノードを表す基底クラスのコンストラクター
//
//	NOTES
//		木の末尾を表すノードである nilNode として初期化される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModTreeNode::ModTreeNode()
	: _color(ModTreeNode::black),
	  _parent(0),
	  _left(0),
	  _right(0)
{ }

//	FUNCTION public
//	ModTreeNode::getColor -- 色を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ノードの枝への色
//
//	EXCEPTIONS
//		なし

inline
ModTreeNode::ColorType
ModTreeNode::getColor() const
{
	return _color;
}

//	FUNCTION public
//	ModTreeNode::setColor -- 色を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode::ColorType	v
// 			設定する色を表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTreeNode::setColor(ColorType v)
{
	_color = v;
}

//	FUNCTION public
//	ModTreeNode::getParent -- 親ノードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		親ノードを表すクラスを格納する領域の先頭アドレスを返す
//
//	EXCEPTIONS
//		なし

inline
ModTreeNode*&
ModTreeNode::getParent()
{
	return _parent;
}

inline
const ModTreeNode*
ModTreeNode::getParent() const
{
	return _parent;
}

//	FUNCTION public
//	ModTreeNode::setParent -- 親ノードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*	v
//			親ノードとして設定したいノードを表す
//			クラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTreeNode::setParent(ModTreeNode* v)
{
	_parent = v;
}

//	FUNCTION public
//	ModTreeNode::getRight -- 右の子ノードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		右の子ノードを表すクラスを格納する領域の先頭アドレスを返す
//
//	EXCEPTIONS
//		なし

inline
ModTreeNode*&
ModTreeNode::getRight()
{
	return _right;
}

inline
const ModTreeNode*
ModTreeNode::getRight() const
{
	return _right;
}

//	FUNCTION public
//	ModTreeNode::setRight -- 右の子ノードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*	v
//			右の子ノードとして設定したいノードを表す
//			クラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTreeNode::setRight(ModTreeNode* v)
{
	_right = v;
}

//	FUNCTION public
//	ModTreeNode::getLeft -- 左の子ノードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		左の子ノードを表すクラスを格納する領域の先頭アドレスを返す
//
//	EXCEPTIONS
//		なし

inline
ModTreeNode*&
ModTreeNode::getLeft()
{
	return _left;
}

inline
const ModTreeNode*
ModTreeNode::getLeft() const
{
	return _left;
}

//	FUNCTION public
//	ModTreeNode::setLeft -- 左の子ノードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*	v
//			左の子ノードとして設定したいノードを表す
//			クラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModTreeNode::setLeft(ModTreeNode* v)
{
	_left = v;
}

//	CLASS
//	ModTreeFunctions --
//		Red-Black 木を操作するための雑関数へのインタフェースクラス
//
//	NOTES

class ModCommonDLL ModTreeFunctions
{
public:
	static void
	increment(ModTreeNode*& node, const ModTreeNode* nilNode);
	static void
	increment(const ModTreeNode*& node, const ModTreeNode* nilNode);
	static void
	decrement(ModTreeNode*& node, const ModTreeNode* nilNode);
	static void
	decrement(const ModTreeNode*& node, const ModTreeNode* nilNode);

	static void
	rotateRight(ModTreeNode* node,
				ModTreeNode*& root, const ModTreeNode* nilNode);
	static void
	rotateLeft(ModTreeNode* node,
			   ModTreeNode*& root, const ModTreeNode* nilNode);

	static void
	insert(ModBoolean leftFlag,
		   ModTreeNode* node1, ModTreeNode* node2, ModTreeNode* node3,
		   ModTreeNode*& rightMost, ModTreeNode*& leftMost,
		   ModTreeNode*& root, ModTreeNode* header, ModTreeNode* nilNode);
	static ModTreeNode*
	erase(ModTreeNode* node,
		  ModTreeNode*& rightMost, ModTreeNode*& leftMost,
		  ModTreeNode*& root, ModTreeNode* header, const ModTreeNode* nilNode);

	static ModTreeNode*
	minimum(ModTreeNode* node,
			ModTreeNode* header, const ModTreeNode* nilNode);
	static ModTreeNode*
	maximum(ModTreeNode* node,
			ModTreeNode* header, const ModTreeNode* nilNode);
};

//	TEMPLATE CLASS
//	ModValueNode<ValueType> -- Red-Black 木のノードを表すクラス
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES

template <class ValueType>
class ModValueNode
	: public	ModTreeNode
{
public:
	// デフォルトコンストラクター
	ModValueNode();
	// コンストラクター
	ModValueNode(const ValueType& v);

	// 値を得る
	ValueType&					getData();
	const ValueType&			getData() const;
	// 値を設定する
	void						setData(const ValueType& v);

private:
	ValueType					_value;			// 値
};

//	TEMPLATE FUNCTION public
//	ModValueNode<ValueType>::ModValueNode --
//		Red-Black 木のノードを表すクラスのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			Red-Black 木に登録する値の型
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
ModValueNode<ValueType>::ModValueNode()
{}

//	TEMPLATE FUNCTION public
//	ModValueNode<ValueType>::ModValueNode --
//		Red-Black 木のノードを表すクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			木に登録される値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&		v
//			設定する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
ModValueNode<ValueType>::ModValueNode(const ValueType& v)
	: _value(v)
{}

//	TEMPLATE FUNCTION public
//	ModValueNode<ValueType>::getData -- 値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			木に登録されている値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType&
ModValueNode<ValueType>::getData()
{
	return _value;
}

//	TEMPLATE FUNCTION public
//	ModValueNode<ValueType>::setData -- 値を設定する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&		v
//			設定する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class ValueType>
inline
void
ModValueNode<ValueType>::setData(const ValueType& v)
{
	_value = v;
}

//	MACRO
//	ModGetValue -- ModTreeNode に設定されている値を得るためのマクロ
//
//	NOTES

#define	ModGetValue(node)	((ModValueNode<ValueType>*) node)->getData()
#define	_ModGetValue(node)	((ModValueNode<_ValueType>*) node)->getData()

//	TEMPLATE CLASS
//	ModTreeIterator<_ValueType> -- Red-Black 木の反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES

template <class _ValueType>
class ModTreeIterator
	: public	ModDefaultObject
{
public:
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType			ValueType;
#endif

	// デフォルトコンストラクター
	ModTreeIterator();
	// コンストラクター
	ModTreeIterator(ModTreeNode* node, ModTreeNode* nilNode);
	// コピーコンストラクター
	ModTreeIterator(const ModTreeIterator<_ValueType>& src);

	// = 演算子
	ModTreeIterator<_ValueType>&
	operator =(const ModTreeIterator<_ValueType>& r);

	// * 単項演算子
	_ValueType&						operator *() const;

	// ++ 前置演算子
	ModTreeIterator<_ValueType>&	operator ++();
	// ++ 後置演算子
	ModTreeIterator<_ValueType>		operator ++(int dummy);
	// -- 前置演算子
	ModTreeIterator<_ValueType>&	operator --();
	// -- 後置演算子
	ModTreeIterator<_ValueType>		operator --(int dummy);

	// == 演算子
	ModBoolean
	operator ==(const ModTreeIterator<_ValueType>& r) const;
	// != 演算子
	ModBoolean
	operator !=(const ModTreeIterator<_ValueType>& r) const;

	// 指しているノードを得る
	ModTreeNode*				getNode() const;

private:
	ModTreeNode*				_node;			// 指しているノード
	ModTreeNode*				_nilNode;		// 木の終端の空ノード
};

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::ModTreeIterator --
//		Red-Black 木の反復子を表すテンプレートクラスの
//		デフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
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

template <class _ValueType>
inline
ModTreeIterator<_ValueType>::ModTreeIterator()
	: _node(0),
	  _nilNode(0)
{}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::ModTreeIterator --
//		Red-Black 木の反復子を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*			node
//			反復子の指すノードを表すクラスを格納する領域の先頭アドレス
//		ModTreeNode*			nilNode
//			木の終端の空ノードを表すクラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>::ModTreeIterator(
	ModTreeNode* node, ModTreeNode* nilNode)
	: _node(node),
	  _nilNode(nilNode)
{}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::ModTreeIterator --
//		Red-Black 木の反復子を表すテンプレートクラスの
//		コピーコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<_ValueType>&	src
//			複写元
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>::ModTreeIterator(
	const ModTreeIterator<_ValueType>& src)
	: _node(src.getNode()),
	  _nilNode(src._nilNode)
{}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<_ValueType>&	r
//			代入元
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>&
ModTreeIterator<_ValueType>::operator =(const ModTreeIterator<_ValueType>& r)
{
	_node = r.getNode();
	_nilNode = r._nilNode;

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator * -- * 単項演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指しているノードに設定されている値へのレファレンスを返す
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
_ValueType&
ModTreeIterator<_ValueType>::operator *() const
{
	return _ModGetValue(getNode());
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator ++ -- ++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子を次のノードを指した自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>&
ModTreeIterator<_ValueType>::operator ++()
{
	ModTreeFunctions::increment(_node, _nilNode);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator ++ -- ++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次のノードを指す前の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>
ModTreeIterator<_ValueType>::operator ++(int dummy)
{
	ModTreeIterator<_ValueType> saved(*this);
	ModTreeFunctions::increment(_node, _nilNode);
	return saved;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator -- -- -- 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子を前のノードを指した自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>&
ModTreeIterator<_ValueType>::operator --()
{
	ModTreeFunctions::decrement(_node, _nilNode);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator -- -- -- 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前のノードを指す前の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeIterator<_ValueType>
ModTreeIterator<_ValueType>::operator --(int dummy)
{
	ModTreeIterator<_ValueType> saved(*this);
	ModTreeFunctions::decrement(_node, _nilNode);
	return saved;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<_ValueType>&	r
//			自分自身と比較する反復子を表すクラス
//
//	RETURN
//		ModTrue
//			自分自身と等しい
//		ModFalse
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModTreeIterator<_ValueType>::operator ==(
	const ModTreeIterator<_ValueType>& r) const
{
	return (getNode() == r.getNode() &&
			_nilNode == r._nilNode) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<_ValueType>&	r
//			自分自身と比較する反復子を表すクラス
//
//	RETURN
//		ModTrue
//			自分自身と等しくない
//		ModFalse
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModTreeIterator<_ValueType>::operator !=(
	const ModTreeIterator<_ValueType>& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModTreeIterator<_ValueType>::getNode -- 指しているノードを得る
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		指しているノードを表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeNode*
ModTreeIterator<_ValueType>::getNode() const
{
	return _node;
}

//	TEMPLATE CLASS
//	ModTreeConstIterator<_ValueType> --
//		Red-Black 木の読み出し専用反復子を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES

template <class _ValueType>
class ModTreeConstIterator
	: public	ModDefaultObject
{
public:
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef _ValueType			ValueType;
#endif
	
	// デフォルトコンストラクター
	ModTreeConstIterator();
	// コンストラクター
	ModTreeConstIterator(const ModTreeNode* node, const ModTreeNode* nilNode);
	// コピーコンストラクター
	ModTreeConstIterator(const ModTreeConstIterator<_ValueType>& src);

	// = 演算子
	ModTreeConstIterator<_ValueType>&
	operator =(const ModTreeConstIterator<_ValueType>& r);

	// * 単項演算子
	const _ValueType&			operator *() const;

	// ++ 前置演算子
	ModTreeConstIterator<_ValueType>&	operator ++();
	// ++ 後置演算子
	ModTreeConstIterator<_ValueType>	operator ++(int dummy);
	// -- 前置演算子
	ModTreeConstIterator<_ValueType>&	operator --();
	// -- 後置演算子
	ModTreeConstIterator<_ValueType>	operator --(int dummy);

	// == 演算子
	ModBoolean
	operator ==(const ModTreeConstIterator<_ValueType>& r) const;
	// != 演算子
	ModBoolean
	operator !=(const ModTreeConstIterator<_ValueType>& r) const;

	// 指しているノードを得る
	const ModTreeNode*			getNode() const;

private:
	const ModTreeNode*			_node;			// 指しているノード
	const ModTreeNode*			_nilNode;		// 木の終端の空ノード
};

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::ModTreeConstIterator --
//		Red-Black 木の読取専用反復子を表すテンプレートクラスの
//		デフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
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

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>::ModTreeConstIterator()
	: _node(0),
	  _nilNode(0)
{}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::ModTreeConstIterator --
//		Red-Black 木の読取専用反復子を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*			node
//			反復子の指すノードを表すクラスを格納する領域の先頭アドレス
//		ModTreeNode*			nilNode
//			木の終端の空ノードを表すクラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>::ModTreeConstIterator(
	const ModTreeNode* node, const ModTreeNode* nilNode)
	: _node(node),
	  _nilNode(nilNode)
{}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::ModTreeConstIterator --
//		Red-Black 木の読取専用反復子を表すテンプレートクラスの
//		コピーコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeConstIterator<_ValueType>&	src
//			複写元
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>::ModTreeConstIterator(
	const ModTreeConstIterator<_ValueType>& src)
	: _node(src.getNode()),
	  _nilNode(src._nilNode)
{}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeConstIterator<_ValueType>&	r
//			代入元
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>&
ModTreeConstIterator<_ValueType>::operator =(
	const ModTreeConstIterator<_ValueType>& r)
{
	_node = r.getNode();
	_nilNode = r._nilNode;

	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator * -- * 単項演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子の指しているノードに設定されている値へのレファレンスを返す
//
//	EXCEPTIONS
//		なし

template <class _ValueType >
inline
const _ValueType&
ModTreeConstIterator<_ValueType>::operator *() const
{
	return _ModGetValue(getNode());
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator ++ -- ++ 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子を次のノードを指した自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>&
ModTreeConstIterator<_ValueType>::operator ++()
{
	ModTreeFunctions::increment(_node, _nilNode);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator ++ -- ++ 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		次のノードを指す前の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>
ModTreeConstIterator<_ValueType>::operator ++(int dummy)
{
	ModTreeConstIterator<_ValueType> saved(*this);
	ModTreeFunctions::increment(_node, _nilNode);
	return saved;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator -- -- -- 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		反復子を前のノードを指した自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>&
ModTreeConstIterator<_ValueType>::operator --()
{
	ModTreeFunctions::decrement(_node, _nilNode);
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator -- -- -- 後置演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		前のノードを指す前の自分自身
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModTreeConstIterator<_ValueType>
ModTreeConstIterator<_ValueType>::operator --(int dummy)
{
	ModTreeConstIterator<_ValueType> saved(*this);
	ModTreeFunctions::decrement(_node, _nilNode);
	return saved;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator == -- == 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeConstIterator<_ValueType>&	r
//			自分自身と比較する読取専用反復子を表すクラス
//
//	RETURN
//		ModTrue
//			自分自身と等しい
//		ModFalse
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModTreeConstIterator<_ValueType>::operator ==(
	const ModTreeConstIterator<_ValueType>& r) const
{
	return (getNode() == r.getNode() &&
			_nilNode == r._nilNode) ? ModTrue : ModFalse;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::operator != -- != 演算子
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<_ValueType>&	r
//			自分自身と比較する読取専用反復子を表すクラス
//
//	RETURN
//		ModTrue
//			自分自身と等しくない
//		ModFalse
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
ModBoolean
ModTreeConstIterator<_ValueType>::operator !=(
	const ModTreeConstIterator<_ValueType>& r) const
{
	return (*this == r) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModTreeConstIterator<_ValueType>::getNode -- 指しているノードを得る
//
//	TEMPLATE ARGUMENTS
//		class _ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		指しているノードを表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class _ValueType>
inline
const ModTreeNode*
ModTreeConstIterator<_ValueType>::getNode() const
{
	return _node;
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
//	TEMPLATE FUNCTION
//	ModValueType -- 反復子の Red-Black 木に登録される値の型を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeIterator<ValueType>&	dummy
//			登録される値の型を得る Red-Black 木の反復子
//
//	RETURN
//		ValueType* にキャストされた 0
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType*
ModValueType(const ModTreeIterator<ValueType>& dummy)
{
	return static_cast<ValueType*>(0);
}

//	TEMPLATE FUNCTION
//	ModValueType -- 読取専用反復子の Red-Black 木に登録される値の型を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			Red-Black 木に登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeConstIterator<ValueType>&	dummy
//			登録される値の型を得る Red-Black 木の読取専用反復子
//
//	RETURN
//		ValueType* にキャストされた 0
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType*
ModValueType(const ModTreeConstIterator<ValueType>& dummy)
{
	return static_cast<ValueType*>(0);
}
#endif

//	TEMPLATE CLASS
//	ModTree<KeyType, ValueType, Compare> --
//		Red-Black 木を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES

template <class KeyType, class ValueType, class Compare>
class ModTree
	: public	ModDefaultObject
{
public:
	typedef ModTreeIterator<ValueType>	Iterator;
	typedef ModTreeConstIterator<ValueType> ConstIterator;

	// コンストラクター
	ModTree(const Compare& compare);
	ModTree(ConstIterator first,
			const ConstIterator& last, const Compare& compare);
	ModTree(const ValueType* first,
			const ValueType* last, const Compare& compare);
	// コピーコンストラクター
	ModTree(const ModTree<KeyType, ValueType, Compare>& src);
	// デストラクター
	~ModTree();

	// = 演算子
	ModTree<KeyType, ValueType, Compare>&
	operator =(const ModTree<KeyType, ValueType, Compare>& r);

	// 探索する
	Iterator				find(const KeyType& key);
	ConstIterator			find(const KeyType& key) const;
	Iterator				lowerBound(const KeyType& key);
	ConstIterator			lowerBound(const KeyType& key) const;
	Iterator				upperBound(const KeyType& key);
	ConstIterator			upperBound(const KeyType& key) const;

	// 値を挿入する
	ModPair<Iterator, ModBoolean>
	insert(const ValueType& value);
	Iterator
	insert(Iterator pos, const ValueType& value);
	void
	insert(Iterator first, const Iterator& last);
	void
	insert(ConstIterator first, const ConstIterator& last);
	void
	insert(const ValueType* first, const ValueType* last);

	// 値を削除する
	ModSize					erase(const KeyType& key);
	void					erase(Iterator position);
	void					erase(Iterator first, const Iterator& last);

	// Red-Black 木を空にする
	void					clear();

	// 先頭のノードを指す反復子を得る
	Iterator				begin();
	ConstIterator			begin() const;
	// 末尾のノードを指す反復子を得る
	Iterator				end();
	ConstIterator			end() const;

	// ひとつも登録されていないか
	ModBoolean				isEmpty() const;
	// ノード数を得る
	ModSize					getSize() const;

private:
	// 挿入下位関数
	Iterator
	insertTree(ModTreeNode* candidate,
			   ModTreeNode* parent, const ValueType& value);
	// 削除下位関数
	void					eraseTree(ModTreeNode* node);

	// コンストラクター下位関数
	void					construct();

	// 最左のノードを得る
	ModTreeNode*&			getLeftMost();
	const ModTreeNode*		getLeftMost() const;
	// 最右のノードを得る
	ModTreeNode*&			getRightMost();
	const ModTreeNode*		getRightMost() const;
	// ルートノードを得る
	ModTreeNode*&			getRoot();
	const ModTreeNode*		getRoot() const;

	ModSize					_count;				// ノード数

	ModTreeNode				_header;			// ルート、最左、最右の
												// ノードを管理する
	ModTreeNode				_nilNode;			// 木の終端の空ノード
	Compare					_compare;			// 比較関数を表すクラス
};

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::ModTree --
//		Red-Black 木を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		Compare&				compare
//			比較関数を提供するクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>::ModTree(const Compare& compare)
	: _count(0),
	  _compare(compare)
{
	construct();
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::ModTree --
//		Red-Black 木を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::ConstIterator		first
//			初期値として使用する最初のノードを指す反復子
//		ModTree<KeyType, ValueType, Compare>::ConstIterator&	last
//			初期値として使用する最後のノードを指す反復子
//		Compare&				compare
//			比較関数を提供するクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>::ModTree(
	ConstIterator first, const ConstIterator& last, const Compare& compare)
	: _count(0),
	  _compare(compare)
{
	construct();
	try {
		insert(first, last);
	} catch (ModException& exception) {
		clear();
		ModRethrow(exception);
	}
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::ModTree --
//		Red-Black 木を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ValueType*				first
//			初期値として使用する最初の値が格納されている領域の先頭アドレス
//		ValueType*				last
//			初期値として使用する最後の値が格納されている領域の先頭アドレス
//		Compare&				compare
//			比較関数を提供するクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>::ModTree(
	const ValueType* first, const ValueType* last, const Compare& compare)
	: _count(0),
	  _compare(compare)
{
	construct();
	try {
		insert(first, last);
	} catch (ModException& exception) {
		clear();
		ModRethrow(exception);
	}
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::ModTree --
//		Red-Black 木を表すテンプレートクラスのコピーコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>&	src
//			複写元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>::ModTree(
	const ModTree<KeyType, ValueType, Compare>& src)
	: _count(0),
	  _compare(src._compare)
{
	construct();
	try {
		insert(src.begin(), src.end());
	} catch (ModException& exception) {
		clear();
		ModRethrow(exception);
	}
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::construct --
//		Red-Black 木を表すテンプレートクラスのコンストラクター下位関数
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
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

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::construct()
{
	_header.setColor(ModTreeNode::red);
	_header.setParent(0);
	_header.setLeft(&_header);
	_header.setRight(&_header);

	getRoot() = &_nilNode;
	getLeftMost() = &_header;
	getRightMost() = &_header;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::~ModTree --
//		Red-Black 木を表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
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

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>::~ModTree()
{
	clear();
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>&	src
//			複写元
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTree<KeyType, ValueType, Compare>&
ModTree<KeyType, ValueType, Compare>::operator =(
	const ModTree<KeyType, ValueType, Compare>& src)
{
	if (this != &src) {
		clear();
		_compare = src._compare;
		insert(src.begin(), src.end());
	}
	return *this;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::find --
//		キーにマッチする値を格納するノードを探す
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			ノードを捜すためのキー
//
//	RETURN
//		end() 以外
//			キーにマッチする値を格納するノードを指す反復子
//		end()
//			キーにマッチする値を格納するノードは見つからなかった
//
// EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::find(const KeyType& key)
{
	Iterator ite = lowerBound(key);
	const Iterator&	end = this->end();
	return (ite == end || _compare(key, (*ite).first) == ModTrue) ?	end : ite;
}

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::ConstIterator
ModTree<KeyType, ValueType, Compare>::find(const KeyType& key) const
{
	ConstIterator ite = lowerBound(key);
	const ConstIterator& end = this->end();
	return (ite == end || _compare(key, (*ite).first) == ModTrue) ? end : ite;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::lowerBound --
//		キーにマッチする値を格納するノードを下限検索する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			ノードを捜すためのキー
//
//	RETURN
//		end() 以外
//			キーにマッチする値を格納するノードを指す反復子
//		end()
//			キーにマッチする値を格納するノードは見つからなかった
//
// EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::lowerBound(const KeyType& key)
{
	ModTreeNode* node1 = getRoot();
	ModTreeNode* node2 = &_header;
	ModBoolean result = ModFalse;

	while (node1 != &_nilNode) {
		node2 = node1;
		node1 = ((result = _compare(
					  ModGetValue(node1).first, key)) == ModTrue) ?
			node1->getRight() : node1->getLeft();
	}

	Iterator ite(node2, &_nilNode);

	if (result == ModTrue)
		++ite;

	return ite;
}

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::ConstIterator
ModTree<KeyType, ValueType, Compare>::lowerBound(const KeyType& key) const
{
	const ModTreeNode* node1 = getRoot();
	const ModTreeNode* node2 = &_header;
	ModBoolean result = ModFalse;

	while (node1 != &_nilNode) {
		node2 = node1;
		node1 = ((result = _compare(
					  ModGetValue(node1).first, key)) == ModTrue) ?
			node1->getRight() : node1->getLeft();
	}

	ConstIterator ite(node2, &_nilNode);

	if (result == ModTrue)
		++ite;

	return ite;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::upperBound --
//		キーにマッチする値を格納するノードを上限検索する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			ノードを捜すためのキー
//
//	RETURN
//		end() 以外
//			キーにマッチする値を格納するノードを指す反復子
//		end()
//			キーにマッチする値を格納するノードは見つからなかった
//
// EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::upperBound(const KeyType& key)
{
	ModTreeNode* node1 = getRoot();
	ModTreeNode* node2 = &_header;
	ModBoolean result = ModFalse;

	while (node1 != &_nilNode) {
		node2 = node1;
		node1 = ((result = _compare(
					  key, ModGetValue(node1).first)) == ModTrue) ?
			node1->getLeft() : node1->getRight();
	}

	return Iterator(node2, &_nilNode);
}

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::ConstIterator
ModTree<KeyType, ValueType, Compare>::upperBound(const KeyType& key) const
{
	const ModTreeNode* node1 = getRoot();
	const ModTreeNode* node2 = &_header;
	ModBoolean result = ModFalse;

	while (node1 != &_nilNode) {
		node2 = node1;
		node1 = ((result = _compare(
					  key, ModGetValue(node1).first)) == ModTrue) ?
			node1->getLeft() : node1->getRight();
	}

	return ConstIterator(node2, &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::insert --	値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			挿入する値
//
//	RETURN
//		Red-Black 木中に値が登録されていないとき
//			挿入した値を持つノードを指す反復子と ModTrue の組
//		Red-Black 木中に値がすでに登録されているとき
//			挿入しようとした値を持つノードを指す反復子と ModFalse の組
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModPair<ModTypename ModTree<KeyType, ValueType, Compare>::Iterator, ModBoolean>
ModTree<KeyType, ValueType, Compare>::insert(const ValueType& value)
{
	ModTreeNode*	candidate = getRoot();
	ModTreeNode*	parent = &_header;
	ModBoolean		result = ModTrue;

	while (candidate != &_nilNode) {
		parent = candidate;
		candidate =	((result =
			_compare(value.first, ModGetValue(candidate).first)) == ModTrue) ?
			candidate->getLeft() : candidate->getRight();
	}

	Iterator ite(parent, &_nilNode);

	if (result == ModTrue) {
		if (ite == begin())
			return ModPair<Iterator, ModBoolean>(
				insertTree(candidate, parent, value), ModTrue);
		--ite;
	}

	return (_compare((*ite).first, value.first) == ModTrue) ?
		ModPair<Iterator, ModBoolean>(
			insertTree(candidate, parent, value), ModTrue) :
		ModPair<Iterator, ModBoolean>(ite, ModFalse);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::insert --
//		反復子で指定された位置以降に値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::Iterator	pos
//			挿入位置を探すための反復子
//		ValueType&			value
//			挿入する値
//
//	RETURN
//		挿入した値を持つノードを指す反復子
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::insert(
	Iterator pos, const ValueType& value)
{
	if (pos == begin())
		return (getSize() && _compare(value.first, (*pos).first) == ModTrue) ?
			insertTree(pos.getNode(), pos.getNode(), value) :
			insert(value).first;

	if (pos == end())
		return (_compare(ModGetValue(getRightMost()).first,
						 value.first) == ModTrue) ?
			insertTree(&_nilNode, getRightMost(), value) :
			insert(value).first;

	Iterator prev(--pos);

	return (_compare((*prev).first, value.first) == ModFalse ||
			_compare(value.first, (*pos).first) == ModFalse) ?
		insert(value).first :
		(prev.getNode()->getRight() == &_nilNode) ?
		insertTree(&_nilNode, prev.getNode(), value) :
		insertTree(pos.getNode(), pos.getNode(), value);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::insert -- 複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::Iterator	first
//			挿入する最初の値を格納するノードを指す反復子
//		ModTree<KeyType, ValueType, Compare>::Iterator&	last
//			挿入する最後の値を格納するノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::insert(
	Iterator first, const Iterator& last)
{
	for (; first != last; ++first)
		insert(*first);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::insert -- 複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::ConstIterator	first
//			挿入する最初の値を格納するノードを指す読取専用反復子
//		ModTree<KeyType, ValueType, Compare>::ConstIterator&	last
//			挿入する最後の値を格納するノードを指す読取専用反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::insert(
	ConstIterator first, const ConstIterator& last)
{
	for (; first != last; ++first)
		insert(*first);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::insert -- 複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ValueType*				first
//			挿入する最初の値が格納されている領域の先頭アドレス
//		ValueType*				last
//			挿入する最後の値が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::insert(
	const ValueType* first, const ValueType* last)
{
	for (; first != last; ++first)
		insert(*first);
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::insertTree -- 挿入下位関数
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*			candidate
//			挿入位置の第一候補であるノードを格納する領域の先頭アドレス
//		ModTreeNode*			parent
//			挿入位置の第一候補の親であるノードを格納する領域の先頭アドレス
//		ValueType&				value
//			挿入する値
//
//	RETURN
//		挿入した値を持つノードを指す反復子
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::insertTree(
	ModTreeNode* candidate, ModTreeNode* parent, const ValueType& value)
{
	ModValueNode<ValueType>* node = new ModValueNode<ValueType>(value);
	++_count;

	ModBoolean left =
		(parent == &_header || candidate != &_nilNode ||
		 _compare(value.first, ModGetValue(parent).first) == ModTrue) ?
		ModTrue : ModFalse;

	ModTreeFunctions::insert(left, candidate, parent, node,
							 getRightMost(), getLeftMost(),
							 getRoot(), &_header, &_nilNode);

	return Iterator(node, &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::erase --
//		キーにマッチする値を格納するノードを削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		KeyType&			key
//			削除するノードを捜すためのキー
//
//	RETURN
//		削除したノードの個数
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
ModSize
ModTree<KeyType, ValueType, Compare>::erase(const KeyType& key)
{
	Iterator		ite(begin());
	const Iterator&	end = this->end();

	for (; ite != end && (*ite).first < key; ++ite) ;
	if (ite == end || (*ite).first > key)
		return 0;

	Iterator	first(ite);
	ModSize		n = 1;
	for (++ite; ite != end && (*ite).first == key; ++ite, ++n) ;

	erase(first, ite);

	return n;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::erase --
//		反復子の指すノードを削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::Iterator pos
//			削除するノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::erase(Iterator pos)
{
	ModValueNode<ValueType>* node =
		(ModValueNode<ValueType>*)
			ModTreeFunctions::erase(pos.getNode(),
									getRightMost(),	getLeftMost(),
									getRoot(), &_header, &_nilNode);
	--_count;
	delete node;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::erase --
//		反復子で指定された範囲のノードを削除する
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTree<KeyType, ValueType, Compare>::Iterator	first
//			削除する最初のノードを指す反復子
//		ModTree<KeyType, ValueType, Compare>::Iterator&	last
//			削除する最後のノードを指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::erase(
	Iterator first,	const Iterator& last)
{
	while (first != last)
		this->erase(first++);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::clear -- 空にする
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
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

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare>::clear()
{
	if (getSize()) {
		eraseTree(getRoot());

		getRoot() = &_nilNode;
		getLeftMost() = &_header;
		getRightMost() = &_header;

		_count = 0;
	}
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::eraseTree -- 削除下位関数
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		ModTreeNode*			node
//			このノード以下のすべてのノードを削除する
//
//	RETURN
//		なし
//
// EXCEPTIONS

template <class KeyType, class ValueType, class Compare>
inline
void
ModTree<KeyType, ValueType, Compare >::eraseTree(ModTreeNode* node)
{
	if (node != &_nilNode) {
		eraseTree(node->getLeft());
		eraseTree(node->getRight());
		delete (ModValueNode<ValueType>*) node;
	}
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::begin --
//		先頭のノードを指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		先頭のノードを指す反復子
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::begin()
{
	return Iterator(getLeftMost(), &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::begin --
//		先頭のノードを指す読取専用反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		先頭のノードを指す読取専用反復子
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::ConstIterator
ModTree<KeyType, ValueType, Compare>::begin() const
{
	return ConstIterator(getLeftMost(), &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::end --
//		末尾のノードを指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		末尾のノードを指す反復子
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::Iterator
ModTree<KeyType, ValueType, Compare>::end()
{
	return Iterator(&_header, &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::end --
//		末尾のノードを指す読取専用反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		末尾のノードを指す読取専用反復子
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTypename ModTree<KeyType, ValueType, Compare>::ConstIterator
ModTree<KeyType, ValueType, Compare>::end() const
{
	return ConstIterator(&_header, &_nilNode);
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::isEmpty -- ひとつも登録されていないか
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
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

template <class KeyType, class ValueType, class Compare>
inline
ModBoolean
ModTree<KeyType, ValueType, Compare>::isEmpty() const
{
	return (getSize()) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModTree<KeyType, ValueType, Compare>::getSize -- ノード数を得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたノード数
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModSize
ModTree<KeyType, ValueType, Compare>::getSize() const
{
	return _count;
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::getLeftMost -- 最左のノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		最左のノードを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTreeNode*&
ModTree<KeyType, ValueType, Compare >::getLeftMost()
{
	return _header.getLeft();
}

template <class KeyType, class ValueType, class Compare>
inline
const ModTreeNode*
ModTree<KeyType, ValueType, Compare >::getLeftMost() const
{
	return _header.getLeft();
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::getRightMost -- 最右のノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		最右のノードを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTreeNode*&
ModTree<KeyType, ValueType, Compare >::getRightMost()
{
	return _header.getRight();
}

template <class KeyType, class ValueType, class Compare>
inline
const ModTreeNode*
ModTree<KeyType, ValueType, Compare >::getRightMost() const
{
	return _header.getRight();
}

//	TEMPLATE FUNCTION private
//	ModTree<KeyType, ValueType, Compare>::getRoot -- ルートノードを得る
//
//	TEMPLATE ARGUMENTS
//		class KeyType
//			木に登録する値のキーの型
//		class ValueType
//			木に登録する値のバリューの型
//		class Compare
//			比較関数を提供するクラス
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ルートノードを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

template <class KeyType, class ValueType, class Compare>
inline
ModTreeNode*&
ModTree<KeyType, ValueType, Compare >::getRoot()
{
	return _header.getParent();
}

template <class KeyType, class ValueType, class Compare>
inline
const ModTreeNode*
ModTree<KeyType, ValueType, Compare >::getRoot() const
{
	return _header.getParent();
}

#endif	// __ModTree_H__

//
// Copyright (c) 1997, 2002, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
