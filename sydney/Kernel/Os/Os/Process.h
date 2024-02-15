// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Process.h -- プロセス関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_PROCESS_H
#define	__TRMEISTER_OS_PROCESS_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
// #include <io.h>
#endif
#ifdef SYD_OS_POSIX
// #include <unistd.h>
// #include <sys/stat.h>
#endif

#include "Os/Module.h"
#include "Os/Path.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	NAMESPACE
//	Os::Process -- プロセスを表す名前空間
//
//	NOTES

namespace Process
{
	//	TYPEDEF
	//	Os::Process::ID -- プロセス ID を表す型
	//
	//	NOTES
#ifdef SYD_OS_WINDOWS
	typedef	DWORD			ID;
#endif
#ifdef SYD_OS_POSIX
	typedef	pid_t			ID;
#endif
	// 呼び出したプロセスのプロセス ID を得る
	inline
	ID						self();
	// カレントワーキングディレクトリを得る
	SYD_OS_FUNCTION
	Path					getCurrentDirectory();
	// UMASK を変更する
	inline
	unsigned int			umask(unsigned int mask);
}

//	FUNCTION public
//	Os::Process::self -- 自分自身のプロセス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身のプロセス ID
//
//	EXCEPTIONS
//		なし

inline
Process::ID
Process::self()
{
#ifdef SYD_OS_WINDOWS
	return ::GetCurrentProcessId();
#endif
#ifdef SYD_OS_POSIX
	return ::getpid();
#endif
}

//	FUNCTION public
//	Os::Process::umask -- プロセスのファイルアクセス権マスクを設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		mask
//			新しいファイルアクセス権マスク
//
//	RETURN
//		設定前のファイルアクセス権マスク
//
//	EXCEPTIONS
//		なし

inline
unsigned int
Process::umask(unsigned int mask)
{
#ifdef SYD_OS_WINDOWS
	return ::_umask(static_cast<int>(mask));
#endif
#ifdef SYD_OS_POSIX
	return ::umask(static_cast<mode_t>(mask));
#endif
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_PROCESS_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
