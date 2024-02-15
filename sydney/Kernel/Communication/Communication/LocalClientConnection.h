// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocalClientConnection.h -- クライアントメモリコネクション
// 
// Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_LOCALCLIENTCONNECTION_H
#define __TRMEISTER_COMMUNICATION_LOCALCLIENTCONNECTION_H

#include "Communication/ClientConnection.h"
#include "Os/CriticalSection.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::LocalClientConnection -- クライアントメモリコネクション
//
//	NOTES
//	クライアントメモリコネクション。
//
class SYD_COMMUNICATION_FUNCTION LocalClientConnection : public ClientConnection
{
public:
	//コンストラクタ
	LocalClientConnection(int iMasterID_, int iSlaveID_);
	//デストラクタ
	virtual ~LocalClientConnection();

	//オープン
	void open();
	//クローズ
	void close();

private:
	//クリティカルセクション
	static Os::CriticalSection m_cCriticalSection;
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMUNICATION_LOCALCLIENTCONNECTION_H

//
//	Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
