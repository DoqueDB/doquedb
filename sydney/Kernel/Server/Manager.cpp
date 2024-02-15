// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
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
#include "SyKernelVersion.h"

#include "Server/Manager.h"
#include "Server/Session.h"
#include "Server/Worker.h"
#include "Server/Connection.h"
#include "Server/Parameter.h"
#include "Server/PasswordFile.h"
#include "Server/FakeError.h"
#include "Server/UserList.h"
#include "Admin/Manager.h"
#include "Admin/Replicator.h"
#include "Admin/Restart.h"
#include "Buffer/Manager.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/Daemon.h"
#include "Common/Assert.h"
#include "Common/Manager.h"
#include "Common/Message.h"
#include "Common/ExceptionMessage.h"
#include "Common/Request.h"
#include "Common/Status.h"
#include "Common/StringData.h"
#include "Communication/AuthorizeMode.h"
#include "Communication/ConnectionSupplier.h"
#include "Communication/Protocol.h"
#include "Communication/ServerConnection.h"
#include "Communication/ConnectionSlaveID.h"
#include "Communication/User.h"
#include "Execution/Manager.h"
#include "Lock/Manager.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalLog/Manager.h"
#include "Opt/Manager.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Manager.h"
#include "PhysicalFile/Manager.h"
#include "Plan/Manager.h"
#include "Schema/Manager.h"
#include "Schema/ObjectID.h"
#include "Statement/Object.h"
#include "Trans/Manager.h"
#include "Trans/Transaction.h"
#include "Trans/AutoTransaction.h"
#include "Utility/Manager.h"
#include "Utility/UNA.h"
#include "Version/Manager.h"
#include "Exception/AuthorizationFailed.h"
#include "Exception/Object.h"
#include "Exception/ServerNotAvailable.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/ModLibraryError.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Exception/GoingShutdown.h"
#include "Exception/BadArgument.h"
#include "Exception/UnknownRequest.h"
#include "Exception/BadPasswordFile.h"
#include "Exception/UserNotFound.h"
#include "Exception/UserRequired.h"
#include "Exception/PrivilegeNotAllowed.h"

#include "DExecution/Manager.h"
#include "DPlan/Manager.h"
#include "DServer/Manager.h"

#include "ModAutoPointer.h"
#include "ModVersion.h"

#include <new>

#ifndef SYD_OS_WINDOWS
#include <sys/mman.h>
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#define _SERVER_MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{
	//	VARIABLE
	//	$$$::_cParameterMiniDumpPath --
	//		ミニダンプを生成するディレクトリの絶対パス名
	//
	//	NOTES

	ParameterString	_cParameterMiniDumpPath("Server_MiniDumpPath", "");

	//	VARIABLE
	//	$$$::_cParameterMiniDumpPermission --
	//		ミニダンプを記録するファイルの許可モード
	//
	//	NOTES

	ParameterInteger _cParameterMiniDumpPermission("Server_MiniDumpPermission",
#ifdef DEBUG
												   0660,
#else
												   0600,
#endif
												   false);

	//
	//	VARIABLE local
	//	_$$::_cParamterMasterID -- マスターID
	//
	//	NOTES
	//	0 .. v14.0 and before
	//	1 .. prepareStatement2(early version)
	//	2 .. prepareStatement2
	//	3 .. hasMoreData, execute(JDBC)
	//	4 .. product version
	ParameterInteger
	_cParameterMasterID("Server_MasterID",
						Communication::Protocol::CurrentVersion,
						false);

	//
	//	VARIABLE local
	//	_$$::_cParameterTrace -- トレースログを出力するかどうか
	//
	ParameterMessage _cParameterTrace("Server_TraceLog");

	//
	//	FUNCTION local
	//	_$$::_traceLog
	//
	void _traceLog(int line, const char* message)
	{
		if (_cParameterTrace.isOutput())
		{
			Common::MessageStreamSelection::getInstance(
				moduleName, srcFile, line,
				_cParameterTrace.getParameterName(),
				Common::MessageStreamBuffer::LEVEL_DEBUG)
					<< message << ModEndl;
		}
	}

#define _TRACE(message)	_traceLog(__LINE__, message)

	//	VARIABLE
	//	$$$::_cParameterPasswordFilePath --
	//		Path setting of password file
	//
	//	NOTES
	//	empty string will cause error

	ParameterString	_cParameterPasswordFilePath("Server_PasswordFilePath", "",
												false);

#ifndef SYD_OS_WINDOWS
	//	VARIABLE
	//	$$$::_cParameterLockMemory --
	//		lock the address space of a process
	//
	//	NOTES
	//	if this parameter is true, execute by root

	ParameterBoolean _cParameterLockMemory("Server_LockMemory", false, false);
#endif

	namespace _UserList
	{
		// password file
		PasswordFile* _pPasswordFile;
		// user list
		UserList* _pUserList;
		// latch
		Os::CriticalSection _latch;

		// initialize user list
		void initialize();
		void terminate();

		// get password file path
		bool getPasswordFilePath(ModUnicodeString& cstrPath_);

	} // namespace UserList
}

// FUNCTION local
//	$$$::_UserList::initialize -- initialize user list
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
_UserList::
initialize()
{
	Os::AutoCriticalSection l(_latch);

	ModUnicodeString cstrPath;
	if (getPasswordFilePath(cstrPath)) {
		_pPasswordFile = new PasswordFile(cstrPath);
		_pUserList = _pPasswordFile->getUserList();
	} else {
		// ignore
		SydInfoMessage << "Warning: user management is disabled." << ModEndl;
	}
}

// FUNCTION local
//	$$$::_UserList::terminate -- terminate user list
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
_UserList::
terminate()
{
	Os::AutoCriticalSection l(_latch);

	if (_pPasswordFile) delete _pPasswordFile, _pPasswordFile = 0;
	if (_pUserList) delete _pUserList, _pUserList = 0;
}

// FUNCTION local
//	$$$::_UserList::getPasswordFilePath -- get password file path
//
// NOTES
//
// ARGUMENTS
//	ModUnicodeString& cstrPath_
//	
// RETURN
//	bool	true if path is successfully read,
//			false if user management is disabled
//
// EXCEPTIONS

// get password file path
bool
_UserList::
getPasswordFilePath(ModUnicodeString& cstrPath_)
{
	cstrPath_ = _cParameterPasswordFilePath.get();
	if (cstrPath_.getLength() > 0) {
		// if password file setting is magic string, it is ignored
		// [NOTES]
		// This string is hard coded because of security reason
		if (cstrPath_.getLength() == 10
			&& cstrPath_[5] == 's'
			&& cstrPath_[6] == 'w'
			&& cstrPath_[0] == 'N'
			&& cstrPath_[9] == 'd'
			&& cstrPath_[3] == 'a'
			&& cstrPath_[7] == 'o'
                        && cstrPath_[1] == 'o'
                        && cstrPath_[2] == 'P'
                        && cstrPath_[8] == 'r'
                        && cstrPath_[4] == 's') {  
			return false;
		} else {
			return true;
		}
	} else {
		// empty password file path is invalid
		_SYDNEY_THROW0(Exception::BadPasswordFile);
	}
}

//
//	VARIABLE static
//	Server::Manager::m_bAvailable
//
//	NOTES
//	Sydney全体が利用可能かどうか
//
bool
Server::Manager::m_bAvailable = true;

//
//	VARIABLE static
//	Server::Manager::m_bNotAvailableLogged
//
//	NOTES
//	Availabilityのログを出したか
//
bool
Server::Manager::m_bNotAvailableLogged = false;

//
//	VARIABLE static
//	Server::Manager::m_cAvailabilityCriticalSection
//
//	NOTES
//	Server::Manager::m_bAvailableの排他制御用のクリティカルセクション
//
Os::CriticalSection
Server::Manager::m_cAvailabilityCriticalSection;

//
//	FUNCTION public
//	Server::Manager::Manager -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Server::Type eType_
//		サーバのタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Manager::Manager(Server::Type eType_)
	: m_eType(eType_), m_iInitialized(0), m_pShutdownConnection(0)
{
}

//
//	FUNCTION public
//	Server::Manager::~Manager -- デストラクタ
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
Manager::~Manager()
{
}

//
//	FUNCTION public
//	Server::Manager::initialize -- 初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	bool bInstall_
//		Sydneyのインストールのための初期化か(default false)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::initialize(bool bInstall_, const ModUnicodeString& regPath)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	int iRetry = 1;
retry:
	if (m_iInitialized++ == 0)
	{
		try
		{
			//利用可能性のチェック
			if (Manager::isAvailable() == false)
			{
				//利用できないので、例外を投げる
				_SYDNEY_THROW0(Exception::ServerNotAvailable);
			}

#ifdef _OPENMP			
			// 入れ子になった並列化を有効にする
			omp_set_nested(true);
#endif

			// 共通ライブラリの初期化
		
			Common::Manager::initialize(regPath);
			_TRACE("Common::Manager::initialize");

			// Utility の初期化

			Utility::Manager::initialize();
			_TRACE("Utility::Manager::initialize");
			
			// Os の初期化
			const Os::Path& path = _cParameterMiniDumpPath.get();
			if (path.getLength())
				Os::Manager::initialize(path,
										_cParameterMiniDumpPermission.get());
			else
				Os::Manager::initialize();
			_TRACE("Os::Manager::initialize");
		}
		catch (...)
		{
			// Commonが初期化されていないので、メッセージは出力できない

			--m_iInitialized;	// terminateがよばれないように...
			//利用可能性を設定する
			setAvailability(false);
			throw;
		}

		try
		{
#ifndef SYD_OS_WINDOWS
			// メモリーをロックするかどうか
			if (_cParameterLockMemory.get() == true)
			{
				if (::mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
				{
					// エラー
					SydErrorMessage << "System call 'mlockall' is failed. "
									<< "errno=" << errno << ModEndl;
				}
			}
#endif
			
			;_SERVER_FAKE_ERROR(Manager::initialize);

			//コネクションの初期化
			Communication::ConnectionSupplier::initialize();
			_TRACE("Communication::ConnectionSupplier::initialize");

			//ステートメントの初期化
			Statement::Object::initialize();
			_TRACE("Statement::Object::initialize");
			// ロックマネージャーの初期化
			Lock::Manager::initialize();
			_TRACE("Lock::Manager::initialize");
			// バッファマネージャーの初期化
			Buffer::Manager::initialize();
			_TRACE("Buffer::Manager::initialize");
			// 論理ログマネージャーの初期化
			LogicalLog::Manager::initialize();
			_TRACE("LogicalLog::Manager::initialize");

			if (bInstall_)
			{

				// トランザクションマネージャー関連のインストールを行う

				Trans::Manager::install();
				_TRACE("Trans::Manager::install");
			}

			// トランザクションマネージャーの初期化

			Trans::Manager::initialize();
			_TRACE("Trans::Manager::initialize");

			// チェックポイント処理マネージャーの初期化
			//
			//【注意】	チェックポイントスレッドは開始されるが、
			//			チェックポイント処理は無効化されている

			Checkpoint::Manager::initialize();
			_TRACE("Checkpoint::Manager::initialize");

			// 版管理マネージャーの初期化
			Version::Manager::initialize();
			_TRACE("Version::Manager::initialize");
			// 物理ファイルマネージャーの初期化
			PhysicalFile::Manager::initialize();
			_TRACE("PhysicalFile::Manager::initialize");
			//ファイルドライバマネージャの初期化
			LogicalFile::FileDriverManager::initialize();
			_TRACE("LogicalFile::FileDriverManager::initialize");

			if (bInstall_)
			{
				// スキーマ関連のインストールを行う

				Trans::AutoTransaction	trans(Trans::Transaction::attach());
				trans->begin(Schema::ObjectID::SystemTable,
							 Trans::Transaction::Category::ReadWrite);
				Schema::Manager::install(*trans);
				trans->commit();
				_TRACE("Schema::Manager::install");
			}

			// スキーママネージャーの初期化

			Schema::Manager::initialize();
			_TRACE("Schema::Manager::initialize");

			//プランモジュールの初期化
			Plan::Manager::initialize();
			_TRACE("Plan::Manager::initialize");
			//Optモジュールの初期化
			Opt::Manager::initialize();
			_TRACE("Opt::Manager::initialize");
			//Executionモジュールの初期化
			Execution::Manager::initialize();
			_TRACE("Execution::Manager::initialize");
			//Adminモジュールの初期化
			Admin::Manager::initialize();

			//分散マネージャー関連の初期化
			DServer::Manager::initialize();
			_TRACE("DServer::Manager::initialize");
			DPlan::Manager::initialize();
			_TRACE("DPlan::Manager::initialize");
			DExecution::Manager::initialize();
			_TRACE("DExecution::Manager::initialize");

			// 必要があれば、データベースの復旧を行う

			if (!bInstall_ && Admin::Restart::recover(iRetry--)) {

				// データベースを復旧したので、
				// 一度システムを終了することにより、
				// 復旧後のデータベースを永続化する

				SydMessage << "Database recovered" << ModEndl;
				stop();
				terminate();
				goto retry;
			}

			// スキーマ情報のキャッシュに関する初期化
			Schema::Manager::ObjectSnapshot::initialize();

			// initialize user list
			_UserList::initialize();

			// チェックポイント処理の実行を解禁する

			Checkpoint::Daemon::enable(Checkpoint::Daemon::Category::Executor);
			_TRACE("Checkpoint::Daemon::enable");

			if (!bInstall_)
			{
				//クライアントからの接続を待つスレッドを起動する
				Communication::ConnectionSupplier::create(
					(m_eType == Server::InProcess) ?
					Communication::ConnectionSupplier::Memory :
					Communication::ConnectionSupplier::Socket);
				_TRACE("Communication::ConnectionSupplier::create");

				// レプリケートデータベースを開始する
				Admin::Replicator::initialize();
				_TRACE("Admin::Restart::initialize");
			}
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			SydErrorMessage << "Server initialize failed" << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			--m_iInitialized;	// terminateしないようにする
			throw;
		}
		catch (ModException& e)
		{
			SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
			SydErrorMessage << "Server initialize failed" << ModEndl;
			Common::Thread::resetErrorCondition();
			//利用可能性を設定する
			setAvailability(false);
			--m_iInitialized;	// terminateしないようにする
			_SYDNEY_THROW1(Exception::ModLibraryError, e);
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			SydErrorMessage << "Server initialize failed" << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			--m_iInitialized;	// terminateしないようにする
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected Error" << ModEndl;
			SydErrorMessage << "Server initialize failed" << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			--m_iInitialized;	// terminateしないようにする
			_SYDNEY_THROW0(Exception::Unexpected);
		}
#endif

		if (bInstall_)
			SydMessage << "Server installed" << ModEndl;
		else
		{
			SydMessage << "Server initialized ("
					   << "kernel v" << SYD_KERNEL_VERSION
					   << "; una v" << Utility::Una::Manager::getVersion()
					   << "; mod v" << ModVersion::getVersion() << ")" << ModEndl;
		}
	}
}

//
//	FUNCTION public
//	Server::Manager::terminate -- 後処理を行う
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
Manager::terminate()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_iInitialized == 1)
	{
		m_iInitialized--;
		
		try
		{
			// レプリケートデータベースを終了する
			Admin::Replicator::terminate();
			_TRACE("Admin::Restart::terminate");
			
			//コネクションの終了処理
			Communication::ConnectionSupplier::terminate();
			_TRACE("Communication::ConnectionSupplier::terminate");

			// destroy user list
			_UserList::terminate();

			// バージョンファイル同期スレッドを終了する

			Checkpoint::Daemon::join(
				Checkpoint::Daemon::Category::FileSynchronizer);
			_TRACE("Checkpoint::Daemon::join");

			// トランザクションマネージャーの終了処理の準備
			Trans::Manager::prepareTermination();
			_TRACE("Trans::Manager::prepareTermination");
			//ファイルドライバマネージャの終了処理の準備
			LogicalFile::FileDriverManager::prepareTerminate();
			_TRACE("LogicalFile::FileDriverManager::prepareTerminate");

			// チェックポイントスレッドを終了する

			Checkpoint::Daemon::join(Checkpoint::Daemon::Category::Executor);
			_TRACE("Checkpoint::Daemon::join");

			//分散マネージャー関連の終了処理
			DServer::Manager::terminate();
			_TRACE("DServer::Manager::terminate");
			DPlan::Manager::terminate();
			_TRACE("DPlan::Manager::terminate");
			DExecution::Manager::terminate();
			_TRACE("DExecution::Manager::terminate");

			// Adminモジュールの終了処理
			Admin::Manager::terminate();
			// Executionモジュールの終了処理
			Execution::Manager::terminate();
			_TRACE("Execution::Manager::terminate");
			// Optモジュールの終了処理
			Opt::Manager::terminate();
			_TRACE("Opt::Manager::terminate");
			// プランモジュールの終了処理
			Plan::Manager::terminate();
			_TRACE("Plan::Manager::terminate");
			//スキーマの終了処理
			Schema::Manager::terminate();
			_TRACE("Schema::Manager::terminate");
			//ファイルドライバマネージャの終了処理
			LogicalFile::FileDriverManager::terminate();
			_TRACE("LogicalFile::FileDriverManager::terminate");
			// 物理ファイルマネージャーの終了処理
			PhysicalFile::Manager::terminate();
			_TRACE("PhysicalFile::Manager::terminate");
			// 版管理マネージャーの終了処理
			Version::Manager::terminate();
			_TRACE("Version::Manager::terminate");
			// チェックポイント処理マネージャーの終了処理
			Checkpoint::Manager::terminate();
			_TRACE("Checkpoint::Manager::terminate");
			// トランザクションマネージャーの終了処理
			Trans::Manager::terminate();
			_TRACE("Trans::Manager::terminate");
			// 論理ログマネージャーの終了処理
			LogicalLog::Manager::terminate();
			_TRACE("LogicalLog::Manager::terminate");
			// バッファマネージャーの終了処理
			Buffer::Manager::terminate();
			_TRACE("Buffer::Manager::terminate");
			// ロックマネージャーの終了処理
			Lock::Manager::terminate();
			_TRACE("Lock::Manager::terminate");
			//ステートメントの終了処理
			Statement::Object::terminate();
			_TRACE("Statement::Object::terminate");

			;_SERVER_FAKE_ERROR(Manager::terminate);

		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			if (m_pShutdownConnection)
			{
				//必要ならここでShutdownを要求したクライアントにOKを送る
				Connection::sendStatus(m_pShutdownConnection, Common::Status::Error, e);
				//クローズ
				m_pShutdownConnection->close();
				delete m_pShutdownConnection, m_pShutdownConnection = 0;
			}
			throw;
		}
		catch (ModException& e)
		{
			SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			if (m_pShutdownConnection)
			{
				//必要ならここでShutdownを要求したクライアントにOKを送る
				Connection::sendStatus(
					m_pShutdownConnection, Common::Status::Error,
					_SERVER_MOD_EXCEPTION(e));
				//クローズ
				m_pShutdownConnection->close();
				delete m_pShutdownConnection, m_pShutdownConnection = 0;
			}
			Common::Thread::resetErrorCondition();
			_SYDNEY_THROW1(Exception::ModLibraryError, e);
		}
#ifndef NO_CATCH_ALL
		catch (std::bad_alloc& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			if (m_pShutdownConnection)
			{
				//必要ならここでShutdownを要求したクライアントにOKを送る
				Connection::sendStatus(m_pShutdownConnection,
									   Common::Status::Error,
									   Exception::MemoryExhaust(
										   moduleName, srcFile, __LINE__));
				//クローズ
				m_pShutdownConnection->close();
				delete m_pShutdownConnection, m_pShutdownConnection = 0;
			}
			_SYDNEY_THROW0(Exception::MemoryExhaust);
		}
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			if (m_pShutdownConnection)
			{
				//必要ならここでShutdownを要求したクライアントにOKを送る
				Connection::sendStatus(m_pShutdownConnection,
									   Common::Status::Error,
									   Exception::Unexpected(
										   moduleName, srcFile, __LINE__));
				//クローズ
				m_pShutdownConnection->close();
				delete m_pShutdownConnection, m_pShutdownConnection = 0;
			}
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected Error" << ModEndl;
			//利用可能性を設定する
			setAvailability(false);
			if (m_pShutdownConnection)
			{
				//必要ならここでShutdownを要求したクライアントにOKを送る
				Connection::sendStatus(m_pShutdownConnection, Common::Status::Error,
					Exception::Unexpected(moduleName, srcFile, __LINE__));
				//クローズ
				m_pShutdownConnection->close();
				delete m_pShutdownConnection, m_pShutdownConnection = 0;
			}
			_SYDNEY_THROW0(Exception::Unexpected);
		}
#endif

		if (m_pShutdownConnection)
		{
			//必要ならここでShutdownを要求したクライアントにOKを送る
			Connection::sendStatus(m_pShutdownConnection,
								   Common::Status::Success);
			//クローズ
			m_pShutdownConnection->close();
			delete m_pShutdownConnection, m_pShutdownConnection = 0;
		}
		
		SydMessage << "Server terminated" << ModEndl;

		//以下のteminateを実行すると、Communication::Connection とか
		//Messageとかがが使用できなくなる

		try
		{
			// Utility の後処理
			
			Utility::Manager::terminate();

			// 共通ライブラリの後処理

			Common::Manager::terminate();

			// Os の終了処理

			Os::Manager::terminate();
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			//利用可能性を設定する
			setAvailability(false);
			throw;
		}

		//利用可能性のクリア
		Manager::clearAvailability();
		Session::clearAvailability();
	}
}

//
//	FUNCTION public
//	Server::Manager::stop -- スレッドに停止要求する
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
Manager::stop()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//ConnectionSupplierをabortすると、サーバスレッドは停止する
	Communication::ConnectionSupplier::abort();
	_TRACE("Communication::ConnectionSupplier::abort");
}

//
//	FUNCTION public
//	Server::Manager::pushConnection -- Connectionを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Connection* pServerConnection_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::pushConnection(Connection* pServerConnection_)
{
	m_cConnectionManager.pushThread(pServerConnection_);
}

//
//	FUNCTION public
//	Server::Manager::reportEndConnection -- Connectionが終了したことをマネージャに知らせる
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iConnectionID_
//		ConnectionID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::reportEndConnection(ID iConnectionID_)
{
	m_cConnectionManager.pushJoinThread(iConnectionID_);
}

//
//	FUNCTION public
//	Server::Manager::pushWorker -- Workerを追加する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Worker* pWorker_
//		追加するワーカ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::pushWorker(Worker* pWorker_)
{
	m_cWorkerManager.pushThread(pWorker_);
}

//
//	FUNCTION public
//	Server::Manager::cancelWorker -- Workerに中断を要求する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iWorkerID_
//		WorkerID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::cancelWorker(ID iWorkerID_)
{
	m_cWorkerManager.stopThread(iWorkerID_);
}

//
//	FUNCTION public
//	Server::Manager::reportEndWorker -- WorkerをJoinするリクエストをWorkerManagerに送る
//
//	NOTES
//
//	ARGUMENTS
//	ID iWorkerID_
//		WorkerID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::reportEndWorker(ID iWorkerID_)
{
	m_cWorkerManager.pushJoinThread(iWorkerID_);
}


//
//	FUNCTION public
//	Server::Manager::popWorker --
//				   Workerにstopを要求し、管理対象から外す.
//				   本メソッドの呼び出し元でJoin()の呼び出しを行ってください。
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iWorkerID__
//
//
//	RETURN
//	Server::Worker
//
//	EXCEPTIONS
//
Worker*
Manager::popWorker(ID iWorkerID_)
{

	return static_cast<Worker*>(m_cWorkerManager.popThread(iWorkerID_));
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Server::Manager::pushWorkerQueue -- 待機ワーカを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Worker* pWorker_
//		待機するワーカ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::pushWorkerQueue(Worker* pWorker_)
{
	m_cWorkerManager.pushQueue(pWorker_);
}
#endif

//
//	FUNCTION private
//	Server::Manager::runnable -- スレッドとして起動されるメソッド
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
Manager::runnable()
{
	//コネクションマネージャを起動する
	m_cConnectionManager.create();
	//ワーカマネージャを起動する
	m_cWorkerManager.create();

	try
	{
		//クライアントからのリクエストを受け付ける
		loop();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
	}

	SydMessage << "Going shutdown..." << ModEndl;
	
	//残っているすべてのワーカを終了する
	m_cWorkerManager.stopAllThread();
	m_cWorkerManager.join();
	
	SydMessage << "Worker stopped." << ModEndl;
	
	//残っているすべてのコネクションを終了する
	m_cConnectionManager.stopAllThread();
	m_cConnectionManager.join();
	
	SydMessage << "Connection stopped." << ModEndl;
}

//
//	FUNCTION private
//	Server::Manager::loop -- クライアントからのリクエストを受け付ける
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
Manager::loop()
{
	while (1)
	{
		//このループ中で作成されたConnectionは、Connection が引き取り削除する。
		ModAutoPointer<Communication::ServerConnection> pConnection
			= new Communication::ServerConnection(
				getMasterID(),
				Communication::ConnectionSlaveID::Any);

		try
		{
			//クライアントの接続を待つ
			pConnection->open();
		}
		catch (Exception::ConnectionRanOut& e)
		{
			SydErrorMessage << e << ModEndl;
			continue;
		}
		catch (Exception::GoingShutdown&)
		{
			//終了
			return;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			//その他の例外が起きたときはサーバを終了する
			stop();
			_SYDNEY_RETHROW;
		}


		try
		{
			//リクエストを読み込む
			ModAutoPointer<Common::Externalizable> pObject
				= pConnection->readObject();
			Common::Request* pRequest
				= dynamic_cast<Common::Request*>(pObject.get());

			if (pRequest == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			switch (pRequest->getCommand())
			{
			case Common::Request::BeginConnection:
				//コネクションを開始する
				beginConnection(pConnection.get());
				//正常に終了したので
				pConnection.release();
				break;
			case Common::Request::Shutdown:
				if (_UserList::_pUserList != 0) {
					// 認証が必要
					_SYDNEY_THROW0(Exception::NotSupported);
				}
				//スレッドに停止要求を行う
				stop();
				m_pShutdownConnection = pConnection.release();
				return;
			case Common::Request::Shutdown2:
				//認証する
				checkUser(pConnection.get());
				//スレッドに停止要求を行う
				stop();
				m_pShutdownConnection = pConnection.release();
				return;
			default:
				_SYDNEY_THROW1(Exception::UnknownRequest,
							   pRequest->getCommand());
			}
		}
		catch (Exception::ServerNotAvailable& e)
		{
			try
			{
				if (Manager::isNotAvailableLogged() == false) {
					// Server not availableのログは一回しか出さない
					SydErrorMessage << e << ModEndl;
					Manager::setNotAvailableLogged(true);
				}
				Connection::sendStatus(pConnection.get(),
									   Common::Status::Error, e);
			}
			catch (...){}	//例外を無視する
		}
		catch (Exception::Object& e)
		{
			try
			{
				SydErrorMessage << e << ModEndl;
				Connection::sendStatus(pConnection.get(),
									   Common::Status::Error, e);
			}
			catch (...){}	//例外を無視する
		}
		catch (ModException& e)
		{
			Common::Thread::resetErrorCondition();
			try
			{
				SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
				Connection::sendStatus(
					pConnection.get(), Common::Status::Error,
					_SERVER_MOD_EXCEPTION(e));
			}
			catch (...){}	//例外を無視する
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			try
			{
				SydErrorMessage << "Unexpected Error" << ModEndl;
				Connection::sendStatus(pConnection.get(), Common::Status::Error,
					Exception::Unexpected(moduleName, srcFile, __LINE__));
			}
			catch (...) {}	//例外を無視する
		}
#endif
	}
}

//
//	FUNCTION private
//	Sever::Manager::beginConnection -- コネクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	Communication::ServerConnection* pConnection_
//		クライアントとのコネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Manager::beginConnection(Communication::ServerConnection* pConnection_)
{
	//クライアントホスト名を受け取る
	ModAutoPointer<Common::Externalizable> pObject1 = pConnection_->readObject();
	Common::StringData* pHostName = dynamic_cast<Common::StringData*>(pObject1.get());

	if (pHostName == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	//サーバの利用可能性をチェックする
	if (Manager::isAvailable() == false)
		_SYDNEY_THROW0(Exception::ServerNotAvailable);

	;_SERVER_FAKE_ERROR(Manager::beginConnection);

	//コネクションのインスタンスを確保する
	Connection* pServerConnection = new Connection(*this, pConnection_, pHostName->getValue());

	//コネクションマネージャに登録する
	pushConnection(pServerConnection);

	//スレッドを起動する
	pServerConnection->create();
}

// FUNCTION private
//	Server::Manager::checkUser -- 認証する
//
// NOTES
//
// ARGUMENTS
//	Communication::ServerConnection* pConnection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::
checkUser(Communication::ServerConnection* pConnection_)
{
	//ユーザー名、パスワードを受け取る
	ModAutoPointer<Common::Externalizable> pObject1 = pConnection_->readObject();
	Common::StringData* pUserName = dynamic_cast<Common::StringData*>(pObject1.get());

	ModAutoPointer<Common::Externalizable> pObject2 = pConnection_->readObject();
	Common::StringData* pPassword = dynamic_cast<Common::StringData*>(pObject2.get());

	if (pUserName == 0 || pPassword == 0) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// 認証する
	UserList::Entry::Pointer pEntry = Manager::verifyPassword(pUserName->getValue(),
															  pPassword->getValue());
	if (pEntry.get() && !pEntry->isSuperUser()) {
		_SYDNEY_THROW0(Exception::PrivilegeNotAllowed);
	}
}

//
//	FUNCTION public static
//	Server::Manager::setAvailability -- 利用可能かどうか設定する
//
//	NOTES
//	
//	ARGUMENTS
//	bool bFlag_
//		利用可能な場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	bool
//		設定されていた値
//
//	EXCEPTIONS
//	なし
//
bool
Manager::setAvailability(bool bFlag_)
{
	Os::AutoCriticalSection cAuto(m_cAvailabilityCriticalSection);

	bool bPrev = m_bAvailable;
	m_bAvailable = bFlag_;

	return bPrev;
}

//
//	FUNCTION public static
//	Server::Manager::isAvailable -- 利用可能かどうか
//
//	NOTES
//	Sydney全体が利用可能かどうかを調べる
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		利用可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Manager::isAvailable()
{
	Os::AutoCriticalSection cAuto(m_cAvailabilityCriticalSection);

	return m_bAvailable;
}

//
//	FUNCTION public static
//	Server::Manager::setNotAvailableLogged -- 利用可能かどうか設定する
//
//	NOTES
//	
//	ARGUMENTS
//	bool bFlag_
//		利用可能な場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	bool
//		設定されていた値
//
//	EXCEPTIONS
//	なし
//
bool
Manager::setNotAvailableLogged(bool bFlag_)
{
	Os::AutoCriticalSection cAuto(m_cAvailabilityCriticalSection);

	bool bPrev = m_bNotAvailableLogged;
	m_bNotAvailableLogged = bFlag_;

	return bPrev;
}

//
//	FUNCTION public static
//	Server::Manager::isNotAvailableLogged -- 利用可能かどうか
//
//	NOTES
//	Sydney全体が利用可能かどうかを調べる
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		利用可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Manager::isNotAvailableLogged()
{
	Os::AutoCriticalSection cAuto(m_cAvailabilityCriticalSection);

	return m_bNotAvailableLogged;
}

// FUNCTION public
//	Server::Manager::getUserList -- get user list
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	UserList*
//
// EXCEPTIONS

//static
UserList*
Manager::
getUserList()
{
	return _UserList::_pUserList;
}

// FUNCTION public
//	Server::Manager::addUser -- add user
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Manager::
addUser(const ModUnicodeString& cstrUserName_,
		const ModUnicodeString& cstrPassword_,
		int iUserID_)
{
	// following operations are done atomically
	//  - add a user to user list
	//  - persist it to password file
	if (_UserList::_pUserList) {
		; _SYDNEY_ASSERT(_UserList::_pPasswordFile);
		_UserList::_pUserList->addUser(*_UserList::_pPasswordFile,
									   cstrUserName_, cstrPassword_, iUserID_);
	} else {
		// if the server is running under no user mode, this operation is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION public
//	Server::Manager::deleteUser -- delete user
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	int iDropBehavior_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Manager::
deleteUser(const ModUnicodeString& cstrUserName_, int iDropBehavior_)
{
	// following operations are done atomically
	//  - delete a user from user list
	//  - persist it to password file
	if (_UserList::_pUserList) {
		; _SYDNEY_ASSERT(_UserList::_pPasswordFile);

		if (iDropBehavior_ == Communication::User::DropBehavior::Cascade) {
			UserList::Entry::Pointer pEntry;
			if (!_UserList::_pUserList->get(cstrUserName_, pEntry)) {
				SydInfoMessage << "No user: " << cstrUserName_ << ModEndl;
				_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
			}
			revokeAll(pEntry->getID());
		}

		_UserList::_pUserList->deleteUser(*_UserList::_pPasswordFile,
										  cstrUserName_);
	} else {
		// if the server is running under no user mode, this operation is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION public
//	Server::Manager::changePassword -- change password
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

//static
void
Manager::
changePassword(const ModUnicodeString& cstrUserName_,
			   const ModUnicodeString& cstrPassword_)
{
	// following operations are done atomically
	//  - modify password part of a user in user list
	//  - persist it to password file
	if (_UserList::_pUserList) {
		; _SYDNEY_ASSERT(_UserList::_pPasswordFile);
		_UserList::_pUserList->changePassword(*_UserList::_pPasswordFile, cstrUserName_, cstrPassword_);
	} else {
		// if the server is running under no user mode, this operation is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// FUNCTION public
//	Server::Manager::verifyPassword -- verify password
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	UserList::Entry::Pointer
//
// EXCEPTIONS

//static
UserList::Entry::Pointer
Manager::
verifyPassword(const ModUnicodeString& cstrUserName_,
			   const ModUnicodeString& cstrPassword_)
{
	UserList::Entry::Pointer pEntry;

	if (_UserList::_pUserList == 0) {
		// password user is unavailable (for backward compatibility)
		// If user name is specified, it's ignored with logging
		if (cstrUserName_.getLength() > 0) {
			SydInfoMessage << "User " << cstrUserName_ << " is specified but ignored." << ModEndl;
		}
	} else {
		// Specified user is checked
		if (!_UserList::_pUserList->get(cstrUserName_, pEntry)) {
			// no such user
			_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
		}
		; _SYDNEY_ASSERT(pEntry.get() != 0);
		if (!pEntry->check(cstrPassword_)) {
			// password is incorrect
			_SYDNEY_THROW0(Exception::AuthorizationFailed);
		}
	}
	// OK
	return pEntry; // if user list is ignored, null pointer will be returned
}

// FUNCTION public
//	Server::Manager::getUserID -- get userID from user name
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
Manager::
getUserID(const ModUnicodeString& cstrUserName_)
{
	if (!_UserList::_pUserList) {
		// if user management is disabled, this method is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// Specified user is checked
	if (cstrUserName_.getLength() == 0) {
		// user is required
		_SYDNEY_THROW0(Exception::UserRequired);
	}
	UserList::Entry::Pointer pEntry;
	if (!_UserList::_pUserList->get(cstrUserName_, pEntry)) {
		// no such user
		_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
	}
	return pEntry->getID();
}

// FUNCTION public
//	Server::Manager::isSuperUser -- check superuser from user name
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrUserName_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Manager::
isSuperUser(const ModUnicodeString& cstrUserName_)
{
	if (!_UserList::_pUserList) {
		// if user management is disabled, this method is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// Specified user is checked
	if (cstrUserName_.getLength() == 0) {
		// user is required
		_SYDNEY_THROW0(Exception::UserRequired);
	}
	UserList::Entry::Pointer pEntry;
	if (!_UserList::_pUserList->get(cstrUserName_, pEntry)) {
		// no such user
		_SYDNEY_THROW1(Exception::UserNotFound, cstrUserName_);
	}
	return pEntry->isSuperUser();
}

// FUNCTION public
//	Server::Manager::recoverPasswordFile -- recover password file
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

//static
void
Manager::
recoverPasswordFile()
{
	ModUnicodeString cstrPath;
	if (_UserList::getPasswordFilePath(cstrPath)) {
		PasswordFile::revertBackupFile(cstrPath);
	}
}

//
//	FUNCTION public static
//	Server::Manager::getMasterID -- マスターIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		このサーバでサポートしているマスターID
//
//	EXCEPTIONS
//
int
Manager::getMasterID()
{
	int iResult = _cParameterMasterID.get();
	
	if (_UserList::_pUserList != 0) {
		iResult |= Communication::AuthorizeMode::Supported;
	}
	return iResult;
}

//
//	FUNCTION private static
//	Server::Manager::clearAvailability -- 利用可能性をクリアする
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
void
Manager::clearAvailability()
{
	Os::AutoCriticalSection cAuto(m_cAvailabilityCriticalSection);

	m_bAvailable = true;
}

// FUNCTION private
//	Server::Manager::revokeAll -- revoke all the privileges from all the databases for one user
//
// NOTES
//
// ARGUMENTS
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Manager::
revokeAll(int iUserID_)
{
	// Block checkpoint thread
	Checkpoint::Daemon::AutoDisabler
		cDisabler(Checkpoint::Daemon::Category::Executor);

	// call revokeAll for all the database
	Schema::Manager::ObjectTree::Database::revokeAll(iUserID_);
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
