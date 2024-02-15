// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Daemon.cpp -- デーモン関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Version";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Version/Configuration.h"
#include "Version/Daemon.h"
#include "Version/DetachedPageCleaner.h"
#include "Version/Manager.h"

#include "Common/Assert.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace 
{

namespace _Daemon
{
	// 参照済バージョンページ記述子破棄スレッド
	DetachedPageCleaner*	_detachedCleaner;
}

}

//	FUNCTION private
//	Version::Manager::Daemon::initialize --
//		マネージャーの初期化のうち、
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッド関連のものを行う
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
	// バージョンファイルに関係する
	// データ構造を処理するデーモンを初期化する

	Version::Daemon::initialize();

	try {
		// バージョンファイルに関係する
		// データ構造を処理するデーモンを開始し、有効にする

		Version::Daemon::create();
		Version::Daemon::enable();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		Version::Daemon::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Version::Manager::Daemon::terminate --
//		マネージャーの後処理のうち、
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッド関連のものを行う
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
	// バージョンファイルに関係する
	// データ構造を処理するデーモンをすべて終了させる

	Version::Daemon::join();

	// バージョンファイルに関係する
	// データ構造を処理するデーモンの後処理を行う

	Version::Daemon::terminate();
}

//	FUNCTION
//	Version::Daemon::initialize --
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッドの初期化を行う
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
		// 参照済バージョンページ記述子破棄スレッドを生成する

		_Daemon::_detachedCleaner = new DetachedPageCleaner(
				Configuration::DetachedPageCleanerPeriod::get(), false);
		; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);

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
//	Version::Daemon::terminate --
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッドの後処理を行う
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
	// 参照済バージョンページ記述子破棄スレッドを表すクラスを破棄する

	delete _Daemon::_detachedCleaner, _Daemon::_detachedCleaner = 0;
}

//	FUNCTION
//	Version::Daemon::create --
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッドを生成する
//
//	NOTES
//		デーモンスレッドの実際の処理は不可な状態で生成されるので、
//		Version::Daemon::enable で可能な状態にしなければならない
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
		; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);
		_Daemon::_detachedCleaner->create();

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
//	Version::Daemon::join --
//		バージョンファイルに関係するデータ構造を処理する
//		デーモンスレッドを終了する
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
	; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);
	_Daemon::_detachedCleaner->join();
}

#ifdef OBSOLETE
//	FUNCTION
//	Version::Daemon::wakeup -- あるデーモンスレッドの処理を再開する
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
Daemon::wakeup(Category::Value category)
{
	switch (category) {
	case Category::DetachedPageCleaner:
		; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);
		_Daemon::_detachedCleaner->wakeup();
		break;
	}
}
#endif

//	FUNCTION
//	Version::Daemon::enable --
//		すべてのデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				Version::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Version::Daemon::disable の入れ子呼び出しを 1 回ぶん無効にする
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

		} while (++i < Category::ValueNum);

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
//	Version::Daemon::enable --
//		あるデーモンスレッドの行うべき処理を実行可能にする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Daemon::Category::Value	category
//			処理を実行可能にするデーモンスレッドの種別
//		bool				force
//			true
//				Version::Daemon::disable の入れ子呼び出しをすべて無効にする
//			false または指定されないとき
//				Version::Daemon::disable の入れ子呼び出しを 1 回ぶん無効にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Daemon::enable(Category::Value category, bool force)
{
	switch (category) {
	case Category::DetachedPageCleaner:
		; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);
		_Daemon::_detachedCleaner->enable(force);
		break;
	}
}

#ifdef OBSOLETE
//	FUNCTION
//	Version::Daemon::disable --
//		すべてのデーモンスレッドの行うべき処理を実行不可にする
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

void
Daemon::disable(bool force)
{
	unsigned int i = 0;
	try {
		do {
			disable(static_cast<Category::Value>(i), force);

		} while (++i < Category::ValueNum);

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
#endif

//	FUNCTION
//	Version::Daemon::disable --
//		あるデーモンスレッドの行うべき処理を実行不可にする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Daemon::Category::Value	category
//			処理を実行不可にするデーモンスレッドの種別
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

void
Daemon::disable(Category::Value category, bool force)
{
	switch (category) {
	case Category::DetachedPageCleaner:
		; _SYDNEY_ASSERT(_Daemon::_detachedCleaner);
		_Daemon::_detachedCleaner->disable(force);
		break;
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
