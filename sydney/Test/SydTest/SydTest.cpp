// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydTest.cpp -- EXEのメイン関数
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/Parser.h"
#include "SydTest/Item.h"
#include "SydTest/Command.h"
#include "SydTest/CascadeConf.h"
#include "SydTest/Executor.h"
#include "Exception/Object.h"
#include "Common/ExceptionMessage.h"
#include "Common/Message.h"
#include "Common/SystemParameter.h"
#include "Common/Parameter.h"
#include "Common/UnicodeString.h"
#include "Communication/CryptMode.h"
#include "ModAutoPointer.h"
#include "ModException.h"
#include "Server/Singleton.h"
#include "SydTest/String.h"
#include "SydTest/Monitor.h"
#include "SydTest/Number.h"
#include "SydTest/SydTestException.h"
#include "SydTest/SydTestMessage.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <signal.h>

#ifdef SYD_OS_WINDOWS
#include <windows.h>
#ifdef DEBUG
#include <crtdbg.h>
#endif
#endif

_SYDNEY_USING
using namespace SydTest;

namespace
{
//	iSydTestInfoMessage
//
//	NOTES
//	0 -- 出力しない
//	1 -- 出力する
//	-1 -- まだ初期化されていない
//

int iSydTestInfoMessage = -1;
int iSydTestDebugMessage = -1;
int iSydTestTimeMessage = -1;

void
checkMessage(int& state, const ModCharString& key)
{
	state = 0;
	// 初めて呼び出されるので、パラメータを読み込む
	ModUnicodeString cstrFileName;
	if (Common::SystemParameter::getValue(key, cstrFileName))
 	{
 		if (cstrFileName.getLength() != 0
 			&& cstrFileName[0] != '0')
		{
			// 出力する
			state = 1;
		}
	}
}

}

bool
isSydTestInfoMessage()
{
 	if (iSydTestInfoMessage == -1)
 	{
		ModCharString registryKey = "SydTest_MessageOutputInfo";
		checkMessage(iSydTestInfoMessage, registryKey);
 	}
 	return iSydTestInfoMessage;
}

bool
isSydTestDebugMessage()
{
 	if (iSydTestDebugMessage == -1)
 	{
		ModCharString registryKey = "SydTest_MessageOutputDebug";
		checkMessage(iSydTestDebugMessage, registryKey);
 	}
 	return iSydTestDebugMessage;
}

bool
isSydTestTimeMessage()
{
 	if (iSydTestTimeMessage == -1)
 	{
		ModCharString registryKey = "SydTest_MessageOutputTime";
		checkMessage(iSydTestTimeMessage, registryKey);
 	}
 	return iSydTestTimeMessage;
}

//
//	FUNCTION public
//  SydTest::SydTest::main -- メイン関数
//
//	NOTES
//  メイン関数
//
//	ARGUMENTS
//    int argc
//      引数の個数
//    char *argv[]
//      引数の配列
//
//	RETURN
//  int
//    終了ステータス
//
//	EXCEPTIONS
//    SydTestException
//
int
main(int argc, char *argv[])
{
	ModMemoryPool::setTotalLimit(ModSizeMax >> 10); // KB単位
	ModOs::Process::setEncodingType(Common::LiteralCode);

	srand(time(0));
	Executor::setEncodingType_sub("");

	// Executorに渡す実行時オプション
	int iOption = ExecuteOption::Nothing; 
	// 解析中の引数の添字
	int iOptInd = 0;

	ModUnicodeString cstrHostName;
	int iPortNumber = -1;
	ModUnicodeString cstrUserName;
	ModUnicodeString cstrPassword;

#if MOD_CONF_REGISTRY == 1
	ModUnicodeString cstrRegPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister";
#else
	ModUnicodeString cstrRegPath;
#endif
	ModVector<CascadeInfo> cascadeInfoVector;
	CascadeConf cascadeConf;

	Communication::CryptMode::Value cryptMode = Communication::CryptMode::Unknown;

	if (argc < 2) {
		std::cout << "USAGE: " << argv[0] << " File" << std::endl;
		std::cout << "option: " << std::endl;
		std::cout << "	/c	show commands" << std::endl;
		std::cout << "	/p	show parameters" << std::endl;
		std::cout << "	/b	batch mode (ignore Pause commands)" << std::endl;
		std::cout << "	/d	debug mode (not execute commands)" << std::endl;
		std::cout << "	/s	show session numbers as `<<N>>'" << std::endl;
		std::cout << "	/e	abort when failing in (Not)Exists command" << std::endl;
		std::cout << "	/i	abort when failing in Initialize command" << std::endl;
		std::cout << "	/r	set parent registry path (default: 'HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh')" << std::endl;
		std::cout << "	/remote <hostname> <portnumber>  connect to remote server" << std::endl;
		std::cout << "	/E	set file encoding (utf8(default)|shiftJis|euc)" << std::endl;
		std::cout << "	/crypt use encryption" << std::endl;
		std::cout << "	/R	replace back slash to path delimiter" << std::endl;
		std::cout << "	/user <username>	default user name for sessions" << std::endl;
		std::cout << "	/password <password>	password for the default user" << std::endl;
		std::cout << "	/x	set explain feature for all the statements" << std::endl;
		std::cout << "	/P	execute all commands as prepared except for schema operations" << std::endl;
		std::cout << "	/dist <hostname> <portnumber> [, <hostname> <portnumber> ...]	set child servers" << std::endl;
		std::cout << "	/distfile <filename>	load a configuration file for child servers" << std::endl;
		return -1;
	} else {
		while(true) {
			iOptInd++;
			if (iOptInd >= argc) {
				std::cout << "file name empty." << std::endl;
				return -1;
			} else if (strcmp(argv[iOptInd], "/c") == 0) {
				iOption |= ExecuteOption::ShowCommand;
			} else if (strcmp(argv[iOptInd], "/p") == 0) {
				iOption |= ExecuteOption::ShowParameter;
			} else if (strcmp(argv[iOptInd], "/s") == 0) {
				iOption |= ExecuteOption::ShowSessionNumber;
			} else if (strcmp(argv[iOptInd], "/b") == 0) {
				iOption |= ExecuteOption::IgnorePause;
			} else if (strcmp(argv[iOptInd], "/d") == 0) {
				iOption |= ExecuteOption::DebugMode
					    |  ExecuteOption::ShowCommand
					    |  ExecuteOption::ShowParameter
					    |  ExecuteOption::ShowSessionNumber;
			} else if (strcmp(argv[iOptInd], "/e") == 0) {
				iOption |= ExecuteOption::AbortWhenMissingPath;
			} else if (strcmp(argv[iOptInd], "/i") == 0) {
				iOption |= ExecuteOption::AbortWhenInitFail;
			} else if (strcmp(argv[iOptInd], "/remote") == 0) {
				cstrHostName = ModUnicodeString(argv[++iOptInd], Common::LiteralCode);
				iPortNumber = ::atoi(argv[++iOptInd]);
			} else if (strcmp(argv[iOptInd], "/r") == 0) {
				cstrRegPath = ModUnicodeString(argv[++iOptInd], Common::LiteralCode);
			} else if (strcmp(argv[iOptInd], "/E") == 0) {
				Executor::setEncodingType_sub(argv[++iOptInd]);
			} else if (strcmp(argv[iOptInd], "/crypt") == 0) {
				std::cout << "invalid arguments." << std::endl;
				return -1;
			} else if (strcmp(argv[iOptInd], "/R") == 0) {
				if (ModOsDriver::File::getPathSeparator()
					!= Common::UnicodeChar::usBackSlash)
				{
					iOption |= ExecuteOption::ReplaceBackSlash;
				}
			} else if (strcmp(argv[iOptInd], "/user") == 0) {
				cstrUserName = ModUnicodeString(argv[++iOptInd], Common::LiteralCode);
			} else if (strcmp(argv[iOptInd], "/password") == 0) {
				cstrPassword = ModUnicodeString(argv[++iOptInd], Common::LiteralCode);
			} else if (strcmp(argv[iOptInd], "/x") == 0) {
				iOption |= ExecuteOption::Explain;
			} else if (strcmp(argv[iOptInd], "/P") == 0) {
				iOption |= ExecuteOption::Prepared;
			} else if (strcmp(argv[iOptInd], "/dist") == 0) {
				if (!cascadeConf.setCascadeInfo(argc, argv, iOptInd, cascadeInfoVector)) {
					std::cout << "invalid arguments." << std::endl;
				}
#if MOD_CONF_REGISTRY == 0
                // Linuxでは分散テスト実行前に regPath の値を入れないと SEGV になってしまう                          
                cstrRegPath = ModUnicodeString("SYDPARAM");
#endif
			} else if (strcmp(argv[iOptInd], "/distfile") == 0) {
				if (!cascadeConf.setCascadeInfo(argv[++iOptInd], cascadeInfoVector)) {
					std::cout << "failed to load configuration file :" << argv[iOptInd] << std::endl;
				}
#if MOD_CONF_REGISTRY == 0
                // Linuxでは分散テスト実行前に regPath の値を入れないと SEGV になってしまう                          
                cstrRegPath = ModUnicodeString("SYDPARAM");
#endif
			} else if (*(argv[iOptInd]) == '/') {
				std::cout << "unknown option /" << argv[iOptInd][1]
					 << "." << std::endl;
				return -1;
			} else {
				break;
			}
		}
	}		

#ifdef SYD_OS_WINDOWS
	if ((iOption & ExecuteOption::AbortWhenMissingPath) == 0
		&&
		(iOption & ExecuteOption::AbortWhenInitFail) == 0) {

// メッセージボックス関係を出力しないようにする
#ifdef DEBUG
		// DEBUG版以外の時はプリプロセッサが無視してくれるらしいが、念のため。

		// RunTime Errorをメッセージボックスからstdoutに切り替える
		// stderrでないのはエラー箇所をわかりやすくするため
	
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
		_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
#endif
		// 一般保護違反メッセージボックスを表示しないようにする
		// Utility/SydServer.cpp を参考にした。
	
		const UINT oldMode = SetErrorMode(0);
		(void) SetErrorMode(oldMode | SEM_NOGPFAULTERRORBOX);

		//SydTestDebugMessage << "SetErrorMode" << ModEndl;
	}
#endif

    // Sydney を異常終了させたか (自動リカバリ系のテストで true となる)
    bool abnormalTerminated = false;

	try {
		// こうしないとcMonitorを初期化できない
		Common::SystemParameter::initialize(cstrRegPath);
		Monitor cMonitor;
		isSydTestInfoMessage();
		isSydTestDebugMessage();
		isSydTestTimeMessage();

		Executor cExecutor(argv[iOptInd], "Main", iOption, cMonitor, 
						   cstrHostName, iPortNumber, cstrRegPath, cascadeInfoVector,
						   cryptMode, cstrUserName, cstrPassword);
		cExecutor.executeMain();
        // テスト終了後に初期化済みのままなら異常終了とみなす
        abnormalTerminated = cExecutor.isInitialized();

		cMonitor.terminate();
	} catch(SydTestException& e) {
		switch (e.getLine())
		{
		case SydTestException::Requested:
			SydTestErrorMessage << "Stopped by your request." << ModEndl;
			break;
		case SydTestException::FileNotFound:
			SydTestErrorMessage << "File not Found." << ModEndl;
			break;
		case SydTestException::BadSyntax:
			SydTestErrorMessage << "Parse Error in line " 
				<< e.getLine() << "." << ModEndl;
		default:
			SydTestErrorMessage << "SydTest-Error #" << e.getDescription() 
				<< " occurred.  Stop." << ModEndl;
		}
		return -1;
	} catch (ModException& err) {
		// Executorが捕捉しなかったModExceptionを取る
		SydTestErrorMessage << "ModException occurred." << ModEndl;
		ModErrorMessage << err << ModEndl;
		return -1;
	} catch (Exception::Object& err) {
		// 念のためSydneyが発する例外も捕捉
		SydTestErrorMessage << err << ModEndl;
		return -1;
	} catch(...) { // 実行時エラーをcatch(access violationなど)
		SydTestErrorMessage 
			<< "Some error occured during the test. Stop."
			<< ModEndl;
		return -1;
	}

#ifdef SYD_OS_POSIX
    if (abnormalTerminated) {
        // RHEL8 対応
        // glibc のバージョンアップに伴い、pthread の undefined behavior の動作が変わったことへの対策。
		// SydTest の Terminate 処理を実行せずに、main() の return でプロセスを終えようとすると、
        // pthread_cond_destroy() でハングすることがあるためプロセスを強制終了する
		//
		// https://bugzilla.redhat.com/show_bug.cgi?id=1647381

        ::kill(::getpid(), SIGKILL);
	}
#endif

	return 0;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
