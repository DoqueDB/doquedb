// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Singleton.h -- Sydneyに必要な初期化と終了処理
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

#ifndef __SYDNEY_SERVER_SINGLETON_H
#define __SYDNEY_SERVER_SINGLETON_H

#include "Server/Module.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

class Manager;

//
//	CLASS
//	Server::Singleton -- Sydneyに必要な初期化と終了処理
//
//	NOTES
//	Sydneyに必要な初期化と終了処理。
//
class Singleton
{
public:
	//
	//	CLASS
	//	Server::Singleton::InProcess
	//
	//	NOTES
	//	InProcess用の初期化と後処理
	//
	class InProcess
	{
	public:
		//初期化
		SYD_SERVER_FUNCTION static void initialize
			(bool installation, const ModUnicodeString& regPath);
		//終了処理
		SYD_SERVER_FUNCTION static void terminate();
		//パラメータを読み直す
		SYD_SERVER_FUNCTION static void reload();
	};

	//
	//	CLASS
	//	Server::Singleton::RemoteServer
	//
	//	NOTES
	//	RemoteServer用の初期化と後処理
	//
	class RemoteServer
	{
	public:
		//初期化
		SYD_SERVER_FUNCTION static void initialize
			(bool installation, const ModUnicodeString& regPath);
		//終了処理
		SYD_SERVER_FUNCTION static void terminate();
		//終了する
		SYD_SERVER_FUNCTION static void stop();
		//終了を待つ
		SYD_SERVER_FUNCTION static void join();
		//起動しているかチェックする
		SYD_SERVER_FUNCTION static bool isRunning();
		//パラメータを読み直す
		SYD_SERVER_FUNCTION static void reload();
	};

private:
	friend class InProcess;
	friend class RemoteServer;

	//Manager
	static Manager* m_pManager;
	//初期化カウンタ
	static int m_iInitialized;
};

_SYDNEY_SERVER_END
_SYDNEY_END

//
//	以下は、SyKernel.dllをLoadLibraryで使用するためのインターフェース
//

extern "C" {

//
//	FUNCTION
//	SydServerInitialize -- サーバの初期化
//
SYD_SERVER_FUNCTION
void SydServerInitialize(bool installation, const unsigned short* regPath);

//
//	FUNCTION
//	SydServerTerminate -- サーバの終了処理
//
SYD_SERVER_FUNCTION
void SydServerTerminate();

//
//	FUNCTION
//	SydServerStop -- サーバの停止
//
SYD_SERVER_FUNCTION
void SydServerStop();

//
//	FUNCTION
//	SydServerJoin -- サーバの停止を待つ
//
SYD_SERVER_FUNCTION
void SydServerJoin();

//
//	FUNCTION
//	SydServerIsRunning -- 起動しているかチェックする
//
SYD_SERVER_FUNCTION
bool SydServerIsRunning();

//
//	FUNCTION
//	SydServerReload -- パラメータを読み直す
//
SYD_SERVER_FUNCTION
void SydServerReload();

}

#endif // __SYDNEY_SERVER_SINGLETON_H

//
//	Copyright (c) 2000, 2002, 2003, 2005, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
