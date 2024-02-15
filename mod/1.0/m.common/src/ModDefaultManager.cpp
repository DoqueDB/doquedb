// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDefaultManager.cpp -- ModDefaultManager のメンバ定義
// 
// Copyright (c) 1997, 1999, 2007, 2009, 2023 Ricoh Company, Ltd.
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


#include "ModConfig.h"
#include "ModCommonDLL.h"
#include "ModDefaultManager.h"

#if MOD_CONF_MEMORY_MANAGEMENT == 3
#include <new>
#endif

//	VARIABLE
//	ModManager<ModDefaultMemoryHandle>::initialized --
//		デフォルトマネージャの初期化フラグ
//
//	NOTES
//	デフォルトマネージャが初期化されているかどうかを示す。

ModCommonDLL
ModTemplateNull
ModBoolean	ModManager<ModDefaultMemoryHandle>::initialized = ModFalse;

//	VARIABLE
//	ModManager<ModDefaultMemoryHandle>::memoryHandle --
//		デフォルトのメモリハンドル
//
//	NOTES
//		MOD 全体で利用されるメモリハンドルである。紆余曲折を経て、
//		今期開発では MOD 全体で利用するメモリハンドルは基本的に一つと決定した。
// 		MOD のオブジェクト、メモリ利用は特に理由がない限り、
//		すべて本メモリハンドルを通して獲得する。

ModCommonDLL
ModTemplateNull
ModDefaultMemoryHandle*	ModManager<ModDefaultMemoryHandle>::memoryHandle = 0;

//
// FUNCTION public
// ModDefaultManager::initialize -- 初期化
//
// NOTES
//	汎用ライブラリ全体の初期化をするcheckAndInitializeを呼び出す。
//	何回呼び出しても害はない。初期化の実体はdoInitialize()とし、
//	ModCommonInitializeからしか呼び出せないようにして一回だけの呼び出し
//	を保証する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		初期化前のみModCommonInitialize::checkAndInitializeの例外参照
//

ModCommonDLL
ModTemplateNull
// static
void
ModManager<ModDefaultMemoryHandle>::initialize()
{
	ModCommonInitialize::checkAndInitialize();
}

//
// FUNCTION public
// ModDefaultManager::terminate -- 標準モジュールの後処理
//
// NOTES
//	本当の後処理は汎用ライブラリ全体の終了処理の機構にまかせ、
//	何もしないので何回呼び出しても害がない。(互換性のために残すだけ。)
//	後処理の実体はdoTerminate()とし、
//	ModCommonInitializeからしか呼び出せないようにして一回だけの呼び出し
//	を保証する。
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

ModCommonDLL
ModTemplateNull
// static
void
ModManager<ModDefaultMemoryHandle>::terminate()
{ }

//
// FUNCTION private
// ModDefaultManager::doInitialize -- デフォルトマネージャの初期化の実体(static)
//
// NOTES
//	全体で使うメモリハンドルを官吏するデフォルトマネージャの初期化を行う。
//	プライベートメソッドであり、ModCommonInitializeから
//	一回だけ呼び出されることが保証される。
//	メモリハンドルの作成時の設定では、メモリ利用の上限値は設けていない。
//	いずれはパラメータ指定できるようにするべきだろう。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModMemoryHandle::new、ModMemoryHandle::ModMemoryHandleの例外参照(初期化前のみModCommonInitialize::checkAndInitializeの例外、等)。可能性の高いModMemoryHandle::newの例外のみ以下に書き下す。
//	ModMemoryErrorPoolLimitSize		(ModMemoryPool::allocateMemory)
//		使用量がプールの制限値を超える
//	ModMemoryErrorOverPoolLimit		(ModMemoryPool::allocateMemory)
//		プールの制限値以上の値を指定
//	ModMemoryErrorOsAlloc			(ModMemoryPool::allocateMemory)
//		メモリーが新たに獲得できない
//	

void
ModDefaultManager::doInitialize()
{
	if (ModDefaultManager::isInitialized() == ModFalse) {

		// メモリーの確保可能総サイズは ModSizeMax にする

		memoryHandle = new ModDefaultMemoryHandle(ModSizeMax);
		initialized = ModTrue;
	}
}

//
// FUNCTION private
// ModDefaultManager::doTerminate -- 標準モジュールの後処理(static)
//
// NOTES
// 	デフォルトメモリハンドルの後処理を行う。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModMemoryHandle::delete、ModMemoryHandle::~ModMemoryHandleの例外参照。
//

void
ModDefaultManager::doTerminate()
{
	if (ModDefaultManager::isInitialized() == ModTrue) {
		delete memoryHandle, memoryHandle = 0;
		initialized = ModFalse;
	}
}

//
// VARIABLE
// ModDefaultMemoryHandle::name -- デフォルトマネージャの名前
// NOTES
// デフォルトメモリハンドルを管理する仮想マネージャ名を示す変数である。
//

const char* ModDefaultMemoryHandle::name = "ModDefaultManager";

//
// FUNCTION public
// ModDefaultMemoryHandle::~ModDefaultMemoryHandle -- デストラクタ
//
// NOTES
//	デフォルト用メモリハンドルのデストラクタ。
//	メモリリークチェックが指定されている時にはメモリリークをチェックする。
//	メモリリークチェックは親クラスの方でしたいが、pure virtualメソッドが
//	利用されている関係でできない。
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

ModDefaultMemoryHandle::~ModDefaultMemoryHandle()
{
	if (ModMemoryHandle::isMemoryLeakChecked())

		// このメモリーハンドルを使った確保したメモリーのうち、
		// 未解放のものの情報を出力する

		this->printMemoryLeak();
}

#if MOD_CONF_MEMORY_MANAGEMENT == 3

// FUNCTION public
//	ModDefaultObject::operator new -- 
//
// NOTES
//	override operator new using ModOsDriver::Memory::allocate
//
// ARGUMENTS
//	size_t size
//	
// RETURN
//	void*
//
// EXCEPTIONS

void*
ModDefaultObject::operator new(size_t size)
{
	void* p = ModOsDriver::Memory::alloc((ModSize)size);
	if (!p) {
		throw std::bad_alloc();
	}
	return p;
}
void*
ModDefaultObject::operator new(size_t size, void* address)
{
	return address;
}

// FUNCTION public
//	ModDefaultObject::operator delete -- 
//
// NOTES
//	override operator delete to used HeapFree
//
// ARGUMENTS
//	void* address
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
ModDefaultObject::operator delete(void* address)
{
	ModOsDriver::Memory::free(address);
}

// FUNCTION public
//	ModDefaultObject::operator delete -- 
//
// NOTES
//
// ARGUMENTS
//	void* pointer
//	void* address
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ModDefaultObject::
operator delete(void* pointer, void* address)
{
	// 何もしない
}
#endif

//
// Copyright (c) 1997, 1999, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
