// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.h -- コネクションクラス
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CONNECTION_H
#define __TRMEISTER_COMMUNICATION_CONNECTION_H

#include "Communication/Module.h"
#include "Communication/SerialIO.h"
#include "Common/ExecutableObject.h"

_TRMEISTER_BEGIN

namespace Common
{
class Externalizable;
}

namespace Communication
{
	class CryptKey;		// 暗号化対応
//
//	CLASS
//	Communication::Connection -- コネクションの共通基底クラス
//
//	NOTES
//	コネクションの共通基底クラス
//
class SYD_COMMUNICATION_FUNCTION Connection : public Common::ExecutableObject
{
public:
	//コンストラクタ
	Connection(int iMasterID_, int iSlaveID_);
	//デストラクタ
	virtual ~Connection();

	//コネクションタイプを得る
	SerialIO::Type getType() const;

	//オープンする
	virtual void open() = 0;
	//クローズする
	virtual void close() = 0;

	//オープンされているか
	bool isOpened() const;

	//オブジェクトを読む
	virtual Common::Externalizable* readObject();
	virtual Common::Externalizable* readObject(Common::Externalizable*	pData_);
	//オブジェクトを書く
	virtual void writeObject(const Common::Externalizable* pObject_);

	//書き出しをフラッシュする
	virtual void flush();

	//入力があるまで待つ
	virtual bool wait(int iMilliseconds_);

	//sync
	virtual void sync() = 0;

	//シリアルIOクラスをセットする
	void setSerialIO(SerialIO* pSerialIO);

	//マスターIDを得る
	int getMasterID() const;
	//完全マスターIDを得る(暗号化対応)
	int getFullMasterID() const;
	//スレーブIDを得る
	int getSlaveID() const;
	//マスターIDを設定する
	void setFullMasterID(int iMasterID_);	// 暗号化対応(旧setMasterID)
	//スレーブIDを設定する
	void setSlaveID(int iSlaveID_);

	//中断処理(Server::Connectionのみが設定する)
	void cancel();
	//中断処理
	bool isCanceled() const;
	//中断フラグをクリアする
	void clearCancel();
	
	// 使用されている暗号アルゴリズムを返す(暗号化対応)
	int		getAlgorithm() const;
	// 共通鍵設定(暗号化対応)
	void	setKey(const CryptKey::Pointer& pKey_);
	// 共通鍵取得(暗号化対応)
	const CryptKey::Pointer& getKey();
	// 認証方式を得る
	int		getAuthorization() const;

protected:
	//シリアルIOクラス
	SerialIO* m_pSerialIO;

	//オープンされているか
	bool m_fOpen;

private:
	//マスタID
	int m_iMasterID;
	//スレーブID
	int m_iSlaveID;

	//キャンセルフラグ
	bool m_bCancel;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CONNECTION_H

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
