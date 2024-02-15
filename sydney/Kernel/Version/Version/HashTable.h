// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HashTable.h --	バージョンファイルやページを管理するための
//					ハッシュ表関連のテンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_VERSION_HASHTABLE_H
#define	__SYDNEY_VERSION_HASHTABLE_H

#include "Version/Module.h"
#include "Version/List.h"

#include "Common/HashTable.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	TEMPLATE CLASS
//	Version::HashTable --
//		バージョンファイルやページを管理するための
//		ハッシュ表を表すテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES

template <class T>
class HashTable
	: public	Common::HashTable<List<T>, T>
{
public:
	// コンストラクター
	HashTable(unsigned int size, T* T::* prev, T* T::* next);
};

//	TEMPLATE FUNCTION public
//	Version::HashTable<T>::HashTable -- 
//		バージョンファイルやページを管理するための
//		ハッシュ表を表すテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class T
//			ハッシュ表に格納する要素の型
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		size
//			バケット数
//		T* T::*				prev
//			要素を表す型の直前の要素へのポインタを格納するメンバへのポインタ
//		T* T::*				next
//			要素を表す型の直後の要素へのポインタを格納するメンバへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class T>
inline
HashTable<T>::HashTable(unsigned int size, T* T::* prev, T* T::* next)
	: Common::HashTable<List<T>, T>(size, prev, next)
{}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_HASHTABLE_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

