// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Sqli.cpp -- EXEのメイン関数
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2014, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Sqli";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyNameSpace.h"
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#endif
#include <iostream>
#include <signal.h>

#include "Sqli/Option.h"
#include "Sqli/ExecLine.h"
#include "Sqli/ExecFile.h"
#include "Sqli/ExecStdin.h"
#ifdef SYD_SQLI_INPROCESS
#include "Server/Singleton.h"
#endif
#include "Client2/Singleton.h"
#include "Client2/DataSource.h"
#include "Exception/Message.h"
#include "Exception/Object.h"
#include "Exception/NumberAuthorizationFailed.h"
#include "Exception/NumberUserNotFound.h"
#include "Exception/NumberUserRequired.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"
#include "Communication/AuthorizeMode.h"
#include "Communication/CryptMode.h"	// 暗号化対応

#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

using namespace std;

_TRMEISTER_USING

namespace
{
	// パスワード入力の最大リトライ数
	int _iRetryMax = 3;
}

// USAGE
void USAGE()
{
	// inprocessは、隠し機能
	// regは、inprocess使用時のみ使われるので、これも隠し機能
#ifdef SYD_OS_WINDOWS
	cout << "Usage: sqli /remote hostname portnumber [options]"		<< endl;
	cout << ""														<< endl;
	cout << "where options include:"								<< endl;
	cout << "   /database databasename          set a database."		<< endl;
	cout << "   /user username                  set session user name."		<< endl;
	cout << "   /password password              set password for the user."		<< endl;
	cout << "   /sql statement                  execute a command line sql."	<< endl;
	cout << "   /shutdown                       shutdown server."	<< endl;
	cout << "   /version                        print server version."	<< endl;
	cout << "   /ipv4 or /ipv6                  set address family."	<< endl;
	cout << "   /time                           print execution time."	<< endl;
	cout << "   /help                           print this message." << endl;
	cout << "   /code codename                  input and ouput character encoding." << endl;
	cout << "                                   codename is 'utf-8' or ''" << endl;
	cout << "                                   (defalt:''. '' is os default.)" << endl;
	cout << ""														<< endl;
#else
	cout << "Usage: sqli -remote hostname portnumber [options]"		<< endl;
	cout << ""														<< endl;
	cout << "where options include:"								<< endl;
	cout << "   -database databasename          set a database."		<< endl;
	cout << "   -user username                  set session user name."		<< endl;
	cout << "   -password password              set password for the user."		<< endl;
	cout << "   -sql statement                  execute a command line sql."	<< endl;
	cout << "   -shutdown                       shutdown server."	<< endl;
	cout << "   -version                        print server version."	<< endl;
	cout << "   -ipv4 or -ipv6                  set address family."	<< endl;
	cout << "   -time                           print execution time."	<< endl;
	cout << "   -help                           print this message." << endl;
	cout << "   -code codename                  input and ouput character encoding." << endl;
	cout << "                                   codename is 'utf-8' or ''" << endl;
	cout << "                                   (defalt:''. '' is euc-jp.)" << endl;
	cout << ""														<< endl;
#endif
}

//
//	FUNCTION global
//	::main -- メイン関数
//
//	NOTES
//	sqliのメイン関数
//
int
main(int argc, char* argv[])
{
#ifdef DEBUG
#if !defined(DEBUG) && !defined(QUANTIFY)

	// 一般保護違反メッセージボックスを表示しないようにする

	const UINT oldMode = SetErrorMode(0);
	(void) SetErrorMode(oldMode & SEM_NOGPFAULTERRORBOX);
#endif
#endif
	ModMemoryPool::setTotalLimit(ModSizeMax >> 10); // KB単位
	ModOs::Process::setEncodingType(Common::LiteralCode);

	// シグナル
	signal(SIGINT, SIG_IGN);

	Option cOption;
	if (cOption.set(argc, argv) == false)
	{
		USAGE();
		return 1;
	}

	try
	{
		Client2::DataSource* pDataSource = 0;

		//初期化
		switch (cOption.getLocation())
		{
#ifdef SYD_SQLI_INPROCESS
		case Option::Location::Install:
			//TRMeisterのインストール
			Server::Singleton::InProcess::initialize(true, cOption.getParentRegPath());
			break;
		case Option::Location::InProcess:
			//InProcessの初期化
			Server::Singleton::InProcess::initialize(false, cOption.getParentRegPath());
			//DataSourceの確保
			pDataSource = Client2::DataSource::createDataSource();
			break;
#endif
		case Option::Location::Remote:
			//RemoteServerの初期化
			Client2::Singleton::RemoteServer::initialize(cOption.getParentRegPath());
			//DataSourceの確保
			pDataSource = Client2::DataSource::createDataSource(cOption.getHostName(), cOption.getPortNumber(), cOption.getFamily());
			break;
		}

		try
		{
			
			if (cOption.isShutdown())
			{
				int iRetry = (cOption.isUserPasswordSpecified()) ? 1 : _iRetryMax + 1;
				for (;;) {
					try {
						//shutdown
						pDataSource->shutdown(cOption.getUserName(), cOption.getPassword());
						break;
					} catch (Exception::Object& e) {
						switch (e.getErrorNumber()) {
						case Exception::ErrorNumber::AuthorizationFailed:
						case Exception::ErrorNumber::UserNotFound:
						case Exception::ErrorNumber::UserRequired:
							{
								if (--iRetry > 0) {
									if (cOption.isUserPasswordSpecified()) {
										ModUnicodeOstrStream stream;
										stream << e;
										cerr << "[ERROR] "
											 << (const char*)Exec::unicodeToMultiByte(stream.getString())
											 << endl;
										ModOsDriver::Thread::sleep(3000);
									}
									cOption.clearUserPassword();
									if (cOption.inputUserName() == false) break;
									if (cOption.inputPassword() == false) break;
									break;
								}
								throw;
							}
						default:
							{
								throw;
							}
						}
					}
				}
			}
			else if (cOption.getLocation() != Option::Location::Install)
			{
				//DataSourceをオープンする
				// 暗号化対応
				pDataSource->open(
					(TRMeister::Client2::DataSource::Protocol::Value)
					(cOption.getProtocol() | cOption.getCryptMode()));
		
				try
				{
					ModAutoPointer<Exec> pExec;

					//次にターゲット
					switch (cOption.getTarget())
					{
					case Option::Target::Stdin:
						//標準入力
						pExec = new ExecStdin(*pDataSource, cOption);
						break;
					case Option::Target::File:
						//ファイル
						pExec = new ExecFile(*pDataSource, cOption);
						break;
					case Option::Target::Line:
						//コマンドライン
						pExec = new ExecLine(*pDataSource, cOption);
						break;
					}

					//初期化
					pExec->initialize();

					try
					{
						// 実行
						pExec->execute();
					}
					catch (Exception::Object& e)
					{
						ModUnicodeOstrStream stream;
						stream << e;
						cerr << "[ERROR] "
							 << (const char*)Exec::unicodeToMultiByte(stream.getString())
							 << endl;
						Exec::setExitStatus(1);
					}
					catch (ModException& e)
					{
						cerr << "[ERROR] ModException occurred. "
							 << e.setMessage() << endl;
						Exec::setExitStatus(1);
					}
					
					//終了処理
					pExec->terminate();
				}
				catch (...)
				{
					pDataSource->close();
					throw;
				}

				// データソースをクローズする
				pDataSource->close();
			}

		}
		catch (...)
		{
			//終了処理
			switch (cOption.getLocation())
			{
#ifdef SYD_SQLI_INPROCESS
			case Option::Location::Install:
			case Option::Location::InProcess:
				pDataSource->release();
				Server::Singleton::InProcess::terminate();
				break;
#endif
			case Option::Location::Remote:
				pDataSource->release();
				Client2::Singleton::RemoteServer::terminate();
				break;
			}

			throw;
		}

		//終了処理
		switch (cOption.getLocation())
		{
#ifdef SYD_SQLI_INPROCESS
		case Option::Location::Install:
		case Option::Location::InProcess:
			pDataSource->release();
			Server::Singleton::InProcess::terminate();
			break;
#endif
		case Option::Location::Remote:
			pDataSource->release();
			Client2::Singleton::RemoteServer::terminate();
			break;
		}
	}
	catch (Exception::Object& e)
	{
		ModUnicodeOstrStream stream;
		stream << e;
		cerr << "[ERROR] "
			 << (const char*)Exec::unicodeToMultiByte(stream.getString())
			 << endl;
		Exec::setExitStatus(1);
	}
	catch (ModException& e)
	{
		cerr << "[ERROR] ModException occurred. "
			 << e.setMessage() << endl;
		Exec::setExitStatus(1);
	}
	catch (...)
	{
		cerr << "[ERROR] Unexpected Error occurred." << endl;
		Exec::setExitStatus(1);
	}

	return Exec::getExitStatus();
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
