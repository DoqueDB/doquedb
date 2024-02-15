// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModRingLinkedList.h -- ModRingLinkedList のクラス定義
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

#ifndef	__ModRingLinkedList_H__
#define __ModRingLinkedList_H__

#include "ModCommon.h"
#include "ModLinkedListBase.h"

//
// モジュールはメモリモジュール(ModModuleMemory)に属する。
// したがって、エラーはModMemoryXXXである。
//

//
// TYPEDEF
//	ModRingLinkedEntry -- リングバッファにつなぐエントリの型
// NOTES
//	単方向リストのエントリと同じ形なので、型定義のみとする。
//	リングバッファにつなぐエントリは本クラスのサブクラスとして作成する
//	必要がある。
//

typedef ModSinglyLinkedEntry ModRingLinkedEntry;

//
// CLASS
// ModRingLinkedListBase -- リングバッファリストModRingLinkedList の基底クラス
//
// NOTES
//	リングバッファは、本クラスの派生クラスとして作成する。
//	リングバッファはメモリネゴシエーション関数エントリの
//	管理のためだけに作成したもので、原則非公開である。
//	イテレータも使わずにアクセスするなど、汎用性は高くない。
//	リングバッファなので、先頭からたどりはじめ、最初と同じエントリにあったら、
//	終了である。つまり削除は想定されていない。
//

// メモリハンドルの管理のために利用されるため、
// メモリ管理よりも下位のモジュールと位置づけ、ModObjectのサブクラスとはしない。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModRingLinkedListBase
{
	friend class ModRingLinkedListIteratorBase;
public:
	// コンストラクタ
	ModRingLinkedListBase();
	virtual ~ModRingLinkedListBase();
	// リストへの登録
	ModCommonDLL
	void insert(ModRingLinkedEntry*);
	// リストからの抹消(eraseに相当)
	ModCommonDLL
	void expunge(ModRingLinkedEntry*);
	// 登録の有無の確認
	ModCommonDLL
	ModBoolean isRegistered(ModRingLinkedEntry*) const;
	// 空のリストかどうかの確認
	ModBoolean				isEmpty() const;
	// リストに登録されているエントリの数を返す
	ModSize getSize() const;
	// 先頭のエントリを返す
	ModRingLinkedEntry* begin() const;
	// メモリ獲得が失敗したとき例外を送出するため
	ModCommonDLL
	void* operator new(size_t size);

protected:
	// リストの実体
	ModRingLinkedEntry* head;
	ModRingLinkedEntry* last;

	ModSize size;
};

//
// FUNCTION
// ModRingLinkedListBase::ModRingLinkedListBase -- リングバッファリストModRingLinkedList基底クラスのコンストラクタ
//
// NOTES
// リングバッファリストを実装するための変数を初期化する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		初期化前の場合のみModCommonInitialize::checkAndInitialize()の例外参照
//
inline
ModRingLinkedListBase::ModRingLinkedListBase()
	:head(0), size(0), last(0)
{
	// 初期化チェック
	// 初期化中のエラーは文字列が送出されるかも。
	ModCommonInitialize::checkAndInitialize();
}

//
// FUNCTION
// ModRingLinkedListBase::~ModRingLinkedListBase -- リングバッファリストModRingLinkedList基底クラスのデストラクタ
//
// NOTES
// リングバッファリスト基底クラスのデストラクタ
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
ModRingLinkedListBase::~ModRingLinkedListBase()
{
}

//
// FUNCTION
// ModRingLinkedListBase::isEmpty -- リングバッファリストが空かどうか
//
// NOTES
// リングバッファリストが空かどうか調べて返す。
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
ModRingLinkedListBase::isEmpty() const
{
	return (this->head) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModRingLinkedListBase::getSize -- リングバッファリストの要素数を得る
//
// NOTES
// リングバッファリストに登録されている要素の数を返す。
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
inline ModSize
ModRingLinkedListBase::getSize() const
{
	return this->size;
}

//
// FUNCTION
// ModRingLinkedListBase::begin -- リングバッファリストの最初の要素を得る
//
// NOTES
//	リングバッファリストに登録されている最初の要素を返す。
//	ない場合は0を返す。リングバッファであるので、最初の要素からたどりはじめて、
//	最初と同じものにあったら一周したことになる。
//
// ARGUMENTS
//	なし
//
// RETURN
//	最初の要素を返す
//
// EXCEPTIONS
//	なし
//
inline ModRingLinkedEntry*
ModRingLinkedListBase::begin() const
{
	return this->head;
}

// ---------------------------------
//	以下はプロトタイプとして作ったが結局使っていないのでコメントを
//	抜く。
#ifdef OBSOLETE
template <class Entry>
class ModRingLinkedList
	: public	ModRingLinkedListBase
{
public:
	// リストへの登録
	void insert(Entry* entry);
	// リストからの抹消
	void expunge(Entry* entry);
	// 登録されているかどうかをチェックする
	ModBoolean isRegistered(Entry*) const;
};

template <class Entry>
inline void
ModRingLinkedList <Entry>::insert(Entry* entry)
{
	ModRingLinkedListBase::insert((ModRingLinkedEntry*)entry);
}

template <class Entry>
inline void
ModRingLinkedList <Entry>::expunge(Entry* entry)
{
	ModRingLinkedListBase::expunge((ModRingLinkedEntry*)entry);
}

template <class Entry>
inline ModBoolean
ModRingLinkedList <Entry>::isRegistered(Entry* entry) const
{
	return ModRingLinkedListBase::isRegistered((ModRingLinkedEntry*)entry);
}

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

class ModRingLinkedListIteratorBase
{
	friend class ModRingLinkedListBase;
public:
	// コンストラクタ
	ModRingLinkedListIteratorBase();
	ModRingLinkedListIteratorBase(ModRingLinkedListBase&);
	// 要素を返す
	ModRingLinkedEntry* operator*();
	// 次へ
	ModRingLinkedListIteratorBase& operator++();
	// = オペレータ
	ModRingLinkedListIteratorBase&
	operator=(const ModRingLinkedListIteratorBase& original);
	// == オペレータ
	ModBoolean operator==(const ModRingLinkedListIteratorBase& other) const;
	// != オペレータ
	ModBoolean operator!=(const ModRingLinkedListIteratorBase& other) const;

	// 有効か
	ModBoolean isAvailable() const;
	// メモリ獲得が失敗したとき例外を送出するため
	void* operator new(size_t size);
private:
	// 現在のエントリ
	ModRingLinkedEntry* currentEntry;
	// 現在のリスト
	ModRingLinkedListBase* currentList;
};

// デフォルトコンストラクタ
inline
ModRingLinkedListIteratorBase::ModRingLinkedListIteratorBase()
	:currentList(0), currentEntry(0)
{
}

inline
ModRingLinkedListIteratorBase&
ModRingLinkedListIteratorBase::operator=(const ModRingLinkedListIteratorBase& original)
{
	this->currentList = original.currentList;
	this->currentEntry = original.currentEntry;

	return *this;
}

inline
ModBoolean
ModRingLinkedListIteratorBase::operator==(const ModRingLinkedListIteratorBase& 
										  other) const
{
	// 0を指していては== とはみなさない。
	if (this->currentList != 0
		&& this->currentList == other.currentList
		&& this->currentEntry == other.currentEntry) {
		return ModTrue;
	} else {
		return ModFalse;
	}
}

inline
ModBoolean
ModRingLinkedListIteratorBase::operator!=(const ModRingLinkedListIteratorBase& 
										  other) const
{
	if (other == *this) {
		return ModFalse;
	} else {
		return ModTrue;
	}
}

inline
ModBoolean
ModRingLinkedListIteratorBase::isAvailable() const
{
	// リストすら決まっていない
	if (this->currentList == 0) {
		return ModFalse;
	}
	// 何もさしていないのはあり。
	return ModTrue;
}
#endif
// -----------------------------------

#endif	// __ModRingLinkedListBase_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
