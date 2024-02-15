// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- 共通ライブラリマネージャ関連の関数定義
// 
// Copyright (c) 2004, 2006, 2009, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ErrorMessageManager.h"
#include "Common/Manager.h"
#include "Common/MessageStreamBuffer.h"
#include "Common/Object.h"
#include "Common/SystemParameter.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
}

//	FUNCTION
//	Common::Manager::initialize -- 共通ライブラリマネージャーの初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	regParent
//			システムパラメータが記憶されている場所の親パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::initialize(const ModUnicodeString& regParent)
{
	// オブジェクトクラス関連の初期化を行う

	Object::initialize();

	// システムパラメータを初期化する

	SystemParameter::initialize(regParent);

	// エラーメッセージ関連の初期化を行う

	ErrorMessageManager::initialize();
}

//	FUNCTION
//	Common::Manager::terminate -- 共通ライブラリマネージャーの後処理を行う
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
	//エラーメッセージの終了処理

	ErrorMessageManager::terminate();

	//メッセージストリームの終了処理

	MessageStreamBuffer::terminate();

	// オブジェクトクラスの後処理

	Common::Object::terminate();
}

//
// Copyright (c) 2004, 2006, 2009, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

