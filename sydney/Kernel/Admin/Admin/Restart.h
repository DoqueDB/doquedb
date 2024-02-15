// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Restart.h -- システムの再起動関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_RESTART_H
#define	__SYDNEY_ADMIN_RESTART_H

#include "Admin/Module.h"

_SYDNEY_BEGIN
_SYDNEY_ADMIN_BEGIN

//	NAMESPACE
//	Admin::Restart -- 再起動処理に関する名前空間
//
//	NOTES

namespace Restart
{
	// システムの再起動時にシステムを必要があれば、回復する
	SYD_ADMIN_FUNCTION
	bool					recover(int iRetry_);
}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_RESTART_H

//
// Copyright (c) 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
