// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModManager.h -- ModManager のテンプレート定義
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

#ifndef	__ModManager_H__
#define __ModManager_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModError.h"
#include "ModException.h"
#include "ModCommonException.h"
#include "ModCommonInitialize.h"
#ifdef PURIFY
#include "ModOsDriver.h"
#else
#include "ModMemoryHandle.h"
#endif

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。
//

// ModMemory.hをおおもとのModManager.hでインクルードすると
// ModMemory.hがModOsManager.hを参照しているため、このように宣言しておく。
class ModMemory;	

//
// CLASS 
// ModLocalManager -- テンプレートクラスModManagerの親クラス
// NOTES
//	現在、ModManagerはテンプレートであり、すべての実装をヘッダに
// 	作成しなくてはならない。この中でメモリModMemoryのメソッドを利用することは
//	できないので、その部分だけを実装ファイル(.cpp)に移すために本クラスを
//	用意した。
//

class ModCommonDLL ModLocalManager
{
public:
	// memoryの保持するメモリ領域の先頭アドレスを返す
	static void *getMemoryAddress(ModMemory* memory);
	// 新たなメモリを確保して返す
	static ModMemory* getNewMemory(void* address, ModSize size);
	// ModMemoryの解放
	static void deleteMemory(ModMemory*);
};

//
// TEMPLATE CLASS
// ModManager -- 各モジュールのマネージャ用テンプレート
//
// TEMPLATE ARGUMENTS
// class MemoryHandle
//		該当モジュール用のメモリハンドルの型。
//		メモリハンドルクラス ModMemoryHandle は抽象クラスなので、
//		実際にはその派生クラスを作成し、指定する。
//		ModXXXManager.h, ModXXXManager.cppを参照のこと。
//
// NOTES
// モジュール別に用意するマネージャのテンプレートである。
//
// MOD のマネージャの要件は、
// (1)専用のメモリハンドルを保持、
// (2)管理下のオブジェクトのメモリ確保及び破棄には、
//    そのメモリハンドルを使う、
// (3)初期化と終了処理を行う各関数が公開されている、
//	(まだ増えるかも)の三点。
//
// ほとんどのメンバ関数はテンプレートにより既に供給されているが、
// 各モジュールで、initialize(), terminate()だけは実装する必要がある。
// そこで、メモリハンドルの初期化やその他必要な処理を行う。
// 後処理についても同様。
// 詳しくは、ModDefaultManager.h, ModDefaultManager.cppを参照のこと。
// 
// 今回の開発では、開発効率向上のためメモリハンドルをモジュール別に
// 用意せずに全体でModDefaultMemoryHandleを使うことになった。(98/3/5)
//
// 一見テンプレートでなく、ModManagerを抽象クラスとし、それのサブクラス
// として実装する方が自由度が増して望ましいように思うが、すべてstatic関数なので
// 仮想関数にすることができないのでこのままにする。
// 

template <class MemoryHandle>
class ModManager
	: private	ModLocalManager
{
public:
	// 自分がもつメモリハンドルからのメモリの獲得、解放
	static void* allocate(ModSize size);
	static void free(void* address, ModSize size);

	// 獲得したメモリをModMemoryオブジェクトとして返す。デフォルトの
	// ModMemory::HouseMode = houseAbandonモードのみ。
	// ModMemoryで、オーバーロード関数が必要な場合は
	// allocateMemory()は利用できない。
	static ModMemory* allocateMemory(ModSize size);
	// ModMemoryオブジェクトのもつメモリとModMemory自身を解放する。
	static void freeMemory(ModMemory* address, ModSize size);

	// 領域獲得時のデバッグメッセージの出力設定
	// ★MT-safeではない。
	static void setAllocateCheck(ModBoolean check);

	static ModBoolean		isInitialized();	// 初期化されているか

	// 以下のメンバーは個々のインスタンスで定義する必要がある
	// また、使用時に DDL からインポートするために ModCommonDLL を指定する

	ModCommonDLL static void initialize();		// 初期化する
	ModCommonDLL static void terminate();		// 後処理する
protected:
	ModCommonDLL static ModBoolean initialized;	// 初期化されているかを表す
	ModCommonDLL static MemoryHandle* memoryHandle;
												// 使用するメモリーハンドル
};

//
// TEMPLATE FUNCTION public
// ModManager<MemoryHandle>::allocate -- メモリハンドルからメモリを獲得
//
// TEMPLATE ARGUMENTS
//	class MemoryHandle
//		モジュールのメモリアロケーションで利用するメモリハンドル
//
// NOTES
// テンプレートで指定されたメモリハンドルからメモリを獲得し、返す。
// 例えばintやcharなどの組み込み型を獲得するような場合に本関数を用いる。
// 開放時には本クラスの提供する関数freeを用いる。
//
// オブジェクトのnewも、間接的に本関数を利用する。その仕組みは、
// 各モジュールでインスタンス化される可能性のあるすべてのオブジェクトの
// 基本クラスに相当するModXXXObjectを定義することによって提供される。実際には
// テンプレートModObjectを利用して宣言する。
//
// ARGUMENTS
// ModSize size
//		要求するメモリサイズ
//
// RETURN
// 獲得したメモリへのポインタを返す
// 
// EXCEPTIONS
//	ModCommonErrorNotInitialized
//		マネージャがまだ初期化されていないか後処理後である
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前の場合のみ)、MemoryHandle::allocateMemoryの例外参照
//

// static
template <class MemoryHandle> 
void*
ModManager<MemoryHandle>::allocate(ModSize size)
{
	// 必要があれば、汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	if (ModManager<MemoryHandle>::isInitialized() == ModFalse) {

		// ここで必要な順に初期化する。
		// 汎用ライブラリの場合、通常ここにはこない。
		// ★terminateの後にはありえるので、そのとき中でinitializeしない処理
		// が必要。

		ModManager<MemoryHandle>::initialize();

		if (memoryHandle == 0)

			// マネージャーを初期化しても、
			// 使用するメモリーハンドルが生成されなかった

			ModThrow(ModModuleOs,
					 ModCommonErrorNotInitialized, ModErrorLevelWarning);
	}
#ifdef PURIFY
	// ★(注意)
	// ここでPURIFYのifdefを入れたので、各モジュールで入れる必要はない
	// 同様にModObject::operator newにも入っている。
	// ただし、これによって、ifdef PURIFYのときにメモリハンドルの機構は
	// 全く動かなくなるので、その過程でのメモリリークは全く調べられない
	// 調べるなら、PURIFYを定義せずにpurifyにかける必要がある。
	// 階層は深くなりすぎるが、起こるか起こらないかはわかる。
	//
	// 実はifdef部分が足りなかったので括弧の構成を変更。(98/4/2)
	return ModOsDriver::Memory::alloc(size);
#else
	return memoryHandle->allocateMemory(size);
#endif
}
//
// TEMPLATE FUNCTION public
// ModManager<MemoryHandle>::free -- メモリハンドルから獲得したメモリの開放
//
// TEMPLATE ARGUMENTS
//	class MemoryHandle
//		モジュールのメモリアロケーションで利用するメモリハンドル
//
// NOTES
//　本クラスが管理するメモリハンドルから獲得したメモリを開放する。
//	獲得元のメモリハンドルが異なる場合、メモリサイズが異なる場合には
//	例外が発生する。また、メモリハンドルのデストラクタではメモリリークを
//	チェックする。
//
// ARGUMENTS
// void* address
//	開放対象となるメモリのアドレス
// ModSize size
//	対象となるメモリのサイズ
//
// RETURN
// なし
// 
// EXCEPTIONS
//	ModCommonErrorNotInitialized
//		マネージャがまだ初期化されていないか後処理後である
//	その他
//		MemoryHandle::freeMemory(多くの場合、ModMemoryErrorFreeUnAllocated)、ModCommonInitialize::checkAndInitialize(初期化前の場合のみ)の例外参照
//

// static
template <class MemoryHandle> 
void 
ModManager<MemoryHandle>::free(void* address, ModSize size)
{
	// 必要があれば、汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

#ifdef PURIFY
	ModOsDriver::Memory::free(address);
#else
	if (ModManager<MemoryHandle>::isInitialized() == ModTrue)

		// マネージャーの後処理がされていないときは、メモリーを解放できる
		//
		//【注意】	静的な大域変数などのデストラクターは main() が
		//			終了した後に呼び出されるので、
		//			その場合、ModManager<MemoryHandle>::terminate() は
		//			呼び出されており、マネージャーは後処理されている
		//
		//【注意】	ModManager<MemoryHandle>::terminate() 時に
		//			解放されていないメモリーが検査し、
		//			その情報を出力することができる

		memoryHandle->freeMemory(address, size);
#endif
}

//
// TEMPLATE FUNCTION public
// ModManager<MemoryHandle>::setAllocateCheck -- メモリハンドルのチェックフラグの設定
//
// TEMPLATE ARGUMENTS
//	class MemoryHandle
//		モジュールのメモリアロケーションで利用するメモリハンドル
//
// NOTES
// テンプレートで指定されたメモリハンドルの領域が獲得されたとき、
// 領域が獲得されたとき、そのアドレスとサイズを出力するかどうかをセットする。
// ModTrueにセットすると出力する。DEBUGが立っているときのみ有効。
// メモリリークの原因追求を想定している。
//
// ARGUMENTS
// ModBoolean check
//		セットする値
//
// RETURN
// なし
// 
// EXCEPTIONS
//	その他
//		初期化前のみ、ModCommonInitialize::checkAndInitializeの例外参照
//

// static
template <class MemoryHandle> 
void
ModManager<MemoryHandle>::setAllocateCheck(ModBoolean check)
{
	// 必要があれば、汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	memoryHandle->setAllocateCheck(check); 
}

//
// TEMPLATE FUNCTION public
// ModManager<MemoryHandle>::isInitialized -- 初期化されているかどうかを返す
//
// TEMPLATE ARGUMENTS
//	class MemoryHandle
//		モジュールのメモリアロケーションで利用するメモリハンドル
//
// NOTES
//	マネージャが初期化されているかどうかを返す。
//	MT-safeに実装するにはMutexロックの機構が必要だが、
//	使い方が「初期化、後処理、setAllocateCheckはスレッドに分かれる前と後に行う」
//	という前提であれば問題ない。考えてから対応する。
//	初期化のメソッドの実装の中で、変数initializedを設定する必要がある。
//
// ARGUMENTS
// なし
//
// RETURN
// 初期化されているかどうかを返す。
// 
// EXCEPTIONS
// なし
//

// static
template <class MemoryHandle> 
ModBoolean
ModManager<MemoryHandle>::isInitialized()
{
	return initialized;
}

//
// TEMPLATE FUNCTION
// ModManager<MemoryHandle>::allocateMemory -- メモリをメモリクラスとして獲得
//
// TEMPLATE ARGUMENTS
// class MemoryHandle
//		このマネージャがメモリを確保するメモリハンドルのクラス
//
// NOTES
// houseAbandonモード(残りサイズが不足している場合、不足がわかった時点で
// オーバーフロー関数を呼び出す)のメモリクラスModMemoryのオブジェクトを生成し、
// 獲得したメモリを設定して返す。
// メモリはこのマネージャクラスがもつメモリハンドル
// (テンプレート引数で指定されたクラス)から獲得するが、
// ModMemoryはModOsManagerのメモリハンドルから獲得される。
// 
// 返されたオブジェクトは本クラスの提供する関数freeMemoryで開放すること。
//
// ModMemoryでオーバーロード関数が必要な場合は、ModMemoryのサブクラスで
// 実装する必要があるため、内部で作成して返すわけにはいかず、
// allocateMemory()が利用できない。
// 本関数を利用せず、関数allocateでメモリを獲得した後、ローカルに
// ModMemoryを生成して、コンストラクタでそのメモリを設定する必要がある。
// (10/3)
//
// ARGUMENTS
// ModSize size
//		要求するメモリサイズ
//
// RETURN
// 		獲得したメモリを設定したメモリクラスのオブジェクト
//
// EXCEPTIONS
//	その他
//		MemoryHandle::allocateMemory、ModCommonInitialize::checkAndInitialize(初期化前のみ)の例外参照
//

template <class MemoryHandle> 
ModMemory*
ModManager<MemoryHandle>::allocateMemory(ModSize size)
{
	void*	p = ModManager<MemoryHandle>::allocate(size);

	try {
		return ModLocalManager::getNewMemory(p, size);

	} catch (ModException& exception) {
		ModManager<MemoryHandle>::free(p, size);
		ModRethrow(exception);
	}
}

//
// TEMPLATE FUNCTION
// ModManager<MemoryHandle>::freeMemory -- メモリとメモリクラスの開放
//
// TEMPLATE ARGUMENTS
// class MemoryHandle
//		このマネージャがメモリを確保するメモリハンドルクラス
//
// NOTES
// メモリModMemoryとそれが保持するメモリを解放する。
// 内部で、ModMemory型のオブジェクト自身もdeleteされる。
// 保持しているメモリは、本クラスが保持するメモリハンドルから
// 獲得されていなければならない。メモリハンドル、メモリサイズは
// チェックされる。
// メモリリークはメモリハンドルのデストラクタでチェックされる。
//
// ARGUMENTS
// ModMemory* memory
//		メモリを保持するメモリオブジェクト
// ModSize size
//		メモリサイズ
//
// RETURN
// なし
// 
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前の場合のみ)、MemoryHandle::freeMemoryの例外参照(多くの場合、ModMemoryErrorFreeUnAllocated)
//

template <class MemoryHandle> 
void 
ModManager<MemoryHandle>::freeMemory(ModMemory* memory, ModSize size)
{
	try {
		ModManager<MemoryHandle>::free(
			ModLocalManager::getMemoryAddress(memory), size);
		ModLocalManager::deleteMemory(memory), memory = 0;

	} catch (ModException& exception) {

		ModLocalManager::deleteMemory(memory), memory = 0;
		ModRethrow(exception);
	}
}

#endif	// __ModManager_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
