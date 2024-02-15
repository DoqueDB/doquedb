// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Daemon.cpp -- デーモン関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Configuration.h"
#include "Checkpoint/Daemon.h"
#include "Checkpoint/Executor.h"
#include "Checkpoint/FileSynchronizer.h"
#include "Checkpoint/Manager.h"

#include "Common/Assert.h"
#ifdef OBSOLETE
#include "Common/Thread.h"
#endif
#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Event.h"

#include "Exception/Object.h"
#include "Exception/RunningCheckpointProcessing.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace 
{

namespace _Daemon
{
	// チェックポイント処理実行スレッド
	Executor*			_executor = 0;
	// バージョンファイル同期スレッド
	FileSynchronizer*	_fileSynchronizer = 0;

	// 終了イベント
	Os::Event			_event;

	// チェックポイント処理実行スレッドかバージョンファイル同期スレッドが
	// 起動しているかどうか

	// 排他制御のためのクリティカルセクション
	Os::CriticalSection	_lock;

	// 実行しているかどうかのフラグ
	bool				_execute = false;
}

}

//	FUNCTION private
//	Checkpoint::Manager::Daemon::initialize --
//		マネージャーの初期化のうち、
//		チェックポイント処理に関するデーモンスレッド関連のものを行う
//
//	NOTES
//		チェックポイント処理実行スレッドは、処理の実行が不可の状態で生成される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Manager::Daemon::initialize()
{
	// チェックポイント処理に関するデーモンを初期化する

	Checkpoint::Daemon::initialize();

	try {
		// チェックポイント処理に関するデーモンを開始する
		//
		//【注意】	チェックポイント処理実行スレッドは、
		//			処理の実行が不可の状態で開始される

		Checkpoint::Daemon::create();
		Checkpoint::Daemon::enable(
			Checkpoint::Daemon::Category::FileSynchronizer);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Checkpoint::Daemon::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Checkpoint::Manager::Daemon::terminate --
//		マネージャーの後処理のうち、
//		チェックポイント処理に関するデーモンスレッド関連のものを行う
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

// static
void
Manager::Daemon::terminate()
{
	// チェックポイント処理に関するデーモンの後処理を行う

	Checkpoint::Daemon::terminate();
}

//	FUNCTION
//	Checkpoint::Daemon::initialize --
//		チェックポイント処理に関するデーモンスレッドの初期化を行う
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
Daemon::initialize()
{
	try {
		// 実行しているかどうかのフラグを初期化する

		_Daemon::_execute = false;
		
		// チェックポイント処理実行スレッドを生成する

		_Daemon::_executor = new Checkpoint::Executor(
			Configuration::Period::get(), false);
		; _SYDNEY_ASSERT(_Daemon::_executor);

		if (Configuration::EnableFileSynchronizer::get()
			!= Configuration::EnableFileSynchronizer::OFF) {

			// バージョンファイル同期スレッドを生成する

			_Daemon::_fileSynchronizer = new FileSynchronizer(
				~static_cast<unsigned int>(0), false);
			; _SYDNEY_ASSERT(_Daemon::_fileSynchronizer);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Daemon::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Checkpoint::Daemon::terminate --
//		チェックポイント処理に関するデーモンスレッドの後処理を行う
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
Daemon::terminate()
{
	// チェックポイント処理実行スレッドを表すクラスを破棄する

	delete _Daemon::_executor, _Daemon::_executor = 0;

	// バージョンファイル同期スレッドを表すクラスを破棄する

	delete _Daemon::_fileSynchronizer, _Daemon::_fileSynchronizer = 0;
}

//	FUNCTION
//	Checkpoint::Daemon::create --
//		チェックポイント処理に関するデーモンスレッドを生成する
//
//	NOTES
//		チェックポイント処理実行スレッドは
//		処理の実行が不可な状態で生成されるので、
//		Checkpoint::Daemon::enable で実行が可能な状態にしなければならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::create()
{
	; _SYDNEY_ASSERT(_Daemon::_executor);
	_Daemon::_executor->create();

	try {
		if (_Daemon::_fileSynchronizer) {
			_Daemon::_fileSynchronizer->create();
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Daemon::join(Category::Executor);
		_SYDNEY_RETHROW;
	}
}

#ifdef OBSOLETE
//	FUNCTION
//	Checkpoint::Daemon::join --
//		チェックポイント処理に関するデーモンスレッドを終了する
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
Daemon::join()
{
	unsigned int i = 0;
	do {
		try {
			join(static_cast<Category::Value>(i));
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// エラーを無視して、残りのスレッドを終了する

			Common::Thread::resetErrorCondition();
		}
	} while (++i < Category::ValueNum) ;
}
#endif

//	FUNCTION
//	Checkpoint::Daemon::join --
//		チェックポイント処理に関するデーモンスレッドを終了する
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			終了するデーモンスレッドの種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::join(Category::Value category)
{
	switch (category) {
	case Category::Executor:
		; _SYDNEY_ASSERT(_Daemon::_executor);
		_Daemon::_executor->join();

		// 最後のチェックポイント処理を行い、
		// システムが正常に終了したことをシステムの論理ログファイルに記録する

		Executor::cause(true);
		break;

	case Category::FileSynchronizer:
		if (_Daemon::_fileSynchronizer) {
			_Daemon::_fileSynchronizer->join();
		}
		break;
	}
}

//
//	FUNCTION
//	Checkpoint::Daemon::wakeup --
//		チェックポイント処理に関するデーモンスレッドの処理を再開する
//
//	NOTES
//	この関数はServer::Worker内から呼び出される
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Daemon::wakeup()
{
	// 実行中か確認する
	Os::AutoCriticalSection cAuto(_Daemon::_lock);

	if (_Daemon::_execute == true)
		_TRMEISTER_THROW0(Exception::RunningCheckpointProcessing);
			
	// 終了イベントを非シグナル化する
	_Daemon::_event.reset();

	// チェックポイントスレッドを再開する
	wakeup(Category::Executor);
}

//
//	FUNCTION
//	Checkpoint::Daemon::wait --
//		チェックポイント処理に関するデーモンスレッドの処理の終了を待つ
//
//	NOTES
//	この関数はServer::Worker内から呼び出される
//
//	ARGUMENTS
//	unsigned int timeout
//		終了待ち時間(msec)。default 0xffffffff
//
//	RETURN
//	bool
//		終了した場合はtrue、タイムアウトした場合はfalse
//
//	EXCEPTIONS
//
bool
Daemon::wait(unsigned int timeout)
{
	// 終了イベントを待つ
	return _Daemon::_event.wait(timeout);
}

//	FUNCTION
//	Checkpoint::Daemon::wakeup --
//		チェックポイント処理に関するデーモンスレッドの処理を再開する
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			処理を再開するデーモンスレッドの種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::wakeup(Category::Value category)
{
	Daemon::AutoState cState;
	
	switch (category)
	{
	case Category::Executor:
		; _SYDNEY_ASSERT(_Daemon::_executor);
		if (_Daemon::_executor->getStatus() == Common::Thread::Running)
		{
			_Daemon::_executor->wakeup();

			// 起動したので、状態報告はそのまま
			
			cState.release();
		}
		break;
		
	case Category::FileSynchronizer:
		if (_Daemon::_fileSynchronizer) {
			
			// 同期処理はEnableでかつスレッドが実行中の時しか起こさない
			
			if (_Daemon::_fileSynchronizer->getStatus()
				== Common::Thread::Running)
			{
				if (_Daemon::_fileSynchronizer->isEnabled())
				{
					// 終了イベントは同期処理スレッド内でシグナル化される
				
					_Daemon::_fileSynchronizer->wakeup();

					// 起動したので、状態報告はそのまま

					cState.release();
				}
				else
				{
					// 同期処理が無効になっている
					
					SydMessage << "Skip database synchronization." << ModEndl;

				}
			}
			else
			{
				// 過去にスレッドが予期せず終了している
					
				SydErrorMessage << "Database synchronizer wakeup failed."
								<< ModEndl;
			}
		}
		break;
		
	}
}

#ifdef OBSOLETE
//	FUNCTION
//	Checkpoint::Daemon::execute --
//		チェックポイント処理に関するデーモンスレッドの処理を実行する
//
//	NOTES
//		デーモンスレッドでなく呼び出しスレッドで実行する
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			処理を実行するデーモンスレッドの種別
//		bool		force
//			true
//				実行不可でも、実行可能になるまで待ち、必ず、実行する
//			false または指定されないとき
//				実行不可ならば、実行をあきらめる
//
//	RETURN
//		true
//			実行した
//		false
//			Checkpoint::Daemon::disable により実行不可になっているので、
//			実行できなかった
//
//	EXCEPTIONS

bool
Daemon::execute(Category::Value category, bool force)
{
	switch (category) {
	case Category::Executor:
		; _SYDNEY_ASSERT(_Daemon::_executor);
		return _Daemon::_executor->execute(force);
	case Category::FileSynchronizer:
		if (_Daemon::_fileSynchronizer) {
			return _Daemon::_fileSynchronizer->execute(force);
		}
		break;
	}
	return true;
}
#endif

//	FUNCTION
//	Checkpoint::Daemon::enable --
//		すべてのデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Checkpoint::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::disable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::enable(bool force)
{
	unsigned int i = 0;
	try {
		do {
			enable(static_cast<Category::Value>(i), force);

		} while (++i < Category::ValueNum);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		while (i--)
			disable(static_cast<Category::Value>(i));
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Checkpoint::Daemon::enable --
//		あるデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			処理を実行可能にするデーモンスレッドの種別
//		bool				force
//			true
//				Checkpoint::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::disable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::enable(Category::Value category, bool force)
{
	switch (category) {
	case Category::Executor:
		; _SYDNEY_ASSERT(_Daemon::_executor);
		_Daemon::_executor->enable(force);
		break;
	case Category::FileSynchronizer:
		if (_Daemon::_fileSynchronizer) {
			_Daemon::_fileSynchronizer->enable(force);
		}
		break;
	}
}

//	FUNCTION
//	Checkpoint::Daemon::disbale --
//		すべてのデーモンスレッドの行うべき処理を実行不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Checkpoint::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::enable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::disable(bool force)
{
	unsigned int i = 0;
	try {
		do {
			disable(static_cast<Category::Value>(i), force);

		} while (++i < Category::ValueNum);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		while (i--)
			enable(static_cast<Category::Value>(i));
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Checkpoint::Daemon::disable --
//		あるデーモンスレッドの行うべき処理を実行不可にする
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			処理を実行不可にするデーモンスレッドの種別
//		bool				force
//			true
//				Checkpoint::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::enable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::disable(Category::Value category, bool force)
{
	switch (category) {
	case Category::Executor:
		; _SYDNEY_ASSERT(_Daemon::_executor);
		_Daemon::_executor->disable(force);
		break;
	case Category::FileSynchronizer:
		if (_Daemon::_fileSynchronizer) {
			_Daemon::_fileSynchronizer->disable(force);
		}
		break;
	}
}

//
//	FUNCTION
//	Checkpoint::Daemon::signalEvent -- 終了イベントをシグナル化する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Daemon::signalEvent()
{
	_Daemon::_event.set();
}

//
//	FUNCTION
//	Checkpoint::Daemon::setExecuteFlag -- 実行中フラグを設定する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Daemon::setExecuteFlag(bool flag_)
{
	Os::AutoCriticalSection cAuto(_Daemon::_lock);

	_Daemon::_execute = flag_;
}

//
//	FUNCTION
//	Checkpoint::Daemon::isExecuting -- 実行中かどうか
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		bool
//			チェックポイントか同期処理が実行中ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Daemon::isExecuting()
{
	Os::AutoCriticalSection cAuto(_Daemon::_lock);

	return _Daemon::_execute;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
