// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModAutoPointer.h -- オートポインタークラス
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModAutoPointer_H__
#define	__ModAutoPointer_H__

#include "ModConfig.h"
#include "ModTypes.h"

//	TEMPLATE CLASS
//	ModAutoPointer -- オートポインターを表すクラス
//
//	TEMPLATE ARGUMENTS		
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//			デストラクターが public である必要がある
//
//	NOTES
//		オートポインターの仕様については STL の auto_ptr を参照のこと

template <class X>
class ModAutoPointer
{
public:
	ModAutoPointer(X* pointer = 0);				// (デフォルト)コンストラクター
	ModAutoPointer(const ModAutoPointer<X>& r);	// コピーコンストラクター
	~ModAutoPointer();							// デストラクター

	ModAutoPointer<X>&		operator =(ModAutoPointer<X>& r);
	ModAutoPointer<X>&		operator =(X* r);
												// = 演算子
	X&						operator *() const;
												// * 前置演算子
	X*						operator ->() const;
												// -> 演算子
	operator				X*() const;			// X* へのキャスト演算子

	X*						get() const;		// 保持する生ポインターを返す
	X*						release();			// 保持する生ポインターの
												// 所有権を放棄する
	ModBoolean				isOwner() const;	// 所有権を保持しているか

	virtual void			free();				// 生ポインターを破棄する
private:

	//【注意】	_owner と _pointer の順序は超重要!
	//			コンストラクターでこの順序で初期化される

	ModBoolean				_owner;				// 生ポインターを所有しているか
	X*						_pointer;			// 保持する生ポインター
};

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::ModAutoPointer<X> --
//		オートポインタークラスの(デフォルト)コンストラクター
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		X*					pointer
//			生成するオートポインターに保持させる生ポインター
//
//	RETURN
//		生成された自分自身
//
//	EXCEPTIONS
//		なし

template <class X>
inline
ModAutoPointer<X>::ModAutoPointer(X* pointer)
	: _owner(pointer ? ModTrue : ModFalse),
	  _pointer(pointer)
{ }

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::ModAutoPointer<X> --
//		オートポインタークラスのコピーコンストラクター
//	
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//		STL のように他のクラスのオブジェクトをさす
//		オートポインターはコピーできない
//
//		以下のように定義すれば、STL のように他のクラスの
//		オブジェクトをさすオートポインターをコピーできるはずである
//
//		template <class X>
//		template <class Y>
//		inline
//		ModAutoPointer<X>::ModAutoPointer(const ModAutoPointer<Y>& r)
//			: _owner(r.isOwner()),
//			  _pointer(r.release())
//		{ }
//
//		SUN C++ 4.2 Patch 104631-04、MS C++ 11.00.7022 では、
//		テンプレートクラスのメンバー関数として、明示的な
//		テンプレート関数をテンプレートクラス外で定義すると、
//		構文エラーになる
//
//		また、SUN C++ 4.2 Patch 104631-04 では、
//		テンプレートクラス内で定義しても、構文エラーになる
//
//	ARGUMENTS
//		ModAutoPointer<Y>&	r
//			コピー元のオートポインター
//
//	RETURN
//		コピーされた自分自身
//
//	EXCEPTIONS
//		なし

template <class X>
inline
ModAutoPointer<X>::ModAutoPointer(const ModAutoPointer<X>& r)
	: _owner(r.isOwner()),
	  _pointer(const_cast<ModAutoPointer<X>&>(r).release())
{ }

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::~ModAutoPointer --
//		オートポインタークラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
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

template <class X>
inline
ModAutoPointer<X>::~ModAutoPointer()
{
	this->free();
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//		STL のように他のクラスのオブジェクトをさす
//		オートポインターを代入できない
//
//		以下のように定義すれば、STL のように他のクラスの
//		オブジェクトをさすオートポインターを代入できるはずである
//
//		template <class X>
//		template <class Y>
//		inline
//		ModAutoPointer<X>&
//		ModAutoPointer<X>::operator =(ModAutoPointer<Y>& r)
//		{
//			if ((void*) this != (void*) &r &&
//				(this->get() != r.get() || r.isOwner())) {
//				this->free();
//				_owner = r.isOwner();
//				_pointer = r.release();
//			}
//			return *this;
//		}
//
//		SUN C++ 4.2 Patch 104631-04、MS C++ 11.00.7022 では、
//		テンプレートクラスのメンバー関数として、明示的な
//		テンプレート関数をテンプレートクラス外で定義すると、
//		構文エラーになる
//
//		また、SUN C++ 4.2 Patch 104631-04 では、
//		テンプレートクラス内で定義しても、構文エラーになる
//
//	ARGUMENTS
//		ModAutoPointer<Y>&	r
//			代入元のオートポインター
//
//		X*					r
//			代入元の生ポインター	
//
//	RETURN
//		代入された自分自身
//
//	EXCEPTIONS

template <class X>
inline
ModAutoPointer<X>&
ModAutoPointer<X>::operator =(ModAutoPointer<X>& r)
{
	if ((void*) this != (void*) &r &&
		(this->get() != r.get() || r.isOwner())) {

		//【注意】	自分自身の生ポインターとまったく同じ物を
		//			代入元が保持していても、それが指すオブジェクトは、
		//			参照カウントを持つものかもしれないので、
		//			自分自身の生ポインターを所有していれば、破棄する

		this->free();
		_owner = r.isOwner();
		_pointer = r.release();
	}
	return *this;
}

template <class X>
inline
ModAutoPointer<X>&
ModAutoPointer<X>::operator =(X* r)
{
	//【注意】	以下のように書くと、
	//
	//			return *this = ModAutoPointer<X>(r);
	//
	//			なぜか gcc 2.95.1 では、
	//
	//			ModAutoPointer<X>::ModAutoPointer(X*)
	//			ModAutoPointer<X>::operator X*()
	//			ModAutoPointer<X>::operator =(X*)
	//
	//			の系列が呼び出され、無限ループしてしまうので、
	//			ModAutoPointer<X> 型の左辺値に
	//			一度代入して回避することにする

	ModAutoPointer<X>	tmp(r);
	return *this = tmp;
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::operator * -- * 前置演算子
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生ポインターのさすオブジェクト
//
//	EXCEPTIONS

template <class X>
inline
X&
ModAutoPointer<X>::operator *() const
{
	return *this->get();
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::operator -> -- -> 演算子
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生ポインター
//
//	EXCEPTIONS
//		なし

template <class X>
inline
X*
ModAutoPointer<X>::operator ->() const
{
	return this->get();
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::operator X* -- X* へのキャスト演算子
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生ポインター
//
//	EXCEPTIONS
//		なし

template <class X>
inline
ModAutoPointer<X>::operator X*() const
{
	return this->get();
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::get -- 保持する生ポインターを返す
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生ポインター
//
//	EXCEPTIONS
//		なし

template <class X>
inline
X*
ModAutoPointer<X>::get() const
{
	return _pointer;
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::release -- 
//		保持する生ポインターの所有権を放棄し、保持する生ポインターを返す
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		所有権の放棄された生ポインター
//
//	EXCEPTIONS
//		なし

template <class X>
inline
X*
ModAutoPointer<X>::release()
{
	_owner = ModFalse;
	return this->get();
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::isOwner -- 保持する生ポインターを所有しているか
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			所有している
//		ModFalse
//			所有していない
//
//	EXCEPTIONS
//		なし

template <class X>
inline
ModBoolean
ModAutoPointer<X>::isOwner() const
{
	return _owner;
}

//	TEMPLATE FUNCTION public
//	ModAutoPointer<X>::free --
//		保持する生ポインターを破棄する
//
//	TEMPLATE ARGUMENTS
//		class X
//			自分自身の保持する生ポインターの指すオブジェクトの型
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

template <class X>
inline
void
ModAutoPointer<X>::free()
{
	if (this->isOwner())
		delete this->release();
}

#if MOD_CONF_NO_INDIRECTIONAL_FOR_BASIC == 1

//【注意】	SUN C++ 4.2 Patch 104631-07 では、
//			char など -> を適用できないクラスのオブジェクトを
//			テンプレート引数に与えると構文エラーになる
//
//			char などの基本的な型については -> 演算子を定義しない
//			ModAutoPointer クラスの特別バージョンを定義することにする

#define	__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(T)						\
ModTemplateNull																\
class ModAutoPointer<T>														\
{																			\
public:																		\
	ModAutoPointer(T* pointer = 0)											\
		: _owner(pointer ? ModTrue : ModFalse), _pointer(pointer)			\
	{ }																		\
	ModAutoPointer(const ModAutoPointer<T>& r)								\
		: _owner(r.isOwner()),												\
		  _pointer(const_cast<ModAutoPointer<T>&>(r).release())				\
	{ }																		\
	~ModAutoPointer()														\
	{ this->free(); }														\
																			\
	ModAutoPointer<T>&		operator =(ModAutoPointer<T>& r)				\
	{																		\
		if ((void*) this != (void*) &r &&									\
			(this->get() != r.get() || r.isOwner())) {						\
			this->free();													\
			_owner = r.isOwner();											\
			_pointer = r.release();											\
		}																	\
		return *this;														\
	}																		\
	ModAutoPointer<T>&		operator =(T* r)								\
	{ return *this = ModAutoPointer<T>(r); }								\
																			\
	T&						operator *() const								\
	{ return *this->get(); }												\
																			\
	T*						get() const										\
	{ return _pointer; }													\
	T*						release()										\
	{																		\
		_owner = ModFalse;													\
		return this->get();													\
	}																		\
																			\
	ModBoolean				isOwner() const									\
	{ return _owner; }														\
																			\
	virtual void			free()											\
	{ if (this->isOwner()) delete this->release(); }						\
private:																	\
	ModBoolean				_owner;											\
	T*						_pointer;										\
};

__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(ModBoolean)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(char)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(unsigned char)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(short)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(unsigned short)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(int)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(unsigned int)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(ModInt32)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(ModUInt32)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(ModInt64)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(ModUInt64)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(float)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(double)
__ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE(long double)

#undef __ModAutoPointer_FOR_NOT_INDIRECTIONAL_TYPE
#endif

#endif	// __ModAutoPointer_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
