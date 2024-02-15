// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Record.cpp -- DLL のエントリ関数
// 
// Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#ifdef SYD_OS_WINNT4_0
#include <windows.h>

BOOL WINAPI DllMain(
	HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:

		// この DLL には DLL_THREAD_ATTACH および
		// DLL_THREAD_DETACH の通知がされないようにする
		//
		//【注意】	エラーは無視する

		(void) ::DisableThreadLibraryCalls(hModule);
		// thru

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
    }

    return TRUE;
}
#endif

//
//	Copyright (c) 2000, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
