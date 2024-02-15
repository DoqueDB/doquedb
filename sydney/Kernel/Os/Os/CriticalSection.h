//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CriticalSection.h -- クリティカルセクション関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2010, 2018, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_CRITICALSECTION_H
#define	__TRMEISTER_OS_CRITICALSECTION_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
// #include <pthread.h>
#endif

#include "Os/Module.h"
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN95
#include "Os/Mutex.h"
#include "Os/Thread.h"
#endif
#endif
#ifdef SYD_OS_POSIX
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
#include "Os/Thread.h"
#endif
#endif

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::CriticalSection -- クリティカルセクションを表すクラス
//
//	NOTES
//		同プロセススレッド間のミューテックスである
//		同じスレッドがなん度でもロックを重ねがけできる

class CriticalSection
{
	friend class Event;
public:
	// コンストラクター
	CriticalSection();
	// デストラクター
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
	SYD_OS_FUNCTION
#endif
#endif
	~CriticalSection();

	// ロックする
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
	SYD_OS_FUNCTION
#endif
#endif
#ifdef SYD_OS_POSIX
	SYD_OS_FUNCTION
#endif
	void					lock();

	// ロックを試みる
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
	SYD_OS_FUNCTION
#endif
#endif
#ifdef SYD_OS_POSIX
	SYD_OS_FUNCTION
#endif
	bool					trylock();

	// ロックをはずす
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
	SYD_OS_FUNCTION
#endif
#endif
#ifdef SYD_OS_POSIX
	SYD_OS_FUNCTION
#endif
	void					unlock();
#ifdef OBSOLETE
	SYD_OS_FUNCTION
	void					unlock(int n);
#endif
#ifdef SYD_OS_LINUX	
	// pthread_mutex_t からオーナースレッドのID(LWP)を取得する
	SYD_OS_FUNCTION
	int 					getOwnerID();
#endif

private:
	//【未定義】 コピーコンストラクター
	CriticalSection(const CriticalSection& src);
	//【未定義】 = 演算子
	CriticalSection&		operator =(const CriticalSection& r);

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
	// クリティカルセクション
	CRITICAL_SECTION		_criticalSection;
#endif
#ifdef SYD_OS_WIN98

	//【注意】	Windows 95, 98, ME では ::TryEnterCriticalSection が
	// 			正しく動作しないので、自分でクリティカルセクションを実装した
	//			しかし、95 には ::InterlockedCompareExchange がないので、
	//			このクラスは 98 以降のみで動作する

	// ロック待ちの準備を行う
	void					prepare();

	// ロック待ちに使用するイベント
	HANDLE					_event;
	// ロックしたスレッドのスレッド ID
	Thread::ID				_locker;
	// ロック数
	int						_count;
	// 入れ子ロック数
	int						_recursiveCount;
#endif
#endif
#ifdef SYD_OS_POSIX
	// ミューテックス
	pthread_mutex_t			_mutex;
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
	// ロックしたスレッドのスレッド ID
	Thread::ID				_locker;
	// 入れ子ロック数
	int						_count;
#endif
#endif
};

//	FUNCTION public
//	Os::CriticalSection::CriticalSection --
//		クリティカルセクションを表すクラスのコンストラクター
//
//	NOTES
// 		同プロセススレッド間のミューテックスである
//		クリティカルセクションを表すクラスをコンストラクトする
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
#ifdef SYD_OS_WINNT4_0
inline
CriticalSection::CriticalSection()
{
#if _WIN32_WINNT >= 0x0403

	// スピンカウントを使うクリティカルセクションを初期化する
	//
	// MSDN によれば、スピンカウントが約 4000 のとき、
	// マルチプロセッサシステムでは
	// 最悪の条件でも高いパフォーマンスが出るということである

	(void) ::InitializeCriticalSectionAndSpinCount(&_criticalSection, 4000);
#else
	// クリティカルセクションを初期化する

	::InitializeCriticalSection(&_criticalSection);
#endif
}
#endif
#ifdef SYD_OS_WIN98
inline
CriticalSection::CriticalSection()
	: _event(0),
	  _locker(0),
	  _count(0),
	  _recursiveCount(0)
{}
#endif
#endif
#ifdef SYD_OS_POSIX
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
inline
CriticalSection::CriticalSection()
	: _count(0)
{
	// ミューテックスを初期化する
	//
	//【注意】	ミューテックス属性を与えないと、
	//			デフォルトで同プロセススレッド間ミューテックスになる
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_init(&_mutex, 0);
}
#endif
#endif

#ifdef OBSOLETE
//	FUNCTION public
//	Os::CriticalSection::CriticalSection --
//		クリティカルセクションを表すクラスのコピーコンストラクター
//
//	NOTES
//		コピーせずに自分自身をたんにコンストラクトする
//
//	ARGUMENTS
//		Os::CriticalSection&	src
//			コピー元のクリティカルセクションを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
inline
CriticalSection::CriticalSection(const CriticalSection& src)
{
#if _WIN32_WINNT >= 0x0500
	::InitializeCriticalSectionAndSpinCount(&_criticalSection, 4000);
#else
	::InitializeCriticalSection(&_criticalSection);
#endif
}
#endif
#ifdef SYD_OS_WIN98
inline
CriticalSection::CriticalSection(const CriticalSection& src)
	: _event(0),
	  _locker(0),
	  _count(0),
	  _recursiveConut(0)
{}
#endif
#endif
#ifdef SYD_OS_POSIX
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
inline
CriticalSection::CriticalSection(const CriticalSection& src)
	: _count(0)
{
	(void) ::pthread_mutex_init(&_mutex, 0);
}
#endif
#endif
#endif

//	FUNCTION public
//	Os::CriticalSection::~CriticalSection --
//		クリティカルセクションを表すクラスのデストラクター
//
//	NOTES
//		ロック待ちしているスレッドが存在する
//		クリティカルセクションをデストラクトすると、
//		ロック待ちしているスレッドの動作は、不定である
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
#ifdef SYD_OS_WIN98
inline
CriticalSection::~CriticalSection()
{
	if (_event)

		// イベントハンドルを破棄する

		(void) ::CloseHandle(_event), _event = 0;
}
#endif
#endif

#ifdef OBSOLETE
//	FUNCTION public
//	Os::CriticalSection::operator = -- = 演算子
//
//	NOTES
//		代入せずに自分自身をたんに返すだけである
//
//	ARGUMENTS
//		Os::CriticalSection&	r
//			自分自身に代入するクリティカルセクションを表すクラス
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
CriticalSection&
CriticalSection::operator =(const CriticalSection& r)
{
	return *this;
}
#endif

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
//	FUNCTION public
//	Os::CriticalSection::lock --
//		クリティカルセクションをロックする
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
void
CriticalSection::lock()
{
	// クリティカルセクションに入る
	// 他のスレッドがすでにクリティカルセクションに入っていれば、
	// そのスレッドが出るまで、待つ

	::EnterCriticalSection(&_criticalSection);
}

//	FUNCTION public
//	Os::CriticalSection::trylock --
//		クリティカルセクションのロックを試みる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックできた
//		false
//			他のスレッドがロックしているのでロックできなかった
//
//	EXCEPTIONS
//		なし	

inline
bool
CriticalSection::trylock()
{
	// クリティカルセクションに入ろうとする
	// 他のスレッドがクリティカルセクションに入っていれば、あきらめる

	return ::TryEnterCriticalSection(&_criticalSection);
}

//	FUNCTION public
//	Os::CriticalSection::unlock --
//		クリティカルセクションのロックをはずす
//
//	NOTES
//		ロックしていないスレッドがロックをはずしても、
//		呼び出し側はエラーにならない
//		または、ロックした回数より多く、
//		ロックをはずしてもエラーにならない
//		ただし、ロック待ちしているスレッドの動作は、不定である
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
void
CriticalSection::unlock()
{
	// クリティカルセクションから出る

	::LeaveCriticalSection(&_criticalSection);
}
#endif
#endif

#ifdef SYD_OS_LINUX

//	FUNCTION public
//	Os::CriticalSection::getOwnerID --
//		クリティカルセクションを保持するスレッドのIDを取得する
//
//	NOTES
//  pthread_mutex_t 列挙体の内部情報を参照してスレッドIDを得る
//  ロックされていない場合、スレッドIDは 0 になる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int スレッドID (LWP)
//
//	EXCEPTIONS
//		なし

inline
int
CriticalSection::getOwnerID()
{
	return _mutex.__data.__owner;
}
#endif

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_CRITICALSECTION_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2010, 2018, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
