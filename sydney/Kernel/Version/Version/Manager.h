// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- 版管理マネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_MANAGER_H
#define	__SYDNEY_VERSION_MANAGER_H

#include "Version/Module.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	NAMESPACE
//	Version::Manager -- 版管理マネージャー全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_VERSION_FUNCTION
	void					initialize();
	// 後処理を行う
	SYD_VERSION_FUNCTION
	void					terminate();

	//	CLASS
	//	Version::Manager::File -- バージョンファイル関連の管理を行うクラス
	//
	//	NOTES

	class File
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
	//	Version::Manager::Page -- バージョンページ関連の管理を行うクラス
	//
	//	NOTES

	class Page
	{
		friend void Manager::initialize();
		friend void Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Version::Manager::Verification -- 整合性検査関連の管理を行うクラス
	//
	//	NOTES

	class Verification
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
	//	Version::Manager::Daemon --
	//		バージョンファイルに関係するデータ構造を処理する
	//		デーモンスレッド関連の管理を行うクラス
	//
	//	NOTES

	class Daemon
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_MANAGER_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
