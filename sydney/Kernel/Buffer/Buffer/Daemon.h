// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Daemon.h -- デーモン関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2004, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_DAEMON_H
#define	__SYDNEY_BUFFER_DAEMON_H

#include "Buffer/Module.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

namespace Daemon
{
	//	CLASS
	//	Buffer::Daemon::Category -- デーモンの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Buffer::Daemon::Category::Value -- デーモンの種別を表す値の列挙型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// ダーティバッファページ書き込みスレッド
			DirtyPageFlusher = 0,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	//	CLASS
	//	Buffer::Daemon::AutoDisabler --
	//		あるデーモンの処理を実行不可にするクラス
	//
	//	NOTES

	class AutoDisabler
	{
	public:
		// コンストラクター
		AutoDisabler(bool force = false);
		AutoDisabler(Category::Value category, bool force = false);
		// デストラクター
		~AutoDisabler();

	private:
		// 処理を実行不可にするデーモンスレッドの種別
		Category::Value			_category;
		// 強制的に実行不可にするか
		bool					_force;
	};

	// すべてのデーモンを初期化する
	SYD_BUFFER_FUNCTION
	void
	initialize();
	// すべてのデーモンの後処理を行う
	SYD_BUFFER_FUNCTION
	void
	terminate();

	// すべてのデーモンを開始する
	SYD_BUFFER_FUNCTION
	void
	create();
	// すべてのデーモンを終了する
	SYD_BUFFER_FUNCTION
	void
	join();
#ifdef OBSOLETE
	// あるデーモンの処理を再開する
	SYD_BUFFER_FUNCTION
	void
	wakeup(Category::Value category);
#endif
	// すべてのデーモンの処理を実行可能にする
	SYD_BUFFER_FUNCTION
	void
	enable(bool force = false);
	// あるデーモンの処理を実行可能にする
	SYD_BUFFER_FUNCTION
	void
	enable(Category::Value category, bool force = false);

	// すべてのデーモンの処理を実行不可にする
	SYD_BUFFER_FUNCTION
	void
	disable(bool force = false);
	// あるデーモンの処理を実行不可にする
	SYD_BUFFER_FUNCTION
	void
	disable(Category::Value category, bool force = false);
}

//	FUNCTION public
//	Version::Daemon::AutoDisabler::AutoDisabler --
//		すべてのデーモンの処理を実行不可にするクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Version::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Version::Daemon::enable の入れ子呼び出しを 1 回ぶん無効にする
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Daemon::AutoDisabler::AutoDisabler(bool force)
	: _category(Category::Unknown)
	, _force(force)
{
	disable(_force);
}

//	FUNCTION public
//	Buffer::Daemon::AutoDisabler::AutoDisabler --
//		あるデーモンの処理を実行不可にするクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Daemon::Category::Value	category
//			処理を実行不可にするデーモンスレッドの種別
//		bool				force
//			true
//				Buffer::Daemon::enable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Buffer::Daemon::enable の入れ子呼び出しを 1 回ぶん無効にする
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Daemon::AutoDisabler::AutoDisabler(Category::Value category, bool force)
	: _category(category)
	, _force(force)
{
	disable(_category, _force);
}

//	FUNCTION public
//	Buffer::Daemon::AutoDisabler::~AutoDisabler --
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
	(_category != Category::Unknown) ?
		enable(_category, _force) : enable(_force);
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_DAEMON_H

//
// Copyright (c) 2002, 2003, 2004, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
