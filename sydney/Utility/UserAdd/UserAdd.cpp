// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UserAdd.cpp -- EXEのメイン関数
// 
// Copyright (c) 2008, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "UserAdd";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyNameSpace.h"
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#endif
#include <iostream>
#include <signal.h>

#include "UserAdd/Option.h"
#include "UserAdd/Exec.h"
#include "UserAdd/ExecAdd.h"
#include "Server/Singleton.h"
#include "Client2/Singleton.h"
#include "Client2/DataSource.h"
#include "Exception/Object.h"
#include "Exception/Message.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"
#include "Communication/CryptMode.h"	// 暗号化対応

#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

using namespace std;

_TRMEISTER_USING

// USAGE
void USAGE()
{
#ifdef SYD_OS_WINDOWS
	cout << "Usage: useradd /remote hostname portnumber [options]"					<< endl;
	cout << ""																		<< endl;
	cout << "where options include:"												<< endl;
	cout << "   /user rootusername              set root user name."				<< endl;
	cout << "   /password rootuserpassword      set password for the root user."	<< endl;
	cout << "   /username newusername           set add user name."					<< endl;
	cout << "   /userid newuserid               set ID for the new user."			<< endl;
	cout << "   /help                           print this message."				<< endl;
	cout << ""																		<< endl;
#else
	cout << "Usage: useradd -remote hostname portnumber [options]"					<< endl;
	cout << ""																		<< endl;
	cout << "where options include:"												<< endl;
	cout << "   -user rootusername              set root user name."				<< endl;
	cout << "   -password rootuserpassword      set password for the root user."	<< endl;
	cout << "   -username newusername           set add user name."					<< endl;
	cout << "   -userid newuserid               set ID for the new user."			<< endl;
	cout << "   -help                           print this message."				<< endl;
	cout << ""																		<< endl;
#endif
}

//
//	FUNCTION global
//	::main -- メイン関数
//
//	NOTES
//	useraddのメイン関数
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

		//RemoteServerの初期化
		Client2::Singleton::RemoteServer::initialize(cOption.getParentRegPath());
		//DataSourceの確保
		pDataSource = Client2::DataSource::createDataSource(cOption.getHostName(), cOption.getPortNumber());

		try
		{
			//DataSourceをオープンする
			// 暗号化対応
			pDataSource->open( (TRMeister::Client2::DataSource::Protocol::Value)(TRMeister::Client2::DataSource::Protocol::Version3 | cOption.getCryptMode()) );
	
			try
			{
				ModAutoPointer<Exec> pExec;
				pExec = new ExecAdd(*pDataSource, cOption);

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
		catch (...)
		{
			//終了処理
			pDataSource->release();
			Client2::Singleton::RemoteServer::terminate();
			throw;
		}

		//終了処理
		pDataSource->release();
		Client2::Singleton::RemoteServer::terminate();
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
//	Copyright (c) 2008, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
