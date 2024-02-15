// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Daemon.cpp -- デーモン関連の関数定義
// 
// Copyright (c) 2000, 2002, 2003, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/Configuration.h"
#include "Buffer/Daemon.h"
#include "Buffer/DirtyPageFlusher.h"
#include "Buffer/StatisticsReporter.h"
#include "Buffer/Manager.h"

#include "Common/Assert.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _Daemon
{
	// ダーティバッファページ書き込みスレッド
	DirtyPageFlusher*		_dirtyFlusher = 0;

	// 統計情報出力スレッド
	StatisticsReporter*		_statisticsReporter = 0;
}

}

//	FUNCTION private
//	Buffer::Manager::Daemon::initialize --
//		マネージャーの初期化のうち、バッファプールの管理する
//		バッファページを処理するデーモンスレッド関連のものを行う
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
Manager::Daemon::initialize()
{
	// バッファプールを操作するデーモンを初期化する

	Buffer::Daemon::initialize();

	try {
		// バッファプールを操作するデーモンを開始し、有効にする

		Buffer::Daemon::create();
		Buffer::Daemon::enable();

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Buffer::Daemon::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Buffer::Manager::Daemon::terminate --
//		マネージャーの後処理のうち、バッファプールの管理する
//		バッファページを処理するデーモンスレッド関連のものを行う
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
Manager::Daemon::terminate()
{
	// バッファプールを操作するデーモンをすべて終了させる
	//
	//【注意】	このとき、ダーティなバッファページはすべてフラッシュされる

	Buffer::Daemon::join();

	// バッファプールを操作するデーモンの後処理を行う

	Buffer::Daemon::terminate();
}

//	FUNCTION
//	Buffer::Daemon::initialize --
//		バッファプールを操作するデーモンスレッドの初期化を行う
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

void
Daemon::initialize()
{
	try {
		// ダーティバッファページ書き込みスレッドを生成する

		_Daemon::_dirtyFlusher = new DirtyPageFlusher(
			Configuration::DirtyPageFlusherPeriod::get(), false);
		; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);

		if (Configuration::StatisticsReporterPeriod::get() != 0)
		{
			// 統計情報出力スレッドを生成する
		
			_Daemon::_statisticsReporter = new StatisticsReporter(
				Configuration::StatisticsReporterPeriod::get());
			; _SYDNEY_ASSERT(_Daemon::_statisticsReporter);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Daemon::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Buffer::Daemon::terminate --
//		バッファプールを操作するデーモンスレッドの後処理を行う
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

void
Daemon::terminate()
{
	// 統計情報出力スレッドを表すクラスを破棄する

	delete _Daemon::_statisticsReporter, _Daemon::_statisticsReporter = 0;
	
	// ダーティバッファページ書き込みスレッドを表すクラスを破棄する

	delete _Daemon::_dirtyFlusher, _Daemon::_dirtyFlusher = 0;
}

//	FUNCTION
//	Buffer::Daemon::create --
//		バッファプールを操作するデーモンスレッドを生成する
//
//	NOTES
//		デーモンスレッドの実際の処理は不可な状態で生成されるので、
//		Buffer::Daemon::enable で可能な状態にしなければならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::create()
{
	try {
		; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);
		_Daemon::_dirtyFlusher->create();

		if (_Daemon::_statisticsReporter)
			_Daemon::_statisticsReporter->create();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Daemon::join();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Buffer::Daemon::join -- バッファプールを操作するデーモンスレッドを終了する
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

void
Daemon::join()
{
	if (_Daemon::_statisticsReporter)
		_Daemon::_statisticsReporter->join();
	; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);
	_Daemon::_dirtyFlusher->join();
}

#ifdef OBSOLETE
//	FUNCTION
//	Buffer::Daemon::wakeup -- あるデーモンスレッドの処理を再開する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Daemon::Category::Value	category
//			処理を再開するデーモンスレッドの種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::wakeup(Category::Value category)
{
	switch (category) {
	case Category::DirtyPageFlusher:
		; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);
		_Daemon::_dirtyFlusher->wakeup();
		break;
	}
}
#endif

//	FUNCTION
//	Buffer::Daemon::enable --
//		すべてのデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Buffer::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Buffer::Daemon::disable の入れ子呼び出しを 1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::enable(bool force)
{
	unsigned int i = 0;
	try {
		do {
			enable(static_cast<Category::Value>(i), force);

		} while (++i < Category::Count);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		while (i--)
			disable(static_cast<Category::Value>(i));
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Buffer::Daemon::enable --
//		あるデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Daemon::Category::Value	category
//			処理を実行可能にするデーモンスレッドの種別
//		bool				force
//			true
//				Buffer::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Buffer::Daemon::disable の入れ子呼び出しを 1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::enable(Category::Value category, bool force)
{
	switch (category) {
	case Category::DirtyPageFlusher:
		; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);
		_Daemon::_dirtyFlusher->enable(force);
		break;
	}
}

//	FUNCTION
//	Buffer::Daemon::disbale --
//		すべてのデーモンスレッドの行うべき処理を実行不可にする
//
//	NOTES
//
//	ARGUMENTS
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

void
Daemon::disable(bool force)
{
	unsigned int i = 0;
	try {
		do {
			disable(static_cast<Category::Value>(i), force);

		} while (++i < Category::Count);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		while (i--)
			enable(static_cast<Category::Value>(i));
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	Buffer::Daemon::disable --
//		あるデーモンスレッドの行うべき処理を実行不可にする
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

void
Daemon::disable(Category::Value category, bool force)
{
	switch (category) {
	case Category::DirtyPageFlusher:
		; _SYDNEY_ASSERT(_Daemon::_dirtyFlusher);
		_Daemon::_dirtyFlusher->disable(force);
		break;
	}
}

//
// Copyright (c) 2000, 2002, 2003, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
