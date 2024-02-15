// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionKeeper.h --
// 
// Copyright (c) 1999, 2000, 2004, 2005, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CONNECTIONKEEPER_H
#define __TRMEISTER_COMMUNICATION_CONNECTIONKEEPER_H

#include "Communication/Module.h"
#include "Communication/ConnectionMasterID.h"
#include "Communication/ConnectionSlaveID.h"
#include "Common/Object.h"
#include "Os/CriticalSection.h"
#include "ModHashMap.h"
#include "ModVector.h"
#include "ModConditionVariable.h"

_TRMEISTER_BEGIN

namespace Communication
{
class Connection;
class SerialIO;
	
//
//	CLASS
//	Communication::ConnectionKeeper -- 接続したソケットを管理する
//
//	NOTES
//	アクセプトしたソケットをキューに格納し、管理する。
//
class ConnectionKeeper : public Common::Object
{
public:
	//コンストラクタ
	ConnectionKeeper();
	//デストラクタ
	virtual ~ConnectionKeeper();

	//初期化
	void initialize();
	//後処理
	void terminate();

	//マップから対応するシリアルIOを取り出す
	SerialIO* consumeConnection(Connection* pConnection_);

	//シリアルIOをマップに登録する
	void execute(SerialIO* pSerialIO_, bool isCrypt_);

private:
	//マップ
	typedef ModHashMap<int, ModVector<ModPair<int, SerialIO*> >,
					   ModHasher<int> > Map;
	//サーバウエイトマップ
	typedef ModHashMap<int, ModConditionVariable*, ModHasher<int> > ServerMap;
	//タイムアウトマップ
	typedef ModHashMap<int, int, ModHasher<int> > TimeoutMap;
	
	//キュー
	Map m_mapSerialIO;
	//ウエイトサーバキュー
	ServerMap m_mapWaitServer;
	//タイムアウトキュー
	TimeoutMap m_mapTimeout;

	//クリティカルセクション
	Os::CriticalSection m_cCriticalSection;
	//終了かどうか
	bool m_fShutdown;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CONNECTIONKEEPER_H

//
//	Copyright (c) 1999, 2000, 2004, 2005, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
