// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMemoryHandle.h -- メモリーハンドル関連のクラス定義
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

#ifndef	__ModMemoryHandle_H__
#define __ModMemoryHandle_H__

#include "ModCommonDLL.h"
#include "ModLinkedList.h"
#include "ModMemoryPool.h"
#include "ModCommonException.h"

//
// モジュールはメモリモジュールに属する。
// したがって、エラーはModMemoryXXXである。
// 普通ならばModDefaultObjectのサブクラスとして作成するべきところだが、
// メモリハンドル、MemoryDebugCellは自分で自分を管理することとなり、
// リカーシブとなるのでメモリ管理モジュールとして特殊。よって
// そのようにはせずにじかにメモリプールからとる。
//

class ModMemoryHandle;

//
// CLASS
//	ModNegotiatingThread -- ネゴシエーション中のスレッド情報
//
// NOTES
//	ネゴシエーション中にメモリ獲得される場合には、またもやネゴシエーションが
//	呼び出されて永久ループとならないように
//	非常用メモリを返すことになっている。そのため、ネゴシエーション中の
//	スレッドはすべて、本クラスのスレッド情報のリストとして覚えておく必要がある。
//	スレッド情報には、スレッドIDと使用中の非常用メモリサイズがある。
//	非常用メモリは、そのネゴシエーション関数内部で解放しなければならない。
//

class ModCommonDLL ModNegotiatingThread
	: public	ModSinglyLinkedEntry
{
	friend class ModMemoryHandle;
public:
	// 本来はnew/deleteをspecialに定義すべきかも。
	ModNegotiatingThread();
#ifdef STD_CPP11
	virtual ~ModNegotiatingThread() noexcept(false);
#else
	virtual ~ModNegotiatingThread();
#endif
	ModThreadId getThreadId() const {return this->threadId; }
	ModSize getUsingMemory() const { return this->usingMemory; }

	void* operator new(size_t size, size_t dummy = 0);	// special
#ifdef STD_CPP11
	void  operator delete(void* address, size_t size) noexcept(false);	// special
	void  operator delete(void* address) noexcept(false);	// ダミー
#else
	void  operator delete(void* address, size_t size);		// special
#endif
private:
	ModThreadId threadId;
	ModSize usingMemory;
};

#ifdef DEBUG
// CLASS
// ModMemoryDebugCell -- メモリの先頭アドレスとサイズを記録するメモリデバッグ用セルクラス
//
// NOTES
// 確保したメモリの先頭アドレスとサイズを組にして記録するクラスである。
// デバッグ時に限り、メモリハンドルごとに獲得したメモリをチェックするため、
// 本クラスにアドレスとサイズを格納してリストを作成する。
// 獲得したソースファイルと行も格納する予定であったが、すべて該当モジュールの
// メモリハンドルのallocateMemoryであることが予想されるため、削除した。

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

class ModMemoryDebugCell
	: public	ModSinglyLinkedEntry
{
    friend class ModMemoryHandle;
public:
    ModMemoryDebugCell(void* address_, ModSize size);
    ~ModMemoryDebugCell();

	void* operator new(size_t size, size_t dummy = 0);	// special
#ifdef STD_CPP11
	void  operator delete(void* address, size_t size) noexcept(false);	// special
#else
	void  operator delete(void* address, size_t size);		// special
#endif

    void print() const;
private:
    void* address;
    ModSize size;

    // ソースファイルと行
	// 実際にはメモリハンドルのnewで呼び出されるので、メモリハンドルの
	// 特定しかできない。
    //char* source;
    //ModSize line;
};
#endif

// CLASS
// ModMemoryHandle -- メモリハンドルのクラス定義
// NOTES
//	メモリハンドル自身を確保するためのメモリとして、
//	メモリハンドルは独自に new, delete を定義する。このメモリは
//	メモリプールから直接獲得され、どのメモリハンドルの管理下にも入らない。
//	作成されたModMemoryHandleは、全モジュールのメモリハンドルを管理する
//	リストで管理される。
// 
//	メモリハンドルを親クラスとして活用させた方がメリットが多いので、
//	すべてのメモリハンドルはModMemoryHandleのサブクラスとして実装するように
//	変更した。これによりメモリ管理戦略テンプレートクラスModMemoryConfiguration
//	は廃止。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMemoryHandle
	: public	ModSinglyLinkedEntry
{
public:
	friend class ModCommonInitialize;
	//
	// ENUM
	// Priority -- 登録するネゴシエーション関数に設定する優先度を示す列挙型
	// NOTES
	//	ネゴシエーション関数に与えられる優先度、つまり
	//	ネゴシエーション関数が解放するメモリの優先度を示す。
	//	値が小さいほど優先度が高く、大きいほど呼び出されやすいことを表す。
	//	ネゴシエーション関数は当初大きい順に呼び出されるが、順番はどんどん
	//	回っていくのであまり深い意味はない。
	//	以前、用意していたpriority0(絶対搾取されない、呼び出されない)は廃止
	enum Priority {
		priority1 = 0,
		priority2 = 1,
		priority3 = 2,
		priorityNumber	// メモリ割り当て優先度の種類の数
	};

	void* operator new(size_t size, size_t dummy = 0);	// special
#ifdef STD_CPP11
	void  operator delete(void* address, size_t size) noexcept(false);	// special
	void  operator delete(void* address) noexcept(false);	// ダミー
#else
	void  operator delete(void* address, size_t size);		// special
#endif

    // コンストラクタ、デストラクタ
	ModCommonDLL
	ModMemoryHandle();
	ModCommonDLL
	ModMemoryHandle(ModSize limitSize_);
	ModCommonDLL
#ifdef STD_CPP11
	virtual ~ModMemoryHandle() noexcept(false);
#else
	virtual ~ModMemoryHandle();
#endif

    // メモリの獲得、解放
    // メモリハンドルごとのサイズの計算、メモリプールに対する操作
	ModCommonDLL
	void* allocateMemory(ModSize size);
	ModCommonDLL
	void freeMemory(void *address, ModSize size);
#ifdef DEBUG
    // デバッグ用
	// デバッグリストの情報をもとに、サイズ引数なしにフリーできる
	ModCommonDLL
    void freeMemory(void *address);
    // void* allocateMemory(ModSize size, char* source, ModSize line);
	// ソースと行数は結局メモリハンドルのallocateの中なので、
	// メモリハンドルさえ特定できれば効果は薄い。よってやめた
#endif
	// 属するメモリハンドルの引越し
	ModCommonDLL
	void moveMemoryHandle(void* address, ModSize size, 
						  ModMemoryHandle* destination);
	// アクセサ
	ModSize getCurrentSize() const;

	// メモリハンドルとしての総量の上限を返す
	ModSize getLimitSize() const;

	// マネージャ名を返す。
	virtual const char* getName() const = 0;
	// 本来はこれを純粋仮想関数にしないで、名前クラスはstaticで
	// 親クラスにもっておいて、一回目で名前をセットするようにしてもらえば
	// いいのだろう。各ハンドルで違う名前をもつことは考えにくいから
	// staticにこだわらなくてもいいのかも。そうすればインスタンス化時に
	// 名前をセットするだけで済む。

	// メモリの解放を試みる関数を登録する
	ModCommonDLL
	static void setNegotiate(void (*negotiate)(ModSize size), 
							 Priority priority);
	// 登録を抹消する
	ModCommonDLL
	static void cancelNegotiate(void (*negotiate)(ModSize size));

	ModCommonDLL
	static ModNegotiatingThread* isNegotiating();

    // デバッグ用
	ModCommonDLL
    virtual void print();		// サブクラスの内容
	ModCommonDLL
	void printHandle();			// 基本クラスの内容
	ModCommonDLL
	static void printAllList();

	// ModTrueにセットすると領域獲得時にアドレスとサイズを出力(DEBUGのみ)
	void					setAllocateCheck(ModBoolean check);

	ModCommonDLL
	void printMemoryLeak();	// サブクラスのデストラクタでの呼び出しを推奨

	static ModBoolean		isMemoryLeakChecked();

private:
	// リストの初期化、後処理(ModCommonInitializeからだけ呼び出せる)
	static void initialize();
	static void terminate();
    // 他のメモリハンドルにネゴを依頼
    void* negotiateToOther(ModSize size);
	// メモリプールの解放とサイズの減算
    void freePoolMemory(void* address, ModSize size);
#ifdef DEBUG
    // デバッグ時のみリストをチェックする(フリーはせず、残したままにする)
    void checkMemory();
#endif
	// 非常用メモリを獲得する[エラー時、自スレッドnego中]
	static void* allocateEmergencyMemory(ModSize size);
	static void freeEmergencyMemory(void* address, ModSize size);

    ModSize	limitSize;

    ModSize	currentSize;

	static const Priority defaultPriority;
	static const ModSize defaultLimitSize;

	// 作成した全メモリハンドルを管理するリスト
	static ModSinglyLinkedList<ModMemoryHandle>* allListing;
#ifdef DEBUG
    // メモリプールのデバッグ用
	// 本メモリハンドルが獲得したメモリのアドレスとサイズを
	// 管理・チェックするためのリスト
	// デバッグ時のみデバッグセルが登録される。
	// 通常はこのリストに対するロックを以って
	// 本メモリハンドル自身に対するロックとする。

	static ModBoolean		useDebugList;
    ModSinglyLinkedList<ModMemoryDebugCell>	debugList;
	ModBoolean				allocateCheck;
#endif
	// negotiate呼び出し中スレッド
	static ModSinglyLinkedList<ModNegotiatingThread>* negotiatingThreadList;
	// メモリリークをチェックするかどうか。
	ModCommonDLL
	static ModBoolean		memoryLeakCheck;
};

// メモリハンドル自身のメモリは特殊なものとしてModMemoryHandleに定義されて
// いるので、それを継承すればOK。

//
// FUNCTION
// ModMemoryHandle::operator new -- メモリハンドルのnew
//
// NOTES
// メモリハンドルはMODの中でも特別な扱いを受ける。
// この他で確保されたメモリはすべてどれかのメモリハンドルに管理されるが、
// メモリハンドル自身を確保するためのメモリはメモリプールから直接獲得
// され、どのメモリハンドルの管理下にも入らない。
// 
// ネゴ関数も今やメモリハンドルに属していないので、
// プールの制限を越えた後、ネゴシエーション関数を順番に呼び出して
// 確保をこころみることも考えられるが、メモリハンドルは普通最初に作って
// しまうので現在のところそこまでは対応していない。
//
// ARGUMENTS
// unsigned int size
//      確保するサイズ
// unsigned int dummy
//		この引数を定義しないと、VC++ 6.0 で C4291 の警告が発生し、
//		new されたオブジェクトのコンストラクターで例外が発生したときに、
//		delete が呼び出されず、確保された自由記憶領域が解放されない
//
// RETURN
// 確保したメモリの先頭アドレス
//
// EXCEPTIONS
//	その他
//		ModMemoryPool::allocateMemoryの例外参照
//

inline
void*
ModMemoryHandle::operator new(size_t size, size_t dummy)
{
	// 初期化チェックはModMemoryPoolで。
	// C++の定義により、引数はunsigned intであったが、ここで変換
	return ModMemoryPool::allocateMemory((ModSize)size);
};

//
// FUNCTION
// ModMemoryHandle::operator delete -- メモリハンドルのdelete
//
// NOTES
// メモリハンドルはMODの中でも特別な扱いを受ける。
// メモリハンドルのnewで確保されたメモリ(メモリプールから直接確保されている)を
// 解放する。
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
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//
inline void
ModMemoryHandle::operator delete(void* address, size_t size)
#ifdef STD_CPP11
noexcept(false)
#endif
{
	// 自分自身のロックはデストラクタで行われる。
	// 初期化チェックはModMemoryPoolで。
	// C++の定義により、引数はsize_tであったが、ここで変換
	ModMemoryPool::freeMemory(address, (ModSize)size);
};

#ifdef STD_CPP11
//
// FUNCTION
// ModMemoryHandle::operator delete -- メモリハンドルのdelete
//
// NOTES
// placement newであるnew(size_t, size_t)に対してdelete(void*, size_t)に
// placement deleteの意味を持たせるには、対応するusual deleteとして
// delete(void*)を定義しておく必要がある。
// そのためにこのdelete演算子を定義するが、呼ばれることはないはずである。
// 呼ばれたときは常に例外を送出する。
//
// ARGUMENTS
// void* address
//		解放するメモリの先頭アドレス
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModMemoryErrorWrongDeleteCalled
//		誤ったdelete演算子が呼ばれた
//
inline void
ModMemoryHandle::operator delete(void* address) noexcept(false)
{
	// 無条件に例外を送出する
	ModThrow(ModModuleMemory,
			 ModMemoryErrorWrongDeleteCalled, ModErrorLevelError);
}
#endif

//	FUNCTION public
//	ModMemoryHandle::setAllocateCheck --
//		領域確保時にその領域の情報を出力するかどうかを指定する
//
//	NOTES
//		領域を確保したとき、その領域のアドレスとサイズを
//		出力するかどうかを指定する
//		ただし、マクロ DEBUG が定義されているときのみ
//
//	ARGUMENTS
//		ModBoolean			check
//			ModTrue
//				出力する
//			ModFalse
//				出力しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModMemoryHandle::setAllocateCheck(ModBoolean check)
{
#ifdef DEBUG
	this->allocateCheck = check;
#endif
}

//
// FUNCTION
// ModMemoryHandle::getLimitSize -- メモリハンドルとして利用できる総量の上限をを得る
//
// NOTES
//	コンストラクタで設定された本メモリハンドルから利用できるメモリ総量の上限
//	を返す。途中で変更できない値なので参照のためにロックする必要はない。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	本メモリハンドルから利用できるメモリ総量の上限(バイト数)
//
// EXCEPTIONS
//	なし
//
inline ModSize
ModMemoryHandle::getLimitSize() const
{
	return this->limitSize;
}

//	FUNCTION public
//	ModMemoryHandle::isMemoryLeakCheck --
//		メモリーハンドルで確保した領域がちゃんと破棄されるかを検査するか
//
//	NOTES
//		メモリーハンドルを介して確保した領域が、そのメモリーハンドルを
//		破棄するまでにちゃんと ModMemoryHandle::freeMemory で
//		破棄されるかを検査するかを得る
//
//		パラメーター StopDebugCheck と StopMemoryLeakCheck の影響を受ける
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			検査する
//		ModFalse
//			検査しない
//
//	EXCEPTIONS
//		なし

// static
inline
ModBoolean
ModMemoryHandle::isMemoryLeakChecked()
{
	return (ModDebug::check) ? ModMemoryHandle::memoryLeakCheck : ModFalse;
}

// 
// FUNCTION
// ModMemoryHandle::getCurrentSize -- メモリの使用量を得る
//
// NOTES
// このメモリハンドルから獲得されたメモリの使用量を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 本メモリハンドルで管理されているメモリの使用量
//
// EXCEPTIONS
// なし
//
inline ModSize
ModMemoryHandle::getCurrentSize() const
{	
    return this->currentSize;
}

// 
// FUNCTION private
// ModMemoryHandle::freePoolMemory -- メモリプールのメモリを解放
//
// NOTES
// メモリプールのメモリを解放し、メモリハンドルで使用しているメモリ量を
// 更新する。初期化チェックや、ロックはあらかじめしてから呼び出すのが前提。
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
// なし
//
inline void
ModMemoryHandle::freePoolMemory(void* address, ModSize size)
{
	ModMemoryPool::freeMemoryUnsafe(address, size);
	this->currentSize -= size;
}

#endif	// __ModMemoryHandle_H__

//
// Copyright (c) 1997, 2009, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
