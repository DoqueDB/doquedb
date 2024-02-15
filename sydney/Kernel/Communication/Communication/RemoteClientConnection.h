// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RemoteClientConnection.h -- クライアントソケットコネクション
// 
// Copyright (c) 1999, 2002, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_REMOTECLIENTCONNECTION_H
#define __TRMEISTER_COMMUNICATION_REMOTECLIENTCONNECTION_H

#include "Communication/ClientConnection.h"
#include "Communication/Socket.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::RemoteClientConnection -- クライアントソケットコネクション
//
//	NOTES
//	クライアントソケットコネクション。
//
class SYD_COMMUNICATION_FUNCTION RemoteClientConnection
	: public ClientConnection
{
public:
	//コンストラクタ
	RemoteClientConnection(const ModUnicodeString& cstrHostName_,
						   int iPortNumber_,
						   int iMasterID_, int iSlaveID_,
						   Socket::Family::Value eDefaultFamily
						   = Socket::Family::Unspec);
	//デストラクタ
	virtual ~RemoteClientConnection();

	//オープン
	void open();
	//クローズ
	void close();

private:
	//ホスト名
	ModUnicodeString m_cstrHostName;
	//ポート番号
	int m_iPortNumber;
	//デフォルトのプロトコルファミリー
	Socket::Family::Value m_eDefaultFamily;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_REMOTECLIENTCONNECTION_H

//
//	Copyright (c) 1999, 2002, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
