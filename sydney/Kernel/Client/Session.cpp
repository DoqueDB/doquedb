// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.cpp --
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
const char moduleName[] = "Client";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client/Session.h"
#include "Client/DataSource.h"
#include "Client/Connection.h"
#include "Client/Port.h"
#include "Client/ResultSet.h"
#include "Client/PrepareStatement.h"

#include "Exception/NotInitialized.h"

#include "Common/IntegerData.h"
#include "Common/DataArrayData.h"
#include "Common/Request.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

//
//	FUNCTION public
//	Client::Session::Session -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Client::DataSource& cDataSource_
//		 データソース
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iSessionID_
//		セッションID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Session::Session(DataSource& cDataSource_,
				 const ModUnicodeString& cstrDatabaseName_, int iSessionID_)
: Object(Type::Session), m_cDataSource(cDataSource_), m_iSessionID(iSessionID_)
{
	m_cstrDatabaseName = cstrDatabaseName_;
	m_cDataSource.incrementCount(Type::Session);
}

//
//	FUNCTION public
//	Client::Session::~Session -- デストラクタ
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
Session::~Session()
{
	try
	{
		close();
	} catch (...) {}	// 例外は無視
	m_cDataSource.decrementCount(Type::Session);
}

//
//	FUNCTION public
//	Client::Session::close -- セッションを破棄する
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
Session::close()
{
	if (m_iSessionID)
	{
		// セッションを終了する
		
		//クライアントコネクションを得る
		Connection* pClientConnection = m_cDataSource.getClientConnection();

		int iWorkerID;

		//Workerを起動する
		ModAutoPointer<Port> pPort = pClientConnection->beginWorker(iWorkerID);

		// [<-] EndSession
		const Common::Request request(Common::Request::EndSession);
		pPort->writeObject(&request);

		// [<-] SessionID
		const Common::IntegerData data(m_iSessionID);
		pPort->writeObject(&data);
		pPort->flush();

		// [->] ステータス
		pPort->readStatus();

		//コネクションを返す
		m_cDataSource.pushPort(pPort.release());

		m_iSessionID = 0;
	}
}

//
//	FUNCTION public
//	Client::Session::executeStatement -- SQL文を実行する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrStatement_
//		SQL文
//	const Common::DataArrayData& cParameter_
//		パラメータ(default Common::DataArrayData())
//
//	RETURN
//	ResultSet*
//		結果セット
//
//	EXCEPTIONS
//
ResultSet*
Session::executeStatement(const ModUnicodeString& cstrStatement_,
						  const Common::DataArrayData& cParameter_)
{
	//クライアントコネクションを得る
	Connection* pClientConnection = m_cDataSource.getClientConnection();

	int iWorkerID;
	//Workerを起動する
	ModAutoPointer<Port> pPort = pClientConnection->beginWorker(iWorkerID);

	// [<-] ExecuteStatement
	const Common::Request request(Common::Request::ExecuteStatement);
	pPort->writeObject(&request);

	// [<-] SessionID
	const Common::IntegerData id(m_iSessionID);
	pPort->writeObject(&id);

	// [<-] SQL文
	const Common::StringData stmt(cstrStatement_);
	pPort->writeObject(&stmt);

	// [<-] パラメータ
	pPort->writeObject(&cParameter_);
	pPort->flush();

	//結果集合を返す
	return new ResultSet(m_cDataSource, iWorkerID, pPort.release());
}

//
//	FUNCTION public
//	Client::Session::executePrepareStatement -- 最適化SQL文を実行する
//
//	NOTES
//
//	ARGUMENTS
//	const Client::PrepareStatement& cPrepareStatement_
//		最適化SQL文
//	const Common::DataArrayData& cParameter_
//		パラメータ
//
//	RETURN
//	ResultSet*
//		結果セット
//
//	EXCEPTIONS
//
ResultSet*
Session::executePrepareStatement(const PrepareStatement& cPrepareStatement_,
								 const Common::DataArrayData& cParameter_)
{
	//クライアントコネクションを得る
	Connection* pClientConnection = m_cDataSource.getClientConnection();

	int iWorkerID;
	//Workerを起動する
	ModAutoPointer<Port> pPort = pClientConnection->beginWorker(iWorkerID);

	// [<-] ExecutePrepare
	const Common::Request request(Common::Request::ExecutePrepare);
	pPort->writeObject(&request);

	// [<-] SessionID
	const Common::IntegerData sessionID(m_iSessionID);
	pPort->writeObject(&sessionID);

	// [<-] PrepareID
	const Common::IntegerData prepareID(cPrepareStatement_.getPrepareID());
	pPort->writeObject(&prepareID);

	// [<-] パラメータ
	pPort->writeObject(&cParameter_);
	pPort->flush();

	//結果集合を返す
	return new ResultSet(m_cDataSource, iWorkerID, pPort.release());
}

//
//	Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
