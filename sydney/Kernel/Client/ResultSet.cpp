// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.cpp --
// 
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
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
#include "Client/ResultSet.h"
#include "Client/DataSource.h"
#include "Client/Port.h"
#include "Client/Connection.h"

#include "Common/Status.h"
#include "Common/DataArrayData.h"
#include "Common/ResultSetMetaData.h"
#include "Common/ClassID.h"

#include "Exception/Unexpected.h"
#include "Exception/NotInitialized.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

//
//	FUNCTION public
//	Client::ResultSet::ResultSet -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Clinet::DataSource& cDataSource_
//		データソース
//	int iWorkerID_
//		ワーカID
//	Client::Port* pPort_
//		ポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ResultSet::ResultSet(DataSource& cDataSource_, int iWorkerID_, Port* pPort_)
	: Object(Type::ResultSet),
	  m_cDataSource(cDataSource_),
	  m_iWorkerID(iWorkerID_),
	  m_pPort(pPort_),
	  m_eStatus(Status::Data),
	  m_pMetaData(0)
{
//	m_cDataSource.incrementCount(Type::ResultSet);
}

//
//	FUNCTION public
//	Client::ResultSet::~ResultSet -- デストラクタ
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
ResultSet::~ResultSet()
{
	try
	{
		close();
	} catch (...) {}	// 例外は無視
//	m_cDataSource.decrementCount(Type::ResultSet);
}

//
//	FUNCTION public
//	Client::ResultSet::close -- 結果集合をクローズする
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
ResultSet::close()
{
	if (m_pPort)
	{
		switch (m_eStatus)
		{
		case Status::Data:
		case Status::MetaData:
			//まだ実行中である可能性があるので、中断を要求する
			cancel();
		case Status::EndOfData:
			//実行ステータスを得ていないので、得とく
			getStatus();
		}

		if (m_eStatus == Status::Success)
		{
			//ポートを返す
			m_cDataSource.pushPort(m_pPort);
		}
		else
		{
			//クローズ
			m_pPort->close();
			m_pPort->release();
		}
		m_pPort = 0;

		//メタデータを開放する
		delete m_pMetaData, m_pMetaData = 0;
	}
}

//
//	FUNCTION public
//	Client::ResultSet::getStatus -- 実行結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client::ResultSet::Status
//		実行ステータス
//
//	EXCEPTIONS
//
ResultSet::Status::Value
ResultSet::getStatus()
{
	while (m_eStatus == Status::MetaData
		   || m_eStatus == Status::Data
		   || m_eStatus == Status::EndOfData)
	{
		Common::DataArrayData* pTuple = 0;
		getNextTuple(pTuple);
		delete pTuple;
	}

	return m_eStatus;
}

//
//	FUNCTION public
//	Client::ResultSet::getNextTuple -- 次のTupleデータを読む
//
//	NOTES
//
//	ARGUMENTS
//	Common::DataArrayData*& pTuple_
//
//	RETURN
//	Clinet::ResultSet::Status::Value
//		ステータス
//
//	EXCEPTIONS
//
ResultSet::Status::Value
ResultSet::getNextTuple(Common::DataArrayData*& pTuple_)
{
	//まず0を代入する
	pTuple_ = 0;

	Status::Value eStatus = Status::Undefined;
	ModAutoPointer<Common::Externalizable> pObject;

	try
	{
		//ポートから1つ読み込む
		pObject = m_pPort->readObject();
	}
	catch (...)
	{
		m_eStatus = Status::Error;
		throw;
	}

	if (pObject.get() == 0)
	{
		//End of Data
		eStatus = Status::EndOfData;
	}
	else if (pObject->getClassID() == Common::ClassID::StatusClass)
	{
		//実行ステータス

		Common::Status* pStatus = dynamic_cast<Common::Status*>(pObject.get());
		
		switch (pStatus->getStatus())
		{
		case Common::Status::Success:
			eStatus = Status::Success;
			break;
		case Common::Status::Canceled:
			eStatus = Status::Canceled;
			break;
		}
	}
	else if (pObject->getClassID() == Common::ClassID::DataArrayDataClass)
	{
		//タプル
		
		Common::DataArrayData* pData
			= dynamic_cast<Common::DataArrayData*>(pObject.get());

		pTuple_ = pData;
		pObject.release();
		eStatus = Status::Data;
	}
	else if (pObject->getClassID() == Common::ClassID::ResultSetMetaDataClass)
	{
		//結果集合メタデータ

		Common::ResultSetMetaData* pMetaData
			= dynamic_cast<Common::ResultSetMetaData*>(pObject.get());

		if (m_pMetaData) delete m_pMetaData;
		m_pMetaData = pMetaData;
		pObject.release();
		eStatus = Status::MetaData;
	}

	if (eStatus == Status::Undefined)
	{
		m_eStatus = Status::Error;
		//予期せぬエラー
		_TRMEISTER_THROW0(Exception::Unexpected);
	}

	//現在の状態をセーブ
	m_eStatus = eStatus;

	return eStatus;
}

//
//	FUNCTION public static
//	Client::ResultSet::releaseTuple -- タプルデータを削除する
//
//	NOTES
//	違うDLLでnew,deleteしないように、タプルデータをdeleteするメソッド
//	を用意しとく。
//
//	ARGUMENTS
//	Common::DataArrayData* pTuple_
//		削除するタプルデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSet::releaseTuple(Common::DataArrayData* pTuple_)
{
	delete pTuple_;
}

//
//	FUNCTION public
//	Client::ResultSet::cancel -- 実行をキャンセルする
//
//	NOTES
//	ワーカに中断をリクエストする。中断されるとは限らない。
//	getStatus()で Status::Canceled がかえったら中断成功。
//	Status::Success がかえったら中断は無視されたことになる。
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
ResultSet::cancel()
{
	Connection* pClientConnection = m_cDataSource.getClientConnection();
	//中断を要求する
	pClientConnection->cancelWorker(m_iWorkerID);
}

//
//	FUNCTION public
//	Client::ResultSet::getMetaData -- 結果集合メタデータを得る
//
//	NOTES
//	プロトコルバージョン2以上の場合のみ有効
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::ResultSetMetaData*
//		メタデータが存在する場合はメタデータ。存在しない場合は0
//
//	EXCEPTIONS
//
const Common::ResultSetMetaData*
ResultSet::getMetaData() const
{
	return m_pMetaData;
}

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
