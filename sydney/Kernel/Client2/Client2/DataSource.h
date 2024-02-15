// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.h -- データソース関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_DATASOURCE_H
#define __TRMEISTER_CLIENT2_DATASOURCE_H

#include "Client2/Module.h"
#include "Client2/Object.h"
#include "Common/Thread.h"
#include "Communication/Protocol.h"
#include "Communication/Crypt.h"
#include "Os/CriticalSection.h"

#include "ModHashMap.h"
#include "ModVector.h"


_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

class Session;
class Connection;
class Port;
class PrepareStatement;



//	NAMESPACE
//	Client2::Database -- データベース

namespace Database
{
	// データベースID
	typedef unsigned int	ID;

	// 全データベースをあらわすID
	const ID	All = 0xffffffff;
}

//	CLASS
//	Client2::DataSource -- データソースをあらわすクラス
//
//	NOTES

class SYD_FUNCTION DataSource : public Common::Thread
{
public:
	// アドレスファミリー
	struct Family {
		enum Value {
			Unspec,		// 指定しない

			IPv4,		// IPv4を指定する
			IPv6		// IPv6を指定する
		};
	};
	
	// プロトコルバージョン
	typedef Communication::Protocol Protocol;

	// コンストラクタ ( InProdess 用 )
	DataSource(bool bCreateThread_ = true);

	// コンストラクタ ( RemoveServer 用 )
	DataSource(const ModUnicodeString&	cstrHostName_,
			   int						iPortNumber_,
			   Family::Value			eFamily = Family::Unspec,
			   bool						bCreateThread_ = true);

	// デストラクタ
	virtual ~DataSource();

	// オープンする
	void open(Protocol::Value	eProtocolVersion_ = Protocol::CurrentVersion);

	// クローズする
	void close();

	// メモリーを解放する
	void release();

	// サーバを停止する
	void shutdown();
	void shutdown(const ModUnicodeString& cstrUserName_,
				  const ModUnicodeString& cstrPassword_);

	// 新しくセッションを作成する
	Session* createSession(const ModUnicodeString&	cstrDatabaseName_);
	Session* createSession(const ModUnicodeString&	cstrDatabaseName_,
						   const ModUnicodeString& cstrUserName_,
						   const ModUnicodeString& cstrPassword_);

	// 新しくプリペアステートメントを作成する
	PrepareStatement* createPrepareStatement(
		const ModUnicodeString&	cstrDatabaseName_,
		const ModUnicodeString&	cstrStatement_);

	// サーバの利用可能性を得る
	bool isServerAvailable();

	// データベースの利用可能性を得る
	bool isDatabaseAvailable(Database::ID	iID_ = Database::All);

	// コネクション管理クラスを得る ( ラウンドロビン方式 )
	Connection* getClientConnection();

	// ポートプールからポートを取り出す
	Port* popPort();

	// ポートプールにポートを挿入する
	void pushPort(Port*	pPort_);

	// 廃棄したポートを登録する
	void expungePort(Port*	pPort_);

	// 新しいポートのインスタンスを得る
	Port* getNewPort(int	iSlaveID_);

	// マスター ID を得る
	int getMasterID();

	// 暗号アルゴリズムを得る(暗号化対応)
	int getAlgorithm();

	// 認証方式を得る(ユーザー管理対応)
	int getAuthorization();

	// セッションが存在しているかどうか
	bool isSessionExist();

	// セッションを削除する
	void removeSession(int	iID_);

	// データソースのインスタンスを得る
	static DataSource* createDataSource(bool bCreateThread_ = true);
	static DataSource* createDataSource(
		const ModUnicodeString&	cstrHostName_,
		int						iPortNumber_,
		Family::Value			eFamily = Family::Unspec,
		bool					bCreateThread = true);

	// 共通鍵取得(暗号化対応)
	Communication::CryptKey::Pointer getKey() const { return m_pCommonKey; }

private:

	// 新しいクライアントコネクションを作成する
	void newClientConnect();

	// ポートプールを管理するスレッド
	void runnable();

	// セッションを登録する
	void putSession(Session*	pSession_);

	// コネクションの配列
	ModVector<Connection*>	m_vecpConnectionArray;

	// 最近返したコネクションの要素番号
	ModSize					m_iConnectionElement;

	// クライアントコネクション用のクリティカルセクション
	Os::CriticalSection		m_cConnectionArrayLatch;

	// ポートのハッシュマップ
	typedef ModHashMap<int, Port*, ModHasher<int> > PortMap;
	PortMap					m_cPortMap;

	// ポートプール排他制御用のクリティカルセクション
	Os::CriticalSection		m_cPortMapLatch;

	// クライアントが廃棄したポート
	ModVector<int>			m_vecpExpungePort;

	// 接続先のホスト名
	ModUnicodeString		m_cstrHostName;

	// ポート番号
	int						m_iPortNumber;

	// アドレスファミリー指定
	Family::Value			m_eFamily;

	// セッションのハッシュマップ
	typedef ModHashMap<int, Session*, ModHasher<int> > SessionMap;
	SessionMap				m_cSessionMap;

	// セッションプール排他制御用のクリティカルセクション
	Os::CriticalSection		m_cSessionMapLatch;

	// クローズしたかどうか
	bool					m_bIsClosed;

	// マスター ID
	int						m_iMasterID;

	// 暗号アルゴリズム(暗号化対応)
	int						m_iAlgorithm;

	// 認証方式
	int						m_iAuthorization;

	// プロトコルバージョン
	Protocol::Value			m_eProtocolVersion;

	// 排他制御用のクリティカルセクション
	Os::CriticalSection		m_cLatch;

	// 共通鍵用(暗号化対応)
	Communication::CryptKey::Pointer m_pCommonKey;

	// ポートプール管理用スレッドを起動するか否か
	bool					m_bCreateThread;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_DATASOURCE_H

//
//	Copyright (c) 2006, 2007, 2008, 2009, 2011, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
