// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SerialIO.h -- 通信を行うクラスの基底クラス
// 
// Copyright (c) 1999, 2000, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_SERIALIO_H
#define __TRMEISTER_COMMUNICATION_SERIALIO_H

#include "Communication/Module.h"
#include "Communication/Crypt.h"
#include "Common/Object.h"
#include "ModSerialIO.h"


_TRMEISTER_BEGIN

namespace Common
{
class Externalizable;
class InputArchive;
class OutputArchive;
}

namespace Communication
{
//
//	CLASS
//	SerialIO -- 通信を行うクラスの基底クラス
//
//	NOTES
//	Communication::Socket, Communication::LocalMemory等の基底クラス
//
class SYD_COMMUNICATION_FUNCTION SerialIO : public Common::Object
{
public:
	//コネクションタイプ
	enum Type
	{
		Local,		//メモリ
		Shared,		//共有メモリ
		Remote,		//ソケット

		Unknown		//不明
	};

	//コンストラクタ
	SerialIO(Type eType_);
	//デストラクタ
	virtual ~SerialIO();

	//オープンする
	virtual void open() = 0;
	//クローズする
	virtual void close() = 0;

	//オブジェクトを読み込む
	virtual Common::Externalizable* readObject();
	virtual Common::Externalizable* readObject(Common::Externalizable*	pData_);
	//オブジェクトを書き出す
	virtual void writeObject(const Common::Externalizable* pObject_);

	//Integerを読み込む
	virtual int readInteger();
	//Integerを書き出す
	virtual void writeInteger(int iValue_);

	//出力をflushする
	virtual void flush();

	//入力が来るまで待つ
	virtual bool wait(int iMilliseconds_) = 0;

	//コネクションタイプを得る
	Type getType() const;

	// 共通鍵設定(暗号化対応)
	virtual void	setKey(const CryptKey::Pointer& pKey_) {};
	// 共通鍵取得(暗号化対応)
	virtual const CryptKey::Pointer& getKey();

protected:
	//アーカイブを作成する
	void allocateArchive(ModSerialIO& cInput_, ModSerialIO& cOutput_);
	//アーカイブを削除する
	void deallocateArchive();

	//InputArchive
	Common::InputArchive* m_pInputArchive;
	//OutputArchive
	Common::OutputArchive* m_pOutputArchive;

private:
	//コネクションタイプ
	Type m_eType;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_SERIALIO_H

//
//	Copyright (c) 1999, 2000, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
