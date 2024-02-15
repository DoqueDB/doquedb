// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiniDump.h -- ミニダンプ関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_MINIDUMP_H
#define	__TRMEISTER_OS_MINIDUMP_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#include "Os/Module.h"
#ifdef SYD_OS_WINDOWS
#include "Os/File.h"
#endif

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

#ifdef SYD_OS_WINDOWS
namespace Manager
{
	class MiniDumper;
}
class Path;

//	CLASS
//	Os::MiniDump -- ミニダンプを表すクラス
//
//	NOTES

class MiniDump
{
	friend class Manager::MiniDumper;
public:
	// 例外情報を含まないミニダンプを生成する
	SYD_OS_FUNCTION
	static bool
	create(const Path& path, File::Permission::Value permission);

private:
	// 例外情報を含むミニダンプを生成する
	static bool
	create(const Path& path, File::Permission::Value permission,
		   EXCEPTION_POINTERS* ep);
};
#endif

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_MINIDUMP_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
