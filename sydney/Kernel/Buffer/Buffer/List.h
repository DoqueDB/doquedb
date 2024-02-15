// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// List.h --	バッファファイルやページを管理するための
//				侵入型リスト関連のテンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_LIST_H
#define	__SYDNEY_BUFFER_LIST_H

#include "Buffer/Module.h"

#include "Common/DoubleLinkedList.h"
#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	TEMPLATE CLASS
//	Buffer::List --
//		バッファファイルやページを管理するための
//		侵入型双方向リストを表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES

template <class T>
class List
	: public	Common::DoubleLinkedList<T>
{
public:
	// デフォルトコンストラクター
	List();
	// コンストラクター
	List(T* T::* prev, T* T::* next);

	// 排他制御用のラッチを得る
	Os::CriticalSection&	getLatch() const;

	// 排他制御用のラッチへのポインタ
	mutable Os::CriticalSection*	_latch;
};

//	TEMPLATE FUNCTION public
//	Buffer::List<T>::List --
//		バッファファイルやページを管理するための
//		侵入型双方向リストを表すテンプレートクラスのデフォルトコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//		生成されたリストをそのまま使用したときの動作は保証されない
//		が、リストの配列を生成するために必要である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
List<T>::List()
	: _latch(0)
{}	

//	TEMPLATE FUNCTION public
//	Buffer::List<T>::List --
//		バッファファイルやページを管理するための
//		侵入型双方向リストを表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		T* T::*				prev
//			要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
//		T* T::*				next
//			要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class T>
inline
List<T>::List(T* T::* prev, T* T::* next)
	: Common::DoubleLinkedList<T>(prev, next), _latch(0)
{}

//	TEMPLATE FUNCTION public
//	Buffer::List::getLatch --
//		バッファファイルやページを管理するための
//		侵入型リストの操作の排他制御をするためのラッチを得る
//
//	TEMPLATE ARGUMENTS
//		class T
//			リストに格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ラッチへのリファレンス
//
//	EXCEPTIONS
//		なし

template <class T>
inline
Os::CriticalSection&
List<T>::getLatch() const
{
	return *_latch;
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_LIST_H

//
// Copyright (c) 2000, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
