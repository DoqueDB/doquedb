// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Singleton.cpp -- Sydney に必要な初期化と終了処理
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Client2/Singleton.h"
#include "Common/Manager.h"
#include "Os/Manager.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	VARIABLE private
//	Client2::Singleton::m_iInitialized -- 初期化カウンタ
//
//	NOTES

int	Singleton::m_iInitialized = 0;

//	FUNCTION public static
//	Client2::Singleton::RemoteServer::initialize -- 初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrRegPath_
//		システムパラメータが記憶されている場所の親パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS

// static
void
Singleton::RemoteServer::initialize(const ModUnicodeString& cstrRegPath_)
{
	if (Singleton::m_iInitialized++ == 0) {

		// 共有ライブラリの初期化
		Common::Manager::initialize(cstrRegPath_);

		// Os の初期化
		Os::Manager::initialize();
	}
}

//	FUNCTION public static
//	Client2::Singleton::RemoteClient2::terminate -- Sydney の終了処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

// static
void
Singleton::RemoteServer::terminate()
{
	if (--Singleton::m_iInitialized == 0) {

		// 共通ライブラリの後処理
		Common::Manager::terminate();

		// Os の終了処理
		Os::Manager::terminate();
	}
}

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
