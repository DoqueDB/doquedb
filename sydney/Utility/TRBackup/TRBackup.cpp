// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TRBackup.cpp --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "TRBackup";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <wchar.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif
#include <iostream>

#include "Common/UnicodeString.h"
#include "Common/SystemParameter.h"
#include "Common/Configuration.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/DataArrayData.h"
#include "Client2/DataSource.h"
#include "Client2/Session.h"
#include "Client2/Singleton.h"
#include "Client2/ResultSet.h"

#include "TRBackup/Option.h"

#include "ModAutoPointer.h"
#include "ModVector.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

using namespace std;
using namespace TRMeister;

void USAGE()
{
	cout << endl;
	cout << "Usage: trbackup [options]" << endl;
	cout << endl;
	cout << "where options include:" << endl;
#ifdef SYD_OS_WINDOWS
	cout << "   /remote hostname portnumber		set host name and port number."		<< endl;
	cout << "   /database databasename          set a database."		<< endl;
	cout << "   /user username                  set session user name."		<< endl;
	cout << "   /password password              set password for the user."		<< endl;
#else
	cout << "   -remote hostname portnumber		set host name and port number."		<< endl;
	cout << "   -database databasename          set a database."		<< endl;
	cout << "   -user username                  set session user name."		<< endl;
	cout << "   -password password              set password for the user."		<< endl;
#endif
}

// トランザクションを開始したセッション
ModVector<Client2::Session*> _vecSession;

// ログ出力先
Common::Configuration::ParameterMessage _cMessage("TRBackup_MessageOutput");

// PIDファイル
ModUnicodeString _pidFile("TRBackup_ProcessIdFile");

// 情報を出力する
void
printInfo(const ModUnicodeString& message_)
{
	if (_cMessage.isOutput())
	{
		_SYDNEY_MESSAGE(
			_cMessage.getParameterName(),
			Common::MessageStreamBuffer::LEVEL_INFO)
				<< message_ << ModEndl;
	}
}

// エラーメッセージを出力する
void
printError(Exception::Object& e_)
{
	if (_cMessage.isOutput())
	{
		_SYDNEY_MESSAGE(
			_cMessage.getParameterName(),
			Common::MessageStreamBuffer::LEVEL_ERROR)
				<< e_ << ModEndl;
	}
}

// セッションを得る
Client2::Session*
getSession(Client2::DataSource* pDataSource_,
		   Option& cOption_,
		   const ModUnicodeString& db_)
{
	if (cOption_.getUserName().getLength() == 0)
	{
		return pDataSource_->createSession(db_);
	}

	return pDataSource_->createSession(db_,
									   cOption_.getUserName(),
									   cOption_.getPassword());
}

// 対象データベースのリストを得る
void
listDatabase(ModVector<ModUnicodeString>& vecDatabase_,
			 Client2::DataSource* pDataSource_,
			 Option& cOption_)
{
	if (cOption_.getDatabaseName().getLength() != 0)
	{
		vecDatabase_.pushBack(cOption_.getDatabaseName());
	}
	else
	{
		// システムDBにアクセスし、すべてのデータベースのリストを得る

		Client2::Session* session = getSession(pDataSource_, cOption_,
											   ModUnicodeString("$$SystemDB"));
		try
		{
			Client2::ResultSet* r = session->executeStatement(
				ModUnicodeString("select name from system_database"));

			for (;;)
			{
				ModAutoPointer<Common::DataArrayData> tuple
					= new Common::DataArrayData;
				Client2::ResultSet::Status::Value status
					= r->getNextTuple(tuple.get());
				if (status == Client2::ResultSet::Status::Data)
				{
					Common::Data::Pointer p = tuple->getElement(0);
					const Common::StringData& s
						= _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
											   *(p.get()));
					vecDatabase_.pushBack(s.getValue());
				}
				else if (status == Client2::ResultSet::Status::EndOfData
						 || status == Client2::ResultSet::Status::MetaData)
				{
				}
				else if (status == Client2::ResultSet::Status::Success
						 || status == Client2::ResultSet::Status::Canceled)
				{
					break;
				}
			}

			r->close();
			r->release();
		}
		catch (...)
		{
			session->close();
			session->release();
			throw;
		}

		session->close();
		session->release();
	}
}

// 対象データベースのバックアップを開始する
void
startBackup(Client2::DataSource* pDataSource_, Option& cOption_)
{
	ModVector<ModUnicodeString> vecDatabase;
	listDatabase(vecDatabase, pDataSource_, cOption_);

	ModVector<ModUnicodeString>::Iterator i
		= vecDatabase.begin();
	for (; i != vecDatabase.end(); ++i)
	{
		printInfo(*i);
		
		// セッションを得る
		
		Client2::Session* session
			= getSession(pDataSource_, cOption_, *i);

		// トランザクションを開始する

		Client2::ResultSet* r = session->executeStatement(
			ModUnicodeString("start transaction read write"));
		try
		{
			r->getStatus();
			r->close();
			r->release();
		}
		catch (...)
		{
			r->close();
			r->release();
			throw;
		}

		// バックアップを開始する
			
		r = session->executeStatement(
			ModUnicodeString("start backup full"));
		try
		{
			r->getStatus();
			r->close();
			r->release();
		}
		catch (...)
		{
			r->close();
			r->release();
			throw;
		}

		// 配列に格納する
		_vecSession.pushBack(session);
	}
}

// 対象データベースのバックアップを終了する
void
endBackup()
{
	ModVector<Client2::Session*>::Iterator i = _vecSession.begin();
	for (; i != _vecSession.end(); ++i)
	{
		// バックアップを終了する

		Client2::ResultSet* r = (*i)->executeStatement(
			ModUnicodeString("end backup"));
		try
		{
			r->getStatus();
			r->close();
			r->release();
		}
		catch (Exception::Object& e)
		{
			r->close();
			r->release();

			printError(e);
		}

		// トランザクションを確定する

		r = (*i)->executeStatement(ModUnicodeString("commit"));
		try
		{
			r->getStatus();
			r->close();
			r->release();
		}
		catch (Exception::Object& e)
		{
			r->close();
			r->release();
			printError(e);
		}

		// セッションを終了する
		(*i)->close();
		(*i)->release();
	}

	_vecSession.clear();
}

#ifdef SYD_OS_WINDOWS
int
main(int argc, char* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
#ifdef SYD_OS_INDOWS
#if !defined(DEBUG) && !defined(QUANTIFY)

	// 一般保護違反メッセージボックスを表示しないようにする

	const UINT oldMode = SetErrorMode(0);
	(void) SetErrorMode(oldMode & SEM_NOGPFAULTERRORBOX);
#endif
#endif

	ModMemoryPool::setTotalLimit(ModSizeMax >> 10); // KB単位
	ModOs::Process::setEncodingType(Common::LiteralCode);

	Option cOption;
	if (cOption.set(argc, argv) == false)
	{
		USAGE();
		return 1;
	}

	Client2::Singleton::RemoteServer::initialize();

	ModUnicodeString hostname("localhost");
	if (cOption.getHostName().getLength())
		hostname = cOption.getHostName();
	
	int portnumber = 0;
	Common::SystemParameter::getValue(
		ModUnicodeString("Communication_PortNumber"), portnumber);
	if (cOption.getPortNumber() != 0)
		portnumber = cOption.getPortNumber();

	Client2::DataSource* pDataSource = 0;
	try
	{
		// サーバに接続する
		pDataSource
			= Client2::DataSource::createDataSource(hostname, portnumber);
		pDataSource->open();

		try
		{
			// バックアップ文を発行する
			startBackup(pDataSource, cOption);
		}
		catch (Exception::Object& e)
		{
			printError(e);
			
			pDataSource->close();
			pDataSource->release();
			::_exit(1);
		}
	}
	catch (Exception::Object& e)
	{
		printError(e);
		::_exit(1);
	}

	// ここでデーモンになる

#ifdef SYD_OS_WINDOWS
#else
	sigset_t ss;
	::sigemptyset(&ss);
	::sigaddset(&ss, SIGTERM);
	::sigaddset(&ss, SIGCHLD);
	::sigaddset(&ss, SIGPIPE);
	::sigaddset(&ss, SIGHUP);
	::sigaddset(&ss, SIGINT);
	::sigaddset(&ss, SIGQUIT);
	::pthread_sigmask(SIG_BLOCK, &ss, 0);

	// デーモンプログラムになる
	int parentPid = ::getpid();
	int pid = ::fork();
		// -lpthread なので fork() == fork1() である
	if (pid > 0) {
		// 親プロセス

		// exitParent or waitAndExit が呼ばれるまで停止する
		::sigemptyset(&ss);
		::sigaddset(&ss, SIGTERM);
		::sigaddset(&ss, SIGCHLD);
		int signo = 0;
		if (::sigwait(&ss, &signo) == 0)
		{
			if (signo == SIGCHLD)
			{
				// 正常な場合にはここには来ない
		
				// startService での初期化中にエラーが発生し
				// 子プロセスが先に終了した場合の処理を行う

				fprintf(stderr, "ERROR: child process start up error.\n");
				
				int	stat_loc;
				::wait(&stat_loc);
				::_exit(1);
			}
			else
			{
				// 正常に子プロセスが起動した
				::_exit(0);
			}
		}

	} else if (pid == 0) {
		//子プロセス
		// デーモンプロセスのための処理を行う

		// ファイルディスクリプターの解放
		::close(0);
		::close(1);
		::close(2);
		// /dev/nullを割り当てる
		if (::open("/dev/null", O_RDWR) == -1)
			// オープンできなければ終了する
			return;
		::dup2(0, 1);
		::dup2(0, 2);

		// setsid は以下のことを行う
		//	新しいセッションを開始し自身がセッションリーダーになる
		//	新しいセッションのプロセスグループのプロセスグループリーダーになる
		//	制御端末から切り放される
		::setsid();

		// プロセスIDを書き出す
		ModUnicodeString fileName;
		if (Common::SystemParameter::getValue(_pidFile, fileName) == false)
			fileName = "/var/run/trbackup.pid";
		FILE* fp = ::fopen(fileName.getString(), "w");
		if (fp != 0)
		{
			::fprintf(fp, "%d", getpid());
			::fclose(fp);
		}
		
		printInfo(ModUnicodeString("Start Backup"));

		::sleep(1);

		// 親プロセスを停止する
		::kill(parentPid, SIGTERM);

		sigset_t ss;
		::sigemptyset(&ss);
		::sigaddset(&ss, SIGTERM);

		int signo = 0;
		while (::sigwait(&ss, &signo) == 0)
		{
			switch (signo)
			{
			case SIGTERM:
				try
				{
					printInfo(ModUnicodeString("End Backup"));
					
					// バックアップ終了文を発行する
					endBackup();

					// データソースをクローズする
					pDataSource->close();
					pDataSource->release();
				}
				catch (...)
				{
					pDataSource->close();
					pDataSource->release();
				}
				
				Client2::Singleton::RemoteServer::terminate();
		
				return 0;
			default:
				break;
			}
		}
		
	} else {
		//エラー
		fprintf(stderr, "Can't fork process.\n");
	}
#endif

	return 0;
}

//
//	Copyright (c) 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
