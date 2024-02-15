// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- OS 管理マネージャー関連の関数定義
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Os/Manager.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace 
{
}

//	FUNCTION
//	Os::Manager::initialize -- OS 管理マネージャーの初期化を行う
//
//	NOTES
//		ミニダンプ関連を初期化しない
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
	// ライブラリ関連の初期化を行う

	Manager::Library::initialize();

	// メモリ関連の初期化を行う

	Manager::Memory::initialize();
}

//	FUNCTION
//	Os::Manager::initialize -- OS 管理マネージャーの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ミニダンプを記録するファイルの親ディレクトリの絶対パス名
//		Os::File::Permission::Value	permission
//			ミニダンプを記録するファイルの許可モード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::initialize(const Path& path, File::Permission::Value permission)
{
	initialize();

	// ミニダンプの生成関連の初期化を行う

	Manager::MiniDumper::initialize(path, permission);
}

//	FUNCTION
//	Os::Manager::terminate -- OS 管理マネージャーの後処理を行う
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
	// ミニダンプの生成関連の後処理を行う

	Manager::MiniDumper::terminate();

	// メモリ関連の後処理を行う

	Manager::Memory::terminate();

	// ライブラリ関連の

	Manager::Library::terminate();
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
