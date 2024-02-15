// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Socket.h -- ソケットクラス
// 
// Copyright (c) 1999, 2002, 2006, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_SOCKET_H
#define __TRMEISTER_COMMUNICATION_SOCKET_H

#include "Communication/Module.h"
#include "Communication/SerialIO.h"
#include "ModSocket.h"
#include "ModCodec.h"


_TRMEISTER_BEGIN

namespace Communication
{
class CryptCodec;	// 暗号化対応
	
//
//	CLASS
//	Communication::Socket -- ソケットクラス
//	
//	NOTES
//	対ソケットの通信を行うクラス
//
class SYD_COMMUNICATION_FUNCTION Socket : public Communication::SerialIO
{
public:
	// プロトコルファミリー
	struct Family {
		enum Value {
			Unspec,		// IPv4 と IPv6 のどちらでも
			
			Only_v4,	// IPv4 のみ
			Only_v6		// IPv6 のみ
		};
	};
	
	//コンストラクタ(2) (クライアント用)
	Socket(const ModString& cstrHostName_,
		   int iPort_,
		   Family::Value eFamily = Family::Unspec,
		   int iBufferSize_ = 4096);
	//デストラクタ
	virtual ~Socket();

	//オープンする
	void open();
	//クローズする
	void close();

	//オブジェクトを読み込む
	Common::Externalizable* readObject();
	//オブジェクトを書き出す
	void writeObject(const Common::Externalizable* pObject_);

	//Integerを読み込む
	int readInteger();
	//Integerを書き出す
	void writeInteger(int iValue_);

	//出力をflushする
	void flush();

	//入力が来るまで待つ
	bool wait(int iMilliseconds_);

	// 共通鍵設定(暗号化対応)
	void	setKey(const CryptKey::Pointer& pKey_);
	// 共通鍵取得(暗号化対応)
	const CryptKey::Pointer& getKey();

	// IPv6か否か
	bool isIPv6() const { return m_pSocket->isIPv6(); }

	//コンストラクタ(1) (サーバ用)
	Socket(ModServerSocket* pServerSocket_, CryptCodec* pCodec_);

	// パラメータから利用プロトコルを得る
	static Family::Value getFamily();
	static Family::Value getCryptFamily();

private:
	//ソケットクラス
	ModSocketBase* m_pSocket;
	//バッファリング用のコーデック
	CryptCodec* m_pCodec;	// 暗号化対応
	//バッファサイズ
	int m_iBufferSize;
	//サーバかどうか
	bool m_bServer;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_SOCKET_H

//
//	Copyright (c) 1999, 2002, 2006, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
