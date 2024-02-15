// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.h -- スレッドを管理するクラス
// 
// Copyright (c) 2000, 2001, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_THREAD_H
#define __TRMEISTER_COMMON_THREAD_H

#include "Common/Object.h"
#include "Common/ExceptionObject.h"

#include "ModThread.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::Thread -- スレッドを管理するクラス
//
//	NOTES
//	スレッドを管理するクラス。
//	ModThreadをそのまま使うと使いにくいので、スレッドを起動するクラスは、
//	このクラスを継承して実装する。
//

class SYD_COMMON_FUNCTION Thread : public Common::Object
{
public:
	//
	//	ENUM
	//	Common::Thread::Status -- スレッドの動作状態を表す
	//
	//	NOTES
	//	スレッドの動作状態を表す。ModThreadと同じ。
	//
	enum Status
	{
		NotStarted	= 0,	//実行前
		Running,			//通常処理中
		Interrupting,		//中断要請中
		Interrupted,		//中断処理中
		Aborting,			//終了要請中
		Aborted,			//終了処理中
		ExitSoon,			//間もなく終了
		Exited				//終了確認
	};
	
	//
	//	ENUM
	//	Common::Thread::ExitStatus -- スレッドの終了ステータスを表す
	//
	//	NOTES
	//	スレッドの終了ステータスを表す。
	//
	enum ExitStatus
	{
		Normally,			//正常終了
		ExitThread,			//終了要請により終了
		ThrowException,		//例外が発生した
		ThrowModException,	//Modの例外が発生した
		KillThread,			//強制終了
		Unknown				//不明
	};

	//コンストラクタ
	Thread();
	//デストラクタ
	virtual ~Thread();

	//スレッドを起動する
	void create();
	//強制終了(安全に終了することはできない)
	void kill();
	//スレッドIDを得る
	ModThreadId getThreadID() const;
	//メッセージ用のスレッドIDを得る(スレッド起動後に有効)
	unsigned int getMessageThreadID() const
		{ return m_uiMessageThreadID; }
	//スレッドの終了を待つ
	ExitStatus join();
	//終了ステータスを得る(joinしたあとのみ有効)
	ExitStatus getExitStatus() const;
	//TRMeisterの例外を得る(終了ステータスがThrowExceptionの場合のみ有効)
	const ExceptionObject& getException() const;
	//Modの例外を得る(終了ステータスがThrowModExceptionの場合のみ有効)
	const ModException& getModException() const;

	//スレッドの中断要請
	bool interrupt();
	//スレッドの終了要請
	bool abort();

	//スレッドの動作状態を得る
	Status getStatus();
	//スレッドの動作状態を設定する
	bool setStatus(Status eStatus_);

	// スレッドのエラー状態を解除する
	static void
	resetErrorCondition();
	// スレッドのエラー状態を調べる
	static bool
	isErrorCondition();

	//中断要請中または中断処理中かどうか調べる
	bool isInterrupted() const;
	//終了要請中または終了処理中かどうかを調べる
	bool isAborted() const;

protected:
	//スレッドとして起動されるメソッド
	virtual void runnable() = 0;

private:
	//スレッドとして起動される関数。runnableを実行する。
	static void* entry(void* pArgument_);

	//スレッドで発生した例外のコピー
	ExceptionObject m_cException;
	//例外が発生したかどうか
	bool m_bThrowException;

	//Modのスレッドクラス
	ModThread m_cThread;
	//メッセージ用のスレッドID
	unsigned int m_uiMessageThreadID;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_THREAD_H

//
//	Copyright (c) 2000, 2001, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
