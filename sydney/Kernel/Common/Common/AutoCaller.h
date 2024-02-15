// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoCaller.h --	オート関数呼び出しクラス
// 
// Copyright (c) 2001, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_AUTOCALLER_H
#define	__TRMEISTER_COMMON_AUTOCALLER_H

#include "Common/Module.h"

#include "ModAutoPointer.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	TEMPLATE CLASS
//	Common::AutoCaller0<T> --
//		デストラクト時に必要があればメンバー関数を呼び出すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//
//	NOTES

template <class T>
class AutoCaller0
	: public	ModAutoPointer<T>
{
public:
	//	TYPEDEF
	//	Common::AutoCaller0<T>::FunctionPointer --
	//		デストラクト時に呼び出すメンバー関数へのポインタ
	//
	//	NOTES

	typedef	void (T::* FunctionPointer)();

	// コンストラクター
	AutoCaller0(T* object, FunctionPointer func);
	AutoCaller0(const AutoCaller0<T>& cOther_);
	// デストラクター
	~AutoCaller0();

	// 必要があればメンバー関数を呼び出す
	virtual void			free();

	// assignment
	AutoCaller0<T>& operator=(AutoCaller0<T>& cOther_);

private:
	// オブジェクトの破棄時に必要があれば呼び出すメンバー関数
	FunctionPointer			_func;
};

//	TEMPLATE CLASS
//	Common::AutoCaller1<T, A> --
//		デストラクト時に必要があればメンバー関数を呼び出すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A
//			呼び出すメンバー関数に渡す値の型
//
//	NOTES

template <class T, class A>
class AutoCaller1
	: public	ModAutoPointer<T>
{
public:
	//	TYPEDEF
	//	Common::AutoCaller1<T, A>::FunctionPointer --
	//		デストラクト時に呼び出すメンバー関数へのポインタ
	//
	//	NOTES

	typedef	void (T::* FunctionPointer)(A);

	// コンストラクター
	AutoCaller1(T* object, FunctionPointer func, A argument);
	AutoCaller1(const AutoCaller1<T,A>& cOther_);
	// デストラクター
	~AutoCaller1();

	// 必要があればメンバー関数を呼び出す
	virtual void			free();

	// assignment
	AutoCaller1<T,A>& operator=(AutoCaller1<T,A>& cOther_);

private:
	// オブジェクトの破棄時に必要があれば呼び出すメンバー関数
	FunctionPointer			_func;
	// 呼び出すメンバー関数に渡す引数
	A						_argument;
};


//	TEMPLATE CLASS
//	Common::AutoCaller2<T, A0, A1> --
//		デストラクト時に必要があればメンバー関数を呼び出すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A0
//			呼び出すメンバー関数に渡す値の型
//		class A1
//			呼び出すメンバー関数に渡す値の型
//
//	NOTES

template <class T, class A0, class A1>
class AutoCaller2
	: public	ModAutoPointer<T>
{
public:
	//	TYPEDEF
	//	Common::AutoCaller2<T, A0, A1>::FunctionPointer --
	//		デストラクト時に呼び出すメンバー関数へのポインタ
	//
	//	NOTES

	typedef	void (T::* FunctionPointer)(A0, A1);

	// コンストラクター
	AutoCaller2(T* object, FunctionPointer func, A0 argument0, A1 argument1);
	AutoCaller2(const AutoCaller2<T,A0,A1>& cOther_);
	// デストラクター
	~AutoCaller2();

	// 必要があればメンバー関数を呼び出す
	virtual void			free();

	// assignment
	AutoCaller2<T,A0,A1>& operator=(AutoCaller2<T,A0,A1>& cOther_);

private:
	// オブジェクトの破棄時に必要があれば呼び出すメンバー関数
	FunctionPointer			_func;
	// 呼び出すメンバー関数に渡す第 1 引数
	A0						_argument0;
	// 呼び出すメンバー関数に渡す第 2 引数
	A1						_argument1;
};

//	TEMPLATE FUNCTION
//	Common::AutoCaller0<T>::AutoCaller0 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//
//	NOTES
//
//	ARGUMENTS
//		T*					object
//			メンバー関数を呼び出すオブジェクトへのポインタ
//		Common::AutoCaller0<T>::FunctionPointer	func
//			呼び出すメンバー関数へのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
AutoCaller0<T>::AutoCaller0(T* object, FunctionPointer func)
	: ModAutoPointer<T>(object),
	  _func(func)
{}

// TEMPLATE FUNCTION public
//	Common::AutoCaller0<T>::AutoCaller0 -- 
//
// TEMPLATE ARGUMENTS
//	class T
//	
// NOTES
//
// ARGUMENTS
//	const AutoCaller0<T>& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class T>
inline
AutoCaller0<T>::
AutoCaller0(const AutoCaller0<T>& cOther_)
	: ModAutoPointer<T>(cOther_),
	  _func(cOther_._func)
{}

//	TEMPLATE FUNCTION
//	Common::AutoCaller0<T>::~AutoCaller0 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
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
AutoCaller0<T>::~AutoCaller0()
{
	free();
}

//	TEMPLATE FUNCTION
//	Common::AutoCaller0<T>::free -- 必要があればメンバー関数を呼び出す
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
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
void
AutoCaller0<T>::free()
{
	if (isOwner())
		if (T* p = release())
			(p->*_func)();
}

// TEMPLATE FUNCTION public
//	Common::AutoCaller0<T>::operator= -- assignment
//
// TEMPLATE ARGUMENTS
//	class T
//	
// NOTES
//
// ARGUMENTS
//	AutoCaller0<T>& cOther_
//	
// RETURN
//	AutoCaller0<T>&
//
// EXCEPTIONS

template <class T>
inline
AutoCaller0<T>&
AutoCaller0<T>::
operator=(AutoCaller0<T>& cOther_)
{
	static_cast<ModAutoPointer<T>&>(*this) = cOther_;
	_func = cOther_._func;	
	return *this;
}

//	TEMPLATE FUNCTION
//	Common::AutoCaller1<T, A>::AutoCaller1 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A
//			呼び出すメンバー関数に渡す引数の型
//
//	NOTES
//
//	ARGUMENTS
//		T*					object
//			メンバー関数を呼び出すオブジェクトへのポインタ
//		Common::AutoCaller1<T, A>::FunctionPointer	func
//			呼び出すメンバー関数へのポインタ
//		A					argument
//			呼び出すメンバー関数へ渡す引数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class A>
inline
AutoCaller1<T, A>::AutoCaller1(T* object, FunctionPointer func, A argument)
	: ModAutoPointer<T>(object),
	  _func(func),
	  _argument(argument)
{}

// TEMPLATE FUNCTION public
//	Common::AutoCaller1<T, A>::AutoCaller1 -- 
//
// TEMPLATE ARGUMENTS
//	class T
//	class A
//	
// NOTES
//
// ARGUMENTS
//	const AutoCaller1<T, A>& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class T, class A>
inline
AutoCaller1<T, A>::
AutoCaller1(const AutoCaller1<T, A>& cOther_)
	: ModAutoPointer<T>(cOther_),
	  _func(cOther_._func),
	  _argument(cOther_._argument)
{}

//	TEMPLATE FUNCTION
//	Common::AutoCaller1<T, A>::~AutoCaller1 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A
//			呼び出すメンバー関数に渡す引数の型
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

template <class T, class A>
inline
AutoCaller1<T, A>::~AutoCaller1()
{
	free();
}

//	TEMPLATE FUNCTION
//	Common::AutoCaller1<T, A>::free -- 必要があればメンバー関数を呼び出す
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A
//			呼び出すメンバー関数に渡す引数の型
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

template <class T, class A>
inline
void
AutoCaller1<T, A>::free()
{
	if (isOwner())
		if (T* p = release())
			(p->*_func)(_argument);
}

// TEMPLATE FUNCTION public
//	Common::AutoCaller1<T, A>::operator= -- 
//
// TEMPLATE ARGUMENTS
//	class T
//	class A
//	
// NOTES
//
// ARGUMENTS
//	AutoCaller1<T, A>& cOther_
//	
// RETURN
//	AutoCaller1<T, A>&
//
// EXCEPTIONS

template <class T, class A>
inline
AutoCaller1<T, A>&
AutoCaller1<T, A>::
operator=(AutoCaller1<T, A>& cOther_)
{
	static_cast<ModAutoPointer<T>&>(*this) = cOther_;
	_func = cOther_._func;
	_argument = cOther_._argument;
	return *this;
}

//	TEMPLATE FUNCTION
//	Common::AutoCaller2<T, A0, A1>::AutoCaller2 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		class A0
//			呼び出すメンバー関数に渡す第 1 引数の型
//		class A1
//			呼び出すメンバー関数に渡す第 2 引数の型
//
//	NOTES
//
//	ARGUMENTS
//		T*					object
//			メンバー関数を呼び出すオブジェクトへのポインタ
//		Common::AutoCaller2<T, A0, A1>::FunctionPointer	func
//			呼び出すメンバー関数へのポインタ
//		A0					argument0
//			呼び出すメンバー関数へ渡す第 1 引数
//		A1					argument1
//			呼び出すメンバー関数へ渡す第 1 引数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T, class A0, class A1>
inline
AutoCaller2<T, A0, A1>::AutoCaller2(T* object, FunctionPointer func,
									A0 argument0, A1 argument1)
	: ModAutoPointer<T>(object),
	  _func(func),
	  _argument0(argument0),
	  _argument1(argument1)
{}

// TEMPLATE FUNCTION public
//	Common::AutoCaller2<T, A0, A1>::AutoCaller2 -- 
//
// TEMPLATE ARGUMENTS
//	class T
//	class A0
//	class A1
//	
// NOTES
//
// ARGUMENTS
//	const AutoCaller2<T, A0, A1>& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class T, class A0, class A1>
inline
AutoCaller2<T, A0, A1>::
AutoCaller2(const AutoCaller2<T, A0, A1>& cOther_)
	: ModAutoPointer<T>(cOther_),
	  _func(cOther_._func),
	  _argument0(cOther_._argument0),
	  _argument1(cOther_._argument1)
{}

//	TEMPLATE FUNCTION
//	Common::AutoCaller2<T, A0, A1>::~AutoCaller2 --
//		デストラクト時に必要があればメンバー関数を呼び出す
//		テンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		A0					argument0
//			呼び出すメンバー関数へ渡す第 1 引数の型
//		A1					argument1
//			呼び出すメンバー関数へ渡す第 2 引数の型
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

template <class T, class A0, class A1>
inline
AutoCaller2<T, A0, A1>::~AutoCaller2()
{
	free();
}

//	TEMPLATE FUNCTION
//	Common::AutoCaller2<T, A0, A1>::free -- 必要があればメンバー関数を呼び出す
//
//	TEMPLATE ARGUMENTS
//		class T
//			呼び出すメンバー関数を持つクラス
//		A0					argument0
//			呼び出すメンバー関数へ渡す第 1 引数の型
//		A1					argument1
//			呼び出すメンバー関数へ渡す第 2 引数の型
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

template <class T, class A0, class A1>
inline
void
AutoCaller2<T, A0, A1>::free()
{
	if (isOwner())
		if (T* p = release())
			(p->*_func)(_argument0, _argument1);
}

// TEMPLATE FUNCTION public
//	Common::AutoCaller2<T, A0, A1>::operator= -- 
//
// TEMPLATE ARGUMENTS
//	class T
//	class A0
//	class A1
//	
// NOTES
//
// ARGUMENTS
//	AutoCaller2<T, A0, A1>& cOther_
//	
// RETURN
//	AutoCaller2<T, A0, A1>&
//
// EXCEPTIONS

template <class T, class A0, class A1>
inline
AutoCaller2<T, A0, A1>&
AutoCaller2<T, A0, A1>::
operator=(AutoCaller2<T, A0, A1>& cOther_)
{
	static_cast<ModAutoPointer<T>&>(*this) = cOther_;
	_func = cOther_._func;
	_argument0 = cOther_._argument0;
	_argument1 = cOther_._argument1;
	return *this;
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_AUTOCALLER_H

//
// Copyright (c) 2001, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
