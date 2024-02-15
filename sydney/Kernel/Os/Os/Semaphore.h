// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Semaphore.h -- セマフォ関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_SEMAPHORE_H
#define	__TRMEISTER_OS_SEMAPHORE_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
// #include <semaphore.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::Semaphore -- セマフォを表すクラス
//
//	NOTES
//		同プロセススレッド間のセマフォである

class Semaphore
{
public:
	// コンストラクター
	SYD_OS_FUNCTION
	Semaphore(unsigned int v);
	// デストラクター
	~Semaphore();

	// 0 より大きくなるまで待ち、1 減らす
	SYD_OS_FUNCTION
	void
	lock();
	// 0 より大きいか調べ、大きかったら 1 減らす
	SYD_OS_FUNCTION
	bool
	trylock();

	// 1 増やす
	SYD_OS_FUNCTION
	void
	unlock();

private:
#ifdef SYD_OS_WINDOWS
	// セマフォオブジェクトハンドル
	HANDLE					_handle;
#endif
#ifdef SYD_OS_POSIX
	// セマフォ
	sem_t					_semaphore;
#endif
};

//	FUNCTION public
//	Os::Semaphore::~Semaphore -- セマフォを表すクラスのデストラクター
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

inline
Semaphore::~Semaphore()
{
#ifdef SYD_OS_WINDOWS
	if (_handle)

		// セマフォハンドルを破棄する
		//
		//【注意】	エラーが起きても無視する

		(void) ::CloseHandle(_handle), _handle = 0;
#endif
#ifdef SYD_OS_POSIX

	// セマフォを破棄する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	(void) ::sem_destroy(&_semaphore);
#endif
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_SEMAPHORE_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
