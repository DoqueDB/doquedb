// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Client.cpp -- ロック要求元関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Lock/AutoItem.h"
#include "Lock/Client.h"
#include "Lock/Manager.h"

#include "Common/Assert.h"
#ifdef DEBUG
#include "Os/Thread.h"
#endif

#include "ModConditionVariable.h"
#include "ModTimeSpan.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{

namespace _Client
{
	// 初期化してからなん回目のデッドロック検出の試みか
	Client::DetectionID	_detectionCounter = 0;
}

}

//	FUNCTION private
//	Manager::Client::initialize --
//		マネージャーの初期化のうち、ロック要求元関連のものを行う
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

// static
void
Manager::Client::initialize()
{}

//	FUNCTION private
//	Manager::Client::terminate --
//		マネージャーの後処理のうち、ロック要求元関連のものを行う
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
Manager::Client::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	_Client::_detectionCounter = 0;
}

//	FUNCTION public
//	Lock::Client::Client -- ロック要求元を表すクラスのコンストラクター
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
Lock::Client::Client()
	: _request(0),
	  _stackRequest(0),
	  _waitingRequest(0),
	  _waitingDuration(Duration::Instant),
	  _waitingMode(Mode::N),
	  _condition(0),
	  _holder(0),
	  _loop(0)
{
#ifdef DEBUG
	this->m_ThreadID = Os::Thread::self();
#endif
}

//	FUNCTION public
//	Lock::Client::~Client -- ロック要求元を表すクラスのデストラクター
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
Client::~Client()
{
	// ロック待ち用の条件変数を破棄する

	delete _condition, _condition = 0;
}

//	FUNCTION public
//	Lock::Client::releaseStackRequest --
//		ロック要求元が SQL の 1 文でかけたロックをすべてはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Duration::Value	duration
//			はずすロックの持続期間
//		bool					isSucceeded
//			true
//				SQL 文の実行が成功した
//			false
//				SQL 文の実行が失敗した
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Client::releaseStackRequest(Duration::Value	duration,
							bool			isSucceeded)
{
	Os::AutoCriticalSection	latch(Manager::getLatch());

	if (_stackRequest) {

		// ロック要求元が SQL の 1 文でかけた
		// ロックを保持するロック要求をすべて処理する

		Request* request = _request;
		Request* next;

		do {
			next = (request == _stackRequest) ? 0 : request->_elder;

			if (isSucceeded) {

				// SQL 文が成功した場合には、ロックをはずさずに
				// SQL の 1 文でかけたロック数を初期化するだけにする

				/*【未実装】	持続期間を処理していない */

				Count::clear(request->_stackCount);

			} else {

				// SQL 文が失敗したときは、
				// SQL の 1 文でかけたロックをすべてはずす

				; _SYDNEY_ASSERT(request->_stackCount);

				Status::Value	sts;
				if ((sts = request->release(*request->_stackCount, duration)) != Status::Succeeded) return sts;

				if (!request->isGranted()) {

					// ロック要求に許可されているロックがないので、
					// ロック要求を破棄する

					Request::detach(request);
				}
			}

		} while (request = next) ;

		_stackRequest = 0;
	}

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::Client::releaseAll -- ロック要求元がかけているロックをすべてはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Duration::Value	duration
//			はずすロックの持続期間
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Client::releaseAll(Duration::Value	duration)
{
	Os::AutoCriticalSection	latch(Manager::getLatch());

	// ロック要求元がかけたロックを保持するロック要求をすべて処理する

	while (_request) {

		// ロック要求の指定された持続期間のロックをすべてはずす

		Status::Value	sts;
		if ((sts = _request->clear(duration)) != Status::Succeeded) return sts;

		if (!_request->isGranted()) {

			// ロック要求に許可されているロックがないので、
			// ロック要求を破棄する

			Request* tmp = _request;
			Request::detach(tmp);
		}
	}

	/*【未実装】	持続期間を処理していない */

	_stackRequest = 0;

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::Client::unlatchAll -- ロック要求元のラッチをすべてアンラッチする
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
Client::unlatchAll()
{
	// ラッチ中のロック項目を管理するベクターを保護するためにラッチする

	Os::AutoCriticalSection	latch(_latch);

	while (_latchedItem.getSize()) {

		// ベクタの末尾に登録されているロック項目を表すクラスを得て、
		// 参照数を 1 増やすことにより、
		// アンラッチしても参照数が 0 にならないようにする
		//
		//【注意】	末尾に登録されているものを処理するのは、
		//			先頭に登録されているものを処理したときに、
		//			ベクタから登録を抹消するのが遅いため

		AutoItem item(Item::attach(*_latchedItem.getBack()));

		// アンラッチする

		item->unlatch(*this);
	}
}

//	FUNCTION private
//	Lock::Client::proceedToWait -- ロック待ちする
//
//	NOTES
//		自分自身のロック要求が許可されるまで、
//		エグゼキューターを停止する
//
//	ARGUMENTS
//		Lock::Request*				request
//			ロック待ちするロック要求
//		Lock::Mode::Value			mode
//			ロック待ちする要求されているロックのモード
//		Lock::Duration::Value		duration
//			ロック待ちする要求されているロックの持続期間
//		Os::AutoTryCriticalSection&	latch
//			ラッチ
//		Lock::Timeout::Value		timeout = Lock::Timeout::Unlimited
//			ロック待ちの最大待ち時間(ミリ秒単位)
//			省略時には、無制限に待つ
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Client::proceedToWait(Request*						request,
					  Mode::Value					mode,
					  Duration::Value				duration,
					  Os::AutoTryCriticalSection&	latch,
					  Timeout::Value				timeout)
{
	if (!timeout) {

		// ロック待ちしないように指定された
		return Status::Timeout;
	}

	// ロック要求元をロック待ち状態にする

	_waitingRequest = request;
	_waitingDuration = duration;
	_waitingMode = mode;

	// ロック要求をロック待ち状態にする

	request->_condition = Request::Condition::Waiting;

	// ロック待ちするとデッドロックになるか調べる

	Status::Value	sts;
	if ((sts = this->detectDeadlock(this, _Client::_detectionCounter++)) != Status::Succeeded) {

		this->notifyRelease(request);

		return sts;
	}

	if (!_condition) {

		// ロック待ち用の条件変数を確保する

		_condition = new ModConditionVariable();
		; _SYDNEY_ASSERT(_condition != 0);

	} else {
		_condition->reset();
	}

	latch.unlock();

	// ロック待ちするためにエグゼキューターを停止する

	if (timeout == Timeout::Unlimited) {

		// 無制限に待つ…

		if (this->_condition->wait() == ModFalse) {

			// シグナル状態にならなかった

			latch.lock();
			this->notifyRelease(request);
			return Status::Timeout;
		}

	} else {

		if (!_condition->wait(ModTimeSpan(0, timeout))) {

			// 時間切れがおきた

			latch.lock();
			this->notifyRelease(request);
			return Status::Timeout;
		}
	}
	latch.lock();

	; _SYDNEY_ASSERT(!_waitingRequest);
	; _SYDNEY_ASSERT(_waitingDuration == Duration::Instant);
	; _SYDNEY_ASSERT(_waitingMode == Mode::N);
	
	switch (request->_condition) {
	case Request::Condition::Granted:

		// ロックがとれた

		break;

	case Request::Condition::Deadlocked:

		// デッドロックのために、
		// ロック待ちしていたロック要求は犠牲になった

		request->_condition = Request::Condition::Granted;

		return Status::Deadlock;

	default:
		; _SYDNEY_ASSERT(false);		/*【考察】今のところ未処理 */
	}

	return Status::Succeeded;
}

void
Client::notifyRelease(Request*	request_)
{
	_waitingRequest = 0;
	_waitingDuration = Duration::Instant;
	_waitingMode = Mode::N;

	request_->_condition = Request::Condition::Granted;
}

//	FUNCTION private
//	Lock::Client::notifyRelease --
//		あるロック要求元のロック待ちを解除する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client*					waiter
//			ロック待ちを解除するロック要求元
//		Lock::Request::Condition::Value	reason
//			ロック待ちが解除される理由
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Client::notifyRelease(Client*					waiter,
					  Request::Condition::Value	reason)
{
	; _SYDNEY_ASSERT(waiter);

	Request*	request = waiter->_waitingRequest;
	; _SYDNEY_ASSERT(request);

	if (reason == Request::Condition::Granted &&
		waiter->_waitingDuration > Duration::Instant) {

		// ロック待ちしていたロックが取れたので、
		// ロック待ちしていたモードのロックを実際に加算する

		const Mode::Value	lub = Mode::getLeastUpperBound(request->_granted,
														   waiter->_waitingMode);
		request->up(waiter->_waitingMode, lub);
	}

	// ロック要求元とロック要求のロック待ち状態を解除する

	waiter->_waitingRequest = 0;
	waiter->_waitingDuration = Duration::Instant;
	waiter->_waitingMode = Mode::N;

	// ロック待ち状態の解除理由をロック要求に設定する

	if ((request->_condition = reason) == Request::Condition::Deadlocked &&
		waiter == this) {

		// 実は自分自身がデッドロックの犠牲者になった
		return Status::Deadlock;
	}

	// ロック待ちで止まっているエグゼキューターを再開する

	; _SYDNEY_ASSERT(waiter->_condition);
	waiter->_condition->signal();

	return Status::Succeeded;
}

//	FUNCTION private
//	Lock::Client::detectDeadlock --
//		自分のロック待ちによるデッドロックの検出
//
//	NOTES
//		自分自身がロック待ちしようとするとき、
//		ロック待ちすると、デッドロックが起きるか調べる
//
//		デッドロックの検出は、ロック要求元の間で、
//		ロック待ち中のロック要求をすべて結んだ
//		待ちグラフ (wait for graph) を作り、
//		自分自身を端点とする閉路が存在するかどうかで調べる
//
//	ARGUMENTS
//		Lock::Client*				waiter
//			デッドロック検出を開始するロック待ちしている、
//			またはロック待ちしようとしているロック要求元
//		Lock::Client::DetectionID	loop
//			ロックマネージャーの初期化時から、
//			何回目のデッドロック検出の試みか
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Client::detectDeadlock(Client*		waiter,
					   DetectionID	loop)
{
	; _SYDNEY_ASSERT(waiter);

	// 与えられたロック要求元に、
	// 今回の検出試みループを記録しておく

	waiter->_loop = loop;

	// 現在ロック待ちしているロック要求が
	// 他のロック要求元のうち、どれのためにロック待ちしているか調べる

	Request* request = waiter->_waitingRequest;
	; _SYDNEY_ASSERT(request);
	const Mode::Value	lub =
		Mode::getLeastUpperBound(request->_granted, waiter->_waitingMode);

	for (request = request->_next; request != waiter->_waitingRequest; request = request->_next) {

		// 現在、ロック待ちしているロック要求が許可後のロックモードと、
		// 他のロック要求元の許可されているロックが競合するか調べる
		//
		//【注意】	他のロック要求元がロック待ちしているかどうかにかかわらず、
		//			競合するか検査しなければならない
		//
		//			たとえ、ロック待ちしていても、
		//			他のロック要求もとの許可済のロックと
		//			競合しなければ問題ないはず

		; _SYDNEY_ASSERT(request);
		if (Mode::isCompatible(request->_granted, lub)) {

			// 両立する

			continue;
		}

		// ロック待ちの原因となっているロック要求の
		// ロック要求元を得る

		Client*	holder = request->_client;
		if (!holder->_waitingRequest) {

			// そのロック要求元はロック待ちしていない

			continue;
		}

		waiter->_holder = holder;
		if (holder == this) {

			// ロック待ちしているロック要求元の間で
			// 自分を短点とする閉路(ループ)ができたので、
			// デッドロックになった

			Client*	victim = this->selectVictim();

			// 犠牲者にデッドロックが起き、
			// ロック待ちが解除されたことを通知する

			Status::Value	sts;
			if ((sts = this->notifyRelease(victim, Request::Condition::Deadlocked)) != Status::Succeeded) return sts;

		} else if (holder->_loop != loop) {

			// 今回の試みで一度もたどっていないロック要求元のとき、
			// それがロック待ちしているロック要求に関して、
			// これまでと同様にロック要求をたどって、
			// 自分自身からの閉路の存在を調べる
			//
			//【注意】	もし検出試みループが等しければ、
			//			すでにたどって閉路がないことがわかっている

			Status::Value	sts;
			if ((sts = this->detectDeadlock(holder, loop)) != Status::Succeeded) return sts;
		}
	}

	return Status::Succeeded;
}

//	FUNCTION private
//	Lock::Client::selectVictim -- デッドロックを解消するための犠牲者の選出
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		犠牲になるロック要求元
//
//	EXCEPTIONS
//		なし

Client*
Client::selectVictim()
{
	/*【考察】	いまのところ、必ず this が犠牲になる */

	return this;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
