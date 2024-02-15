// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ID.h -- トランザクション識別子関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_ID_H
#define	__SYDNEY_TRANS_ID_H

#include "Trans/Module.h"
#include "Trans/TimeStamp.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

//	TYPEDEF
//	Trans::ID -- トランザクション識別子を表す型
//
//	NOTES

typedef	TimeStamp			ID;

//	CONST
//	Trans::IllegalID -- 不正なトランザクション識別子
//
//	NOTES

const ID					IllegalID;

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_ID_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
