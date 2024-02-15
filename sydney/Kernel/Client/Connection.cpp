// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.cpp --
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client/Connection.h"
#include "Client/DataSource.h"
#include "Client/Port.h"
#ifdef SYD_COVERAGE
#include "Client/Parameter.h"
#endif

#include "Common/Request.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/StringData.h"
#include "Common/IntegerArrayData.h"

#include "Communication/ConnectionSlaveID.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/BadArgument.h"

#include "ModAutoPointer.h"
#ifdef SYD_COVERAGE
#include "ModOsDriver.h"
#endif

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

namespace {

#ifdef SYD_COVERAGE
//
//	VARIABLE local
//	_$$::_iOpenWaitTime -- ポートのオープンを待つ時間(DEBUG用)
//
ParameterInteger _iOpenWaitTime("Client_OpenWaitTime", 0);
#endif
}

//
//	FUNCTION public
//	Client::Connection::Connection -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Clinet::DataSource& cDataSource_
//		データソース
//	Port* pPort_
//		クライアントコネクション用のポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Connection::Connection(DataSource& cDataSource_, Port* pPort_)
: Object(Type::Connection), m_cDataSource(cDataSource_), m_pPort(pPort_)
{
	m_cDataSource.incrementCount(Type::Connection);
}

//
//	FUNCTION public
//	Client::Connection::~Connection -- デストラクタ
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
Connection::~Connection()
{
	m_cDataSource.decrementCount(Type::Connection);
}

//
//	FUNCTION public
//	Client::Connection::close -- 接続を切る
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
Connection::close()
{
	if (m_pPort)
	{
		try
		{
			// [<-] リクエストを送る
			const Common::Request request(Common::Request::EndConnection);
			m_pPort->writeObject(&request);
			m_pPort->flush();
			// [->] ステータスを受けとる
			m_pPort->readStatus();

			//コネクションをクローズする
			m_pPort->close();
		}
		catch (...) {} // 例外は無視する

		delete m_pPort, m_pPort = 0;
	}
}

//
//	FUNCTION public
//	Client::Connection::beginWorker -- Workerを起動する
//
//	NOTES
//
//	ARGUMENTS
//	int& iWorkerID_
//		起動したWorkerのID
//
//	RETURN
//	Clinet::Port*
//		Workerとのポート
//
//	EXCEPTIONS
//
Port*
Connection::beginWorker(int& iWorkerID_)
{
	//コネクションプールからコネクションを得る
	ModAutoPointer<Port> pPort = m_cDataSource.popPort();
	int iSlaveID = Communication::ConnectionSlaveID::Any;
	if (pPort.get()) iSlaveID = pPort->getSlaveID();

	ModAutoPointer<Common::IntegerData> pSlaveID;
	ModAutoPointer<Common::IntegerData> pWorkerID;

	{
		Os::AutoCriticalSection cAuto(m_cPortLatch);

		// [<-] リクエストを送る
		const Common::Request request(Common::Request::BeginWorker);
		m_pPort->writeObject(&request);
		// [<-] コネクションのスレーブIDを送る
		const Common::IntegerData data(iSlaveID);
		m_pPort->writeObject(&data);
		m_pPort->flush();

		// [->] コネクションのスレーブIDを受け取る
		pSlaveID = m_pPort->readIntegerData();

		// [->] WorkerIDを受け取る
		pWorkerID = m_pPort->readIntegerData();

		// [->] ステータス
		m_pPort->readStatus();
	}

	//WorkerIDを設定
	iWorkerID_ = pWorkerID->getValue();

	if (iSlaveID == Communication::ConnectionSlaveID::Any)
	{
		// 新しいコネクション
		pPort = m_cDataSource.getNewPort(pSlaveID->getValue());
#ifdef SYD_COVERAGE
		ModOsDriver::Thread::sleep(_iOpenWaitTime.get());
#endif
		pPort->open();
	}
	else
	{
		//プールされているコネクション -> 同期を取る
		pPort->sync();
	}

	return pPort.release();
}

//
//	FUNCTION public
//	Client::Connection::cancelWorker -- Workerに中断をリクエストする
//
//	NOTES
//
//	ARGUMENTS
//	int iWorkerID_
//		中断するWorkerのID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Connection::cancelWorker(int iWorkerID_)
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	const Common::Request request(Common::Request::CancelWorker);
	m_pPort->writeObject(&request);
	// [<-] WorkerIDを送る
	const Common::IntegerData data(iWorkerID_);
	m_pPort->writeObject(&data);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//
//	FUNCTION public
//	Client::Connection::erasePrepareStatement -- 最適化結果を削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Connection::erasePrepareStatement(const ModUnicodeString& cstrDatabaseName_,
								  int iPrepareID_)
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	const Common::Request request(Common::Request::ErasePrepareStatement);
	m_pPort->writeObject(&request);
	// [<-] データベース名を送る
	const Common::StringData name(cstrDatabaseName_);
	m_pPort->writeObject(&name);
	// [<-] 最適化結果IDを送る
	const Common::IntegerData id(iPrepareID_);
	m_pPort->writeObject(&id);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//
//	FUNCTION public
//	Client::Connection::disconnectPort -- 使用しないポートを切断する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<int>& veciSlaveID_
//		切断するポートのスレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Connection::disconnectPort(const ModVector<int>& veciSlaveID_)
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	const Common::Request request(Common::Request::NoReuseConnection);
	m_pPort->writeObject(&request);
	// [<-] スレーブIDの配列を送る
	const Common::IntegerArrayData data(veciSlaveID_);
	m_pPort->writeObject(&data);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//
//	FUNCTION public
//	Client::Connection::beginConnection -- 新しいコネクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Clinet::Connection*
//		新しいクライアントコネクション
//
//	EXCEPTIONS
//
Connection*
Connection::beginConnection()
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	const Common::Request request(Common::Request::BeginConnection);
	m_pPort->writeObject(&request);
	m_pPort->flush();
	// [->] SlaveIDを受け取る
	ModAutoPointer<Common::IntegerData> pSlaveID = m_pPort->readIntegerData();

	// 新しいコネクション
	ModAutoPointer<Port> pPort = m_cDataSource.getNewPort(pSlaveID->getValue());
	pPort->open();

	// [->] ステータス
	m_pPort->readStatus();

	// [->] ステータス
	pPort->readStatus();

	return new Connection(m_cDataSource, pPort.release());
}

//
//	FUNCTION public
//	Client::Connection::isServerAvailable -- サーバ利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		サーバが利用可能な場合はtrue、利用不可能な場合はfalse
//
//	EXCEPTIONS
//
bool
Connection::isServerAvailable()
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	Common::Request request(Common::Request::CheckAvailability);
	m_pPort->writeObject(&request);
	// [<-] チェック対象を送る
	Common::IntegerData target(Common::Request::AvailabilityTarget::Server);
	m_pPort->writeObject(&target);
	m_pPort->flush();
	
	// [->] チェック結果
	ModAutoPointer<Common::IntegerData> pResult = m_pPort->readIntegerData();
	// [->] ステータス
	m_pPort->readStatus();

	return (pResult->getValue() == 1) ? true : false;
}

//
//	FUNCTION public
//	Client::Connection::isDatabaseAvailable -- データベース利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	Client::Database::ID
//		データベースID
//
//	RETURN
//	bool
//		データベースが利用可能な場合はtrue、利用不可能な場合はfalse
//
//	EXCEPTIONS
//
bool
Connection::isDatabaseAvailable(Database::ID id_)
{
	Os::AutoCriticalSection cAuto(m_cPortLatch);

	// [<-] リクエストを送る
	Common::Request request(Common::Request::CheckAvailability);
	m_pPort->writeObject(&request);
	// [<-] チェック対象を送る
	Common::IntegerData target(Common::Request::AvailabilityTarget::Database);
	m_pPort->writeObject(&target);
	// [<-] データベースIDを送る
	Common::UnsignedIntegerData id(id_);
	m_pPort->writeObject(&id);
	m_pPort->flush();
	
	// [->] チェック結果
	ModAutoPointer<Common::IntegerData> pResult = m_pPort->readIntegerData();
	// [->] ステータス
	m_pPort->readStatus();

	return (pResult->getValue() == 1) ? true : false;
}

//
//	Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
