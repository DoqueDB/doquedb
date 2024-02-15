// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MemoryDaemon.cpp -- メモリーデーモン
// 
// Copyright (c) 1999, 2001, 2011, 2023 Ricoh Company, Ltd.
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
#include "Communication/MemoryDaemon.h"
#include "Communication/ConnectionKeeper.h"
#include "Communication/LocalMemory.h"
#include "Communication/Memory.h"
#include "Common/Message.h"

_TRMEISTER_USING

using namespace Communication;

//
//	VARIABLE private
//	Communication::MemoryDaemon::m_pClientLocalMemory
//				-- クライアントが最初の接続の時に使用するローカルメモリ
//	NOTES
//	クライアントが最初の接続の時に使用するローカルメモリ
//
LocalMemory*
MemoryDaemon::m_pClientLocalMemory = 0;

//
//	VARIABLE private
//	Communication::MemoryDaemon::m_pServerLocalMemory
//				-- サーバが最初の接続の時に使用するローカルメモリ
//	NOTES
//	サーバが最初の接続の時に使用するローカルメモリ
//
LocalMemory*
MemoryDaemon::m_pServerLocalMemory = 0;

//
//	FUNCTION public
//	Communication::MemoryDaemon::MemoryDaemon -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Communication::ConnectionKeeper* pKeeper_
//		接続プールクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MemoryDaemon::MemoryDaemon(ConnectionKeeper* pKeeper_)
: m_pKeeper(pKeeper_)
{
}

//
//	FUNCTION public
//	Communication::MemoryDaemon::~MemoryDaemon -- デストラクタ
//
//	NTOES
//	デストラクタ
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
MemoryDaemon::~MemoryDaemon()
{
}

//
//	FUNCTION public
//	Communication::MemoryDaemon::initialize -- 初期化
//
//	NOTES
//	初期化を行う。ローカルメモリを確保する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MemoryDaemon::initialize()
{
	Memory* pInput = new Memory;
	Memory* pOutput = new Memory;
	m_pServerLocalMemory = new LocalMemory(pInput, pOutput);
	m_pClientLocalMemory = new LocalMemory(pOutput, pInput);
	m_pServerLocalMemory->open();
	m_pClientLocalMemory->open();
}

//
//	FUNCTION public
//	Communication::MemoryDaemon::terminate -- 後処理
//
//	NOTES
//	後処理を行う。Modのサーバソケットを解放する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MemoryDaemon::terminate()
{
	//スレッドを終了する
	abort();
	join();
	//ローカルメモリ
	m_pServerLocalMemory->close();
	m_pClientLocalMemory->close();
	delete m_pServerLocalMemory;
	delete m_pClientLocalMemory;
}

//
//	FUNCTION public static
//	Communication::MemoryDaemon::getClientLocalMemory
//					-- クライアントが最初に接続するローカルメモリを得る
//
//	NOTES
//	クライアントが最初の接続するローカルメモリを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::LocalMemory*
//		ローカルメモリのクライアント側
//
//	EXCEPTIONS
//	なし
//
LocalMemory*
MemoryDaemon::getClientLocalMemory()
{
	return m_pClientLocalMemory;
}

//
//	FUNCTION private
//	Communication::MemoryDaemon::runnable -- スレッドとして起動される関数
//
//	NOTES
//	スレッドとして起動される関数。acceptする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MemoryDaemon::runnable()
{
#ifdef DEBUG
	SydDebugMessage << "Communication::MemoryDaemon is running." << ModEndl;
#endif
	while (1)
	{
		while (m_pServerLocalMemory->wait(500) != true)
		{
			if (isAborted() == true)
			{
				//終了指示
				setStatus(Common::Thread::Aborted);
				m_pKeeper->execute(0, 0);
				return;
			}
		}
		//クライアントからの接続要求が来た
		int dummy = m_pServerLocalMemory->readInteger();
		Memory* pInput = new Memory;
		Memory* pOutput = new Memory;
		LocalMemory* pServerMemory = new LocalMemory(pInput, pOutput);
		LocalMemory* pClientMemory = new LocalMemory(pOutput, pInput);
		//ローカルなのでポインタをそのまま送る
		m_pServerLocalMemory->writePointer(pClientMemory);
		m_pServerLocalMemory->flush();
		m_pKeeper->execute(pServerMemory, 0);
	}
}

//
//	Copyright (c) 1999, 2001, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
