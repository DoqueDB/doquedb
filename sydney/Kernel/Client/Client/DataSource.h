// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.h --
// 
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT_DATASOURCE_H
#define __TRMEISTER_CLIENT_DATASOURCE_H

#include "Client/Module.h"
#include "Client/Object.h"
#include "Common/Thread.h"
#include "Os/CriticalSection.h"

#include "ModHashMap.h"
#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

class Session;
class Connection;
class Port;
class PrepareStatement;

//
//	NAMESPACE
//	Client::Database -- データベース
//
namespace Database
{
	// データベースID
	typedef unsigned int	ID;

	// 全データベースをあらわすID
	const ID	All = 0xffffffff;
}

//
//	CLASS
//	Client::DataSource --
//
//	NOTES
//	InProcessで使用する場合には、Client::DataSource::open()
//	を実行する前に、Server::Singleton::InProcess::initialize()
//	を実行する必要がある。
//	RemoteServerで使用する場合には、Client::DataSource::open()
//	を実行する前に、Client::Singleton::RemoteServer::initialize()
//	を実行する必要がある。
//
class SYD_FUNCTION DataSource : public Common::Thread
{
public:
	// プロトコルバージョン
	struct Protocol
	{
		enum Value
		{
			Version1 = 0,	// v14互換モード
			Version2,		// v15初期バージョン

			ValueNum,
			CurrentVersion = ValueNum - 1
		};
	};

	//コンストラクタ(InProcess用)
	DataSource();
	//コンストラクタ(RemoteServer用)
	DataSource(const ModUnicodeString& cstrHostName_,
				int iPortNumber_);
	//デストラクタ
	virtual ~DataSource();

	//接続する
	void open(Protocol::Value eVersion_ = Protocol::Version1);
	//切断する
	void close();
	//メモリーを解放する
	void release();

	//サーバを停止する(RemoteServer用)
	void shutdown();

	//Sessionを作成する
	Session* createSession(const ModUnicodeString& cstrDatabaseName_);

	//PrepareStatementを作成する
	PrepareStatement* createPrepareStatement(
		const ModUnicodeString& cstrDatabaseName_,
		const ModUnicodeString& cstrStatement_);

	// サーバの利用可能性を得る
	bool isServerAvailable();
	// データベースの利用可能性を得る
	bool isDatabaseAvailable(Database::ID databaseID_ = Database::All);

	//以下はClient内クラス用のメソッド

	//コネクション管理クラスを得る(ラウンドロビン方式)
	Connection* getClientConnection();

	//ポートプールからポートを取り出す
	Port* popPort();
	//ポートプールにポートを挿入する
	void pushPort(Port* pPort_);

	//新しいポートのインスタンスを得る
	Port* getNewPort(int iSlaveID_);

	//指定されたクラスのインスタンス数をインクリメントする
	void incrementCount(Client::Object::Type::Value eType_);
	//指定されたクラスのインスタンス数をデクリメントする
	void decrementCount(Client::Object::Type::Value eType_);
	//指定されたクラスのインスタンス数を得る
	int getCount(Client::Object::Type::Value eType_);

	// データソースのインスタンスを得る
	static DataSource* createDataSource();		// InProcess用
	static DataSource* createDataSource(
		const ModUnicodeString& cstrHostName_,
		int iPortNumber_);		// RemoteServer用

private:
	//typedef
	typedef ModHashMap<int, Port*, ModHasher<int> > PortMap;

	//ポートプールを管理するスレッド
	void runnable();

	//新しいクライアントコネクションをはる
	void newClientConnect();

	//コネクション管理クラス
	ModVector<Connection*> m_vecpClientConnection;
	//最近返したコネクション
	int m_iConnectionArgument;
	//クライアントコネクション用のクリティカルセクション
	Os::CriticalSection m_cClientConnectionLatch;

	//ポートプール
	PortMap m_mapPort;
	//ポートプール排他制御用のクリティカルセクション
	Os::CriticalSection m_cPortMapLatch;

	//接続先のホスト名(RemoteServer用)
	ModUnicodeString m_cstrHostName;
	//ポート番号(RemoteServer用)
	int m_iPortNumber;

	//オープンカウンタ
	int m_iOpenCounter;

	//各クラスのインスタンス数
	int m_pInstanceCount[Client::Object::Type::ElementSize];
	//インスタンス数の排他制御用のクリティカルセクション
	Os::CriticalSection m_cInstanceLatch;

	// プロトコルバージョン
	Protocol::Value m_eProtocolVersion;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_DATASOURCE_H

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
