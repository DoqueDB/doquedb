// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- 版管理マネージャー関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "Version/Manager.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace 
{
}

//	FUNCTION
//	Version::Manager::initialize -- 版管理マネージャーの初期化を行う
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

void
Manager::initialize()
{
	// バージョンページ関連の初期化を行う

	Manager::Page::initialize();

	// バージョンファイル関連の初期化を行う

	Manager::File::initialize();

	// 整合性検査関連の初期化を行う

	Manager::Verification::initialize();

	// デーモンスレッド関連の初期化を行う

	Manager::Daemon::initialize();
}

//	FUNCTION
//	Version::Manager::terminate -- 版管理マネージャーの後処理を行う
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
Manager::terminate()
{
	// デーモンスレッド関連の後処理を行う

	Manager::Daemon::terminate();

	// 整合性検査関連の後処理を行う

	Manager::Verification::terminate();

	// バージョンファイル関連の後処理を行う

	Manager::File::terminate();

	// バージョンページ関連の後処理を行う

	Manager::Page::terminate();

	// 設定をすべてリセットする

	Configuration::reset();
}

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
