// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SocketDaemon.h -- ソケットデーモン
// 
// Copyright (c) 1999, 2001, 2006, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_SOCKETDAEMON_H
#define __TRMEISTER_COMMUNICATION_SOCKETDAEMON_H

#include "Communication/Module.h"
#include "Communication/Socket.h"

#include "Common/Thread.h"

#include "ModSocket.h"
#include "ModCodec.h"

_TRMEISTER_BEGIN

namespace Communication
{
class ConnectionKeeper;
class CryptCodec;	// 暗号化対応

//
//	CLASS
//	Communication::SocketDaemon -- ソケットデーモン
//
//	NOTES
//	クライアントからの接続を処理するソケットデーモンクラス。
//
class SocketDaemon : public Common::Thread
{
public:
	//コンストラクタ
	SocketDaemon(ConnectionKeeper* pKeeper_);
	//デストラクタ
	~SocketDaemon();

	//初期化
	void initialize();
	//後処理
	void terminate();

	// ポート番号を得る
	static int getPortNumber();
	static int getCryptPortNumber();

private:
	//スレッドで起動されるメソッド
	void runnable();
	
	//コーデックを確保する
	CryptCodec* allocateCodec();	// 暗号化対応
	
	//接続プールクラス
	ConnectionKeeper* m_pKeeper;
	//バッファサイズ
	int m_iBufferSize;

	//サーバソケット
	ModServerSocket* m_pServerSocket;
	//コーデック
	CryptCodec* m_pCodec;	// 暗号化対応
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_SOCKETDAEMON_H

//
//	Copyright (c) 1999, 2001, 2006, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
