// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.cpp -- 結果集合クラスの関数定義
// 
// Copyright (c) 2006, 2007, 2012, 2015, 2023 Ricoh Company, Ltd.
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
#include "Client2/ResultSet.h"
#include "Client2/DataSource.h"
#include "Client2/Port.h"
#include "Client2/Connection.h"

#include "Common/Status.h"
#include "Common/DataArrayData.h"
#include "Common/ResultSetMetaData.h"
#include "Common/ClassID.h"
#include "Common/ColumnMetaData.h"
#include "Common/DataInstance.h"

#include "Exception/Unexpected.h"
#include "Exception/ConnectionRanOut.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	FUNCTION public
//	Client2::ResultSet::ResultSet -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Client2::DataSource&	cDataSource_
//		データソース
//	Client2::Port*			pPort_
//		通信ポート
//
//	RETURN
//	なし
//
//	EXCEPTIONS

ResultSet::ResultSet(DataSource&	cDataSource_,
					 Port*			pPort_)
	: Object(Object::Type::ResultSet),
	  m_cDataSource(cDataSource_),
	  m_pPort(pPort_),
	  m_eStatus(Status::Data),
	  m_pMetaData(0),
	  m_pTupleData(0)
{
}

//	FUNCTION public
//	Client2::ResultSet::~ResultSet -- デストラクタ
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

ResultSet::~ResultSet()
{
	if (m_pPort != 0) {
		m_cDataSource.expungePort(m_pPort);
	}
	if (m_pMetaData != 0) delete m_pMetaData;
	if (m_pTupleData != 0) delete m_pTupleData;
}

//	FUNCTION public
//	Client2::ResultSet::close -- クローズする
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
ResultSet::close()
{
	if (m_pPort != 0) {

		try {

			switch (m_eStatus) {
			case Status::Data:
			case Status::MetaData:
			case Status::HasMoreData:
				// まだ実行中である可能性があるので、中断を要求する
				cancel();
				// thru.
			case Status::EndOfData:
				// 実行ステータスを得ていないので、得る
				getStatus(true /* skip until success or canceled is obtained */);
				break;
			}

		} catch (...) {
			Common::Thread::resetErrorCondition();
			// 例外は無視する
		}
	}
}

//	FUNCTION public
//	Client2::ResultSet::getStatus -- 実行ステータスを得る
//
//	NOTES
//	実行結果のステータスのみを返す。タプルデータがあっても読み捨てられる
//	insert文やdelete文等のタプルを返さないSQL文用
//
//	ARGUMENTS
//	bool bSkipAll_ = true
//		trueなら複文の場合にすべての文の結果を飛ばす
//
//	RETURN
//	Client2::ResultSet::Status::Value
//		実行ステータス
//
//	EXCEPTIONS

ResultSet::Status::Value
ResultSet::getStatus(bool bSkipAll_ /* = true */)
{
	while (m_eStatus == Status::MetaData
		   || m_eStatus == Status::Data
		   || m_eStatus == Status::EndOfData
		   || (bSkipAll_ && m_eStatus == Status::HasMoreData)) {

		getNextTuple(0);
	}

	return m_eStatus;
}

//
//	FUNCTION public
//	Client2::ResultSet::getNextTuple -- 次のタプルデータを読む
//
//	NOTES
//	タプルデータであればタプルデータを返し、
//	ステータスであればステータスを返す。
//	select文等のタプルを返すSQL文用であるが、
//	タプルを返さないものでも問題はない。
//	プロトコルバージョン2以上の場合は、更新系のSQL文でも結果が返る。
//	更新系の場合は、影響を及ぼした行のROWIDが返ってくる。
//
//	ARGUMENTS
//	Common::DataArrayData*	pTuple_
//		読み込んだタプルデータ。
//		呼び出し側は空の DataArrayData を設定する必要がある
//
//	RETURN
//	Client2::ResultSet::Status::Value
//		実行ステータス
//
//	EXCEPTIONS

ResultSet::Status::Value
ResultSet::getNextTuple(Common::DataArrayData*	pTuple_)
{
	//if (m_pPort == 0) return m_eStatus;
	if (m_pPort == 0) {
		if (m_eStatus == Status::Error) {
			_TRMEISTER_THROW0(Exception::ConnectionRanOut);
		} else {
			return m_eStatus;
		}
	}

	Status::Value	eStatus = Status::Undefined;
	ModAutoPointer<Common::Externalizable>	pObject;

	if (m_pTupleData != 0) {

		// 中身を assign する
		if (pTuple_ != 0) pTuple_->assign(m_pTupleData);

	} else {

		// 中身を解放する
		if (pTuple_ != 0) pTuple_->clear();
	}

	try {

		try {
			// 通信ポートから 1 つ読み込む
			pObject = m_pPort->readObject(pTuple_);

		} catch (...) {

			m_eStatus = Status::Error;
			throw;
		}

		if (pObject.get() == 0) {

			// データ終了
			eStatus = Status::EndOfData;
			if (m_pMetaData != 0) {
				delete m_pMetaData;
				m_pMetaData = 0;
			}
			if (m_pTupleData != 0) {
				delete m_pTupleData;
				m_pTupleData = 0;
			}

		} else if (pObject->getClassID() == Common::ClassID::ResultSetMetaDataClass) {

			// メタデータ
			eStatus = Status::MetaData;

			if (m_pMetaData) delete m_pMetaData;
			m_pMetaData = dynamic_cast<Common::ResultSetMetaData*>(pObject.release());
			delete m_pTupleData;
			m_pTupleData = createTupleData();

		} else if (pObject->getClassID() == Common::ClassID::StatusClass) {

			// 実行ステータス

			Common::Status*	pStatus = dynamic_cast<Common::Status*>(pObject.get());

			switch (pStatus->getStatus()) {
			case Common::Status::Success:
				eStatus = Status::Success;
				break;
			case Common::Status::Canceled:
				eStatus = Status::Canceled;
				break;
			case Common::Status::HasMoreData:
				eStatus = Status::HasMoreData;
				break;
			}

		} else if (pObject->getClassID() == Common::ClassID::DataArrayDataClass) {

			// タプル

			eStatus = Status::Data;

			if (pTuple_ != 0)
				pObject.release();
		}

		if (eStatus == Status::Undefined) {

			m_eStatus = Status::Error;
			//予期せぬエラー
			_TRMEISTER_THROW0(Exception::Unexpected);
		}

		//現在の状態をセーブ
		m_eStatus = eStatus;

	} catch (...) {

		terminateGetNextTuple();

		throw;
	}

	terminateGetNextTuple();

	return eStatus;
}

//	FUNCTION private
//	Client2::ResultSet::terminateGetNextTuple -- getNextTuple の後始末
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
ResultSet::terminateGetNextTuple()
{
	switch (m_eStatus) {
	case Status::Data:
	case Status::EndOfData:
	case Status::MetaData:
	case Status::HasMoreData:
		{
			break;
		}
	case Status::Success:
		{
			m_cDataSource.pushPort(m_pPort);
			m_pPort = 0;
			break;
		}
	case Status::Canceled:
		{
			if (m_pPort->getMasterID() >= DataSource::Protocol::Version3) {
				m_cDataSource.pushPort(m_pPort);
				m_pPort = 0;
				break;
			}
			// thru.
		}
	case Status::Error:
	case Status::Undefined:
	default:
		{
			if (m_pPort->isReuse()) {
				m_cDataSource.pushPort(m_pPort);
			} else {
				m_pPort->close();
				m_pPort->release();
			}
			m_pPort = 0;
			break;
		}
	}
}

//	FUNCTION public
//	Client2::ResultSet::cancel -- 実行をキャンセルする
//
//	NOTES
//	ワーカに中断をリクエストする。中断されるとは限らない。
//	getStatus(true)で Status::Canceled がかえったら中断成功。
//	Status::Success がかえったら中断は無視されたことになる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
ResultSet::cancel()
{
	// クライアントコネクションを得る
	Connection*	pClientConnection = m_cDataSource.getClientConnection();

	// 中断を要求する
	pClientConnection->cancelWorker(m_pPort->getWorkerID());
}

//	FUNCTION public
//	Client2::ResultSet::getMetaData -- ResultSetMetaData を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::ResultSetMetaData*
//		メタデータが存在する場合はメタデータ。存在しない場合は 0
//
//	EXCEPTIONS

const Common::ResultSetMetaData*
ResultSet::getMetaData() const
{
	return m_pMetaData;
}

//	FUNCTION private
//	Client2::ResultSet::createTupleData --
//		メタデータから適切なデータ型が格納された DataArrayData を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataArrayData*
//		DataArrayData
//
//	EXCEPTIONS

Common::DataArrayData*
ResultSet::createTupleData()
{
	int	size = m_pMetaData->getCount();
	ModAutoPointer<Common::DataArrayData> pTuple = new Common::DataArrayData();
	pTuple->reserve(size);

	for (int i = 0; i < size; i++) {

		const Common::ColumnMetaData&	cColumnMetaData = (*m_pMetaData)[i];
		Common::Data*	pColumn = Common::DataInstance::create(cColumnMetaData.getDataType());
		if (pColumn == 0) {
			// Tuple data can't be prepared for lack of type information
			return 0;
		}
		pTuple->pushBack(pColumn);
	}
	return pTuple.release();
}

//
//	Copyright (c) 2006, 2007, 2012, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
