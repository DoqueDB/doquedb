// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Client.h -- ロック要求元関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_CLIENT_H
#define	__SYDNEY_LOCK_CLIENT_H

#include "Lock/Module.h"
#include "Lock/Request.h"
#include "Lock/Status.h"

#include "Os/AutoCriticalSection.h"

#include "ModVector.h"

class ModConditionVariable;

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::Client -- ロック要求元を表すクラス
//
//	NOTES

class Client
{
	friend class Request;
	friend class Item;
	friend class LatchableItem;
public:
	//	TYPEDEF
	//	Lock::Client::DetectionID --
	//		デッドロック検出の試みを一意に識別する値の型
	//
	//	NOTES

	typedef	unsigned long	DetectionID;

	// コンストラクター
	SYD_LOCK_FUNCTION
	Client();
	// デストラクター
	SYD_LOCK_FUNCTION
	~Client();

	// SQL の 1 文でかけたロックをすべてはずす
	SYD_LOCK_FUNCTION
	Status::Value			releaseStackRequest(Duration::Value	duration,
												bool			isSucceeded);
	// かけているロックをすべてはずす
	SYD_LOCK_FUNCTION
	Status::Value			releaseAll(Duration::Value duration);

	// ラッチをすべてアンラッチする
	SYD_LOCK_FUNCTION
	void					unlatchAll();

private:
	// ロック要求が許可されるまで、ロック待ちを行う
	Status::Value			proceedToWait(Request*						request,
										  Mode::Value					mode,
										  Duration::Value				duration,
										  Os::AutoTryCriticalSection&	latch,
										  Timeout::Value				timeout = Timeout::Unlimited);

	// このロック要求元のロック待ちを解除する
	void
	notifyRelease(Request*	request_);
	Status::Value			notifyRelease(Request::Condition::Value	reason);
	Status::Value			notifyRelease(Client*					waiter,
										  Request::Condition::Value	reason);

	// このロック要求元のロック待ちによるデッドロックの検出
	Status::Value			detectDeadlock(Client*				waiter,
										   Client::DetectionID	loop);

	// デッドロックを解消するための犠牲者を選出する
	Client*					selectVictim();

	// このロック要求元によるロック要求
	Request*				_request;
	// 1 つの SQL 文の中でスタックされているロック要求
	Request*				_stackRequest;

	// 以下のメンバーを保護するためのラッチ
	Os::CriticalSection		_latch;
	// ラッチ中のロック項目を管理するベクター
	ModVector<Item*>		_latchedItem;

	// 以下のメンバーはロック待ち時に使用する

	// ロック待ち中のロック要求
	Request*				_waitingRequest;
	// 要求中のロックの持続期間
	Duration::Value			_waitingDuration;
	// 要求中のロックのモード
	Mode::Value				_waitingMode;
	// ロック待ちに使用する条件変数
	ModConditionVariable*	_condition;

	// 以下のメンバーはデッドロック検出に使用する

	// ロック待ちの相手
	Client*					_holder;
	// 最後のデッドロック検出番号
	DetectionID				_loop;

#ifdef DEBUG
	// ロック要求元を生成したスレッドのスレッド識別子
#ifdef SYD_OS_WINDOWS
	DWORD		m_ThreadID;
#endif
#ifdef SYD_OS_POSIX
	pthread_t	m_ThreadID;
#endif
#endif
};

//	FUNCTION private
//	Lock::Client::notifyRelease --
//		自分自身のロック待ちを解除する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Request::Condition::Value	reason
//			ロック待ちが解除される原因
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

inline
Status::Value
Lock::Client::notifyRelease(Lock::Request::Condition::Value	reason)
{
	return notifyRelease(this, reason);
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_CLIENT_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
