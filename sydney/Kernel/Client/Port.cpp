// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Port.cpp --
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
#include "Client/Port.h"
#include "Communication/LocalClientConnection.h"
#include "Communication/RemoteClientConnection.h"

#include "Common/ClassID.h"
#include "Common/ExceptionObject.h"

#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

//
//	FUNCTION public
//	Client::Port::Port -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	int iMasterID_
//		マスターID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Port::Port(int iMasterID_, int iSlaveID_)
	: Object(Type::Port)
{
	m_pConnection
		= new Communication::LocalClientConnection(iMasterID_, iSlaveID_);
}

//
//	FUNCTION public
//	Client::Port::Port -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHostName_
//		ホスト名
//	int iPortNumber_
//		ポート番号
//	int iMasterID_
//		マスターID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Port::Port(const ModUnicodeString& cstrHostName_, int iPortNumber_,
		   int iMasterID_, int iSlaveID_)
	: Object(Type::Port)
{
	m_pConnection
		= new Communication::RemoteClientConnection(cstrHostName_, iPortNumber_,
													iMasterID_, iSlaveID_);
}

//
//	FUNCTION public
//	Client::Port::~Port -- デストラクタ
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
Port::~Port()
{
	delete m_pConnection;
}

//
//	FUNCTION public
//	Client::Port::open -- オープンする
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
Port::open()
{
	m_pConnection->open();
}

//
//	FUNCTION public
//	Client::Port::close -- クローズする
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
Port::close()
{
	m_pConnection->close();
}

//
//	FUNCTION public
//	Client::Port::sync -- 同期を取る
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
Port::sync()
{
	m_pConnection->sync();
}

//
//	FUNCTION public
//	Client::Port::writeObject -- オブジェクトを書き出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Externalizable* pObject_
//		オブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Port::writeObject(const Common::Externalizable* pObject_)
{
	m_pConnection->writeObject(pObject_);
}

//
//	FUNCTION public
//	Client::Port::flush -- 出力をフラッシュする
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
Port::flush()
{
	m_pConnection->flush();
}

//
//	FUNCTION public
//	Client::Port::readObject -- オブジェクトを読み出す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Externalizable*
//		読み出したオブジェクト
//
//	EXCEPTIONS
//
Common::Externalizable*
Port::readObject()
{
	ModAutoPointer<Common::Externalizable> pObject
		= m_pConnection->readObject();
	if (pObject.get()
		&& pObject->getClassID() == Common::ClassID::ExceptionClass)
	{
		//例外クラスなので、例外を投げる
		Common::ExceptionObject* pException
			= dynamic_cast<Common::ExceptionObject*>(pObject.get());
		pException->throwClassInstance();
	}

	return pObject.release();
}

//
//	FUNCTION public
//	Client::Port::readIntegerData -- IntegerDataを読む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::IntegerData*
//		IntegerData
//
//	EXCEPTIONS
//
Common::IntegerData*
Port::readIntegerData()
{
	ModAutoPointer<Common::Externalizable> pObject = readObject();
	Common::IntegerData* pData
		= dynamic_cast<Common::IntegerData*>(pObject.get());
	if (pData == 0)
	{
		//ここに来るのはサーバとの同期が取れていないこと
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	pObject.release();
	return pData;
}

//
//	FUNCTION public
//	Client::Port::readStatus -- ステータスを読む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Status::Type
//		読み込んだステータス
//
//	EXCEPTIONS
//
Common::Status::Type
Port::readStatus()
{
	ModAutoPointer<Common::Externalizable> pObject = readObject();
	Common::Status* pStatus
		= dynamic_cast<Common::Status*>(pObject.get());
	if (pStatus == 0)
	{
		//ここに来るのはサーバとの同期が取れていないこと
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	return pStatus->getStatus();
}

//
//	FUNCTION public
//	Client::Port::getSlaveID -- スレーブIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スレーブID
//
//	EXCEPTIONS
//
int
Port::getSlaveID()
{
	return m_pConnection->getSlaveID();
}

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
