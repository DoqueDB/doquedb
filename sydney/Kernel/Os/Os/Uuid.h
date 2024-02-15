//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Uuid.h -- UUIDを表すクラス
// 
// Copyright (c) 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_UUID_H
#define	__TRMEISTER_OS_UUID_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <rpc.h>
#endif
#ifdef SYD_OS_POSIX
// #include <uuid/uuid.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//
//	CLASS
//	Os::Uuid -- UUIDを表すクラス
//
//	NOTES
//	
class SYD_OS_FUNCTION Uuid
{
public:
	//【注意】	UUID自体は16バイトのIDであるが、WindowsとLinuxとでは
	//			構造体が異なっている。(ヒット列は同じなのかもしれないが...)
	//			WindowsとLinux間でファイルの互換性を維持するため、
	//			バイナリとして直接ファイル等でやり取りするメソッドは用意しない
	//
	//			ファイルへの保存は文字列形式(36バイト)で行うことを想定
	
	// コンストラクター
	Uuid();
	// デストラクター
	virtual ~Uuid();

	// 新しくUUIDを生成し、自身に設定する
	void generate();

	// 比較する
	int compare(const Uuid& cOther_);

	// 文字列からUUID設定する
	void parse(const char* pBuffer_);
	// 文字列で取り出す
	void unparse(char* pBuffer_);

private:
#ifdef SYD_OS_WINDOWS
	UUID	m_cUUID;
#endif
#ifdef SYD_OS_POSIX
	uuid_t	m_cUUID;
#endif
};

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_UUID_H

//
// Copyright (c) 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
