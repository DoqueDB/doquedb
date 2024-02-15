// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoRWLock.h --
//		自動読み取り書き込みロック関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_AUTORWLOCK_H
#define	__TRMEISTER_OS_AUTORWLOCK_H

#include "Os/Module.h"
#include "Os/RWLock.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::AutoRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックをロックし、
//		破棄時に自動的にロックをはずすクラス
//
//	NOTES

class AutoRWLock
{
public:
	// コンストラクター
	AutoRWLock(RWLock& rwlock, RWLock::Mode::Value mode = RWLock::Mode::Read);
	// デストラクター
	~AutoRWLock();

private:
	// 操作する読み取り書き込みロック
	RWLock&					_rwlock;
	// かかっているロックのモード
	RWLock::Mode::Value		_mode;
};

//	CLASS
//	Os::AutoTryRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックにロックを試み、
//		破棄時にロックしていれば、自動的にはずすクラス
//
//	NOTES

class AutoTryRWLock
{
public:
	// コンストラクター
	AutoTryRWLock(RWLock& rwlock);
	AutoTryRWLock(RWLock& rwlock, RWLock::Mode::Value mode);
	// デストラクター
	~AutoTryRWLock();

	// ロックする
	void					lock(RWLock::Mode::Value mode);
	// ロックを試みる
	bool					trylock(RWLock::Mode::Value mode);
	// ロックをはずす
	void					unlock();
	// ロックしているか
	bool					isLocked() const;

private:
	// 操作する読み取り書き込みオブジェクト
	RWLock&					_rwlock;
	// かかっているロックのモード
	RWLock::Mode::Value		_mode;
};

//	CLASS
//	Os::AutoRWLock::AutoRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックをロックし、
//		破棄時に自動的にロックをはずすクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock&			rwlock
//			操作する読み取り書き込みロックのリファレンス
//		Os::RWLock::Mode::Value	mode
//			指定されたとき
//				自動的にかけるロックのモード
//			指定されないとき
//				Os::RWLock::Mode::Read が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoRWLock::AutoRWLock(RWLock& rwlock, RWLock::Mode::Value mode)
	: _rwlock(rwlock),
	  _mode(mode)
{
	_rwlock.lock(_mode);
}

//	CLASS
//	Os::AutoRWLock::~AutoRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックをロックし、
//		破棄時に自動的にロックをはずすクラスのデストラクター
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
AutoRWLock::~AutoRWLock()
{
	_rwlock.unlock(_mode);
}

//	CLASS
//	Os::AutoTryRWLock::AutoTryRWLock --
//		オブジェクト破棄時に読み取り書き込みロックしていれば、
//		自動的にはずすクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock&			rwlock
//			操作する読み取り書き込みロックのリファレンス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoTryRWLock::AutoTryRWLock(RWLock& rwlock)
	: _rwlock(rwlock),
	  _mode(RWLock::Mode::Unknown)
{}

//	CLASS
//	Os::AutoTryRWLock::AutoTryRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックにロックを試み、
//		破棄時にロックしていれば、自動的にはずすクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock&			rwlock
//			操作する読み取り書き込みロックのリファレンス
//		Os::RWLock::Mode::Value	mode
//			自動的にかけるロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoTryRWLock::AutoTryRWLock(RWLock& rwlock, RWLock::Mode::Value mode)
	: _rwlock(rwlock),
	  _mode((rwlock.trylock(mode)) ? mode : RWLock::Mode::Unknown)
{}

//	CLASS
//	Os::AutoTryRWLock::AutoTryRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックにロックを試み、
//		破棄時にロックしていれば、自動的にはずすクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock&			rwlock
//			操作する読み取り書き込みロックのリファレンス
//		Os::RWLock::Mode::Value	mode
//			自動的にかけるロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoTryRWLock::~AutoTryRWLock()
{
	unlock();
}

//	FUNCTION public
//	Os::AutoTryRWLock::lock -- ロックする
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock::Mode::Value	mode
//			かけるロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
AutoTryRWLock::lock(RWLock::Mode::Value mode)
{
	if (!isLocked()) {
		_rwlock.lock(mode);
		_mode = mode;
	}
}

//	FUNCTION public
//	Os::AutoTryRWLock::trylock -- ロックしていなければ、ロックを試みる
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock::Mode::Value mode
//			かけるロックのモード
//
//	RETURN
//		true
//			ロックできた
//		false
//			ロックできなかった
//
//	EXCEPTIONS

inline
bool
AutoTryRWLock::trylock(RWLock::Mode::Value mode)
{
	if (isLocked())
		return true;
	else if (_rwlock.trylock(mode)) {
		_mode = mode;
		return true;
	}
	return false;
}

//	FUNCTION public
//	Os::AutoTryRWLock::unlock -- ロックされていれば、ロックをはずす
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
AutoTryRWLock::unlock()
{
	if (isLocked()) {
		_rwlock.unlock(_mode);
		_mode = RWLock::Mode::Unknown;
	}
}

//	FUNCTION public
//	AutoTryRWLock::isLocked -- 読み取り書き込みロックをロックしているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックしている
//		false
//			ロックしていない
//
//	EXCEPTIONS
//		なし

inline
bool
AutoTryRWLock::isLocked() const
{
	return _mode != RWLock::Mode::Unknown;
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_AUTORWLOCK_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

