// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Server/Connection.h"
#include "Server/Parameter.h"
#include "Server/Session.h"
#include "Server/Manager.h"
#include "Server/Worker.h"
#include "Server/InstanceManager.h"
#include "Server/FakeError.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/ExceptionMessage.h"
#include "Common/ExceptionObject.h"
#include "Common/Externalizable.h"
#include "Common/Status.h"
#include "Common/Request.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"
#include "Common/IntegerArrayData.h"

#include "Os/AutoCriticalSection.h"

#include "Exception/Unexpected.h"
#include "Exception/BadArgument.h"
#include "Exception/UnknownRequest.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/ModLibraryError.h"
#include "Exception/InvalidStatementIdentifier.h"
#include "Exception/ServerNotAvailable.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/ConnectionClosed.h"
#include "Exception/UserLevel.h"

#include "Communication/Connection.h"
#include "Communication/ConnectionSlaveID.h"
#include "Communication/ServerConnection.h"
#include "Communication/Crypt.h"

#include "Opt/Optimizer.h"

#include "Checkpoint/Database.h"

#include "Schema/Database.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"

#include <new>

#define _SERVER_MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{

//
//	VARIABLE local
//	_$$::_cCriticalSection -- ConnectionID取得時の排他制御用
//
Os::CriticalSection _cCriticalSection;

//
//	VARIABLE local
//	_$$::_iNewConnectionID -- 新しいConnectionID
//
ID _iNewConnectionID = 0;

//
//	FUNCTION local
//	_$$::_getNewConnectionID() -- 新しいConnectionIDを得る
//
ID _getNewConnectionID()
{
	Os::AutoCriticalSection cAuto(_cCriticalSection);
	return ++_iNewConnectionID;
}

//
//	CLASS
//	_$$::_AutoReport -- ServerConnectionが終了したことをServerManagerに知らせる
//
class _AutoReport
{
public:
	//コンストラクタ
	_AutoReport(Connection& cConnection_) : m_cConnection(cConnection_)
	{
		m_cConnection.getInstanceManager()
			.pushServerConnection(&m_cConnection);
	}
	//デストラクタ
	~_AutoReport()
	{
		m_cConnection.getInstanceManager()
			.removeServerConnection(&m_cConnection);
		Manager& manager
			= m_cConnection.getInstanceManager().getServerManager();
		m_cConnection.terminate();	// 終了処理を行う
		manager.reportEndConnection(m_cConnection.getID());
	}

private:
	Connection& m_cConnection;
};

// コネクション関連の例外が発生した時にログに出力するか否か
ParameterBoolean _cParameterLogConnectionException(
	"Server_LogConnectionException", true);

} // namespace {

//
//	FUNCTION public
//	Server::Connection::Connection -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Server::Manager& cServerManager_
//		サーバマネージャー
//	Communication::ServerConnection* pConnection_
//		メインのコネクション
//	const ModUnicodeString& cstrHostName_
//		クライアントのホスト名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Connection::Connection(Manager& cServerManager_,
					   Communication::ServerConnection* pConnection_,
					   const ModUnicodeString& cstrHostName_)
: m_pConnection(pConnection_)
{
	m_iConnectionID = _getNewConnectionID();

	//InstanceManagerを得る
	m_pInstanceManager =  InstanceManager::create(cServerManager_,
											 cstrHostName_,
											 pConnection_->getMasterID());
	// 暗号化対応
	m_pInstanceManager->setKey(pConnection_->getKey());
}

//
//	FUNCTION public
//	Server::Connection::Connection -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Server::Connection::Pointer pInstanceManager_
//		InstanceManager
//	Communication::ServerConnection* pConnection
//		クライアントとのコネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Connection::Connection(Pointer pInstanceManager_,
					   Communication::ServerConnection* pConnection_)
: m_pConnection(pConnection_), m_pInstanceManager(pInstanceManager_)
{
	m_iConnectionID = _getNewConnectionID();
}

//
//	FUNCTION public
//	Server::Connection::~Connection -- デストラクタ
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
Connection::~Connection()
{
}

//
//	FUNCTION public static
//	Server::Connection::sendStatus -- クライアントに実行ステータスを送る
//
//	NOTES
//
//	ARGUMENTS
//	Communication::ServerConnection* pConnection_
//		クライアントとのコネクション
//	const Commun::Status& cStatus_
//		ステータス
//	const Exception::Object& cException_
//		例外(default Exception::Object())
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Connection::sendStatus(Communication::ServerConnection* pConnection_,
					   const Common::Status& cStatus_,
					   const Exception::Object& cException_)
{
	if (pConnection_)
	{
		if (cStatus_.getStatus() == Common::Status::Success
			|| cStatus_.getStatus() == Common::Status::Canceled)
		{
			//正常なので、ステータスを送る
			pConnection_->writeObject(&cStatus_);
		}
		else
		{
			//正常じゃないので、例外を送る
			const Common::ExceptionObject e(cException_);
			pConnection_->writeObject(&e);
		}

		//フラッシュする
		pConnection_->flush();
	}
}

//
//	FUNCTION public
//	Server::Connection::terminate -- 終了処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
// 	EXCEPTIONS
//
void
Connection::terminate()
{
	if (m_pConnection)
	{
		m_pConnection->close();
		delete m_pConnection;
	}

	m_pInstanceManager.release();
}

//
//	FUNCTION private
//	Server::Connection::runnable -- スレッドとして起動するメソッド
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
Connection::runnable()
{
	_AutoReport cAutoReport(*this);

	//クライアントからの要求を受付処理する
	loop();

	//OK <- EndConnectionのリクエストに対して
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::loop -- クライアントからの要求を処理する
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
Connection::loop()
{
	// クライアントにOKを送る
	sendStatus(m_pConnection, Common::Status::Success);

	while (1)
	{
		try
		{
			//ポーリングしながら待つ
			while (m_pConnection->wait(500) != true)
			{
				if (isAborted() == true)
				{
					// 終了要求が来ているのでスレッドを終了する
					// この終了要求はサーバ側から来ているものなので、
					// クライアントにはコネクションをクローズすることで通知する。

					m_pConnection->close();
					delete m_pConnection, m_pConnection = 0;
					return;
				}
			}

			// リクエストを読み込む
			ModAutoPointer<Common::Externalizable> pObject = m_pConnection->readObject();
			Common::Request* pRequest = dynamic_cast<Common::Request*>(pObject.get());

			if (pRequest == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			switch (pRequest->getCommand())
			{
			case Common::Request::BeginConnection:
				//新しいサーバコネクションを作成する
				beginConnection();
				break;
			case Common::Request::BeginWorker:
				//Workerで処理する
				beginWorker();
				break;
			case Common::Request::CancelWorker:
				//Workerを中断する
				cancelWorker();
				break;
			case Common::Request::ErasePrepareStatement:
				//最適化結果を削除する
				erasePrepareStatement();
				break;
			case Common::Request::EndConnection:
				//サーバコネクションを終了する
				return;
			case Common::Request::NoReuseConnection:
				//再利用しないコネクションを削除する
				disconnectConnection();
				break;
			case Common::Request::CheckAvailability:
				//利用可能性をチェックする
				checkAvailability();
				break;
			default:
				_SYDNEY_THROW1(Exception::UnknownRequest, pRequest->getCommand());
			}
		}
		catch (Exception::ConnectionRanOut& e)
		{
			if (_cParameterLogConnectionException.get())
				SydInfoMessage << e << ModEndl;
			//クライアントとの接続が切れたので、サーバコネクションを終了する
			throw;
		}
		catch (Exception::ConnectionClosed& e)
		{
			if (_cParameterLogConnectionException.get())
				SydInfoMessage << e << ModEndl;
			//クライアントとの接続が切れたので、サーバコネクションを終了する
			throw;
		}
		catch (Exception::UserLevel& e)
		{
			//ユーザレベルの例外が発生した
			SydInfoMessage << e << ModEndl;
			sendStatus(m_pConnection, Common::Status::Error, e);
		}
		catch (Exception::ServerNotAvailable& e)
		{
			//Servor not availableの例外が発生した
			if (Manager::isNotAvailableLogged() == false) {
				SydErrorMessage << e << ModEndl;
				Manager::setNotAvailableLogged(true);
			}
			sendStatus(m_pConnection, Common::Status::Error, e);
		}
		catch (Exception::Object& e)
		{
			//その他の例外が発生した
			SydErrorMessage << e << ModEndl;
			sendStatus(m_pConnection, Common::Status::Error, e);
		}
		catch (ModException& e)
		{
			//Modの例外が発生した
			SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
			if (e.getErrorNumber() == ModCommonErrorUnexpected)
				Manager::setAvailability(false);	//サーバ利用可能性を設定
			sendStatus(m_pConnection, Common::Status::Error,
					   _SERVER_MOD_EXCEPTION(e));
			// エラーをリセットする
			Common::Thread::resetErrorCondition();
		}
#ifndef NO_CATCH_ALL
		catch (std::bad_alloc& e)
		{
			//標準C++の例外が発生した
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			sendStatus(m_pConnection, Common::Status::Error,
					   Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		}
		catch (std::exception& e)
		{
			//bad_alloc以外の標準C++の例外が発生した
			Manager::setAvailability(false);	//サーバ利用可能性を設定
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			sendStatus(m_pConnection, Common::Status::Error,
					   Exception::Unexpected(moduleName, srcFile, __LINE__));
		}
		catch (...)
		{
			//予期せぬ例外が発生した
			Manager::setAvailability(false);	//サーバ利用可能性を設定
			SydErrorMessage << "Unexpected Exception" << ModEndl;
			sendStatus(m_pConnection, Common::Status::Error,
						Exception::Unexpected(moduleName, srcFile, __LINE__));
		}
#endif
	}
}

//
//	FUNCTION private
//	Server::Connection::beginConnection -- 新しいコネクションを起動する
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
Connection::beginConnection()
{
	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Connection::beginConnection);

	//新しいSlaveIDを得る
	int iSlaveID = Communication::ConnectionSlaveID::allocateID();

	//SlaveIDを送る
	const Common::IntegerData data(iSlaveID);
	m_pConnection->writeObject(&data);
	m_pConnection->flush();

	//新たに得る
	ModAutoPointer<Communication::ServerConnection> pNewConnection
		= new Communication::ServerConnection(Manager::getMasterID(), iSlaveID);
	pNewConnection->open();


	//新しいコネクションを得る
	ModAutoPointer<Connection> pConnection
		= new Connection(m_pInstanceManager, pNewConnection.release());


	//コネクションマネージャに登録する
	m_pInstanceManager->getServerManager().pushConnection(pConnection.release());

	//スレッドを起動する
	pConnection->create();
	
	//OK
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::beginWorker -- Workerを起動する
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
Connection::beginWorker()
{
	//スレーブIDを受け取る
	ModAutoPointer<Common::Externalizable> pObject = m_pConnection->readObject();
	Common::IntegerData* pSlaveID = dynamic_cast<Common::IntegerData*>(pObject.get());

	if (pSlaveID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Connection::beginWorker);

	int iSlaveID = pSlaveID->getValue();
	if (iSlaveID == Communication::ConnectionSlaveID::Any)
	{
		//新しいスレーブIDを確保する
		iSlaveID = Communication::ConnectionSlaveID::allocateID();
	}
	{
	//SlaveIDを送る
	const Common::IntegerData data(iSlaveID);
	m_pConnection->writeObject(&data);
	}
	//Workerのインスタンスを確保する
	Worker* pWorker = new Worker(*m_pInstanceManager, iSlaveID);
	//WorkerManagerに登録する
	m_pInstanceManager->pushWorker(pWorker);
	{
	//WorkerIDを送る
	const Common::IntegerData data(pWorker->getID());
	m_pConnection->writeObject(&data);
	}
	//スレッドを起動する
	pWorker->create();

	//OK
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::cancelWorker -- Workerに中断要求を出す
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
Connection::cancelWorker()
{
	//WorkerIDを取得する
	ModAutoPointer<Common::Externalizable> pObject = m_pConnection->readObject();
	Common::IntegerData* pWorkerID = dynamic_cast<Common::IntegerData*>(pObject.get());

	if (pWorkerID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Connection::cancelWorker);

	//指定されたWorker中断要求を行う(中断されるとは限らない...)
	m_pInstanceManager->cancelWorker(pWorkerID->getValue());

	//クライアントにOKを送る
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::erasePrepareStatement -- 最適化結果を削除する
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
Connection::erasePrepareStatement()
{
	//データベース名を取得する
	ModAutoPointer<Common::Externalizable> pObject0 = m_pConnection->readObject();
	Common::StringData* pDatabaseName = dynamic_cast<Common::StringData*>(pObject0.get());

	//最適化IDを取得する
	ModAutoPointer<Common::Externalizable> pObject1 = m_pConnection->readObject();
	Common::IntegerData* pPrepareID = dynamic_cast<Common::IntegerData*>(pObject1.get());

	if (pDatabaseName == 0 || pPrepareID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Connection::erasePrepareStatement);

	// ここにくるのはセッション間で共有するPrepareのみである
	// -> 奇数のはず
	if ((pPrepareID->getValue() % 2) == 0) {
		SydErrorMessage << "Bad prepare ID: " << pPrepareID->getValue() << ModEndl;
		_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
	}
	int iID = (pPrepareID->getValue() - 1) / 2;

	//最適化結果が存在するかチェックする
	if (m_pInstanceManager->checkPrepareID(pDatabaseName->getValue(), iID) == false)
	{
		//存在していないのでエラー
		SydErrorMessage << "Bad prepare ID: " << pPrepareID->getValue() << ModEndl;
		_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
	}

	//最適化結果を削除する
	m_pInstanceManager->popPrepareID(pDatabaseName->getValue(), iID);

	//Optimizerのも削除
	Opt::Optimizer::erasePrepareStatement(iID);

	//クライアントにOKを送る
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::disconnectConnection -- 再利用しないコネクションを切断する
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
Connection::disconnectConnection()
{
	//再利用しないコネクションのSlaveIDを得る
	ModAutoPointer<Common::Externalizable> pObject0 = m_pConnection->readObject();
	Common::IntegerArrayData* pSlaveIDArray = dynamic_cast<Common::IntegerArrayData*>(pObject0.get());

	if (pSlaveIDArray == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Connection::disconnectConnection);

	//コネクションを削除する
	const ModVector<int>& vecSlaveID = pSlaveIDArray->getValue();
	for (ModVector<int>::ConstIterator i = vecSlaveID.begin(); i != vecSlaveID.end(); ++i)
	{
		if (Communication::ServerConnection* pConnection = m_pInstanceManager->popConnection(*i)) {
			pConnection->close();
			delete pConnection;
		}
	}

	//クライアントにOKを送る
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Connection::checkAvailability -- 利用可能性をチェックする
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
Connection::checkAvailability()
{
	// チェック対象が何かを受け取る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::IntegerData* pData
		= dynamic_cast<Common::IntegerData*>(pObject0.get());
	if (pData == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	switch (pData->getValue())
	{
	case Common::Request::AvailabilityTarget::Server:
		{
			// サーバの利用可能性をチェックする
			
			Common::IntegerData data(Manager::isAvailable() ? 1 : 0);
			m_pConnection->writeObject(&data);
		}
		break;
	case Common::Request::AvailabilityTarget::Database:
		{
			// データベースの利用可能性をチェックする

			// データベースIDを受け取る
			ModAutoPointer<Common::Externalizable> pObject1
				= m_pConnection->readObject();
			Common::UnsignedIntegerData* pID
				= dynamic_cast<Common::UnsignedIntegerData*>(pObject1.get());

			Common::IntegerData data;

			Schema::ObjectID::Value	uiDatabaseID;
			if (pID == 0) {
				// Java 側からは UnsingedIntegerData を送ることができない。
				// データベース ID は Java 側からは IntegerData で送るようにしてあるので、
				// IntegerData でリトライしてみる
				Common::IntegerData*	pSignedID = dynamic_cast<Common::IntegerData*>(pObject1.get());
				if (pSignedID == 0)
					_SYDNEY_THROW0(Exception::BadArgument);
				
				uiDatabaseID = static_cast<Schema::ObjectID::Value>(pSignedID->getValue());
			} else {
				uiDatabaseID = pID->getValue();
			}

			if (uiDatabaseID == Schema::ObjectID::Invalid)
			{
				// すべてのデータベースの利用可能性をチェックする
				data.setValue(
					Checkpoint::Database::isAvailable() ? 1 : 0);
			}
			else
			{
				// 該当するデータベースの利用可能性をチェックする
				data.setValue(
					Schema::Database::isAvailable(uiDatabaseID) ? 1 : 0);
			}

			// 結果を送る
			m_pConnection->writeObject(&data);
		}
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	//クライアントにOKを送る
	sendStatus(m_pConnection, Common::Status::Success);
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
