// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DaemonThread.cpp -- 常駐型スレッド関連の関数定義
// 
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/DaemonThread.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoEvent.h"

#include "Exception/ModLibraryError.h"
#include "Exception/Unexpected.h"

#include "ModError.h"

#include <exception>

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{
}

//	FUNCTION public
//	Buffer::DaemonThread::join -- 常駐型スレッドの終了を待つ
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

void
DaemonThread::join()
{
	ExitStatus	stat;

	switch (getStatus()) {
	case Running:
	case Interrupting:
	case Interrupted:

		// スレッドは実行中なので、終了を指示する

		if (!abort()) {

			// 終了を指示できなかった

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// スレッドは停止中かもしれないので、起こす

		wakeup();

		// 繰り返し関数の実行が
		// 不可になっているかもしれないので、可能にする

		enable(true);
		// thru

	case Aborting:
	case Aborted:
	case ExitSoon:

		// スレッドに対して、終了が指示されているので、
		// 終了を待つ
		//
		//【注意】	Microsoft C/C++ Compiler Version 12.00.8804 で
		//			以下のように書くと、なぜか C2352 のエラーになる
		//
		//			stat = Common::Thread::join();

		stat = Thread::join();
		break;

	case Exited:

		// スレッドはすでに終了しているので、
		// スレッドの終了状況を得る

		stat = getExitStatus();
		break;

	case NotStarted:

		// スレッドはそもそも実行されていないので、
		// なにもしない

		return;
	}

	switch (stat) {
	case Normally:

		// スレッドは正常に終了した

		break;

	case ThrowException:

		// スレッドは例外が発生したために異常終了しているので、
		// その例外を発生する

		SydErrorMessage << "rethrow exception." << ModEndl;
		getException().throwClassInstance();

	case ThrowModException:
	{
		// スレッドは MOD の例外が発生したために異常終了しているので、
		// その MOD の例外を発生する

		const ModException&	e = getModException();
		_SYDNEY_THROW1(Exception::ModLibraryError, e);
	}
	default:

		// それ以外の理由でスレッドは終了した

		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

//	FUNCTION public
//	Buffer::DaemonThread::execute -- スレッドで処理すべき内容を実際に実行する
//
//	NOTES
//
//	ARGUMENTS
//		bool	force
//			true
//				実行不可でも、実行可能になるまで待ち、必ず、実行する
//			false または指定されないとき
//				実行不可ならば、実行をあきらめる
//
//	RETURN
//		true
//			実行できた
//		false
//			Buffer::DaemonThread::disable により実行不可になっているので、
//			実行できなかった
//
//	EXCEPTIONS

bool
DaemonThread::execute()
{
	do {
		{
		// 繰り返し関数が実行可能かを表すイベントを保護するためにラッチする

		Os::AutoTryCriticalSection	latch(_latch, false);
		latch.lock();

		if (!isEnabled())

			// 繰り返し関数は実行できないので、あきらめる

			return false;

		if (isInactive()) {

			// 繰り返し関数が実行中でないので、
			// 繰り返し関数を実行中にする

			Os::AutoEvent event(_inactiveEvent, false);

			// 繰り返し関数が実行可能か表すイベントを
			// 保護するためのラッチをはずす

			latch.unlock();

			// 繰り返し関数を実行する

			repeatable();

			break;
		}
		}
		// 繰り返し関数の実行が終了するまで待つ

		_inactiveEvent.wait();

	} while (true) ;

	// 繰り返し関数を実行し終えた

	return true;
}

bool
DaemonThread::execute(bool force)
{
	while (force) {

		// 繰り返し関数を実行してみる

		if (execute())

			// 繰り返し関数を実行できた

			return true;

		// 繰り返し関数が実行できないために、実行をあきらめたので、
		// 繰り返し関数が実行可能になるまで待つ

		_enableEvent.wait();
	}

	// 繰り返し関数の実行を一度だけ試みる

	return execute();
}

//	FUNCTION public
//	Buffer::DaemonThread::disable -- 繰り返し関数の実行を一時的に不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Buffer::DaemonThead::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Buffer::DaemonThread::enable の呼び出し 1 回ぶんを無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DaemonThread::disable(bool force)
{
	{
	// 繰り返し関数が実行可能かを表すイベントを保護するためにラッチする

	Os::AutoCriticalSection	latch(_latch);

	// 繰り返し関数を実行不可にする

	if (force && _count > 0)
		_count = 0;
	else
		--_count;

	if (!_count)
		_enableEvent.reset();
	}
	// 繰り返し関数が実行中でなくなるまで、待つ

	_inactiveEvent.wait();
}

//	FUNCTION public
//	Buffer::DaemonThread::enable -- 繰り返し関数の実行を可能にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Buffer::DaemonThread::disable の入れ子呼び出しを
//				すべて無効にする
//			false または指定されないとき
//				Buffer::DaemonThread::disable の呼び出し 1 回ぶんを無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
DaemonThread::enable(bool force)
{
	// 繰り返し関数が実行可能かを表すイベントを保護するためにラッチする

	Os::AutoCriticalSection	latch(_latch);

	// 繰り返し関数を実行可能にする

	if (force) {
		if (_count <= 0)
			_count = 1;
		else {
			_count++;
			return;
		}
	} else
		if (_count++)
			return;

	_enableEvent.set();
}

//	FUNCTION private
//	Buffer::DaemonThread::runnable -- スレッド本体を表す関数
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

void
DaemonThread::runnable()
{
	try
	{
		// 一定時間、スレッドを停止する

		_wakeupEvent.wait(_timeout);

		// スレッドに対して終了要請が行われるまで、
		// 以下の処理を繰り返す

		while (getStatus() != Aborting) {

			// 繰り返し関数を実行する

			(void) execute(true);
			
			// 一定時間、スレッドを停止する

			_wakeupEvent.wait(_timeout);
		}

		// 終了指示を受領したことを記録する

		(void) setStatus(Aborted);
		
	}
	catch (Exception::Object& e)
	{
		SydErrorMessage << e << ModEndl;
		throw;
	}
	catch (ModException& e)
	{
		SydErrorMessage
			<< Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
			<< ModEndl;
		throw;
	}
#ifndef NO_CATCH_ALL
	catch (std::exception& e)
	{
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;
		throw;
	}
	catch (...)
	{
		SydErrorMessage << "Unexpected Exception" << ModEndl;
		throw;
	}
#endif
}

//
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
