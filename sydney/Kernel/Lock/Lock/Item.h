// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Item.h -- ロック項目関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_ITEM_H
#define	__SYDNEY_LOCK_ITEM_H

#include "Lock/Module.h"
#include "Lock/Count.h"
#include "Lock/Duration.h"
#include "Lock/Name.h"
#include "Lock/Timeout.h"
#include "Lock/Status.h"

#include "Os/CriticalSection.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

class Client;
class Request;

//	CLASS
//	Lock::Item -- ロック項目を表すクラス
//
//	NOTES

class Item
{
	friend class Client;
	friend class LatchableItem;
	friend class ModAutoPointer<Item>;
	friend class Request;
public:

	static void initializeItemTable();

	// ロック項目を表すクラスを生成する
	SYD_LOCK_FUNCTION
	static Item*			attach(const Name&	name);
	// ロック項目を表すクラスを破棄する
	SYD_LOCK_FUNCTION
	static void				detach(Item*&	item);

	SYD_LOCK_FUNCTION
	Status::Value			hold(Client&			client,
								 Mode::Value		mode,
								 Duration::Value	duration, 
								 Timeout::Value		timeout
														= Timeout::Unlimited);
												// ロックをかける
	// ロックをはずす
	SYD_LOCK_FUNCTION
	Status::Value			release(Client&			client,
									Mode::Value		mode,
									Duration::Value	duration);

	SYD_LOCK_FUNCTION
	Status::Value			convert(Client&			client,
									Mode::Value		from,
									Duration::Value	fromDuration,
									Mode::Value		to,
									Duration::Value	toDuration,
									Timeout::Value	timeout
														= Timeout::Unlimited);

	SYD_LOCK_FUNCTION
	Status::Value			convert(Client&			client,
									Mode::Value		from,
									Duration::Value	duration,
									Mode::Value		to,
									Timeout::Value	timeout
														= Timeout::Unlimited);
												// ロックを変換する

	// ラッチする
	SYD_LOCK_FUNCTION
	virtual void			latch(Client&	client);
	// アンラッチする
	SYD_LOCK_FUNCTION
	virtual void			unlatch(Client&	client);

	// 一意に識別するためのロック名を得る
	const Name&
	getName() const;

	// 解放したインスタンスをつなぐリスト内のロック項目を破棄する
	static void terminateFreeList();

private:
	// コンストラクター
	Item(const Name&	name);
	// デストラクター
	virtual ~Item();

	// 初期化する
	void					initialize(const Name&	name);

	// 参照数を 1 増やす
	static Item*			attach(const Item&	item);
	// 参照数を 1 減らす
	static void				detach(const Item&	item);

	// インスタンスを得る
	static Item*			popFreeList(const Name&	name_);
	// インスタンスをリストにつなぐ
	static void				pushFreeList(Item*	item);
	
	// 参照されているか
	bool					isAttached() const;

	// 競合しているロック要求元のロックが競合しなくなるか調べる
	Status::Value
	checkCompatible();

	// 一意に識別するためのロック名
	Name					_name;
	// 参照回数
	mutable unsigned int	_refCount;

	// 同じハッシュ値 v のロック項目リスト
	//
	//							┌─────────┐
	//				v			↓		_next		│
	//	hashTable[]	────→	Item	────→	Item
	//									_prev
	//							│		←────	↑
	//							└─────────┘

	Item*					_prev;				// 直前のロック項目
	Item*					_next;				// 直後のロック項目

	Request*				_queue;				// このロック項目に対する
												// 最新のロック要求
	Count*					_count;				// このロック項目に対する
												// ロックモード別ロック総数
	Mode::Value				_granted;			// このロック項目に対する
												// 現在有効なロックモード
};

//	CLASS
//	Lock::LatchableItem -- ラッチ可能なロック項目を表すクラス
//
//	NOTES

class LatchableItem
	: public	Item
{
	friend class Item;
public:
	// ラッチする
	SYD_LOCK_FUNCTION
	virtual void			latch(Client& client);
	// アンラッチする
	SYD_LOCK_FUNCTION
	virtual void			unlatch(Client& client);

private:
	// コンストラクター
	LatchableItem(const Name&	name);
	// デストラクター
	~LatchableItem();

	// ラッチ・アンラッチのためのクリティカルセクション
	Os::CriticalSection		_criticalSection;
};

//	FUNCTION private
//	Lock::Item::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	Lock::Name&		name
//		生成するロック項目を表すクラスを識別するためのロック名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

inline
void
Item::initialize(const Name&	name)
{
	_name = name;
	_refCount = 0;
	_prev = 0;
	_next = 0;
	_queue = 0;
	_granted = Mode::N;
	_count = Count::attach(Count());
}

//	FUNCTION private
//	Lock::Item::isAttached -- ロック項目を表すクラスが参照中か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			参照中である
//		false
//			参照中でない
//
//	EXCEPTIONS
//		なし

inline
bool
Item::isAttached() const
{
	return _refCount;
}

//	FUNCTION public
//	Lock::Item::getName -- 一意に識別するためのロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたロック名のリファレンス
//
//	EXCEPTIONS
//		なし

inline
const Name&
Item::getName() const
{
	return _name;
}

//	FUNCTION private
//	Lock::LatchableItem::LatchableItem --
//		ラッチ可能なロック項目を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			name
//			生成するロック項目を表すクラスを識別するためのロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
LatchableItem::LatchableItem(const Name&	name)
	: Item(name)
{}

//	FUNCTION private
//	Lock::LatchableItem::~LatchableItem --
//		ラッチ可能なロック項目を表すクラスのデストラクター
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
LatchableItem::~LatchableItem()
{}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_ITEM_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
