// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServerConnection.h -- サーバメモリコネクション
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_SERVERCONNECTION_H
#define __TRMEISTER_COMMUNICATION_SERVERCONNECTION_H

#include "Communication/Module.h"
#include "Communication/Connection.h"
#include "Os/CriticalSection.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::ServerConnection -- サーバコネクション
//
//	NOTES
//	サーバメモリコネクション。
//
class SYD_COMMUNICATION_FUNCTION ServerConnection : public Connection
{
public:
	//コンストラクタ
	ServerConnection(int iMasterID_, int iSlaveID_);
	//デストラクタ
	~ServerConnection();

	//オープンする
	void open();
	//クローズする
	void close();

	//sync
	void sync();

	//使いまわし排他用のクリティカルセクションを得る
	Os::CriticalSection& getLockObject() { return m_cCriticalSection; }

private:
	//使いまわし排他用
	Os::CriticalSection m_cCriticalSection;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_SERVERCONNECTION_H

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
