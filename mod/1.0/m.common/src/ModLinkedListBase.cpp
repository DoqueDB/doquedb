// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLinkedList.cpp -- リスト関連のメンバ定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#include "ModCommon.h"
#include "ModLinkedListBase.h"

// ** 以下、singly linked list 関連の定義

//
// FUNCTION 
// ModSinglyLinkedListBase::operator new -- メモリを確保する
//
// NOTES
//	グローバルな::newでメモリを確保し、返す。もしメモリが確保できなければ、
//	エラー番号ModErrorSystemMemoryExhaustを設定しModExceptionを送出する。
//
// ARGUMENTS
//	size_t size
//		要求サイズ
// RETURN
//	確保できたメモリの先頭アドレス
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない(::newに失敗)
//
void*
ModSinglyLinkedListBase::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//
// FUNCTION public
// ModSinglyLinkedListBase::insert -- 単方向リストへ要素の登録(基底クラス)
//
// NOTES
// 単方向リストへの要素の登録をする。要素はリストの先頭につなぐ。
// リストに格納される要素はポインタのみである。本リストに登録する要素は
// すべて要素クラスModSinglyinkedEntryの派生クラスである必要がある。
//
// ARGUMENTS
// ModSinglyLinkedEntry* entry
//	登録する要素オブジェクトへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
void
ModSinglyLinkedListBase::insert(ModSinglyLinkedEntry* entry)
{
	entry->next = this->head;
	this->head = entry;
	this->size ++;
}

//
// FUNCTION public
// ModSinglyLinkedListBase::expunge -- 単方向リストへ要素の抹消(基底クラス)
//
// NOTES
// 要素のポインタを指定して、単方向リストに登録されている要素を抹消する。
//
// ARGUMENTS
// ModSinglyLinkedEntry* entry
//	対象となる要素オブジェクトへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModCommonErrorEntryNotFound
//		指定されたエントリがみつからない
//
void
ModSinglyLinkedListBase::expunge(ModSinglyLinkedEntry* entry)
{
	if (head == entry) {
		head = entry->next;
	} else {
		ModSinglyLinkedEntry* prev;
		for (prev = this->head; prev != 0; prev = prev->next) {
			if (prev->next == entry) {
				prev->next = entry->next;
				break;
			}
		}
		if (prev == 0) {
			// 存在しないエントリを抹消しようとした。
			ModThrow(ModModuleStandard, ModCommonErrorEntryNotFound,
					 ModErrorLevelError);
		}
	}
	entry->next = 0;
	this->size --;
}

//
// FUNCTION
// ModSinglyLinkedListBase::isRegistered -- 単方向リストへ登録の有無の確認(基底クラス)
//
// NOTES
// 要素のポインタを指定して、単方向リストに登録されているかどうか確認する。
// 登録されているかどうかは、オブジェクトの内容ではなくポインタ値の比較で
// 判断される。
//
// ARGUMENTS
// ModSinglyLinkedEntry* entry
//	対象となる要素オブジェクトへのポインタ
//
// RETURN
// 登録されていればModTrue、されていなければModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModSinglyLinkedListBase::isRegistered(ModSinglyLinkedEntry* entry) const
{
	for (ModSinglyLinkedEntry* item = this->head; item != 0; 
		 item = item->next) {
		if (item == entry) {
			return ModTrue;
		}
	}
	return ModFalse;
}

//
// FUNCTION 
// ModSinglyLinkedListBase::operator new -- メモリを確保する
//
// NOTES
//	グローバルな::newでメモリを確保し、返す。もしメモリが確保できなければ、
//	エラー番号ModErrorSystemMemoryExhaustを設定しModExceptionを送出する。
//
// ARGUMENTS
//	size_t size
//		要求サイズ
// RETURN
//	確保できたメモリの先頭アドレス
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない(::newに失敗)
//
void*
ModSinglyLinkedListIteratorBase::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}
//
// FUNCTION
// ModSinglyLinkedListIteratorBase::ModsinglyLinkedListIteratorBase -- 単方向リスト反復子のコンストラクタ(基底クラス)
//
// NOTES
// 単方向リストの反復子の基底クラスのコンストラクタ。
//
// ARGUMENTS
// ModSinglyLinkedList& list
//	反復子の対象となる単方向リスト
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
ModSinglyLinkedListIteratorBase::ModSinglyLinkedListIteratorBase(
	ModSinglyLinkedListBase& list)
{
	this->currentList = &list;
	this->currentEntry = list.head;
}

//
// FUNCTION
// ModSinglyLinkedListIteratorBase::operator*() -- 指している要素を返す(基底クラス)
//
// NOTES
// 反復子が指している要素を返す。リストの最後の場合は0を返す。
//
// ARGUMENTS
// なし
//
// RETURN
//	反復子が指す要素オブジェクトへのポインタ
//
// EXCEPTIONS
// なし
//
ModSinglyLinkedEntry*
ModSinglyLinkedListIteratorBase::operator*()
{
	return this->currentEntry;
}

//
// ModSinglyLinkedListIterator<Entry>::operator++() -- 反復子を進める前置演算子(基底クラス)
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
//	ModCommonErrorOutOfRange
//		範囲を超えてアクセスしようとした
//
ModSinglyLinkedListIteratorBase&
ModSinglyLinkedListIteratorBase::operator++()
{
	ModSinglyLinkedEntry* entry = this->currentEntry;
	if (entry == 0) {
		// 既に最後である
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange, 
				 ModErrorLevelError);
	}
	this->currentEntry = entry->next;
	return *this;
}

// ** 以下、doubly linked list 関連の定義(整備していない。要望次第)
// 
#if 0	// not implemented
// doubly linked list への登録
void
ModDoublyLinkedListBase::insert(ModDoublyLinkedEntry* entry)
{
	entry->next = head.next;
	entry->prev = &head;
	head.next->prev = entry;
	head.next = entry;
}

// doubly linked list からの抹消
void
ModDoublyLinkedListBase::expunge(ModDoublyLinkedEntry* entry)
{
	// リストに登録されていることを事前に確認すれば、なお安全。
	// ただし、オーバヘッドも大きいので、やるならデバッグ時のみと
	// そうするべき。
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
	entry->next = 0;
	entry->prev = 0;
}

// doubly linked list に登録済かテストする
ModBoolean
ModDoublyLinkedListBase::isRegistered(ModDoublyLinkedEntry* entry) const
{
	for (ModDoublyLinkedEntry* reg = this->head.next; reg != &this->head; reg = reg->next) {
		if (reg == entry) {
			return ModTrue;
		}
	}
	return ModFalse;
}

// doubly linked list の反復子のコンストラクタ
ModDoublyLinkedListIteratorBase::ModDoublyLinkedListIteratorBase(ModDoublyLinkedListBase& list)
{
	this->currentList = &list;
	this->currentEntry = &list.head;
}

// doubly linked list の要素を返す。
ModDoublyLinkedEntry*
ModDoublyLinkedListIteratorBase::operator()()
{
	this->currentEntry = this->currentEntry->next;
	ModDoublyLinkedEntry* entry = this->currentEntry;
	if (entry == &this->currentList->head) {
		entry = 0;
	}
	return entry;
}
#endif	// not implemented
// 以上、doubly linked list 関連の定義

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
