// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModRingLinkedList.cpp -- リングバッファ関連のメンバ定義
// 
// Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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
#include "ModRingLinkedList.h"

// ** 以下、ring linked list 関連の定義

//
// FUNCTION 
// ModRingLinkedList::operator new -- メモリを確保する
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
ModRingLinkedListBase::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}
//
// FUNCTION public
// ModRingLinkedListBase::insert -- リングバッファへの要素の登録(基底クラス)
//
// NOTES
// リングバッファへの要素の登録をする。要素はリングバッファの先頭につなぐ。
// リングバッファに格納される要素はポインタのみである。本リストに登録する要素は
// すべて要素型ModRinginkedEntryの派生クラスである必要がある。
//
// ARGUMENTS
// ModRingLinkedEntry* entry
//	登録する要素オブジェクトへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
void
ModRingLinkedListBase::insert(ModRingLinkedEntry* entry)
{
	// 先頭に挿入される
	if (this->head == 0) {
		; ModAssert(this->last == 0);
		this->last = entry;
	} else {
		entry->next = this->head;
	}
	this->head = entry;
	this->last->next = entry;
	this->size ++;
}

//
// FUNCTION public
// ModRingLinkedListBase::expunge -- リングバッファ要素の抹消(基底クラス)
//
// NOTES
// 要素のポインタを指定して、リングバッファに登録されている要素を抹消する。
//
// ARGUMENTS
// ModRingLinkedEntry* entry
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
ModRingLinkedListBase::expunge(ModRingLinkedEntry* entry)
{
	if (this->head == 0) {
		// 存在しないエントリを抹消しようとした。
		ModThrow(ModModuleMemory, ModCommonErrorEntryNotFound,
				 ModErrorLevelError);
	}
	if (this->head == entry) {
		if (this->head->next == this->head) {
			// ターゲットが最初かつ最後、つまりエントリが一つだけのとき
			// すべてなくなる。
			; ModAssert(this->last == this->head);
			this->head = this->last = 0;
		} else {
			// 2エントリ以上で、先頭を消すとき
			// lastは不変。
			last->next = this->head->next;
			head = entry->next;
		}
	} else {
		ModSinglyLinkedEntry* prev;
		prev = this->head;
		do {
			if (prev->next == entry) {
				prev->next = entry->next;
				break;
			}
			prev = prev->next;
		} while (prev != this->head);
		// 最後を消すときは最後を一つ前に更新
		if (this->last == entry) {
			this->last = prev;
		}
		if (prev == 0) {
			// 存在しないエントリを抹消しようとした。
			ModThrow(ModModuleMemory, ModCommonErrorEntryNotFound,
					 ModErrorLevelError);
		}
	}
	entry->next = 0;
	this->size --;
}

//
// FUNCTION
// ModRingLinkedListBase::isRegistered -- リングバッファへの登録の有無の確認(基底クラス)
//
// NOTES
// 要素のポインタを指定して、リングバッファに登録されているかどうか確認する。
// 登録されているかどうかは、オブジェクトの内容ではなくポインタ値の比較で
// 判断される。
//
// ARGUMENTS
// ModRingLinkedEntry* entry
//	対象となる要素オブジェクトへのポインタ
//
// RETURN
// 登録されていればModTrue、されていなければModFalse
//
// EXCEPTIONS
// なし
//
ModBoolean
ModRingLinkedListBase::isRegistered(ModRingLinkedEntry* entry) const
{
	ModRingLinkedEntry* item = this->head;
	if (item == 0) {
		return ModFalse;
	}
	do {
		if (item == entry) {
			return ModTrue;
		}
		item = item->next;
	} while(item != this->head); 

	return ModFalse;
}

//
// 	以下は使用していないプロトタイプ。よってコメントは抜く
//
#if 0
void*
ModRingLinkedListIteratorBase::operator new(size_t size)
{
	return ModErrorHandle::newSetHandler((ModSize)size);
}

ModRingLinkedListIteratorBase::ModRingLinkedListIteratorBase(
	ModRingLinkedListBase& list)
{
	this->currentList = &list;
	this->currentEntry = list.head;
}

//
// ModRingLinkedListIteratorBase::operator*() -- 指している要素を返す(基底クラス)
//
// **NOTES
// 反復子が指している要素を返す。リストの最後の場合、反復子が指しているものが
// 意味を持たない場合は0を返す。
//
// **ARGUMENTS
// なし
//
// **RETURN
//	反復子が指す要素オブジェクトへのポインタ
//
// **EXCEPTIONS
// なし
//
ModRingLinkedEntry*
ModRingLinkedListIteratorBase::operator*()
{
	return this->currentEntry;
}

//
// ModRingLinkedListIterator<Entry>::operator++() -- 反復子を進める前置演算子(基底クラス)
// 
// NOTES
//	1つだけ反復子を進める。自分が次の要素を指すように変更して、自分への
//	参照を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	進んだ後の位置を指す自分自身への参照
//
// EXCEPTIONS
//	なし
//
ModRingLinkedListIteratorBase&
ModRingLinkedListIteratorBase::operator++()
{
	ModSinglyLinkedEntry* entry = this->currentEntry;
	// 無効の0のときはそのまま。
	if (entry != 0) {
		// ずっと回る。最後ということはない。
		this->currentEntry = entry->next;
	}
	return *this;
}

#endif

//
// Copyright (c) 1997, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
