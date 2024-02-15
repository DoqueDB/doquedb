// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionSupplier.h -- サーバシリアルIOのデーモンを実現するクラス
// 
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CONNECTIONSUPPLIER_H
#define __TRMEISTER_COMMUNICATION_CONNECTIONSUPPLIER_H

#include "Communication/Module.h"

_TRMEISTER_BEGIN

namespace Communication
{
class ConnectionKeeper;
class SocketDaemon;
class MemoryDaemon;
class Connection;

//
//	CLASS
//	ConnectionSupplier -- サーバシリアルIOのデーモンを実現するためのクラス
//
//	NOTES
//	サーバシリアルIOのデーモンを実現するためのクラス。
//
class ConnectionSupplier
{
public:
	//デーモン種別
	enum
	{
		Socket,
		Memory
	};

	//初期化を行う
	SYD_COMMUNICATION_FUNCTION static void initialize();
	//後処理を行う
	SYD_COMMUNICATION_FUNCTION static void terminate();

	//コネクションデーモンを起動する
	SYD_COMMUNICATION_FUNCTION static void create(int iType_);
	//コネクションデーモンを停止する
	SYD_COMMUNICATION_FUNCTION static void abort();

	//コネクションオブジェクトを関連づける
	SYD_COMMUNICATION_FUNCTION static void consumeConnection(Connection* pConnection_);

private:
	//ソケットデーモン
	static SocketDaemon* m_pSocketDaemon;
	//メモリーデーモン
	static MemoryDaemon* m_pMemoryDaemon;
	//アクセプトしたシリアルIOを保存する
	static ConnectionKeeper* m_pConnectionKeeper;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CONNECTIONSUPPLIER_H

//
//	Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
