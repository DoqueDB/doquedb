// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModVector.h -- ベクターに関するクラス定義
// 
// Copyright (c) 1997, 1999, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModVector_H__
#define __ModVector_H__

#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModException.h"
#include "ModAlgorithm.h"

//	TEMPLATE CLASS
//	ModVector -- ベクターを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//		ベクターは、型 ValueType の値を持つ要素からなる可変長のシーケンスである
//		シーケンスは、型 ValueType の配列に格納される

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class _ValueType>
class ModVectorConstIterator
{
public:
	typedef _ValueType			ValueType;

	// コンストラクタ
	ModVectorConstIterator(const _ValueType* p = 0)
		: pointer(const_cast<_ValueType*>(p)) {}
	// デストラクタ
	~ModVectorConstIterator() {}

	// * 単項演算子
	const _ValueType& operator *() const { return *pointer; }
	// -> 単項演算子
	const _ValueType* operator ->() const { return &(**this); }

	// ++ 前置演算子
	ModVectorConstIterator<_ValueType>& operator ++()
		{
			++pointer;
			return *this;
		}
	// ++ 後置演算子
	ModVectorConstIterator<_ValueType> operator ++(int)
		{
			ModVectorConstIterator<_ValueType> tmp = *this;
			++(*this);
			return tmp;
		}
	// -- 前置演算子
	ModVectorConstIterator<_ValueType>& operator --()
		{
			--pointer;
			return *this;
		}
	// -- 後置演算子
	ModVectorConstIterator<_ValueType> operator --(int)
		{
			ModVectorConstIterator<_ValueType> tmp = *this;
			--(*this);
			return tmp;
		}

	// += 演算子
	ModVectorConstIterator<_ValueType>& operator +=(ModSize distance)
		{
			pointer += distance;
			return *this;
		}
	// + 演算子
	ModVectorConstIterator<_ValueType> operator +(ModSize distance) const
		{
			ModVectorConstIterator<_ValueType> tmp = *this;
			return (tmp += distance);
		}
	// + 二項演算子
	friend ModVectorConstIterator<_ValueType> operator +(
		ModSize distance,
		const ModVectorConstIterator<_ValueType>& right)
		{
			return (right + distance);
		}

	// -= 演算子
	ModVectorConstIterator<_ValueType>& operator -=(ModSize distance)
		{
			pointer -= distance;
			return *this;
		}
	// - 演算子
	ModVectorConstIterator<_ValueType> operator - (ModSize distance) const
		{
			ModVectorConstIterator<_ValueType> tmp = *this;
			return (tmp -= distance);
		}

	// 距離を返す
	int operator - (const ModVectorConstIterator<_ValueType>& right) const
		{
			return (int)(pointer - right.pointer);
		}
	friend int operator - (const _ValueType* p,
						   const ModVectorConstIterator<_ValueType>& right)
		{
			return (int)(p - right.pointer);
		}

	// [] 演算子
	const _ValueType& operator [](int position) const
		{
			return *(pointer + position);
		}

	// == 演算子
	bool operator ==(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer == other.pointer);
		}
	// != 演算子
	bool operator !=(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer != other.pointer);
		}
	// < 演算子
	bool operator <(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer < other.pointer);
		}
	// > 演算子
	bool operator >(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer > other.pointer);
		}
	// <= 演算子
	bool operator <=(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer <= other.pointer);
		}
	// >= 演算子
	bool operator >=(const ModVectorConstIterator<_ValueType>& other) const
		{
			return (pointer >= other.pointer);
		}

protected:
	mutable _ValueType* pointer;
};

template <class _ValueType>
class ModVectorIterator : public ModVectorConstIterator<_ValueType>
{
	typedef ModVectorConstIterator<_ValueType>	Super;
	
public:
	// コンストラクタ
	ModVectorIterator(_ValueType* p = 0)
		: ModVectorConstIterator<_ValueType>(p) {}
	// デストラクタ
	~ModVectorIterator() {}

	// * 単項演算子
	_ValueType& operator *() const { return *Super::pointer; }
	// -> 単項演算子
	_ValueType* operator ->() const { return &(**this); }

	// ++ 前置演算子
	ModVectorIterator<_ValueType>& operator ++()
		{
			++Super::pointer;
			return *this;
		}
	// ++ 後置演算子
	ModVectorIterator<_ValueType> operator ++(int)
		{
			ModVectorIterator<_ValueType> tmp = *this;
			++(*this);
			return tmp;
		}
	// -- 前置演算子
	ModVectorIterator<_ValueType>& operator --()
		{
			--Super::pointer;
			return *this;
		}
	// -- 後置演算子
	ModVectorIterator<_ValueType> operator --(int)
		{
			ModVectorIterator<_ValueType> tmp = *this;
			--(*this);
			return tmp;
		}

	// += 演算子
	ModVectorIterator<_ValueType>& operator +=(ModSize distance)
		{
			Super::pointer += distance;
			return *this;
		}
	// + 演算子
	ModVectorIterator<_ValueType> operator +(ModSize distance) const
		{
			ModVectorIterator<_ValueType> tmp = *this;
			return (tmp += distance);
		}
	// + 二項演算子
	friend ModVectorIterator<_ValueType> operator +(
		ModSize distance,
		const ModVectorIterator<_ValueType>& right)
		{
			return (right + distance);
		}

	// -= 演算子
	ModVectorIterator<_ValueType>& operator -=(ModSize distance)
		{
			Super::pointer -= distance;
			return *this;
		}
	// - 演算子
	ModVectorIterator<_ValueType> operator - (ModSize distance) const
		{
			ModVectorIterator<_ValueType> tmp = *this;
			return (tmp -= distance);
		}

	// 距離を返す
	int operator - (const ModVectorIterator<_ValueType>& right) const
		{
			return (int)(Super::pointer - right.pointer);
		}
	friend int operator - (const _ValueType* p,
						   const ModVectorIterator<_ValueType>& right)
		{
			return (int)(p - right.pointer);
		}

	// [] 演算子
	_ValueType& operator [](int position) const
		{
			return *(Super::pointer + position);
		}
	
};
#endif

template <class ValueType>
class ModVector
	: public	ModDefaultObject
{
public:
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	typedef ModVectorIterator<ValueType>		Iterator;
	typedef ModVectorConstIterator<ValueType>	ConstIterator;
#else
	typedef ValueType*			Iterator;
	typedef const ValueType*	ConstIterator;
#endif

	ModVector();
	ModVector(ModSize n, const ValueType& value = ValueType());
	ModVector(const ModVector<ValueType>& src);	// コンストラクター
	~ModVector();								// デストラクター

	ModVector<ValueType>&	operator =(const ModVector<ValueType>& src);
												// = 演算子

	void					assign(ConstIterator first, ConstIterator last);
	void					assign(ModSize n,
								   const ValueType& value = ValueType());
												// 値を割り当てる

	ModBoolean				isEmpty() const;	// 空か調べる

	Iterator				insert(Iterator position,
								   const ValueType& value = ValueType());
	void					insert(Iterator position,
								   ModSize n, const ValueType& value);
	void					insert(Iterator position,
								   ConstIterator first, ConstIterator last);
												// ある位置へ値を挿入する

	void					pushFront(const ValueType& value);
												// 先頭に値を挿入する
	void					pushBack(const ValueType& value);
												// 末尾に値を挿入する

	void					reserve(ModSize n);	// 要素格納用領域を確保する

	Iterator				erase(Iterator position);
	Iterator				erase(Iterator first, const Iterator last);
												// 削除する

	void					popFront();			// 先頭の要素を削除する
	void					popBack();			// 末尾の要素を削除する

	void					clear();			// 全要素および
												// 要素格納用領域を破棄する

	void					swap(ModVector<ValueType>& r);
												// 全要素を入れ替える

	Iterator				find(const ValueType& key);
	ConstIterator			find(const ValueType& key) const;
												// 等しい値を持つ要素を
												// 先頭から探す
	ModBoolean				isFound(const ValueType& key) const;
												// 等しい値をもつ要素が
												// あるか調べる

	ValueType&				getFront();
	const ValueType&		getFront() const;	// 先頭の要素の値を得る

	ValueType&				getBack();
	const ValueType&		getBack() const;	// 最後の要素の値を得る

	// 以下、近日中に廃止します
	ValueType&				back();
	const ValueType&		back() const;		// 最後の要素の値を得る
	// 以上、近日中に廃止します

	ValueType&				at(ModSize i);
	const ValueType&		at(ModSize i) const;
												// 先頭から指定された
												// 数番目の要素の値を得る
	ValueType&				operator [](ModSize i);
	const ValueType&		operator [](ModSize i) const;
												// [] 演算子

	Iterator				begin();		
	ConstIterator			begin() const;		// 先頭の要素を指す反復子を得る
	Iterator				end();
	ConstIterator			end() const;		// end を指す反復子を得る

	ModSize					getSize() const;	// 要素数を得る
	ModSize					getCapacity() const;// 格納可能な要素数を得る

private:
	ValueType*				_begin;				// 先頭の要素を指す反復子
	ValueType*				_end;				// end を指す反復子
	ValueType*				_endOfStorage;		// 要素を格納する領域の
												// 末尾を指す反復子
};

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::ModVector -- ベクターのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
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
ModVector<ValueType>::ModVector()
	: _begin(0),
	  _end(0),
	  _endOfStorage(0)
{ }

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::ModVector --
//		ある値を複数個持つベクターのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				n
//			生成されるベクターの要素数
//		ValueType&			value
//			指定されたとき
//				生成されるベクターの要素の値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::insert より)

template <class ValueType>
inline
ModVector<ValueType>::ModVector(ModSize n, const ValueType& value)
	: _begin(0),
	  _end(0),
	  _endOfStorage(0)
{
	this->insert(this->begin(), n, value);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::ModVector -- ベクターのコピーコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>&	src
//			生成されるベクターに割り当てる要素を持つベクター
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::assign より)

template <class ValueType>
inline
ModVector<ValueType>::ModVector(const ModVector<ValueType>& src)
	: _begin(0),
	  _end(0),
	  _endOfStorage(0)
{
	// 指定されたベクターの要素をコピーして、割り当てる

	this->assign(src.begin(), src.end());
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::~ModVector -- ベクターのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
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
ModVector<ValueType>::~ModVector()
{
	// 全要素を削除し、要素格納用の領域を破棄する

	this->clear();
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>&	src
//			自分自身へ代入するベクター
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::assign より)

template <class ValueType>
inline
ModVector<ValueType>&
ModVector<ValueType>::operator =(const ModVector<ValueType>& src)
{
	if (this != &src)
		this->assign(src.begin(), src.end());

	return *this;
}

//	TEMPLATE FUNCTION private
//	ModVector<ValueType>::assign -- 指定された範囲の複数の値を割り当てる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//		自分自身の持つ要素はすべて削除され、
//		与えられた複数の値が新しい要素になる
//
//	ARGUMENTS
//		ModVector<ValueType>::ConstIterator		first
//			最初に割り当てる値を持つ要素を指す反復子
//		ModVector<ValueType>::ConstIterator		last
//			最後に割り当てる値を持つ要素の 1 つ後を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::insert より)

template <class ValueType>
// inline
void
ModVector<ValueType>::assign(ConstIterator first, ConstIterator last)
{
	ModSize n = last - first;
	if (first == this->begin())
		(void) this->erase(this->begin() + n, this->end());
	else if (n > this->getCapacity()) {
		this->clear();
		this->insert(this->begin(), first, last);
	} else {
		(void) this->erase(this->begin(), this->end());
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		_end = &*ModCopy(first, last, this->begin());
#else
		_end = ModCopy(first, last, this->begin());
#endif
	}
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::assign -- ある値を複数個割り当てる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
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
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::insert より)

template <class ValueType>
// inline
void
ModVector<ValueType>::assign(ModSize n, const ValueType& value)
{
	if (n > this->getCapacity()) {
		this->clear();
		this->insert(this->begin(), n, value);
	} else if (this->getSize() >= n) {
		ModFillWithNumber(this->begin(), n, value);
		(void) this->erase(this->begin() + n, this->end());
	} else {
		(void) this->erase(this->begin(), this->end());
		ModFillWithNumber(this->begin(), n, value);
		_end = _begin + n;
	}
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::isEmpty -- ベクターに要素がないか調べる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ベクターに要素はない
//		ModFalse
//			ベクターに要素はある
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModBoolean
ModVector<ValueType>::isEmpty() const
{
	return (this->getSize()) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::insert -- ある値を 1 つ挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>::Iterator			position
//			値を挿入する位置を指す反復子
//		ValueType&			value
//			指定されたとき
//				挿入する値
//			指定されないとき
//				ValueType() が指定されたものとみなす
//
//	RETURN
//		挿入された値を持つ要素を指す反復子
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている

template <class ValueType>
inline
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::insert(Iterator position, const ValueType& value)
{
	if (position == this->end() && this->end() != _endOfStorage) {
		*_end = value;
		return _end++;
	}

	ModSize ofs = position - this->begin();
	this->insert(position, 1, value);
	return this->begin() + ofs;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::insert -- ある値を複数個挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>::Iterator			position
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
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている

template <class ValueType>
// inline
void
ModVector<ValueType>::insert(Iterator position,
							 ModSize n, const ValueType& value)
{
	if (n == 0)
		return;

	if ((ModSize) (_endOfStorage - this->end()) >= n) {

		// 確保済の領域で挿入する個数分を収められる

		if (position + n < this->end())
			(void) ModCopyBackward(position, this->end(), this->end() + n);
		else
			(void) ModCopy(position, this->end(), position + n);

		ModFillWithNumber(position, n, value);

		_end += n;
		return;
	}

	// 確保済の領域では足りないので、
	// 最低でも現状の 2 倍以上格納できるように領域を確保する

	ModSize	current = this->getSize();
	ModSize	size = current + ModMax(current, n);

	ValueType* p;
	try {
		//【注意】	new 演算子から呼び出される関数は
		//			もし、関数 operator ValueType::new[] が定義されていれば、
		//			それが呼び出され、そうでなければ、
		//			大域関数 operator ::new[] が呼び出される
		//
		//			これらの関数の領域の確保に失敗したときの動作は、
		//			予想できないため、ModException が発生したとき以外は、
		//			すべて、例外 ModOsErrorSystemMemoryExhaust を
		//			発生させることにする

		p = new ValueType[size];

	} catch (ModException& exception) {
		ModRethrow(exception);
	} catch (...) {
		ModThrow(ModModuleStandard,
				 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);
	}
	if (p == 0)
		ModThrow(ModModuleStandard,
				 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);

	try {
		// 新しく確保した領域に現在の値と、挿入する値を複写する

		(void) ModCopy(this->begin(), position, p);
		ModFillWithNumber(p + (position - this->begin()), n, value);
		(void) ModCopy(position, this->end(),
					   p + (position - this->begin()) + n);

	} catch (ModException& exception) {

		delete [] p;
		ModRethrow(exception);
	}

	// 全要素を削除し、要素格納用の領域を破棄する

	this->clear();

	// 新しく確保した領域を使用するようにする

	_begin = p;
	_end = p + current + n;
	_endOfStorage = p + size;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::insert -- 指定された範囲の複数の値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>::Iterator			position
//			値を挿入する位置を指す反復子
//		ModVector<ValueType>::ConstIterator		first
//			最初に挿入する値を持つ要素を指す反復子
//		ModVector<ValueType>::ConstIterator		last
//			最後に挿入する値を持つ要素の 1 つ後を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている

template <class ValueType>
// inline
void
ModVector<ValueType>::insert(Iterator position,
							 ConstIterator first, ConstIterator last)
{
	if (first == last || last < first)
		return;

	ModSize	n = last - first;
	if ((ModSize) (_endOfStorage - this->end()) >= n) {

		// 確保済の領域で挿入する個数分を収められる

		if (position + n < this->end())
			(void) ModCopyBackward(position, this->end(), this->end() + n);
		else
			(void) ModCopy(position, this->end(), position + n);

		(void) ModCopy(first, last, position);

		_end += n;
		return;
	}

	// 確保済の領域では足りないので、
	// 最低でも現状の 2 倍以上格納できるように領域を確保する

	ModSize	current = this->getSize();
	ModSize	size = current + ModMax(current, n);

	ValueType* p;
	try {
		//【注意】	new 演算子から呼び出される関数は
		//			もし、関数 operator ValueType::new[] が定義されていれば、
		//			それが呼び出され、そうでなければ、
		//			大域関数 operator ::new[] が呼び出される
		//
		//			これらの関数の領域の確保に失敗したときの動作は、
		//			予想できないため、ModException が発生したとき以外は、
		//			すべて、例外 ModOsErrorSystemMemoryExhaust を
		//			発生させることにする

		p = new ValueType[size];

	} catch (ModException& exception) {
		ModRethrow(exception);
	} catch (...) {
		ModThrow(ModModuleStandard,
				 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);
	}
	if (p == 0)
		ModThrow(ModModuleStandard,
				 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);

	try {
		// 新しく確保した領域に現在の値と、挿入する値を複写する

		(void) ModCopy(this->begin(), position, p);
		(void) ModCopy(first, last, p + (position - this->begin()));
		(void) ModCopy(position, this->end(),
					   p + (position - this->begin()) + n);

	} catch (ModException& exception) {

		delete [] p;
		ModRethrow(exception);
	}

	// 全要素を削除し、要素格納用の領域を破棄する

	this->clear();

	// 新しく確保した領域を使用するようにする

	_begin = p;
	_end = p + current + n;
	_endOfStorage = p + size;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::pushFront -- 先頭に値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			先頭に挿入する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::insert より)

template <class ValueType>
inline
void
ModVector<ValueType>::pushFront(const ValueType& value)
{
	this->insert(this->begin(), 1, value);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::pushBack -- 末尾に値を挿入する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&			value
//			挿入する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている
//			(ModVector<ValueType>::insert より)

template <class ValueType>
inline
void
ModVector<ValueType>::pushBack(const ValueType& value)
{
	if (this->end() == _endOfStorage)

		// 要素格納用領域を使い切っているので、
		// 格納用領域を増やしてから、末尾に指定された値を挿入する

		this->insert(this->end(), 1, value);
	else
		// 要素格納用領域が余っている

		*_end++ = value;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::reserve -- 要素の格納に使用する領域を確保する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				n
//			確保する要素格納用の領域に格納可能な要素数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムで利用可能なメモリーを使い切っている

template <class ValueType>
// inline
void
ModVector<ValueType>::reserve(ModSize n)
{
	// 指定された数が
	// 現在確保されている領域に格納な要素数と異り、
	// 現在の要素数以上であるか調べる
	//
	//【注意】	現在の要素数以上であれば、
	//			現在確保されている領域に格納可能な要素数より
	//			小さくすることができる

	ModSize	current;
	if (this->getCapacity() != n &&	(current = this->getSize()) <= n) {

		// 指定された数の要素を格納可能な領域を確保する

		ValueType*	p;
		try {
			p = new ValueType[n];

		} catch (ModException& exception) {
			ModRethrow(exception);
		} catch (...) {
			ModThrow(ModModuleStandard,
					 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);
		}
		if (p == 0)
			ModThrow(ModModuleStandard,
					 ModOsErrorSystemMemoryExhaust, ModErrorLevelFatal);

		try {
			// 新しく確保した領域に現在の値を複写する

			(void) ModCopy(this->begin(), this->end(), p);

		} catch (ModException& exception) {

			delete [] p;
			ModRethrow(exception);
		}

		// 全要素を削除し、要素格納用の領域を破棄する

		this->clear();

		// 新しく確保した領域を使用するようにする

		_begin = p;
		_end = p + current;
		_endOfStorage = p + n;
	}
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::erase -- 反復子の指す要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>::Iterator		position
//			削除する要素を指す反復子
//
//	RETURN
//		削除した要素の次の要素を指す反復子
//
//	EXCEPTIONS

template <class ValueType>
inline
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::erase(Iterator position)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	_end = &*ModCopy(position + 1, this->end(), position);
#else
	_end = ModCopy(position + 1, this->end(), position);
#endif
	return position;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::erase -- 反復子で指定された範囲の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>::Iterator		first
//			最初に削除する要素を指す反復子
//		ModVector<ValueType>::Iterator		last
//			最後に削除するものの 1 つ後の要素を指す反復子
//
//	RETURN
//		削除した要素の次の要素を指す反復子
//
//	EXCEPTIONS

template <class ValueType>
inline
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::erase(Iterator first, const Iterator last)
{
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	_end = &*ModCopy(last, this->end(), first);
#else
	_end = ModCopy(last, this->end(), first);
#endif
	return first;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::popFront -- 先頭の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
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
ModVector<ValueType>::popFront()
{
	(void) this->erase(this->begin());
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::popBack -- 末尾の要素を削除する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
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
ModVector<ValueType>::popBack()
{
	if (this->isEmpty() == ModFalse)
		(void) this->erase(this->end() - 1);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::clear -- 全要素を削除し、要素格納用の領域を破棄する
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//		この関数は要素格納用の領域を破棄するので、
//		erase(begin(), end()) と異なる
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
ModVector<ValueType>::clear()
{
	// デストラクターが呼び出される必要があるので、
	// operator delete で壊す

	delete [] _begin, _begin = 0;

	_end = 0;
	_endOfStorage = 0;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::swap -- 全要素を入れ替える
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>&	r
//			自分自身と全要素を交換するベクター
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
void
ModVector<ValueType>::swap(ModVector<ValueType>& r)
{
	ModSwap(_begin, r._begin);
	ModSwap(_end, r._end);
	ModSwap(_endOfStorage, r._endOfStorage);
}

// ModSwap の ModVector<ValueType> の特別バージョンを定義する

template <class ValueType>
inline
void
ModSwap(ModVector<ValueType>& l, ModVector<ValueType>& r)
{
	l.swap(r);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::find -- ある値と等しい値を持つ要素を先頭から探す
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//		指定された値と等しい値を持つ要素を先頭から探し、
//		最初に見つかった要素を指す反復子を返す
//		見つからなかったとき、end を指す反復子を返す
//
//	ARGUMENTS
//		ValueType&				key
//			ベクター中を探す要素
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
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::find(const ValueType& key)
{
	const Iterator	end = this->end();

	if (this->isEmpty() == ModFalse) {
		Iterator	iterator = this->begin();
		for (; iterator != end; ++iterator)
			if (*iterator == key)
				return iterator;	// 見つかった
	}

	return end;
}

template <class ValueType>
inline
ModTypename ModVector<ValueType>::ConstIterator
ModVector<ValueType>::find(const ValueType& key) const
{
	ConstIterator	end = this->end();

	if (this->isEmpty() == ModFalse) {
		ConstIterator	iterator = this->begin();
		for (; iterator != end; ++iterator)
			if (*iterator == key)
				return iterator;	// 見つかった
	}

	return end;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::isFound -- ある値と等しい値をもつ要素があるか調べる
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ValueType&				key
//			ベクター中を探す要素
//
//	RETURN
//		ModTrue
//			あった
//		ModFalse
//			なかった
//
//	EXCEPTIONS

template <class ValueType>
inline
ModBoolean
ModVector<ValueType>::isFound(const ValueType& key) const
{
	return (isEmpty() || find(key) == end()) ? ModFalse : ModTrue;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::getFront -- 先頭の要素の値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたベクターの先頭の要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			サイズが 0 のベクターの先頭の要素を得ようとしている

template <class ValueType>
inline
ValueType&
ModVector<ValueType>::getFront()
{
	if (this->getSize() == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this->begin();
}

template <class ValueType>
inline
const ValueType&
ModVector<ValueType>::getFront() const
{
	if (this->getSize() == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *this->begin();
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::getBack -- 末尾の要素の値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたベクターの末尾の要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			サイズが 0 のベクターの末尾の要素を得ようとしている

template <class ValueType>
inline
ValueType&
ModVector<ValueType>::getBack()
{
	if (this->getSize() == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *(this->end() - 1);
}

template <class ValueType>
inline
const ValueType&
ModVector<ValueType>::getBack() const
{
	if (this->getSize() == 0)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *(this->end() - 1);
}

template <class ValueType>
inline
ValueType&
ModVector<ValueType>::back()
{
	return this->getBack();
}

template <class ValueType>
inline
const ValueType&
ModVector<ValueType>::back() const
{
	return this->getBack();
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::at --
//		先頭から指定された数番目の要素の値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModSize			i
//			値を得たい要素が先頭から何番目か(先頭の要素は 0 番目)
//
//	RETURN
//		得られたベクターの要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された数番目の要素はベクターに存在しない

template <class ValueType>
inline
ValueType&
ModVector<ValueType>::at(ModSize i)
{
	if (i >= this->getSize())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *(this->begin() + i);
}

template <class ValueType>
inline
const ValueType&
ModVector<ValueType>::at(ModSize i) const
{
	if (i >= this->getSize())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return *(this->begin() + i);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::operator [] --
//		先頭から指定された数番目の要素の値を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModSize			i
//			値を得たい要素が先頭から何番目か(先頭の要素は 0 番目)
//
//	RETURN
//		得られたベクターの要素の値
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された数番目の要素はベクターに存在しない

template <class ValueType>
inline
ValueType&
ModVector<ValueType>::operator [](ModSize i)
{
	return *(this->begin() + i);
}

template <class ValueType>
inline
const ValueType&
ModVector<ValueType>::operator [](ModSize i) const
{
	return *(this->begin() + i);
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::begin -- 先頭の要素を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		先頭の要素を指す反復子
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::begin()
{
	return _begin;
}

template <class ValueType>
inline
ModTypename ModVector<ValueType>::ConstIterator
ModVector<ValueType>::begin() const
{
	return _begin;
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::end -- end (末尾の要素の次)を指す反復子を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		end を指す反復子
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ModTypename ModVector<ValueType>::Iterator
ModVector<ValueType>::end()
{
	return _end;
}

template <class ValueType>
inline
ModTypename ModVector<ValueType>::ConstIterator
ModVector<ValueType>::end() const
{
	return _end;
}

//	TEMPLATE FUNCTION public
//	ModVector::getSize -- 要素数を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ベクターに格納されている要素数
//
// EXCEPTIONS
//		なし

template <class ValueType>
inline
ModSize
ModVector<ValueType>::getSize() const
{
	return (ModSize) (this->end() - this->begin());
}

//	TEMPLATE FUNCTION public
//	ModVector<ValueType>::getCapacity -- 格納可能な要素数を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ベクターに格納可能な要素数
//
// EXCEPTIONS
//		なし

template <class ValueType>
inline
ModSize
ModVector<ValueType>::getCapacity() const
{
	return (ModSize) (_endOfStorage - this->begin());
}

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
//	TEMPLATE FUNCTION
//	ModValueType<ValueType> --
//		ベクターの反復子の指す要素の値の型を得る
//
//	TEMPLATE ARGUMENTS
//		class ValueType
//			ベクターに登録する値の型
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<ValueType>&	dummy
//			指す要素の値の型を得たいベクター
//
//	RETURN
//		要素の値の型へのポインターにキャストされた 0
//
//	EXCEPTIONS
//		なし

template <class ValueType>
inline
ValueType*
ModValueType(const ModVector<ValueType>& dummy)
{
	return static_cast<ValueType*>(0);
}
#endif

#endif	// __ModVector_H__

//
// Copyright (c) 1997, 1999, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
