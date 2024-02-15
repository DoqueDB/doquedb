// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServiceModule.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDSERVER_SERVICEMODULE_H
#define __SYDNEY_SYDSERVER_SERVICEMODULE_H

//
//	CLASS
//	SydServer::ServiceModule --
//
//	NOTES
//
//
class ServiceModule 
{
public:
#ifdef SYD_OS_WINDOWS
	//サービスに登録する
	static void
	registerService(const unsigned short* regPath,
					const unsigned short* serviceName,
					const unsigned short* displayName,
					const unsigned short* userName,
					const unsigned short* password,
					const unsigned short* description);
	//サービスから削除する
	static void unRegisterService(const unsigned short* serviceName);

	//SyKernel.dllをロードする
	static bool loadSydney();
	//SyKernel.dllをアンロードする
	static void unloadSydney();

	//初期化関数
	static void (*initializeFunc)(bool, const unsigned short*);
	//終了関数
	static void (*terminateFunc)(void);
	//停止関数
	static void (*stopFunc)(void);
	//停止待機関数
	static void (*joinFunc)(void);
	//起動チェック関数
	static bool (*isRunningFunc)(void);
	//パラメータ再読み込み関数
	static void (*reloadFunc)(void);
#endif

	//サービスを開始する
	static void startService(const unsigned short* regPath,
							 bool bDebug_ = false);

private:
#ifdef SYD_OS_WINDOWS
	//サービスのメイン関数
	static void serviceMain(DWORD argc, LPTSTR* argv);

	//サービスハンドラー
	static void serviceHandler(DWORD code);
	//サービスの状態を報告する
	static void reportStatus(DWORD state);

	//サービスに登録されているか
	static bool isRegisterService(const unsigned short* serviceName);
#endif
};

#endif //__SYDNEY_SYDSERVER_SERVICEMODULE_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
