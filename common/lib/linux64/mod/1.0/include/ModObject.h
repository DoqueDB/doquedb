// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModObject.h -- ModObject のクラス定義
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

#ifndef	__ModObject_H__
#define __ModObject_H__

//
// TEMPLATE CLASS
// ModObject -- 各モジュールのオブジェクト用テンプレート
//
// TEMPLATE ARGUMENTS
// class _Manager
//	該当モジュールを統括するマネージャの型
// 
// NOTES
// モジュール別に用意するオブジェクトのテンプレートである。
// マネージャ_Managerに属するオブジェクトはすべてModObject<_Manager>の
// 派生クラスとして作成する。これにより、メモリ管理をクラス_Managerがもつ
// メモリハンドルに集中させることができる。
// 組み込み型のアロケートは別途、_Manager::allocateMemory()を呼び出すこと。
//
template <class _Manager>
class ModObject {
public:
	// C++の定義で、引数はunsigned intである。
#ifdef MOD_SELF_MEMORY_MANAGEMENT_OFF
	// new, delete の定義をしないことによって、
	// 大域的な new, delete が呼ばれるようになる
#else
#if MOD_CONF_MEMORY_MANAGEMENT == 3
	// ModDefaultObject で定義している
#else
	void* operator new(size_t size, size_t dummy = 0);

	void* operator new(size_t size, void* address);
	void  operator delete(void* address, size_t size);
	void  operator delete(void* pointer, void* address);
#endif
#endif
};


//
// **TEMPLATE CLASS
// ModAllocator -- 各モジュールのオブジェクト用テンプレート(使用しない)
//
// **TEMPLATE ARGUMENTS
// class _Manager
//	該当モジュールを統括するマネージャの型
// 
// **NOTES
// クラスPureClassで提供される機能をもったオブジェクトを、
// マネージャ_Managerの管理下で作成するためのテンプレートであるが、
// うまくいかなかったのでコメントアウトした。
// テンプレートでなく同様のサブクラスを作成し、オリジナルと同じコンストラクタ
// を用意すれば成功する場合もある。オブジェクト自身を返すような関数がある場合は
// 工夫が必要。
// 
// クラスとして提供する機能は別途PureClassとして実装し、
// 実際にオブジェクトとして作成するときには、モジュール別の管理下に置く
// ための機構である。
// 本テンプレートを使わなくても、クラスPureClassはグローバルな
// new/deleteを使って作成/破棄できてしまうので注意する必要がある。
// 統一をとるため、PureClassに相当するクラスはModPureXXXという名前で
// 作成すること。
//
#if 0
template <class _PureClass, class _Manager>
class ModAllocator: public ModObject<_Manager>, public _PureClass {
	// 内容はない。new/deleteをModObject<_Manager>から継承させたいだけ。
public:
	// デフォルトコンストラクタ
	ModAllocator(): _PureClass() {}
	// 上記以外のコンストラクタは書きようがない
};
#endif

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
#if MOD_CONF_MEMORY_MANAGEMENT != 3
//
// TEMPLATE FUNCTION public
// ModObject<_Manager>::operator new -- オブジェクト作成
//
// TEMPLATE ARGUMENTS
//	class _Manager
//		該当モジュールのマネージャクラス
//
// NOTES
// テンプレート引数で指定されたマネージャのメモリハンドルから
// メモリを獲得し、オブジェクトを作成する。本テンプレートを利用して、
// 該当モジュールでインスタンス化される可能性のあるすべてのオブジェクトの
// 基本クラスに相当するModXXXObjectを定義する。すると、ModObjectのnew, delete
// が継承され、正しいメモリハンドルからメモリを確保することができる。
//
// ARGUMENTS
// size_t size
//		要求するメモリサイズ
// size_t dummy
//		この引数を定義しないと、VC++ 6.0 で C4291 の警告が発生し、
//		new されたオブジェクトのコンストラクターで例外が発生したときに、
//		delete が呼び出されず、確保された自由記憶領域が解放されない
//
// RETURN
// 獲得したメモリへのポインタを返す
// 
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前の場合のみ)、MemoryHandle::allocateMemoryの例外参照
//

template <class _Manager>
inline
void*
ModObject<_Manager>::operator new(size_t size, size_t dummy)
{
	// 初期化チェックは_Managerにまかせる

	// C++の定義で、引数はsize_tである。
	return _Manager::allocate((ModSize)size);
}

//
// TEMPLATE FUNCTION public
// ModObject<_Manager>::operator new -- オブジェクト作成
//
// TEMPLATE ARGUMENTS
//	class _Manager
//		該当モジュールのマネージャクラス
//
// NOTES
// テンプレート引数で指定されたマネージャのメモリハンドルから
// メモリを獲得し、オブジェクトを作成する。本テンプレートを利用して、
// 該当モジュールでインスタンス化される可能性のあるすべてのオブジェクトの
// 基本クラスに相当するModXXXObjectを定義する。すると、ModObjectのnew, delete
// が継承され、正しいメモリハンドルからメモリを確保することができる。
// 
// 本関数は、newの仕様にしたがって作成されている。
//
// ARGUMENTS
// unsigned int size
//		要求するメモリサイズ
// void* address
//		すでに確保されたメモリのアドレス
//
// RETURN
// 獲得されたメモリへのポインタを返す
// 
// EXCEPTIONS
//	なし
//
template <class _Manager>
inline
void*
ModObject<_Manager>::operator new(size_t size, void* address)
{
	return address;
}

//
// TEMPLATE FUNCTION public
// ModObject<_Manager>::operator delete -- オブジェクト解放
//
// TEMPLATE ARGUMENTS
//	class _Manager
//		該当モジュールのマネージャクラス
//
// NOTES
// テンプレート引数で指定されたマネージャのメモリハンドルから確保した
// メモリを解放し、オブジェクトを消去する。本テンプレートを利用して、
// 該当モジュールでインスタンス化される可能性のあるすべてのオブジェクトの
// 基本クラスに相当するModXXXObjectを定義する。すると、ModObjectのnew, delete
// が継承され、正しいメモリハンドルからメモリを解放することができる。
// 
// ARGUMENTS
// void* address
//		対象となるメモリのアドレス
// size_t size
//		確保しているメモリサイズ
//
// RETURN
// なし
// 
// EXCEPTIONS
//	その他
//		MemoryHandle::freeMemory(多くの場合、ModMemoryErrorFreeUnAllocated)、ModCommonInitialize::checkAndInitialize(初期化前の場合のみ)、ModCommonErrorNotInitialized(マネージャ初期化前のみ)の例外参照
//
template <class _Manager>
inline
void
ModObject<_Manager>::operator delete(void* address, size_t size)
{
	// 初期化チェックは_Managerにまかせる

	// C++の定義で、引数はsize_tである。
	_Manager::free(address, (ModSize)size);
}

// TEMPLATE FUNCTION public
//	ModObject<_Manager>::operator delete -- 
//
// TEMPLATE ARGUMENTS
//	class _Manager
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

template <class _Manager>
inline
void
ModObject<_Manager>::
operator delete(void* pointer, void* address)
{
	// 何もしない
}

#endif	// MOD_CONF_MEMORY_MANAGEMENT != 3
#endif	// !MOD_SELF_MEMORY_MANAGEMENT_OFF

#endif // __ModObject_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
