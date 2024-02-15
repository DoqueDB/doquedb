// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Worker.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyKernelVersion.h"

#include "Server/Worker.h"
#include "Server/Connection.h"
#include "Server/FakeError.h"
#include "Server/InstanceManager.h"
#include "Server/Manager.h"
#ifndef SYD_COVERAGE
#include "Server/Parameter.h"
#endif
#include "Server/Session.h"
#include "Server/SQLDispatchEntry.h"
#include "Server/Transaction.h"
#include "Server/UserList.h"

#include "Admin/Manager.h"

#include "Schema/Database.h"
#include "Schema/Manager.h"

#include "Checkpoint/Daemon.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/BinaryData.h"
#include "Common/ErrorLevel.h"
#include "Common/ExceptionMessage.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/OutputArchive.h"
#include "Common/Request.h"
#include "Common/StringData.h"
#include "Common/UnsignedInteger64Data.h"

#include "Communication/Protocol.h"
#include "Communication/ServerConnection.h"
#include "Communication/User.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"
#include "Exception/ConnectionClosed.h"
#include "Exception/ConnectionNotExist.h"
#include "Exception/ClientNotExist.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/DatabaseChanged.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/InvalidStatementIdentifier.h"
#include "Exception/LogFileCorrupted.h"
#include "Exception/LogItemNotFound.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/ModLibraryError.h"
#include "Exception/NotSupported.h"
#include "Exception/OfflineDatabase.h"
#include "Exception/PrivilegeNotAllowed.h"
#include "Exception/ReadOnlyDatabase.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Exception/ServerNotAvailable.h"
#include "Exception/SessionNotAvailable.h"
#include "Exception/SessionNotExist.h"
#include "Exception/SessionBusy.h"
#include "Exception/CanceledBySuperUser.h"
#include "Exception/SQLSyntaxError.h"
#include "Exception/Unexpected.h"
#include "Exception/UnknownRequest.h"
#include "Exception/UserLevel.h"

#include "Execution/Executor.h"
#include "Execution/Program.h"

#include "Opt/Explain.h"
#include "Opt/Optimizer.h"
#include "Opt/Planner.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"
#include "Os/Timer.h"

#include "Statement/CheckpointStatement.h"
#include "Statement/ExplainStatement.h"
#include "Statement/Object.h"
#include "Statement/SQLWrapper.h"
#include "Statement/SQLParser.h"
#include "Statement/StartExplainStatement.h"
#include "Statement/SyncStatement.h"
#include "Statement/DisconnectStatement.h"
#include "Statement/DeclareStatement.h"
#include "Statement/VariableName.h"
#include "Statement/Type.h"

#include "Trans/AutoLatch.h"
#include "Trans/Branch.h"
#include "Trans/LogData.h"

#include "ModAutoPointer.h"
#include "ModMemory.h"
#include "ModOsDriver.h"
#include "ModTime.h"
#include "ModException.h"

#include <new>

#define _SERVER_MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{

//
//	VARIABLE local
//	_$$::_cCriticalSection -- WorkerID取得時の排他制御用
//
Os::CriticalSection _cCriticalSection;

//
//	VARIABLE local
//	_$$::_cSyncLatch -- sync実行時の排他制御用
Os::CriticalSection _cSyncLatch;

//
//	VARIABLE local
//	_$$::_iNewWorkerID -- 新しいWorkerID
//
ID _iNewWorkerID = 0;

//
//	FUNCTION local
//	_$$::_getNewWorkerID() -- 新しいWorkerIDを得る
//
ID _getNewWorkerID()
{
	Os::AutoCriticalSection cAuto(_cCriticalSection);
	return ++_iNewWorkerID;
}

//
//	CLASS
//	_$$::_AutoReport -- Workerが終了したことをInstanceManagerに知らせる
//
class _AutoReport
{
public:
	//コンストラクタ
	_AutoReport(Worker& cWorker_) : m_cWorker(cWorker_) {}
	//デストラクタ
	~_AutoReport()
	{
		m_cWorker.reportEndWorker();
	}

private:
	Worker& m_cWorker;
};

class _AutoLockMap
{
public:
	_AutoLockMap() { InstanceManager::lockMap(); }
	~_AutoLockMap() {InstanceManager::unlockMap(); }
};

#ifndef SYD_COVERAGE

//
//	VARIABLE local
//	_$$::_cParameterPrintSQLStatement -- SQL文の出力先を指定するパラメータ
//
//	NOTES
//
ParameterMessage _cParameterPrintSQLStatement("Server_PrintSQLStatement");

//
//	VARIABLE local
//	_$$::_cParameterPrintParameter -- パラメータの出力先を指定するパラメータ
//
//	NOTES
//
ParameterMessage _cParameterPrintParameter("Server_PrintParameter");

//
//	VARIABLE local
//	_$$::_cParameterPrintSessionID -- セッションIDの出力先を指定するパラメータ
//
//	NOTES
//
ParameterMessage _cParameterPrintSessionID("Server_PrintSessionID");

//
// VARIABLE local
// _$$::_cParameterPrintTime -- 実行時間の出力先を指定するパラメータ
//
// NOTES
//
ParameterMessage _cParameterPrintTime("Server_PrintTime");

//
//	VARIABLE local
//	_$$::_cParameterPrintSlowQuery -- 遅い要求の出力先を指定するパラメータ
//
ParameterMessage _cParameterPrintSlowQuery("Server_PrintSlowQuery");

//
//	VARIABLE local
//	_$$::_cParameterSlowQueryTime -- 遅い要求かどうか判定する時間(ms)
//
ParameterInteger _cParameterSlowQueryTime("Server_SlowQueryTime", 10*1000);

//	CLASS
//	_$$::_AutoPrintTime -- 実行時間を自動的に出力する
//
//	NOTES

class _AutoPrintTime
{
public:
	// コンストラクター
	_AutoPrintTime(const char* name)
		: _name(name)
	{
		if (_cParameterPrintTime.isOutput())
			_start = ModTime::getCurrentTime();
	}

	// デストラクター
	~_AutoPrintTime()
	{
		if (_start.getTime() != 0)
		{
			ModTimeSpan interval(ModTime::getCurrentTime() - _start);

			_SYDNEY_MESSAGE(_cParameterPrintTime.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
				<< _name << " Time: "
				<< ModCharString().format(
					"%02d:%02d:%02d.%03d",
					interval.getTotalHours(), interval.getMinutes(),
					interval.getSeconds(), interval.getMilliSeconds())
				<< ModEndl;
		}
	}

private:
	// 実行時間を出力する処理の名前
	const char*					_name;
	// 処理の開始時刻
	ModTime						_start;
};

//	CLASS
//	_$$::_CheckPrintSlowQuery -- 遅い要求を出力するクラス
//
//	NOTES

class _PrintSlowQuery
{
public:
	// コンストラクター
	_PrintSlowQuery() : m_bOutput(false)
	{
		if (_cParameterPrintSlowQuery.isOutput())
		{
			m_bOutput = true;
			m_cTimer.start();
		}
	}

	// デストラクター
	~_PrintSlowQuery() {}

	// 必要なら出力する
	bool output(const Trans::Transaction& cTransaction_,
				const ModUnicodeString& cDatabaseName_,
				const ModUnicodeString* pSQL_)
	{
		bool r = false;
		
		if (m_bOutput && pSQL_)
		{
			m_cTimer.end();	// 計測終わり
			
 			if (m_cTimer.get() >
				static_cast<unsigned int>(_cParameterSlowQueryTime.get()))
			{
				// 条件を満たしているので出力する
				r = true;

				_SYDNEY_MESSAGE(
					_cParameterPrintSlowQuery.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
						<< "[Slow Query] query-time: "
						<< timeMessage(m_cTimer.get())
						<< ", lock-time: "
						<< timeMessage(cTransaction_.getLockTimer().get())
						<< ", lock-count: "
						<< cTransaction_.getLockCount()
						<< ", ref-pages: "
						<< cTransaction_.getPageReferenceCount()
						<< ", read-pages: "
						<< cTransaction_.getPageReadCount()
						<< ", send-rows: "
						<< cTransaction_.getSendRowCount()
						<< ModEndl;

				_SYDNEY_MESSAGE(
					_cParameterPrintSlowQuery.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
						<< "[Slow Query] ["
						<< cDatabaseName_
						<< "]: "
						<< *pSQL_ << ModEndl;
			}
		}
		return r;
	}

private:
	// 秒を表示する
	ModCharString timeMessage(int msec_) const
		{
			return ModCharString().format("%d.%03d (s)",
										  msec_ / 1000, msec_ % 1000);
		}
		
	// 処理の開始時刻
	Os::Timer			m_cTimer;
	// 出力するか否か
	bool				m_bOutput;
};

#endif

}	// namespace {

//
//	FUNCTION public
//	Server::Worker::Worker -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Server::InstanceManager& cInstanceManager_
//		InstanceManager
//	int iConnectionSlaveID_
//		コネクションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Worker::Worker(InstanceManager& cInstanceManager_,
			   int iConnectionSlaveID_)
	: m_cInstanceManager(cInstanceManager_),
	  m_iConnectionSlaveID(iConnectionSlaveID_),
	  m_pConnection(0)
#ifdef OBSOLETE
	  m_bExecution(false)
#endif
{
	m_iWorkerID = _getNewWorkerID();
}

//
//	FUNCTION public
//	Server::Worker::~Worker -- デストラクタ
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
Worker::~Worker()
{
}

//
//	FUNCTION public
//	Server::Worker::reportEndWorker -- Workerの終了を通知する
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
Worker::reportEndWorker()
{
	m_cInstanceManager.reportEndWorker(getID());
}

//
//	FUNCTION public
//	Server::Worker::stop -- 実行を中断する
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
Worker::stop()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (m_pConnection.get()) m_pConnection->cancel();
}


//
//	FUNCTION public
//	Server::Worker::trySyncLock -- syncLockを取得する
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
bool
Worker::trylockSync()
{
	return m_cSyncSection.trylock();
}

//	FUNCTION public
//	Server::Worker::unlockSync -- syncLockを解除する
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
Worker::unlockSync()
{
	return m_cSyncSection.unlock();
}



//
//	FUNCTION public static
//	Server::Worker::isOperationApplicable --
//		データベースに対して実行可能な操作であるか確認する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTransaction_
//			実行可能な操作か確認するトランザクションのトランザクション記述子
//		const Schema::Database& cDatabase_
//			操作対象のデータベースのスキーマオブジェクトを表すクラス
//		int iStatementType_
//			実行可能か確認する操作を行う SQL 文のタイプ
//
//	RETURN
//		操作対象であるデータベースを表すクラス
//
//	EXCEPTIONS
//		Exception::ReadOnlyTransaction
//			READ ONLY トランザクションで実行不可能な操作を実行しようとした
//		Exception::ReadOnlyDatabase
//			READ ONLY データベースに対して実行不可能な操作を実行しようとした
//		Exception::OfflineDatabase
//			オフラインなデータベースに対して実行不可能な操作を実行しようとした

// static
void
Worker::isOperationApplicable(Trans::Transaction& cTransaction_,
							  const Schema::Database& cDatabase_,
							  int iStatementType_)
{
	const SQLDispatchEntry& entry = SQLDispatchTable::getEntry(iStatementType_);

	if (cTransaction_.getCategory() == Trans::Transaction::Category::ReadOnly &&
		entry.isExecutableInsideReadOnlyTransaction() == Boolean::False)

		// READ ONLY トランザクションで実行不可能な操作が要求された

		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);

	if (cDatabase_.isReadOnly() &&
		entry.isExecutableOnReadOnlyDatabase() == Boolean::False)

		// READ ONLY データベースに対して実行不可能な操作が要求された

		_SYDNEY_THROW1(Exception::ReadOnlyDatabase, cDatabase_.getName());

	if (cDatabase_.getMasterURL().getLength() != 0 &&
		entry.isExecutableOnSlaveDatabase() == Boolean::False)

		// スレーブデータベースに対して実行不可能な操作が要求された

		_SYDNEY_THROW1(Exception::ReadOnlyDatabase, cDatabase_.getName());

	if (!cDatabase_.isOnline() &&
		entry.isExecutableOnOfflineDatabase() == Boolean::False)

		// オフラインデータベースに対して実行不可能な操作が要求された

		_SYDNEY_THROW1(Exception::OfflineDatabase, cDatabase_.getName());
}

//
//	FUNCTION public static
//	Server::Worker::isOperationApplicable --
//		データベースに対して実行可能な操作であるか確認する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	cTransaction_
//			実行可能な操作か確認するトランザクションのトランザクション記述子
//		const Schema::Database& cDatabase_
//			操作対象のデータベースのスキーマオブジェクトを表すクラス
//		const Statement::Object* pStatement_
//			実行可能か確認する操作を行う SQL 文
//		Session* pSession_
//			操作中のセッションオブジェクト
//
//	RETURN
//		操作対象であるデータベースを表すクラス
//
//	EXCEPTIONS
//		Exception::ReadOnlyTransaction
//			READ ONLY トランザクションで実行不可能な操作を実行しようとした
//		Exception::ReadOnlyDatabase
//			READ ONLY データベースに対して実行不可能な操作を実行しようとした
//		Exception::OfflineDatabase
//			オフラインなデータベースに対して実行不可能な操作を実行しようとした
//
void
Worker::isOperationApplicable(Trans::Transaction& cTransaction_,
							  const Schema::Database& cDatabase_,
							  const Statement::Object* pStatement_,
							  Session* pSession_)
{
	_SYDNEY_ASSERT(pStatement_);

	// At first, check with rough category
	isOperationApplicable(cTransaction_, cDatabase_, pStatement_->getType());

	if (pSession_ && pSession_->getUserName().getLength() > 0) {
		// Then, check privilege
		const SQLDispatchEntry& cEntry = SQLDispatchTable::getEntry(pStatement_->getType());
		// If the operation is allowed only to the superuser, check it
		if ((cEntry.m_ePrivilegeCategory == Common::Privilege::Category::SuperUser)
			||(cDatabase_.isSuperUserMode())) {
			if (!pSession_->isSuperUser()) {
				// no permission
				_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
			}
		} else {
			// When privilege is not obtained, obtain it
			if (!pSession_->isPrivilegeInitialized()) {
				pSession_->initializePrivilege(cTransaction_, cDatabase_);
			}
			// check the value
			if (!pSession_->checkPrivilege(cEntry.m_ePrivilegeCategory,
										   cEntry.m_ePrivilegeValue)) {
				// no permission
				_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
			}
		}
	}
}
	
//
//	FUNCTION private
//	Server::Worker::runnable -- スレッドと起動されるメイン関数
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
Worker::runnable()
{
	//Workerが終了したことを知らせるために
	_AutoReport cAutoReport(*this);

	try
	{
#ifndef SYD_COVERAGE
		_AutoPrintTime tmsg("Worker");
#endif
		//コネクションを得る
		//	この段階ではまだcancelは来ないので、排他する必要はない
		m_pConnection = m_cInstanceManager.popConnection(m_iConnectionSlaveID);
		if (m_pConnection.get() == 0)
		{
			//プールされていないので、新たに得る
			ModAutoPointer<Communication::ServerConnection> pNewConnection
				= new Communication::ServerConnection(Manager::getMasterID(),
													  m_iConnectionSlaveID);
			pNewConnection->open();
			{
				Os::AutoCriticalSection cAuto(m_cCriticalSection);
				m_pConnection = pNewConnection;
			}
			// 共通鍵設定(暗号化対応)
			m_pConnection->setKey(m_cInstanceManager.getKey());
		}
		else
		{
			Os::AutoCriticalSection cAuto(m_pConnection->getLockObject());
			//プールされているものを使用するので、同期を取る
			m_pConnection->sync();
		}

		//リクエストを読む
		ModAutoPointer<Common::Externalizable> pObject
			= m_pConnection->readObject();
		Common::Request* pRequest
			= dynamic_cast<Common::Request*>(pObject.get());
		if (pRequest == 0)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		switch (pRequest->getCommand())
		{
		case Common::Request::BeginSession:
			//セッションを開始する
			beginSession();
			break;
		case Common::Request::BeginSession2:
			//セッションを開始する
			beginSession2();
			break;
		case Common::Request::EndSession:
			//セッションを終了する
			endSession();
			break;
		case Common::Request::EndSession2:
			//セッションを終了する
			endSession2();
			break;
		case Common::Request::ExecuteStatement:
			//SQL文を実行する
			executeStatement();
			break;
		case Common::Request::PrepareStatement:
			//SQL文をコンパイルし、結果を保持する
			prepareStatement();
			break;
		case Common::Request::PrepareStatement2:
			//SQL文をコンパイルし、結果を保持する
			prepareStatement2();
			break;
		case Common::Request::ExecutePrepare:
			//コンパイルしたSQL文を実行する
			executePrepare();
			break;
		case Common::Request::ErasePrepareStatement2:
			//最適化結果を削除する
			erasePrepareStatement();
			break;
		case Common::Request::CreateUser:
			// create new user
			createUser();
			break;
		case Common::Request::DropUser:
			// drop a user
			dropUser();
			break;
		case Common::Request::ChangeOwnPassword:
			// change own password
			changeOwnPassword();
			break;
		case Common::Request::ChangePassword:
			// change password for a user
			changePassword();
			break;
		case Common::Request::CheckReplication:
			// レプリケーションを確認する
			checkReplication();
			break;
		case Common::Request::TransferLogicalLog:
			// 論理ログを転送する
			transferLogicalLog();
			break;
		case Common::Request::StopTransferLogicalLog:
			// 論理ログの転送を停止する
			stopTransferLogicalLog();
			break;
		case Common::Request::StartReplication:
			// レプリケーションを開始する
			startReplication();
			break;
		case Common::Request::QueryProductVersion:
			// query product version
			queryProductVersion();
			break;
		default:
			//エラー
			_SYDNEY_THROW1(Exception::UnknownRequest, pRequest->getCommand());
		}
	}
	catch (Exception::Cancel& e)
	{
		if(Schema::Database::isSuperUserMode(
			   m_cInstanceManager.getDatabaseName(m_iSessionID)))
		{
			// スーパーユーザモードの場合は例外を別にする
			Exception::CanceledBySuperUser e0(moduleName, srcFile, __LINE__);
			SydInfoMessage << e0 << ModEndl;
			sendStatus(Common::Status::Error, e0);
			throw e0;
		}
		// キャンセルは返すステータスが違う
		SydInfoMessage << e << ModEndl;
		sendStatus(Common::Status::Canceled, e);
		throw;
	}
	catch (Exception::ConnectionRanOut& e)
	{
		SydErrorMessage << e << ModEndl;
		//クライアントとの接続が切れたので、エラーは送れない
		if (m_pConnection.get())
		{
			m_pConnection->close();
			m_pConnection = 0;
		}
		throw;
	}
	catch (Exception::ConnectionClosed& e)
	{
		SydInfoMessage << e << ModEndl;
		//クライアントとの接続が切れたので、エラーは送れない
		if (m_pConnection.get())
		{
			m_pConnection->close();
			m_pConnection = 0;
		}
		throw;
	}
	catch (Exception::UserLevel& e)
	{
		//ユーザレベルの例外
		SydInfoMessage << e << ModEndl;
		sendStatus(Common::Status::Error, e);
		throw;
	}
	catch (Exception::ServerNotAvailable& e)
	{
		//Server not availableの例外
		if (Manager::isNotAvailableLogged() == false) {
			SydErrorMessage << e << ModEndl;
			Manager::setNotAvailableLogged(true);
		}
		sendStatus(Common::Status::Error, e);
		throw;
	}
	catch (Exception::Object& e)
	{
		//その他の例外
		SydErrorMessage << e << ModEndl;
		sendStatus(Common::Status::Error, e);
		throw;
	}
#ifndef NO_CATCH_ALL
	catch (ModException& e)
	{
		//Modの例外が発生した
		SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
		if (e.getErrorNumber() == ModCommonErrorUnexpected)
			Manager::setAvailability(false);	//サーバ利用可能性を設定
		sendStatus(Common::Status::Error, _SERVER_MOD_EXCEPTION(e));
		Common::Thread::resetErrorCondition();
		_SYDNEY_THROW1(Exception::ModLibraryError, e);
	}
	catch (std::bad_alloc& e)
	{
		//標準C++の例外が発生した
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;
		sendStatus(Common::Status::Error,
				   Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
		throw;
	}
	catch (std::exception& e)
	{
		//bad_alloc以外の標準C++の例外が発生した
		Manager::setAvailability(false);	//サーバ利用性を設定する
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;
		sendStatus(Common::Status::Error,
				   Exception::Unexpected(moduleName, srcFile, __LINE__));
		throw;
	}
	catch (...)
	{
		//予期せぬ例外が発生した
		Manager::setAvailability(false);	//サーバ利用性を設定する
		SydErrorMessage << "Unexpected Exception" << ModEndl;
		sendStatus(Common::Status::Error,
				   Exception::Unexpected(moduleName, srcFile, __LINE__));
		_SYDNEY_THROW0(Exception::Unexpected);
	}
#endif
	//OK
	sendStatus(Common::Status::Success);
}

//
//	FUNCTION private
//	Server::Worker::beginSession -- セッションを開始する
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
Worker::beginSession()
{
	try
	{
		//クライアントからデータベース名を受け取る
		ModAutoPointer<Common::Externalizable> pObject
			= m_pConnection->readObject();
		Common::StringData* pDatabaseName
			= dynamic_cast<Common::StringData*>(pObject.get());
		if (pDatabaseName == 0)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		;_SERVER_FAKE_ERROR(Worker::beginSession);

		if (Manager::getUserList()) {
			// If user authentication is required, this method is not supported
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// チェックポイント処理を実行不可にする
		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::Executor);
				
		//セッションオブジェクトを作成する
		ModAutoPointer<Session> pSession = new Session;

		//データベース名を設定する
		pSession->setDatabaseName(pDatabaseName->getValue());

		// スキーマに使用開始をレポートする
		Schema::Database::reserve(pSession->getID());

		//セッションを登録する
		m_cInstanceManager.pushSession(pSession.release());

		//セッションIDをクライアントに送る
		Common::IntegerData data(pSession->getID());
		m_pConnection->writeObject(&data);

#ifndef SYD_COVERAGE
		//SessionID is required to be output?
		if (_cParameterPrintSessionID.isOutput())
		{
			_SYDNEY_MESSAGE(
				_cParameterPrintSessionID.getParameterName(),
				Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "["
					<< m_cInstanceManager.getDatabaseName(pSession->getID())
					<< "] "
					<< "SessionID: " << pSession->getID() << " start." << ModEndl;
		}
#endif
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Server::Worker::beginSession2 -- Start session
//
//	NOTES
//		With user management
//
//	ARGUMENTS
//	Nothing
//
//	RETURN
//	Nothing
//
//	EXCEPTIONS
//
void
Worker::beginSession2()
{
	try
	{
		//Receive database name from client
		ModAutoPointer<Common::Externalizable> pObject
			= m_pConnection->readObject();
		Common::StringData* pDatabaseName
			= dynamic_cast<Common::StringData*>(pObject.get());
		// Receive user name
		ModAutoPointer<Common::Externalizable> pObject2
			= m_pConnection->readObject();
		Common::StringData* pUserName
			= dynamic_cast<Common::StringData*>(pObject2.get());
		// Receive password
		ModUnicodeString cstrPassword;
		recvPassword(cstrPassword);

		if (pDatabaseName == 0 || pUserName == 0)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		// check the password
		UserList::Entry::Pointer pEntry =
			Manager::verifyPassword(pUserName->getValue(), cstrPassword);

		;_SERVER_FAKE_ERROR(Worker::beginSession);

		// Block checkpoint thread
		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::Executor);
				
		// create new session object
		ModAutoPointer<Session> pSession = new Session;

		// set database name
		pSession->setDatabaseName(pDatabaseName->getValue());

		if (pEntry.get()) {
			// set user name
			pSession->setUserName(pUserName->getValue());
			// set user ID
			pSession->setUserID(pEntry->getID());
			// set superuser or not
			pSession->setIsSuperUser(pEntry->isSuperUser());
		}

		// Tell Schema module with session ID
		Schema::Database::reserve(pSession->getID());

		// Register session object
		m_cInstanceManager.pushSession(pSession.release());

		// Send the session ID to client
		Common::IntegerData data(pSession->getID());
		m_pConnection->writeObject(&data);

#ifndef SYD_COVERAGE
		//SessionID is required to be output?
		if (_cParameterPrintSessionID.isOutput())
		{
			_SYDNEY_MESSAGE(
				_cParameterPrintSessionID.getParameterName(),
				Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "["
					<< m_cInstanceManager.getUserName(pSession->getID())
					<< "@"
					<< m_cInstanceManager.getDatabaseName(pSession->getID())
					<< "] "
					<< "SessionID: " << pSession->getID() << " start." << ModEndl;
		}
#endif
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Server::Worker::endSession -- セッションを終了する
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
Worker::endSession()
{
	//クライアントからSessionIDを受け取る
	ID iSessionID = recvSessionID();

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	disconnectSession(m_cInstanceManager, iSessionID);
}

//
//	FUNCTION private
//	Server::Worker::endSession2 -- End a session
//
//	NOTES
//		with user management
//
//	ARGUMENTS
//	Nothing
//
//	RETURN
//	Nothing
//
//	EXCEPTIONS
//
void
Worker::endSession2()
{
	// for now, endSession2 is not used
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION private
//	Server::Worker::executeStatement -- SQL文を実行する
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
Worker::executeStatement()
{
	//SessionIDを得る
	ID iSessionID = recvSessionID();

	//SQL文を得る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::StringData* pSQL
		= dynamic_cast<Common::StringData*>(pObject0.get());

	//パラメータを得る
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::DataArrayData* pParameter
		= dynamic_cast<Common::DataArrayData*>(pObject1.get());

	//エラーチェック
	if (pSQL == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	try
	{
		;_SERVER_FAKE_ERROR(Worker::executeStatement);

		//SQL文をパースする
		Statement::SQLParser cSQLParser;

		//SQL文を与える
		cSQLParser.setText(pSQL->getValue());

		//パース結果を1文づつ取り出す
		Statement::Object* pObject = 0;

		bool bFirst = true;
		Common::Status cHasMoreData(Common::Status::HasMoreData);

		while (cSQLParser.parse(pObject) == Statement::SQLParser::PARSE_ACCEPT)
		{
			if (pObject != 0)
			{
				if (!bFirst && m_pConnection->getMasterID() >= Communication::Protocol::Version4) {
					// for jdbc.Statement.getMoreResults
					m_pConnection->writeObject(&cHasMoreData);
				}
				bFirst = false;

				ModAutoPointer<Statement::SQLWrapper> pSQLWrapper
					= dynamic_cast<Statement::SQLWrapper*>(pObject);

#ifndef SYD_COVERAGE
				//SessionIDを出力するか
				if (_cParameterPrintSessionID.isOutput())
				{
					_SYDNEY_MESSAGE(
						_cParameterPrintSessionID.getParameterName(),
						Common::MessageStreamBuffer::LEVEL_DEBUG)
							<< "["
							<< m_cInstanceManager.getDatabaseName(iSessionID)
							<< "] "
							<< "SessionID: " << iSessionID << ModEndl;
				}

				const ModUnicodeString* pSQL = pSQLWrapper->getSQLString();
				
				//SQL文を出力するか
				if (_cParameterPrintSQLStatement.isOutput())
				{
					if (pSQL)
					{
						_SYDNEY_MESSAGE(
							_cParameterPrintSQLStatement.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "SQL: " << *pSQL << ModEndl;
					}
					else
					{
						_SYDNEY_MESSAGE(
							_cParameterPrintSQLStatement.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "SQL: " << "(null)" << ModEndl;
					}
				}

				Common::DataArrayData cParameter;
				Common::DataArrayData* pCurrentParameter = 0;
				if (pParameter)
				{
					int size = pParameter->getCount();
					int i = pSQLWrapper->getPlaceHolderLower();
					for (; i < pSQLWrapper->getPlaceHolderUpper() &&
							 i < size; ++i)
					{
						cParameter.pushBack(
							pParameter->getElement(i));
					}
					if (cParameter.getCount() > 0)
						pCurrentParameter = &cParameter;
				}
				
				//パラメータを出力するか
				if (_cParameterPrintParameter.isOutput())
				{
					_SYDNEY_MESSAGE(
						_cParameterPrintParameter.getParameterName(),
						Common::MessageStreamBuffer::LEVEL_DEBUG)
							<< "Parameter: " << cParameter << ModEndl;
				}
#endif

				//SQL文本体
				Statement::Object* pStatement = pSQLWrapper->getObject();

				// execute sql statement
				executeStatement(iSessionID, pStatement, pParameter,
								 pSQL, pCurrentParameter);
			}
		}
	}
	catch (Exception::SessionNotExist&)
	{
		// セッションがない -> データベース名は出力できない
		SydErrorMessage	<< "SessionID=" << iSessionID << ModEndl;
		throw;
	}
	catch (Exception::UserLevel&)
	{
		// ユーザレベルの例外
		SydInfoMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << *pSQL << ModEndl;
		throw;
	}
	catch (Exception::ServerNotAvailable&)
	{
		// Server not availableの例外
		if (Manager::isNotAvailableLogged() == false) {
			SydErrorMessage
				<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
				<< "SQL=" << *pSQL << ModEndl;
		}
		throw;
	}
	catch (Exception::Object&)
	{
		// その他の例外
		SydErrorMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << *pSQL << ModEndl;
		throw;
	}
#ifndef NO_CATCH_ALL
	catch (ModException&)
	{
		//Modのエラー
		SydErrorMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << *pSQL << ModEndl;
		throw;
	}
	catch (std::exception&)
	{
		// 標準C++の例外
		SydErrorMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << *pSQL << ModEndl;
		throw;
	}
	catch (...)
	{
		//その他
		SydErrorMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << *pSQL << ModEndl;
		throw;
	}
#endif
}

// FUNCTION private
//	Server::Worker::executeStatement -- execute statement
//
// NOTES
//
// ARGUMENTS
//	ID iSessionID_
//	const Statement::Object* pStatement_
//	Common::DataArrayData* pParameter_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
executeStatement(ID iSessionID_, Statement::Object* pStatement_,
				 Common::DataArrayData* pParameter_,
				 const ModUnicodeString* pSQL_,
				 Common::DataArrayData* pCurrentParameter_)
{
	// SQL文のタイプによってスイッチ

	switch (pStatement_->getType()) {
	case Statement::ObjectType::StartTransactionStatement:
	case Statement::ObjectType::SetTransactionStatement:
	case Statement::ObjectType::CommitStatement:
	case Statement::ObjectType::RollbackStatement:
	case Statement::ObjectType::XA_StartStatement:
	case Statement::ObjectType::XA_EndStatement:
	case Statement::ObjectType::XA_PrepareStatement:
	case Statement::ObjectType::XA_CommitStatement:
	case Statement::ObjectType::XA_RollbackStatement:
	case Statement::ObjectType::XA_RecoverStatement:
	case Statement::ObjectType::XA_ForgetStatement:
		// トランザクション制御文を実行する
		executeTransactionStatement(
				iSessionID_, *pStatement_, pSQL_);
		break;

	case Statement::ObjectType::SyncStatement:
		//同期処理(チェックポイントも)を実行する
		sync(iSessionID_, pStatement_, pSQL_);
		break;

	case Statement::ObjectType::CheckpointStatement:
		//チェックポイント処理を実行する
		checkpoint(iSessionID_, pStatement_, pSQL_);
		break;

	case Statement::ObjectType::ExplainStatement:
		// explain one statement
		explain(iSessionID_, pStatement_, pParameter_,
				pSQL_, pCurrentParameter_);
		break;
	case Statement::ObjectType::StartExplainStatement:
		// set explain on
		startExplain(iSessionID_, pStatement_, pSQL_);
		break;
	case Statement::ObjectType::EndExplainStatement:
		// set explain off
		endExplain(iSessionID_, pStatement_, pSQL_);
		break;
	case Statement::ObjectType::DisconnectStatement:
		// セッションを強制終了する
		disconnect(iSessionID_, pStatement_, pSQL_);
		break;
	case Statement::ObjectType::DeclareStatement:
		declareVariable(iSessionID_, pStatement_, pSQL_);
		break;		
	default:
		//その他
		executeOperation(iSessionID_, pStatement_, pParameter_,
						 pSQL_, pCurrentParameter_);
		break;
	}
}

//	FUNCTION private
//	Server::Worker::executeTransactionStatement --
//		トランザクション関連の SQL 文を実行する
//
//	NOTES
//
//	ARGUMENTS
//		Server::ID			sessionID
//			SQL 文を実行するセッションのセッション識別子
//		Statement::Object&	stmt
//			実行する SQL 文
//		Common::DataArrayData*	dummy
//			使用しない
//		const ModUnicodeString* pSQL_
//			SQL文
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::SessionNotAvailable
//			与えられたセッション識別子の表すセッションは利用不可である

void
Worker::executeTransactionStatement(
	ID sessionID, const Statement::Object& stmt,
	const ModUnicodeString* pSQL_)
{
	if (!Session::isAvailable(sessionID))

		// 与えられたセッション識別子の表すセッションは利用不可である

		_SYDNEY_THROW0(Exception::SessionNotAvailable);

	// 以降、この関数を抜けるまでチェックポイント処理を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	// 与えられたセッション識別子の表すセッションを得る

	AutoSession session(m_cInstanceManager, sessionID, stmt.getType());

	// SQL文を設定する
	session->setCurrentSQL(pSQL_, 0);
	
	Transaction& trans = session->getTransaction();

	switch (stmt.getType()) {
	case Statement::ObjectType::StartTransactionStatement:
		startTransaction(session, stmt);				break;
	case Statement::ObjectType::SetTransactionStatement:
		trans.set(stmt);								break;
	case Statement::ObjectType::CommitStatement:
		trans.commit(stmt);								break;
	case Statement::ObjectType::RollbackStatement:
		trans.rollback(stmt);							break;
	case Statement::ObjectType::XA_StartStatement:
		xa_start(session, stmt);						break;
	case Statement::ObjectType::XA_EndStatement:
		trans.xa_end(stmt);								break;
	case Statement::ObjectType::XA_PrepareStatement:
		trans.xa_prepare(stmt);							break;
	case Statement::ObjectType::XA_CommitStatement:
		trans.xa_commit(stmt);							break;
	case Statement::ObjectType::XA_RollbackStatement:
		trans.xa_rollback(stmt);						break;
	case Statement::ObjectType::XA_RecoverStatement:
		trans.xa_recover(stmt, *m_pConnection);			break;
	case Statement::ObjectType::XA_ForgetStatement:
		trans.xa_forget(stmt);							break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//
//	FUNCTION private
//	Server::Worker::startTransaction -- トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	Server::AutoSession& session
//		セッション
//	const Statement::Object& stmt
//		ステートメント
//
//	RETURN
//	なし
//
//	EXCEPTION
//
void
Worker::startTransaction(AutoSession& session_,
						 const Statement::Object& stmt_)
{
	// トランザクションを開始するためには、データベースIDが必要
	// しかし、データベースIDを得るためのデータベースオブジェクトを
	// 取得するには、トランザクションが必要
	// 一旦、別のトランザクションでデータベース名->データベースIDに
	// 変換し、そのIDでトランザクションを開始し、
	// データベースオブジェクトを取得してから、
	// データベースIDが同じかどうかを確認する

	//【注意】	!! 以前と仕様が異なっている !!
	//			前は、トランザクション開始時ではなく、最初のSQL文実行時に、
	//			データベースの有無を確認していた
	//			しかし、今後は、トランザクション開始時に確認する

	Transaction& trans = session_->getTransaction();

	while (true)
	{
		
		// トランザクションを開始する
	
		trans.start(session_->getDatabaseID(), stmt_);

		// データベースを得る
	
		Schema::Database* pDatabase
			= Schema::Database::getLocked(
				trans.getDescriptor(), session_->getDatabaseName(),
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport);

		if (!pDatabase)
		{
			trans.rollback();
			
			// これまでにデータベースが破棄されている
		
			_SYDNEY_THROW1(Exception::DatabaseNotFound,
						   session_->getDatabaseName());
		}

		// 取得できたデータベースのIDとセッションのIDが同じか確認する

		if (pDatabase->getID() != session_->getDatabaseID())
		{
			// データベースIDが異なっている
			//	-> 同じ名前のデータベースが作り直されている

			session_->setDatabaseInfo(pDatabase->getID(), pDatabase->isSlave());
			trans.rollback();

			continue;
		}

		break;
	}
}

//
//	FUNCTION private
//	Server::Worker::xa_start -- 分散トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	Server::AutoSession& session
//		セッション
//	const Statement::Object& stmt
//		ステートメント
//
//	RETURN
//	なし
//
//	EXCEPTION
//
void
Worker::xa_start(AutoSession& session_,
				 const Statement::Object& stmt_)
{
	// トランザクションを開始するためには、データベースIDが必要
	// しかし、データベースIDを得るためのデータベースオブジェクトを
	// 取得するには、トランザクションが必要
	// 一旦、別のトランザクションでデータベース名->データベースIDに
	// 変換し、そのIDでトランザクションを開始し、
	// データベースオブジェクトを取得してから、
	// データベースIDが同じかどうかを確認する

	//【注意】	!! 以前と仕様が異なっている !!
	//			前は、トランザクション開始時ではなく、最初のSQL文実行時に、
	//			データベースの有無を確認していた
	//			しかし、今後は、トランザクション開始時に確認する

	Transaction& trans = session_->getTransaction();

	while (true)
	{
		
		// 分散トランザクションを開始する
	
		Trans::XID xid = trans.xa_start(session_->getDatabaseID(), stmt_);

		// データベースを得る
	
		Schema::Database* pDatabase
			= Schema::Database::getLocked(
				trans.getDescriptor(), session_->getDatabaseName(),
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport);

		if (!pDatabase)
		{
			trans.xa_end(xid, Trans::Branch::EndOption::Unknown);
			trans.xa_rollback(xid);
		
			// これまでにデータベースが破棄されている
		
			_SYDNEY_THROW1(Exception::DatabaseNotFound,
						   session_->getDatabaseName());
		}

		// 取得できたデータベースのIDとセッションのIDが同じか確認する

		if (pDatabase->getID() != session_->getDatabaseID())
		{
			// データベースIDが異なっている
			//	-> 同じ名前のデータベースが作り直されている

			session_->setDatabaseInfo(pDatabase->getID(), pDatabase->isSlave());

			trans.xa_end(xid, Trans::Branch::EndOption::Unknown);
			trans.xa_rollback(xid);

			continue;
		}

		if (pDatabase->getMasterURL().getLength())
		{
			trans.xa_end(xid, Trans::Branch::EndOption::Unknown);
			trans.xa_rollback(xid);
			
			// スレーブデータベースでは
			// 暗黙のトランザクションしか実行できない

			_SYDNEY_THROW0(Exception::NotSupported);
		}

		break;
	}
}

//
//	FUNCTION private
//	Server::Worker::sync -- 同期処理を実行する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		ステートメント
//	Common::DataArrayData* pParameter_
//		パラメータ
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//

void
Worker::sync(ID iSessionID_,
			 Statement::Object* pStatement_,
			 const ModUnicodeString* pSQL_)
{
	const Statement::SyncStatement& stmt =
		*_SYDNEY_DYNAMIC_CAST(const Statement::SyncStatement*,
							  pStatement_);

	int n = stmt.getCount();
	
	//セッションを取得する
	AutoSession session(m_cInstanceManager, iSessionID_, stmt.getType());
	if (!session.get()->isSuperUser())
	{
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}
	
	// SQL文を設定する
	session->setCurrentSQL(pSQL_, 0);
	
	if (trylockSync())
	{
		try
		{
			sync(n);
		}
		catch (...)
		{
			unlockSync();
			throw;
		}
	}
	else
	{
		_SYDNEY_THROW0(Exception::Cancel);
	}
}

// FUNCTION private
//	Server::Worker::sync -- 指定された回数だけ同期処理を実行する
//
// NOTES
//
// ARGUMENTS
//	int iCount_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::sync(int iCount_)
{
	// sync処理を直列化するため
	Os::AutoCriticalSection cAuto(_cSyncLatch);

	
	for (int i = 0; i < iCount_; ++i)
	{
		// 同期処理を起動する
		Checkpoint::Daemon::wakeup();

		// 終了を待つ
		Checkpoint::Daemon::wait();

		{
		// ここでFileSynchronizerを一瞬disableにすることにより
		// 同期処理が確実に終わっていることを保障する
		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::FileSynchronizer);
		}

		// ちょっと待つ
		ModOsDriver::Thread::sleep(500);
	}
}

//
//	FUNCTION private
//	Server::Worker::disconnect -- 実行中のセッション、コネクションを強制終了する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		ステートメント
//	Common::DataArrayData* pParameter_
//		パラメータ
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::disconnect(ID iSessionID_,
				   Statement::Object* pStatement_,
				   const ModUnicodeString* pSQL_)
{
	const Statement::DisconnectStatement& cStmt =
		*_SYDNEY_DYNAMIC_CAST(
			const Statement::DisconnectStatement*, pStatement_);

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);
	
	_AutoLockMap instanceMgrLock; 
	AutoSession sessionLock(m_cInstanceManager, iSessionID_, cStmt.getType());
	
	if (!m_cInstanceManager.isSuperUser(iSessionID_))
	{
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}
	
	// SQL文を設定する
	sessionLock->setCurrentSQL(pSQL_, 0);
	
	switch (cStmt.getDisconnectMode())
	{
	case Statement::DisconnectStatement::Mode::CLIENT_MODE:
		disconnectClient(cStmt.getClientId());
		break;
	case Statement::DisconnectStatement::Mode::SESSION_MODE:
		if (iSessionID_ == cStmt.getSessionId())
		{
			_SYDNEY_THROW0(Exception::SessionBusy);
		}
		disconnectSession(cStmt.getClientId(), cStmt.getSessionId());
		break;
	default:
		_SYDNEY_ASSERT(false);
	}
}


//
//	FUNCTION private
//	Server::Worker::disconnectSession -- セッションとそれに関連づいたトランザクションををクローズする
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		ステートメント
//	Common::DataArrayData* pParameter_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void Worker::disconnectSession(ID iClinentID_, ID iSessionID_){
	
	InstanceManagerPointer pInstMgr = InstanceManager::lowerBound(iClinentID_);
	if ((pInstMgr.get() == 0) || (pInstMgr->getID() != iClinentID_)) {
		_SYDNEY_THROW1(Exception::ClientNotExist, iClinentID_);
	}
	Session* pSession = pInstMgr->lowerBoundSession(iSessionID_);
	if (pSession == 0 || pSession->getID() != iSessionID_) {
		_SYDNEY_THROW1(Exception::SessionNotExist, iSessionID_);
	}

	ModList<Server::ID> listSessionID;
	listSessionID.pushBack(iSessionID_);
	pInstMgr->terminateWorkers(listSessionID);
	disconnectSession(*pInstMgr, iSessionID_);
}



//
//	FUNCTION private
//	Server::Worker::disconnectSession -- セッションとそれに関連づいたトランザクションををクローズする
//
//	NOTES
//
//	ARGUMENTS
//	Server::InstanceManager 
//		セッションを持つインスタンスマネージャー
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void Worker::disconnectSession(InstanceManager& cInstanceManager_, ID iSessionID_){
#ifndef SYD_COVERAGE
	//SessionID is required to be output?
	if (_cParameterPrintSessionID.isOutput())
	{
		_SYDNEY_MESSAGE(
			_cParameterPrintSessionID.getParameterName(),
			Common::MessageStreamBuffer::LEVEL_DEBUG)
				<< "["
				<< cInstanceManager_.getDatabaseName(iSessionID_)
				<< "] "
				<< "SessionID: " << iSessionID_ << " end." << ModEndl;
	}
#endif
				
	//セッションオブジェクトを得る(管理リストからも削除)
	Session* pSession = cInstanceManager_.popSession(iSessionID_);
	if (pSession == 0) {
		SydErrorMessage << "disconnectSession: Session "
						<< iSessionID_ << " does not exist." << ModEndl;
		_SYDNEY_THROW0(Exception::ConnectionNotExist);
	}
	
	Transaction& trans = pSession->getTransaction();
	
	try	{
		;_SERVER_FAKE_ERROR(Worker::disconnectSession);

		if (trans.isInProgress())

			// 終了するセッションで開始された
			// トランザクションが実行中であれば、ロールバックする

			trans.rollback();

		// 終了するセッションと連係している
		// トランザクションブランチをすべて待機する

		trans.xa_end();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "rollback failed" << ModEndl;
		
		//オブジェクトを開放する
		delete pSession;
		
		_SYDNEY_RETHROW;
	}

	//オブジェクトを開放する
	delete pSession;

}



//
//	FUNCTION private
//	Server::Worker::disconnectClient -- クライアントIDに関連づくコネクション、セッション、トランザクションを強制終了する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iClinentID_
//		クライアントID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::disconnectClient(ID iClientID_)
{
	if(iClientID_ == m_cInstanceManager.getID()){
		_SYDNEY_THROW0(Exception::SessionBusy);
	}
	InstanceManagerPointer pInstMgr = InstanceManager::lowerBound(iClientID_);
	if((pInstMgr.get() == 0) || (pInstMgr->getID() != iClientID_)){
		_SYDNEY_THROW1(Exception::ClientNotExist, iClientID_);
	}
	pInstMgr->abortServerConnections();
	return;
}

//
//	FUNCTION private
//	Server::Worker::declareVariable -- 変数を宣言する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		ステートメント
//	Common::DataArrayData* pParameter_
//		パラメータ
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::declareVariable(ID iSessionID_,
						Statement::Object* pStatement_,
						const ModUnicodeString* pSQL_)
{
	const Statement::DeclareStatement& stmt =
		*_SYDNEY_DYNAMIC_CAST(const Statement::DeclareStatement*,
							  pStatement_);	
	// セッションを取得する
	AutoSession session(m_cInstanceManager, iSessionID_, stmt.getType());
	
	// SQL文を設定する
	session->setCurrentSQL(pSQL_, 0);
	
	session->generateBitSetVariable(*stmt.getVariableName()->getName());
}


//
//	FUNCTION private
//	Server::Worker::checkpoint -- チェックポイント処理を実行する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		ステートメント
//	Common::DataArrayData* pParameter_
//		パラメータ
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::checkpoint(ID iSessionID_,
				   Statement::Object* pStatement_,
				   const ModUnicodeString* pSQL_)
{
	const Statement::CheckpointStatement& stmt =
		*_SYDNEY_DYNAMIC_CAST(const Statement::CheckpointStatement*,
							  pStatement_);

	int n = stmt.getCount();
	
	//セッションを取得する
	AutoSession session(m_cInstanceManager, iSessionID_, stmt.getType());
	
	if (!session.get()->isSuperUser())
	{
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}
	
	// SQL文を設定する
	session->setCurrentSQL(pSQL_, 0);
	
	checkpoint(n);
}

// FUNCTION private
//	Server::Worker::checkpoint
//		-- 指定された回数だけチェックポイント処理を実行する
//
// NOTES
//
// ARGUMENTS
//	int iCount_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::checkpoint(int iCount_)
{
	// checkpoint処理を直列化するため
	Os::AutoCriticalSection cAuto(_cSyncLatch);
	
	for (int i = 0; i < iCount_; ++i)
	{
		// 同期処理は実行しないので、disableにする
		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::FileSynchronizer);

		// 同期処理を起動する
		Checkpoint::Daemon::wakeup();

		// 終了を待つ
		Checkpoint::Daemon::wait();

		// ちょっと待つ
		ModOsDriver::Thread::sleep(500);
	}
}

//
//	FUNCTION private
//	Server::Worker::explain -- explain one statement
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		session ID
//	Statement::Object* pStatement_
//		SQL statement
//	Common::DataArrayData* pParameter_
//		parameter
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	Nothing
//
//	EXCEPTIONS
//
void
Worker::
explain(ID iSessionID_, Statement::Object* pStatement_,
		Common::DataArrayData* pParameter_,
		const ModUnicodeString* pSQL_,
		Common::DataArrayData* pCurrentParameter_)
{
	const Statement::ExplainStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::ExplainStatement*, pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	Opt::Explain::Option::Value iOption =
		Opt::Explain::getOption(*(pStatement->getOption()));

	{
		AutoSession pSession(m_cInstanceManager, iSessionID_,
							 Statement::ObjectType::StartExplainStatement);
		// start explain
		pSession->startExplain(iOption);
	}
	
	// execute statement
	executeStatement(iSessionID_, pStatement->getStatement(), pParameter_,
					 pSQL_, pCurrentParameter_);
	
	{
		AutoSession pSession(m_cInstanceManager, iSessionID_,
							 Statement::ObjectType::EndExplainStatement);
		// end explain
		pSession->endExplain();
	}
}

//
//	FUNCTION private
//	Server::Worker::startExplain -- start explain
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		session ID
//	Statement::Object* pStatement_
//		SQL statement
//	Common::DataArrayData* pParameter_
//		parameter
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	Nothing
//
//	EXCEPTIONS
//
void
Worker::
startExplain(ID iSessionID_, Statement::Object* pStatement_,
			 const ModUnicodeString* pSQL_)
{
	const Statement::StartExplainStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::StartExplainStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	Opt::Explain::Option::Value iOption
		= Opt::Explain::getOption(*(pStatement->getOption()));

	AutoSession pSession(m_cInstanceManager, iSessionID_,
						 pStatement_->getType());

	if (pSession->getExplain() != Opt::Explain::Option::None) {
		// start explain can't be executed while another explain has been
		// started
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// SQL文を設定する
	pSession->setCurrentSQL(pSQL_, 0);
	
	// start explain
	pSession->startExplain(iOption);
}

//
//	FUNCTION private
//	Server::Worker::endExplain -- end explain
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		session ID
//	Statement::Object* pStatement_
//		SQL statement
//	Common::DataArrayData* pParameter_
//		parameter
//	const ModUnicodeString* pSQL_
//		SQL文
//
//	RETURN
//	Nothing
//
//	EXCEPTIONS
//
void
Worker::
endExplain(ID iSessionID_, Statement::Object* pStatement_,
		   const ModUnicodeString* pSQL_)
{
	AutoSession pSession(m_cInstanceManager, iSessionID_,
						 pStatement_->getType());

	if (pSession->getExplain() == Opt::Explain::Option::None) {
		// end explain can't be executed while no explain have been started
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// SQL文を設定する
	pSession->setCurrentSQL(pSQL_, 0);
	
	// end explain
	pSession->endExplain();
}

//
//	FUNCTION private
//	Server::Worker::executeOperation -- トランザクション以外のSQL文を実行する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//	Statement::Object* pStatement_
//		SQL文
//	Common::DataArrayData* pParameter_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::executeOperation(ID iSessionID_,
						 Statement::Object* pStatement_,
						 Common::DataArrayData* pParameter_,
						 const ModUnicodeString* pSQL_,
						 Common::DataArrayData* pCurrentParameter_)
{
#ifdef OBSOLETE
	if (m_bExecution == false)
	{
		// 最大同時実行数が決められているので、一旦キューに登録する
		m_cInstanceManager.pushWorkerQueue(this);
		// 実行許可がおりるまで待つ
		waitEvent();

		//一度許可されたら、もうチェックしない
		m_bExecution = true;
	}
#endif
	// チェックポイント処理を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	// 実行が指示された SQL 文がどのような特性があるか調べる

	const SQLDispatchEntry& entry
		= SQLDispatchTable::getEntry(pStatement_->getType());

	bool bNeedCheckpoint = false;		// cause checkpoint after commit?
	bool bNeedReCache = false;			// cause reCache after commit?

	for (;;)
	{
		// 必要があれば、暗黙のトランザクションを開始する

		AutoSession session(m_cInstanceManager, iSessionID_,
							pStatement_->getType());
		
		// SQL文を設定する
		session->setCurrentSQL(pSQL_, pCurrentParameter_);
	
		Transaction& trans = session->getTransaction();

		trans.startStatement(session.get(),
							 *m_pConnection, entry,
							 (entry.getModuleType() == Module::Optimizer) ?
							 Trans::Transaction::IsolationLevel::Unknown :
							 Trans::Transaction::IsolationLevel::Serializable);

		_PrintSlowQuery cSlowQuery;		// 遅いクエリをログ出力するクラス

		try
		{
			;_SERVER_FAKE_ERROR(Worker::executeOperation);

			// SQL 文を実際に実行する

			switch (entry.getModuleType()) {
			case Module::Optimizer:

				// データ操作を行う
				executeOptimizer(
					session.get(), pStatement_, pParameter_, entry, trans);
				break;

			case Module::Schema:
			{
				// スキーマ操作を行う
				//
				//【注意】	操作対象のデータベースの存在を確認し、
				//			そのデータベースに対して実行可能な
				//			スキーマ操作であるかの確認は、この中で行う

				Schema::Manager::SystemTable::Result::Value iResult =
					Schema::Manager::SystemTable::reorganize(
						trans.getDescriptor(), session.get(),
						session->getDatabaseName(), pStatement_);
				bNeedCheckpoint	= (
					iResult &
					Schema::Manager::SystemTable::Result::NeedCheckpoint);
				bNeedReCache = (
					iResult &
					Schema::Manager::SystemTable::Result::NeedReCache);
				break;
			}
			case Module::Admin:

				// 運用管理操作を行う
				//
				//【注意】	操作対象のデータベースの存在を確認し、
				//			そのデータベースに対して実行可能な
				//			運用管理操作であるかの確認は、この中で行う

				bNeedReCache = Admin::Manager::executeStatement(
					trans.getDescriptor(),
					session.get(),
					*pStatement_, *m_pConnection);
				break;

			default:
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
		catch (Exception::DatabaseChanged&)
		{
			// データベース名は同じだが実体は異なっているので、再実行する

			try
			{
				// ロールバックを行う
				
				trans.rollbackStatement();
			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object& e)
#else
			catch (...)
#endif
			{
				// データベースが変わったかどうかは、
				// 実行前にチェックしているので、このロールバックが
				// 失敗することは考えにくいので、細かい場合分けは行わない
				
				SydErrorMessage << "Unexpected Exception" << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
				
				_SYDNEY_RETHROW;
			}
			continue;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// ロールバックを行う
			try
			{
				trans.rollbackStatement();
			}
			catch (Exception::Object& e)
			{
				SydErrorMessage << e << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#ifndef NO_CATCH_ALL
			catch (ModException& e)
			{
				SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (std::exception& e)
			{
				SydErrorMessage << "std::exception occurred. "
								<< (e.what() ? e.what() : "") << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (...)
			{
				SydErrorMessage << "Unexpected Exception" << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#endif
			_SYDNEY_RETHROW;
		}

		// 実行が遅かったらログにクエリの情報を出力する
	
		cSlowQuery.output(trans.getDescriptor(),
						  session->getDatabaseName(),
						  pSQL_);
	
		// SQL 文の終了を指示し、
		// 暗黙のトランザクションが開始されていれば、コミットする

		trans.commitStatement(bNeedReCache);
		
		break;
	}

	if (bNeedCheckpoint) {
		// コミット後にチェックポイントを起こす
		disabler.enable();
		checkpoint(2);
	}
}

//
//	FUNCTION private
//	Server::Worker::executeOptimizer -- データ操作を行う
//
//	NOTES
//
//	ARGUMENTS
//	Server::Session* pSession_
//		セッション
//	Statement::Object* pStatement_
//		SQL文
//	Common::DataArrayData* pParameter_
//		パラメータ
//	const SQLDispatchEntry& cEntry_
//		SQL文種別
//	Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::executeOptimizer(Session* pSession_,
						 Statement::Object* pStatement_,
						 Common::DataArrayData* pParameter_,
						 const SQLDispatchEntry& cEntry_,
						 Transaction& trans)
{
	//セッションの利用可能性をチェックする
	if (pSession_->isAvailable() == false)
		_SYDNEY_THROW0(Exception::SessionNotAvailable);

	// メタデータベース、データベース表をロックし、
	// ワーカーが操作するデータベースの名前を使って、
	// データベース表中のそのデータベースを格納するタプルをロックしながら、
	// データベースを表すクラスを取得する

	Schema::Hold::Operation::Value eHoldOperationMetaTable
		= Schema::Hold::Operation::ReadForImport;
	Schema::Hold::Operation::Value eHoldOperationMetaTuple
		= Schema::Hold::Operation::ReadForImport;

	if (pStatement_->getType() == Statement::ObjectType::BatchInsertStatement
		&& trans.getDescriptor().isImplicit()) {
		// If batch insert is executed under an implicit transaction,
		// database should be locked by VX
		eHoldOperationMetaTable = Schema::Hold::Operation::Drop;
		eHoldOperationMetaTuple = Schema::Hold::Operation::Drop;
	}

	Schema::Database* pDatabase = Schema::Database::getLocked(
		trans.getDescriptor(), pSession_->getDatabaseName(),
		Lock::Name::Category::Tuple, eHoldOperationMetaTable,
		Lock::Name::Category::Tuple, eHoldOperationMetaTuple);

	if (!pDatabase)

		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound,
					   pSession_->getDatabaseName());

	if (pDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースIDが異なっている
		// 同じ名前のデータベースが作り直されている

		pSession_->setDatabaseInfo(pDatabase->getID(), pDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	isOperationApplicable(trans.getDescriptor(),
						  *pDatabase,
						  pStatement_,
						  pSession_);

	if (pDatabase->isSlave())
	{
		// スレーブデータベース

		if (pSession_->isSlaveDatabase() == false)
			// マップもスレーブに修正する
			pSession_->setDatabaseInfo(pDatabase->getID(), true);

		// 読み取り専用トランザクションの場合は、
		// 版管理のトランザクションしか実行できない

		if (trans.getDescriptor().getCategory()
			== Trans::Transaction::Category::ReadOnly &&
			trans.getDescriptor().isNoVersion() == true)
			// トランザクションのやり直し
			_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// キャッシュが破棄されないようにopenする
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// 論理ログファイルをロック
	if (pStatement_->isRecordLog())
	{
		Schema::Manager::SystemTable::hold(trans.getDescriptor(),
										   Schema::Hold::Target::LogicalLog,
										   Lock::Name::Category::Tuple,
										   Schema::Hold::Operation::ReadWrite,
										   0,
										   Trans::Log::File::Category::Database,
										   pDatabase);
	}

	// setLogを実行する
	trans.setLog(*pDatabase);

	if (pDatabase->hasCascade(trans.getDescriptor()))
	{
		// 分散マネージャとしてSQL文を実行する必要あり
		
		// 必要なら子サーバのトランザクションブランチを開始する
		trans.startXATransaction(cEntry_, *pDatabase);
	}

	// 検索条件を正規化する
	pStatement_->expandCondition();

	Execution::Program cProgram;
	Opt::Explain::Option::Value iExplain = pSession_->getExplain();

	//オプティマイザーを実行する
	{
#ifndef SYD_COVERAGE
	_AutoPrintTime tmsg("Optimizer");
#endif
	Opt::Optimizer::optimize(pDatabase, &cProgram, m_pConnection,
							 pStatement_, pParameter_, &trans.getDescriptor(),
							 iExplain);
	}

	;_SERVER_FAKE_ERROR(Worker::executeOptimizer);

	if (m_pConnection->isCanceled())

		// 中断要求が来ているので例外を投げる

		_SYDNEY_THROW0(Exception::Cancel);

	if ((iExplain & Opt::Explain::Option::Explain) != 0
		&& (iExplain & Opt::Explain::Option::Execute) != 0) {
		if (m_pConnection->getMasterID() >= Communication::Protocol::Version4) {
			Common::Status cHasMoreData(Common::Status::HasMoreData);
			// for jdbc.Statement.getMoreResults
			m_pConnection->writeObject(&cHasMoreData);
		}
	}

	//エグゼキュータを実行する
	if ((iExplain & Opt::Explain::Option::Explain) == 0
		|| (iExplain & Opt::Explain::Option::Execute) != 0) {
#ifndef SYD_COVERAGE
		_AutoPrintTime tmsg("Executor");
#endif
		if (trans.getDescriptor().isBatchMode()) {
			// wait for end of file synchronizer and disable it
			Checkpoint::Daemon::AutoDisabler
				disabler(Checkpoint::Daemon::Category::FileSynchronizer);

			Execution::Executor().execute(cProgram);
		} else {
			Execution::Executor().execute(cProgram);
		}
	}
}

//
//	FUNCTION private
//	Server::Worker::prepareStatement -- SQL文をコンパイルする
//
//	NOTES
//	SQL文をコンパイルしてその結果を保持する。
//	今はOptimizerだけにその機能がある。
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
Worker::prepareStatement()
{
	//データベース名を受け取る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::StringData* pDatabaseName
		= dynamic_cast<Common::StringData*>(pObject0.get());

	//SQL文を得る
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::StringData* pSQL
		= dynamic_cast<Common::StringData*>(pObject1.get());

	if (pDatabaseName == 0|| pSQL == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	Statement::Object* pStatement = 0;
	ModAutoPointer<Statement::Object> pSQLWrapper;

	try
	{
		//SQL文をパースする
		Statement::SQLParser cSQLParser;

		//SQL文を与える
		cSQLParser.setText(pSQL->getValue());

		//パース結果を1文づつ取り出す
		Statement::Object* pObject = 0;

		while (cSQLParser.parse(pObject) == Statement::SQLParser::PARSE_ACCEPT)
		{
			if (pObject != 0)
			{
				pSQLWrapper = pObject;

				if (pStatement)
				{
					//複文はサポートしていない
					SydInfoMessage
						<< "[" << pDatabaseName->getValue() << "] "
						<< "SQL=" << *pSQL << ModEndl;
					_SYDNEY_THROW0(Exception::NotSupported);
				}

				pStatement
					= _SYDNEY_DYNAMIC_CAST(Statement::SQLWrapper*,
										   pSQLWrapper.get())->getObject();
			}
		}

#ifndef SYD_COVERAGE
		//SQL文を出力するか
		if (_cParameterPrintSQLStatement.isOutput())
		{
			const ModUnicodeString* pSQL
				= _SYDNEY_DYNAMIC_CAST(Statement::SQLWrapper*,
									   pSQLWrapper.get())->getSQLString();
			if (pSQL)
				_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "SQL: " << *pSQL << ModEndl;
			else
				_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "SQL: " << "(null)" << ModEndl;
		}
#endif
	}
	catch (Exception::SQLSyntaxError&)
	{
		SydInfoMessage
			<< "[" << pDatabaseName->getValue() << "] "
			<< "SQL=" << pSQL->getValue() << ModEndl;
		throw;
	}

#ifdef OBSOLETE
	// 最大同時実行数が決められているので、一旦キューに登録する
	m_cInstanceManager.pushWorkerQueue(this);
	// 実行許可がおりるまで待つ
	waitEvent();
#endif

	// 実行が指示された SQL 文がどのような特性があるか調べる

	const SQLDispatchEntry& entry
		= SQLDispatchTable::getEntry(pStatement->getType());

	if (entry.getModuleType() != Module::Optimizer)
	{
		//Optimizer以外はサポート外
		SydInfoMessage
			<< "[" << pDatabaseName->getValue() << "] "
			<< "SQL=" << *pSQL << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// セッションオブジェクトを作成する
	ModAutoPointer<Session> pSession = new Session;
	// データベース名を設定する
	pSession->setDatabaseName(pDatabaseName->getValue());
	// スキーマに使用開始をレポートする
	Schema::Database::reserve(pSession->getID());
	// セッションを登録する
	m_cInstanceManager.pushSession(pSession.get());

	//セッションの利用可能性はチェックしない(ありえない)

	try
	{
		
		// チェックポイント処理を実行不可にする

		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::Executor);

		int iID = 0;	//PrepareStatementID

		// 暗黙のトランザクションを開始する

		AutoSession session(m_cInstanceManager, pSession->getID(),
							Statement::ObjectType::PrepareStatement);
		
		// SQL文を設定する
		session->setCurrentSQL(&(pSQL->getValue()), 0);
	
		Transaction& trans = session->getTransaction();

		trans.startStatement(session.get(),
							 *m_pConnection, entry,
							 Trans::Transaction::IsolationLevel::Unknown);

		try
		{
			try
			{
				// SQL 文をコンパイルする

				// メタデータベース、データベース表をロックし、
				// ワーカーが操作するデータベースの名前を使って、
				// データベース表中のそのデータベースを格納する
				// タプルをロックしながら、データベースを表すクラスを取得する

				Schema::Database* pDatabase = Schema::Database::getLocked(
					trans.getDescriptor(),
					session->getDatabaseName(),
					Lock::Name::Category::Tuple,
					Schema::Hold::Operation::ReadForImport,
					Lock::Name::Category::Tuple,
					Schema::Hold::Operation::ReadForImport);

				if (!pDatabase)

					// これまでにデータベースが破棄されている

					_SYDNEY_THROW1(Exception::DatabaseNotFound,
								   session->getDatabaseName());

				// 操作を実行するトランザクションにおいて実行可能な SQL 文
				// か調べる。また、操作対象であるデータベース対して実行可能な
				// SQL 文か調べる

				isOperationApplicable(trans.getDescriptor(), *pDatabase,
									  pStatement,
									  session.get());

				// キャッシュが破棄されないようにopenする
				pDatabase->open();
				// スコープを抜けるときにデータベースのcloseを呼ぶ
				Common::AutoCaller1<Schema::Database, bool>
					autoCloser(pDatabase, &Schema::Database::close, false);

				;_SERVER_FAKE_ERROR(Worker::prepareStatement);

				// 論理ログファイルをロック
				if (pStatement->isRecordLog())
				{
					Schema::Manager::SystemTable::hold(
						trans.getDescriptor(),
						Schema::Hold::Target::LogicalLog,
						Lock::Name::Category::Tuple,
						Schema::Hold::Operation::ReadWrite,
						0,
						Trans::Log::File::Category::Database,
						pDatabase);
				}

				// setLogを実行する
				trans.setLog(*pDatabase);

				// 検索条件を正規化する
				pStatement->expandCondition();

				//オプティマイザーを実行する
				{
#ifndef SYD_COVERAGE
				_AutoPrintTime tmsg("Optimizer(prepare)");
#endif
				iID = Opt::Optimizer::prepare(
					pDatabase, m_pConnection,
					pStatement, &trans.getDescriptor());
				}

#ifndef SYD_COVERAGE
				//SQL文を出力するか
				if (_cParameterPrintSQLStatement.isOutput())
				{
					_SYDNEY_MESSAGE(
						_cParameterPrintSQLStatement.getParameterName(),
						Common::MessageStreamBuffer::LEVEL_DEBUG)
							<< "PREPARE-ID: " << (iID * 2 + 1) << ModEndl;

				}
#endif

				// サーバコネクションに登録する
				m_cInstanceManager.pushPrepareID(
					pDatabaseName->getValue(), iID);
			}
			catch (Exception::UserLevel&)
			{
				//ユーザレベルのエラー
				SydInfoMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
				throw;
			}
			catch (Exception::ServerNotAvailable&)
			{
				//Server not availableのエラー
				if (Manager::isNotAvailableLogged() == false) {
					SydErrorMessage
						<< "[" << session->getDatabaseName() << "] "
						<< "SQL=" << *pSQL << ModEndl;
				}
				throw;
			}
			catch (Exception::Object&)
			{
				//Sydneyのエラー
				SydErrorMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
				throw;
			}
#ifndef NO_CATCH_ALL
			catch (ModException&)
			{
				//Modのエラー
				SydErrorMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
				throw;
			}
			catch (std::exception&)
			{
				//標準C++の例外
				SydErrorMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
				throw;
			}
			catch (...)
			{
				//その他のエラー
				SydErrorMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
				throw;
			}
#endif
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				// 例外が発生したのでロールバックする

				trans.rollbackStatement();
			}
			catch (Exception::Object& e)
			{
				SydErrorMessage << e << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#ifndef NO_CATCH_ALL
			catch (ModException& e)
			{
				SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (std::exception& e)
			{
				SydErrorMessage << "std::exception occurred. "
								<< (e.what() ? e.what() : "") << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (...)
			{
				SydErrorMessage << "Unexpected Exception" << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#endif
			_SYDNEY_RETHROW;
		}

		// SQL 文の終了を指示し、
		// 暗黙のトランザクションが開始されていれば、コミットする

		trans.commitStatement();

		// PrepareStatementIDをクライアントに返す
		// 古いバージョンのIDは奇数にする
		iID *= 2;
		++iID;
		Common::IntegerData data(iID);
		m_pConnection->writeObject(&data);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// セッションを削除する
		m_cInstanceManager.popSession(pSession->getID());
		_SYDNEY_RETHROW;
	}
	
	// セッションを削除する
	m_cInstanceManager.popSession(pSession->getID());
}

//
//	FUNCTION private
//	Server::Worker::prepareStatement2 -- SQL文をコンパイルする
//
//	NOTES
//	SQL文をコンパイルしてその結果を保持する。
//	今はOptimizerだけにその機能がある。
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
Worker::prepareStatement2()
{
	//セッションIDを得る
	ID iSessionID = recvSessionID();
	
	//SQL文を得る
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::StringData* pSQL
		= dynamic_cast<Common::StringData*>(pObject1.get());

	if (pSQL == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	Statement::Object* pStatement = 0;
	ModAutoPointer<Statement::Object> pSQLWrapper;

	try
	{
		//SQL文をパースする
		Statement::SQLParser cSQLParser;

		//SQL文を与える
		cSQLParser.setText(pSQL->getValue());

		//パース結果を1文づつ取り出す
		Statement::Object* pObject = 0;

		while (cSQLParser.parse(pObject) == Statement::SQLParser::PARSE_ACCEPT)
		{
			if (pObject != 0)
			{
				pSQLWrapper = pObject;

				if (pStatement)
				{
					//複文はサポートしていない
					_SYDNEY_THROW0(Exception::NotSupported);
				}

				pStatement
					= _SYDNEY_DYNAMIC_CAST(Statement::SQLWrapper*,
										   pSQLWrapper.get())->getObject();
			}
		}

#ifndef SYD_COVERAGE
		//SessionIDを出力するか
		if (_cParameterPrintSessionID.isOutput())
		{
			_SYDNEY_MESSAGE(
				_cParameterPrintSessionID.getParameterName(),
				Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "["
					<< m_cInstanceManager.getDatabaseName(iSessionID)
					<< "] "
					<< "SessionID: " << iSessionID << ModEndl;
		}

		//SQL文を出力するか
		if (_cParameterPrintSQLStatement.isOutput())
		{
			const ModUnicodeString* pSQL
				= _SYDNEY_DYNAMIC_CAST(Statement::SQLWrapper*,
									   pSQLWrapper.get())->getSQLString();
			if (pSQL)
				_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "SQL: " << *pSQL << ModEndl;
			else
				_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
					Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< "SQL: " << "(null)" << ModEndl;
		}
#endif
	}
	catch (Exception::SessionNotExist&)
	{
		SydErrorMessage
			<< "SessionID=" << iSessionID << ModEndl;
		throw;
	}
	catch (Exception::UserLevel&)
	{
		SydInfoMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << pSQL->getValue() << ModEndl;
		throw;
	}
	catch (Exception::ServerNotAvailable&)
	{
		if (Manager::isNotAvailableLogged() == false) {
			SydErrorMessage
				<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
				<< "SQL=" << pSQL->getValue() << ModEndl;
		}
		throw;
	}
	catch (Exception::Object&)
	{
		SydErrorMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << pSQL->getValue() << ModEndl;
		throw;
	}

#ifdef OBSOLETE
	// 最大同時実行数が決められているので、一旦キューに登録する
	m_cInstanceManager.pushWorkerQueue(this);
	// 実行許可がおりるまで待つ
	waitEvent();
#endif

	// 実行が指示された SQL 文がどのような特性があるか調べる

	const SQLDispatchEntry& entry
		= SQLDispatchTable::getEntry(pStatement->getType());

	if (entry.getModuleType() != Module::Optimizer)
	{
		//Optimizer以外はサポート外
		SydInfoMessage
			<< "[" << m_cInstanceManager.getDatabaseName(iSessionID) << "] "
			<< "SQL=" << pSQL->getValue() << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	{	// pObject1 より前に session がデストラクトされるようにするため
		
	// 必要があれば、暗黙のトランザクションを開始する

	AutoSession session(m_cInstanceManager, iSessionID,
						Statement::ObjectType::PrepareStatement);
	
	// SQL文を設定する
	session->setCurrentSQL(&(pSQL->getValue()), 0);
	
	Transaction& trans = session->getTransaction();

	trans.startStatement(session.get(),
						 *m_pConnection, entry,
						 Trans::Transaction::IsolationLevel::Unknown);
	
	int iID = 0;	//PrepareStatementID

	try
	{
		try
		{
			//セッションの利用可能性をチェックする
			if (session->isAvailable() == false)
				_SYDNEY_THROW0(Exception::SessionNotAvailable);
		
			// SQL 文をコンパイルする

			// メタデータベース、データベース表をロックし、
			// ワーカーが操作するデータベースの名前を使って、
			// データベース表中のそのデータベースを格納する
			// タプルをロックしながら、データベースを表すクラスを取得する

			Schema::Database* pDatabase = Schema::Database::getLocked(
				trans.getDescriptor(),
				session->getDatabaseName(),
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport);

			if (!pDatabase)

				// これまでにデータベースが破棄されている

				_SYDNEY_THROW1(Exception::DatabaseNotFound,
							   session->getDatabaseName());

			// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
			// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

			isOperationApplicable(trans.getDescriptor(), *pDatabase,
								  pStatement,
								  session.get());

			// キャッシュが破棄されないようにopenする
			pDatabase->open();
			// スコープを抜けるときにデータベースのcloseを呼ぶ
			Common::AutoCaller1<Schema::Database, bool>
				autoCloser(pDatabase, &Schema::Database::close, false);
			
			;_SERVER_FAKE_ERROR(Worker::prepareStatement2);


			// 論理ログファイルをロック
			if (pStatement->isRecordLog())
			{
				Schema::Manager::SystemTable::hold(
					trans.getDescriptor(),
					Schema::Hold::Target::LogicalLog,
					Lock::Name::Category::Tuple,
					Schema::Hold::Operation::ReadWrite,
					0,
					Trans::Log::File::Category::Database,
					pDatabase);
			}

			// setLogを実行する
			trans.setLog(*pDatabase);

			// 検索条件を正規化する
			pStatement->expandCondition();

			ModAutoPointer<Opt::Planner> pPlanner;

			//オプティマイザーを実行する
			{
#ifndef SYD_COVERAGE
			_AutoPrintTime tmsg("Optimizer(prepare)");
#endif
			// To enclose new-delete pair in Server module,
			// Planner object should be created here.
			pPlanner = new Opt::Planner;
			Opt::Optimizer::prepare2(*pPlanner, pDatabase, m_pConnection,
									 pStatement, &trans.getDescriptor());
			}

			// セッションに登録する
			iID = session->pushPreparePlan(pPlanner.release(),
										   pSQL->getValue());

#ifndef SYD_COVERAGE
			//SQL文を出力するか
			if (_cParameterPrintSQLStatement.isOutput())
			{
				_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
								Common::MessageStreamBuffer::LEVEL_DEBUG)
									<< "PREPARE-ID: " << (iID * 2 + 2)
									<< ModEndl;

			}
#endif
		}
		catch (Exception::UserLevel&)
		{
			SydInfoMessage
				<< "[" << session->getDatabaseName() << "] "
				<< "SQL=" << *pSQL << ModEndl;
			throw;
		}
		catch (Exception::ServerNotAvailable&)
		{
			if (Manager::isNotAvailableLogged() == false) {
				SydErrorMessage
					<< "[" << session->getDatabaseName() << "] "
					<< "SQL=" << *pSQL << ModEndl;
			}
			throw;
		}
		catch (Exception::Object&)
		{
			SydErrorMessage
				<< "[" << session->getDatabaseName() << "] "
				<< "SQL=" << *pSQL << ModEndl;
			throw;
		}
#ifndef NO_CATCH_ALL
		catch (ModException&)
		{
			SydErrorMessage
				<< "[" << session->getDatabaseName() << "] "
				<< "SQL=" << *pSQL << ModEndl;
			throw;
		}
		catch (std::exception&)
		{
			SydErrorMessage
				<< "[" << session->getDatabaseName() << "] "
				<< "SQL=" << *pSQL << ModEndl;
			throw;
		}
		catch (...)
		{
			SydErrorMessage
				<< "[" << session->getDatabaseName() << "] "
				<< "SQL=" << *pSQL << ModEndl;
			throw;
		}
#endif
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		try
		{
			// 例外が発生したのでロールバックする

			trans.rollbackStatement();
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			SydErrorMessage << "rollback failed." << ModEndl;
		}
#ifndef NO_CATCH_ALL
		catch (ModException& e)
		{
			SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
			SydErrorMessage << "rollback failed." << ModEndl;
		}
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			SydErrorMessage << "rollback failed." << ModEndl;
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected Exception" << ModEndl;
			SydErrorMessage << "rollback failed." << ModEndl;
		}
#endif
		_SYDNEY_RETHROW;
	}

	// SQL 文の終了を指示し、
	// 暗黙のトランザクションが開始されていれば、コミットする

	trans.commitStatement();

	// PrepareStatementIDをクライアントに返す
	// 新しいバージョンのIDは偶数にする
	// ID must be larger than zero.
	++iID;
	iID *= 2;
	Common::IntegerData data(iID);
	m_pConnection->writeObject(&data);

	}
}

//
//	FUNCTION private
//	Server::Worker::executePrepare -- コンパイルしたものを実行する
//
//	NOTES
//	コンパイルしたSQL文を実行する
//	今はOptimizerだけにその機能がある。
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
Worker::executePrepare()
{
	//SessionIDを得る
	ID iSessionID = recvSessionID();

	//PrepareStatementIDを得る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::IntegerData* pID
		= dynamic_cast<Common::IntegerData*>(pObject0.get());

	//パラメータを得る
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::DataArrayData* pParameter
		= dynamic_cast<Common::DataArrayData*>(pObject1.get());

	//エラーチェック
	if (pID == 0)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

#ifndef SYD_COVERAGE
	//SessionIDを出力するか
	if (_cParameterPrintSessionID.isOutput())
	{
		_SYDNEY_MESSAGE(_cParameterPrintSessionID.getParameterName(),
						Common::MessageStreamBuffer::LEVEL_DEBUG)
							<< "["
							<< m_cInstanceManager.getDatabaseName(iSessionID)
							<< "] "
							<< "SessionID: " << iSessionID << ModEndl;
	}

	//SQL文を出力するか
	if (_cParameterPrintSQLStatement.isOutput())
	{
		_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
						Common::MessageStreamBuffer::LEVEL_DEBUG)
							<< "PREPARE-ID: " << pID->getValue() << ModEndl;
	}
	
	//パラメータを出力するか
	if (_cParameterPrintParameter.isOutput())
	{
		if (pParameter)
		{
			_SYDNEY_MESSAGE(_cParameterPrintParameter.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "Parameter: " << *pParameter << ModEndl;
		}
		else
		{
			_SYDNEY_MESSAGE(_cParameterPrintParameter.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "Parameter: " << "(null)" << ModEndl;
		}
	}
#endif

	{	// pObject1 より前に session がデストラクトされるようにするため
	
	// セッションを得る
	AutoSession session(m_cInstanceManager, iSessionID,
						Statement::ObjectType::ExecutePrepareStatement);

	//セッションの利用可能性をチェックする
	if (session->isAvailable() == false)
		_SYDNEY_THROW0(Exception::SessionNotAvailable);

	// IDが偶数か奇数かで処理が違う
	int iID;
	int iStatementType;
	Opt::Planner* pPlanner = 0;
	const ModUnicodeString* pSQL = 0;
	
	if (pID->getValue() % 2) {
		// 奇数 → セッション間で共有する
		iID = (pID->getValue() - 1) / 2;

		//Optimizerから該当するStatement::Objectのタイプを取り出す
		iStatementType
			= Opt::Optimizer::getPrepareStatementType(iID);
	} else {
		// 偶数 → セッション間で共有しない
		iID = pID->getValue() / 2;
		--iID; // 1-base => 0-base

		// Sessionから該当するPlannerオブジェクトを取り出す
		pPlanner = session->getPreparePlan(iID);
		if (!pPlanner) {
			//存在していないのでエラー
			SydErrorMessage << "Bad prepare ID: "
							<< pID->getValue() << ModEndl;
			_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
		}

		// Statement::Objectのタイプを取り出す
		iStatementType = pPlanner->getStatementType();

		// SQL文も取り出す
		pSQL = session->getPrepareSQL(iID);

		// 実行中SQL文を設定する
		Common::DataArrayData* pCurrentParameter = 0;
		if (pParameter && pParameter->getCount() > 0)
			pCurrentParameter = pParameter;
		session->setCurrentSQL(pSQL, pCurrentParameter);
	}

#ifdef OBSOLETE
	// 最大同時実行数が決められているので、一旦キューに登録する
	m_cInstanceManager.pushWorkerQueue(this);
	// 実行許可がおりるまで待つ
	waitEvent();
#endif

	switch (iStatementType) {
	case Statement::ObjectType::DeclareStatement:
	{
		if (!pPlanner) {
			pPlanner = &Opt::Planner::get(iID);
			if (!pPlanner) 
				_SYDNEY_THROW0(Exception::NotSupported);
		}
		
		const Statement::DeclareStatement& stmt =
			*_SYDNEY_DYNAMIC_CAST(const Statement::DeclareStatement*,
								  pPlanner->getStatement());
		session.get()->generateBitSetVariable(
			*stmt.getVariableName()->getName());
		break;
	}
	default:
		doExecutePrepare(iStatementType, session, pPlanner, pParameter,
						 iID, pSQL);
	}
	}
}

//
//	FUNCTION private
//	Server::Worker::doExecutePrepare -- コンパイルしたものを実行する
//
//	NOTES
//	コンパイルしたSQL文を実行する
//	今はOptimizerだけにその機能がある。
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
Worker::doExecutePrepare(int iStatementType_,
						 AutoSession& cSession_,
						 Opt::Planner* pPlanner_,
						 Common::DataArrayData* pParameter_,
						 int iID_,
						 const ModUnicodeString* pSQL_)
{
	// 実行が指示された SQL 文がどのような特性があるか調べる
	const SQLDispatchEntry& entry = SQLDispatchTable::getEntry(iStatementType_);

	// チェックポイント処理を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	for (;;)
	{
		// 必要なら、暗黙のトランザクションを開始する

		cSession_.changeStatementType(iStatementType_);
		Transaction& trans = cSession_->getTransaction();

		trans.startStatement(cSession_.get(),
							 *m_pConnection, entry,
							 Trans::Transaction::IsolationLevel::Unknown);

		_PrintSlowQuery cSlowQuery;		// 遅いクエリをログ出力するクラス

		try
		{
			// メタデータベース、データベース表をロックし、
			// ワーカーが操作するデータベースの名前を使って、
			// データベース表中のそのデータベースを格納するタプルを
			// ロックしながら、データベースを表すクラスを取得する

			Schema::Hold::Operation::Value eHoldOperationMetaTable = Schema::Hold::Operation::ReadForImport;
			Schema::Hold::Operation::Value eHoldOperationMetaTuple = Schema::Hold::Operation::ReadForImport;

			if (iStatementType_ == Statement::ObjectType::BatchInsertStatement
				&& trans.getDescriptor().isImplicit()) {
				// If batch insert is executed under an implicit transaction,
				// database should be locked by VX
				eHoldOperationMetaTable = Schema::Hold::Operation::Drop;
				eHoldOperationMetaTuple = Schema::Hold::Operation::Drop;
			}

			Schema::Database* pDatabase = Schema::Database::getLocked(
				trans.getDescriptor(),
				cSession_->getDatabaseName(),
				Lock::Name::Category::Tuple, eHoldOperationMetaTable,
				Lock::Name::Category::Tuple, eHoldOperationMetaTuple);

			if (!pDatabase)

				// これまでにデータベースが破棄されている

				_SYDNEY_THROW1(Exception::DatabaseNotFound,
							   cSession_->getDatabaseName());

			if (pDatabase->getID() != cSession_->getDatabaseID())
			{
				// データベースIDが異なっている
				// 同じ名前のデータベースが作り直されている

				cSession_->setDatabaseInfo(pDatabase->getID(),
										   pDatabase->isSlave());
				_SYDNEY_THROW0(Exception::DatabaseChanged);
			}

			// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
			// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

			isOperationApplicable(
				trans.getDescriptor(), *pDatabase, iStatementType_);
			// privilege is not checked
			// because it should have been checked in prepareStatement

			// キャッシュが破棄されないようにopenする
			pDatabase->open();
			// スコープを抜けるときにデータベースのcloseを呼ぶ
			Common::AutoCaller1<Schema::Database, bool>
				autoCloser(pDatabase, &Schema::Database::close, false);

			// 論理ログファイルをロック
			if (entry.isDatabaseLogRecorded() == Boolean::True)
			{
				Schema::Manager::SystemTable::hold(
					trans.getDescriptor(),
					Schema::Hold::Target::LogicalLog,
					Lock::Name::Category::Tuple,
					Schema::Hold::Operation::ReadWrite,
					0,
					Trans::Log::File::Category::Database,
					pDatabase);
			}

			// setLogを実行する
			trans.setLog(*pDatabase);

			if (pDatabase->hasCascade(trans.getDescriptor()))
			{
				// 分散マネージャとしてSQL文を実行する必要あり
		
				// 必要なら子サーバのトランザクションブランチを開始する
				trans.startXATransaction(entry, *pDatabase);
			}

			Execution::Program cProgram;
			Opt::Explain::Option::Value iExplain = cSession_->getExplain();

			//オプティマイザーを実行する
			{
#ifndef SYD_COVERAGE
				_AutoPrintTime tmsg("Optimizer(generate)");
#endif
				if (pPlanner_)
					Opt::Optimizer::generate2(pDatabase,
											  &cProgram,
											  m_pConnection,
											  *pPlanner_,
											  pParameter_,
											  &trans.getDescriptor(),
											  iExplain);
				else
					Opt::Optimizer::generate(pDatabase,
											 &cProgram,
											 m_pConnection,
											 iID_,
											 pParameter_,
											 &trans.getDescriptor(),
											 iExplain);
			}

			;_SERVER_FAKE_ERROR(Worker::executePrepare);

			if (m_pConnection->isCanceled())

				// 中断要求が来ているので例外を投げる

				_SYDNEY_THROW0(Exception::Cancel);

			if ((iExplain & Opt::Explain::Option::Explain) != 0
				&& (iExplain & Opt::Explain::Option::Execute) != 0) {
				if (m_pConnection->getMasterID()
					>= Communication::Protocol::Version4) {
					Common::Status cHasMoreData(Common::Status::HasMoreData);
					// for jdbc.Statement.getMoreResults
					m_pConnection->writeObject(&cHasMoreData);
				}
			}

			//エグゼキュータを実行する
			if ((iExplain & Opt::Explain::Option::Explain) == 0
				|| (iExplain & Opt::Explain::Option::Execute) != 0) {
#ifndef SYD_COVERAGE
				_AutoPrintTime tmsg("Executor");
#endif
				Execution::Executor().execute(cProgram);
			}
		}
		
		catch (Exception::DatabaseChanged&)
		{
			// データベース名は同じだが実体は異なっているので、再実行する

			try
			{
				// ロールバックを行う
				
				trans.rollbackStatement();
			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object& e)
#else
			catch (...)
#endif
			{
				// データベースが変わったかどうかは、
				// 実行前にチェックしているので、このロールバックが
				// 失敗することは考えにくいので、細かい場合分けは行わない
				
				SydErrorMessage << "Unexpected Exception" << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
				
				_SYDNEY_RETHROW;
			}
			continue;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			if (pSQL_)
			{
				SydInfoMessage
					<< "[" << m_cInstanceManager.getDatabaseName(
						cSession_->getID()) << "] "
					<< "SQL=" << *pSQL_ << ModEndl;
			}
			if (pParameter_)
			{
				SydInfoMessage
					<< "[" << m_cInstanceManager.getDatabaseName(
						cSession_->getID()) << "] "
					<< "Parameter=" << *pParameter_ << ModEndl;
			}
		
			try
			{
				//例外が発生したのでロールバックする

				trans.rollbackStatement();
			}
			catch (Exception::Object& e)
			{
				SydErrorMessage << e << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#ifndef NO_CATCH_ALL
			catch (ModException& e)
			{
				SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (std::exception& e)
			{
				SydErrorMessage << "std::exception occurred. "
								<< (e.what() ? e.what() : "") << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
			catch (...)
			{
				SydErrorMessage << "Unexpected Exception" << ModEndl;
				SydErrorMessage << "rollback failed." << ModEndl;
			}
#endif
			_SYDNEY_RETHROW;
		}

		// 実行が遅かったらログにクエリの情報を出力する
	
		cSlowQuery.output(trans.getDescriptor(),
						  cSession_->getDatabaseName(),
						  pSQL_);
	
		// SQL 文の終了を指示し、
		// 暗黙のトランザクションが開始されていれば、コミットする

		trans.commitStatement();

		break;
	}
}

//
//	FUNCTION private
//	Server::Worker::erasePrepareStatement -- 最適化結果を削除する
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
Worker::erasePrepareStatement()
{
	//セッションIDを受け取る
	ID iSessionID = recvSessionID();

	//最適化IDを取得する
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::IntegerData* pPrepareID
		= dynamic_cast<Common::IntegerData*>(pObject1.get());

	if (pPrepareID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	// セッションを得る
	AutoSession session(m_cInstanceManager, iSessionID,
						Statement::ObjectType::ErasePrepareStatement);

	//セッションの利用可能性をチェックする
	if (session->isAvailable() == false)
		_SYDNEY_THROW0(Exception::SessionNotAvailable);

	;_SERVER_FAKE_ERROR(Worker::erasePrepareStatement);

	// IDが偶数か奇数かで処理が変わる
	if (pPrepareID->getValue() % 2) {
		// 奇数 → セッション間で共有するもの

		int iID = (pPrepareID->getValue() - 1) / 2;

		//最適化結果が存在するかチェックする
		if (session->checkPrepareID(iID) == false)
		{
			//存在していないのでエラー
			SydErrorMessage << "Bad prepare ID: "
							<< pPrepareID->getValue() << ModEndl;
			_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
		}

#ifndef SYD_COVERAGE
		if (_cParameterPrintSQLStatement.isOutput())
		{
			_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "Erase PREPARE-ID: "
								<< pPrepareID->getValue() << ModEndl;
		}
#endif
	
		//最適化結果を削除する
		session->popPrepareID(iID);

		//Optimizerのも削除
		Opt::Optimizer::erasePrepareStatement(iID);

	} else {
		// 偶数 → セッション内で閉じるもの

		int iID = pPrepareID->getValue() / 2;
		--iID;

		// 最適化結果を得る
		Opt::Planner* pPlanner = session->getPreparePlan(iID);
		if (!pPlanner) {
			// 存在していないのでエラー
			SydErrorMessage << "Bad prepare ID: "
							<< pPrepareID->getValue() << ModEndl;
			_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
		}

#ifndef SYD_COVERAGE
		if (_cParameterPrintSQLStatement.isOutput())
		{
			_SYDNEY_MESSAGE(_cParameterPrintSQLStatement.getParameterName(),
							Common::MessageStreamBuffer::LEVEL_DEBUG)
								<< "Erase PREPARE-ID: "
								<< pPrepareID->getValue() << ModEndl;
		}
#endif

		// 最適化結果を削除する
		session->popPreparePlan(iID);
		// ★注意★
		// これ以降pPlannerは不正アドレスになる
	}
}

// FUNCTION private
//	Server::Worker::createUser -- create new user
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
createUser()
{
	// Receive SessionID
	ID iSessionID = recvSessionID();

	// Receive new user name
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::StringData* pUserName
		= dynamic_cast<Common::StringData*>(pObject0.get());
	// Receive password for new user
	ModUnicodeString cstrPassword;
	recvPassword(cstrPassword);

	// Receive user id
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::IntegerData* pUserID
		= dynamic_cast<Common::IntegerData*>(pObject1.get());
	if (pUserName == 0 || pUserID == 0)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if (!m_cInstanceManager.isSuperUser(iSessionID)) {
		// No permission
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}

	Manager::addUser(pUserName->getValue(), cstrPassword, pUserID->getValue());
}

// FUNCTION private
//	Server::Worker::dropUser -- drop a user
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
dropUser()
{
	// Receive SessionID
	ID iSessionID = recvSessionID();

	// Receive dropped user name
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::StringData* pUserName
		= dynamic_cast<Common::StringData*>(pObject0.get());

	// Receive drop behavior
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::IntegerData* pDropBehavior
		= dynamic_cast<Common::IntegerData*>(pObject1.get());

	if (!m_cInstanceManager.isSuperUser(iSessionID)) {
		// No permission
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}

	if (pUserName == 0 || pDropBehavior == 0)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if (m_cInstanceManager.getUserName(iSessionID).compare(pUserName->getValue(),
														   ModFalse /* case insensitive */)
		== 0) {
		// User can't drop himself
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}

	Manager::deleteUser(pUserName->getValue(), pDropBehavior->getValue());
	if (pDropBehavior->getValue() == Communication::User::DropBehavior::Cascade) {
		// cause checkpoint
		checkpoint(2);
	}
}

// FUNCTION private
//	Server::Worker::changeOwnPassword -- change own password
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
changeOwnPassword()
{
	// Receive SessionID
	ID iSessionID = recvSessionID();

	// Receive new password for session user
	ModUnicodeString cstrPassword;
	recvPassword(cstrPassword);

	Manager::changePassword(m_cInstanceManager.getUserName(iSessionID),
							cstrPassword);
}

// FUNCTION private
//	Server::Worker::changePassword -- change password for a user
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
changePassword()
{
	// Receive SessionID
	ID iSessionID = recvSessionID();

	// Receive target user name
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::StringData* pUserName
		= dynamic_cast<Common::StringData*>(pObject0.get());

	// Receive new password for specified user
	ModUnicodeString cstrPassword;
	recvPassword(cstrPassword);

	if (!m_cInstanceManager.isSuperUser(iSessionID)) {
		// No permission
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}

	if (pUserName == 0)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Manager::changePassword(pUserName->getValue(), cstrPassword);
}

//
//	FUNCTION private
//	Server::Worker::checkReplication -- レプリケーションを確認する
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
Worker::checkReplication()
{
	//	スレーブサーバがレプリケーションを開始する時に一度だけ実行される
	//
	//	このリクエストにはセッションが必要
	//	セッション取得時にユーザ認証されるので、レプリケーションできるか
	//	どうかは、セッションの認証で判断している
	//	すでにセッションが取得できている状態でこのメソッドは呼ばれるので、
	//	このメソッドは指定したデータベースのIDを返すだけ
	//
	//	再接続時には、StartReplication -> TransferLogicalLog が実行される
	
	// セッションIDを受け取る
	ID iSessionID = recvSessionID();

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	// セッションを得る
	AutoSession session(m_cInstanceManager, iSessionID,
						Statement::ObjectType::QueryExpression);
	
	// データベース名からデータベースIDを得る
	Trans::AutoTransaction pTrans(Trans::Transaction::attach());
	pTrans->begin(Schema::ObjectID::SystemTable,
				  Trans::Transaction::Category::ReadWrite);
	Schema::ObjectID::Value iDatabaseID
		= Schema::Database::getID(session->getDatabaseName(), *pTrans);

	if (iDatabaseID == Schema::ObjectID::Invalid)
	{
		pTrans->rollback();
		// データベースが存在しない
		_SYDNEY_THROW1(Exception::DatabaseNotFound, session->getDatabaseName());
	}
	
	pTrans->commit();

	// データベースIDを返す(UInt32)
	Common::UnsignedIntegerData v(iDatabaseID);
	m_pConnection->writeObject(&v);
}

//
//	FUNCTION private
//	Server::Worker::transferLogicalLog -- スレーブに論理ログを転送する
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
Worker::transferLogicalLog()
{
	// データベースIDを受け取る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::UnsignedIntegerData* pDatabaseID
		= dynamic_cast<Common::UnsignedIntegerData*>(pObject0.get());
	if (pDatabaseID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);
	Schema::ObjectID::Value databaseID
		= static_cast<Schema::ObjectID::Value>(pDatabaseID->getValue());

	// マスター最終LSNを受け取る
	//
	//【注意】
	// このLSNまでスレーブが受け取っていて、この次のログから反映する必要がある
	
	ModAutoPointer<Common::Externalizable> pObject1
		= m_pConnection->readObject();
	Common::UnsignedInteger64Data* pLSN
		= dynamic_cast<Common::UnsignedInteger64Data*>(pObject1.get());
	if (pLSN == 0)
		_SYDNEY_THROW0(Exception::BadArgument);
	Trans::Log::LSN lastLSN
		= static_cast<Trans::Log::LSN>(pLSN->getValue());

	// スレーブのホスト名を受け取る
	ModAutoPointer<Common::Externalizable> pObject2
		= m_pConnection->readObject();
	Common::StringData* pSlaveHostName
		= dynamic_cast<Common::StringData*>(pObject2.get());

	// スレーブポート番号を受け取る
	ModAutoPointer<Common::Externalizable> pObject3
		= m_pConnection->readObject();
	Common::IntegerData* pSlavePortNumber
		= dynamic_cast<Common::IntegerData*>(pObject3.get());

	// スレーブデータベース名を受け取る
	ModAutoPointer<Common::Externalizable> pObject4
		= m_pConnection->readObject();
	Common::StringData* pSlaveDatabaseName
		= dynamic_cast<Common::StringData*>(pObject4.get());

	SydMessage << "Slave Server Connecting... "
			   << pSlaveHostName->getValue() << ":"
			   << pSlavePortNumber->getValue() << "/"
			   << pSlaveDatabaseName->getValue() << ModEndl;

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	//
	// セッションに結びついていないトランザクションを開始する
	//
	// このトランザクションはデータベースファイルを変更することはないので、
	// 版管理を利用した読み取り専用トランザクションとする
	//
	Trans::Transaction::Mode mode(
		Trans::Transaction::Category::ReadOnly,
		Trans::Transaction::IsolationLevel::Serializable,
		Boolean::True);
	Trans::AutoTransaction pTransaction(Trans::Transaction::attach());
	pTransaction->begin(databaseID, mode);

	// データベースを取得する
	
	Schema::Database* pDatabase = Schema::Database::getLocked(
		*pTransaction,
		databaseID,
		Lock::Name::Category::Tuple,
		Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple,
		Schema::Hold::Operation::ReadForImport);

	if (pDatabase == 0)
	{
		// データベースが破棄されている

		ModUnicodeOstrStream str;
		str << "DatabaseID: " << databaseID;
		
		_SYDNEY_THROW1(Exception::DatabaseNotFound,
					   str.getString());
	}
	
	// キャッシュが破棄されないようにopenする
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	//	論理ログの転送は二段階で行う
	//
	//	ファーストステップ
	//	論理ログを１つづつロードし、最後のログまで転送する
	//	論理ログをロードしている間だけログファイルをロックする
	//
	//	セカンドステップ
	//	最後のログまで転送して、スレーブとのコネクションをログファイルに設定する
	//	セカンドステップの間論理ログはロックしっぱなしにする

	// 論理ログをattachする
	Trans::Log::AutoFile logFile(pDatabase->getLogFile());

	Trans::Log::LSN lsn = lastLSN;

	// 最終のLSNをチェックする
	{
		Trans::AutoLatch latch(*pTransaction, logFile->getLockName());
		Trans::Log::LSN tmp = logFile->getLastLSN();
		if (lastLSN == tmp)
		{
			// 現時点では、
			// マスターには転送するべき論理ログはない
			
			lsn = Trans::Log::IllegalLSN;
		}
		else if (lastLSN < tmp)
		{
			// この次の論理ログから転送すればいい
			
			lsn = logFile->getNextLSN(lsn);
			if (lsn == Trans::Log::IllegalLSN)
			{
				// 必要な論理ログは消されてしまっている
				_SYDNEY_THROW0(Exception::LogItemNotFound);
			}
		}
		else
		{
			// スレーブ側の方が大きい
			// LSNの同期がおかしいので、エラーとする

			_SYDNEY_THROW0(Exception::LogItemNotFound);
		}
	}

	//	ここまできたら接続成功とする
	//	成功のステータスを送る

	sendStatus(Common::Status::Success);
	

	// 論理ログファイルからスレーブとのキューを得る

	Trans::Log::File::Queue* queue
		= logFile->getQueue(pSlaveHostName->getValue(),
							pSlavePortNumber->getValue());

	//
	// ファーストステップ
	// ここでは、論理ログのロックは最小限にする
	//
	while (lsn != Trans::Log::IllegalLSN)
	{
		// 最後に読み込んだLSNを覚えておく
		lastLSN = lsn;

		// 論理ログを得る
		ModAutoPointer<const Trans::Log::Data> data;
		
		{
			Trans::AutoLatch latch(*pTransaction, logFile->getLockName());
			
			data = logFile->load(lsn);
			lsn = logFile->getNextLSN(lsn);
		}
		
		if (!data.isOwner())
			// 最後まで読み出した
			break;

		// 論理ログをキューに書き込み
		sendLogicalLog(queue, databaseID, data, lastLSN);
	}

	//
	//	セカンドステップ
	//	最後に、論理ログをロックして、転送する
	//
	{
		// ここに来るまでに、論理ログが追加されているかもしれない
		// そのため、ファーストステップと同じことを行う
		
		Trans::AutoLatch latch(*pTransaction, logFile->getLockName());

		if (lastLSN >= logFile->getLastLSN())
		{
			// マスターには転送すべき論理ログはない
			
			lsn = Trans::Log::IllegalLSN;
		}
		else
		{
			// 転送すべき論理ログがあるので、
			// 次の論理ログから転送する

			lsn = logFile->getNextLSN(lastLSN);
			if (lsn == Trans::Log::IllegalLSN)
			{
				// 上でチェックしているので、通常ありえない
				
				_SYDNEY_THROW0(Exception::LogItemNotFound);
			}
		}

		while (lsn != Trans::Log::IllegalLSN)
		{
			ModAutoPointer<const Trans::Log::Data> data;

			// 論理ログを得る
			data = logFile->load(lsn);
			lastLSN = lsn;
			lsn = logFile->getNextLSN(lsn);

			if (!data.isOwner())
				break;

			// 論理ログを転送する
			sendLogicalLog(queue, databaseID, data, lastLSN);
		}

		// ログファイルにキューを設定する
		logFile->setQueue(pSlaveHostName->getValue(),
						  pSlavePortNumber->getValue(),
						  queue);
	}
	
	SydMessage << "Slave Server Connected. " << ModEndl;
}

//
//	FUNCTION private
//	Server::Worker::stopTransferLogicalLog -- スレーブへの転送を停止する
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
Worker::stopTransferLogicalLog()
{
	// データベースIDを受け取る
	ModAutoPointer<Common::Externalizable> pObject0
		= m_pConnection->readObject();
	Common::UnsignedIntegerData* pDatabaseID
		= dynamic_cast<Common::UnsignedIntegerData*>(pObject0.get());
	if (pDatabaseID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);
	Schema::ObjectID::Value databaseID
		= static_cast<Schema::ObjectID::Value>(pDatabaseID->getValue());

	// スレーブのホスト名を受け取る
	ModAutoPointer<Common::Externalizable> pObject2
		= m_pConnection->readObject();
	Common::StringData* pSlaveHostName
		= dynamic_cast<Common::StringData*>(pObject2.get());

	// スレーブポート番号を受け取る
	ModAutoPointer<Common::Externalizable> pObject3
		= m_pConnection->readObject();
	Common::IntegerData* pSlavePortNumber
		= dynamic_cast<Common::IntegerData*>(pObject3.get());

	// スレーブデータベース名を受け取る
	ModAutoPointer<Common::Externalizable> pObject4
		= m_pConnection->readObject();
	Common::StringData* pSlaveDatabaseName
		= dynamic_cast<Common::StringData*>(pObject4.get());

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	//
	// セッションに結びついていないトランザクションを開始する
	//
	// このトランザクションはデータベースファイルを変更することはないので、
	// 版管理を利用した読み取り専用トランザクションとする
	//
	Trans::Transaction::Mode mode(
		Trans::Transaction::Category::ReadOnly,
		Trans::Transaction::IsolationLevel::Serializable,
		Boolean::True);
	Trans::AutoTransaction pTransaction(Trans::Transaction::attach());
	pTransaction->begin(databaseID, mode);

	// データベースを取得する
	
	Schema::Database* pDatabase = Schema::Database::getLocked(
		*pTransaction,
		databaseID,
		Lock::Name::Category::Tuple,
		Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple,
		Schema::Hold::Operation::ReadForImport);

	if (pDatabase == 0)
	{
		// データベースが破棄されている

		ModUnicodeOstrStream str;
		str << "DatabaseID: " << databaseID;
		
		_SYDNEY_THROW1(Exception::DatabaseNotFound,
					   str.getString());
	}
	
	// キャッシュが破棄されないようにopenする
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	//	論理ログの転送を停止する

	// 論理ログをattachする
	Trans::Log::AutoFile logFile(pDatabase->getLogFile());

	// 停止する
	logFile->stopTransferLog(pSlaveHostName->getValue(),
							 pSlavePortNumber->getValue());

	SydMessage << "Slave Server Disconnected: "
			   << pSlaveHostName->getValue() << ":"
			   << pSlavePortNumber->getValue() << "/"
			   << pSlaveDatabaseName->getValue() << ModEndl;
}

//
//	FUNCTION private
//	Server::Worker::startReplication -- レプリケーションを開始する
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
Worker::startReplication()
{
	// スレーブのホスト名を受け取る
	ModAutoPointer<Common::Externalizable> pObject2
		= m_pConnection->readObject();
	Common::StringData* pSlaveHostName
		= dynamic_cast<Common::StringData*>(pObject2.get());

	// スレーブポート番号を受け取る
	ModAutoPointer<Common::Externalizable> pObject3
		= m_pConnection->readObject();
	Common::IntegerData* pSlavePortNumber
		= dynamic_cast<Common::IntegerData*>(pObject3.get());

	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	Communication::Connection* pConnection = 0;
	
	{
		Os::AutoCriticalSection cAuto(m_cCriticalSection);
		pConnection = m_pConnection.release();
		m_pConnection = 0;
	}

	//【注意】	ここはスレーブ側との通信路の確立のみ
	//			この pConnection でレプリケーション用のデータを送信する
	//			スレーブ側では、この成功ステータスを受け取ってから、
	//			データベースのレプリケーションの依頼をする
	//			そのため、成功ステータスを送る前にレプリケーション用の
	//			データが送信されることはない

	// ログを転送するスレッドを起動する
	Trans::Log::File::startReplication(pSlaveHostName->getValue(),
									   pSlavePortNumber->getValue(),
									   pConnection);

	//	成功のステータスを送る
	Common::Status cStatus(Common::Status::Success);
	pConnection->writeObject(&cStatus);
	pConnection->flush();

	SydMessage << "Slave Server Connected: "
			   << pSlaveHostName->getValue() << ":"
			   << pSlavePortNumber->getValue() << ModEndl;
}

// FUNCTION private
//	Server::Worker::queryProductVersion -- query product version
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
queryProductVersion()
{
	// send kernel version
	Common::StringData cVersion(_TRMEISTER_U_STRING(SYD_KERNEL_VERSION));
	m_pConnection->writeObject(&cVersion);
}

//
//	FUNCTION private
//	Server::Worker::sendStatus -- クライアントに実行ステータスを送る
//
//	NOTES
//
//	ARGUMENTS
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
Worker::sendStatus(const Common::Status& cStatus_,
				   const Exception::Object& cException_)
{
	Communication::ServerConnection* pConnection;

	{
		Os::AutoCriticalSection cAuto(m_cCriticalSection);
		pConnection = m_pConnection.release();
		m_pConnection = 0;
		if (pConnection == 0)
		{
			return;
		}
	}

	try
	{
		//コネクションを再利用するためにプールする
		pConnection->clearCancel();
		m_cInstanceManager.pushConnection(pConnection);
		int masterID = 0;

		{
			Os::AutoCriticalSection cAuto(pConnection->getLockObject());
			//クライアントにステータスを送る
			masterID = pConnection->getMasterID();
			if (masterID >= Communication::Protocol::Version3 && cStatus_.getStatus() == Common::Status::Error)
			{
				Common::ErrorLevel level(cException_.isUserLevel() ?
										 Common::ErrorLevel::User :
										 Common::ErrorLevel::System);
				pConnection->writeObject(&level);
			}
				
			Connection::sendStatus(pConnection, cStatus_, cException_);
		}

		if (masterID >= Communication::Protocol::Version3)
		{
			if (cStatus_.getStatus() == Common::Status::Success
				|| cStatus_.getStatus() == Common::Status::Canceled
				|| (cStatus_.getStatus() == Common::Status::Error
					&& cException_.isUserLevel()))
			{
				// 再利用する
				pConnection = 0;
			}
		}

		if (pConnection != 0
			&& cStatus_.getStatus() != Common::Status::Success)
		{
			//再利用しない
			pConnection->close();
			m_cInstanceManager.popConnection(pConnection->getSlaveID());
			delete pConnection;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		//エラーなので、このコネクションは再利用しない
		pConnection->close();
		m_cInstanceManager.popConnection(pConnection->getSlaveID());
		delete pConnection;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Server::Worker::recvSessionID -- クライアントからSessionIDを受け取る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Server::ID
//		セッションID
//
//	EXCEPTIONS
//
ID
Worker::recvSessionID()
{
	//クライアントからSessionIDを受け取る
	ModAutoPointer<Common::Externalizable> pObject
		= m_pConnection->readObject();
	Common::IntegerData* pSessionID
		= dynamic_cast<Common::IntegerData*>(pObject.get());

	if (pSessionID == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	m_iSessionID = pSessionID->getValue();
	return m_iSessionID;
}

// FUNCTION private
//	Server::Worker::recvPassword -- recieve password
//
// NOTES
//	This method is prepared so that password communication can be encoded
//
// ARGUMENTS
//	ModUnicodeString& cstrPassword_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Worker::
recvPassword(ModUnicodeString& cstrPassword_)
{
	ModAutoPointer<Common::Externalizable> pObject
		= m_pConnection->readObject();
	Common::StringData* pPassword
		= dynamic_cast<Common::StringData*>(pObject.get());
	if (pPassword == 0)
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	cstrPassword_ = pPassword->getValue();
}

//
//	FUNCTION private
//	Server::Worker::sendLogicalLog -- 論理ログを送る
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::File::Queue* pQueue_
//		コネクション
//	Schema::ObjectID::Value uiDatabaseID_,
//		データベースID
//	const Trans::Log::Data* pData_
//		論理ログ
//	Trans::Log::LSN lsn_
//		送る論理ログのマスター側のLSN
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Worker::sendLogicalLog(Trans::Log::File::Queue* pQueue_,
					   Schema::ObjectID::Value uiDatabaseID_,
					   const Trans::Log::Data* pData_,
					   Trans::Log::LSN lsn_)
{
	// サイズを調べる
	ModSerialSize serialSize;
	Common::OutputArchive tmp(serialSize);
	tmp.writeObject(pData_);
	ModSize size = tmp.getSize();

	// 領域を確保する
	void* p = Os::Memory::allocate(size);

	try
	{
		// 確保した領域に符号化する

		ModMemory memory(p, size);
		Common::OutputArchive(memory).writeObject(pData_);

		// キューに書き込む

		pQueue_->pushBack(uiDatabaseID_, pData_->getCategory(),
						  p, size, lsn_);
	}
	catch (...)
	{
		Os::Memory::free(p);
		_SYDNEY_RETHROW;
	}

	// 領域を解放する
	Os::Memory::free(p);
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
