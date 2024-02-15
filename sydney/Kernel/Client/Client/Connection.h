// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.h --
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

#ifndef __TRMEISTER_CLIENT_CONNECTION_H
#define __TRMEISTER_CLIENT_CONNECTION_H

#include "Client/Module.h"
#include "Client/Object.h"
#include "Client/DataSource.h"
#include "Os/CriticalSection.h"
#include "Common/Externalizable.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

class Port;

//
//	CLASS
//	Client::Connection --
//
//	NOTES
//
//
class Connection : public Object
{
public:
	//コンストラクタ
	Connection(DataSource& cDataSource, Port* pPort_);
	//デストラクタ
	virtual ~Connection();

	//切断する
	void close();

	//workerを起動する
	Port* beginWorker(int& iWorkerID_);

	//workerをcancelする
	void cancelWorker(int iWorkerID_);

	//最適化結果を削除する
	void erasePrepareStatement(const ModUnicodeString& cstrDatabaseName_,
								int PrepareID_);

	//使用しないポートを削除する
	void disconnectPort(const ModVector<int>& veciSlaveID_);

	//新しいクライアントコネクションを得る
	Connection* beginConnection();

	// 利用可能性をチェックする
	bool isServerAvailable();
	bool isDatabaseAvailable(Database::ID id_);

private:
	//データソース
	DataSource& m_cDataSource;
	//サーバコネクションとのポート
	Port* m_pPort;
	//サーバとの ポートの排他制御用のクリティカルセクション
	Os::CriticalSection m_cPortLatch;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_CONNECTION_H

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
