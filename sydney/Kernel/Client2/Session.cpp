// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.cpp -- セッション関連の関数定義
// 
// Copyright (c) 2006, 2007, 2015, 2016, 2023 Ricoh Company, Ltd.
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
#include "Client2/Session.h"
#include "Client2/DataSource.h"
#include "Client2/Connection.h"
#include "Client2/Port.h"
#include "Client2/ResultSet.h"
#include "Client2/PrepareStatement.h"

#include "Exception/SessionNotAvailable.h"
#include "Exception/NotSupported.h"

#include "Common/IntegerData.h"
#include "Common/DataArrayData.h"
#include "Common/Request.h"

#include "Communication/AuthorizeMode.h"

#include "Os/AutoCriticalSection.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

namespace {
	
	// プロダクトバージョンを問い合わせる機能のないバージョンの開発バージョン
	ModUnicodeString	_ProductVersionList[] = {
		ModUnicodeString("14.0"),		// 0: v14.0以下
		ModUnicodeString("15.0"),		// 1: v15.0初期バージョン
		ModUnicodeString("15.0"),		// 2: v15.0リリースバージョン
		ModUnicodeString("TR1.2"),		// 3: jdbc execute対応版
		ModUnicodeString("TR1.2.1")		// 4: ユーザー管理対応版
	};
}

//	FUNCTION public
//	Client2::Session::Session -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Client2::DataSource&	cDataSource_
//		 データソース
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//	int						iSessionID_
//		セッションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Session::Session(DataSource&				cDataSource_,
				 const ModUnicodeString&	cstrDatabaseName_,
				 int						iSessionID_)
	: Object(Object::Type::Session),
	  m_cDataSource(cDataSource_),
	  m_cstrDatabaseName(cstrDatabaseName_),
	  m_cstrUserName(),
	  m_iSessionID(iSessionID_)
{
}

//	FUNCTION public
//	Client2::Session::Session -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Client2::DataSource&	cDataSource_
//		 データソース
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//	const ModUnicodeString&	cstrUserName_
//		ユーザー名
//	int						iSessionID_
//		セッションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Session::Session(DataSource&				cDataSource_,
				 const ModUnicodeString&	cstrDatabaseName_,
				 const ModUnicodeString&	cstrUserName_,
				 int						iSessionID_)
	: Object(Object::Type::Session),
	  m_cDataSource(cDataSource_),
	  m_cstrDatabaseName(cstrDatabaseName_),
	  m_cstrUserName(cstrUserName_),
	  m_iSessionID(iSessionID_)
{
}

//	FUNCTION public
//	Client2::Session::~Session -- デストラクタ
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

Session::~Session()
{
}

//	FUNCTION public
//	Client2::Session::close -- クローズする
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
Session::close()
{
	int	iID = closeInternal();

	if (iID != 0) {

		// セッションを削除する
		getDataSource().removeSession(iID);
	}
}

//	FUNCTION public
//	Client2::Session::executeStatement -- SQL 文を実行する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&			cstrStatement_
//		SQL 文
//	const Common::DataArrayData&	cParameter_
//		パラメータ (default Common::DataArrayData())
//
//	RETURN
//	Client2::ResultSet*
//		結果セット
//
//	EXCEPTIONS

ResultSet*
Session::executeStatement(const ModUnicodeString& cstrStatement_,
						  const Common::DataArrayData& cParameter_)
{
	if (isValid() == false) _TRMEISTER_THROW0(Exception::SessionNotAvailable);

	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::ExecuteStatement);
	pPort->writeObject(&cRequest);
	// [<-] セッション ID
	const Common::IntegerData	cSessionID(m_iSessionID);
	pPort->writeObject(&cSessionID);
	// [<-] SQL 文
	const Common::StringData	cStatement(cstrStatement_);
	pPort->writeObject(&cStatement);
	// [<-] パラメータ
	pPort->writeObject(&cParameter_);
	pPort->flush();

	// 結果集合を返す
	return new ResultSet(m_cDataSource, pPort.release());
}

//	FUNCTION public
//	Client2::Session::executePrepareStatement -- プリペアした SQL 文を実行する
//
//	NOTES
//
//	ARGUMENTS
//	const Client2::PrepareStatement&	cPrepareStatement_
//		プリペアステートメント
//	const Common::DataArrayData&		cParameter_
//		パラメータ
//
//	RETURN
//	Client2::ResultSet*
//		結果セット
//
//	EXCEPTIONS

ResultSet*
Session::executePrepareStatement(const PrepareStatement&		cPrepareStatement_,
								 const Common::DataArrayData&	cParameter_)
{
	if (isValid() == false) _TRMEISTER_THROW0(Exception::SessionNotAvailable);

	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::ExecutePrepare);
	pPort->writeObject(&cRequest);
	// [<-] セッション ID
	const Common::IntegerData	cSessionID(m_iSessionID);
	pPort->writeObject(&cSessionID);
	// [<-] プリペアステートメント ID
	const Common::IntegerData	cPrepareID(cPrepareStatement_.getPrepareID());
	pPort->writeObject(&cPrepareID);
	// [<-] パラメータ
	pPort->writeObject(&cParameter_);
	pPort->flush();

	// 結果集合を返す
	return new ResultSet(m_cDataSource, pPort.release());
}

//	FUNCTION public
//	Client2::Session::createPrepareStatement -- 新しくプリペアステートメントを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrStatement_
//		SQL 文
//
//	RETURN
//	Client2::PrepareStatement*
//		新しく作成したプリペアステートメント
//
//	EXCEPTIONS

PrepareStatement*
Session::createPrepareStatement(const ModUnicodeString&	cstrStatement_)
{
	if (m_cDataSource.getMasterID() < DataSource::Protocol::Version3) {

		// v14 に接続した場合は PrepareStatement2 がない
		return m_cDataSource.createPrepareStatement(m_cstrDatabaseName, cstrStatement_);
	}

	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	int	iPrepareID;

	try {

		// [<-] リクエスト
		const Common::Request	cRequest(Common::Request::PrepareStatement2);
		pPort->writeObject(&cRequest);
		// [<-] セッション ID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
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
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// コネクションをプールする
	m_cDataSource.pushPort(pPort.release());

	return new PrepareStatement(this, iPrepareID);
}


// FUNCTION public
//	Client2::Session::createUser -- Create new user
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	int iUserID_ = Communication::User::ID::Auto
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
createUser(const ModUnicodeString& cstrUserName_,
		   const ModUnicodeString& cstrPassword_,
		   int iUserID_ /* = Communication::User::ID::Auto */)
{
	if (m_cDataSource.getAuthorization() != Communication::AuthorizeMode::Password) {

		// Don't support user management
		_TRMEISTER_THROW0(Exception::NotSupported);
	}

	// Get client connection 
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// begin new worker
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	try {

		// [<-] Request
		const Common::Request	cRequest(Common::Request::CreateUser);
		pPort->writeObject(&cRequest);
		// [<-] SessionID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
		// [<-] New user name
		const Common::StringData	cName(cstrUserName_);
		pPort->writeObject(&cName);
		// [<-] Password
		const Common::StringData	cPassword(cstrPassword_);
		pPort->writeObject(&cPassword);
		// [<-] UserID
		const Common::IntegerData	cUserID(iUserID_);
		pPort->writeObject(&cUserID);
		pPort->flush();

		// [->] Status
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// Pool the connection
	m_cDataSource.pushPort(pPort.release());
}

// FUNCTION public
//	Client2::Session::dropUser -- Drop a user
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	int iBehavior_ = Communication::User::Behavior::Ignore
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
dropUser(const ModUnicodeString& cstrUserName_,
		 int iBehavior_ /* = Communication::User::Behavior::Ignore */)
{
	if (m_cDataSource.getAuthorization() != Communication::AuthorizeMode::Password) {

		// Don't support user management
		_TRMEISTER_THROW0(Exception::NotSupported);
	}

	// Get client connection 
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// begin new worker
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	try {

		// [<-] Request
		const Common::Request	cRequest(Common::Request::DropUser);
		pPort->writeObject(&cRequest);
		// [<-] SessionID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
		// [<-] dropped user name
		const Common::StringData	cName(cstrUserName_);
		pPort->writeObject(&cName);
		// [<-] drop behavior
		const Common::IntegerData	cBehavior(iBehavior_);
		pPort->writeObject(&cBehavior);
		pPort->flush();

		// [->] Status
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// Pool the connection
	m_cDataSource.pushPort(pPort.release());
}


// FUNCTION public
//	Client2::Session::changeOwnPassword -- Change session user's password
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
changeOwnPassword(const ModUnicodeString& cstrPassword_)
{
	if (m_cDataSource.getAuthorization() != Communication::AuthorizeMode::Password) {

		// Don't support user management
		_TRMEISTER_THROW0(Exception::NotSupported);
	}

	// Get client connection 
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// begin new worker
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	try {

		// [<-] Request
		const Common::Request	cRequest(Common::Request::ChangeOwnPassword);
		pPort->writeObject(&cRequest);
		// [<-] SessionID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
		// [<-] new Password
		const Common::StringData	cPassword(cstrPassword_);
		pPort->writeObject(&cPassword);
		pPort->flush();

		// [->] Status
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// Pool the connection
	m_cDataSource.pushPort(pPort.release());
}

// FUNCTION public
//	Client2::Session::changePassword -- Change a user's password
//
// NOTES
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
Session::
changePassword(const ModUnicodeString& cstrUserName_,
			   const ModUnicodeString& cstrPassword_)
{
	if (m_cDataSource.getAuthorization() != Communication::AuthorizeMode::Password) {

		// Don't support user management
		_TRMEISTER_THROW0(Exception::NotSupported);
	}

	// Get client connection 
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// begin new worker
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	try {

		// [<-] Request
		const Common::Request	cRequest(Common::Request::ChangePassword);
		pPort->writeObject(&cRequest);
		// [<-] SessionID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
		// [<-] target user name
		const Common::StringData	cName(cstrUserName_);
		pPort->writeObject(&cName);
		// [<-] Password
		const Common::StringData	cPassword(cstrPassword_);
		pPort->writeObject(&cPassword);
		pPort->flush();

		// [->] Status
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// Pool the connection
	m_cDataSource.pushPort(pPort.release());
}

//
// FUNCTION public
// Client2::Session::queryProductVersion
//
// NOTES
//
// ARGUMENTS
// なし
//	
// RETURN
// ModUnicodeString
//		プロダクトバージョン
//
// EXCEPTIONS
//
ModUnicodeString
Session::queryProductVersion()
{
	if (m_cDataSource.getMasterID() < Communication::Protocol::Version5)
	{
		// プロダクトバージョンの問い合わせに対応していないため、
		// マスターIDによる固定文字列を返す
		
		return _ProductVersionList[m_cDataSource.getMasterID()];
	}

	ModUnicodeString version;

	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();
	// Workerを起動する
	ModAutoPointer<Port> pPort = pClientConnection->beginWorker();

	try
	{
		// [<-] リクエスト
		const Common::Request cRequest(Common::Request::QueryProductVersion);
		pPort->writeObject(&cRequest);
		pPort->flush();

		// [->] プロダクトバージョン
		ModAutoPointer<Common::StringData> pVersion	= pPort->readStringData();
		version = pVersion->getValue();

		// [->] ステータス
		pPort->readStatus();
	}
	catch (...)
	{
		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// Pool the connection
	m_cDataSource.pushPort(pPort.release());

	return version;
}

//	FUNCTION public
//	Client2::Session::getDataSource -- データソースオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::DataSource&
//		データソースオブジェクト
//
//	EXCEPTIONS

DataSource&
Session::getDataSource()
{
	return m_cDataSource;
}

//	FUNCTION public
//	Client2::Session::getDatabaseName -- データベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		データベース名
//
//	EXCEPTIONS

const ModUnicodeString&
Session::getDatabaseName()
{
	return m_cstrDatabaseName;
}

//	FUNCTION public
//	Client2::Session::getID -- セッション ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		セッション ID
//
//	EXCEPTIONS

int
Session::getID()
{
	return m_iSessionID;
}

//	FUNCTION public
//	Client2::Session::erasePrepareStatement -- プリペアステートメントを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int	iPrepareID_
//		プリペアステートメント ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Session::erasePrepareStatement(int	iPrepareID_)
{
	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

	try {

		// [<-] リクエスト
		const Common::Request	cRequest(Common::Request::ErasePrepareStatement2);
		pPort->writeObject(&cRequest);
		// [<-] セッション ID
		const Common::IntegerData	cSessionID(m_iSessionID);
		pPort->writeObject(&cSessionID);
		// [<-] プリペアステートメント ID
		const Common::IntegerData	cPrepareID(iPrepareID_);
		pPort->writeObject(&cPrepareID);
		pPort->flush();

		// [->] ステータス
		pPort->readStatus();

	} catch (...) {

		if (pPort->isReuse()) {
			m_cDataSource.pushPort(pPort.release());
		} else {
			pPort->close();
		}
		throw;
	}

	// ポートをプールする
	m_cDataSource.pushPort(pPort.release());
}

//	FUNCTION public
//	Client2::Session::closeInternal -- クローズする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		セッション ID
//
//	EXCEPTIONS

int
Session::closeInternal()
{
	Os::AutoCriticalSection	cAuto(m_cLatch);

	int	iID = getID();

	if (isValid()) {

		try {

			Connection*	pClientConnection = m_cDataSource.getClientConnection();
			if (pClientConnection != 0) {

				// ワーカを起動し、セッションを終了する

				// ワーカを起動する
				ModAutoPointer<Port>	pPort = pClientConnection->beginWorker();

				try {

					// [<-] リクエスト
					const Common::Request	cRequest(Common::Request::EndSession);
					pPort->writeObject(&cRequest);
					// [<-] セッション ID
					const Common::IntegerData	cSessionID(m_iSessionID);
					pPort->writeObject(&cSessionID);
					pPort->flush();

					// [->] ステータス
					pPort->readStatus();

				} catch (...) {

					if (pPort->isReuse()) {
						m_cDataSource.pushPort(pPort.release());
					} else {
						pPort->close();
					}
					throw;
				}

				// ポートをプールする
				m_cDataSource.pushPort(pPort.release());
			}

		} catch (...) {
			Common::Thread::resetErrorCondition();
			// 例外は無視する
		}
	}

	invalid();

	return iID;
}

//	FUNCTION public
//	Client2::Session::invalid -- セッションを利用不可にする
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
Session::invalid()
{
	m_iSessionID = 0;	// 0 は使用していない
}

//	FUNCTION public
//	Client2::Session::isValid -- セッションが利用可能かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		セッションが利用可能な場合は true 、それ以外の場合は false
//
//	EXCEPTIONS

bool
Session::isValid()
{
	return (m_iSessionID != 0);
}

//
//	Copyright (c) 2006, 2007, 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
