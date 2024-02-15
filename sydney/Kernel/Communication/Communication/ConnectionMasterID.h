// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionMasterID.h -- マスターIDを管理する
// 
// Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CONNECTIONMASTERID_H
#define __TRMEISTER_COMMUNICATION_CONNECTIONMASTERID_H

#include "Communication/Module.h"
#include "ModCriticalSection.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::ConnectionMasterID -- マスターIDを管理する
//
//	NOTES
//	コネクションのマスターIDとはプロトコルバージョンである。
//
class ConnectionMasterID
{
public:
	//マスターIDが正当なものかチェックする
	SYD_COMMUNICATION_FUNCTION static bool isNormal(int iID_);

	//特殊なマスターID
	enum {
		Minimum		= 0,			//最小値
		Maximum		= 0x7fffffff,	//最大値
		Undefined	= 0xffffffff	//未定義をあらわす数
	};
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CONNECTIONMASTERID_H

//
//	Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
