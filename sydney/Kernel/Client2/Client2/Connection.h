// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.h -- クライアントコネクション関連のクラス定義、関数宣言
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

#ifndef __TRMEISTER_CLIENT2_CONNECTION_H
#define __TRMEISTER_CLIENT2_CONNECTION_H

#include "Client2/Module.h"
#include "Client2/Object.h"
#include "Client2/DataSource.h"
#include "Os/CriticalSection.h"
#include "Common/Externalizable.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

class Port;

//	CLASS
//	Client2::Connection --
//		クライアントコネクションクラス
//
//	NOTES

class Connection : public Object
{
public:

	// コンストラクタ
	Connection(	DataSource&	cDataSource_,
				Port*		pPort_);

	// デストラクタ
	virtual ~Connection();

	// クローズする
	void close();

	// ワーカを起動する
	Port* beginWorker();

	// ワーカをキャンセルする
	void cancelWorker(int	iWorkerID_);

	// プリペアステートメントを削除する
	void erasePrepareStatement(	const ModUnicodeString&	cstrDatabaseName_,
								int						iPrepareID_);

	// 使用しない通信ポートを削除する
	void disconnectPort(const ModVector<int>&	veciSlaveID_);

	// 新しいコネクションを得る
	Connection* beginConnection();

	// 利用可能性をチェックする
	bool isServerAvailable();
	bool isDatabaseAvailable(Database::ID	iID_);

private:

	// 自分の属するデータソース
	DataSource&			m_cDataSource;

	// サーバ側コネクションスレッドとの通信ポート
	Port*				m_pPort;

	// サーバとのポートの排他制御用のクリティカルセクション
	Os::CriticalSection	m_cPortLatch;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_CONNECTION_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
