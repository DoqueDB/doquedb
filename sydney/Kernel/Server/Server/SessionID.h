// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SessionID.h -- セッション ID 関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SESSION_SESSIONID_H
#define __SYDNEY_SESSION_SESSIONID_H

#include "Server/Module.h"
#include "Server/Type.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

//	TYPEDEF
//	Session::SessionID -- セッション ID を表す型
//
//	NOTES

typedef ID					SessionID;

//	CONST
//	Session::IllegalID -- 不正なセッション ID
//
//	NOTES

const SessionID				IllegalSessionID = 0xffffffff;

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SESSION_SESSIONID_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
