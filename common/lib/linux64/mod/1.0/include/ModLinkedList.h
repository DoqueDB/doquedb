// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLinkedList.h -- ModLinkedList のクラス定義
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

#ifndef	__ModLinkedList_H__
#define __ModLinkedList_H__

#include "ModLinkedListBase.h"

//
// モジュールは標準ライブラリー(ModModuleStandard)に属する。
// したがって、エラーはModStandardXXXを利用する。
// ModStandardObjectのサブクラスとして作成しないことについては以下の注意
// を参照のこと。
//
// [注意]
//	メモリ管理実現のため、クラスModSinglyLinkedListが利用される関係で、
//	以下をインクルードできない。これらはメモリ管理よりも下位のクラスと
//	みなし、特にModDefaultObjectのサブクラスとはしない。
//	リストを利用する側で責任をもってnew, deleteを継承するために
//	ModXXXObjectのサブクラスとすることで、回避できるはずである。
//	ただし、ModSinglyLinkedList自体をどのクラスにも属さない形で
//	作成した場合には普通のnewが呼ばれてしまう。(対策は今のところなし)
//
// #include "ModDefaultManager.h"
//

// singly linked list の実装について
//  これは侵入的リスト(参照)である。
//	リストクラスは、MFC, STL 等で提供されるが、メモリ確保の効率化と
//	一元的な排他機構 (mutex) のサポートを目標に、あえて以下の実装を
//	行った。
//
//	ここでの実装では、リストへの登録 (insert), 抹消 (expunge) 
//  において、メモリの確保及び解放は発生しない。これは、リストに繋がれる
//	エントリのコンストラクタ、デストラクタで、insert, expunge の
//	各関数が呼び出されることを想定している。
//
//	つまり、エントリの生成と破棄に応じて、安全に、リストへの登録、抹消を
//	行うことができる（はず）。
//

//
// TEMPLATE CLASS
// ModSinglyLinkedList -- リストのテンプレート
//
// TEMPLATE ARGUMENTS
// Entry entry
//	登録するオブジェクトの型。
//
// NOTES
// ModSinglyLinkedListBaseの派生クラスのテンプレート。
// 実際のリストはModSinglyLinkedList<登録するオブジェクトの型>である。
// リストには、オブジェクトへのポインタがそのまま格納され、複製は
// 作成されない。
// 
template <class Entry>
class ModSinglyLinkedList : public ModSinglyLinkedListBase {
public:
	// リストへの登録
	void insert(Entry* entry);
	// リストからの抹消
	void expunge(Entry* entry);
	// 登録されているかどうかをチェックする
	ModBoolean isRegistered(Entry*) const;

};

//
// TEMPLATE FUNCTION
// ModSinglyLinkedList<Entry>::insert -- リストへの要素の登録
//
// TEMPLATE ARGUMENTS
//	class Entry
//		登録するオブジェクトの型
//
// NOTES
//	要素をリストの先頭に挿入する。
//
// ARGUMENTS
//	Entry* entry
//		登録する要素へのポインタ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
template <class Entry>
inline void
ModSinglyLinkedList <Entry>::insert(Entry* entry)
{
	ModSinglyLinkedListBase::insert((ModSinglyLinkedEntry*)entry);
}

//
// TEMPLATE FUNCTION
// ModSinglyLinkedListBase<Entry>::expunge -- リストからの要素の抹消
//
// TEMPLATE ARGUMENTS
//	class Entry
//		登録されているオブジェクトの型
//
// NOTES
//	要素をリストから抹消する。
//
// ARGUMENTS
//	Entry* entry
//		抹消する要素へのポインタ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModSinglyLinkedListBase::expungeの例外参照
//
template <class Entry>
inline void
ModSinglyLinkedList <Entry>::expunge(Entry* entry)
{
	ModSinglyLinkedListBase::expunge((ModSinglyLinkedEntry*)entry);
}

//
// TEMPLATE FUNCTION
// ModSinglyLinkedListBase<Entry>::isRegistered -- 登録の有無の確認
//
// TEMPLATE ARGUMENTS
//	class Entry
//		登録されているオブジェクトの型
//
// NOTES
//	指定した要素が登録されているかどうかを返す。
//
// ARGUMENTS
//	Entry* entry
//		確認する要素へのポインタ
//
// RETURN
//	登録されていればModTrue、存在しなければModFalseを返す。
//
// EXCEPTIONS
//	なし
//
template <class Entry>
inline ModBoolean
ModSinglyLinkedList <Entry>::isRegistered(Entry* entry) const
{
	return ModSinglyLinkedListBase::isRegistered((ModSinglyLinkedEntry*)entry);
}


// ** singly linked list の反復子の派生クラスのテンプレート

//
// TEMPLATE CLASS
// ModSinglyLinkedListIterator -- 単方向リストModSinlyLinkedListの反復子テンプレート
//
// TEMPLATE ARGUMENTS
// Entry entry
//	リストに登録されるオブジェクトの型。
//
// NOTES
// ModSinglyLinkedListIteratorBaseの派生クラスのテンプレート。
// 実際の反復子はModSinglyLinkedListIterator<登録するオブジェクトの型>である。
// 
template <class Entry>
class ModSinglyLinkedListIterator : public ModSinglyLinkedListIteratorBase {
public:
	// コンストラクタ
	ModSinglyLinkedListIterator(ModSinglyLinkedList<Entry>& list);
	// 要素を返す
	ModSinglyLinkedListIterator<Entry>& operator++();
	Entry* operator*();
};

//
// TEMPLATE FUNCTION
// ModSinglyLinkedListIterator<Entry>::ModSinglyLinkedListIterator -- コンストラクタ
//
// TEMPLATE ARGUMENTS
//	class Entry
//		リストに登録されている要素の型
//
// NOTES
// 単方向リスト反復子のコンストラクタである。引数には、対象となる単方向リストを
// 指定する。
//
// ARGUMENTS
//	ModSinglyLinkedList<Entry&> list
//		反復子の対象となる単方向リスト
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
template <class Entry>
inline
ModSinglyLinkedListIterator<Entry>::ModSinglyLinkedListIterator(ModSinglyLinkedList<Entry>& list)
	: ModSinglyLinkedListIteratorBase(list)
{
}


//
// TEMPLATE FUNCTION
// ModSinglyLinkedListIterator<Entry>::operator++() -- 反復子を進める前置演算子
// 
// TEMPLATE ARGUMENTS
//	class Entry
//		リストに登録されている要素の型
//
// NOTES
//	1つだけ反復子を進める。自分が次の要素を指すように変更して、自分への
//	参照を返す。イテレータが指しているエントリが削除されてしまった場合の
//	動作はチェックされず、不定となるので注意すること。
//
// ARGUMENTS
//	なし
//
// RETURN
//	進んだ後の位置を指す自分自身への参照
//
// EXCEPTIONS
//	その他
//		ModCommonErrorOutOfRange(ModSinglyLinkedListIteratorBase::operator++の例外参照)
//
template <class Entry>
ModSinglyLinkedListIterator<Entry>&
ModSinglyLinkedListIterator<Entry>::operator++()
{
	return (ModSinglyLinkedListIterator<Entry>&)
			ModSinglyLinkedListIteratorBase::operator++();
}


//
// TEMPLATE FUNCTION
// ModSinglyLinkedListIterator<Entry>::operator*() -- 指している要素を返す
// 
// TEMPLATE ARGUMENTS
//	class Entry
//		リストに登録されている要素の型
//
// NOTES
// 反復子が指している要素を返す。リストの最後の場合は0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	反復子が指す要素オブジェクトへのポインタ
//
// EXCEPTIONS
//	なし
//
template <class Entry>
Entry*
ModSinglyLinkedListIterator<Entry>::operator*()
{
	return (Entry*)ModSinglyLinkedListIteratorBase::operator*(); 	
}

// ** 以下は整備していない。要望に応じて整備をすすめる

#if 0 // not implemented

// doubly linked list の派生クラスのテンプレート
template <class Entry>
class ModDoublyLinkedList : public ModDoublyLinkedListBase {
public:
	// リストへの登録
	void insert(Entry* entry) { 
		ModDoublyLinkedListBase::insert((ModDoublyLinkedEntry*)entry); 
	};
	// リストからの抹消
	void expunge(Entry* entry) { 
		ModDoublyLinkedListBase::expunge((ModDoublyLinkedEntry*)entry); 
	};
};

// doubly linked list の反復子の派生クラスのテンプレート
template <class Entry>
class ModDoublyLinkedListIterator : public ModDoublyLinkedListIteratorBase {
public:
	// コンストラクタ
	ModDoublyLinkedListIterator(ModDoublyLinkedList<Entry>& list)
		: ModDoublyLinkedListIteratorBase(list) {};
	// 要素を返す
	Entry* operator()() {
		return (Entry*)ModDoublyLinkedListIteratorBase::operator()(); 
	};
};
#endif	// not implemented 

#endif // __ModLinkedList_H__

//
// Copyright (c) 1997, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
