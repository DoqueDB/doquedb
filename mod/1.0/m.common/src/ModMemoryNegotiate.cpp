// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryNegotiate.cpp -- ModMemoryNegotiate のメンバ定義
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

#include "ModAutoMutex.h"
#include "ModRingLinkedList.h"
#include "ModMemoryNegotiate.h"
#include "ModMemoryPool.h"

//
// VARIABLE
// ModMemoryNegotiate::negotiateListing -- メモリネゴシエーション関数登録リスト
//
// NOTES
// 登録されたすべてのネゴシエーション関数を管理するリスト。
// ModMemoryNegotiate::initialize()で初期化され、使えるようになる。
// ModMemoryNegotiate::negotiateListingを初期化フラグのかわりに利用している。
// 
ModMemoryNegotiateRing* ModMemoryNegotiate::negotiateListing = 0;

//
// VARIABLE
// ModMemoryNegotiate::start -- 次に呼び出すネゴシエーション関数の候補
//
// NOTES
//	次に呼び出すネゴシエーション関数を覚えておく。
//	マルチスレッドでは、異なるスレッドやメモリハンドルからどんどん
//	ネゴシエーションが行なわれる可能性がある。
//	一回のネゴシエーションでは、ネゴ関数の登録されているリングバッファを
//	メモリがとれるまで回り、一巡するとあきらめる。
//	一方、たどり始めのネゴ関数エントリは、この変数でとっておき、次の
//	ネゴシエーションの最初はそこから始める。
//	この方法ではあまり「優先順位」が重視されないが、いつも優先度が大きいところ
//	からとっていくと先頭のネゴシエーション関数に集中しすぎることが考えられる。
//	今後チューニングが必要かもしれない。
ModMemoryNegotiate* ModMemoryNegotiate::start = 0;

//
// FUNCTION public
// ModMemoryNegotiateRing::insert -- ネゴシエーション関数のリングバッファへの挿入
//
// NOTES
//	ネゴシエーション関数をプライオリティ順にリングバッファに登録する。
//
// ARGUMENTS
//	登録するネゴシエーション関数エントリ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
void
ModMemoryNegotiateRing::insert(ModMemoryNegotiate* newEntry)
{
	ModMemoryHandle::Priority priority = newEntry->getPriority();
	ModMemoryNegotiate* begin = (ModMemoryNegotiate*)this->head;

	// 必ず大きい順にならんでいる。
	if (begin == 0) {
		// 何も要素がないときはそのまま挿入
		ModRingLinkedListBase::insert(newEntry);
		return;
	} else {
		// そうでないときは優先順位の順番、
		// 同じ順位の中では先頭にする
		// 大きい順に搾取されるのでその順にならべる。
		ModMemoryNegotiate* entry = begin;
		ModMemoryNegotiate* prev = 0;
		do {
			if (entry->getPriority() <= priority) {
				// entryの前に入れる
				if (prev == 0) {
					// 先頭に入れればよい
					ModRingLinkedListBase::insert(newEntry);
					return;
				} else {
					// prevとentryの間につなぐ
					break;
				}
			}
			// 次へ。
			prev = entry;
			entry = entry->getNext();
		} while (entry != begin);

		prev->setNext(newEntry);
		newEntry->setNext(entry);
		// もし一番最後への挿入だったらここまで回ってしまう。
		// その場合はprev == last, entry == beginのはず
		if (this->last == prev) {
			; ModAssert(entry == begin);
			this->last = newEntry;
		}
	}
}

//
// FUNCTION private
// ModMemoryNegotiate::initialize -- メモリネゴシエーションクラス全体の初期化
//
// NOTES
//	メモリネゴシエーションクラス全体の初期化を行なう。
//	メモリネゴシエーション関数登録リストの初期化を行なう。
//	メモリハンドルの初期化関数からしか呼び出されない。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない
//
void
ModMemoryNegotiate::initialize()
{
	try {
		if (ModMemoryNegotiate::negotiateListing == 0) {
#ifdef RINGTEMP
			ModMemoryNegotiate::negotiateListing
				= new ModRingLinkedList<ModMemoryNegotiate>();
#endif
			ModMemoryNegotiate::negotiateListing
				= new ModMemoryNegotiateRing();
		}
	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleOs);
	}
#endif
}

//
// FUNCTION private
// ModMemoryNegotiate::terminate -- メモリネゴシエーションクラス全体の後処理
//
// NOTES
//	メモリネゴシエーションクラス全体の後処理を行なう。
//	メモリネゴシエーション関数登録リストの後処理を行なう。
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
void
ModMemoryNegotiate::terminate()
{
	ModMemoryNegotiate* entry
		= (ModMemoryNegotiate*)ModMemoryNegotiate::negotiateListing->begin();
	ModMemoryNegotiate* begin = entry;
	if (entry != 0) {
		do {
			ModMemoryNegotiate* tmp = entry->getNext();
			delete entry;
			entry = tmp;
		} while (entry != begin);
	}
	// エントリとともにリングバッファも全部いっきょに消すので
	// ちまちまリストからはずすexpungeをする必要はない。
	delete ModMemoryNegotiate::negotiateListing;
	ModMemoryNegotiate::negotiateListing = 0;
}

//
// FUNCTION
// ModMemoryNegotiate::ModMemoryNegotiate -- メモリネゴシエーションクラスのコンストラクタ
//
// NOTES
//	メモリネゴシエーション関数エントリのコンストラクタ。
//	優先度や、関数ポインタを設定し、有効フラグをModTrueに設定する。
//
// ARGUMENTS
//	void (*negotiate_)(ModSize size)
//		登録するネゴシエーション関数ポインタ
//	ModMemoryHandle::Priority priority_
//		設定する優先度
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
ModMemoryNegotiate::ModMemoryNegotiate(void (*negotiate_)(ModSize size), 
									   ModMemoryHandle::Priority priority_)
	:negotiate(negotiate_), priority(priority_), available(ModTrue)
{
}

//
// FUNCTION
// ModMemoryNegotiate::~ModMemoryNegotiate -- メモリネゴシエーションクラスのデストラクタ
//
// NOTES
//	メモリネゴシエーション関数エントリのデストラクタ。何もしない。
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
ModMemoryNegotiate::~ModMemoryNegotiate()
{
}

//
// FUNCTION
// ModMemoryNegotiate::getStartPoint -- 次に呼び出すネゴシエーション関数エントリを得る
//
// NOTES
//	次に呼び出すべき順番のネゴシエーション関数を登録したエントリを
//	スタートポイントという。スタートポイントを返し、
//	スタートポイントを次に進める。スタートポイントは、スレッドやメモリハンドル
//	に依存することなく、どんどんリングバッファ上を回っていく。
//	一つも登録されていなければ0を返す。
//	スタートポイントとして設定されているエントリが無効でもそのまま返す。
//	そうでないと、呼び出し元で覚えているbeginが無効になってしまった場合に
//	スタートポイントを進める処理が永久ループになりかねない。
//
//	ロックしてから呼び出すことが前提である。
//
// ARGUMENTS
//	なし
//
// RETURN
//	次に呼び出すべきネゴシエーション関数エントリへのポインタ
//
// EXCEPTIONS
//	なし
//
ModMemoryNegotiate*
ModMemoryNegotiate::getStartPoint()
{
	ModMemoryNegotiate* target = 0;
	ModMemoryNegotiate* entry = ModMemoryNegotiate::start;
	// ひとつもない
	if (entry == 0) {
		// 本当に空
		if (ModMemoryNegotiate::negotiateListing->isEmpty() == ModTrue) {
			return entry;
		} else {
			// 一回目なので設定されていなかった。もはや空ではないから設定する
			ModMemoryNegotiate::start
				= (ModMemoryNegotiate*)ModMemoryNegotiate::negotiateListing->begin(); 	   return ModMemoryNegotiate::start;
		}
	}
/*
	ModRingLinkedEntry* begin = entry;
    while(entry->isAvailable() == ModFalse) {
		// 関数がすでに登録抹消されている
		// 次へすすめてみる。
		entry = entry->next;
		if (entry == begin) {
			// 全部まわってしまった。
			// 抹消されていないのがひとつもなかったら0を返す
			return 0;
		}
	}
*/
	target = entry;

	// とりあえず次の人のためにすすめておく。一つしかなければ同じものになる。
	// 無効でもそのまま返す。でないと、呼び出し元で覚えているbeginが
	// 無効になったら終らなくなる。
	ModMemoryNegotiate::start = entry->getNext();
	return target;
}

//
// FUNCTION
// ModMemoryNegotiate::callNegotiate -- ネゴシエーション関数を呼び出す
//
// NOTES
//	エントリに登録されているネゴシエーション関数を呼び出す。
//
// ARGUMENTS
//	ModSize size
//		要求サイズ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ネゴシエーション関数の例外をそのまま送出
//
void
ModMemoryNegotiate::callNegotiate(ModSize size)
{
	(*this->negotiate)(size);
}

//
// FUNCTION
// ModMemoryNegotiate::getNext -- 次のネゴシエーション関数エントリを得る
//
// NOTES
//	次のネゴシエーション関数エントリを返す。リングバッファなので、
//	エントリが一つの場合、次もこのエントリになる。
//
// ARGUMENTS
//	なし
//
// RETURN
//	次のネゴシエーション関数エントリへのポインタ
//
// EXCEPTIONS
//	なし
//
ModMemoryNegotiate*
ModMemoryNegotiate::getNext() const
{
	if (this == 0) {
		return 0;
	}
	return (ModMemoryNegotiate*)this->next;
}

//
// FUNCTION
// ModMemoryNegotiate::setNext -- 次のネゴシエーション関数エントリを設定する
//
// NOTES
//	次のネゴシエーション関数エントリを設定する。
//	プライオリティ順の挿入のために利用される。
//
// ARGUMENTS
//	ModMemoryNegotiate* entry
//		次に挿入したいエントリ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
void
ModMemoryNegotiate::setNext(ModMemoryNegotiate* entry)
{
	this->next = entry;
}

//
// FUNCTION
// ModMemoryNegotiate::print -- ネゴシエーション関数エントリ内容を出力
//
// NOTES
//	オブジェクトの内容をデバッグ出力する。
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
void
ModMemoryNegotiate::print() const
{
	ModDebugMessage << ModHex
#ifdef MOD64
					<< (ModUInt64)negotiate
#else
					<< (unsigned int)negotiate
#endif
					<< ", " << ModDec 
					<< priority << ", " 
					<< this->available << ", " << ModEndl;
}

//
// FUNCTION
// ModMemoryNegotiate::printList -- ネゴシエーション関数エントリリングバッファ内容を出力
//
// NOTES
//	ネゴシエーション関数として登録されているエントリの内容をすべて
//	デバッグ出力する。設定されているスタートポイントも示す。
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
void
ModMemoryNegotiate::printList()
{
	ModMemoryNegotiate* begin = (ModMemoryNegotiate*)
		ModMemoryNegotiate::negotiateListing->begin();
	ModMemoryNegotiate* entry = begin;

	ModSize count = ModMemoryNegotiate::negotiateListing->getSize();
	ModDebugMessage << "negotiateListing count = " << count << ModEndl;
	if (begin != 0) {
		do {
			entry->print();
			if (ModMemoryNegotiate::start == entry) {
				ModDebugMessage << "^^^^^^^<----start";
			}
			entry = entry->getNext();
		} while (entry != begin);
	}
}

//
// FUNCTION
// ModMemoryNegotiate::operator new -- メモリネゴシエーションクラスのnew
//
// NOTES
// メモリ管理のために用意する本クラスはMODの中でも特別な扱いを受ける。
// 通常、メモリはすべてどれかのメモリハンドルに管理されるが、
// MemoryNegotiateオブジェクト自身を確保するためのメモリは
// メモリプールから直接獲得され、どのメモリハンドルの管理下にも入らない。
// 理由についてはModMemoryDebugCellと同様。
//
// ARGUMENTS
// size_t size
//      確保するサイズ
// size_t dummy
//		この引数を定義しないと、VC++ 6.0 で C4291 の警告が発生し、
//		new されたオブジェクトのコンストラクターで例外が発生したときに、
//		delete が呼び出されず、確保された自由記憶領域が解放されない
//
// RETURN
// 確保したメモリの先頭アドレス
//
// EXCEPTIONS
//	その他
//		ModMemoryPool::allocateMemoryの例外参照(主なものは以下に書き下す)
//	ModMemoryErrorOsAlloc				(ModMemory::allocateMemory参照)
//		メモリーが新たに獲得できない
//	ModMemoryErrorPoolLimitSize			(ModMemory::allocateMemory参照)
//		使用量がプールの制限値を超える

void*
ModMemoryNegotiate::operator new(size_t size, size_t dummy)
{
	// C++の定義により、引数はsize_tであったが、ここで変換
	return ModMemoryPool::allocateMemory((ModSize)size);
};

//
// FUNCTION
// ModMemoryNegotiate::operator delete -- メモリネゴシエーションクラスのdelete
//
// NOTES
// メモリ管理のために用意する本クラスはMODの中でも特別な扱いを受ける。
// メモリネゴシエーションクラスのnewで確保されたメモリ(メモリプールから直接
// 確保されている)を解放する。
//
// ARGUMENTS
// void* address
//		解放するメモリの先頭アドレス
// size_t size
//      確保したサイズ
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Mutex::lock, unlockの例外参照
//
void
ModMemoryNegotiate::operator delete(void* address, size_t size)
{
	// C++の定義により、引数はsize_tであったが、ここで変換
	ModMemoryPool::freeMemory(address, (ModSize)size);
};

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//


