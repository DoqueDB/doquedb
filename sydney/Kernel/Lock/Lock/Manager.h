// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- ロックマネージャ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_MANAGER_H
#define __SYDNEY_LOCK_MANAGER_H

#include "Lock/Module.h"

_SYDNEY_BEGIN

namespace Os
{
	class CriticalSection;
}

_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::Manager -- ロックマネージャー全体の管理を行うクラス
//
//	NOTES

class Manager
{
	friend class Client;
	friend class Item;
	friend class Request;
public:
	// 初期化を行う
	SYD_LOCK_FUNCTION
	static void				initialize();
	// 後処理を行う
	SYD_LOCK_FUNCTION
	static void				terminate();

	//	CLASS
	//	Lock::Manager::Client -- ロック要求元関連の管理を行うクラス
	//
	//	NOTES

	class Client
	{
		friend class Manager;
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Lock::Manager::Request -- ロック要求関連の管理を行うクラス
	//
	//	NOTES

	class Request
	{
		friend class Manager;
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Lock::Manager::Count -- ロック数関連の管理を行うクラス
	//
	//	NOTES

	class Count
	{
		friend class Manager;
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Lock::Manager::Item -- ロック項目関連の管理を行うクラス
	//
	//	NOTES

	class Item
	{
		friend class Manager;
	private:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

private:
	// ロックモジュール内部のスレッド間排他制御用のラッチを得る
	static Os::CriticalSection&
	getLatch();
};

_SYDNEY_LOCK_END
_SYDNEY_END

#endif //__SYDNEY_LOCK_MANAGER_H

//
//	Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
