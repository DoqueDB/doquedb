// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- OS 管理マネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_MANAGER_H
#define	__TRMEISTER_OS_MANAGER_H

#include "Os/Module.h"
#include "Os/File.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

class Path;
class UnicodeString;

//	NAMESPACE
//	Os::Manager -- OS 管理マネージャー全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_OS_FUNCTION
	void
	initialize();
	SYD_OS_FUNCTION
	void
	initialize(const Path& path, File::Permission::Value permission);
	// 後処理を行う
	SYD_OS_FUNCTION
	void					terminate();

	//	CLASS
	//	Os::Manager::Library -- ライブラリ関連の管理を行うクラス
	//
	//	NOTES

	class Library
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Os::Manager::Memory -- メモリ関連の管理を行うクラス
	//
	//	NOTES

	class Memory
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Os::Manager::MiniDumper -- ミニダンプの生成関連の管理を行うクラス
	//
	//	NOTES

	class MiniDumper
	{
		friend void Manager::initialize(const Path&, File::Permission::Value);
		friend void Manager::terminate();
	public:
		// 初期化時に指定されたディレクトリの下に
		// 例外情報を含まないミニダンプを生成する
		SYD_OS_FUNCTION
		static void
		execute();
		SYD_OS_FUNCTION
		static void
		execute(const UnicodeString& name);

	private:
		// 初期化を行う
		static void
		initialize(const Path& path, File::Permission::Value permission);
		// 後処理を行う
		static void			terminate();

#ifdef SYD_OS_WINDOWS
		// 初期化時に指定されたディレクトリの下に
		// 例外情報を含むミニダンプを生成する
		static void
		execute(EXCEPTION_POINTERS* ep);
		static void
		execute(const UnicodeString& name, EXCEPTION_POINTERS* ep);

		// 未処理の例外を処理する
		static LONG WINAPI
		topLevelExceptionFilter(EXCEPTION_POINTERS* ep);
#endif
	};
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_MANAGER_H

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
