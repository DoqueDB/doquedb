// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Port.h --
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

#ifndef __TRMEISTER_CLIENT_PORT_H
#define __TRMEISTER_CLIENT_PORT_H

#include "Client/Object.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"
#include "Common/Status.h"
#include "Common/DataArrayData.h"
#include "Communication/Connection.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

//
//	CLASS
//	Client::Port --
//
//	NOTES
//
//
class Port : public Object
{
public:
	//コンストラクタ(InProcess用)
	Port(int iMasterID_, int iSlaveID_);
	//コンストラクタ(RemoteServer用)
	Port(const ModUnicodeString& cstrHostName_, int iPortNumber_,
		 int iMasterID_, int iSlaveID_);
	//デストラクタ
	virtual ~Port();

	//オープン・クローズ
	void open();
	void close();

	//同期を取る
	void sync();

	//オブジェクトを書き込む
	void writeObject(const Common::Externalizable* pObject_);
	//出力をフラッシュする
	void flush();

	//オブジェクトを読み込む
	Common::Externalizable* readObject();

	//IntegerDataを読み込む
	Common::IntegerData* readIntegerData();

	//Statusを読み込む
	Common::Status::Type readStatus();

	//SlaveIDを得る
	int getSlaveID();

private:
	//コネクション
	Communication::Connection* m_pConnection;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_PORT_H

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
