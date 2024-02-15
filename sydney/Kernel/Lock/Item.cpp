// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Item.cpp -- ロック項目関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
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

#include "Lock/Client.h"
#include "Lock/Config.h"
#include "Lock/Item.h"
#include "Lock/Manager.h"
#include "Lock/Request.h"

#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{

typedef Common::HashTable<Common::DoubleLinkedList<Item>, Item> _HashTable;

namespace _Item
{
	// すべてのロック項目を表すクラスを管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int			itemTableHash(const Name&	name);

	// 生成済のロック項目を表すクラスを探す
	Item*					find(_HashTable::Bucket&	bucket,
								 const Name&			name);

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// すべてのロック項目を表すクラスを管理するハッシュ表
	_HashTable*				_itemTable = 0;
	_HashTable::Length		_itemTableLength = 0;

	// 解放したインスタンスをつなぐリスト
	// ※ latchable ではない item
	Item*					_freeList = 0;
	// リストの数 (_freeList)
	ModSize					_freeListCount = 0;

	// 解放したインスタンスをつなぐリスト
	// ※ latchable な item
	Item*					_latchableFreeList = 0;
	// リストの数 (_latchableFreeList)
	ModSize					_latchableFreeListCount = 0;
}

//	FUNCTION
//	$$$::_Item::itemTableHash --
//		すべてのロック項目を表すクラスを管理する
//		ハッシュ表に登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	name
//			ハッシュ表に登録するロック項目を表すクラスを識別するためのロック名
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Item::itemTableHash(const Name&	name)
{
	return *name;
}

//	FUNCTION
//	$$$::_Item::find -- 生成済のあるロック項目を表すクラスを探す
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_HashTable::Bucket&	bucket
//			ロック項目を表すクラスが格納されるべきハッシュ表のバケット
//		const Lock::Name&			name
//			ロック項目を表すクラスを識別するロック名
//
//	RETURN
//		0 以外の値
//			得られたロック項目を表すクラスを格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS

Item*
_Item::find(_HashTable::Bucket&	bucket,
			const Name&			name)
{
	//【注意】	呼び出し側で _Item::_latch をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているあるロック項目を表すクラスのうち、
		// 指定されたロック名で識別されるものを探す

		//_HashTable::Bucket::Iterator		begin(bucket.begin());
		//_HashTable::Bucket::Iterator		ite(begin);
		_HashTable::Bucket::Iterator		ite = bucket.begin();
		const _HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			Item& item = *ite;

			if (item.getName() == name) {

				// 見つかったロック項目を表すクラスを
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(bucket.begin(), bucket, ite);

				return &item;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Item& item = bucket.getFront();

		if (item.getName() == name) {

			// 見つかった

			return &item;
		}

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

}

//	FUNCTION private
//	Lock::Manager::Item::initialize --
//		マネージャーの初期化のうち、ロック項目関連のものを行う
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
Manager::Item::initialize()
{
	Lock::Item::initializeItemTable();
}

//	FUNCTION private
//	Lock::Manager::Item::terminate --
//		マネージャーの後処理のうち、ロック項目関連のものを行う
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
Manager::Item::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_Item::_itemTable) {

		// すべてのロック項目を表すクラスを管理する
		// ハッシュ表のバケットはすべて空であるべき

		; _SYDNEY_ASSERT(_Item::_itemTable->isEmpty());

		// すべてのロック項目を表すクラスを管理するハッシュ表を破棄する

		delete _Item::_itemTable, _Item::_itemTable = 0;
	}

	// 解放したインスタンスをつなぐリスト内のロック項目を破棄する

	Lock::Item::terminateFreeList();
}

// static
void
Item::initializeItemTable()
{
	_Item::_itemTable =	new _HashTable(Config::HashSize::get(),
									   &Item::_prev,
									   &Item::_next);
	; _SYDNEY_ASSERT(_Item::_itemTable);
	_Item::_itemTableLength = _Item::_itemTable->getLength();
}

//	FUNCTION public
//	Lock::Item::termnateFreeList --
//		解放したインスタンスをつなぐリスト内のロック項目を破棄する
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
Item::terminateFreeList()
{
	while (_Item::_latchableFreeList) {
		Item*	item = _Item::_latchableFreeList;
		_Item::_latchableFreeList = _Item::_latchableFreeList->_next;
		delete item;
	}
	while (_Item::_freeList) {
		Item*	item = _Item::_freeList;
		_Item::_freeList = _Item::_freeList->_next;
		delete item;
	}
}

//	FUNCTION public
//	Lock::Item::Item -- ロック項目を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	name
//			生成するロック項目を表すクラスを識別するためのロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Item::Item(const Name&	name)
	: _name(name),
	  _refCount(0),
	  _prev(0),
	  _next(0),
	  _queue(0),
	  _count(0),
	  _granted(Mode::N)
{
	_count = Count::attach(Count());
	; _SYDNEY_ASSERT(_count);
}

//	FUNCTION private
//	Lock::Item::~Item -- ロック項目を表すクラスのデストラクター
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

Item::~Item()
{
	Count::detach(_count);
}

//	FUNCTION public
//	Lock::Item::attach -- ロック項目を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	name
//			得ようとしているロック項目を表すクラスを一意に識別するロック名
//
//	RETURN
//		得られたロック名を表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Item*
Item::attach(const Name&	name)
{
	// ロック項目を表すクラスの生成・破棄に関する情報を
	// 保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_Item::_latch);

//	if (!_Item::_itemTable) {

		// すべてのロック項目を表すクラスを管理する
		// ハッシュ表が確保されていないので、まず、確保する

//		_Item::_itemTable =	new _HashTable(
//			Config::HashSize::get(), &Item::_prev, &Item::_next);
//		; _SYDNEY_ASSERT(_Item::_itemTable);
//	}

	// 与えられたロック名で識別されるロック項目を表すクラスを
	// 格納すべきハッシュ表のバケットを求める

	const unsigned int addr =
		_Item::itemTableHash(name) % _Item::_itemTableLength;	//_Item::_itemTable->getLength();
	_HashTable::Bucket& bucket = _Item::_itemTable->getBucket(addr);

	// 与えられたロック名で識別されるロック項目を表すクラスが
	// 求められたバケットに登録されていれば、それを得る

	Item* item = _Item::find(bucket, name);
	if (item) {

		// 見つかったので、参照回数を 1 増やす

		++item->_refCount;

	} else {
		// 見つからなかったので、生成する
		// フリーリストにインスタンスがあればそれを利用し、
		// なければ新たにインスタンスを生成する

		item = popFreeList(name);
		if (item) {
			// あったので初期化する
			item->initialize(name);
		} else {
			// なかったので生成する
			switch (name.getCategory()) {
			case Name::Category::LogicalLog:
			case Name::Category::File:
				item = new LatchableItem(name);	break;
			default:
				item = new Item(name);			break;
			}
		}
		; _SYDNEY_ASSERT(item);

		// 参照回数を 1 にする

		item->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*item);
	}

	return item;
}

//	FUNCTION private
//	Lock::Item::attach -- ロック項目を表すクラスの参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Item&	item
//			参照数を 1 増やすロック項目を表すクラス
//
//	RETURN
//		参照数が 1 増えたロック項目を表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Item*
Item::attach(const Item&	item)
{
	// ロック項目を表すクラスの生成・破棄に関する情報を
	// 保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_Item::_latch);

	// 参照回数を 1 増やす

	++item._refCount;

	return &const_cast<Item&>(item);
}

//	FUNCTION private
//	Lock::Item::detach -- ロック項目を表すクラスの参照数を 1 減らす
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Item&	item
//			参照数を 1 減らすロック項目を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Item::detach(const Item&	item)
{
	// ロック項目を表すクラスの生成・破棄に関する情報を
	// 保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_Item::_latch);

	// 参照回数を 1 減らす

	--item._refCount;

	// 必ずどこからか参照されていなければならない

	; _SYDNEY_ASSERT(item.isAttached());
}

//	FUNCTION private
//	Lock::Item::popFreepList -- キャッシュしているインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	name
//			ロック項目を表すクラスを識別するロック名
//
//	RETURN
//	Lock::Item*
//		キャッシュしているインスタンス。存在しない場合は0。
//
//	EXCEPTIONS

// static
Item*
Item::popFreeList(const Name&	name_)
{
	// 【注意】
	//	このメソッドの呼び出しはすべて _Item::_latch で保護されてるので、
	//	ここでは保護しない

	Item* item = 0;
	switch (name_.getCategory()) {
	case Name::Category::File:
	case Name::Category::LogicalLog:
		if (_Item::_latchableFreeList) {
			item = _Item::_latchableFreeList;
			_Item::_latchableFreeList = _Item::_latchableFreeList->_next;
			--_Item::_latchableFreeListCount;
		}
		break;
	default:
		if (_Item::_freeList) {
			item = _Item::_freeList;
			_Item::_freeList = _Item::_freeList->_next;

			--_Item::_freeListCount;
		}
		break;
	}

	return item;
}

//	FUNCTION
//	Lock::Item::pushFreeList -- 不要になったインスタンスをフリーリストにつなぐ
//
//	NOTES
//
//	ARGUMENTS
//	Lock::Item* item_
//		不要になったインスタンス
//
//	RETURN
//
//	EXCEPTIONS

// static
void
Item::pushFreeList(Item*	item_)
{
	// 【注意】
	//	このメソッドの呼び出しはすべて _Item::_latch で保護されてるので、
	//	ここでは保護しない

	if ((_Item::_freeListCount + _Item::_latchableFreeListCount) < Config::ItemInstanceCacheSize::get()) {
		switch (item_->getName().getCategory()) {
		case Name::Category::File:
		case Name::Category::LogicalLog:
			item_->_next = _Item::_latchableFreeList;
			_Item::_latchableFreeList = item_;
			++_Item::_latchableFreeListCount;
			break;
		default:
			item_->_next = _Item::_freeList;
			_Item::_freeList = item_;
			++_Item::_freeListCount;
			break;
		}
	} else {
		delete item_;
	}
}

//	FUNCTION public
//	Lock::Item::detach -- ロック項目を表すクラスの参照をやめる　
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Item*&	item
//			参照をやめるロック項目を表すクラスを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Item::detach(Item*&	item)
{
	if (item) {

		// ロック項目を表すクラスの生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection	latch(_Item::_latch);

		if (item->isAttached()) {

			// 参照数を 1 減らす

			--item->_refCount;
		}
		if (!item->isAttached()) {

			// どこからも参照されなくなったので、破棄できる

			; _SYDNEY_ASSERT(!item->_queue);

			// 与えられたロック項目を表すクラスを格納する
			// ハッシュ表のバケットを求め、
			// ロック項目を表すクラスを取り除く
			//
			//【注意】	バケットは _Item::_latch で保護される

			; _SYDNEY_ASSERT(_Item::_itemTable);
			const unsigned int addr =
				_Item::itemTableHash(item->getName()) % _Item::_itemTableLength;
					//_Item::_itemTable->getLength();
			_HashTable::Bucket& bucket = _Item::_itemTable->getBucket(addr);

			bucket.erase(*item);

			Count::detach(item->_count);

			// ロック項目を表すクラスをフリーリストにつなぐ

			pushFreeList(item);
		}

		// 与えられたポインタは 0 をさすようにする

		item = 0;
	}
}

//	FUNCTION public
//	Lock::Item::hold -- あるロック項目にロックをかける
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&			client
//			かけるロックのロック要求元
//		Lock::Mode::Value		mode
//			かけるロックのモード
//		Lock::Duration::Value	duration
//			かけるロックの持続期間
//		Lock::Timeout::Value	timeout = Lock::Timeout::Unlimited
//			ロック待ちするとき、その最大待ち時間(ミリ秒単位)
//			省略時には、無制限に待つ
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Item::hold(Client&			client,
		   Mode::Value		mode,
		   Duration::Value	duration,
		   Timeout::Value	timeout)
{
	// ロックするロック項目に対する
	// 指定されたロック要求元のロック要求を得る

	Request* request = Request::attach(*this, client);
	; _SYDNEY_ASSERT(request);

	// ロック要求をロックする

	Status::Value	sts;
	if ((sts = request->hold(mode, duration, timeout)) != Status::Succeeded) {

		// ロックに失敗したロック要求に許可されているロックがなければ、
		// 今回の試みで生成したものなので、このロック要求を破棄する

		if (!request->isGranted()) {

			Request::detach(request);
		}

		return sts;
	}

	if (duration == Duration::Instant && !request->isGranted()) {

		// 必要ないのでロック要求は破棄する

		Request::detach(request);
	}

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::Item::release -- あるロック項目のロックをはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&			client
//			はずすロックのロック要求元
//		Lock::Mode::Value		mode
//			はずすロックのモード
//		Lock::Duration::Value	duration
//			はずすロックの持続期間
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Item::release(Client&			client,
			  Mode::Value		mode,
			  Duration::Value	duration)
{
	Os::AutoCriticalSection	latch(Manager::getLatch());

	// ロックをはずすロック項目に対する
	// 指定されたロック要求元のロック要求があるか調べる

	Request* request = Request::find(*this, client);
	if (request) {

		// ロック要求のロックをはずす

		Status::Value	sts;
		if ((sts = request->release(mode, duration)) != Status::Succeeded) return sts;

		if (!request->isGranted()) {

			// ロックをはすした結果、
			// ロック要求に許可されているロックがないので、
			// このロック要求を破棄する

			Request::detach(request);
		}
	}
#ifdef DEBUG
	else {
		// too much release
		_SYDNEY_THROW0(Exception::Unexpected);
	}
#endif

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::Item::convert --
//		あるロック項目のロックのモードを他のモードに変換する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&			client
//			モードを変換するロックのロック要求元
//		Lock::Mode::Value		from
//			変換するロックのモード
//		Lock::Duration::Value	fromDuration / duration
//			変換するロックの持続期間
//		Lock::Mode::Value		to
//			変換後のモード
//		Lock::Duration::Value	toDuration
//			変換後のロックの持続期間
//		Lock::Timeout::Value	timeout = Lock::Timeout::Unlimited
//			変換時にロック待ちするとき、その最大待ち時間(ミリ秒単位)
//			省略時には、無制限に待つ
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Item::convert(Client&			client,
			  Mode::Value		from,
			  Duration::Value	fromDuration,
			  Mode::Value		to,
			  Duration::Value	toDuration,
			  Timeout::Value	timeout)
{
	return this->convert(client,
						 from,
						 fromDuration,
						 to,
						 timeout);
}

Status::Value
Item::convert(Client&			client,
			  Mode::Value		from,
			  Duration::Value	duration,
			  Mode::Value		to,
			  Timeout::Value	timeout)
{
	if (from != to) {

		// ロックされているロック項目に対する
		// 指定されたロック要求元のロック要求を得る

		Request* request = Request::attach(*this, client);

		// ロック要求の指定されたモードのロックを
		// 異なるモードに変換する
		Status::Value	sts;
		if ((sts = request->convert(from, duration, to, timeout)) != Status::Succeeded) {

			// ロックモードの変換に失敗したロック要求に
			// 許可されているロックがなければ、
			// 今回の試みで生成したものなので、このロック要求を破棄する

			if (!request->isGranted()) {

				Request::detach(request);
			}

			return sts;
		}
	}

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::Item::latch -- ラッチする
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&	client
//			ラッチを行うロック要求元
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotSupported
//			ラッチできないロック項目をラッチしようとした

void
Item::latch(Client&	client)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Lock::Item::unlatch -- アンラッチする
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&	client
//			アンラッチするロック要求元
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotSupported
//			ラッチできないロック項目をアンラッチしようとした

void
Item::unlatch(Client&	client)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	Lock::Item::checkCompatible --
//		このロック項目について競合している
//		ロック要求元のロックが競合しなくなるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Lock::Status::Value
//			ロックステータス
//
//	EXCEPTIONS

Status::Value
Item::checkCompatible()
{
	//【注意】	Manager::getLatch をラッチ済であること

	// すべてのロック要求元の有効なロックモードを求める

	; _SYDNEY_ASSERT(_count);
	_granted = (Config::EnableDowngrade::get() ? *(*_count) :
				(*(*_count) == Mode::N ? Mode::N : _granted));

	// 競合している他のロック要求元のロックが競合しなくなるか調べる
	//
	// つまり、競合しているロック要求元の要求が許可されたときの
	// ロックモードとすべてのロック要求元の有効なロックモードが
	// 両立するか調べる

	if (Request* request = _queue) {
		do {
			; _SYDNEY_ASSERT(request);

			if (request->_condition == Request::Condition::Waiting &&
				request->isCompatible(
					Mode::getLeastUpperBound(
						request->_granted, request->_client->_waitingMode))) {

				// 両立するので、このロック要求のロック待ちを解除する

				Status::Value	sts;
				if ((sts = request->_client->notifyRelease(Request::Condition::Granted)) != Status::Succeeded) return sts;
			}

		} while ((request = request->_next) != _queue) ;
	}

	return Status::Succeeded;
}

//	FUNCTION public
//	Lock::LatchableItem::latch -- ラッチする
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&	client
//			ラッチを行うロック要求元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
LatchableItem::latch(Client&	client)
{
	// ラッチを表すクリティカルセクションへ入る
	//
	//【注意】	ラッチ時にタイムアウトを指定可能にするには、
	//			クリティカルセクションでなく、イベントを使う必要がある

	_criticalSection.lock();
	{
	// ラッチされたロック項目をロック要求元に登録しておく

	Os::AutoCriticalSection	latch(client._latch);

	client._latchedItem.pushBack(this);
	}
	// ラッチされたロック項目を表すクラスを
	// アンラッチする前に破棄されないように
	// 参照数を 1 増やしておく
	//
	//【注意】	ラッチされたロック項目を表すクラスは
	//			ロックされたロック項目を表すクラスのように
	//			ハッシュ表で管理されないので、
	//			ラッチ後にロック項目を表すクラスを破棄されると、
	//			アンラッチ時にロック項目を表すクラスが見つけられなくなる

	(void) attach(*this);
}

//	FUNCTION public
//	Lock::LatchableItem::unlatch -- アンラッチする
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&	client
//			アンラッチするロック要求元
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
LatchableItem::unlatch(Client&	client)
{
	{
	// ロック要求元に登録されているラッチされたロック項目を抹消する

	Os::AutoCriticalSection	latch(client._latch);

	ModVector<Item*>::Iterator ite(client._latchedItem.find(this));
	; _SYDNEY_ASSERT(ite != client._latchedItem.end());
	client._latchedItem.erase(ite);
	}
	// ラッチ時に増やした参照数を 1 減らす

	detach(*this);

	// ラッチを表すクリティカルセクションから抜ける

	_criticalSection.unlock();
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
