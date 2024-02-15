// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Port.cpp -- 通信ポート関連の関数定義
// 
// Copyright (c) 2006, 2008, 2011, 2023 Ricoh Company, Ltd.
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
#include "Client2/Port.h"
#include "Communication/LocalClientConnection.h"
#include "Communication/RemoteClientConnection.h"

#include "Common/ClassID.h"
#include "Common/ExceptionObject.h"
#include "Common/ErrorLevel.h"

#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	FUNCTION public
//	Client2::Port::Port -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int	iMasterID_
//		マスター ID
//	int	iSlaveID_
//		スレーブ ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Port::Port(int	iMasterID_,
		   int	iSlaveID_)
	: Object(Object::Type::Port),
	  m_bReuse(false)
{
	m_pConnection
		= new Communication::LocalClientConnection(iMasterID_, iSlaveID_);
}

//	FUNCTION public
//	Client2::Port::Port -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString&	cstrHostName_
//		ホスト名
//	int						iPortNumber_
//		ポート番号
//	int						iMasterID_
//		マスター ID
//	int						iSlaveID_
//		スレーブ ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Port::Port(const ModUnicodeString&	cstrHostName_,
		   int						iPortNumber_,
		   Communication::Socket::Family::Value eFamily_,
		   int						iMasterID_,
		   int						iSlaveID_)
	: Object(Object::Type::Port),
	  m_bReuse(false)
{
	m_pConnection = new Communication::RemoteClientConnection(cstrHostName_,
															  iPortNumber_,
															  iMasterID_,
															  iSlaveID_,
															  eFamily_);
}

//	FUNCTION public
//	Client2::Port::~Port -- デストラクタ
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

Port::~Port()
{
	delete m_pConnection;
}

//	FUNCTION public
//	Client2::Port::getMasterID -- マスター ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		マスター ID
//
//	EXCEPTIONS

int
Port::getMasterID()
{
	return m_pConnection->getMasterID();
}

//	FUNCTION public
//	Client2::Port::getSlaveID -- スレーブ ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スレーブ ID
//
//	EXCEPTIONS

int
Port::getSlaveID()
{
	return m_pConnection->getSlaveID();
}

//	FUNCTION public
//	Client2::Port::open -- オープンする
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
Port::open()
{
	m_pConnection->open();
}

//	FUNCTION public
//	Client2::Port::close -- クローズする
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
Port::close()
{
	m_pConnection->close();
}

//	FUNCTION public
//	Client2::Port::sync -- サーバと同期を取る
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
Port::sync()
{
	m_pConnection->sync();
}

//	FUNCTION public
//	Client2::Port::readObject -- オブジェクトを読み込む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Externalizable*
//		読み込んだオブジェクト
//
//	EXCEPTIONS

Common::Externalizable*
Port::readObject()
{
	ModAutoPointer<Common::Externalizable>	pObject = m_pConnection->readObject();
	if (pObject.get()) {

		Common::ClassID::Value	classID = static_cast<Common::ClassID::Value>(pObject->getClassID());
		if (classID == Common::ClassID::ErrorLevelClass) {

			Common::ErrorLevel*	level = dynamic_cast<Common::ErrorLevel*>(pObject.get());
			m_bReuse = (level->getLevel() == Common::ErrorLevel::User);

			// ErrorLevel の次は必ず例外
			pObject = m_pConnection->readObject();
			Common::ExceptionObject*	pException = dynamic_cast<Common::ExceptionObject*>(pObject.get());
			// 例外を投げる
			pException->throwClassInstance();

		} else if (classID == Common::ClassID::ExceptionClass) {

			Common::ExceptionObject*	pException = dynamic_cast<Common::ExceptionObject*>(pObject.get());
			// 例外を投げる
			pException->throwClassInstance();
		}

		return pObject.release();
	}

	return 0;
}

//	FUNCTION public
//	Client2::Port::readObject -- オブジェクトを読み込む
//
//	NOTES
//
//	ARGUMENTS
//	Common::Externalizable*	pData_
//		データを格納する Externalizable オブジェクト
//
//	RETURN
//	Common::Externalizable*
//		読み込んだオブジェクト
//
//	EXCEPTIONS

Common::Externalizable*
Port::readObject(Common::Externalizable*	pData_)
{
	ModAutoPointer<Common::Externalizable>	pObject = m_pConnection->readObject(pData_);
	if (pObject.get()) {

		Common::ClassID::Value	classID = static_cast<Common::ClassID::Value>(pObject->getClassID());
		if (classID == Common::ClassID::ErrorLevelClass) {

			Common::ErrorLevel*	level = dynamic_cast<Common::ErrorLevel*>(pObject.get());
			m_bReuse = (level->getLevel() == Common::ErrorLevel::User);

			// ErrorLevel の次は必ず例外
			pObject = m_pConnection->readObject();
			Common::ExceptionObject*	pException = dynamic_cast<Common::ExceptionObject*>(pObject.get());
			// 例外を投げる
			pException->throwClassInstance();

		} else if (classID == Common::ClassID::ExceptionClass) {

			Common::ExceptionObject*	pException = dynamic_cast<Common::ExceptionObject*>(pObject.get());
			// 例外を投げる
			pException->throwClassInstance();
		}

		return pObject.release();
	}

	return 0;
}

//	FUNCTION public
//	Client2::Port::readIntegerData -- IntegerDataを読み込む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::IntegerData*
//		読み込んだ IntegerData
//
//	EXCEPTIONS

Common::IntegerData*
Port::readIntegerData()
{
	ModAutoPointer<Common::Externalizable>	pObject = readObject();
	Common::IntegerData*	pData = dynamic_cast<Common::IntegerData*>(pObject.get());
	if (pData == 0) {
		// ここに来るのはサーバとの同期が取れていないこと
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	pObject.release();
	return pData;
}

//	FUNCTION public
//	Client2::Port::readStringData -- StringData を読み込む
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::StringData*
//		読み込んだ StringData
//
//	EXCEPTIONS

Common::StringData*
Port::readStringData()
{
	ModAutoPointer<Common::Externalizable>	pObject = readObject();
	Common::StringData*	pData = dynamic_cast<Common::StringData*>(pObject.get());
	if (pData == 0) {
		// ここに来るのはサーバとの同期が取れていないこと
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	pObject.release();
	return pData;
}

//	FUNCTION public
//	Client2::Port::readStatus -- ステータスを読み込む
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

Common::Status::Type
Port::readStatus()
{
	ModAutoPointer<Common::Externalizable>	pObject = readObject();
	Common::Status* pStatus = dynamic_cast<Common::Status*>(pObject.get());
	if (pStatus == 0) {
		//ここに来るのはサーバとの同期が取れていないこと
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	return pStatus->getStatus();
}

//	FUNCTION public
//	Client2::Port::writeObject -- オブジェクトを書き出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Externalizable*	pObject_
//		書き出すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Port::writeObject(const Common::Externalizable*	pObject_)
{
	m_pConnection->writeObject(pObject_);
}

//	FUNCTION public
//	Client2::Port::flush -- 出力をフラッシュする
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
Port::flush()
{
	m_pConnection->flush();
}

//	FUNCTION public
//	Client2::Port::getWorkerID -- ワーカ ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ワーカ ID
//
//	EXCEPTIONS

int
Port::getWorkerID()
{
	return m_iWorkerID;
}

//	FUNCTION public
//	Client2::Port::setWorkerID -- ワーカ ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int	iWorkerID_
//		ワーカ ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Port::setWorkerID(int	iWorkerID_)
{
	m_iWorkerID = iWorkerID_;
}

//	FUNCTION public
//	Client2::Port::isReuse
//		-- 例外が発生した場合に、このポートが再利用可能かどうかを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		再利用可能な場合は true 、それ以外の場合は false
//
//	EXCEPTIONS

bool
Port::isReuse()
{
	return m_bReuse;
}

//
//	FUNCTION public
//	Client2::Port::reset
//		-- 通信路を再利用するので、保存している状態などを初期化する
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
Port::reset()
{
	// 再利用可能フラグをリセットする
	m_bReuse = false;
}

//
//	Copyright (c) 2006, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
