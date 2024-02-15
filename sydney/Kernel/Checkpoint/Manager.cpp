// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- チェックポイント処理マネージャー関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Configuration.h"
#include "Checkpoint/Manager.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace 
{
}

//	FUNCTION
//	Checkpoint::Manager::initialize --
//		チェックポイント処理マネージャーの初期化を行う
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
	// シリアル化可能なオブジェクト関連の初期化を行う

	Externalizable::initialize();

	// タイムスタンプ関連の初期化を行う

	Manager::TimeStamp::initialize();

	// データベースに関する処理関連の初期化を行う

	Manager::Database::initialize();
#ifdef OBSOLETE
	// ファイル移動関連の初期化を行う

	Manager::FileMover::initialize();
#endif

	// ファイル破棄関連の初期化を行う

	Manager::FileDestroyer::initialize();

	// バージョンファイルの同期処理関連の初期化を行う

	Manager::FileSynchronizer::initialize();

	// チェックポイントデーモンスレッド関連の初期化を行う

	Manager::Daemon::initialize();
}

//	FUNCTION
//	Checkpoint::Manager::terminate --
//		チェックポイント処理マネージャーの後処理を行う
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
	// チェックポイントデーモンスレッド関連の後処理を行う

	Manager::Daemon::terminate();

	// バージョンファイルの同期処理関連の後処理を行う

	Manager::FileSynchronizer::terminate();

	// ファイル破棄関連の後処理を行う

	Manager::FileDestroyer::terminate();

#ifdef OBSOLETE
	// ファイル移動関連の後処理を行う

	Manager::FileMover::terminate();
#endif
	// データベースに関する処理関連の後処理を行う

	Manager::Database::terminate();

	// タイムスタンプ関連の後処理を行う

	Manager::TimeStamp::terminate();

	// シリアル化可能なオブジェクト関連の後処理を行う

	Externalizable::terminate();

	// 設定をすべてリセットする

	Configuration::reset();
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
