// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.cpp -- データソース関連の関数定義
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client2/DataSource.h"
#include "Client2/Connection.h"
#include "Client2/Parameter.h"
#include "Client2/Port.h"
#include "Client2/Session.h"
#include "Client2/PrepareStatement.h"

#include "Common/StringData.h"
#include "Common/Request.h"
#include "Common/Parameter.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Communication/AuthorizeMode.h"

#include "Communication/RemoteClientConnection.h"
#include "Communication/ConnectionSlaveID.h"

#include "Os/SysConf.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/BadArgument.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/ConnectionClosed.h"
#include "Exception/NotInitialized.h"
#include "Exception/NotSupported.h"		// 暗号化対応

#include "ModAutoPointer.h"
#include "ModOsDriver.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

namespace
{

//	VARIABLE local
//	_$$::_iConnectionThreshold -- 1 つのコネクションが管理する最大セッション数
//
//	NOTES
//	新しいコネクションを作成する閾値。セッションの数が基準。デフォルト20

ParameterInteger
_iConnectionThreshold("Client2_ConnectionThreshold", 20);

//	VARIABLE local
//	_$$::_iTimeUnit -- ポートプールチェック間隔の最小単位 [msec]
//
//	NOTES

ParameterInteger
_iTimeUnit("Client2_TimeUnit", 500);

//	VARIABLE local
//	_$$::_iMaximumConnectionPoolCount -- ポートプールの最大ポート数
//
//	NOTES
//	ポートプールの最大値。この値以上のポートがプールされた場合には余ったポートは削除される。

ParameterInteger
_iMaximumConnectionPoolCount("Client2_MaximumConnectionPoolCount", 10);

//	VARIABLE local
//	_$$::_iCheckConnectionPoolPeriod -- ポートプールをチェックする間隔
//
//	NOTES
//	ポートプール数をチェックする間隔。
//	単位はミリ秒。ただし最小単位は500である。

ParameterInteger
_iCheckConnectionPoolPeriod("Client2_CheckConnectionPoolPeriod", 60000);

}

//	FUNCTION public
//	Client2::DataSource::DataSource -- コンストラクタ ( InProcess 用 )
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

DataSource::DataSource(bool bCreateThread_)
	: m_iConnectionElement(0),
	  m_iPortNumber(0),
	  m_eFamily(Family::Unspec),
	  m_bIsClosed(false),
	  m_iMasterID(0),
	  m_iAlgorithm(0),	// 暗号化対応
	  m_iAuthorization(0),
	  m_eProtocolVersion(Protocol::CurrentVersion),
	  m_bCreateThread(bCreateThread_)
{
}

//	FUNCTION public
//	Client2::DataSource::DataSource -- コンストラクタ ( RemoteServer 用 )
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrHostName_
//		接続先ホスト名
//	int						iPortNumber_
//		ポート番号
//	DataSource::Family::Value eFamily_
//		アドレスファミリー
//	bool bCreteThread_
//		ポートプール監視用スレッドを起動するか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS

DataSource::DataSource(const ModUnicodeString&	cstrHostName_,
					   int						iPortNumber_,
					   Family::Value			eFamily_,
					   bool						bCreateThread_)
	: m_iConnectionElement(0),
	  m_cstrHostName(cstrHostName_),
	  m_iPortNumber(iPortNumber_),
	  m_eFamily(eFamily_),
	  m_bIsClosed(false),
	  m_iMasterID(0),
	  m_iAlgorithm(0),	// 暗号化対応
	  m_iAuthorization(0),
	  m_eProtocolVersion(Protocol::CurrentVersion),
	  m_bCreateThread(bCreateThread_)
{
}

//	FUNCTION public
//	Client2::DataSource::~DataSource -- デストラクタ
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

DataSource::~DataSource()
{
	close();
}

//	FUNCTION public
//	Client2::DataSource::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	Client2::DataSource::Protocol::Value
//		プロトコルバージョン (default Protocol::CurrentVersion)
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::open(Protocol::Value eProtocolVersion_)
{
	if (eProtocolVersion_ & Communication::CryptMode::MaskAlgorithm)
	{
		_TRMEISTER_THROW0(Exception::NotSupported);
	}
	
	Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);

	// プロトコルバージョンを設定する
	// [注意]
	// 認証方式が指定されていない時はPassword認証を要求する
	m_eProtocolVersion = eProtocolVersion_;
	if ((m_eProtocolVersion & Communication::AuthorizeMode::MaskMode) == 0) {
		m_eProtocolVersion |= Communication::AuthorizeMode::Password;
	}

	// 新たにポートを得る
	ModAutoPointer<Port> pPort
		= getNewPort(Communication::ConnectionSlaveID::Any);
	pPort->open();

	// [<-] リクエストを送る
	const Common::Request	cRequest(Common::Request::BeginConnection);
	pPort->writeObject(&cRequest);

	// [<-] クライアントのホスト名を送る
	const Common::StringData	cHostName(Os::SysConf::HostName::get());
	pPort->writeObject(&cHostName);
	pPort->flush();

	// [->] ステータスを得る
	pPort->readStatus();

	// クライアントコネクションを作成する
	ModAutoPointer<Connection>	pClientConnection
		= new Connection(*this, pPort.get());

	// 配列に加える
	m_vecpConnectionArray.pushBack(pClientConnection.release());

	// マスター ID を保存する
	m_iMasterID = pPort->getMasterID();

	// 暗号アルゴリズムを保存する
	m_iAlgorithm = pPort->getAlgorithm();

	// 認証方式を保存する
	m_iAuthorization = pPort->getAuthorization();

	pPort.release();

	if (m_bCreateThread)
	{
		// スレッドを起動する
		create();
	}

	m_bIsClosed = false;
}

//	FUNCTION public
//	Client2::DataSource::close -- クローズする
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

void
DataSource::close()
{
	Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);

	if (m_bIsClosed) return;

	// スレッドを停止する
	if (m_bCreateThread)
	{
		try {
			abort();
			join();
		} catch (...) {
			Common::Thread::resetErrorCondition();
			// 例外は無視する
		}
	}

	{
		Os::AutoCriticalSection	cAuto2(m_cSessionMapLatch);

		// すべてのセッションをクローズする
		SessionMap::Iterator	i = m_cSessionMap.begin();
		for (; i != m_cSessionMap.end(); i++) {

			try {
				(*i).second->closeInternal();
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}
			(*i).second->release();
		}
		m_cSessionMap.clear();
	}

	{
		// すべてのコネクションを切断する
		ModVector<Connection*>::Iterator	i = m_vecpConnectionArray.begin();
		for (; i != m_vecpConnectionArray.end(); ++i) {

			try {
				(*i)->close();
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}
			(*i)->release();
		}
		m_vecpConnectionArray.clear();
		m_iConnectionElement = 0;
	}

	{
		Os::AutoCriticalSection	cAuto2(m_cPortMapLatch);

		// すべてのポートを切断する
		PortMap::Iterator	i = m_cPortMap.begin();
		for (; i != m_cPortMap.end(); i++) {

			try {
				(*i).second->close();
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}
			(*i).second->release();
		}
		m_cPortMap.clear();
	}

	m_bIsClosed = true;
}

//	FUNCTION public
//	Client2::DataSource::release -- メモリーを解放する
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

void
DataSource::release()
{
	delete this;
}

//	FUNCTION public
//	Client2::DataSource::shutdown -- サーバを停止する
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

void
DataSource::shutdown()
{
	// 新たにポートを得る
	ModAutoPointer<Port>	pPort = getNewPort(Communication::ConnectionSlaveID::Any);
	pPort->open();

	// [<-] リクエストを送る
	const Common::Request cRequest(Common::Request::Shutdown);
	pPort->writeObject(&cRequest);
	pPort->flush();

	// [->] ステータスを得る
	pPort->readStatus();

	pPort->close();
}

// FUNCTION public
//	Client2::DataSource::shutdown -- サーバを停止する
//
// NOTES
//	ユーザー認証のあるサーバーに対してはこちらを使わなくてはならない
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DataSource::
shutdown(const ModUnicodeString& cstrUserName_,
		 const ModUnicodeString& cstrPassword_)
{
	// 認証方式はPassword認証を要求する
	Protocol::Value eProtocolVersionSave = m_eProtocolVersion;
	m_eProtocolVersion |= Communication::AuthorizeMode::Password;

	// 新たにポートを得る
	ModAutoPointer<Port> pPort
		= getNewPort(Communication::ConnectionSlaveID::Any);
	pPort->open();

	// ProtocolVersionを元に戻しておく
	m_eProtocolVersion = eProtocolVersionSave;

	try
	{

		if (pPort->getAuthorization() == Communication::AuthorizeMode::None) {
			// Server don't support user management

			// [<-] リクエストを送る
			const Common::Request cRequest(Common::Request::Shutdown);
			pPort->writeObject(&cRequest);
			pPort->flush();

			// [->] ステータスを得る
			pPort->readStatus();
		}
		else
		{

			// [<-] リクエストを送る
			const Common::Request cRequest(Common::Request::Shutdown2);
			pPort->writeObject(&cRequest);
			// [<-] User Name
			const Common::StringData cUserName(cstrUserName_);
			pPort->writeObject(&cUserName);
			// [<-] Password
			const Common::StringData cPassword(cstrPassword_);
			pPort->writeObject(&cPassword);
			pPort->flush();

			// [->] ステータスを得る
			pPort->readStatus();
		}
		
	} catch (...) {
		pPort->close();
		throw;
	}
	
	pPort->close();
}

//	FUNCTION public
//	Client2::DataSource::createSession -- セッションを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//
//	RETURN
//	Client2::Session*
//		セッションオブジェクト
//
//	EXCEPTIONS

Session*
DataSource::createSession(const ModUnicodeString&	cstrDatabaseName_)
{
	Port*	p = 0;
	
	{
		// createSession は catch 節で close -> open を行うので、
		// 同時に実行できない
		// 他のメソッドもConnectionRanOutをcatchして close -> open を
		// 行うのなら、同様に m_cLatch で排他しないといけない。
		
		Os::AutoCriticalSection cAuto(m_cLatch);
	
		Connection*	pClientConnection = 0;

		try {
			// クライアントコネクションを得る
			pClientConnection = getClientConnection();
		} catch (Exception::NotInitialized&) {
			// close のみ実施されているので、open する
			try {
				Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
				open(m_eProtocolVersion);
				pClientConnection = getClientConnection();
			}
			catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}
		}

		if (pClientConnection == 0)
			_TRMEISTER_THROW0(Exception::ConnectionRanOut);

		try {

			// ワーカを起動する
			p = pClientConnection->beginWorker();

		}
		catch (Exception::ConnectionRanOut&)
		{
			// セッションが存在している場合は、再初期化できない
			if (isSessionExist() == true)
				throw;

			// サーバが再起動したかもしれないので、DataSource を初期化する
			try {
				
				{
					Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
					close();
					open(m_eProtocolVersion);
				}
				
				// クライアントコネクションを得る
				pClientConnection = getClientConnection();
				// ワーカを起動する
				p = pClientConnection->beginWorker();
				
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}

			if (p == 0)
				// 再接続を試みてもだめだった
				throw;
		}
		catch (Exception::ConnectionClosed&)
		{
			// セッションが存在している場合は、再初期化できない
			if (isSessionExist() == true)
				throw;

			// サーバが再起動したかもしれないので、DataSource を初期化する
			try {
				
				{
					Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
					close();
					open(m_eProtocolVersion);
				}
				
				// クライアントコネクションを得る
				pClientConnection = getClientConnection();
				// ワーカを起動する
				p = pClientConnection->beginWorker();
				
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}

			if (p == 0)
				// 再接続を試みてもだめだった
				throw;
		}
	}

	ModAutoPointer<Port>	pPort = p;
	int	iSessionID;

	try {

		// [<-] リクエスト
		const Common::Request	cRequest(Common::Request::BeginSession);
		pPort->writeObject(&cRequest);
		// [<-] データベース名
		const Common::StringData	cDatabaseName(cstrDatabaseName_);
		pPort->writeObject(&cDatabaseName);
		pPort->flush();

		// [->] セッション番号
		ModAutoPointer<Common::IntegerData>	pSessionID = pPort->readIntegerData();
		iSessionID = pSessionID->getValue();
		// [->] ステータス
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			pushPort(pPort.release());
		} else {
			pPort->close();
		}

		throw;
	}

	// コネクションをプールする
	pushPort(pPort.release());

	// セッションを確保
	Session*	pSession = new Session(*this,
									   cstrDatabaseName_,
									   iSessionID);
	putSession(pSession);

	// 必要ならあたらしいクライアントコネクションを得る
	newClientConnect();

	return pSession;
}

// FUNCTION public
//	Client2::DataSource::createSession -- create new session with user name and password
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString&	cstrDatabaseName_
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

Session*
DataSource::
createSession(const ModUnicodeString&	cstrDatabaseName_,
			  const ModUnicodeString&	cstrUserName_,
			  const ModUnicodeString&	cstrPassword_)
{
	if (getAuthorization() == Communication::AuthorizeMode::None) {
		// Server don't support user management
		return createSession(cstrDatabaseName_);
	}

	Port*	p = 0;

	{
		// createSession は catch 節で close -> open を行うので、
		// 同時に実行できない
		// 他のメソッドもConnectionRanOutをcatchして close -> open を
		// 行うのなら、同様に m_cLatch で排他しないといけない。
		
		Os::AutoCriticalSection cAuto(m_cLatch);

		Connection*	pClientConnection = 0;

		try {
			// クライアントコネクションを得る
			pClientConnection = getClientConnection();
		} catch (Exception::NotInitialized&) {
			// close のみ実施されているので、open する
			try {
				Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
				open(m_eProtocolVersion);
				pClientConnection = getClientConnection();
			}
			catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}
		}

		if (pClientConnection == 0)
			_TRMEISTER_THROW0(Exception::ConnectionRanOut);
		
		try {

			// start Worker thread
			p = pClientConnection->beginWorker();

		} catch (Exception::ConnectionRanOut&) {

			// セッションが存在している場合は、再初期化できない
			if (isSessionExist() == true)
				throw;
			
			// Server might have been restarted, so initialize DataSource again
			try	{
				
				{
					Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
					close();
					open(m_eProtocolVersion);
				}

				// クライアントコネクションを得る
				pClientConnection = getClientConnection();
				// ワーカを起動する
				p = pClientConnection->beginWorker();
				
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}

			if (p == 0)
				// 再接続を試みてもだめだった
				throw;
			
		} catch (Exception::ConnectionClosed&) {

			// セッションが存在している場合は、再初期化できない
			if (isSessionExist() == true)
				throw;
			
			// Server might have been restarted, so initialize DataSource again
			try	{
				
				{
					Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);
					close();
					open(m_eProtocolVersion);
				}

				// クライアントコネクションを得る
				pClientConnection = getClientConnection();
				// ワーカを起動する
				p = pClientConnection->beginWorker();
				
			} catch (...) {
				Common::Thread::resetErrorCondition();
				// 例外は無視する
			}

			if (p == 0)
				// 再接続を試みてもだめだった
				throw;
		}
	}

	ModAutoPointer<Port>	pPort = p;
	int	iSessionID;

	try {

		// [<-] Request
		const Common::Request	cRequest(Common::Request::BeginSession2);
		pPort->writeObject(&cRequest);
		// [<-] Database Name
		const Common::StringData	cDatabaseName(cstrDatabaseName_);
		pPort->writeObject(&cDatabaseName);
		// [<-] User Name
		const Common::StringData	cUserName(cstrUserName_);
		pPort->writeObject(&cUserName);
		// [<-] Password
		const Common::StringData	cPassword(cstrPassword_);
		pPort->writeObject(&cPassword);
		pPort->flush();

		// [->] Session ID
		ModAutoPointer<Common::IntegerData>	pSessionID = pPort->readIntegerData();
		iSessionID = pSessionID->getValue();
		// [->] Status
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			pushPort(pPort.release());
		} else {
			pPort->close();
		}

		throw;
	}

	// Pool the connection
	pushPort(pPort.release());

	// Create new Session object
	Session*	pSession = new Session(*this,
									   cstrDatabaseName_,
									   cstrUserName_,
									   iSessionID);
	putSession(pSession);

	// New client connection is created if needed
	newClientConnect();

	return pSession;
}

//	FUNCTION public
//	Client2::DataSource::createPrepareStatement -- 新しくプリペアステートメントを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//	const ModUnicodeString&	cstrStatement_
//		SQL 文
//
//	RETURN
//	Client2::PrepareStatement*
//		新しく作成したプリペアステートメント
//
//	EXCEPTIONS

PrepareStatement*
DataSource::createPrepareStatement(const ModUnicodeString&	cstrDatabaseName_,
								   const ModUnicodeString&	cstrStatement_)
{
	// クライアントコネクションを得る
	Connection*	pClientConnection = getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	int	iPrepareID;

	try {

		// [<-] リクエスト
		const Common::Request	cRequest(Common::Request::PrepareStatement);
		pPort->writeObject(&cRequest);
		// [<-] データベース名
		const Common::StringData	cDatabaseName(cstrDatabaseName_);
		pPort->writeObject(&cDatabaseName);
		// [<-] SQL 文
		const Common::StringData	cStatement(cstrStatement_);
		pPort->writeObject(&cStatement);
		pPort->flush();

		// [->] プリペアステートメント ID
		ModAutoPointer<Common::IntegerData>	pPrepareID = pPort->readIntegerData();
		iPrepareID = pPrepareID->getValue();

		// [->] ステータス
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			pushPort(pPort.release());
		} else {
			pPort->close();
		}

		throw;
	}

	// コネクションをプールする
	pushPort(pPort.release());

	// 新しくプリペアステートメントを作成する
	PrepareStatement*	pPrepareStatement = new PrepareStatement(*this,
																 cstrDatabaseName_,
																 iPrepareID);

	return pPrepareStatement;
}

//	FUNCTION public
//	Client2::DataSource::isServerAvailable -- サーバの利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		サーバが利用可能な場合は true 、利用不可能な場合は false
//
//	EXCEPTIONS

bool
DataSource::isServerAvailable()
{
	// クライアントコネクションを得る
	Connection*	pClientConnection = getClientConnection();
	// 利用可能性を問い合わせる
	return pClientConnection->isServerAvailable();
}

//	FUNCTION public
//	Client2::DataSource::isDatabaseAvailable -- データベースの利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Database::ID	iID_
//		データベースID
//		Database::All が指定された場合はすべてのデータベースの利用可能性をチェックする
//
//	RETURN
//	bool
//		データベースが利用可能な場合は true 、利用不可能な場合は false
//
//	EXCEPTIONS

bool
DataSource::isDatabaseAvailable(Database::ID	iID_/* = Database::All */)
{
	// クライアントコネクションを得る
	Connection*	pClientConnection = getClientConnection();
	// 利用可能性を問い合わせる
	return pClientConnection->isDatabaseAvailable(iID_);
}

//	FUNCTION public
//	Client2::DataSource::getClientConnection -- クライアントコネクションを得る
//
//	NOTES
//	コネクション管理クラスを得る。ラウンドロビン方式。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::Connection*
//		クライアントコネクション
//
//	EXCEPTIONS

Connection*
DataSource::getClientConnection()
{
	Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);

	if (m_vecpConnectionArray.getSize() == 0) {
		// すでにクローズされている
		_TRMEISTER_THROW0(Exception::NotInitialized);
	}

	if (m_iConnectionElement >= m_vecpConnectionArray.getSize()) {
		m_iConnectionElement = 0;
	}

	return m_vecpConnectionArray[m_iConnectionElement++];
}

//	FUNCTION public
//	Client2::DataSource::popPort -- ポートプールからポートを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::Port*
//		得られたポート
//		プールされていない場合は 0 を返す
//
//	EXCEPTIONS

Port*
DataSource::popPort()
{
	Port*	pPort = 0;
	Os::AutoCriticalSection	cAuto(m_cPortMapLatch);

	PortMap::Iterator	i = m_cPortMap.begin();
	if (i != m_cPortMap.end()) {

		pPort = (*i).second;
		m_cPortMap.erase(pPort->getSlaveID());
	}

	return pPort;
}

//	FUNCTION public
//	Client2::DataSource::pushPort -- ポートプールにポートを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Port*	pPort_
//		ポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::pushPort(Port*	pPort_)
{
	// 再利用する前に初期化する
	pPort_->reset();
	
	Os::AutoCriticalSection	cAuto(m_cPortMapLatch);

	m_cPortMap.insert(pPort_->getSlaveID(), pPort_);
}

//	FUNCTION public
//	Client2::DataSource::expungePort -- 廃棄したポートを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Port*	pPort_
//		ポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::expungePort(Port*	pPort_)
{
	Os::AutoCriticalSection	cAuto(m_cPortMapLatch);

	m_vecpExpungePort.pushBack(pPort_->getSlaveID());
	pPort_->close();
}

//	FUNCTION public
//	Client2::DataSource::getNewPort -- 新しいポートのインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//	int	iSlaveID_
//		スレーブID
//
//	RETURN
//	Client2::Port* 
//		 新しく作成されたポート
//
//	EXCEPTIONS

Port*
DataSource::getNewPort(int	iSlaveID_)
{
	Port*	pPort;

	if (m_iPortNumber == 0) {

		// Local
		pPort = new Port(m_eProtocolVersion, iSlaveID_);

	} else {

		// RemoteServer
		Communication::Socket::Family::Value family;
		switch (m_eFamily)
		{
		case Family::IPv4:
			family = Communication::Socket::Family::Only_v4;
			break;
		case Family::IPv6:
			family = Communication::Socket::Family::Only_v6;
			break;
		case Family::Unspec:
		default:
			family = Communication::Socket::Family::Unspec;
		}
		
		pPort = new Port(m_cstrHostName, m_iPortNumber, family,
						 m_eProtocolVersion, iSlaveID_);
	}

	return pPort;
}

//	FUNCTION public
//	Client2::DataSource::getMasterID -- マスター ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		マスター ID
//
//	EXCEPTIONS

int
DataSource::getMasterID()
{
	return m_iMasterID;
}

//	FUNCTION public
//	Client2::DataSource::getAlgorithm -- 暗号アルゴリズムを得る(暗号化対応)
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		暗号アルゴリズムを得る
//
//	EXCEPTIONS

int
DataSource::getAlgorithm()
{
	return m_iAlgorithm;
}

// FUNCTION public
//	Client2::DataSource::getAuthorization -- 認証方式を得る(ユーザー管理対応)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
DataSource::
getAuthorization()
{
	return m_iAuthorization;
}

//	FUNCTION private
//	Client2::DataSource::newClientConnect --
//		新しいクライアントコネクションを確保する
//
//	NOTES
//	セッション数が Client2_ConnectionThreshold を越えた場合に、
//	新しいクライアントコネクションを作成する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::newClientConnect()
{
	Os::AutoCriticalSection	cAuto(m_cConnectionArrayLatch);

	ModSize	size = 0;
	{
		Os::AutoCriticalSection	cAuto2(m_cSessionMapLatch);
		size = m_cSessionMap.getSize();
	}

	if (size > _iConnectionThreshold.get() * m_vecpConnectionArray.getSize()) {

		// セッション数が超えた -> 新しいクライアントコネクションを作成する

		// 今接続しているクライアントコネクションを得る
		Connection*	pClientConnection = getClientConnection();

		// 新しいクライアントコネクションを得る
		Connection*	pNewClientConnection = pClientConnection->beginConnection();

		// 配列に加える
		m_vecpConnectionArray.pushBack(pNewClientConnection);
	}
}

//	FUNCTION private
//	Client2::DataSource::runnable -- ポートプールを管理するスレッド
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

void
DataSource::runnable()
{
	while (true) {

		int	iCount = 0;
		int	iTimeUnit = _iTimeUnit.get();
		int	iTotalCount = _iCheckConnectionPoolPeriod.get() / iTimeUnit;

		while (true) {

			ModOsDriver::Thread::sleep(iTimeUnit);
			if (isAborted()) {
				//終了
				return;
			}
			iCount++;
			if (iCount >= iTotalCount) {
				break;
			}
		}

		ModVector<int>	veciSlaveID;

		{
			// ポートプールの数をチェックする
			Os::AutoCriticalSection	cAuto(m_cPortMapLatch);

			int	size = m_cPortMap.getSize() + m_vecpExpungePort.getSize();
			if (size > _iMaximumConnectionPoolCount.get()) {

				// 超えているので超えている分を削除する
				int	s = m_cPortMap.getSize() - _iMaximumConnectionPoolCount.get() - m_vecpExpungePort.getSize();
				if (s > 0) {

					PortMap::Iterator	ite = m_cPortMap.begin();
					for (int i = 0; i < s; ++i, ite++) {

						Port*	pPort = (*ite).second;
						veciSlaveID.pushBack(pPort->getSlaveID());
						pPort->close();
						pPort->release();
					}
					// マップから削除
					m_cPortMap.erase(m_cPortMap.begin(), ite);
				}
				// 廃棄分を追加する
				ModVector<int>::ConstIterator	ite = m_vecpExpungePort.begin();
				ModVector<int>::ConstIterator	endOfExpungePort = m_vecpExpungePort.end();
				for (; ite != endOfExpungePort; ite++) {
					veciSlaveID.pushBack(*ite);
				}
				m_vecpExpungePort.clear();
			}

			if (veciSlaveID.getSize()) {

				// 削除したものがあるのでサーバに知らせる

				// クライアントコネクションを得る
				Connection*	pClientConnection = getClientConnection();
				if (pClientConnection != 0) {

					try {
						pClientConnection->disconnectPort(veciSlaveID);
					} catch (...) {
						Common::Thread::resetErrorCondition();
						// 例外は無視する
					}
				}
			}
		}
	}
}

//	FUNCTION public
//	Client2::DataSource::isSessionExist -- セッションが存在しているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		存在している場合は true 、それ以外の場合は false
//
//	EXCEPTIONS

bool
DataSource::isSessionExist()
{
	Os::AutoCriticalSection	cAuto(m_cSessionMapLatch);

	return (m_cSessionMap.getSize() != 0);
}

//	FUNCTION private
//	Client2::DataSource::putSession -- セッションを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Session*	pSession_
//		マップに登録するセッションオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::putSession(Session*	pSession_)
{
	Os::AutoCriticalSection	cAuto(m_cSessionMapLatch);

	m_cSessionMap.insert(pSession_->getID(), pSession_);
}

//	FUNCTION public
//	Client2::DataSource::removeSession -- セッションを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int	iID_
//		セッション番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataSource::removeSession(int	iID_)
{
	Os::AutoCriticalSection	cAuto(m_cSessionMapLatch);

	m_cSessionMap.erase(iID_);
}

//	FUNCTION public static
//	Client2::DataSource::createDataSource -- データソースを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::DataSource*
//		新しいデータソース
//
//	EXCEPTIONS

// static
DataSource*
DataSource::createDataSource(bool bCreateThread_)
{
	return new DataSource(bCreateThread_);
}

//	FUNCTION public static
//	Client2::DataSource::createDataSource -- データソースを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrHostName_
//		ホスト名
//	int						iPortNumber_
//		ポート番号
//
//	RETURN
//	Client2::DataSource*
//		新しいデータソース
//
//	EXCEPTIONS

// static
DataSource*
DataSource::createDataSource(const ModUnicodeString&	cstrHostName_,
							 int						iPortNumber_,
							 Family::Value				eFamily_,
							 bool 						bCreateThread_)
{
	// 引数チェック
	if (cstrHostName_.getLength() == 0 || iPortNumber_ <= 0)
		_TRMEISTER_THROW0(Exception::BadArgument);

	return new DataSource(cstrHostName_, iPortNumber_,
						  eFamily_, bCreateThread_);
}

//
//	Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
