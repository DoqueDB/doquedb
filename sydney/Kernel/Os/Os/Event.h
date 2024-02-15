// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Event.h -- イベント関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_EVENT_H
#define	__TRMEISTER_OS_EVENT_H

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

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::Event -- イベントを表すクラス
//
//	NOTES
//		同プロセススレッド間のイベントである

class Event
{
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
	friend class CriticalSection;
#endif
#endif
public:
	//	CLASS
	//	Os::Event::Category -- イベントの種類を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Os::Event::Category::Value --
		//		イベントの種類を表す値を提供する列挙型
		//
		//	NOTES

		typedef unsigned short	Value;
		enum
		{
			// 待っているスレッドのうちひとつしか起こさない
			// スレッドが起きたときに、リセットされる
			WakeUpOnlyOne =	0,
			// 待っているスレッドをすべて起こす
			// スレッドが起きたときにリセットされる
			WakeUpAll,
			// 明示的にリセットしなければ、リセットされない
			// 結果的に、待っているスレッドはすべて起こすことになる
			ManualReset,
			// 値の数
			ValueNum
		};
	};

	// コンストラクター
#ifdef SYD_OS_POSIX
	SYD_OS_FUNCTION
#endif
	Event(Category::Value category = Category::WakeUpOnlyOne,
		  bool signaled = false);
	// デストラクター
	~Event();

	// シグナル化する
	SYD_OS_FUNCTION
	void					set();
	// 非シグナル化する
	SYD_OS_FUNCTION
	void					reset();

	// シグナル化されるまで待つ
#ifdef SYD_OS_POSIX
	SYD_OS_FUNCTION
#endif
	void					wait();
	// シグナル化されるまで、指定された時間待つ
	SYD_OS_FUNCTION
	bool					wait(unsigned int msec);

private:
#ifdef SYD_OS_WINDOWS
	// イベントを操作する準備をする
	void
	prepare();
	// イベントハンドルが生成されていなければ、生成する
	static void
	prepare(HANDLE& handle, bool manual, bool signaled);
#endif
	// イベントの種類
	const Category::Value	_category;
#ifdef SYD_OS_WINDOWS
	// シグナル化されたイベントを生成するか
	const bool				_signaled;
	// イベントオブジェクトハンドル
	HANDLE					_handle;
#endif
#ifdef SYD_OS_POSIX
	// 条件変数を保護するためのミューテックス
	pthread_mutex_t			_mutex;
	// 条件変数
	pthread_cond_t			_condition;
	// シグナル化を待っているスレッドの数
	unsigned int			_waiters;
	// シグナル化されているか
	bool					_signaled;
#endif
};

#ifdef SYD_OS_WINDOWS
//	FUNCTION public
//	Os::Event::Event -- イベントを表すクラスのコンストラクター
//
//	NOTES
//		
//	ARGUMENTS
//		Category::Value		category
//			生成するイベントの種類
//		bool				signaled
//			true
//				シグナル化されたイベントを生成する
//			false または指定されないとき
//				非シグナル化されたイベントを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Event::Event(Category::Value category, bool signaled)
	: _category(category),
	  _signaled(signaled),
	  _handle(0)
{}
#endif

//	FUNCTION public
//	Os::Event::~Event -- イベントを表すクラスのデストラクター
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
Event::~Event()
{
	if (_handle)

		// イベントハンドルを破棄する
		//
		//【注意】	エラーが起きても無視する

		(void) ::CloseHandle(_handle), _handle = 0;
}
#endif
#ifdef SYD_OS_POSIX
inline
Event::~Event()
{
	// 条件変数とミューテックスを破棄する
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず
	//
	//			EBUSY のエラーは無視する

	(void) ::pthread_cond_destroy(&_condition);
	(void) ::pthread_mutex_destroy(&_mutex);
}
#endif

//	FUNCTION public
//	Os::Event::wait -- イベントがシグナル化するまで待つ
//
//	NOTES
//		
//	ARGUMENTS
//		unsigned int		msec
//			指定されたとき
//				最大待ち時間(単位ミリ秒)
//			指定されないとき
//				永久に待つ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

#ifdef SYD_OS_WINDOWS
inline
void
Event::wait()
{
	// イベントがシグナル化するまで、無制限に待つ

	(void) this->wait(INFINITE);
}
#endif

#ifdef SYD_OS_WINDOWS
//	FUNCTION private
//	Os::Event::prepare -- イベントを操作する準備をする
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

inline
void
Event::prepare()
{
	if (!_handle)
		prepare(_handle,
				_category == Category::WakeUpAll ||
				_category == Category::ManualReset, _signaled);
}
#endif

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_EVENT_H

//
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
