// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.cpp -- クライアントコネクション関連の関数定義
// 
// Copyright (c) 2006, 2013, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client2/Connection.h"
#include "Client2/DataSource.h"
#include "Client2/Port.h"
#ifdef SYD_COVERAGE
#include "Client2/Parameter.h"
#endif

#include "Common/Request.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/StringData.h"
#include "Common/IntegerArrayData.h"

#include "Communication/ConnectionSlaveID.h"

#include "Os/AutoCriticalSection.h"

#include "ModAutoPointer.h"
#ifdef SYD_COVERAGE
#include "ModOsDriver.h"
#endif

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

namespace {

#ifdef SYD_COVERAGE

//	VARIABLE local
//	_$$::_iOpenWaitTime -- ポートのオープンを待つ時間 ( DEBUG 用)

ParameterInteger _iOpenWaitTime("Client2_OpenWaitTime", 0);

#endif

}

//	FUNCTION public
//	Client2::Connection::Connection -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Client2::DataSource&	cDataSource_
//		属するデータソースオブジェクト
//	Client2::Port*			pPort_
//		サーバ側のコネクションスレッドとの通信ポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Connection::Connection(DataSource&	cDataSource_,
					   Port*		pPort_)
	: Object(Object::Type::Connection),
	  m_cDataSource(cDataSource_),
	  m_pPort(pPort_)
{
}

//	FUNCTION public
//	Client2::Connection::~Connection -- デストラクタ
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

Connection::~Connection()
{
}

//	FUNCTION public
//	Client2::Connection::close -- クローズする
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

void
Connection::close()
{
	if (m_pPort != 0) {

		try {

			// [<-] リクエスト
			const Common::Request	cRequest(Common::Request::EndConnection);
			m_pPort->writeObject(&cRequest);
			m_pPort->flush();
			// [->] ステータス
			m_pPort->readStatus();

			//コネクションをクローズする
			m_pPort->close();

		} catch (...) {
			Common::Thread::resetErrorCondition();
			// 例外は無視する
		}

		m_pPort->release(), m_pPort = 0;
	}
}

//	FUNCTION public
//	Client2::Connection::beginWorker -- ワーカを起動する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::Port*
//		ワーカとの通信ポート
//
//	EXCEPTIONS

Port*
Connection::beginWorker()
{
	// コネクションプールからコネクションを得る
	ModAutoPointer<Port>	pPort = m_cDataSource.popPort();

	int	iSlaveID = Communication::ConnectionSlaveID::Any;
	ModAutoPointer<Common::IntegerData>	pSlaveID;
	ModAutoPointer<Common::IntegerData>	pWorkerID;

	try {

		if (pPort.get() != 0) iSlaveID = pPort->getSlaveID();


		{
			Os::AutoCriticalSection	cAuto(m_cPortLatch);

			// [<-] リクエスト
			const Common::Request	cRequest(Common::Request::BeginWorker);
			m_pPort->writeObject(&cRequest);
			// [<-] スレーブ ID
			const Common::IntegerData	cSlaveID(iSlaveID);
			m_pPort->writeObject(&cSlaveID);
			m_pPort->flush();

			// [->] スレーブ ID
			pSlaveID = m_pPort->readIntegerData();
			// [->] ワーカ ID
			pWorkerID = m_pPort->readIntegerData();
			// [->] ステータス
			m_pPort->readStatus();
		}

	} catch (...) {

		if (pPort.get() != 0 ) {
			if (pPort->isReuse()) {
				m_cDataSource.pushPort(pPort.release());
			} else {
				pPort->close();
			}
		}
		throw;
	}

	if (iSlaveID == Communication::ConnectionSlaveID::Any) {

		// 新しい通信ポート
		pPort = m_cDataSource.getNewPort(pSlaveID->getValue());
#ifdef SYD_COVERAGE
		ModOsDriver::Thread::sleep(_iOpenWaitTime.get());
#endif
		pPort->open();
		
		// 共通鍵を渡す(暗号化対応)
		pPort->setKey(m_cDataSource.getKey());
	} else {

		// プールされている通信ポート -> 同期を取る
		pPort->sync();
	}

	// ワーカ ID を設定
	pPort->setWorkerID(pWorkerID->getValue());

	return pPort.release();
}

//	FUNCTION public
//	Client2::Connection::cancelWorker -- ワーカをキャンセルする
//
//	NOTES
//
//	ARGUMENTS
//	int	iWorkerID_
//		キャンセルするワーカの ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Connection::cancelWorker(int	iWorkerID_)
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::CancelWorker);
	m_pPort->writeObject(&cRequest);
	// [<-] ワーカ ID
	const Common::IntegerData	cWorkerID(iWorkerID_);
	m_pPort->writeObject(&cWorkerID);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//	FUNCTION public
//	Client2::Connection::erasePrepareStatement -- プリペアステートメントを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//	int						iPrepareID_
//		プリペアステートメント ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Connection::erasePrepareStatement(	const ModUnicodeString&	cstrDatabaseName_,
									int						iPrepareID_)
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::ErasePrepareStatement);
	m_pPort->writeObject(&cRequest);
	// [<-] データベース名
	const Common::StringData	cDatabaseName(cstrDatabaseName_);
	m_pPort->writeObject(&cDatabaseName);
	// [<-] プリペアステートメント ID
	const Common::IntegerData	cPrepareID(iPrepareID_);
	m_pPort->writeObject(&cPrepareID);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//	FUNCTION public
//	Client2::Connection::disconnectPort -- 使用しない通信ポートを切断する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<int>&	veciSlaveID_
//		切断する通信ポートのスレーブ ID の配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Connection::disconnectPort(const ModVector<int>&	veciSlaveID_)
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::NoReuseConnection);
	m_pPort->writeObject(&cRequest);
	// [<-] スレーブ ID の配列
	const Common::IntegerArrayData	cSlaveID(veciSlaveID_);
	m_pPort->writeObject(&cSlaveID);
	m_pPort->flush();

	// [->] ステータス
	m_pPort->readStatus();
}

//	FUNCTION public
//	Client2::Connection::beginConnection -- 新しいコネクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::Connection*
//		新しいクライアントコネクション
//
//	EXCEPTIONS

Connection*
Connection::beginConnection()
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	const Common::Request	cRequest(Common::Request::BeginConnection);
	m_pPort->writeObject(&cRequest);
	m_pPort->flush();

	// [->] スレーブ ID
	ModAutoPointer<Common::IntegerData>	pSlaveID = m_pPort->readIntegerData();

	// 新しい通信ポートを得る
	ModAutoPointer<Port>	pPort = m_cDataSource.getNewPort(pSlaveID->getValue());
	pPort->open();

	// [->] ステータス
	m_pPort->readStatus();

	// [->] 新しい通信ポートからステータスを得る
	pPort->readStatus();

	return new Connection(m_cDataSource, pPort.release());
}

//	FUNCTION public
//	Client2::Connection::isServerAvailable -- サーバ利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		サーバが利用可能な場合は true 、利用不可能な場合は false
//
//	EXCEPTIONS

bool
Connection::isServerAvailable()
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	Common::Request	cRequest(Common::Request::CheckAvailability);
	m_pPort->writeObject(&cRequest);
	// [<-] チェック対象
	Common::IntegerData	cTarget(Common::Request::AvailabilityTarget::Server);
	m_pPort->writeObject(&cTarget);
	m_pPort->flush();
	
	// [->] チェック結果
	ModAutoPointer<Common::IntegerData>	pResult = m_pPort->readIntegerData();
	// [->] ステータス
	m_pPort->readStatus();

	return (pResult->getValue() == 1) ? true : false;
}

//	FUNCTION public
//	Client2::Connection::isDatabaseAvailable -- データベース利用可能性を得る
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Database::ID	iID_
//		データベース ID
//
//	RETURN
//	bool
//		データベースが利用可能な場合は true 、利用不可能な場合は false
//
//	EXCEPTIONS

bool
Connection::isDatabaseAvailable(Database::ID	iID_)
{
	Os::AutoCriticalSection	cAuto(m_cPortLatch);

	// [<-] リクエスト
	Common::Request	cRequest(Common::Request::CheckAvailability);
	m_pPort->writeObject(&cRequest);
	// [<-] チェック対象
	Common::IntegerData	cTarget(Common::Request::AvailabilityTarget::Database);
	m_pPort->writeObject(&cTarget);
	// [<-] データベース ID
	Common::UnsignedIntegerData cID(iID_);
	m_pPort->writeObject(&cID);
	m_pPort->flush();
	
	// [->] チェック結果
	ModAutoPointer<Common::IntegerData>	pResult = m_pPort->readIntegerData();
	// [->] ステータス
	m_pPort->readStatus();

	return (pResult->getValue() == 1) ? true : false;
}

//
//	Copyright (c) 2006, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
