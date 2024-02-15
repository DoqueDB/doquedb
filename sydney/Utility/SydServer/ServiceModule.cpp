// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServiceModule.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2013, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydServer";
}

//#define TRACE_LOG

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyNameSpace.h"
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#include <process.h>
#include <iostream>
#include <stdio.h>
using namespace std;
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
#include "Server/Singleton.h"
#include "Os/Event.h"
#include "Common/SystemParameter.h"
#include "ModUnicodeString.h"
#ifdef SYD_OS_LINUX
#include "Os/CriticalSectionManager.h"
#endif
#endif
#include "SydServer/ServiceModule.h"

extern FILE* fp;

#ifndef SYD_OS_WINDOWS
_SYDNEY_USING
#endif

#ifdef SYD_OS_WINDOWS
void (*ServiceModule::initializeFunc)(bool, const unsigned short*) = 0;
void (*ServiceModule::terminateFunc)(void) = 0;
void (*ServiceModule::stopFunc)(void) = 0;
void (*ServiceModule::joinFunc)(void) = 0;
bool (*ServiceModule::isRunningFunc)(void) = 0;
void (*ServiceModule::reloadFunc)(void) = 0;
#endif
	
namespace
{
	// レジストリの親パス
	const unsigned short* _regPath;

	// デバッグモードかどうか
#ifdef TRACE_LOG
	bool _bDebug = true;
#else
	bool _bDebug = false;
#endif

#ifdef SYD_OS_WINDOWS
	//
	//	VARIABLE local
	//	_$$::_pLibrary -- SyKernel.dllのモジュールハンドル
	//
	HMODULE _pLibrary = 0;

	//
	//	VARIABLE local
	//	_$$::_pStopEvent -- サービス停止時にシグナル化される
	//
	HANDLE _pStopEvent = 0;

	//
	//	VARIABLE local
	//	_$$::_pServiceStatusHandle -- サービスステータスを報告するハンドル
	//
	SERVICE_STATUS_HANDLE _pServiceStatusHandle;

	//
	//	VARIABLE local
	//	_$$::_cServiceStatus -- サービスステータス
	//
	SERVICE_STATUS _cServiceStatus;

	// デフォルトのサービス名
	const unsigned short* _serviceName = L"SydServer";
	// デフォルトの表示名
	const unsigned short* _displayName = L"TRMeister";
	// デフォルトの説明
	const unsigned short* _description = L"Ricoh Database System";

	// スレッドの終了ステータス
	int _status = 0;

	// 初期化
    unsigned int __stdcall _SydneyInitialize(void* arg)
	{
		_status = 0;
		try
		{
			// Sydneyの初期化を行う
			(*ServiceModule::initializeFunc)(false, _regPath);
		}
		catch (...)
		{
			_status = -1;
		}
		return 0;
	}

	// 終了処理
	unsigned int __stdcall _SydneyTerminate(void* arg)
	{
		_status = 0;
		try
		{
			// サーバを停止する
			(*ServiceModule::stopFunc)();
			// 停止するまで待つ
			(*ServiceModule::joinFunc)();
			// 終了処理
			(*ServiceModule::terminateFunc)();
		}
		catch (...)
		{
			_status = -1;
		}
		return 0;
	}

#define _TRACE(message)	if (fp) ::fprintf(fp, message " (%d)\n", __LINE__), ::fflush(fp);

#else
	// PIDファイル
	ModUnicodeString _pidFile("SydServer_ProcessIdFile");

	extern "C" void _SignalWaitFunction(void* p)
	{
		// プロセスIDを書き出す
		ModUnicodeString fileName;
		if (Common::SystemParameter::getValue(_pidFile, fileName) == false)
			fileName = "/var/run/trmeister.pid";
		FILE* fp = ::fopen(fileName.getString(), "w");
		if (fp != 0)
		{
			::fprintf(fp, "%d", getpid());
			::fclose(fp);
		}
			
		sigset_t ss;
		::sigemptyset(&ss);
		::sigaddset(&ss, SIGTERM);
		::sigaddset(&ss, SIGHUP);
		::sigaddset(&ss, SIGUSR1);
		
		int signo = 0;
		while (::sigwait(&ss, &signo) == 0)
		{
			switch (signo)
			{
			case SIGTERM:
				try
				{
					// サーバを停止する
					Server::Singleton::RemoteServer::stop();
				}
				catch (...) {}
				return;
			case SIGHUP:
				try
				{
					// パラメータの読み直し、ログファイルのみ開き直し
					Server::Singleton::RemoteServer::reload();
				}
				catch (...) {}
				break;
			case SIGUSR1:
#ifdef SYD_OS_LINUX				
				try
				{
					// ロック中のクリティカルセクション情報を出力する
					Os::CriticalSectionManager::printOut();
				}
				catch (...) {}
#endif				
				break;
			default:
				break;
			}
		}
	}

#endif
}

#ifdef SYD_OS_WINDOWS
//	FUNCTION public static
//	ServiceModule::registerService -- サービスに登録する
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
ServiceModule::registerService(
	const unsigned short* regPath,
	const unsigned short* serviceName,
	const unsigned short* displayName,
	const unsigned short* userName,
	const unsigned short* password,
	const unsigned short* description)
{
	if (serviceName == 0)
		serviceName = _serviceName;
	if (displayName == 0)
		displayName = _displayName;
	if (description == 0)
		description = _description;

	//すでにインストールしてあるか
	if (isRegisterService(serviceName) == true)
		return;

	//サービスマネージャーをオープンする
	SC_HANDLE manager = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (manager == 0)
	{
		//エラー
		cerr << "Can't open service manager. (ERROR="
			 << ::GetLastError() << ")" << endl;
		return;
	}

	//Sydneyにとって動くために必要なサービス
	const unsigned short* dependencies = L"Tcpip\0\0";

	//プロセスのファイル名を得る
	unsigned short imagePath[_MAX_PATH];
	::GetModuleFileName(0, imagePath, _MAX_PATH);

	// /RegistryPathの指定をimagePathに加える
	unsigned short imagePathWithOption[1024];
	swprintf(imagePathWithOption,
			 L"\"%s\" /RegistryPath \"%s\"", imagePath, regPath);

	//サービスに登録する
	SC_HANDLE service =
		::CreateService(manager,						//database handle
						serviceName,					//service name
						displayName,					//display name
						SERVICE_ALL_ACCESS,				//desired access
						SERVICE_WIN32_OWN_PROCESS,		//service type
						SERVICE_AUTO_START,				//start type
						SERVICE_ERROR_NORMAL,			//error control
						imagePathWithOption,			//execute file
						0,								//load order group
						0,								//tag ID
						dependencies,					//dependencies
						userName,						//user name
						password						//password
			);									
	if (service == 0)
	{
		cerr << "Can't install service. (ERROR="
			 << ::GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
		return;
	}
#ifdef SYD_OS_WINNT5_0
	SERVICE_DESCRIPTION info;
	info.lpDescription = (unsigned short*)description;

	(void) ::ChangeServiceConfig2(service,
								  SERVICE_CONFIG_DESCRIPTION,
								  &info);
#endif
	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
}

//	FUNCTION public static
//	ServiceModule::unRegisterService -- サービスから削除する
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
ServiceModule::unRegisterService(const unsigned short* serviceName)
{
	if (serviceName == 0)
		serviceName = _serviceName;

	//登録してあるか
	if (isRegisterService(serviceName) == false)
		return;

	//サービスマネージャーをオープンする
	SC_HANDLE manager = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
	if (manager == 0)
	{
		cerr << "Can't open service manager. (ERROR="
			 << ::GetLastError() << ")" << endl;
		return;
	}

	SC_HANDLE service =
		::OpenService(manager,								// database handle
					  serviceName,							// service name
					  SERVICE_ALL_ACCESS);					// dsired access
	if (service == 0)
	{
		cerr << "Service not found. (ERROR="
			 << ::GetLastError() << ")" << endl;
		::CloseServiceHandle(manager);
		return;
	}

	if (!::DeleteService(service))
	{
		cerr << "Can't delete service. (ERROR="
			 << ::GetLastError() << ")" << endl;
		::CloseServiceHandle(service);
		::CloseServiceHandle(manager);
		return;
	}

	::CloseServiceHandle(service);
	::CloseServiceHandle(manager);
}

//
//	FUNCTION public static
//	ServiceModule::loadSydney -- SyKernel.dllをロードする
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	bool
//		ロードできた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ServiceModule::loadSydney()
{
	if (_pLibrary == 0)
	{
		_pLibrary = ::LoadLibrary(L"SyKernel.dll");
		if (_pLibrary == 0)
		{
			cout << "Can't load library (ERROR="
				 << ::GetLastError() << ")" << endl;
			return false;
		}
		initializeFunc = (void (*)(bool, const unsigned short*))
			::GetProcAddress(_pLibrary, "SydServerInitialize");
		terminateFunc = (void (*)(void))
			::GetProcAddress(_pLibrary, "SydServerTerminate");
		stopFunc = (void (*)(void))
			::GetProcAddress(_pLibrary, "SydServerStop");
		joinFunc = (void (*)(void))
			::GetProcAddress(_pLibrary, "SydServerJoin");
		isRunningFunc = (bool (*)(void))
			::GetProcAddress(_pLibrary, "SydServerIsRunning");
		reloadFunc = (void (*)(void))
			::GetProcAddress(_pLibrary, "SydServerReload");
	}
	return true;
}

//
//	FUNCTION public
//	ServiceModule::unloadSydney -- SyKernel.dllをアンロードする
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
ServiceModule::unloadSydney()
{
	initializeFunc = 0;
	terminateFunc = 0;
	stopFunc = 0;
	joinFunc = 0;
	isRunningFunc = 0;
	reloadFunc = 0;

	::FreeLibrary(_pLibrary);
	_pLibrary = 0;
}

#endif

//
//	FUNCTION public static
//	ServiceModule::startService -- サービスを起動する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString&	regPath
//		レジストリの親パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ServiceModule::startService(const unsigned short* regPath, bool bDebug_)
{
	_regPath = regPath;
	if (bDebug_ == true) _bDebug = bDebug_;

#ifdef SYD_OS_WINDOWS
	_TRACE("begin startService");

	//【注意】	SERVICE_WIN32_OWN_PROCESS
	//			のときのサービス名は無視されるので、
	//			_pServiceName を渡してもよい

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ const_cast<unsigned short*>(_serviceName),
		  (LPSERVICE_MAIN_FUNCTION) ServiceModule::serviceMain },
		{0, 0}
	};

	_TRACE("StartServiceCtrlDispatcher");
	BOOL stat = ::StartServiceCtrlDispatcher(DispatchTable);
	if (bDebug_ && stat == FALSE)
	{
		// エラー発生
		if (fp) ::fprintf(fp, "error: %d (%d)\n", ::GetLastError(), __LINE__);
		if (fp) ::fflush(fp);
	}
#else
	// 子プロセスの準備が完了するまで待って exit する
	sigset_t ss;
	::sigemptyset(&ss);
	::sigaddset(&ss, SIGTERM);
	::sigaddset(&ss, SIGCHLD);
	::sigaddset(&ss, SIGPIPE);
	::sigaddset(&ss, SIGHUP);
	::sigaddset(&ss, SIGINT);
	::sigaddset(&ss, SIGQUIT);
	::sigaddset(&ss, SIGUSR1);
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
				// 正常な場合にはこの関数には来ない
		
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

		try
		{
			// Sydneyの初期化を行う
			Server::Singleton::RemoteServer::initialize(false, _regPath);
			
			// シグナル待機スレッドを起動する
			pthread_t th;
			if (::pthread_create(&th, 0, _SignalWaitFunction, 0) != 0)
			{
				fprintf(stderr, "ERROR: signal wait thread start up error.\n");
				::_exit(1);
			}
			::pthread_detach(th);
			
			// 親プロセスを停止する
			::kill(parentPid, SIGTERM);
			
			// 停止するまで待つ
			Server::Singleton::RemoteServer::join();
			// シグナル待機スレッドを停止する
//			::pthread_kill(th, SIGTERM);
			// 終了処理
			Server::Singleton::RemoteServer::terminate();
		}
		catch (...) {}
		
	} else {
		//エラー
		fprintf(stderr, "Can't fork process.\n");
	}
#endif
}

#ifdef SYD_OS_WINDOWS
//
//	FUNCTION private static
//	ServiceModule::serviceMain -- サービスから起動されるメイン関数
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ServiceModule::serviceMain(DWORD argc, LPTSTR* argv)
{
	_cServiceStatus.dwServiceType				= SERVICE_WIN32_OWN_PROCESS;
	_cServiceStatus.dwCurrentState				= SERVICE_STOPPED;
	_cServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP |
												  SERVICE_ACCEPT_SHUTDOWN;
	_cServiceStatus.dwWin32ExitCode				= 0;
	_cServiceStatus.dwServiceSpecificExitCode	= 0;
	_cServiceStatus.dwCheckPoint				= 0;
	_cServiceStatus.dwWaitHint					= 0;

	_TRACE("begin");

	// イベントを作成する
	_pStopEvent = ::CreateEvent(0, false, false, 0);
	if (_pStopEvent == NULL)
		return;
	_TRACE("CreateEvent");

	//通知
	reportStatus(SERVICE_START_PENDING);

	//サービスマネージャーにコントロールハンドラーを登録する

	//【注意】	SERVICE_WIN32_OWN_PROCESS
	//			のときのサービス名は無視されるので、
	//			_pServiceName を渡してもよい

	_pServiceStatusHandle = ::RegisterServiceCtrlHandler(
		_serviceName,
		(LPHANDLER_FUNCTION) ServiceModule::serviceHandler);

	if (_pServiceStatusHandle == 0)
		return;
	_TRACE("register service");

	//通知
	reportStatus(SERVICE_START_PENDING);

	int retryCount = 0;
	unsigned int _id;
	HANDLE hThread;

	// SyKernel.dllをロードする
	loadSydney();
	_TRACE("load SyKernel.dll");
	
	//通知
	reportStatus(SERVICE_START_PENDING);

 retry:
	hThread = (HANDLE)::_beginthreadex(0, 0, _SydneyInitialize, 0, 0, &_id);
	if (hThread == 0)
	{
		//通知
		reportStatus(SERVICE_START_PENDING);
		// スレッドの起動でエラーが発生した、リトライする
		if (++retryCount <= 2)
		{
			//通知
			reportStatus(SERVICE_START_PENDING);
			_TRACE("begin thread failed");
			goto retry;
		}
	}
	if (hThread)
	{
		_TRACE("begin thread");
		while (::WaitForSingleObject(hThread, 1000) == WAIT_TIMEOUT)
		{
			//通知
			reportStatus(SERVICE_START_PENDING);
			_TRACE("pending");
		}
		::CloseHandle(hThread);
	}
	else
	{
		// リトライしてもスレッドが起動できないので、
		// このスレッドで実行する

		_SydneyInitialize(0);
		_TRACE("initialize on this thread");
	}
	
	if (_status != 0)
	{
		// 初期化でエラーが発生した
		_TRACE("error occurred");
		return;
	}

	//通知
	reportStatus(SERVICE_RUNNING);
	_TRACE("trmeister running");

	//サービス停止イベントを待つ
	while (::WaitForSingleObject(_pStopEvent, 500) == WAIT_TIMEOUT)
	{
		//動作を確認する
		if ((*isRunningFunc)() == false)
			break;
	}

	//通知
	reportStatus(SERVICE_STOP_PENDING);

	hThread	= (HANDLE)::_beginthreadex(0, 0, _SydneyTerminate, 0, 0, &_id);
	while (::WaitForSingleObject(hThread, 1000) == WAIT_TIMEOUT)
	{
		//通知
		reportStatus(SERVICE_STOP_PENDING);
	}
	::CloseHandle(hThread);
	if (_status != 0)
		// 終了処理でエラーが発生した
		return;

	// イベントをクローズする
	::CloseHandle(_pStopEvent);

	// SyKernel.dll をアンロードする
	unloadSydney();

	//通知
	reportStatus(SERVICE_STOPPED);
}

//
//	FUNCTION private static
//	ServiceModule::serviceHandler -- サービスハンドラー
//
//	NOTES
//
//	ARGUMENTS
//	DWORD code
//		渡されるサービスの状態
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
void
ServiceModule::serviceHandler(DWORD code)
{
	switch (code)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		//サービス終了開始を報告する
		reportStatus(SERVICE_STOP_PENDING);
		//終了イベントを発生させる
		::SetEvent(_pStopEvent);
		break;
	case SERVICE_CONTROL_PARAMCHANGE:
		//パラメータを読み直す
		(*ServiceModule::reloadFunc)();
		// thru
	default:
		reportStatus(SERVICE_RUNNING);
	}
}

//
//	FUNCTION private static
//	ServiceModule::reportStatus -- サービスマネージャーにサービスの状態を報告する
//
//	NOTES
//
//	ARGUMENTS
//	DWORD state
//		サービスの状態
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ServiceModule::reportStatus(DWORD state)
{
	static DWORD dwCheckPoint = 1;

	_cServiceStatus.dwCurrentState = state;

	if (state == SERVICE_RUNNING || state == SERVICE_STOPPED)
		_cServiceStatus.dwCheckPoint = 0;
	else
		_cServiceStatus.dwCheckPoint = dwCheckPoint++;

	if (state == SERVICE_STOP_PENDING || state == SERVICE_START_PENDING)
		_cServiceStatus.dwWaitHint = 60*1000;
	else
		_cServiceStatus.dwWaitHint = 0;

	//サービスマネージャに報告
	::SetServiceStatus(_pServiceStatusHandle, &_cServiceStatus);
}

//	FUNCTION private static
//	ServiceModule::isRegisterService -- サービスとして登録されているか
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString&	serviceName
//		調べるサービス名
//
//	RETURN
//	bool
//		存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS

bool
ServiceModule::isRegisterService(const unsigned short* serviceName)
{
	bool result = false;

	//サービスマネージャーをオープンする

	if (SC_HANDLE manager = ::OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS)) {
		if (SC_HANDLE service =
			::OpenService(manager,
						  serviceName,
						  SERVICE_ALL_ACCESS)) {
			result = true;
			::CloseServiceHandle(service);
		}

		::CloseServiceHandle(manager);
	}

	return result;
}
#endif

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2013, 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
