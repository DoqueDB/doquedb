// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RWLock.h -- 読み取り書き込みロック関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_RWLOCK_H
#define	__TRMEISTER_OS_RWLOCK_H

#ifdef SYD_OS_SOL2_7
// #define SYD_POSIX_RWLOCK
#endif

#ifdef SYD_POSIX_RWLOCK
// #include <pthread.h>
#endif

#include "Os/Module.h"
#ifdef SYD_POSIX_RWLOCK
#else
#include "Os/CriticalSection.h"
#include "Os/Event.h"
#endif

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::RWLock -- 読み取り書き込みロックを表すクラス
//
//	NOTES
//		同プロセススレッド間読み取り書き込みロックである
//
//				┌─────┐
//				│  既  存  │
//				├─┬─┬─┤
//				│N │R │W │
//		┌─┬─┼─┼─┼─┤
//		│新│R │○│○│×│
//		│  ├─┼─┼─┼─┤
//		│規│w │○│×│×│
//		└─┴─┴─┴─┴─┘

class RWLock
{
public:
	//	CLASS
	//	Os::RWLock::Mode -- 読み取り書き込みロックのモードを表すクラス
	//
	//	NOTES

	struct Mode
	{
		//	ENUM
		//	Os::RWLock::Mode::Value --
		//		読み取り書き込みロックのモードの値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// 読み取りロック
			Read,
			// 書き込みロック
			Write,
			// モードの種類数
			ValueNum
		};
	};

	// コンストラクター
	RWLock();
	// デストラクター
	~RWLock();

	// ロックする
	SYD_OS_FUNCTION
	void					lock(Mode::Value mode);
	// ロックを試みる
	SYD_OS_FUNCTION
	bool					trylock(Mode::Value mode);
	// ロックをはずす
	SYD_OS_FUNCTION
	void					unlock(Mode::Value mode);

private:
	// 読み取りロックをする
	void					lockRead();
	// 読み取りロックを試みる
	bool					trylockRead();
	// 読み取りロックをはずす
	void					unlockRead();

	// 書き込みロックをする
	void					lockWrite();
	// 書き込みロックを試みる
	bool					trylockWrite();
	// 書き込みロックをはずす
	void					unlockWrite();

	// ロック時の引数エラー処理を行う
	void					lockBadArgument();
	// ロック試行時の引数エラー処理を行う
	bool					trylockBadArgument();
	// アンロック時の引数エラー処理を行う
	void					unlockBadArgument();

#ifdef SYD_POSIX_RWLOCK
	// 読み取り書き込みロック
	pthread_rwlock_t		_rwlock;
#else
	// 以下の情報を保護するためのラッチ
	CriticalSection			_latch;
	// 読み取りロック数
	unsigned int			_reader;
	// 書き込みロック待ちしているスレッド数
	unsigned int			_waiter;
	// 書き込みロック待ちしているスレッドを起こすためのイベント
	Event					_event;
#endif
};

//	FUNCTION public
//	Os::RWLock::RWLock --
//		読み取り書き込みロックを表すクラスのコンストラクター
//
//	NOTES
//		同プロセススレッド間読み取り書き込みロックを表すクラスを
//		コンストラクトする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifdef SYD_POSIX_RWLOCK
inline
RWLock::RWLock()
{
	// 読み取り書き込みロックを初期化する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	(void) ::pthread_rwlock_init(&_rwlock, 0);
}
#else
inline
RWLock::RWLock()
	: _reader(0),
	  _waiter(0),
	  _event(Event::Category::ManualReset, false)
{}
#endif

//	FUNCTION public
//	Os::RWLock::~RWLock --
//		読み取り書き込みロックを表すクラスのデストラクター
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

#ifdef SYD_POSIX_RWLOCK
inline
RWLock::~RWLock()
{
	// 読み取り書き込みロックを破棄する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	(void) ::pthread_rwlock_destroy(&_rwlock);
}
#else
inline
RWLock::~RWLock()
{}
#endif

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_RWLOCK_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
