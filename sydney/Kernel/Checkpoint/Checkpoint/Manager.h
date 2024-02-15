// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- チェックポイント処理マネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_CHECKPOINT_MANAGER_H
#define	__SYDNEY_CHECKPOINT_MANAGER_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN

//	NAMESPACE
//	Checkpoint::Manager --
//		チェックポイント処理マネージャー全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_CHECKPOINT_FUNCTION
	void					initialize();
	// 後処理を行う
	SYD_CHECKPOINT_FUNCTION
	void					terminate();

	//	CLASS
	//	Checkpoint::Manager::Daemon --
	//		チェックポイントデーモンスレッド関連の管理を行うクラス
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
#ifdef OBSOLETE
	//	CLASS
	//	Checkpoint::Manager::FileMover --
	//		ファイル移動関連のチェックポイント処理の管理を行うクラス
	//
	//	NOTES

	class FileMover
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};
#endif
	//	CLASS
	//	Checkpoint::Manager::FileDestroyer --
	//		ファイル破棄関連のチェックポイント処理の管理を行うクラス
	//
	//	NOTES

	class FileDestroyer
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
	//	Checkpoint::Manager::FileSynchronizer --
	//		バージョンファイルの同期処理の管理を行うクラス
	//
	//	NOTES

	class FileSynchronizer
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
	//	Checkpoint::Manager::Database --
	//		データベースに関する処理の管理を行うクラス
	//
	//	NOTES

	class Database
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
	//	Checkpoint::Manager::TimeStamp --
	//		タイムスタンプ関連の管理を行うクラス
	//
	//	NOTES

	class TimeStamp
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
	//	Checkpoint::Manager::Externalizable --
	//		シリアル化可能なオブジェクト関連の管理を行うクラス
	//
	//	NOTES

	class Externalizable
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

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_MANAGER_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
