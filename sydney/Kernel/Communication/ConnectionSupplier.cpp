// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionSupplier.cpp -- シリアルIOのデーモンを管理するクラス
// 
// Copyright (c) 1999, 2001, 2003, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Communication/Connection.h"
#include "Communication/ConnectionKeeper.h"
#include "Communication/ConnectionSlaveID.h"
#include "Communication/ConnectionSupplier.h"
#include "Communication/MemoryDaemon.h"
#include "Communication/SocketDaemon.h"

#include "Common/Configuration.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"

_TRMEISTER_USING

using namespace Communication;

//
//	VARIABLE private
//	Communication::ConnectionSupplier::m_pSocketDaemon
//												-- ソケットデーモン
//
//	NOTES
//	Communication::RemoteClientConnectionからの接続要求を受け付けるための
//  ソケットデーモンクラス
//
SocketDaemon*
ConnectionSupplier::m_pSocketDaemon = 0;

//
//	VARIABLE private
//	Communication::ConnectionSupplier::m_pMemoryDaemon
//												-- メモリーデーモン
//
//	NOTES
//	Communication::LocalClientConnectionからの接続要求を受け付けるための
//  メモリーデーモンクラス
//
MemoryDaemon*
ConnectionSupplier::m_pMemoryDaemon = 0;

//
//	VARIABLE private
//	Communication::ConnectionSupplier::m_pConnectionKeeper
//												-- コネクションキーパー
//
//	NOTES
//	アクセプトしたシリアルIOをキューに保存するためのキーパークラス
//
ConnectionKeeper*
ConnectionSupplier::m_pConnectionKeeper = 0;

//
//	FUNCTION public static
//	Communication::ConnectionSupplier::initialize -- 初期化
//
//	NOTES
//	初期化。シリアルIOデーモンを起動する。
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
ConnectionSupplier::initialize()
{
	m_pConnectionKeeper = new ConnectionKeeper;
	m_pConnectionKeeper->initialize();
}

//
//	FUNCTION public static
//	Communication::ConnectionSupplier::terminate -- 後処理
//
//	NOTES
//	後処理。シリアルIOデーモンを終了する。
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
void
ConnectionSupplier::terminate()
{
	m_pConnectionKeeper->terminate();
	delete m_pConnectionKeeper;
}

//
//	FUNCTION public static
//	Communication::ConnectionSupplier::create
//								-- コネクションデーモンを起動する
//
//	NOTES
//	コネクションデーモンを起動する。
//
//	ARGUMENTS
//	int iType_
//		デーモンタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ConnectionSupplier::create(int iType_)
{
	if (iType_ == Memory)
	{
		// メモリデーモン
		
		m_pMemoryDaemon = new MemoryDaemon(m_pConnectionKeeper);
		try {
			m_pMemoryDaemon->initialize();
			m_pMemoryDaemon->create();
#ifdef NO_CATCH_ALL
		} catch (Exception::Object&) {
#else
		} catch (...) {
#endif
			delete m_pMemoryDaemon, m_pMemoryDaemon = 0;
			_SYDNEY_RETHROW;
		}
	}
	else if (iType_ == Socket)
	{
		// ソケットデーモン
		
		m_pSocketDaemon = new SocketDaemon(m_pConnectionKeeper);
		try {
			m_pSocketDaemon->initialize();
			m_pSocketDaemon->create();
#ifdef NO_CATCH_ALL
		} catch (Exception::Object&) {
#else
		} catch (...) {
#endif
			delete m_pSocketDaemon, m_pSocketDaemon = 0;
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public static
//	Communication::ConnectionSupplier::abort
//									-- コネクションデーモンを停止する
//
//	NOTES
//	コネクションデーモンを停止する。
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
ConnectionSupplier::abort()
{
	if (m_pSocketDaemon)
	{
		m_pSocketDaemon->terminate();
		delete m_pSocketDaemon; m_pSocketDaemon = 0;
	}
	if (m_pMemoryDaemon)
	{
		m_pMemoryDaemon->terminate();
		delete m_pMemoryDaemon; m_pMemoryDaemon = 0;
	}
}

//
//	FUNCTION public static
//	Communication::ConnectionSupplier::consumeConnection
//													-- シリアルIOを取得する
//
//	NOTES
//	引数で指定されたConnectionに応じたSerialIOを
//	m_pConnectionKeeperから取得する
//
//	ARGUMENTS
//	Communication::Connection* pConnection_
//		Connectionオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ConnectionSupplier::consumeConnection(
	Connection* pConnection_)
{
	SerialIO* p = m_pConnectionKeeper->consumeConnection(pConnection_);
	if (pConnection_->getSlaveID() == ConnectionSlaveID::Any)
	{
		pConnection_->setSlaveID(ConnectionSlaveID::allocateID());
	}
	pConnection_->setSerialIO(p);
}

//
//	Copyright (c) 1999, 2001, 2003, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
