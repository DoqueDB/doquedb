// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoRWLock.h --
//		自動読み取り書き込みロック関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_AUTORWLOCK_H
#define	__SYDNEY_SCHEMA_AUTORWLOCK_H

#include "Schema/Module.h"

#include "Os/RWLock.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::AutoRWLock --
//		オブジェクト生成時に自動的に読み取り書き込みロックをロックし、
//		破棄時に自動的にロックをはずすクラス
//
//	NOTES

class AutoRWLock
{
public:
	// コンストラクター
	AutoRWLock(Os::RWLock& rwlock, Os::RWLock::Mode::Value mode = Os::RWLock::Mode::Read);
	// デストラクター
	~AutoRWLock();

	// モードを変える
	void convert(Os::RWLock::Mode::Value mode = Os::RWLock::Mode::Read);

private:
	// 操作する読み取り書き込みロック
	Os::RWLock&				_rwlock;
	// かかっているロックのモード
	Os::RWLock::Mode::Value	_mode;
};

//	CLASS
//	Schema::AutoRWLock::AutoRWLock --
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
AutoRWLock::AutoRWLock(Os::RWLock& rwlock, Os::RWLock::Mode::Value mode)
	: _rwlock(rwlock),
	  _mode(mode)
{
	_rwlock.lock(_mode);
}

//	CLASS
//	Schema::AutoRWLock::~AutoRWLock --
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
//	Schema::AutoRWLock::convert --
//		読み書きロックのモードを変更する
//
//	NOTES
//
//	ARGUMENTS
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
void
AutoRWLock::convert(Os::RWLock::Mode::Value	mode)
{
	// 現在のモードはunlockする
	_rwlock.unlock(_mode);
	// 新しいモードでロックしなおす
	_rwlock.lock(_mode = mode);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_AUTORWLOCK_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

