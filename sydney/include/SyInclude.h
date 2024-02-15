// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyInclude.h --
//		必要な OS のインクルードファイルを一挙にインクルードする
// 
// Copyright (c) 2000, 2003, 2007, 2011, 2013, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef	__SY_INCLUDE_H
#define	__SY_INCLUDE_H

#ifdef SYD_OS_WINDOWS

// winsock2 を利用する
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Os/Assert.h
#include <assert.h>
// Os/AsyncStatus.h
#include <windows.h>
// Os/AutoCriticalSection.h
// なし
// Os/AutoEvent.h
// なし
// Os/AutoRWLock.h
// なし
// Os/AutoSemaphore.h
// なし
// Os/CriticalSection.h
#include <windows.h>
// Os/Event.h
#include <windows.h>
// Os/Host.h
// なし
// Os/InterlockedVariable.h
#include <windows.h>
// Os/Limits.h
#include <limits.h>
#include <float.h>
// Os/Math.h
#include <math.h>
// Os/MiniDump.h
// なし
// Os/Memory.h
#include <windows.h>
#ifdef OBSOLETE
// Os/Mutex.h
#include <windows.h>
#endif
// Os/Path.h
// なし
// Os/Process.h
#include <windows.h>
#include <io.h>
// Os/RWLock.h
// なし
// Os/Semaphore.h
#include <windows.h>
// Os/SysConf.h
// なし
// Os/Thread.h
#include <windows.h>
// Os/ThreadLocalStorage.h
#include <windows.h>
// Os/Timer.h
// なし
// Os/Unicode.h
// なし
// Os/Uuid.h
#include <rpc.h>
#endif

#ifdef SYD_OS_POSIX
// Os/Assert.h
#include <assert.h>
// Os/AsyncStatus.h
// なし
// Os/AutoCriticalSection.h
// なし
// Os/AutoEvent.h
// なし
// Os/AutoRWLock.h
// なし
// Os/AutoSemaphore.h
// なし
// Os/CriticalSection.h
#include <pthread.h>
// Os/Event.h
#include <pthread.h>
// Os/File.h
#include <sys/uio.h>
// Os/Host.h
// なし
// Os/InterlockedVariable.h
// なし
// Os/Limits.h
#include <limits.h>
#include <float.h>
// Os/Math.h
#include <math.h>
// Os/Memory.h
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
// Os/MiniDump.h
// なし
#ifdef OBSOLETE
// Os/Mutex.h
// なし
#endif
// Os/Path.h
// なし
// Os/Process.h
#include <unistd.h>
#include <sys/stat.h>
// Os/RWLock.h
#ifdef SYD_OS_SOL2_7
#include <pthread.h>
#endif
// Os/Semaphore.h
#include <semaphore.h>
// Os/SysConf.h
// なし
// Os/Thread.h
#include <pthread.h>
// Os/ThreadLocalStorage.h
#include <pthread.h>
// Os/Timer.h
#include <time.h>
// Os/Unicode.h
// なし
// Os/Uuid.h
#include <uuid/uuid.h>
#endif

#endif

//
// Copyright (c) 2000, 2003, 2007, 2011, 2013, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
