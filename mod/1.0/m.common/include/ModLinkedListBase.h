// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModLinkedListBase.h -- ModLinkedListBase のクラス定義
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModLinkedListBase_H__
#define __ModLinkedListBase_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModCommonInitialize.h"

//
// モジュールは標準ライブラリー(ModModuleStandard)に属する。
// したがって、エラーはModStandardXXXである。
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
// CLASS
// ModSinglyLinkedEntry -- 単方向リスト要素の基底クラス
//
// NOTES
// 単方向リストModSinglyLinkedList で繋がれる要素オブジェクトの基底クラス
// である。
// ModSinglyLinkedListでつなぐエントリの型は、本クラスの派生クラスとして
// 生成する必要がある。
//
// ★注意★もし、一つのオブジェクトを二本以上のリストにつなぐ
// 場合には、問題があるので単純に使ってはいけない。
// その場合は別に用意する非侵入的リストModListを用いること。
//
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModSinglyLinkedEntry
{
	friend class ModRingLinkedListBase;
	friend class ModSinglyLinkedListBase;
	friend class ModSinglyLinkedListIteratorBase;
public:
	// コンストラクター
	ModSinglyLinkedEntry();
	// デストラクター
	~ModSinglyLinkedEntry()
	{}

protected:
	ModSinglyLinkedEntry* next;
};

//
// FUNCTION public
// ModSinglyLinkedEntry::ModSinglyLinkedEntry -- 単方向リスト要素のコンストラクタ
//
// NOTES
// 単方向リストの要素の基底クラスのコンストラクタである。
// リストのリンク用変数を初期化する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
inline
ModSinglyLinkedEntry::ModSinglyLinkedEntry()
	:next(0)
{}

//
// CLASS
// ModSinglyLinkedListBase -- 単方向リストModSinglyLinkedList の基底クラス
//
// NOTES
// リストは、本クラスの派生クラスとして作成する。
// 実際には、リスト用のテンプレートクラスが用意されている。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModSinglyLinkedListBase
{
	friend class ModSinglyLinkedListIteratorBase;
public:
	// コンストラクター
	ModSinglyLinkedListBase();
	// デストラクター
	virtual ~ModSinglyLinkedListBase();
	// リストへの登録
	ModCommonDLL
	void insert(ModSinglyLinkedEntry*);
	// リストからの抹消(eraseに相当)
	ModCommonDLL
	void expunge(ModSinglyLinkedEntry*);
	// 登録の有無の確認
	ModCommonDLL
	ModBoolean isRegistered(ModSinglyLinkedEntry*) const;
	// 空のリストかどうかの確認
	ModBoolean				isEmpty() const;
	// リストに登録されているエントリの数を返す
	ModSize getSize() const;
	// メモリ獲得が失敗したとき例外を送出するため
	ModCommonDLL
	void* operator new(size_t size);

protected:
	// リストの実体
	ModSinglyLinkedEntry* head;
	ModSize size;
};

//
// FUNCTION
// ModSinglyLinkedListBase::ModSinglyLinkedListBase -- 単方向リストModSinglyLinkedList基底クラスのコンストラクタ
//
// NOTES
// 単方向リストを実装するための変数を初期化する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		初期化前の場合のみ、ModCommonInitialize::checkAndInitializeの例外参照
//
inline
ModSinglyLinkedListBase::ModSinglyLinkedListBase()
	:head(0), size(0)
{
	// 初期化チェック
	// 初期化中のエラーは文字列が送出されるかも。
	ModCommonInitialize::checkAndInitialize();
}

//
// FUNCTION
// ModSinglyLinkedListBase::~ModSinglyLinkedListBase -- 単方向リストModSinglyLinkedList基底クラスのデストラクタ
//
// NOTES
// 単方向リスト基底クラスのデストラクタ
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModSinglyLinkedListBase::~ModSinglyLinkedListBase()
{
}

//
// FUNCTION
// ModSinglyLinkedListBase::isEmpty -- 単方向リストが空かどうか
//
// NOTES
// 単方向リストが空かどうか調べて返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	空であればModTrue、要素があればModFalseを返す。
//
// EXCEPTIONS
//	なし
//

inline
ModBoolean
ModSinglyLinkedListBase::isEmpty() const
{
	return (this->head) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModSinglyLinkedListBase::getSize -- 単方向リストの要素数を得る
//
// NOTES
// 単方向リストに登録されている要素の数を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	登録されている要素数
//
// EXCEPTIONS
//	なし
//
inline
ModSize
ModSinglyLinkedListBase::getSize() const
{
	return this->size;
}

//
// CLASS
// ModSinglyLinkedListIteratorBase -- ModSinglyLinkedListIteratorの基底クラス
//
// NOTES
// ModSinglyLinkedListの反復子ModSinglyLinkedListIteratorは、
// 本クラスの派生クラスとして生成する必要がある。
// 実際には、派生クラスであるModSinglyLinkedListIteratorを用いることになる。
//

class ModCommonDLL ModSinglyLinkedListIteratorBase
{
	friend class ModSinglyLinkedListBase;
public:
	// コンストラクタ
	ModSinglyLinkedListIteratorBase(ModSinglyLinkedListBase&);
	// 要素を返す
	ModSinglyLinkedEntry* operator*();
	// 次へ
	ModSinglyLinkedListIteratorBase& operator++();
	// メモリ獲得が失敗したとき例外を送出するため
	void* operator new(size_t size);
private:
	// 現在のエントリ
	ModSinglyLinkedEntry* currentEntry;
	// 現在のリスト
	ModSinglyLinkedListBase* currentList;
};

// ** 以下は整備していない。要望に応じて整備をすすめる

#ifdef OBSOLETE // not implemented

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

// doubly linked list で繋がれるオブジェクトの基底クラス
class ModDoublyLinkedEntry
{
	friend class ModDoublyLinkedListBase;
	friend class ModDoublyLinkedListIteratorBase;
public:
	// コンストラクター
	ModDoublyLinkedEntry()
		: next(0), prev(0)
	{}
	// デストラクター
	~ModDoublyLinkedEntry()
	{}

private:
	ModDoublyLinkedEntry* next;
	ModDoublyLinkedEntry* prev;
};

// doubly linked list の基底クラス
class ModCommonDLL ModDoublyLinkedListBase
{
	friend class ModDoublyLinkedListIteratorBase;
public:
	// コンストラクタ
	ModDoublyLinkedListBase() { head.next = &head; head.prev = &head; };
	// リストへの登録
	void insert(ModDoublyLinkedEntry* entry);
	// リストからの抹消
	void expunge(ModDoublyLinkedEntry* entry);
	// 登録の有無の確認
	ModBoolean isRegistered(ModDoublyLinkedEntry*) const;
	// 空のリストかどうかの確認
	ModBoolean isEmpty() { return (head.next == &head) ? ModTrue : ModFalse; };
private:
	// リストの実体
	ModDoublyLinkedEntry head;
};

// doubly linked list の反復子の基底クラス
class ModCommonDLL ModDoublyLinkedListIteratorBase
{
public:
	// コンストラクタ
	ModDoublyLinkedListIteratorBase(ModDoublyLinkedListBase&);
	// 要素を返す
	ModDoublyLinkedEntry* operator()();
private:
	// 現在のエントリ
	ModDoublyLinkedEntry* currentEntry;
	// 現在のリスト
	ModDoublyLinkedListBase* currentList;
};
#endif	// not implemented 

#endif	// __ModLinkedListBase_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
