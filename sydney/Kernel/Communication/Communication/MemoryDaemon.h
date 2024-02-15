// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MemoryDaemon.h -- ソケットデーモン
// 
// Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_MEMORYDAEMON_H
#define __TRMEISTER_COMMUNICATION_MEMORYDAEMON_H

#include "Communication/Module.h"
#include "Common/Thread.h"

_TRMEISTER_BEGIN

namespace Communication
{
class ConnectionKeeper;
class LocalMemory;

//
//	CLASS
//	Communication::MemoryDaemon -- メモリーデーモン
//
//	NOTES
//	クライアントからの接続を処理するメモリーデーモンクラス。
//
class MemoryDaemon : public Common::Thread
{
public:
	//コンストラクタ
	MemoryDaemon(ConnectionKeeper* pKeeper_);
	//デストラクタ
	~MemoryDaemon();

	//初期化
	void initialize();
	//後処理
	void terminate();

	//クライアント用のローカルメモリを得る
	static LocalMemory* getClientLocalMemory();

private:
	//スレッドで起動されるメソッド
	void runnable();
	
	//接続プールクラス
	ConnectionKeeper* m_pKeeper;
	//ローカルメモリ
	static LocalMemory* m_pClientLocalMemory;
	static LocalMemory* m_pServerLocalMemory;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_MEMORYDAEMON_H

//
//	Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
