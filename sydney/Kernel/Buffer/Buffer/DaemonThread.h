// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DaemonThread.h -- 常駐型スレッド関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_DEAMONTHREAD_H
#define	__SYDNEY_BUFFER_DEAMONTHREAD_H

#include "Buffer/Module.h"

#include "Common/Thread.h"
#include "Os/CriticalSection.h"
#include "Os/Event.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	CLASS
//	Buffer::DaemonThread -- 常駐型スレッドを表すクラス
//
//	NOTES
//		常駐型スレッドとは、ある処理を定期的に繰り返し実行するスレッドである

class DaemonThread
	: public	Common::Thread
{
public:
	// コンストラクター
	DaemonThread(unsigned int timeout, bool enable);
	// デストラクター
	~DaemonThread();

	// 起動する
	// (Common::Thread::create は上書きされる)
	void
	create();
	// 終了を待つ
	// (Common::Thread::join は上書きされる)
	SYD_BUFFER_FUNCTION
	void
	join();

	// 繰り返し関数の実行を開始する
	void
	wakeup();
	// スレッドで処理すべき内容を実際に実行する
	SYD_BUFFER_FUNCTION
	bool
	execute();
	SYD_BUFFER_FUNCTION
	bool
	execute(bool force);

	// 繰り返し関数の実行を一時的に不可にする
	SYD_BUFFER_FUNCTION
	void
	disable(bool force = false);
	// 繰り返し関数の実行を可能にする
	SYD_BUFFER_FUNCTION
	void
	enable(bool force = false);
	// 繰り返し関数を実行可能か
	bool
	isEnabled();
	// 繰り返し関数が実行中でないか
	bool
	isInactive();

protected:
	// スレッドが繰り返し実行する関数
	virtual void
	repeatable() = 0;

	// 繰り返し関数の実行間隔
	unsigned int			_timeout;

private:
	// スレッド本体を表す関数
	SYD_BUFFER_FUNCTION
	virtual void
	runnable();

	// 排他制御用のラッチ
	mutable Os::CriticalSection _latch;

	// 入れ子呼び出し回数
	int						_count;
	// 繰り返し関数の実行が可能であることを表すイベント
	Os::Event				_enableEvent;
	// 繰り返し関数の実行を開始することを表すイベント
	Os::Event				_wakeupEvent;
	// 繰り返し関数が実行中でないことを表すイベント
	Os::Event				_inactiveEvent;
};

//	FUNCTION public
//	Buffer::DaemonThread::DaemonThread --
//		常駐型スレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		timeout
//			スレッドが繰り返し実行する関数の実行間隔(単位ミリ秒)
//		bool				enable
//			true
//				繰り返し関数の実行を可能にする
//			false
//				繰り返し関数の実行を不可にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
DaemonThread::DaemonThread(unsigned int timeout, bool enable)
	: _count(enable)
	, _enableEvent(Os::Event::Category::ManualReset, enable)
	, _timeout(timeout)
	, _wakeupEvent(Os::Event::Category::WakeUpOnlyOne, false)
	, _inactiveEvent(Os::Event::Category::ManualReset, true)
{}

//	FUNCTION public
//	Buffer::DaemonThread::~DaemonThread -- 
//		常駐型スレッドを表すクラスのデストラクター
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
DaemonThread::~DaemonThread()
{}

//	FUNCTION public
//	Buffer::DaemonThread::create -- 常駐型スレッドを生成する
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
DaemonThread::create()
{
	if (getStatus() == NotStarted) {

		// スレッドを生成していないので、生成する
		//
		//【注意】	Microsoft C/C++ Compiler Version 12.00.8804 で
		//			以下のように書くと、なぜか C2352 のエラーになる
		//
		//			Common::Thread::create();

		Thread::create();
	}
}

//	FUNCTION public
//	Buffer::DaemonThread::wakeup -- 繰り返し関数の実行を開始する
//
//	NOTES
//		繰り返し関数が実行されていなければ、実行を開始する
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
DaemonThread::wakeup()
{
	// スレッドが停止中であれば、起こす

	_wakeupEvent.set();
}

//	FUNCTION public
//	Buffer::DaemonThread::isEnabled -- 繰り返し関数の実行が可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			可能である
//		false
//			不可である
//
//	EXCEPTIONS

inline
bool
DaemonThread::isEnabled()
{
	return _enableEvent.wait(0);
}

//	FUNCTION public
//	Buffer::DaemonThread::isInactive -- 繰り返し関数の実行中でないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			実行中でない
//		false
//			実行中である
//
//	EXCEPTIONS

inline
bool
DaemonThread::isInactive()
{
	return _inactiveEvent.wait(0);
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_DEAMONTHREAD_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
