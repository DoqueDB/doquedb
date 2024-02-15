// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- トランザクションマネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_MANAGER_H
#define	__SYDNEY_TRANS_MANAGER_H

#include "Trans/Module.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

namespace Log
{
	class File;
}

//	NAMESPACE
//	Trans::Manager -- トランザクションマネージャー全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_TRANS_FUNCTION
	void					initialize();
	// 後処理の準備を行う
	SYD_TRANS_FUNCTION
	void
	prepareTermination();
	// 後処理を行う
	SYD_TRANS_FUNCTION
	void					terminate();

	// インストールする
	SYD_TRANS_FUNCTION
	void					install();
	// アンインストールする
	SYD_TRANS_FUNCTION
	void					uninstall();

	//	CLASS
	//	Trans::Manager::Branch -- トランザクションブランチ関連の管理を行うクラス
	//
	//	NOTES

	class Branch
	{
		friend void Manager::initialize();
		friend void Manager::prepareTermination();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void
		initialize();
		// 後処理の準備を行う
		static void
		prepareTermination();
		// 後処理を行う
		static void
		terminate();
	};

	//	CLASS
	//	Trans::Manager::Transaction -- トランザクション関連の管理を行うクラス
	//
	//	NOTES

	class Transaction
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	NAMESPACE
	//	Trans::Manager::Log -- 論理ログ関連の管理を行う名前空間
	//
	//	NOTES

	namespace Log
	{
		//	CLASS
		//	Trans::Manager::Log::File --
		//		ある論理ログファイルに関する情報関連の管理を行うクラス
		//
		//	NOTES

		class File
		{
			friend void Manager::initialize();
			friend void Manager::terminate();
		private:
			// 初期化を行う
			static void			initialize();
			// 後処理を行う
			static void			terminate();
		};
	}

	//	CLASS
	//	Trans::Manager::TimeStamp -- タイムスタンプ関連の管理を行うクラス
	//
	//	NOTES

	class TimeStamp
	{
		friend void Manager::initialize();
		friend void	Manager::terminate();
		friend void Manager::install();
		friend void Manager::uninstall();
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();

		// インストールする
		static void			install();
		// アンインストールする
		static void			uninstall();
	};

	//	CLASS
	//	Trans::Manager::Externalizable --
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

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_MANAGER_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
