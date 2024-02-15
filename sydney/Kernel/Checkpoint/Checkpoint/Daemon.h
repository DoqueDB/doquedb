// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Daemon.h -- デーモン関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_DEAMON_H
#define	__SYDNEY_CHECKPOINT_DEAMON_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

namespace Daemon
{
	//	CLASS
	//	Checkpoint::Daemon::Category -- デーモンの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Checkpoint::Daemon::Category::Value --
		//		デーモンの種別を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			//【注意】	以下の値の順序で join が実行されるので、
			//			この順序は重要である

			// バージョンファイル同期スレッド
			FileSynchronizer = 0,
			// チェックポイント処理実行スレッド
			Executor,
			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	Checkpoint::Daemon::AutoDisabler --
	//		あるデーモンの処理を実行不可にするクラス
	//
	//	NOTES
	//		このクラスは new することはないはずなので、
	//		Common::Object の子クラスにしない

	class AutoDisabler
	{
	public:
		// コンストラクター
		AutoDisabler(bool force = false);
		AutoDisabler(Category::Value category, bool force = false);
		// デストラクター
		~AutoDisabler();

		// 実行可能にする
		void					enable(bool force = false);
		// 実行不可にする
		void					disable(bool force = false);

	private:
		// 処理を実行不可にするデーモンスレッドの種別
		const Category::Value	_category;
		// 強制的に実行不可にするか
		const bool				_force;
		// 実行不可の入れ子回数
		int						_count;
	};

	//	CLASS
	//	Checkpoint::Daemon::AutoState --
	//		デーモン実行中の状態を報告するクラス

	class AutoState
	{
	public:
		// コンストラクタ
		AutoState(bool isOnlyExecuteFlag = false);
		// デストラクター
		~AutoState();

		// 開放する
		void release() { _owner = false; }

	private:
		// オーナーかどうか
		bool		_owner;
		// 実行中フラグのみかどうか
		bool		_only;
	};

	// デーモンを初期化する
	SYD_CHECKPOINT_FUNCTION
	void					initialize();
	// デーモンの後処理を行う
	SYD_CHECKPOINT_FUNCTION
	void					terminate();

	// デーモンを開始する
	SYD_CHECKPOINT_FUNCTION
	void					create();
	// デーモンを終了する
#ifdef OBSOLETE
	SYD_CHECKPOINT_FUNCTION
	void
	join();
#endif
	SYD_CHECKPOINT_FUNCTION
	void
	join(Category::Value category);

	// デーモンの処理を起動する(for Worker)
	SYD_CHECKPOINT_FUNCTION
	void					wakeup();
	// デーモンの処理の終了を待つ(for Worker)
	SYD_CHECKPOINT_FUNCTION
	bool					wait(unsigned int timeout = 0xffffffff);

	// デーモンの処理を再開する
	SYD_CHECKPOINT_FUNCTION
	void					wakeup(Category::Value category);
#ifdef OBSOLETE
	// デーモンの処理内容を実行する
	SYD_CHECKPOINT_FUNCTION
	bool
	execute(Category::Value category, bool force = false);
#endif
	// すべてのデーモンの処理を実行可能にする
	SYD_CHECKPOINT_FUNCTION
	void					enable(bool force = false);
	// あるデーモンの処理を実行可能にする
	SYD_CHECKPOINT_FUNCTION
	void
	enable(Category::Value category, bool force = false);

	// すべてのデーモンの処理を実行不可にする
	SYD_CHECKPOINT_FUNCTION
	void					disable(bool force = false);
	// あるデーモンの処理を実行不可にする
	SYD_CHECKPOINT_FUNCTION
	void
	disable(Category::Value category, bool force = false);

	// 終了イベントをシグナル化する
	void signalEvent();
	// 実行中フラグを設定する
	void setExecuteFlag(bool flag_);

	// 実行中かどうかを得る
	SYD_CHECKPOINT_FUNCTION
	bool					isExecuting();
}

//	FUNCTION public
//	Checkpoint::Daemon::AutoDisabler::AutoDisabler --
//		すべてのデーモンの処理を実行不可にするクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Checkpoint::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::enable の
//				入れ子呼び出しを 1 回ぶん無効にする
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Daemon::AutoDisabler::AutoDisabler(bool force)
	: _category(Category::ValueNum),
	  _force(force),
	  _count(0)
{
	disable(_force);
}

//	FUNCTION public
//	Checkpoint::Daemon::AutoDisabler::AutoDisabler --
//		あるデーモンの処理を実行不可にするクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Checkpoint::Daemon::Category::Value	category
//			処理を実行不可にするデーモンスレッドの種別
//		bool				force
//			true
//				Checkpoint::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::enable の入れ子呼び出しを
//				1 回ぶん無効にする
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Daemon::AutoDisabler::AutoDisabler(Category::Value category, bool force)
	: _category(category),
	  _force(force),
	  _count(0)
{
	disable(_force);
}

//	FUNCTION public
//	Checkpoint::Daemon::AutoDisabler::~AutoDisabler --
//		あるデーモンの処理を実行不可にするクラスのデストラクター
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
Daemon::AutoDisabler::~AutoDisabler()
{
	if (_force)
		enable(_force);
	else {
		while (_count > 0)
			enable();
		while (_count < 0)
			disable();
	}
}

//	FUNCTION public
//	Checkpoint::AutoDisabler::enable -- 処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Checkpoint::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::disable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Daemon::AutoDisabler::enable(bool force)
{
	(_category < Category::ValueNum) ?
		Daemon::enable(_category, force) : Daemon::enable(force);

	if (force && _count > 0)
		_count = 0;
	else
		--_count;
}

//	FUNCTION public
//	Checkpoint::AutoDisabler::disable -- 処理を実行不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Checkpoint::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Checkpoint::Daemon::enable の入れ子呼び出しを
//				1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Daemon::AutoDisabler::disable(bool force)
{
	(_category < Category::ValueNum) ?
		Daemon::disable(_category, force) : Daemon::disable(force);

	if (force && _count <= 0)
		_count = 1;
	else
		++_count;
}

//	FUNCTION public
//	Daemon::AutoState::AutoState -- コンストラクタ

inline
Daemon::AutoState::AutoState(bool isOnlyExecuteFlag)
	: _owner(true), _only(isOnlyExecuteFlag)
{
	// デーモン実行中フラグを設定する
	
	Daemon::setExecuteFlag(true);
}

//	FUNCTION public
//	Daemon::AutoState::~AutoState -- デストラクタ

inline
Daemon::AutoState::~AutoState()
{
	if (_owner)
	{
		// デーモン実行中フラグを落とす
		
		Daemon::setExecuteFlag(false);

		if (_only == false)
			
			// ワーカーに実行が終了したことを報告する

			Daemon::signalEvent();
	}
}

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_DEAMON_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
