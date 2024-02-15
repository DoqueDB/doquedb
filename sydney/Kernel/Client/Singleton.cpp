// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Singleton.cpp -- Sydneyに必要な初期化と終了処理
// 
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Client/Singleton.h"
#include "Common/Manager.h"
#include "Os/Manager.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

//
//	VARIABLE private
//	Singleton::m_iInitialized -- 初期化カウンタ
//
//	NOTES
//	初期化カウンタ
//
int Singleton::m_iInitialized = 0;

//
//	FUNCTION public static
//	Client::Singleton::RemoteServer::initialize -- 初期化を行う
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
//
void
Singleton::RemoteServer::initialize(const ModUnicodeString& regPath)
{
	if (Singleton::m_iInitialized++ == 0)
	{
		// 共有ライブラリの初期化

		Common::Manager::initialize(regPath);

		// Os の初期化

		Os::Manager::initialize();
	}
}

//
//	FUNCTION public static
//	Client::Singleton::RemoteClient::terminate -- Sydneyの終了処理を行う
//
//	NOTES
//	Sydneyの終了処理を行う。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Singleton::RemoteServer::terminate()
{
	if (--Singleton::m_iInitialized == 0)
	{
		// 共通ライブラリの後処理

		Common::Manager::terminate();

		// Os の終了処理

		Os::Manager::terminate();
	}
}

//
//	Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
