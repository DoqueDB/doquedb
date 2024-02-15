// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Request.h -- ロック要求関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_REQUEST_H
#define	__SYDNEY_LOCK_REQUEST_H

#include "Lock/Module.h"
#include "Lock/Count.h"
#include "Lock/Duration.h"
#include "Lock/Mode.h"
#include "Lock/Timeout.h"
#include "Lock/Status.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

class Client;
class Item;

//	CLASS
//	Lock::Request -- ロック要求を表すクラス
//
//	NOTES

class Request
{
	friend class Client;
	friend class Item;
public:
	//	CLASS
	//	Lock::Request::Condition -- ロック要求の状態を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Condition
	{
		//	ENUM
		//	Lock::Request::Condition::Value -- ロック要求の状態の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Granted =			0,				// ロック中
			Waiting,							// ロック待ち中
			Converting,							// ロック変換
												// (のためロック待ち)中
			Deadlocked,							// デッドロックが起きた
			ValueNum							// 値の種類数
		};
	};

	// 解放したインスタンスをつなぐリスト内のロック要求を破棄する
	static void terminateFreeList();

private:
	Request(Item&	item,
			Client&	client);					// コンストラクター
	~Request();									// デストラクター

	// 初期化する
	void initialize(Item& item, Client& client);
	// 終了処理する
	void terminate();

	// ロック要求を表すクラスを生成する
	static Request*
	attach(Item& item, Client& client);
	// ロック要求を表すクラスを破棄する
	static void
	detach(Request*& request);

	// インスタンスを得る
	static Request* popFreeList();
	// インスタンスをリストにつなぐ
	static void pushFreeList(Request* request);

	// ロック要求を表すクラスを探す
	static Request*
	find(const Item& item, const Client& client);

	Status::Value			hold(Mode::Value		mode,
								 Duration::Value	duration,
								 Timeout::Value		timeout
														= Timeout::Unlimited);
												// ロックをかける
	// ロックをはずす
	Status::Value			release(Mode::Value		mode,
									Duration::Value	duration,
									Count::Value	n = 1);
	Status::Value			release(const Count&	count,
									Duration::Value	duration);

	Status::Value			convert(Mode::Value		from,
									Duration::Value	duration,
									Mode::Value		to,
									Timeout::Value	timeout
														= Timeout::Unlimited);
												// ロックを変換する

	// すべてのロックをなくす
	Status::Value			clear(Duration::Value	duration);

	// ロック数を加算する
	void
	up(Mode::Value mode, Mode::Value lub);

	// 他のロック要求元があるモードでロックしたとき、両立するか
	bool
	isCompatible(Mode::Value mode) const;
	// あるモードでロックするのに親のロックは十分か
	bool
	isAllowableChild(Mode::Value mode) const;

	bool					isAllowableParent(Mode::Value	mode) const;
												// ロック項目をあるモードで
												// ロックしたとき、
												// そのすべての子のロックを
												// 保持するのに十分か
	// 許可されているか
	bool
	isGranted() const;

	// 同じロック項目に対するロック要求リスト
	//
	//						┌─────────┐
	//			_queue		↓		_next		│
	//	Item	────→	Request	────→	Request	
	//			_item				_prev
	//			←────	│		←────	↑
	//						└─────────┘

	Item*					_item;				// ロック対象であるロック項目
	Request*				_prev;				// 直前のロック要求
	Request*				_next;				// 直後のロック要求

	// 同じロック要求元によるロック要求リスト
	//
	//						┌─────────┐
	//			_request	↓		_elder		│
	//	Client	────→	Request	────→	Request	
	//			_client				_younger
	//			←────	│		←────	↑
	//						└─────────┘

	Client*					_client;			// ロック要求元
	Request*				_elder;				// 直前のロック要求
	Request*				_younger;			// 直後のロック要求

	Condition::Value		_condition;			// ロック要求の状態
	Count*					_count;				// ロックモード別ロック数
	Mode::Value				_granted;			// 現在有効なロックモード

	Count*					_stackCount;		// 1つのSQL文の中で
												// スタックされているロック数
};

//	FUNCTION private
//	Lock::Request::isGranted --
//		ロック要求が許可されているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			許可されている
//		false
//			許可されていない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Request::isGranted() const
{
	return _granted != Mode::N;
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_REQUEST_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
