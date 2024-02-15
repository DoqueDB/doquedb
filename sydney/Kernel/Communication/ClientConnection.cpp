// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ClientConnection.cpp -- クライアントメモリコネクション
// 
// Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Communication/ClientConnection.h"
#include "Common/Request.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
using namespace Communication;

//
//	FUNCTION public
//	Communication::ClientConnection::ClientConnection
//		-- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	int iMasterID_
//		マスタID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ClientConnection::ClientConnection(int iMasterID_, int iSlaveID_)
	: Connection(iMasterID_, iSlaveID_)
{
}

//
//	FUNCTION public
//	Communication::ClientConnection::~ClientConnection
//		-- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ClientConnection::~ClientConnection()
{
}

//
//	FUNCTION public
//	Communication::ClientConnection::sync -- 相手との同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ClientConnection::sync()
{
//	syncは遅いのでやめた
//
//	//クライアント側なのでsyncのリクエストを受け取る
//
//	//サーバから受け取る
//	while (1)
//	{
//		ModAutoPointer<Common::Externalizable> pObject = readObject();
//		Common::Request* pRequest
//			= dynamic_cast<Common::Request*>(pObject.get());
//		if (pRequest)
//		{
//			if (pRequest->getCommand() == Common::Request::Sync)
//				break;
//		}
//	}
//
//	//サーバに送る
//	const Common::Request request(Common::Request::Sync);
//	writeObject(&request);
//	flush();
}

//
//	Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
