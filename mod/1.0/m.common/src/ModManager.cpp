// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModManager.cpp --- マネージャテンプレートで供給する関数
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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


#include "ModManager.h"
#include "ModMemory.h"

//
// FUNCTION public
// ModLocalManager::getMemoryAddress -- メモリクラスの保持するメモリを得る
//
// NOTES
//	メモリクラスの保持するメモリの先頭アドレスを得る。メモリクラスのメソッドを
//	呼び出すだけであるが、ModManagerはテンプレートであり、そのヘッダから
//	直接メモリクラスのメソッドを利用できないため、その基底クラスとなる
//	ModLocalManagerで実装し、実装ファイル(.cpp)に置いたものである。
//
// ARGUMENTS
//  ModMemory* memory
//
// RETURN
//	メモリのもつ先頭アドレス
//
// EXCEPTIONS
//	なし
//
void*
ModLocalManager::getMemoryAddress(ModMemory* memory)
{
	return memory->getHeadAddress();
}

//
// FUNCTION public
// ModLocalManager::getNewMemory -- 新たにメモリオブジェクトを作成
//
// NOTES
//	確保済みのメモリとサイズを渡し、それを管理する
//	メモリクラスのオブジェクトを新たに作成して返す。メモリクラスのメソッドを
//	呼び出すだけであるが、ModManagerはテンプレートであり、そのヘッダから
//	直接メモリクラスのメソッドを利用できないため、その基底クラスとなる
//	ModLocalManagerで実装し、実装ファイル(.cpp)に置いたものである。
//
// ARGUMENTS
//  void* address
//		確保済みのメモリのアドレス
//	ModSize size
//		確保済みのメモリのサイズ
//
// RETURN
//	作成したメモリの先頭アドレス
//
// EXCEPTIONS
//	その他
//		ModDefaultManager::allocateの例外参照、つまり初期化されていない場合を除けばModMemoryHandle::allocateMemoryの例外参照
//		
//

ModMemory*
ModLocalManager::getNewMemory(void* address, ModSize size)
{
	return new ModMemory(address, size, ModMemory::houseAbandon);
}

//
// FUNCTION public
// ModLocalManager::deleteMemory -- メモリオブジェクトを消去する
//
// NOTES
//	メモリクラスのオブジェクトを消去する。メモリクラスのメソッドを
//	呼び出すだけであるが、ModManagerはテンプレートであり、そのヘッダから
//	直接メモリクラスのメソッドを利用できないため、その基底クラスとなる
//	ModLocalManagerで実装し、実装ファイル(.cpp)に置いたものである。
//
// ARGUMENTS
//  ModMemory* memory
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModMemoryHandle::freeMemoryの例外参照(多くの場合、ModMemoryErrorFreeUnAllocated)
//
void
ModLocalManager::deleteMemory(ModMemory* memory)
{
	delete memory;
}

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
