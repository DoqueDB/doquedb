// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Timeout.h -- ロックの待ち時間制限関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_LOCK_TIMEOUT_H
#define	__SYDNEY_LOCK_TIMEOUT_H

#include "Lock/Module.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

namespace Timeout
{
	//	TYPEDEF
	//	Lock::Timeout::Value --	ロック待ち時間を表す値の型
	//
	//	NOTES

	typedef	unsigned int	Value;

	//	CONST
	//	Lock::Timeout::Unlimited -- 無制限に待つことを表す値
	//
	//	NOTES

	const Value				Unlimited = ~static_cast<Value>(0);
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_TIMEOUT_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
