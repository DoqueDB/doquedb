// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.cpp --
// 
// Copyright (c) 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Client";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client/DataSource.h"
#include "Client/Connection.h"
#include "Client/Parameter.h"
#include "Client/Port.h"
#include "Client/Session.h"
#include "Client/PrepareStatement.h"

#include "Common/StringData.h"
#include "Common/Request.h"
#include "Common/Parameter.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"

#include "Communication/LocalClientConnection.h"
#include "Communication/RemoteClientConnection.h"
#include "Communication/ConnectionSlaveID.h"

#include "Os/SysConf.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/ConnectionRanOut.h"
#include "Exception/Unexpected.h"
#include "Exception/NotInitialized.h"
#include "Exception/BadArgument.h"

#include "ModAutoPointer.h"
#include "ModVector.h"
#include "ModOsDriver.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

namespace
{
//
//	VARIABLE local
//	_$$::_iConnectionThreshold -- 新しいコネクションを作成する閾値
//
//	NOTES
//	新しいコネクションを作成する閾値。セッションの数が基準。デフォルト20
//
ParameterInteger
_iConnectionThreshold("Client_ConnectionThreshold", 20);

//
//	VARIABLE local
//	_$$::_iMaximumConnectionPoolCount -- ポートプールの最大値
//
//	NOTES
//	ポートプールの最大値。この値以上のポートがプールされた場合には余ったポートは削除される。
//
ParameterInteger
_iMaximumConnectionPoolCount("Client_MaximumConnectionPoolCount", 10);

//
//	VARIABLE local
//	_$$::_iCheckConnectionPoolPeriod -- ポートプールをチェックする間隔
//
//	NOTES
//	ポートプール数をチェックする間隔。
//	単位はミリ秒。ただし最小単位は500である。
//
ParameterInteger
_iCheckConnectionPoolPeriod("Client_CheckConnectionPoolPeriod", 60000);

}

//
//	FUNCTION public
//	Client::DataSource::DataSource -- コンストラクタ(InProcess用)
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
//	なし
//
DataSource::DataSource()
	: m_iConnectionArgument(0),
	  m_iPortNumber(0),
	  m_iOpenCounter(0),
	  m_eProtocolVersion(Protocol::Version1)
{
	ModOsDriver::Memory::reset(m_pInstanceCount,
							   sizeof(int)*Client::Object::Type::ElementSize);
}

//
//	FUNCTION public
//	Client::DataSource::DataSource -- コンストラクタ(RemoteServer用)
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHostName_
//		ホスト名
//	int iPortNumber_
//		ポート番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DataSource::DataSource(const ModUnicodeString& cstrHostName_,
					   int iPortNumber_)
	: m_iConnectionArgument(0),
	  m_cstrHostName(cstrHostName_),
	  m_iPortNumber(iPortNumber_),
	  m_iOpenCounter(0),
	  m_eProtocolVersion(Protocol::Version1)
{
	ModOsDriver::Memory::reset(m_pInstanceCount,
							   sizeof(int)*Client::Object::Type::ElementSize);
}

//
//	FUNCTION public
//	Client::DataSource::~DataSource -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DataSource::~DataSource()
{
	try
	{
		close();
	} catch (...) {}	// 例外は無視
}

//
//	FUNCTION public
//	Client::DataSource::open -- サーバと接続する
//
//	NOTES
//	
//	ARGUMENTS
//	Client::DataSource::Protocol::Value
//		プロトコルバージョン(default Protocol::Version1)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataSource::open(Protocol::Value eProtocolVersion_)
{
	Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);

	if (m_iOpenCounter++ == 0)
	{
		// プロトコルバージョンを設定する
		m_eProtocolVersion = eProtocolVersion_;

		ModAutoPointer<Port> pPort
			= getNewPort(Communication::ConnectionSlaveID::Any);

		pPort->open();

		//リクエストを送る
		const Common::Request request(Common::Request::BeginConnection);
		pPort->writeObject(&request);

		//クライアントのホスト名を送る
		const Common::StringData data(Os::SysConf::HostName::get());
		pPort->writeObject(&data);
		pPort->flush();

		//ステータスを得る
		pPort->readStatus();

		//クライアントコネクションを作成
		ModAutoPointer<Connection> pClientConnection
			= new Connection(*this, pPort.release());

		//配列に加える
		m_vecpClientConnection.pushBack(pClientConnection.release());

		//ポートチェックスレッドを起動する
		create();
	}
}

//
//	FUNCTION public
//	Client::DataSource::close -- サーバとの接続を切断する
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
DataSource::close()
{
	Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);
		
	if (--m_iOpenCounter == 0)
	{
		//ポートチェックスレッドを停止する
		abort();
		join();

		{
			Os::AutoCriticalSection cAuto1(m_cPortMapLatch);

			//ポートプール内のものをすべて切断する
			PortMap::Iterator k = m_mapPort.begin();
			for (; k != m_mapPort.end(); ++k)
			{
				try
				{
					(*k).second->close();
				} catch (...) {}	// 例外は無視
				(*k).second->release();
			}
			m_mapPort.clear();
		}
		{
			//すべてのコネクションを切断する
			ModVector<Connection*>::Iterator i = m_vecpClientConnection.begin();
			for (; i != m_vecpClientConnection.end(); ++i)
			{
				(*i)->close();
				(*i)->release();
			}
			m_vecpClientConnection.clear();
			m_iConnectionArgument = 0;
		}
	}
}

//
//	FUNCTION public
//	Client::DataSource::release -- メモリーを解放する
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
DataSource::release()
{
	delete this;
}

//
//	FUNCTION public
//	Client::DataSource::shutdown -- サーバを停止する
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
DataSource::shutdown()
{
	Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);

	ModAutoPointer<Port> pPort
		= getNewPort(Communication::ConnectionSlaveID::Any);

	pPort->open();

	//リクエストを送る
	const Common::Request request(Common::Request::Shutdown);
	pPort->writeObject(&request);
	pPort->flush();

	//ステータスを得る
	pPort->readStatus();

	pPort->close();
}

//
//	FUNCTION public
//	Client::DataSource::createSession -- セッションを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//
//	RETURN
//	Client::Session*
//		セッションオブジェクト
//
//	EXCEPTIONS
//
Session*
DataSource::createSession(const ModUnicodeString& cstrDatabaseName_)
{
	Port*	p = 0;

	{
		Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);

		Connection* pClientConnection = 0;

		try {
			//クライアントコネクションを得る
			pClientConnection = getClientConnection();
		} catch (Exception::NotInitialized&) {
			// close のみ実施されているので、open する
			try {
				open(m_eProtocolVersion);
				pClientConnection = getClientConnection();
			}
			catch (...) {}
		}

		if (pClientConnection == 0)
			_TRMEISTER_THROW0(Exception::ConnectionRanOut);

		//Workerを起動する
		int iWorkerID;
		try {
			p = pClientConnection->beginWorker(iWorkerID);
		} catch (Exception::ConnectionRanOut&) {
			if (getCount(Client::Object::Type::Session) != 0)
				throw;
			// サーバが再起動したかもしれないので、DataSourceを初期化する
			try {
				close();
				open(m_eProtocolVersion);
				pClientConnection = getClientConnection();
				p = pClientConnection->beginWorker(iWorkerID);
			} catch (...) {}
			if (p == 0)
				throw;
		}
	}
	
	ModAutoPointer<Port> pPort = p;

	// [<-] BeginSession
	const Common::Request request(Common::Request::BeginSession);
	pPort->writeObject(&request);

	// [<-] データベース名
	const Common::StringData data(cstrDatabaseName_);
	pPort->writeObject(&data);
	pPort->flush();

	// [->] セッション番号
	ModAutoPointer<Common::IntegerData> pSessionID = pPort->readIntegerData();

	// [->] ステータス
	pPort->readStatus();

	//セッションを確保
	Session* pSession = new Session(*this,
									cstrDatabaseName_,
									pSessionID->getValue());

	//コネクションを返す
	pushPort(pPort.release());

	//必要ならあたらしいクライアントコネクションを得る
	newClientConnect();

	return pSession;
}

//
//	FUNCTION public
//	Client::DataSource::createPrepareStatement -- 最適化結果を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	const ModUnicodeString& cstrStatement_
//
//	RETURN
//	Client::PrepareStatement*
//		最適化結果
//
//	EXCEPTIONS
//
PrepareStatement*
DataSource::createPrepareStatement(const ModUnicodeString& cstrDatabaseName_,
								   const ModUnicodeString& cstrStatement_)
{
	//クライアントコネクションを得る
	Connection* pClientConnection = getClientConnection();

	//Workerを起動する
	int iWorkerID;
	ModAutoPointer<Port> pPort = pClientConnection->beginWorker(iWorkerID);

	// [<-] BeginSession
	const Common::Request request(Common::Request::PrepareStatement);
	pPort->writeObject(&request);

	// [<-] データベース名
	const Common::StringData name(cstrDatabaseName_);
	pPort->writeObject(&name);

	// [<-] SQL文
	const Common::StringData stmt(cstrStatement_);
	pPort->writeObject(&stmt);
	pPort->flush();

	// [->] PrepareID
	ModAutoPointer<Common::IntegerData> pPrepareID = pPort->readIntegerData();

	// [->] ステータス
	pPort->readStatus();

	//最適化結果を確保
	PrepareStatement* pPrepareStatement
		= new PrepareStatement(*this, cstrDatabaseName_,
							   pPrepareID->getValue());

	//コネクションを返す
	pushPort(pPort.release());

	return pPrepareStatement;
}

//
//	FUNCTION public
//	Client::DataSource::isServerAvailable -- サーバの利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		サーバが利用可能な場合はtrue、利用不可能な場合はfalse
//
//	EXCEPTIONS
//
bool
DataSource::isServerAvailable()
{
	// クライアントコネクションを得る
	Connection* pClientConnection = getClientConnection();
	// 利用可能性を問い合わせる
	return pClientConnection->isServerAvailable();
}

//
//	FUNCTION public
//	Client::DataSource::isDatabaseAvailable -- データベースの利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	Client::DataSource::Database::ID id_
//		データベースID
//		Database::Allが指定された場合はすべてのデータベースの利用可能性を
//		チェックする
//
//	RETURN
//	bool
//		データベースが利用可能な場合はtrue、利用不可能な場合はfalse
//
//	EXCEPTIONS
//
bool
DataSource::isDatabaseAvailable(Database::ID id_)
{
	// クライアントコネクションを得る
	Connection* pClientConnection = getClientConnection();
	// 利用可能性を問い合わせる
	return pClientConnection->isDatabaseAvailable(id_);
}

//
//	FUNCTION public
//	Client::DataSource::getClientConnection -- クライアントコネクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client::Connection*
//		クライアントコネクション
//
//	EXCEPTIONS
//
Connection*
DataSource::getClientConnection()
{
	Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);

	if (m_vecpClientConnection.getSize() == 0)
		// すでにクローズされている
		_TRMEISTER_THROW0(Exception::NotInitialized);

	if (m_iConnectionArgument
		>= static_cast<int>(m_vecpClientConnection.getSize()))
		m_iConnectionArgument = 0;

	return m_vecpClientConnection[m_iConnectionArgument++];
}

//
//	FUNCTION public
//	Client::DataSource::popPort -- ポートを削除する
//
//	NOTES
//	ポートを削除する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client::Port*
//		プールされていない場合は0を返す
//
//	EXCEPTIONS
//
Port*
DataSource::popPort()
{
	Os::AutoCriticalSection cAuto(m_cPortMapLatch);

	Port* pPort = 0;

	PortMap::Iterator i = m_mapPort.begin();
	if (i != m_mapPort.end())
	{
		pPort = (*i).second;
		m_mapPort.erase(i);
	}

	return pPort;
}

//
//	FUNCTION public
//	Client::DataSource::pushPort -- ポートを追加する
//
//	NOTES
//	ポートを追加する
//
//	ARGUMENTS
//	Clinet::Port* pPort_
//		コネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataSource::pushPort(Port* pPort_)
{
	Os::AutoCriticalSection cAuto(m_cPortMapLatch);
	m_mapPort.insert(pPort_->getSlaveID(), pPort_);
}

//
//	FUNCTION public
//	Client::DataSource::getNewPort -- 新しいポートを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	Client::Port* 
//		 新しいポートのインスタンス
//
//	EXCEPTIONS
//
Client::Port*
DataSource::getNewPort(int iSlaveID_)
{
	Port* pPort;

	if (m_iPortNumber == 0)
	{
		//Local
		pPort = new Port(m_eProtocolVersion, iSlaveID_);
	}
	else
	{
		//RemoteServer
		pPort = new Port(m_cstrHostName, m_iPortNumber,
						 m_eProtocolVersion, iSlaveID_);
	}

	return pPort;
}

//
//	FUNCTION public
//	Client::DataSource::incrementCount -- インスタンス数をインクリメントする
//
//	NOTES
//
//	ARGUMENTS
//	Client::Object::Type::Value eType_
//		クラス種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DataSource::incrementCount(Client::Object::Type::Value eType_)
{
	Os::AutoCriticalSection cAuto(m_cInstanceLatch);
	m_pInstanceCount[eType_]++;
}

//
//	FUNCTION public
//	Client::DataSource::decrementCount -- インスタンス数をデクリメントする
//
//	NOTES
//
//	ARGUMENTS
//	Client::Object::Type::Value eType_
//		クラス種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
DataSource::decrementCount(Client::Object::Type::Value eType_)
{
	Os::AutoCriticalSection cAuto(m_cInstanceLatch);
	m_pInstanceCount[eType_]--;
}

//
//	FUNCTION public
//	Client::DataSource::getCount -- インスタンス数を得る
//
//	NOTES
//
//	ARGUMENTS
//	Client::Object::Type::Value eType_
//		クラス種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
int
DataSource::getCount(Client::Object::Type::Value eType_)
{
	Os::AutoCriticalSection cAuto(m_cInstanceLatch);
	return m_pInstanceCount[eType_];
}

//
//	FUNCTION public static
//	Client::DataSource::createDataSource -- データソースを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client::DataSource*
//		新しいデータソース
//
//	EXCEPTIONS
//
DataSource*
DataSource::createDataSource()
{
	return new DataSource;
}

//
//	FUNCTION public static
//	Client::DataSource::createDataSource -- データソースを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHostName_
//		ホスト名
//	int iPortNumber_
//		ポート番号
//
//	RETURN
//	Client::DataSource*
//		新しいデータソース
//
//	EXCEPTIONS
//
DataSource*
DataSource::createDataSource(const ModUnicodeString& cstrHostName_,
							 int iPortNumber_)
{
	//引数チェック
	if (cstrHostName_.getLength() == 0 || iPortNumber_ <= 0)
	{
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return new DataSource(cstrHostName_, iPortNumber_);
}

//
//	FUNCTION private
//	Client::DataSource::runnable -- ポートプールを管理するスレッド
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
DataSource::runnable()
{
	//チェック間隔の単位
	const int TIME_UNIT = 500;

	while (true)
	{
		int iCount = 0;
		int iTotalCount = _iCheckConnectionPoolPeriod.get() / TIME_UNIT;

		while (true)
		{
			ModOsDriver::Thread::sleep(TIME_UNIT);
			if (isAborted() == true)
				//終了
				return;
			iCount++;
			if (iCount >= iTotalCount)
				break;
		}

		ModVector<int> veciSlaveID;

		{
			// ポートプールの数をチェックする
			Os::AutoCriticalSection cAuto(m_cPortMapLatch);

			if (static_cast<int>(m_mapPort.getSize())
				> _iMaximumConnectionPoolCount.get())
			{
				//超えているので超えている分を削除する
				int s = m_mapPort.getSize()
					- _iMaximumConnectionPoolCount.get();
				PortMap::Iterator e = m_mapPort.begin();
				for (int i = 0; i < s; ++i)
				{
					Port* pPort = (*e).second;
					veciSlaveID.pushBack(pPort->getSlaveID());
					pPort->close();
					pPort->release();
					e++;
				}

				//マップから削除
				m_mapPort.erase(m_mapPort.begin(), e);
			}
		}

		if (veciSlaveID.getSize())
		{
			//削除したものがあるので、サーバに知らせる

			//クライアントコネクションを得る
			Connection* pClientConnection = getClientConnection();
			//最適化結果を削除する
			pClientConnection->disconnectPort(veciSlaveID);
		}
	}
}

//
//	FUNCTION private
//	Client::DataSource::newClientConnect
//		-- 新しいクライアントコネクションを確保する
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
DataSource::newClientConnect()
{
	Os::AutoCriticalSection cAuto(m_cClientConnectionLatch);

	if (getCount(Client::Object::Type::Session)
		> getCount(Client::Object::Type::Connection) *
		_iConnectionThreshold.get())
	{
		//セッション数が超えた -> 新しいコネクションをはる

		//クライアントコネクションを得る
		Connection* pClientConnection = getClientConnection();

		//新しいクライアントコネクションを得る
		Connection* pNewClientConnection = pClientConnection->beginConnection();

		//配列に加える
		m_vecpClientConnection.pushBack(pNewClientConnection);
	}
}

//
//	Copyright (c) 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
