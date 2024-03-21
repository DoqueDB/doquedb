// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryHandle.cpp -- ModMemoryHandle のメンバ定義
// 
// Copyright (c) 1997, 2011, 2023, 2024 Ricoh Company, Ltd.
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

#include "ModMemoryPool.h"
#include "ModMemoryHandle.h"
#include "ModAutoMutex.h"
#include "ModThread.h"
#include "ModMemoryNegotiate.h"
#include "ModParameter.h"

//
// DEBUG FUNCITON
//
#ifdef DEBUG
extern "C" {
	void ModAllocateMemoryWatchPoint();
};

void* ModAllocateMemoryWatchAddress = 0;

void
ModAllocateMemoryWatchPoint()
{
}
#endif

//
// VARIABLE
// ModMemoryHandle::allListing -- メモリハンドルのリスト
//
// NOTES
// MOD内部で作成されたすべてのメモリハンドルを管理するリスト。
// ModMemoryHandle::initialize()で初期化され、使えるようになる。
// ModMemoryHandle::allListingを初期化フラグのかわりに利用している。
// 
ModSinglyLinkedList<ModMemoryHandle>* ModMemoryHandle::allListing = 0;

ModSinglyLinkedList<ModNegotiatingThread>* 
ModMemoryHandle::negotiatingThreadList = 0;

//
// VARIABLE
// ModMemoryHandle::defaultPriority -- デフォルトプライオリティ
//
// NOTES
// デフォルトで作成されたメモリハンドルに与えられる優先度。
// 
const ModMemoryHandle::Priority 
ModMemoryHandle::defaultPriority = ModMemoryHandle::priority3;

//
// VARIABLE
// ModMemoryHandle::defaultLimitSize -- デフォルトのメモリサイズ限界値
//
// NOTES
// デフォルトで作成されたメモリハンドルに与えられるメモリサイズ限界値
// 
const ModSize ModMemoryHandle::defaultLimitSize = 10 * 1024;

//	VARIABLE
//	ModMemoryHandle::memoryLeakCheck --
//		メモリーハンドルで確保した領域がちゃんと破棄されるかを検査するか
//
//	NOTES
//		メモリーハンドルを介して確保した領域が、そのメモリーハンドルを
//		破棄するまでにちゃんと ModMemoryHandle::freeMemory で
//		破棄されるかを検査するかを表す
//
//		ModDebug::check が ModTrue のときのみ有効で、
//		メモリーハンドルの初期化時にパラメーター StopMemoryLeakCheck が
//		未定義か、False ならば、検査する

ModBoolean	ModMemoryHandle::memoryLeakCheck = ModTrue;

#ifdef DEBUG
// VARIABLE
// ModMemoryHandle::useDebugList -- アロックリストを使用するかどうか
//
// NOTES
// アロッククリストを使用するかどうか。メモリハンドル全体に対して有効。
// 現在のところ、メモリハンドルごとには設定できない。
// パラメータ"UseMemoryDebugList"にModBooleanの値で設定することで変更できる。
// UseMemoryDebugList = True ならば、useDebugList = ModTrue となり、
// 獲得したメモリのアドレスとサイズを管理・チェックするための
// ModMemoryDebugCell をアロックリスト (debugList) に登録する。
// デフォルトは ModTrue でアロックリストを使用する。
// DEBUG 時の実行速度をあげたい場合には False とすること。
// 
ModBoolean ModMemoryHandle::useDebugList = ModTrue;
#endif

struct _MemoryHandle
{
	static ModBoolean	_selfMemoryManagement;
};

ModBoolean _MemoryHandle::_selfMemoryManagement = ModTrue;

//
// FUNCTION
// ModNegotiatingThread::ModNegotiatingThread -- ネゴ中のスレッド情報コンストラクタ
//
// NOTES
//	ネゴシエーション中のスレッド情報のコンストラクタである。自スレッドの
//	IDを設定する。
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
ModNegotiatingThread::ModNegotiatingThread()
	: usingMemory(0)
{
	this->threadId = ModThisThread::self();
}

//
// FUNCTION
// ModNegotiatingThread::ModNegotiatingThread -- ネゴ中のスレッド情報デストラクタ
//
// NOTES
//	ネゴシエーション中のスレッド情報のデストラクタである。
//	デバッグ時、該当スレッドがネゴ中に確保した非常用メモリが解放されていない
//	場合にはメッセージが出力される。
//	メモリハンドルのallocateメソッドでも正式に例外が送出されるはず。
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
ModNegotiatingThread::~ModNegotiatingThread()
#if STD_CPP11
noexcept(false)
#endif
{
	if (this->usingMemory > 0) {
		ModDebugMessage << 
			"You must free allocated area in negotiate function" << ModEndl;
	}
}

//
// FUNCTION
// ModNegotiatingThread::operator new -- メモリ管理関係のクラスのdelete
//
// NOTES
// メモリ管理関係のクラスはMODの中でも特別な扱いを受ける。
// この他で確保されたメモリはすべてどれかのメモリハンドルに管理されるが、
// メモリデバッグセル自身を確保するためのメモリはメモリプールから直接獲得
// され、どのメモリハンドルの管理下にも入らない。
// 理由についてはModMemoryDebugCellの説明参照。
//
// ARGUMENTS
// size_t size
//      確保するサイズ
// size_t dummy
//		この引数を定義しないと、VC++ 6.0 で C4291 の警告が発生し、
//		new されたオブジェクトのコンストラクターで例外が発生したときに、
//		delete が呼び出されず、確保された自由記憶領域が解放されない
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			確保する領域のサイズとして 0 が指定された
//		ModMemoryErrorEmergencyLimit
//			指定されたサイズの領域を確保すると、
//			非常用領域の総サイズの上限を超える

void*
ModNegotiatingThread::operator new(size_t size, size_t dummy)
{
	try {
		return ModMemoryPool::allocateMemory((ModSize)size);

	} catch (ModException& exception) {

		switch (exception.getErrorNumber()) {
		case ModMemoryErrorOverPoolLimit:
		case ModMemoryErrorPoolLimitSize:
		case ModMemoryErrorOsAlloc:

			// 領域削減交渉によって、なんとかなる可能性もあるので、
			// 非常用領域から確保して、この場をしのぐ

			return ModMemoryPool::allocateEmergencyMemory((ModSize)size);
		}
		ModRethrow(exception);
	}
}

//
// FUNCTION
// ModNegotiatingThread::operator delete -- メモリ管理関係のクラスのdelete
//
// NOTES
// メモリ管理を実現するためのクラスはMODの中でも特別な扱いを受ける。
// ModNegotiatingThread::operator newで確保されたメモリ(メモリプールから直接
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
ModNegotiatingThread::operator delete(void* address, size_t size)
#ifdef STD_CPP11
noexcept(false)
#endif
{
	if (ModMemoryPool::isEmergencyMemory(address))
		ModMemoryPool::freeEmergencyMemory(address, (ModSize)size);
	else
		ModMemoryPool::freeMemory(address, (ModSize)size);
}

#ifdef STD_CPP11
//
// FUNCTION
// ModNegotiatingThread::operator delete -- メモリハンドルのdelete
//
// NOTES
// ModMemoryHandle::operator delete(void*)を参照のこと。
//
void
ModNegotiatingThread::operator delete(void* address) noexcept(false)
{
	// 無条件に例外を送出する
	ModThrow(ModModuleMemory,
			 ModMemoryErrorWrongDeleteCalled, ModErrorLevelError);
}
#endif

//
// FUNCTION private
// ModMemoryHandle::initialize -- メモリハンドル全体の初期化
//
// NOTES
//	メモリプールとメモリハンドル全体の初期化を行なう。
//	メモリハンドルリストと、ネゴ中のスレッドリスト、ネゴ関数登録リストの
//	初期化を行なう。
//	プライベートメソッドであり、ModCommonInitializeから
//	一回だけ呼び出されることが保証される。
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
//	その他
//		ModMemoryPool::doInitializeの例外参照(主にModOsDriver::mallocの例外(上記以外ではModCommonErrorOutOfRangeなど)
//
void
ModMemoryHandle::initialize()
{
	try {
		if (ModMemoryHandle::allListing == 0) {
			// 一回目の初期化

			// (1)メモリプールの初期化
			ModMemoryPool::doInitialize();

			// (2)メモリハンドル内部のリストの初期化
			ModMemoryHandle::allListing
				= new ModSinglyLinkedList<ModMemoryHandle>();

			ModMemoryHandle::negotiatingThreadList
				= new ModSinglyLinkedList<ModNegotiatingThread>();

			ModMemoryNegotiate::initialize();

			// パラメーターの検査

			ModParameter	parameter(ModFalse);
			ModBoolean		boolean;

			if (parameter.getBoolean(boolean,
									 "StopMemoryLeakCheck") == ModTrue)
				ModMemoryHandle::memoryLeakCheck = boolean;
#ifdef DEBUG				
			if (parameter.getBoolean(boolean, "UseMemoryDebugList") == ModTrue)
				ModMemoryHandle::useDebugList = boolean;
#endif
			if (parameter.getBoolean(
					boolean, "SelfMemoryManagement") == ModTrue)
				_MemoryHandle::_selfMemoryManagement = boolean;
		}
	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif
}

//
// FUNCTION private
// ModMemoryHandle::terminate -- メモリハンドル全体の後処理
//
// NOTES
//	メモリハンドル全体の後処理を行なう。
//	メモリハンドルリストと、ネゴ中のスレッドリスト、ネゴ関数登録リスト
//	の後処理を行なう。
//	プライベートメソッドであり、ModCommonInitializeから
//	一回だけ呼び出されることが保証される。
//	利用したメモリにメモリリークがある場合はデバッグ時に限り、
//	ここでチェックされる。
//	後処理の後にデストラクタなどから何かのメソッドがどうしても
//	呼び出されてしまう場合はModCommonInitialize::isExited()でチェックし、
//	不都合が起きないようにコーディングする必要がある。
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
ModMemoryHandle::terminate()
{
	// メモリリークのチェックをするかどうか

	if (ModMemoryHandle::isMemoryLeakChecked())

		// デバッグ情報
		// 残存メモリーハンドルごとに、リークがあれば出力する。
		// システムの終了部分で、ModMemoryPool::terminateの前に呼び出す
		// ものではあるが、忘れては元も子もないのでここで呼び出しておく。
		// 必要に応じて移動すること。

		ModMemoryHandle::printAllList();

	delete ModMemoryHandle::allListing;
	ModMemoryHandle::allListing = 0;

	delete ModMemoryHandle::negotiatingThreadList;
	ModMemoryHandle::negotiatingThreadList = 0;

	ModMemoryNegotiate::terminate();

	// プールの後処理
	ModMemoryPool::doTerminate();
}
//
// FUNCTION
// ModMemoryHandle::ModMemoryHandle  -- メモリハンドルのコンストラクタ
//
// NOTES
// メモリハンドルクラスのコンストラクタである。引数には、このメモリハンドルに
// 与えられるメモリ確保の優先度と、メモリ使用量の限界値を指定する。
// メモリ使用量の限界値にはバイト単位の値を指定する。
// メモリ確保の優先度は、ModMemoryHandle::Priorityで定義される
// 4種類から選択する。
// メモリハンドルが生成されると、メモリハンドルを管理するリストに登録される。
//
// ARGUMENTS
// ModSize limitSize_
//      メモリ使用の限界値(バイト単位)
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
// 		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//

ModMemoryHandle::ModMemoryHandle(ModSize limitSize_)
	: currentSize(0)
#ifdef DEBUG
	, allocateCheck(ModFalse)
#endif
{
	// プライオリティ別のリストに挿入する
	this->limitSize = limitSize_;

	// 初期化チェック
	// 初期化中のエラーは文字列が送出されるかも。
	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	// リストに挿入する
	ModMemoryHandle::allListing->insert(this);
}

//
// FUNCTION
// ModMemoryHandle::ModMemoryHandle  -- メモリハンドルのデフォルトコンストラクタ
//
// NOTES
// メモリハンドルクラスのデフォルトコンストラクタである。
// メモリ確保の優先度は最低、メモリ使用量の限界値は10Kに過ぎない。
// メモリハンドルが生成されると、メモリハンドルを管理するリストに登録される。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
// 		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//
ModMemoryHandle::ModMemoryHandle()
	: currentSize(0)
#ifdef DEBUG
	, allocateCheck(ModFalse)
#endif
{
    this->limitSize = ModMemoryHandle::defaultLimitSize;

	// 初期化チェック
	// 初期化中のエラーは文字列が送出されるかも。
	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	// リストに挿入する
	ModMemoryHandle::allListing->insert(this);
}

//
// FUNCTION
// ModMemoryHandle::~ModMemoryHandle  -- メモリハンドルのデストラクタ
//
// NOTES
// メモリハンドルクラスのデストラクタである。
// メモリハンドルの後処理の際、メモリリークをチェックし、
// MOD_DEBUGかつ、ModDebug::checkが真の場合、かつ
// StopMemoryLeakCheckがFalseの場合には結果を出力する。
// 詳しい情報が必要な場合はサブクラスのデストラクタでprintMemoryLeak()を
// 呼び出すと、デバッグ版ではメモリハンドルに登録されているメモリ
// アロケーションの内容、マネージャ名まで出力する。
// 
// それと同時に、メモリハンドルを管理するリストからこのメモリハンドル
// オブジェクトを削除する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Mutex::lock, unlockの例外参照
//
ModMemoryHandle::~ModMemoryHandle()
#ifdef STD_CPP11
noexcept(false)
#endif
{
    // 自分をさがして、リストから削除する
	try {
		if (ModMemoryHandle::allListing) {

			// このときはModCommonInitialize::isInitialized()がFalseを
			// 返すはずである。文脈でわかりやすいのでこちらを採用
			// している。
			// terminate()の後にグローバル変数の後処理が呼ばれた時には
			// allListingが既に消滅している。
			// この場合は、かならずシングルスレッドで動作しているのを
			// 前提とする。
			// allListingからのエントリの消去は行なわない。

			ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
			(void) m.lock();

			ModMemoryHandle::allListing->expunge(this);
		}
#ifdef MOD_DEBUG
		// ハンドルがもっているデバッグセルのチェックが必要。
		// 残っている場合は出力される。しかし後始末はしない。

		if (this->currentSize && ModMemoryHandle::isMemoryLeakChecked()) {

			// デストラクタの中で純粋仮想関数を呼び出すと落ちる。
			// (this->print(); の中のgetName())
			// その通りといえばその通りであるが、驚いた。
			// 必要に応じてサブクラスのデストラクタからprintMemoryLeak()を
			// 呼び出すように変更する。

			// しかしすべてをサブクラスのデストラクタに移動させてしまうと
			// 呼び出しを忘れたときに困るのでこれだけは残す。

			ModDebugMessage << "memory leaked (size = " 
							<< this->currentSize << ")\n" 
							<< ModEndl;
		}
#endif
	} catch (ModException& exception) {
		// 登録されているはずなのに、みつからずに削除できなかった場合
		// まずはないはず、あったらバグ。
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif
}

// FUNCTION
// ModMemoryHandle::printMemoryLeak  -- 管理しているメモリのリーク内容の出力
//
// NOTES
// メモリハンドルクラスが管理しているメモリのリーク内容を出力する。
// メモリリークをチェックし、
// MOD_DEBUGかつ、ModDebug::checkが真の場合には結果を出力する。
// デバッグ版ではメモリハンドルに登録されているメモリアロケーションの
// 内容まで、マネージャ名とともに出力する。
// メモリハンドルの派生クラスのデストラクタで呼び出すことが期待される。
// もともとは本クラスのデストラクタで呼び出していたが、マネージャ名の
// 呼び出しで仮想関数が呼び出せないので断念した。
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

void
ModMemoryHandle::printMemoryLeak()
{
#ifdef MOD_DEBUG
	// 厳密にはここで本メモリハンドルをロックすべきであるが、
	// this->print() 内部で行うので省略。
	// たとえ、currentSizeの内容がこの後変わっても深刻な問題ではない。

	if (this->currentSize && ModMemoryHandle::isMemoryLeakChecked())
		this->print();
#endif
}

// FUNCTION public
// ModMemoryHandle::allocateMemory  -- メモリハンドルからメモリを確保する
//
// NOTES
// メモリハンドルからメモリを確保する。
// 内部的にはメモリプールからメモリを確保し、確保できなければ他のメモリハンドル
// と優先度の値の大きい順にネゴシエーションを行う。自分自身のメモリ限界を
// 超える場合は、自分自身のネゴシエーションが呼び出される。
// デバッグ時には、確保したメモリサイズとアドレスを格納したセルをリストで
// 管理する。
//
// ARGUMENTS
// ModSize size
//	要求するサイズ
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModmemoryErrorHandleLimit
//		メモリハンドルの限界値を超える
// その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, AutoMutex::unlockAllの例外参照。その他の主なものは以下に示す。
//	ModOsErrorSystemMemoryExhaust		(::newによる)
//		システムメモリが確保できない	
//	ModCommonErrorBadArgument			(ModMemoryPool::allocateMemory)
//		引数エラー
//	ModMemoryErrorOverPoolLimit			(ModMemoryPool::allocateMemory)
//		プールの制限値以上の値を指定
//	ModMemoryErrorEmergencyLimit		(ModMemoryPool::allocateEmergencyMemory)
//		非常用メモリの制限を超えている
//	ModMemoryErrorNotFreeEmergencyArea	(ModMemoryHandle::negotiateToOther)
//		ネゴシエーションが終わっても非常用で獲得した領域を解放していない
//	ModMemoryErrorNegotiateFailed		(ModMemoryHandle::negotiateToOther)		
//		全てのメモリハンドルとのネゴシエーションに失敗

void*
ModMemoryHandle::allocateMemory(ModSize size)
{
	if (!_MemoryHandle::_selfMemoryManagement)
		return ModOsDriver::Memory::alloc(size);

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	if (size > this->limitSize - this->currentSize) {

		// 現在確保可能なサイズよりも大きな領域を確保しようとしている
		// この場合、現在自分が確保中の領域用の削減交渉関数は存在しないので、
		// そのまま例外を発生する

		ModThrow(ModModuleMemory,
				 ModMemoryErrorHandleLimit, ModErrorLevelError);
	}

    void	*address;
	try {
		address = ModMemoryPool::allocateMemoryUnsafe(size);
		this->currentSize += size;

	} catch (ModException& exception) {

		switch (exception.getErrorNumber()) {
		case ModCommonErrorBadArgument:
		case ModMemoryErrorOverPoolLimit:

			// 領域削減交渉するわけに行かない

			ModRethrow(exception);
		}

		// メモリープールから領域を確保できないので、
		// 領域削減交渉を開始する

		// エラー処理中に領域確保をしようとして、
		// できなかったかどうかを調べる
		//
		//【注意】	メモリープールからの領域確保に失敗しても
		//			エラー状態にならない

		ModBoolean	errored = ModErrorHandle::isError();

		try {
			ModNegotiatingThread*	thread =
				ModMemoryHandle::isNegotiating();
			if (thread) {

				// 領域削減交渉中の領域確保に失敗したので、
				// 非常用領域から領域を確保する

				address = ModMemoryHandle::allocateEmergencyMemory(size);

				// 非常用領域から確保されたサイズを記録しておく

				thread->usingMemory += size;
			} else {

				// 優先度の高いほうから領域削減交渉関数を呼び出し、
				// 必要なサイズ以上破棄された時点で再度領域確保を試みる

				address = this->negotiateToOther(size);

				// 通常の領域から確保されたサイズを記録しておく

				this->currentSize += size;
			}
		} catch (ModException& exception) {

			switch (exception.getErrorNumber()) {
			case ModOsErrorSystemMemoryExhaust:
			case ModMemoryErrorNegotiateFailed:
				if (errored) {

					// エラー処理中に実際の領域確保や領域削減交渉に失敗したら、
					// 非常用領域から領域を確保する
					//
					//【注意】	このとき、非常用領域から確保された
					//			サイズを記録しない

					address = ModMemoryHandle::allocateEmergencyMemory(size);
					break;
				}
				// thru
			default:
				ModRethrow(exception);
			}
		}
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrow(ModModuleMemory);
	}
#endif
#ifdef DEBUG
	if (ModMemoryHandle::useDebugList)
		try {
			// 確保した領域を管理リストへ追加する

			ModMemoryDebugCell*	cell = new ModMemoryDebugCell(address, size);
			this->debugList.insert(cell);

			if (this->allocateCheck) {
				ModDebugMessage << "Allocate Check:" << ModEndl;
				cell->print();
				ModDebugMessage << ModEndl;
			}
		} catch (ModException& exception) {
			ModRethrow(exception);
		} catch (...) {
			ModUnexpectedThrow(ModModuleMemory);
		}

	if (address == ModAllocateMemoryWatchAddress)
		ModAllocateMemoryWatchPoint();
#endif
	return address;
}

// FUNCTION public
// ModMemoryHandle::freeMemory  -- メモリハンドルから確保したメモリを解放
//
// NOTES
// メモリハンドルから確保したメモリを解放する。
// 内部的にはメモリプールに対してメモリの使用済みを知らせる。
//
// ARGUMENTS
// void* address
//		解放するメモリの先頭アドレス
// ModSize size
//		確保したサイズ
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModMemoryErrorWrongSize
//		アドレスとサイズが合っていない
//	ModMemoryErrorFreeUnAllocated
//		獲得していない領域を解放しようとした
// 	その他
//		下位モジュールの例外はそのまま送出

void
ModMemoryHandle::freeMemory(void* address, ModSize size)
{
	if (address) {
		if (!_MemoryHandle::_selfMemoryManagement) {
			ModOsDriver::Memory::free(address);
			return;
		}

		// スレッド間排他制御用ロックをかける

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		try {
#ifdef DEBUG
			if (ModMemoryHandle::useDebugList) {

				// 以下の検査を行う
				//
				// * 現在確保している領域の先頭アドレスが渡されているか
				// * 破棄しようとしている領域のサイズが確保時のサイズと等しいか

				// 以下のように書かずに低レベルのものを使う
				//
				// ModSinglyLinkedListIterator<ModMemoryDebugCell>
				//									item(this->debugList);

				ModBoolean	notFound = ModTrue;
				ModSinglyLinkedListIteratorBase item(this->debugList);
				for (; *item != 0; ++item) {
					ModMemoryDebugCell*	cell = (ModMemoryDebugCell*) *item;
					if (cell->address == address) {

						// 破棄する領域の先頭アドレスは、
						// 現在確保している領域の先頭アドレスである

						if (cell->size != size) {

							// 与えられたサイズが確保持のサイズと異なる

							ModThrow(ModModuleMemory, ModMemoryErrorWrongSize,
									 ModErrorLevelError);
						}

						// 管理リストから破棄する領域の情報を破棄する

						this->debugList.expunge(cell);
						delete cell;

						notFound = ModFalse;
						break;
					}
				}
				if (notFound) {

					// 与えられたアドレスは
					// 現在確保している領域の先頭アドレスでない

					ModThrow(ModModuleMemory, ModMemoryErrorFreeUnAllocated,
							 ModErrorLevelError);
				}
			}
#endif	// DEBUG
			if (ModMemoryPool::isEmergencyMemory(address)) {

				// 破棄しようとしている領域は非常用領域である

				ModMemoryHandle::freeEmergencyMemory(address, size);

				// 領域削減交渉中に確保された非常用領域のサイズは記録されている

				ModNegotiatingThread*	thread =
					ModMemoryHandle::isNegotiating();
				if (thread)
					thread->usingMemory -= size;
			} else

				// 破棄しようとしている領域は通常の領域である

				this->freePoolMemory(address, size);

		} catch (ModException& exception) {
			ModRethrow(exception);
		}
#ifndef NO_CATCH_ALL
		catch (...) {
			ModUnexpectedThrow(ModModuleMemory);
		}
#endif
	}
}

#ifdef DEBUG
// FUNCTION
// ModMemoryHandle::freeMemory  -- メモリハンドルから確保したメモリをサイズなしに解放
//
// NOTES
// デバッグ版のみ。
// サイズを指定することなく、メモリハンドルから確保したメモリを解放する。
// 内部的にはメモリプールに対してメモリの使用済みを知らせる。
// STLのDEBUG時のみのアロケータとして用い、メモリエラーをみつけるのが
// 目的であるが今回は使われないだろう。
//
// ARGUMENTS
// void* address
//		解放するメモリの先頭アドレス
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModMemoryErrorFreeUnAllocated
//		獲得していない領域を解放しようとした
// 	その他
//		下位モジュールの例外はそのまま送出
//

void	
ModMemoryHandle::freeMemory(void* address)
{
	if (address) {
		if (!_MemoryHandle::_selfMemoryManagement) {
			ModOsDriver::Memory::free(address);
			return;
		}

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		if (ModMemoryHandle::useDebugList) {
			ModBoolean found = ModFalse;
			ModSize size;
			ModSinglyLinkedListIteratorBase item(this->debugList);
		
			for (; *item != 0; ++item) {
				ModMemoryDebugCell* memory = (ModMemoryDebugCell*) *item;
				if (memory->address == address) {
					size = memory->size;
					found = ModTrue;
					// みつかったセルを消す
					// このへんでも例外があり得る
					this->debugList.expunge(memory);
					delete memory;
					break;
				}
			}
			if (!found) {
				ModThrow(ModModuleMemory, ModMemoryErrorFreeUnAllocated,
						 ModErrorLevelError);
				return;
			}
			this->freePoolMemory(address, size);
		} else {
			ModThrow(ModModuleMemory, ModCommonErrorNotSupported,
					 ModErrorLevelFatal);
		}
	}
}
#endif	// DEBUG

// 
// FUNCTION
// ModMemoryHandle::moveMemoryHandle  -- 属しているメモリハンドルの引越し
//
// NOTES
//
// ARGUMENTS
// void* address
//		引っ越すメモリの先頭アドレス
// ModSize size
//		サイズ
// ModMemoryHandle* 
//		引越し先のメモリハンドル
//
// RETURN
// なし
//
// EXCEPTIONS
// ModmemoryErrorHandleLimit
//		メモリハンドルの限界値を超える
//	ModMemoryErrorWrongSize
//		アドレスとサイズが合っていない
//	ModMemoryErrorFreeUnAllocated
//		獲得していない領域を対象としている
//	その他
//		下位モジュールの例外はそのまま送出
//
void
ModMemoryHandle::moveMemoryHandle(void* address, ModSize size,
								   ModMemoryHandle* destination)
{
	if (_MemoryHandle::_selfMemoryManagement) {

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		// 先に引越し先のサイズをチェック
		if (destination->currentSize + size > destination->limitSize) {
			ModThrow(ModModuleMemory, ModMemoryErrorHandleLimit,
					 ModErrorLevelError);
		}
#ifdef DEBUG	
		if (ModMemoryHandle::useDebugList) {
			// this から取り出す。
			ModSinglyLinkedListIteratorBase item(this->debugList);
			ModMemoryDebugCell* cell;
			ModBoolean found = ModFalse;
			// デバッグリストの引越し
			for (; *item != 0; ++item) {
				cell = (ModMemoryDebugCell*)*item;
				if (cell->address == address) {
					if (cell->size != size) {
						ModThrow(ModModuleMemory, ModMemoryErrorWrongSize,
								 ModErrorLevelError);
					}
					found = ModTrue;
					this->debugList.expunge(cell);
					break;
				}
			}
			if (found == ModFalse) {
				// 該当するものがない
				ModThrow(ModModuleMemory, ModMemoryErrorFreeUnAllocated,
						 ModErrorLevelError);
			}
			// destinationへ引越し。
			destination->debugList.insert(cell);
		}
#endif
		// サイズの計算
		this->currentSize -= size;
		destination->currentSize += size;
	}
}

// 
// FUNCTION private
// ModMemoryHandle::negotiateToOther -- 順にネゴシエーション関数を呼び出す
//
// NOTES
// 優先度の値の大きい順に、登録されているネゴシエーション関数を呼び出す。
// 確保できた段階で、確保したメモリを返す。確保できなければ例外が送出される。
// ロックしたまま呼び出されることが前提。
//
// ARGUMENTS
// ModSize size
//		要求サイズ
//
// RETURN
// 確保したメモリの先頭アドレスを返す
//
// EXCEPTIONS
//	ModMemoryErrorNegotiateFailed
//		全てのネゴシエーションに失敗
//	その他
//		ModOsDriver::Mutex::lock, unlockAllの例外参照。その他は以下に示す。
//	ModOsErrorSystemMemoryExhaust		(::newによる)
//		システムメモリが不足		
//	ModMemoryErrorNotFreeEmergencyArea	(ModMemoryHandle::negotiateToOther)
//		ネゴシエーションが終わっても非常用で獲得した領域を解放していない
//
void*
ModMemoryHandle::negotiateToOther(ModSize size)
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	// 既にロック済み。この状態で返す。
	int lockCount = ModCommonMutex::getLockCount();

	// まずnego中のスレッドをリストに覚えておく。
	// negoの処理のためにallocateが必要な場合に堂々巡りになるのを
	// 避ける。
	ModNegotiatingThread* thread = new ModNegotiatingThread();
	ModMemoryHandle::negotiatingThreadList->insert(thread);

	// 優先度0は無視して、1まで。
	// 優先度の高い方からサイズ分持っているハンドルに縮小要求
	// (自分も含む)
	
	// リングバッファを一周するまで実行する。

	// どの関数から始めるべきか。始めたエントリを覚えておき、
	// 一周するか、あるいはメモリがとれるまで実行する。
	// 
    ModMemoryNegotiate* negotiateEntry = ModMemoryNegotiate::getStartPoint();
	ModMemoryNegotiate* begin = negotiateEntry;
	if (negotiateEntry == 0) {
		// ひとつもない
		ModAssert(ModMemoryNegotiate::negotiateListing->isEmpty() == ModTrue);
		goto Error;
	}

	do {
		if (negotiateEntry->isAvailable() == ModFalse) {
			// つぎへ。
			negotiateEntry = negotiateEntry->getNext();
			continue;
		}
#ifdef MOD64
		ModDebugMessage << "Call Nego for " << ModHex 
						<< (ModUInt64)negotiateEntry << ModEndl;
#else
		ModDebugMessage << "Call Nego for " << ModHex 
						<< (unsigned int)negotiateEntry << ModEndl;
#endif
		try {
			// 必要最小限だけ、要求を出す
			// デッドロックを避けるため、
			// ロックを一時的に全面解除
			m.unlockAll();
			negotiateEntry->callNegotiate(size);

			// 非常用メモリのチェック
			// negoが終ったら非常用メモリが返されていることをチェック
			// スレッドごと。同じスレッドが同時にネゴすることは決してない
			if (thread->getUsingMemory() > 0) {
				ModThrow(ModModuleMemory, ModMemoryErrorNotFreeEmergencyArea,
						 ModErrorLevelError);
			}

			(void) m.lock(lockCount);	

#ifdef MOD64
			ModDebugMessage << "Call Nego Success " << ModHex
							<< (ModUInt64)negotiateEntry << ModEndl;
#else
			ModDebugMessage << "Call Nego Success " << ModHex
							<< (unsigned int)negotiateEntry << ModEndl;
#endif
			// ここで直接とってみる
			// だめなら次のネゴに挑戦する
			void *address = ModMemoryPool::allocateMemoryUnsafe(size);
			// 縮小分とれたら終了

			// ネゴ中を終了
			ModMemoryHandle::negotiatingThreadList->expunge(thread);
			delete thread;

			return address;

		} catch (ModException& exception) {
			// negotiation error
			// 何ごともなかったように次の候補にいくため
			// 一応キャッチする。

			// lock回数はここで元に戻しておく
			if (ModCommonMutex::getLockCount() == 0) {
				(void) m.lock(lockCount);
			}
			if (exception.getErrorNumber() == ModMemoryErrorNotFreeEmergencyArea) {
				// この場合だけは次にいかずにスローしたい
				ModMemoryHandle::negotiatingThreadList->expunge(thread);
				delete thread;
				ModRethrow(exception);
			}
		} catch (...) {
			if (ModCommonMutex::getLockCount() == 0) {
				(void) m.lock(lockCount);
			}
			ModMemoryHandle::negotiatingThreadList->expunge(thread);
			delete thread;
			ModUnexpectedThrow(ModModuleMemory);
		}
		negotiateEntry = negotiateEntry->getNext();
	} while (negotiateEntry != begin);
Error:
	ModMemoryHandle::negotiatingThreadList->expunge(thread);
	delete thread;
	// すべての優先度のいずれのネゴシエーション関数を試してもだめだった
	// 場合にここにくる。

	ModThrow(ModModuleMemory, ModMemoryErrorNegotiateFailed, 
			 ModErrorLevelError);
	return 0;	// 絶対こない
}

#ifdef DEBUG
// FUNCTION private
// ModMemoryHandle::checkMemory -- デバッグ用アロックリストのチェック
//
// NOTES
// デバッグ用のリストをチェックし、最後のメモリーリークのチェックをする。
// エラーがある場合には、エラーメッセージを出力する。
// メモリハンドルの後処理で呼び出す。
// MOD_DEBUGが立っていない場合には、リストに何も登録されていないので
// 何も出力されない。
// 本メモリハンドルをロックしてから呼び出すことが前提である。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし

void
ModMemoryHandle::checkMemory()
{
	ModSize size = this->debugList.getSize();
    if (size > 0) {
		ModDebugMessage << "ALLOCLIST size = " << size << ModEndl;		
		//ModDebugMessage << "following memory leak detected" << ModEndl;
		ModSinglyLinkedListIteratorBase item(this->debugList);
		for ( ; *item != 0; ++item) {
			((ModMemoryDebugCell*) *item)->print();

			// 残したままにしておく。
			// 呼ぶならリストから消し、セルを消去し、freePoolMemory()を呼ぶ
		}
		ModDebugMessage << ModEndl;
	}
}
#endif

// 
// FUNCTION public
// ModMemoryHandle::allocateEmergencyMemory -- 非常用メモリの獲得
//
// NOTES
//	メモリプールから非常用メモリを獲得して返すための、メモリハンドルからの
//	I/F。
//	非常用メモリは、ネゴ中のメモリ獲得、あるいはメモリが不足時のエラー処理
//	で用いられる。そのため、非常用メモリでは、メモリハンドルごとの
//	限界サイズチェックは行われていない。
//	現在は、メモリフリーの時のサイズチェックのためのデバッグ用のセルも
//	用意していない。
//
// ARGUMENTS
// ModSize size
//	   要求サイズ
//
// RETURN
//	獲得したメモリのアドレス
//
// EXCEPTIONS
//	その他
//		ModMemoryPool::allocateEmergencyMemoryの例外参照(主なものは以下に書き下す)
//	ModCommonErrorBadArgument		(ModMemoryPool::allocateEmergencyMemory)
//		引数エラー(0である)
//	ModMemoryErrorEmergencyLimit	(ModMemoryPool::allocateEmergencyMemory)
//		非常用メモリの制限を超えている
//

void*
ModMemoryHandle::allocateEmergencyMemory(ModSize size)
{
	// 現在は非常用メモリについて
	// サイズ、アドレスとも何もチェックしていない。(セルを作っていない)
	// 作るならば、
	// いずれは非常用のデバッグ用リストの作成はハンドルごとでなく、
	// 全体で管理し、スレッドIDも格納しておくとよい。ネゴ中のスレッドが
	// 非常用メモリを返却したかどうかをそれでチェックできる
	//

	return ModMemoryPool::allocateEmergencyMemory(size);
}

// 
// FUNCTION public
// ModMemoryHandle::allocateEmergencyMemory -- 非常用メモリの解放
//
// NOTES
//	非常用メモリを解放するためのメモリハンドルからのI/F。
//	非常用メモリは、ネゴ中のメモリ獲得、あるいはメモリが不足時のエラー処理
//	で用いられる。そのため、非常用メモリでは、メモリハンドルごとの
//	限界サイズチェックは行われていない。
//	また、メモリフリーの時のサイズチェックのためのデバッグ用のセルも
//	用意していないのでサイズチェックやアドレスのチェックは行われない。
//
// ARGUMENTS
// void* address
//		解放する非常用メモリのアドレス
// ModSize size
//	   要求サイズ
//
// RETURN
//	獲得したメモリのアドレス
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//
void 
ModMemoryHandle::freeEmergencyMemory(void* address, ModSize size)
{
	ModMemoryPool::freeEmergencyMemory(address, size);
}

// 
// FUNCTION public
// ModMemoryHandle::printHandle -- メモリハンドルの内容出力
//
// NOTES
// メモリハンドルの内部の値をデバッグメッセージとして出力する。
// デバッグ時にはメソッドcheckMemoryによるメモリーリークチェックも行われる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModOsMutex::lock, unlockの例外参照
//
void
ModMemoryHandle::printHandle()
{
	ModCommonMutex::lock();
    ModDebugMessage << "LIMIT    " << this->limitSize << ModEndl;
    ModDebugMessage << "CURRENT  " << this->currentSize << ModEndl;
#ifdef DEBUG
    // デバッグ用アロックリストの出力

    this->checkMemory();
#endif
	ModCommonMutex::unlock();
}

//
// FUNCTION public
// ModMemoryHandle::print -- デバッグ用出力関数
//
// NOTES
//	デバッグ用に、メモリハンドルの内容を出力する。
//	モジュール名の出力用に純粋仮想関数を呼び出す。仮想関数なので、
//	他の情報を出力する場合には適当に定義しなおせばそちらが呼び出される。
//	
// ARGUMENTS
//	なし
//
// RETURN
// なし
// 
// EXCEPTIONS
//	その他
//		ModOsMutex::lock, unlockの例外参照
//
void
ModMemoryHandle::print()
{
	ModDebugMessage << "NAME     " << this->getName() << ModEndl;
	this->printHandle();
}

//
// FUNCTION public
// ModMemoryHandle::setNegotiate -- メモリの解放を試みる関数を登録する
//
// NOTES
//	ネゴシエーション関数を登録する。ネゴシエーション関数は、allocate()による
//	メモリ獲得時、メモリプールからメモリが獲得できない場合に、
//	呼び出される。
//	ネゴシエーション関数では、引数に指定されたサイズ以上のメモリの解放を行う
//	ことが求められる。解放できなかった場合には例外を送出すること。
//	処理の都合上、メモリ解放の前に別のメモリの確保が必要な場合は、自動的に
//	非常用メモリが用いられるためその要求は最小限にとどめ、
//	なおかつネゴシエーション関数が終了する前に解放しなければならない。
//	ネゴシエーション関数の呼び出しはメモリ管理用のロックをはずした状態で行われ、
//	同時に複数のスレッドから呼び出される可能性もあるので
// 	MT-safeに実装しなければならない。
//
//	ネゴシエーション関数登録は、MOD全体に対して行われ、その呼び出しは指定した
//	優先度の値が大きい順になる。
//	同じ関数を二重に登録することはできない。
//	
// ARGUMENTS
//	void (*negotiate)(ModSize size)
//		ネゴシエーション関数
//	Priority priority
//		関数に与える優先度
//
// RETURN
// なし
// 
// EXCEPTIONS
//	ModMemoryErrorNegotiateRegistered
//		同じ関数が既に登録済み
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//	ModOsErrorSystemMemoryExhaust		(::newによる)
//		システムメモリが不足		
//
void
ModMemoryHandle::setNegotiate(void (*negotiate)(ModSize size), 
								 Priority priority)
{
	ModMemoryNegotiate* negotiateEntry;
	// 初期化チェック
	// 初期化中のエラーは文字列が送出されるかも。
	ModCommonInitialize::checkAndInitialize();

	// リストをロック
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	// 二重登録チェック
	ModRingLinkedEntry* item = ModMemoryNegotiate::negotiateListing->begin();
	ModRingLinkedEntry* begin = item;
	if (begin != 0) {
		do {
			negotiateEntry = (ModMemoryNegotiate*)item;
			if (negotiateEntry->negotiate == negotiate
				&& negotiateEntry->isAvailable() == ModTrue) {
				// 有効でないものは除く
				// あった
				ModThrow(ModModuleMemory, ModMemoryErrorNegotiateRegistered, 
						 ModErrorLevelError);
			}
			item = negotiateEntry->getNext();
		} while (begin != item);
	}

	// なかった
	negotiateEntry = new ModMemoryNegotiate(negotiate, priority);
	ModMemoryNegotiate::negotiateListing->insert(negotiateEntry);
}

//
// FUNCTION public
// ModMemoryHandle::cancelNegotiate -- ネゴシエーション関数の登録を抹消する
//
// NOTES
//	登録されているネゴシエーション関数を登録抹消する。
//	実際にはリングバッファの該当エントリの有効フラグをModFalseにする。
//	該当するネゴシエーション関数が呼び出されている間に登録抹消された場合は、
//	(ネゴシエーション関数はロックなしで呼び出されるので十分ありえる)
//	呼び出しが終了した段階で関数登録が無効となる。
//	
// ARGUMENTS
//	void (*negotiate)(ModSize size)
//		登録抹消するネゴシエーション関数
//
// RETURN
// なし
// 
// EXCEPTIONS
//	ModCommonErrorEntryNotFound
//		指定したネゴシエーション関数は登録されていない
//	その他
//		ModOsDriver::Mutex::lock, unlockの例外参照
//
void
ModMemoryHandle::cancelNegotiate(void (*negotiate)(ModSize size))
{
	//
	// 本当は、呼び出し中の時のキャンセルは避けたい。
	// ただしロックがかかっていないため、不可能。
	//

	ModMemoryNegotiate* negotiateEntry = 0;
	// リストをロック
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	(void) m.lock();

	ModRingLinkedEntry* item = ModMemoryNegotiate::negotiateListing->begin();
	ModRingLinkedEntry* begin = item;
	if (begin == 0) {
		// みつからなかった
		ModThrow(ModModuleMemory, ModCommonErrorEntryNotFound, 
				 ModErrorLevelError);
	}
	ModMemoryNegotiate* candidEntry = 0;
	do {
		candidEntry = (ModMemoryNegotiate*)item;
	    if (candidEntry->negotiate == negotiate
			&& candidEntry->isAvailable() == ModTrue) {
			// 有効でないものは除く
			negotiateEntry = candidEntry;
			break;
	    }
		item = candidEntry->getNext();
	} while (begin != item);

	if (negotiateEntry == 0) {
		// みつからなかった
		ModThrow(ModModuleMemory, ModCommonErrorEntryNotFound, 
				 ModErrorLevelError);
	} else {
		// ★注意
		// negoしている間はロックがかかっていないので
		// その間にもキャンセルされる可能性がある。
		// 呼び出されている方の問題は登録関数の方で対応してもらう。

		// キャンセルはされるものの、リストの続きはたどりたい。
		// スレッド固有の番号をnegotiateEntryにつけていけば、
		// 同じマークがなくなったところから次をたどり始めて
		// 同じマークにあたったらおわればよい。
		// ただ、negotiateEntryの数はたいして多くないだろうし、動的に
		// 変化することも少ないと予想されるので、
		// そのコーディングの手間を省き、登録抹消はそれを意味する
		// マークをつけるだけとする。リストから本当に削除しない。
		// ModMemoryNegotiate::negotiateListing->expunge(negotiateEntry);
		negotiateEntry->available = ModFalse;
    }
}

// 
// FUNCTION public
// ModMemoryHandle::printAllList -- メモリハンドルのリストの出力(static)
//
// NOTES
//	登録されている全メモリハンドルの優先度別のリストの内容を
//	デバッグ用に出力する。staticメソッドである。
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
void
ModMemoryHandle::printAllList()
{
	try {
		// 初期化チェック
		// 初期化中のエラーは文字列が送出されるかも。
		ModCommonInitialize::checkAndInitialize();

		// 全リストのロック
		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		(void) m.lock();

		if (ModMemoryHandle::allListing == 0) {
			// このときはModCommonInitialize::isInitialized()がFalseを
			// 返すはずである。文脈でわかりやすいのでこちらを採用
			// している。
			ModDebugMessage << "memoryHandleList is already terminated."
							<< ModEndl;
			return;
		}
		ModSize size = ModMemoryHandle::allListing->getSize();
		if (size > 0) {
			ModDebugMessage << "ModMemoryHandle::allListing LENGTH : " 
							<< size << "\n" << ModEndl;
			ModDebugMessage << "following memoryHandle remained" << ModEndl;

			ModSinglyLinkedListIteratorBase item(*ModMemoryHandle::allListing);
			for (; *item != 0; ++item)
				((ModMemoryHandle*) *item)->print();

			ModDebugMessage << ModEndl;
		}
	} catch (...) {

		// 例外が発生しても無視する

		ModErrorHandle::reset();
	}
}

//
// FUNCTION private
// ModMemoryHandle::isNegotiating -- ネゴシエーション中のスレッドオブジェクトを返す
//
// NOTES
//	実行スレッドが、ネゴシエーション中かどうかを返す。
//	ミューテックスはあらかじめロックして呼び出すことが前提。
//	ネゴ中の場合はそのスレッドを表すオブジェクトを返す。
//	そうでない場合は0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	実行スレッドがネゴ中ならば、スレッドオブジェクトを返す。そうでなければ0を返す
// 
// EXCEPTIONS
//	なし
//
ModNegotiatingThread*
ModMemoryHandle::isNegotiating()
{
	ModThreadId self = ModThisThread::self();
	ModNegotiatingThread* thread = 0;
	ModSinglyLinkedListIteratorBase 
		item(*ModMemoryHandle::negotiatingThreadList);
	for (; *item != 0; ++item) {
		thread = (ModNegotiatingThread*)*item;
		if (thread->getThreadId() == self) {
			return thread;
		}
	}
	// nego中ではない
	return 0;
}

#ifdef DEBUG
// ** 以下はModMemoryDebugCell のメソッド。デバッグ時のみ利用される。

// 
// FUNCTION public
// ModMemoryDebugCell::ModMemoryDebugCell -- メモリデバッグ用セルのコンストラクタ
//
// NOTES
// 確保したメモリの先頭アドレスとサイズを記録するためのセルのコンストラクタ。
//
// ARGUMENTS
// void* address_
//	メモリの先頭アドレス
// ModSize memorySize
//	メモリのサイズ
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
ModMemoryDebugCell::ModMemoryDebugCell(void* address_, ModSize memorySize)
	:address(address_), size(memorySize)
{
}

// 
// FUNCTION public
// ModMemoryDebugCell::~ModMemoryDebugCell -- メモリデバッグ用セルのデストラクタ
//
// NOTES
// 確保したメモリの先頭アドレスとサイズを記録するためのセルのデストラクタ。
// 特に何もしない。
//
// ARGUMENTS
//	なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
ModMemoryDebugCell::~ModMemoryDebugCell()
{
    // とくになし
}

//
// FUNCTION
// ModMemoryDebugCell::operator new -- メモリデバッグセルのnew
//
// NOTES
// メモリデバッグセルはMODの中でも特別な扱いを受ける。
// この他で確保されたメモリはすべてどれかのメモリハンドルに管理されるが、
// メモリデバッグセル自身を確保するためのメモリはメモリプールから直接獲得
// され、どのメモリハンドルの管理下にも入らない。そうでないと、自分自身を
// 自分自身が管理することになり、永久ループに陥る。
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
//

void*
ModMemoryDebugCell::operator new(size_t size, size_t dummy)
{
	// 領域の確保に失敗したときは、非常用領域から確保しない

	return ModMemoryPool::allocateMemory((ModSize)size);
}

//
// FUNCTION
// ModMemoryDebugCell::operator delete -- メモリデバッグセルのdelete
//
// NOTES
// メモリデバッグセルはMODの中でも特別な扱いを受ける。
// メモリデバッグセルのnewで確保されたメモリ(メモリプールから直接
// 確保されている)を解放する。
//
// ARGUMENTS
// void* address
//		解放するメモリの先頭アドレス
// ModSize size
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
ModMemoryDebugCell::operator delete(void* address, size_t size)
#ifdef STD_CPP11
noexcept(false)
#endif
{
	ModMemoryPool::freeMemory(address, (ModSize)size);
}

// 
// FUNCTION public
// ModMemoryDebugCell::print -- メモリデバッグ用セルの内容を出力する
//
// NOTES
// メモリデバッグ用セルの内容をデバッグ用に出力する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModMemoryDebugCell::print() const
{
    ModDebugMessage << "address: 0x" << ModSetW(8) 
					<< ModSetFill('0') << ModHex
#ifdef MOD64
					<< (ModUInt64)this->address
#else
					<< (unsigned int)this->address
#endif
					<< ModEndl;
		
    ModDebugMessage << "size:    " << ModDec << this->size << ModEndl;
}
#endif	// DEBUG

//
// Copyright (c) 1997, 2011, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
