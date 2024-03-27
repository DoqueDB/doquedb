// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryNegotiate.h -- ネゴシエーション関数登録 クラス定義(非公開)
// 
// Copyright (c) 1997, 2009, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef	__ModMemoryNegotiate_H__
#define __ModMemoryNegotiate_H__

// 公開するヘッダファイルを減らすため、ModMemoryHandle.h からファイルを分ける。
// メモリハンドルのネゴシエーション関数のためのクラスである。

#include "ModMemoryHandle.h"
#include "ModRingLinkedList.h"

//
//	モジュールはメモリモジュールに属する。
//	したがって、エラーはModMemoryXXXである。
//
//	ModMemoryNegotiateRing, ModMemoryNegotiateとも
//	メモリハンドルの管理のために使われるので、メモリハンドルを通じて
//	メモリを獲得するのではなく、specialなnew/deleteを定義する。
//	もちろんModObjectのサブクラスとしては作成しない。

class ModMemoryNegotiate;

//
// CLASS
// ModMemoryNegotiateRing -- ネゴシエーション関数管理用リングバッファ
//
// NOTES
// メモリのネゴシエーション関数を登録、管理するリングバッファクラス。
// ネゴシエーション関数と優先度を組にしたエントリを登録する。
// デフォルトのリングバッファと異なり、プライオリティ順に挿入する必要がある
// のでメソッドinsertのみ用意した。
//

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

class ModMemoryNegotiateRing
	: public	ModRingLinkedListBase
{
public:
	// プライオリティ順、同じプライオリティ中では最初に挿入
	void insert(ModMemoryNegotiate* negotiateEntry);
};

//
// CLASS
// ModMemoryNegotiate -- メモリネゴシエーション関数管理のためのクラス
//
// NOTES
// メモリのネゴシエーション関数を登録、管理するためのクラスである。
// ネゴシエーション関数と優先度を組にしたエントリを表す。
// これらをリングバッファに登録して管理する。
// 登録抹消すると、有効フラグがFalseになる。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMemoryNegotiate
	: public	ModRingLinkedEntry
{
public:
	// 初期化関数限定よびだしのため。
	friend class ModMemoryHandle;

	ModCommonDLL
	ModMemoryNegotiate(void (*negotiate_)(ModSize size),
					   ModMemoryHandle::Priority priority_);
	ModCommonDLL
	virtual ~ModMemoryNegotiate();

	// ネゴシエーション関数を呼び出す。
	ModCommonDLL
	void callNegotiate(ModSize size);
	// 無効になっていないかどうか
	ModBoolean isAvailable() const;

	// ターゲットとなるエントリを得て、次にすすめておく
	ModCommonDLL
	static ModMemoryNegotiate* getStartPoint();
	// 次のエントリを返す
	ModCommonDLL
	ModMemoryNegotiate* getNext() const;
	// 次のエントリを設定する
	ModCommonDLL
	void setNext(ModMemoryNegotiate*);
	// プライオリティを返す
	ModMemoryHandle::Priority getPriority() const;

	// (デバッグ用)内容をプリントする
	ModCommonDLL
	void print() const;
	// (デバッグ用)リストをプリントする
	ModCommonDLL
	static void printList();

	ModCommonDLL
	void* operator new(size_t size, size_t dummy = 0);	// special
#ifdef STD_CPP11
	ModCommonDLL
	void  operator delete(void* address, size_t size) noexcept(false);	// special
	ModCommonDLL
	void  operator delete(void* address) noexcept(false);	// ダミー
#else
	ModCommonDLL
	void  operator delete(void* address, size_t size);		// special
#endif

private:
	// リストの初期化、後処理
	static void initialize();
	static void terminate();

	// 登録情報(関数、優先度)
	void (*negotiate)(ModSize size);
	ModMemoryHandle::Priority priority;
	// 有効フラグ
	ModBoolean available;

	// 優先度順のリストを用意する
	// static ModRingLinkedListBase* negotiateListing;
	static ModMemoryNegotiateRing* negotiateListing;

	// 次のネゴの最初のターゲット
	// static ModRingLinkedListIteratorBase start;
	// static ModRingLinkedEntry* start;
	static ModMemoryNegotiate* start;
};

//
// FUNCTION
// ModMemoryNegotiate::isAvailable -- ネゴシエーション関数登録が有効かどうか
//
// NOTES
//	このエントリが有効かどうかを返す。
//	管理している関数が登録抹消されている場合はFalseを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	そのエントリが有効ならばModTrue、無効ならばModFalse
//
// EXCEPTIONS
//	なし
//
inline
ModBoolean
ModMemoryNegotiate::isAvailable() const
{
	return this->available;
}

//
// FUNCTION
// ModMemoryNegotiate::getPriority -- ネゴシエーション関数に設定された優先度を得る
//
// NOTES
//	このネゴシエーション関数に設定された優先度を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	優先度をModMemoryHandle::Priorityの値で返す。
//	この値が大きい順に呼び出される。
//
// EXCEPTIONS
//	なし
//
inline
ModMemoryHandle::Priority
ModMemoryNegotiate::getPriority() const
{
	return this->priority;
}

#endif	// __ModMemoryNegotiate_H__

//
// Copyright (c) 1997, 2009, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
