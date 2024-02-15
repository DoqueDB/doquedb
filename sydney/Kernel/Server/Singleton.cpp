// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Singleton.cpp -- Sydneyに必要な初期化と終了処理
// 
// Copyright (c) 2000, 2002, 2003, 2005, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Server/Type.h"
#include "Server/Singleton.h"
#include "Server/Manager.h"
#include "Common/Message.h"
#include "Common/MessageStreamBuffer.h"
#include "Common/SystemParameter.h"
#include "Common/Thread.h"
#include "Common/UnicodeString.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

//
//	VARIABLE private
//	Singleton::m_pManager -- サーバオブジェクト
//
//	NOTES
//	サーバオブジェクト。
//
Manager* Singleton::m_pManager = 0;

//
//	VARIABLE private
//	Singleton::m_iInitialized -- 初期化カウンタ
//
//	NOTES
//	初期化カウンタ
//
int Singleton::m_iInitialized = 0;

//	FUNCTION public
//	Server::Singleton::InProcess::initialize -- 初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	bool			installation
//		true
//			初期化しながら、インストールも行う
//		false または指定されいないとき
//			ただ、初期化する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Singleton::InProcess::initialize(bool installation,
								 const ModUnicodeString& regPath)
{
	if (Singleton::m_iInitialized++ == 0)
	{

		Singleton::m_pManager = new Manager(Server::InProcess);

		// Singletonのinitializeでは
		// サーバースレッドのrunnable中での
		// 初期化を待っていてはクライアント側の
		// 操作が初期化前に実行される可能性があるので
		// ここで初期化する
		Singleton::m_pManager->initialize(installation, regPath);

		if (!installation)
			// スレッドを起動する
			Singleton::m_pManager->create();
	}
}

//
//	FUNCTION public
//	Server::Singleton::InProcess::terminate -- Sydneyの終了処理を行う
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
Singleton::InProcess::terminate()
{
	if (--Singleton::m_iInitialized == 0)
	{
		// 停止のリクエストをする
		Singleton::m_pManager->stop();

		try
		{
			// スレッドが終了するのを待つ
			Singleton::m_pManager->join();
		}
		catch (ModException&)
		{
			//スレッドが起動していないと例外が発生する
			Common::Thread::resetErrorCondition();
		}

		// 後処理を行う
		Singleton::m_pManager->terminate();

		delete Singleton::m_pManager;
	}
}

//
//	FUNCTION public
//	Server::Singleton::InProcess::reload -- パラメータを読み直す
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
Singleton::InProcess::reload()
{
	// RemoteServerと同じ
	Singleton::RemoteServer::reload();
}

//	FUNCTION public
//	Server::Singleton::RemoteServer::initialize -- 初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	bool			installation
//		true
//			初期化しながら、インストールも行う
//		false または指定されいないとき
//			ただ、初期化する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Singleton::RemoteServer::initialize(bool installation,
									const ModUnicodeString& regPath)
{
	if (Singleton::m_iInitialized++ == 0)
	{
		Singleton::m_pManager = new Manager(Server::RemoteServer);

		// Singletonのinitializeでは
		// サーバースレッドのrunnable中での
		// 初期化を待っていてはクライアント側の
		// 操作が初期化前に実行される可能性があるので
		// ここで初期化する

		Singleton::m_pManager->initialize(installation, regPath);

		if (!installation)
			// スレッドを起動する
			Singleton::m_pManager->create();
	}
}

//
//	FUNCTION public
//	Server::Singleton::RemoteServer::terminate -- Sydneyの終了処理を行う
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
		// 後処理を行う
		Singleton::m_pManager->terminate();

		delete Singleton::m_pManager;
	}
}

//
//	FUNCTION public
//	Server::Singleton::RemoteServer::stop -- Sydneyを終了する
//
//	NOTES
//	Sydneyを終了する。
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
Singleton::RemoteServer::stop()
{
	// 終了する
	Singleton::m_pManager->stop();
}

//
//	FUNCTION public
//	Server::Singleton::RemoteServer::join -- Sydneyの終了を待つ
//
//	NOTES
//	Sydneyの終了処理を待つ。
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
Singleton::RemoteServer::join()
{
	try
	{
		// 終了を待つ
		Singleton::m_pManager->join();
	}
	catch (ModException&)
	{
		//起動していないと例外が発生する
		Common::Thread::resetErrorCondition();
	}
}

//
//	FUNCTION public
//	Server::Singleton::RemoteServer::isRunning -- Sydneyが起動しているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		起動している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Singleton::RemoteServer::isRunning()
{
	return (m_pManager->getStatus() == Common::Thread::Running);
}

//
//	FUNCTION public
//	Server::Singleton::RemoteServer::reload -- パラメータを読み直す
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
Singleton::RemoteServer::reload()
{
	// パラメータを読み直す
	Common::SystemParameter::reset();
	
	// ログファイルをオープンし直す
	Common::MessageStreamBuffer::reset();

	// ログ出力する
	SydInfoMessage << "Parameter reloaded." << ModEndl;
}

//
//	以下、SyKernel.dllをLoadLibraryするためのインターフェース
//	すべての関数が extern "C" してある。
//

//
//	FUNCTION global
//	SydServerInitialize -- サーバの初期化
//
void
SydServerInitialize(bool installation, const unsigned short* regPath)
{
	ModMemoryPool::setTotalLimit(ModSizeMax >> 10); // KB単位
	ModOs::Process::setEncodingType(Common::LiteralCode);

	Singleton::RemoteServer::initialize(installation, regPath);
}

//
//	FUNCTION global
//	SydServerTerminate -- サーバの終了処理
//
void
SydServerTerminate()
{
	Singleton::RemoteServer::terminate();
}

//
//	FUNCTION global
//	SydServerStop -- サーバを停止する
//
void
SydServerStop()
{
	Singleton::RemoteServer::stop();
}

//
//	FUNCTION global
//	SydServerJoin -- サーバの停止を待つ
//
void
SydServerJoin()
{
	Singleton::RemoteServer::join();
}

//
//	FUNCTION global
//	SydServerIsRunning -- サーバが起動しているかチェックする
//
bool
SydServerIsRunning()
{
	return Singleton::RemoteServer::isRunning();
}

//
//	FUNCTION global
//	SydServerReload -- パラメータを読み直す
//
void
SydServerReload()
{
	return Singleton::RemoteServer::reload();
}

//
//	Copyright (c) 2000, 2002, 2003, 2005, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
