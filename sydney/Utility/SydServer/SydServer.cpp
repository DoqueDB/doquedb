// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydServer.cpp --
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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
#include "SydServer/ServiceModule.h"
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <wchar.h>
#else
#include "Server/Singleton.h"
#include "Common/UnicodeString.h"
#endif
#include <iostream>
using namespace std;

#ifndef SYD_OS_WINDOWS
#include "ModMemoryPool.h"
#include "ModAutoPointer.h"
#include "ModCharTrait.h"
#endif

#ifndef SYD_OS_WINDOWS
_SYDNEY_USING
#endif

#ifdef SYD_OS_WINDOWS
void local(const unsigned short* path)
{
	try
	{
		if (ServiceModule::loadSydney() == false)
			return;
		
		(*ServiceModule::initializeFunc)(false, path);
		(*ServiceModule::joinFunc)();
		(*ServiceModule::terminateFunc)();
	}
	catch (...) {}
	ServiceModule::unloadSydney();
}
void install(const unsigned short* path)
{
	try
	{
		if (ServiceModule::loadSydney() == false)
			return;
		
		(*ServiceModule::initializeFunc)(true, path);
		(*ServiceModule::terminateFunc)();
	}
	catch (...) {}
	ServiceModule::unloadSydney();
}

bool compare(const unsigned short* s1, const unsigned short* s2)
{
	while (*s1 && *s2)
	{
		unsigned short c1 = towupper(*s1++);
		unsigned short c2 = towupper(*s2++);
		if (c1 != c2) return false;
	}
	return (*s1 == 0 && *s2 == 0);
}

#else
void local(const ModUnicodeString& path)
{
	try
	{
		Server::Singleton::RemoteServer::initialize(false, path);
		Server::Singleton::RemoteServer::join();
		Server::Singleton::RemoteServer::terminate();
	}
	catch (...) {}
}
void install(const ModUnicodeString& path)
{
	try
	{
		Server::Singleton::RemoteServer::initialize(true, path);
		Server::Singleton::RemoteServer::terminate();
	}
	catch (...) {}
}
#endif

void USAGE()
{
	cout << endl;
	cout << "Usage: SydServer [options]" << endl;
	cout << endl;
	cout << "where options include:" << endl;
#ifdef SYD_OS_WINDOWS
	cout << "    /Local         run local server." << endl;
	cout << "    /Install       install TRMeister." << endl;
#ifdef SYD_OS_WINNT4_0
	cout << "    /RegService    register service." << endl;
	cout << "    /UnRegService  unregister service." << endl;
	cout << "    /ServiceName   manipulate this name of service" << endl;
	cout << "                   (default: 'SydServer')." << endl;
	cout << "    /DisplayName   this is display name for registering service"
		 << endl;
	cout << "                   (default: 'TRMeister')." << endl;
	cout << "    /UserName      service executing user." << endl;
	cout << "                   (format: DomainName\\UserName," << endl;
	cout << "                    default: LocalSystem)." << endl;
	cout << "    /Password      user password." << endl;
	cout << "    /Description   this is description for registering service"
		 << endl;
	cout << "                   (default: 'Ricoh Database System')." << endl;
#endif
	cout << "    /RegistryPath  set registry path (default: 'HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister')." << endl;
#else
	cout << "    -local         run local server." << endl;
	cout << "    -install       install TRMeister." << endl;
#endif
}

#ifdef SYD_OS_WINDOWS

FILE* fp = 0;

#define _TRACE(message)	if (fp) ::fprintf(fp, message " (%d)\n", __LINE__), ::fflush(fp);

int
wmain(int argc, wchar_t* argv[])
{
#ifdef TRACE_LOG
	fp = ::fopen("c:\\temp\\sydney-service.log", "a");
#endif
	_TRACE("begin wmain");
	
#if !defined(DEBUG) && !defined(QUANTIFY)

	// 一般保護違反メッセージボックスを表示しないようにする

	const UINT oldMode = SetErrorMode(0);
	(void) SetErrorMode(oldMode | SEM_NOGPFAULTERRORBOX);

	_TRACE("SetErrorMode");
#endif

	enum {
		LOCAL = 1,
		INSTALL,
		REG_SERVICE,
		UN_REG_SERVICE,
		SERVICE
	};

#ifndef SYD_OS_WINNT4_0
	int flag = LOCAL;
#else
	int flag = SERVICE;
	bool bDebug = false;
#endif
	const unsigned short* regPath = L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister";
#ifdef SYD_OS_WINNT4_0
	const unsigned short* serviceName = 0;
	const unsigned short* displayName = 0;
	const unsigned short* description = 0;
	const unsigned short* userName = 0;
    const unsigned short* password = 0;
#endif

	for (int i = 1; i < argc; ++i)
	{
		wchar_t* p = argv[i];
		if (*p == L'/' || *p == L'-')
		{
			p++;
			//オプション指定
			if (compare(p, L"Local"))
			{
				flag = LOCAL;
			}
			else if (compare(p, L"Install"))
			{
				flag = INSTALL;
			}
#ifdef SYD_OS_WINNT4_0
			else if (compare(p, L"RegService"))
			{
				flag = REG_SERVICE;
			}
			else if (compare(p, L"UnRegService"))
			{
				flag = UN_REG_SERVICE;
			}
#endif
			else if (compare(p, L"RegistryPath"))
			{
				regPath = argv[++i];
			}
#ifdef SYD_OS_WINNT4_0
			else if (compare(p, L"ServiceName"))
			{
				serviceName = argv[++i];
			}
			else if (compare(p, L"DisplayName"))
			{
				displayName = argv[++i];
			}
			else if (compare(p, L"UserName"))
			{
				userName = argv[++i];
			}
			else if (compare(p, L"Password"))
			{
				password = argv[++i];
			}
			else if (compare(p, L"Description"))
			{
				description = argv[++i];
			}
			else if (compare(p, L"Trace"))
			{
				bDebug = true;
			}
#endif
			else {
				USAGE();
				return 1;
			}
		}
	}

	_TRACE("option");
	
	switch (flag)
	{
	case LOCAL:
		//ローカルサーバとして実行する
		_TRACE("begin local");
		local(regPath);
		break;
	case INSTALL:
		_TRACE("begin install");
		install(regPath);
		break;
#ifdef SYD_OS_WINNT4_0
	case REG_SERVICE:
		//サービスを登録する
		_TRACE("begin registerService");
		ServiceModule::registerService(
			regPath, serviceName, displayName, userName, password, description);
		break;
	case UN_REG_SERVICE:
		//サービスから削除する
		_TRACE("begin unRegisterService");
		ServiceModule::unRegisterService(serviceName);
		break;
	case SERVICE:
		//サービスとして起動する
		_TRACE("begin startService");
		ServiceModule::startService(regPath, bDebug);
#endif
	}

	if (fp) ::fclose(fp);

	return 0;
}
#else
int
main(int argc, char* argv[])
{
	ModMemoryPool::setTotalLimit(ModSizeMax >> 10); // KB単位
	ModOs::Process::setEncodingType(Common::LiteralCode);

	enum {
		LOCAL = 1,
		INSTALL,
		DAEMON
	};

	int flag = DAEMON;
	ModUnicodeString regPath;

	for (int i = 1; i < argc; ++i)
	{
		char* p = argv[i];
		if (*p == L'-')
		{
			p++;
			//オプション指定
			if (ModCharTrait::compare(p, "local", ModFalse) == 0)
			{
				flag = LOCAL;
			}
			else if (ModCharTrait::compare(p, "install", ModFalse) == 0)
			{
				flag = INSTALL;
			}
			else {
				USAGE();
				return 1;
			}
		}
	}

	switch (flag)
	{
	case LOCAL:
		// ローカルサーバとして実行する
		local(regPath);
		break;
	case INSTALL:
		// 
		install(regPath);
		break;
	case DAEMON:
		// デーモンとして起動する
		ServiceModule::startService(regPath);
	}

	return 0;
}
#endif

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
