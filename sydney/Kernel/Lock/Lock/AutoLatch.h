// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLatch.h -- オートラッチ関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_AUTOLATCH_H
#define	__SYDNEY_LOCK_AUTOLATCH_H

#include "Lock/Module.h"
#include "Lock/AutoItem.h"
#include "Lock/Client.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::AutoLatch -- オートラッチを表すクラス
//
//	NOTES

class AutoLatch
{
public:
	// コンストラクター
	AutoLatch(Client& client, const Name& name);
	// デストラクター
	~AutoLatch();

	// ラッチをはずす
	void
	unlatch();

private:
	// ラッチするロック要求元
	Client&					_client;
	// ラッチされるロック項目
	AutoItem				_item;
	// ラッチ済か
	bool					_latched;
};

//	CLASS
//	Lock::AutoLatch -- オートアンラッチを表すクラス
//
//	NOTES

class AutoUnlatch
{
public:
	// コンストラクター
	AutoUnlatch(Client& client, const Name& name);
	// デストラクター
	~AutoUnlatch();

	// ラッチする
	void
	latch();

private:
	// アンラッチするロック要求元
	Client&					_client;
	// アンラッチされるロック項目
	AutoItem				_item;
	// アンラッチ済か
	bool					_unlatched;
};

//	FUNCTION public
//	Lock::AutoLatch::AutoLatch --
//		オートラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&		client
//			ラッチするロック要求元
//		Lock::Name&			name
//			ラッチされるロック項目のロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoLatch::AutoLatch(Client& client, const Name& name)
	: _client(client),
	  _item(Item::attach(name)),
	  _latched(false)
{
	// 指定されたロック要求元は
	// 指定されたロック名のロック項目をラッチする

	_item->latch(_client);
	_latched = true;
}

//	FUNCTION public
//	Lock::AutoLatch::~AutoLatch --
//		オートラッチを表すクラスのデストラクター
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

inline
AutoLatch::~AutoLatch()
{
	unlatch();
}

//	FUNCTION public
//	Lock::AutoLatch::unlatch -- オートラッチをアンラッチする
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

inline
void
AutoLatch::unlatch()
{
	if (_latched) {

		// ラッチされているので、アンラッチする

		_item->unlatch(_client);
		_latched = false;
	}
}

//	FUNCTION public
//	Lock::AutoUnlatch::AutoUnlatch --
//		オートアンラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Client&		client
//			アンラッチするロック要求元
//		Lock::Name&			name
//			アンラッチされるロック項目のロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoUnlatch::AutoUnlatch(Client& client, const Name& name)
	: _client(client),
	  _item(Item::attach(name)),
	  _unlatched(false)
{
	// 指定されたロック要求元は
	// 指定されたロック名のロック項目をアンラッチする

	_item->unlatch(_client);
	_unlatched = true;
}

//	FUNCTION public
//	Lock::AutoUnlatch::~AutoUnlatch --
//		オートアンラッチを表すクラスのデストラクター
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

inline
AutoUnlatch::~AutoUnlatch()
{
	latch();
}

//	FUNCTION public
//	Lock::AutoUnlatch::latch -- オートアンラッチをラッチする
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

inline
void
AutoUnlatch::latch()
{
	if (_unlatched) {

		// アンラッチされているので、ラッチする

		_item->latch(_client);
		_unlatched = false;
	}
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_AUTOLATCH_H

//
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
