// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServerSocket.h -- サーバ側のソケットクラス
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_SERVERSOCKET_H
#define __TRMEISTER_COMMUNICATION_SERVERSOCKET_H

#include "Communication/Module.h"
#include "Communication/Socket.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::ServerSocket -- ソケットクラス
//	
//	NOTES
//	対ソケットの通信を行うクラス
//
class ServerSocket : public Socket
{
public:
	//コンストラクタ
	ServerSocket(ModServerSocket* pServerSocket_, CryptCodec* pCodec_);	// 暗号化対応
	//デストラクタ
	virtual ~ServerSocket();

	//クローズする
	void close();
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_SERVERSOCKET_H

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
