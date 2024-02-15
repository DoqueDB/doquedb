// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- 
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_UTILITY_MANAGER_H
#define	__TRMEISTER_UTILITY_MANAGER_H

#include "Utility/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_UTILITY_BEGIN

//	NAMESPACE
//	Utility::Manager -- ユーティリティモジュール全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_UTILITY_FUNCTION
	void					initialize();
	// 後処理を行う
	SYD_UTILITY_FUNCTION
	void					terminate();

	//	CLASS
	//	Utility::Manager::OpenMP -- OpenMP関連の管理を行うクラス
	//
	//	NOTES

	class OpenMP
	{
	public:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};

	//	CLASS
	//	Utility::Manager::Una -- UNA関連の管理を行うクラス
	//
	//	NOTES

	class Una
	{
	public:
		// 初期化を行う
		static void			initialize();
		// 後処理を行う
		static void			terminate();
	};
}

_TRMEISTER_UTILITY_END
_TRMEISTER_END

#endif	// __SYDNEY_UTILITY_MANAGER_H

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
