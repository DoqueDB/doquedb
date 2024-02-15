// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ThreadLocalStorage.h -- スレッドローカル記憶域関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_TLS_H
#define	__TRMEISTER_OS_TLS_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef OBSOLETE
#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
// #include <pthread.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::ThreadLocalStorage -- スレッドローカル記憶域を表すクラス
//
//	NOTES

class ThreadLocalStorage
{
public:
	// コンストラクター
	SYD_OS_FUNCTION 
	ThreadLocalStorage();
	// デストラクター
	~ThreadLocalStorage();

	// 値を設定する
	SYD_OS_FUNCTION 
	void					setValue(void* value);
	// 値を取得する
	void*					getValue();

private:
#ifdef SYD_OS_WINDOWS
	// スレッドローカル記憶域インデックス
	DWORD					_index;
#endif
#ifdef SYD_OS_POSIX
	// スレッド固有データキー
	pthread_key_t			_key;
#endif
};

//	FUNCTION public
//	Os::ThreadLocalStorage::~ThreadLocalStorage --
//		スレッドローカル記憶域を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifdef SYD_OS_WINDOWS
inline
ThreadLocalStorage::~ThreadLocalStorage()
{
	// スレッドローカル記憶域を破棄する
	//
	//【注意】	引数はおかしくないはずなので、
	//			エラーにならないはず
	
	(void) ::TlsFree(_index);
}
#endif
#ifdef SYD_OS_POSIX
inline
ThreadLocalStorage::~ThreadLocalStorage()
{
	// スレッド固有データキーを破棄する
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL のエラーにならないはず

	(void) ::pthread_key_delete(_key);
}
#endif

//	FUNCTION public
//	Os::ThreadLocalStorage::getValue --
//		スレッドローカル記憶域に設定されている値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		設定されている値を返す
//		値が設定されていなければ、0 を返す
//
//	EXCEPTIONS
//		なし

#ifdef SYD_OS_WINDOWS
inline
void*
ThreadLocalStorage::getValue()
{
	// スレッドローカル記憶域から値を得る
	//
	//【注意】	エラーになったときは、値が設定されていないとみなす

	return ::TlsGetValue(_index);
}
#endif
#ifdef SYD_OS_POSIX
inline
void*
ThreadLocalStorage::getValue()
{
	// スレッド固有データキーに設定されている値を得る

	return ::pthread_getspecific(_key);
}
#endif

_TRMEISTER_OS_END
_TRMEISTER_END
#endif

#endif	// __TRMEISTER_OS_TLS_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
