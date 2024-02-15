// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.h -- チェックポイント処理実行スレッドに関するクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_EXECUTOR_H
#define	__SYDNEY_CHECKPOINT_EXECUTOR_H

#include "Checkpoint/Module.h"

#include "Buffer/DaemonThread.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::Executor -- チェックポイント処理実行スレッドを表すクラス
//
//	NOTES

class Executor
	: public	_SYDNEY::Buffer::DaemonThread
{
public:
	// コンストラクター
	Executor(unsigned int timeout, bool enable);
	// デストラクター
	~Executor();

	// システム全体のチェックポイント処理を行う
	SYD_CHECKPOINT_FUNCTION
	static void
	cause(bool aborting);
	// あるデータベースに関するチェックポイント処理を行う
	SYD_CHECKPOINT_FUNCTION
	static void
	cause(Trans::Transaction& trans, Schema::Database& database);

private:
	// スレッドが繰り返し実行する関数
	void					repeatable();
};

//	FUNCTION private
//	Checkpoint::Executor::Executor --
//		チェックポイント処理実行スレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		timeout
//			チェックポイント処理の実行間隔(単位ミリ秒)
//		bool				enable
//			true
//				チェックポイント処理を可能にする
//			false
//				チェックポイント処理を不可にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Executor::Executor(unsigned int timeout, bool enable)
	: _SYDNEY::Buffer::DaemonThread(timeout, enable)
{}

//	FUNCTION private
//	Checkpoint::Executor::~Executor --
//		チェックポイント処理実行スレッドを表すクラスのデストラクター
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
Executor::~Executor()
{}

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_EXECUTOR_H

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
