// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Request.cpp -- ロック要求関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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
#include "Lock/Config.h"
#include "Lock/Manager.h"
#include "Lock/Request.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{

namespace _Request
{
	// 解放したインスタンスをつなぐリスト
	Request*				_freeList = 0;
	// リストの数
	ModSize					_freeListCount = 0;
}

}

//	FUNCTION private
//	Lock::Manager::Request::initialize --
//		マネージャーの初期化のうち、ロック要求関連のものを行う
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
Manager::Request::initialize()
{
}

//	FUNCTION private
//	Lock::Manager::Request::terminate --
//		マネージャーの後処理のうち、ロック要求関連のものを行う
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
Manager::Request::terminate()
{
	// 解放したインスタンスをつなぐリスト内のロック要求を破棄する
	Lock::Request::terminateFreeList();
}

//	FUNCTION public
//	Lock::Request::termnateFreeList --
//		解放したインスタンスをつなぐリスト内のロック要求を破棄する
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
Request::terminateFreeList()
{
	while (_Request::_freeList) {
		Request*	req = _Request::_freeList;
		_Request::_freeList = _Request::_freeList->_next;
		delete req;
	}
}

//	FUNCTION private
//	Lock::Request::Request -- ロック要求を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Item&		item
//			この対象であるロック項目
//		Lock::Client&	client
//			これを生成するロック要求元
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Request::Request(Item&		item,
				 Client&	client)
{
	initialize(item, client);
}

//	FUNCTION private
//	Lock::Request::~Request -- ロック要求を表すクラスのデストラクター
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

Request::~Request()
{
}

//	FUNCTION private
//	Lock::Request::initialize -- 初期化する
//
//	NOTES
//	
//	ARGUMENTS
//		Lock::Item&			item
//			このロック項目に対する client で指定される
//			ロック要求元によるロック要求を表すクラスを生成する
//		Lock::Client&		client
//			item で指定されるロック項目に対する
//			このロック要求元によるロック要求を表すクラスを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Request::initialize(Item& item, Client& client)
{
	// 【注意】
	//	このメソッドは Manager::getLatch() で保護されてることが前提

	_count = Count::attach(Count());
	; _SYDNEY_ASSERT(_count);
	_stackCount = _count->attach();
	; _SYDNEY_ASSERT(_stackCount == _count);

	_item = &item;
	_prev = 0;
	_next = 0;
	_client = &client;
	_condition = Request::Condition::Granted;
	_granted = Mode::N;

	if (_item->_queue) {
		this->_next = _item->_queue;
		this->_prev = _item->_queue->_prev;
		this->_next->_prev = this;
		this->_prev->_next = this;
	} else {
		_next = _prev = this;
	}

	_item->_queue = this;

	if (client._request) {
		this->_elder = _client->_request;
		this->_younger = _client->_request->_younger;
		this->_elder->_younger = this;
		this->_younger->_elder = this;
	} else {
		_elder = _younger = this;
	}

	client._request = this;
}

//	FUNCTION private
//	Lock::Request::terminate -- 終了処理を行う
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

void
Request::terminate()
{
	(_prev->_next = _next)->_prev = _prev;

	if (_item->_queue == this) {
		_item->_queue = (_next != this) ? _next : 0;
	}

	(_younger->_elder = _elder)->_younger = _younger;

	if (_client->_request == this) {
		_client->_request = (_elder != this) ? _elder : 0;
	}
	
	Count::detach(_stackCount);
	Count::detach(_count);
}

//	FUNCTION private
//	Lock::Request::attach -- ロック要求を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Item&		item
//			このロック項目に対する client で指定される
//			ロック要求元によるロック要求を表すクラスを生成する
//		Lock::Client&	client
//			item で指定されるロック項目に対する
//			このロック要求元によるロック要求を表すクラスを生成する
//
//	RETURN
//		生成したロック要求を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Request*
Request::attach(Item& item, Client& client)
{
	Os::AutoCriticalSection	latch(Manager::getLatch());

	Request* request = Request::find(item, client);
	if (!request) {

		// 見つからなかったので、生成する
		// フリーリストにインスタンスがあればそれを利用し、
		// なければ新たにインスタンスを生成する

		request = popFreeList();
		if (request) {
			// あったので初期化する

			request->initialize(item, client);

		} else {
			// なかったので生成する

			request = new Request(item, client);
		}
		; _SYDNEY_ASSERT(request);

		// 生成されたロック要求を表すクラスが存在する間に
		// 与えられたロック項目を表すクラスが破棄されると困るので、
		// 参照回数を 1 増やして、破棄されないようにする

		(void) Item::attach(item);

	} else {

		// 2005/05/19
		if (client._request != request) {
			request->_younger->_elder = request->_elder;
			request->_elder->_younger = request->_younger;
			request->_younger = client._request->_younger;
			request->_elder = client._request;
			client._request->_younger = request;
			request->_younger->_elder = request;
			client._request = request;
		}
	}

	return request;
}

//	FUNCTION private
//	Lock::Request::detach -- ロック要求を表すクラスを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Request*&	request
//			破棄するロック要求を表すクラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Request::detach(Request*& request)
{
	if (request) {
		
		// ロック項目は破棄するので取り出す
		
		Item* item = request->_item;

		{
			Os::AutoCriticalSection	latch(Manager::getLatch());
			
			// ロック要求の終了処理を実行する
		
			request->terminate();

			// ロック要求をフリーリストにつなぐ
		
			pushFreeList(request);
		}

		// 与えられたポインタは 0 をさすようにする
		
		request = 0;

		// ロック項目を破棄する
		
		Item::detach(item);
	}
}

//	FUNCTION private
//	Lock::Request::poFreepList -- キャッシュしているインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::Request*
//		キャッシュしているインスタンス。存在しない場合は0。
//
//	EXCEPTIONS

// static
Request*
Request::popFreeList()
{
	// 【注意】
	//	このメソッドは Manager::getLatch() で保護されてることが前提
	
	Request* request = 0;
	if (_Request::_freeList) {
		request = _Request::_freeList;
		_Request::_freeList = _Request::_freeList->_next;

		--_Request::_freeListCount;
	}

	return request;
}

//	FUNCTION
//	Lock::Request::pushFreeList -- 不要になったインスタンスをフリーリストにつなぐ
//
//	NOTES
//
//	ARGUMENTS
//	Lock::Request* request_
//		不要になったインスタンス
//
//	RETURN
//
//	EXCEPTIONS

// static
void
Request::pushFreeList(Request* request_)
{
	// 【注意】
	//	このメソッドは Manager::getLatch() で保護されてることが前提
	
	if (_Request::_freeListCount < Config::RequestInstanceCacheSize::get()) {
		request_->_next = _Request::_freeList;
		_Request::_freeList = request_;
		
		++_Request::_freeListCount;
	} else {
		delete request_;
	}
}

//	FUNCTION private
//	Lock::Request::find --
//		あるロック項目に対するあるロック要求元によるロック要求を探す
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Item&	item
//			このロック項目に対する
//			client で指定されるロック要求元によるロック要求を探す
//		const Lock::Client&	client
//			item で指定されるロック項目に対する
//			このロック要求元によるロック要求を探す
//
//	RETURN
//		0 以外の値
//			見つかったロック要求を格納する領域の先頭アドレス
//		0
//			ロック要求は見つからなかった
//
//	EXCEPTIONS
//		なし

// static
Request*
Request::find(const Item& item, const Client& client)
{
	//【注意】	Manager::getLatch をラッチ済であること

	if (Request* request = item._queue) {
		do {
			if (request->_client == &client) {
				if (request != item._queue) {

					// 次に同じ物を探しにきたときに、
					// 即、見つかるように、リストの先頭へ移動しておく

					request->_prev->_next = request->_next;
					request->_next->_prev = request->_prev;

					(request->_prev = item._queue->_prev)->_next = request;
					(request->_next = item._queue)->_prev = request;
				}
				return request;
			}
		} while ((request = request->_next) != item._queue) ;
	}

	return 0;
}

//	FUNCTION private
//	Lock::Request::hold --
//		ロック要求にロックをかける
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value		mode
//			加算するロックのモード
//		Lock::Duration::Value	duration
//			加算するロックの持続期間
//		Lock::Timeout::Value	timeout = Lock::Timeout::Unlimited
//			加算時にロック待ちするとき、その最大待ち時間(ミリ秒単位)
//			省略時には、無制限に待つ
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Request::hold(Mode::Value		mode,
			  Duration::Value	duration,
			  Timeout::Value	timeout)
{
	Os::AutoTryCriticalSection	latch(Manager::getLatch());
	latch.lock();

	; _SYDNEY_ASSERT(_condition == Request::Condition::Granted);

	// 指定されたモードでロックしたときの
	// 新しい有効なロックモードを求める

	Mode::Value	lub = Mode::getLeastUpperBound(_granted, mode);

	if (_granted != lub) {

		// 指定されたモードでロックすると、
		// 有効なロックモードが変化する
		// つまり、許可されていないロックを新たに要求することになる

		if (Config::LackOfParentDetection::get() &&
			!this->isAllowableChild(lub)) {

			// ロック要求元が親に許可されているロックでは、
			// 新しい有効なロックモードでロックできない

			return Status::LackOfParent;
		}

		// 新しい有効なロックモードでロックすると、
		// 他のロック要求元のロックと競合しないか調べる

		if (!this->isCompatible(lub)) {

			// 競合するので、競合しなくなるまで待つことにする

			// isCompatibleでチェックしているので、
			// デッドロックは起こらないはず
			return _client->proceedToWait(this, mode, duration, latch, timeout);

			// ロック待ちが終わると、指定されたモードでロックされている
		}
	}

	if (duration > Duration::Instant) {

		// 与えられたモードのロックを実際に加算する

		up(mode, lub);
	}

	return Status::Succeeded;
}

//	FUNCTION private
//	Lock::Request::release -- ロック要求のロックをはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value		mode
//			減算するロックのモード
//		Lock::Duration::Value	duration
//			減算するロックの持続期間
//		Lock::Count::Value		n
//			指定されたとき
//				指定されたモードのロックをいくつ減算するか
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Request::release(Mode::Value mode, Duration::Value duration, Count::Value n)
{
	Count count;
	count.up(mode, n);
	return release(count, duration);
}

//	FUNCTION private
//	Lock::Request::release -- ロック要求のロックをはずす
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Count&		count
//			減算するロックのロック数
//		Lock::Duration::Value	duration
//			減算するロックの持続期間
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Request::release(const Count& count, Duration::Value duration)
{
	Os::AutoCriticalSection	latch(Manager::getLatch());

	; _SYDNEY_ASSERT(_condition == Request::Condition::Granted);

	// はずした後のロックを求める

	; _SYDNEY_ASSERT(_count);
	Count downed(*_count);
	downed -= count;

	// 指定されたモードのロックをはすした後の
	// ひとつのロック要求元について有効なロックモードを求める
	//
	//【注意】	ロックのダウングレードが可能かで計算の仕方が異なる
	//
	//			可能な場合は、はずした後のロックの最小上界とする
	//			不可な場合は、はずした後のロックが Mode::N でなければ
	//			そのままにし、そうでなければ Mode::N にする

	const Mode::Value lub = (Config::EnableDowngrade::get() ? *downed :
							 (*downed == Mode::N ? Mode::N : _granted));

	const bool changed = (_granted != lub);
	if (changed) {

		// 指定されたモードのロックをはずすと、
		// ひとつのロック要求元について有効なロックモードが変化する
		// つまり、ひとつのロック要求元に許可されていた
		// ロックの一部またはすべてをはずすことになる

		if (Config::LackForChildDetection::get() &&
			!isAllowableParent(lub)) {

			// 新しい有効なロックモードでは、
			// ロック要求元が子に許可されているロックが認められない

			return Status::LackForChild;
		}

		// 実際に有効なロックモードを変更する

		_granted = lub;
	}

	// すべてのロック要求元のロック総数を実際に減算する

	Count::down(_item->_count, count);

	// 指定されたモードのロックを実際に減算する

	Count::substitute(_count, downed);
	Count::down(_stackCount, count);

	if (changed) {

		// 競合している他のロック要求元のロックが競合しなくなるか調べる

		return _item->checkCompatible();
	}

	return Status::Succeeded;
}

//	FUNCTION private
//	Lock::Request::clear -- ロック要求にあるロックをすべてなくす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Duration::Value	duration
//			すべてなくすロックの持続期間
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Request::clear(Duration::Value duration)
{
	//【注意】	Manager::getLatch をラッチ済であること

	; _SYDNEY_ASSERT(_condition == Request::Condition::Granted);

	if (isGranted()) {

		// ひとつのロック要求元に許可されていた
		// ロックをすべてはずすので、有効なロックモードが変化する

		if (Config::LackForChildDetection::get() &&
			!isAllowableParent(Mode::N)) {

			// ロック要求元は子にロックを許可されているので、
			// ロックをすべて外せない

			return Status::LackForChild;
		}

		// 実際に有効なロックモードを変更する

		_granted = Mode::N;

		// すべてのロック要求元のロック総数を実際に減算する

		; _SYDNEY_ASSERT(_count);
		Count::down(_item->_count, *_count);

		// 許可されているロックを実際にすべてはずす

		Count::clear(_count);
		Count::clear(_stackCount);

		// 競合している他のロック要求元のロックが競合しなくなるか調べる

		return _item->checkCompatible();
	}

	return Status::Succeeded;
}

//	FUNCTION private
//	Lock::Request::convert --
//		ロック要求のあるモードのロックを他のモードに変換する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value		from
//			変換元のロックのモード
//		Lock::Duration::Value	duration
//			変換元のロックの持続期間
//		Lock::Mode::Value		to
//			変換先のロックのモード
//		Lock::Timeout::Value	timeout = Lock::Timeout::Unlimited
//			変換先のモードのロックの加算時にロック待ちするとき、
//			その最大待ち時間(ミリ秒単位)
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Request::convert(Mode::Value		from,
				 Duration::Value	duration,
				 Mode::Value		to,
				 Timeout::Value		timeout)
{
	if (from == to) return Status::Succeeded;

	// 変換元のモードと変換後のモードが等しくなければ、
	// まず、変換先のモードでロックしてから、
	// 変換元のモードのロックをはずす
	//
	//【注意】	変換元のモードのロックがないためになにもはずさなくても、
	//			変換後のモードでロックする

	Status::Value	sts;
	if ((sts = hold(to, duration, timeout)) == Status::Succeeded) sts = release(from, duration);
	return sts;
}

//	FUNCTION private
//	Lock::Request::up --
//		ロック要求にあるモードのロックを加算する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value		mode
//			加算するロックのモード
//		Lock::Mode::Value		lub
//			ロック要求にロックを加算後の有効なロックモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Request::up(Mode::Value mode, Mode::Value lub)
{
	// 同じロック項目に対するロック総数に指定されたモードを加算し、
	// 同じロック項目に対する許可されているロックの最小上界を求めなおす

	Count::up(_item->_count, mode, 1);
	_item->_granted = Mode::getLeastUpperBound(_item->_granted, lub);

	// 指定されたモードのロックを実際に加算する

	Count::up(_count, mode, 1);
	_granted = lub;

	// 1 つの SQL 文ごとにスタックするロック数も加算する

	Count::up(_stackCount, mode, 1);

//	if (_client->_request != this && _client->_stackRequest != this) {

		// 現在実行中の SQL 文でロックを加算したロック要求を
		// _client._request から Request::_elder をたどって
		// _client._stackRequest までたどることで取得可能にするため、
		// 同じロック要求元によるロック要求のリストの先頭へ移動する

//		(_younger->_elder = _elder)->_younger = _younger;
//		_younger = (_elder = _client->_request)->_younger;
//		_elder->_younger = _younger->_elder = this;
//	}

	if (!_client->_stackRequest) {

		// ロック要求元が実行している SQL 文で
		// 初めてロックを加算したので、ロック要求をおぼえておく

		_client->_stackRequest = this;
	}
}

//	FUNCTION private
//	Lock::Request::isCompatible --
//		新しいひとつのロック要求元について有効なロックモードが
//		すべてのロック要求について有効なロックモードと両立するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			新しいひとつのロック要求元について有効なロックモード
//
//	RETURN
//		true
//			両立する
//		false
//			両立しない
//
//	EXCEPTIONS
//		なし

bool
Request::isCompatible(Mode::Value mode) const
{
	; _SYDNEY_ASSERT(_item->_count);
	Count count(*_item->_count);
	; _SYDNEY_ASSERT(_count);
	count -= *_count;
	return Mode::isCompatible(*count, mode);
}

//	FUNCTION private
//	Lock::Request::isAllowableChild --
//		ロックするロック項目に親が存在するとき、
//		ロック項目をあるモードでロックするのに、
//		ロック要求元はその親を十分にロックしているか
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			親のロックが十分か調べるロック項目に
//			かけようとしているロックのモード
//
//	RETURN
//		true
//			指定されたモードでロックするのに
//			十分なモードで親をロックしている
//		false
//			不足である
//
//	EXCEPTIONS

bool
Request::isAllowableChild(Mode::Value mode) const
{
	Name parent(_item->_name.getParent());
	if (parent == Name()) {

		// ロックするロック項目は最上位なので、ロックできる

		return true;
	}
	do {
		// 今調べている上位のロック項目に対する
		// ロック要求元のロック要求が存在するか調べ、
		// 存在すれば、その許可済のロックが
		// 指定されたモードのロックを認めるのに十分か調べる

		AutoItem item(Item::attach(parent));
		Request* request = find(*item, *_client);
		if (request && Mode::isPossible(request->_granted, mode)) {
			return true;
		}

	} while ((parent = parent.getParent()) != Name()) ;

	// 最上位のロック項目まで調べたが、
	// 指定されたモードのロックを認めるのに十分でなかった

	return false;
}

//	FUNCTION private
//	Lock::Request::isAllowableParent --
//		ロック要求元があるロック項目のロックをはずした後に
//		有効なロックモードが、その子の有効なロックモードを
//		保持するのに十分か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			ロックをはずした後に有効なロックモード
//
//	RETURN
//		true
//			指定されたモードは、子のロックを保持するのに十分である
//		false
//			不足である
//
//	EXCEPTIONS
//		なし

bool
Request::isAllowableParent(const Mode::Value	mode) const
{
	// 同じロック要求元のロック要求ごとに調べていく

	for (Request* request = _elder;
		 request != this; request = request->_elder) {

		// 今調べているロック要求のロック対象が、
		// 自分自身のロック対象の子孫であるとき、
		// 自分自身について有効なロックモードが
		// 子孫について有効なロックモードを保持するのに十分か調べる

		; _SYDNEY_ASSERT(request);
		if (request->_item->_name.isDescendant(_item->_name) &&
			!Mode::isPossible(mode, request->_granted)) {
			return false;
		}
	}
	return true;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
