// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Singleton.h -- クライアントに必要な初期化と終了処理
// 
// Copyright (c) 2002, 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT_SINGLETON_H
#define __TRMEISTER_CLIENT_SINGLETON_H

#include "Client/Module.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

//
//	CLASS
//	Client::Singleton -- クライアントに必要な初期化と終了処理
//
//	NOTES
//	クライアントに必要な初期化と終了処理。
//
class Singleton
{
public:
	//
	//	CLASS
	//	Client::Singleton::RemoteServer
	//
	//	NOTES
	//	RemoteServer用の初期化と後処理
	//
	class RemoteServer
	{
	public:
		//初期化
		SYD_FUNCTION static void
		initialize(const ModUnicodeString& regPath = ModUnicodeString());
		//終了処理
		SYD_FUNCTION static void terminate();
	};

private:
	friend class RemoteServer;

	//初期化カウンタ
	static int m_iInitialized;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif // __TRMEISTER_CLIENT_SINGLETON_H

//
//	Copyright (c) 2002, 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
