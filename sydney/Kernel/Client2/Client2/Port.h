// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Port.h -- 通信ポート関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_PORT_H
#define __TRMEISTER_CLIENT2_PORT_H

#include "Client2/Object.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"
#include "Common/Status.h"
#include "Common/DataArrayData.h"
#include "Communication/Connection.h"
#include "Communication/Socket.h"

_TRMEISTER_BEGIN
// 暗号化対応
namespace Communication
{
	class CryptKey;
};
_TRMEISTER_CLIENT2_BEGIN

//	CLASS
//	Client2::Port -- 通信ポートをあらわすクラス
//
//	NOTES

class Port : public Object
{
public:

	// コンストラクタ ( InProcess 用 )
	Port(int iMasterID_,
		 int iSlaveID_);

	// コンストラクタ ( RemoteServer 用 )
	Port(const ModUnicodeString& cstrHostName_,
		 int iPortNumber_,
		 Communication::Socket::Family::Value eFamily_,
		 int iMasterID_,
		 int iSlaveID_);

	// デストラクタ
	virtual ~Port();

	// マスター ID を得る
	int getMasterID();

	// スレーブ ID を得る
	int getSlaveID();

	// オープンする
	void open();

	// クローズする
	void close();

	// サーバと同期を取る
	void sync();

	// オブジェクトを読み込む
	Common::Externalizable* readObject();

	// オブジェクトを読み込む
	Common::Externalizable* readObject(Common::Externalizable*	pData_);

	// IntegerData を読み込む
	Common::IntegerData* readIntegerData();

	// StringData を読み込む
	Common::StringData* readStringData();

	// Status を読み込む
	Common::Status::Type readStatus();

	// オブジェクトを書き出す
	void writeObject(const Common::Externalizable* pObject_);

	// 出力をフラッシュする
	void flush();

	// ワーカ ID を得る
	int getWorkerID();

	// ワーカ ID を設定する
	void setWorkerID(int iWorkerID_);

	// 例外が発生した場合に、このポートが再利用可能かどうかを得る
	bool isReuse();

	// 使用されている暗号アルゴリズムを返す(暗号化対応)
	int getAlgorithm() const
	{
		return m_pConnection->getAlgorithm();
	}
	// 共通鍵設定(暗号化対応)
	void setKey(Communication::CryptKey::Pointer pKey_)
	{
		m_pConnection->setKey(pKey_);
	}

	// 認証方式を得る
	int getAuthorization() const
	{
		return m_pConnection->getAuthorization();
	}

	// 再利用のため、初期化する
	void reset();

	// 入力があるまで待つ
	bool wait(int iMilliseconds_)
		{ return m_pConnection->wait(iMilliseconds_); }

private:
	// コネクション
	Communication::Connection*	m_pConnection;

	// ワーカ ID
	int							m_iWorkerID;

	// エラー時に再利用可能かどうか
	bool						m_bReuse;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_PORT_H

//
//	Copyright (c) 2006, 2007, 2008, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
