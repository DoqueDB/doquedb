// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.cpp -- スレッドを管理するクラス
// 
// Copyright (c) 2000, 2001, 2003, 2006, 2014, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Thread.h"
#include "Common/Message.h"
#include "Exception/Unexpected.h"

#include "ModError.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::Thread::Thread -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Thread::Thread()
	: m_uiMessageThreadID(0)
{
}

//
//	FUNCTION public
//	Common::Thread::~Thread -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Thread::~Thread()
{
}

//
//	FUNCTION public
//	Common::Thread::create -- スレッドを起動する
//
//	NOTES
//	スレッドを起動する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Thread::create()
{
	m_bThrowException = false;
	m_cThread.create(entry, this);
}

//
//	FUNCTION public
//	Common::Thread::kill -- スレッドを強制終了する
//
//	NOTES
//	スレッドを強制終了する。
//	安全には終了出来ないので、通常は使用してはいけない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Thread::kill()
{
	m_cThread.kill();
}

//
//	FUNCTION public
//	Common::Thread::getThreadID -- スレッドIDを得る
//
//	NOTES
//	現在実行中のスレッドのIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModThreadId
//		スレッドID
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModThreadId
Thread::getThreadID() const
{
	return m_cThread.getThreadId();
}

//
//	FUNCTION public
//	Common::Thread::join -- スレッドの終了を待つ
//
//	NOTES
//	スレッドの終了を待つ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Thread::ExitStatus
//		スレッドの終了ステータス
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Thread::ExitStatus
Thread::join()
{
	m_cThread.join();
	return getExitStatus();
}

//
//	FUNCTION public
//	Common::Thread::getExitStatus -- スレッドの終了ステータスを得る
//
//	NOTES
//	スレッドの終了ステータスを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Thread::ExitStatus
//		スレッドの終了ステータス
//
//	EXCEPTIONS
//	なし
//
Thread::ExitStatus
Thread::getExitStatus() const
{
	ExitStatus eStatus;
	if (m_bThrowException == true)
	{
		//Sydneyの例外が投げられた
		eStatus = ThrowException;
	}
	else
	{
		const ModThread::ExitStatus& status = m_cThread.getExitStatus();
		//それ以外の場合はModのExitStatusをチェックする
		switch (status.getType())
		{
		case ModThread::terminateNormally:
			eStatus = Normally;
			break;
		case ModThread::exitThread:
			eStatus = ExitThread;
			break;
		case ModThread::throwException:
			eStatus = ThrowModException;
			break;
		case ModThread::killThread:
			eStatus = KillThread;
			break;
		default:
			eStatus = Unknown;
		}
	}
	return eStatus;
}

//
//	FUNCTION public
//	Common::Thread::getException -- Sydney例外を得る
//
//	NOTES
//	Sydneyの例外を得る。
//	終了ステータスがThrowExceptionの場合のみ有効な値が得られる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::Exception&
//		例外クラス。投げられる例外はこのクラスの派生クラスであるが、
//		ここで得られるものは基底クラスのインスタンスである。
//
//	EXCEPTIONS
//	なし
//
const ExceptionObject&
Thread::getException() const
{
	return m_cException;
}

//
//	FUNCTION public
//	Common::Thread::getModException -- Modの例外を得る
//
//	NOTES
//	Modの例外を得る。
//	終了ステータスがThrowModExceptionの場合のみ有効な値が得られる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModException&
//		Modの例外クラス
//
//	EXCEPTIONS
//	なし
//
const ModException&
Thread::getModException() const
{
	const ModThread::ExitStatus& status = m_cThread.getExitStatus();
	return *(status.getException());
}

//
//	FUNCTION public
//	Common::Thread::interrupt -- スレッドの中断要請
//
//	NOTES
//	スレッドに中断を要請する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		要求できた場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
Thread::interrupt()
{
	bool result = false;
	if (m_cThread.interrupt() == ModTrue)
		result = true;
	return result;
}

//
//	FUNCTION public
//	Common::Thread::abort -- スレッドの終了要請
//
//	NOTES
//	スレッドに終了を要請する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		要求できた場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
Thread::abort()
{
	bool result = false;
	if (m_cThread.abort() == ModTrue)
		result = true;
	return result;
}

//
//	FUNCTION public
//	Common::Thread::getStatus -- スレッドの動作状態を得る
//
//	NOTES
//	スレッドの動作状態を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Thread::Status
//		スレッドの動作状態
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Thread::Status
Thread::getStatus()
{
	Status stat;
	switch (m_cThread.getStatus())
	{
	case ModThread::notStarted:		//実行前
		stat = NotStarted;
		break;
	case ModThread::running:		//通常処理中
		stat = Running;
		break;
	case ModThread::interrupting:	//中断要請中
		stat = Interrupting;
		break;
	case ModThread::interrupted:	//中断処理中
		stat = Interrupted;
		break;
	case ModThread::aborting:		//終了要請中
		stat = Aborting;
		break;
	case ModThread::aborted:		//終了処理中
		stat = Aborted;
		break;
	case ModThread::exitSoon:		//まもなく終了
		stat = ExitSoon;
		break;
	case ModThread::exited:			//終了確認
		stat = Exited;
	}
	return stat;
}

//
//	FUNCTION public
//	Common::Thread::setStatus -- スレッドの状態を設定する
//
//	NOTES
//	スレッドの状態を設定する。
//
//	ARGUMENTS
//	Common::Thread::Status eStatus_
//		スレッド動作状態
//
//	RETURN
//	bool
//		設定できればtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
Thread::setStatus(Status eStatus_)
{
	ModThread::Status stat;
	switch (eStatus_)
	{
	case NotStarted:		//実行前
		stat = ModThread::notStarted;
		break;
	case Running:		//通常処理中
		stat = ModThread::running;
		break;
	case Interrupting:	//中断要請中
		stat = ModThread::interrupting;
		break;
	case Interrupted:	//中断処理中
		stat = ModThread::interrupted;
		break;
	case Aborting:		//終了要請中
		stat = ModThread::aborting;
		break;
	case Aborted:		//終了処理中
		stat = ModThread::aborted;
		break;
	case ExitSoon:		//まもなく終了
		stat = ModThread::exitSoon;
		break;
	case Exited:			//終了確認
		stat = ModThread::exited;
	}
	bool result = false;
	//ModThreadのメソッドを実行する
	if (m_cThread.setStatus(stat) == ModTrue)
		result = true;
	return result;
}

//
//	FUNCTION public
//	Common::Thread::isInterrupted -- 中断要請中または中断処理中かどうか
//
//	NOTES
//	スレッドが中断要請中または中断処理中かどうかを調べる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		中断中の場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//	なし
//
bool
Thread::isInterrupted() const
{
	bool result = false;
	if (m_cThread.isInterrupted() == ModTrue)
		result = true;
	return result;
}

//
//	FUNCTION public
//	Common::Thread::isAborted -- 終了要請中または終了処理中かどうか
//
//	NOTES
//	スレッドが終了要請中または終了処理中かどうかを調べる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		終了中の場合はtrue、それ以外の場合はfalseを返す。
//
//	EXCEPTIONS
//	なし
//
bool
Thread::isAborted() const
{
	bool result = false;
	if (m_cThread.isAborted() == ModTrue)
		result = true;
	return result;
}

//
//	FUNCTION private static
//	Common::Thread::entry -- スレッドとして起動されるエントリ関数
//
//	NOTES
//	スレッドとして起動されるエントリ関数。
//
//	ARGUMENTS
//	void* pArgument_
//		thisが渡される
//
//	RETURN
//	void*
//		常に0を返す。
//
//	EXCEPTIONS
//	ModException
//		Sydneyの例外をキャッチした場合はその後の処理のためにModの例外を投げる
//		ただし、そのエラー番号は不定である。
//	その他
//		下位の例外はそのまま再送
//
void*
Thread::entry(void* pArgument_)
{
	Thread* t = static_cast<Thread*>(pArgument_);
	try
	{
		// メッセージ用のスレッドIDを設定する
		t->m_uiMessageThreadID = Message::getThreadID();
		
		t->runnable();
	}
	catch (Exception::Object& e)
	{
		//例外をコピーする
		t->m_cException = e;
		t->m_bThrowException = true;
	}
	catch (ModException&)
	{
		//Modの例外はそのまま再送
		throw;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		//その他の例外はUnexpectedにする
		t->m_cException = Exception::Unexpected(moduleName,
												srcFile,
												__LINE__);
		t->m_bThrowException = true;
	}
#endif
	return 0;
}

//	FUNCTION public
//	Common::Thread::resetErrorCondition -- スレッドのエラー状態を解除する
//
//	NOTES
//		MOD の例外が発生すると、MOD スレッドはエラー状態になる
//
//		例外処理で、例外をキャッチした後、その例外を再送しなかったり、
//		新たな例外を送らないときは、
//		この関数によってエラー状態を解除しなければならない
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
Thread::resetErrorCondition()
{
	ModErrorHandle::reset();
}

//	FUNCTION public
//	Common::Thread::isErrorCondition -- スレッドのエラー状態を調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			呼び出しスレッドはエラー状態である
//		false
//			呼び出しスレッドはエラー状態でない
//
//	EXCEPTIONS
//		なし

// static
bool
Thread::isErrorCondition()
{
	return ModErrorHandle::isError();
}

//
//	Copyright (c) 2000, 2001, 2003, 2006, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
