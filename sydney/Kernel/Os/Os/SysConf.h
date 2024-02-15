// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SysConf.h -- システム設定関連のクラス定義、関数宣言
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

#ifndef __TRMEISTER_OS_SYSCONF_H
#define	__TRMEISTER_OS_SYSCONF_H

#include "Os/Module.h"
#include "Os/Host.h"
#include "Os/Memory.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

namespace SysConf
{
	namespace HostName
	{
		// ホスト名を得る
		SYD_OS_FUNCTION
		const Host::Name&	get();
	}

	namespace OpenMax
	{
		// プロセスがオープンできるファイル数の最大値を得る
		SYD_OS_FUNCTION
		unsigned int		get();
	}

	namespace PageSize
	{
		// メモリページサイズを得る
		SYD_OS_FUNCTION
		Memory::Size		get();
	}
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_SYSCONF_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
