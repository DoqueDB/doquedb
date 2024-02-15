// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoArrayPointer.h -- オート配列ポインタクラス
// 
// Copyright (c) 2001, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_AUTOARRAYPOINTER_H
#define	__TRMEISTER_COMMON_AUTOARRAYPOINTER_H

#include "Common/Module.h"

#include "ModAutoPointer.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	TEMPLATE CLASS
//	Common::AutoArrayPointer -- オート配列ポインタを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
//
//	NOTES

template <class T>
class AutoArrayPointer
	: public	ModAutoPointer<T>
{
public:
	// コンストラクター
	AutoArrayPointer(T* p = 0);
	// デストラクター
	~AutoArrayPointer();

	// = 演算子
	AutoArrayPointer<T>&	operator =(T* p);
	// [] 演算子
	T&						operator [](unsigned int i);
	const T&				operator [](unsigned int i) const;

	// 領域を破棄する
	void					free();
};

//	TEMPLATE FUNCTION public
//	Common::AutoArrayPointer<T>::AutoArrayPointer --
//		オート配列ポインタを表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T*					p
//			指定されたとき
//				生成するオート配列ポインタに保持させる
//				配列を格納する領域の先頭アドレス
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
AutoArrayPointer<T>::AutoArrayPointer(T* p)
	: ModAutoPointer<T>(p)
{}

//	TEMPLATE FUNCTION public
//	Common::AutoArrayPointer<T>::~AutoArrayPointer --
//		オート配列ポインタを表すテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
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
AutoArrayPointer<T>::~AutoArrayPointer()
{
	free();
}

//	TEMPLATE FUNCTION public
//	Common::AutoArrayPointer<T>::operator = -- = 演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T*					p
//			代入する配列を格納する領域の先頭アドレス
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

template <class T>
inline
AutoArrayPointer<T>&
AutoArrayPointer<T>::operator =(T* p)
{
	ModAutoPointer<T>::operator =(ModAutoPointer<T>(p));
	return *this;
}

//	TEMPLATE FUNCTION public
//	Common::AutoArrayPointer<T>::operator [] -- [] 演算子
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
//
//	NOTES
//		保持する配列を格納する領域の先頭アドレスが 0 のときに、
//		この関数を呼び出したときの動作は、不定である
//
//	ARGUMENTS
//		unsigned int		i
//			参照する要素の位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class T>
inline
T&
AutoArrayPointer<T>::operator [](unsigned int i)
{
	return (get())[i];
}

template <class T>
inline
const T&
AutoArrayPointer<T>::operator [](unsigned int i) const
{
	return (get())[i];
}

//	TEMPLATE FUNCTION public
//	Common::AutoArrayPointer<T>::free -- 保持する領域を破棄する
//
//	TEMPLATE ARGUMENTS
//		class T
//			保持する配列の要素の型
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
AutoArrayPointer<T>::free()
{
	if (isOwner())
		delete [] release();
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_AUTOARRAYPOINTER_H

//
// Copyright (c) 2001, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
