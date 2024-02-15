// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.cpp -- コネクションの共通基底クラス
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Communication/Connection.h"
#include "Communication/AuthorizeMode.h"
#include "Communication/Crypt.h"	// 暗号化対応
#include "Common/Status.h"
#include "ModAutoPointer.h"

_TRMEISTER_USING

using namespace Communication;

//
//	FUNCTION public
//	Communication::Connection::Connection -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iMasterID_
//		マスタID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Connection::Connection(int iMasterID_, int iSlaveID_)
: m_pSerialIO(0), m_fOpen(false),
  m_iMasterID(iMasterID_), m_iSlaveID(iSlaveID_), m_bCancel(false)
{
}

//
//	FUNCTION public
//	Communication::Connection::~Connection -- デストラクタ
//
//	NOTES
//	デストラクタ
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
Connection::~Connection()
{
}

//
//	FUNCTION public
//	Communication::Connection::getType -- コネクションタイプを得る
//
//	NOTES
//	コネクションタイプを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::SerialIO::Type
//		コネクションタイプ
//
//	EXCEPTIONS
//	なし
//
SerialIO::Type
Connection::getType() const
{
	return m_pSerialIO->getType();
}

//
//	FUNCTION public
//	Communication::Connection::isOpened -- オープンされているか
//
//	NOTES
//	オープンされているか
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		オープンされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Connection::isOpened() const
{
	return m_fOpen;
}

//
//	FUNCTION public
//	Communication::Connection::readObject -- オブジェクトを読む
//
//	NOTES
//	オブジェクトを読む
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Externalizable*
//		読込んだオブジェクト
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable*
Connection::readObject()
{
	return m_pSerialIO->readObject();
}

//	FUNCTION public
//	Communication::Connection::readObject -- オブジェクトを読む
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
//	その他
//		下位の例外はそのまま再送

Common::Externalizable*
Connection::readObject(Common::Externalizable*	pData_)
{
	return m_pSerialIO->readObject(pData_);
}

//
//	FUNCTION public
//	Communication::Connection::writeObject -- オブジェクトを書く
//
//	NOTES
//	オブジェクトを書く
//
//	ARGUMENTS
//	const Common::Externalizable* pObject_
//		オブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Connection::writeObject(
	const Common::Externalizable* pObject_)
{
	m_pSerialIO->writeObject(pObject_);
}

//
//	FUNCTION public
//	Communication::Connection::flush -- 書き出しをフラッシュする
//
//	NOTES
//	書き出しバッファをフラッシュする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Connection::flush()
{
	m_pSerialIO->flush();
}

//
//	FUNCTION public
//	Communication::Connection::wait -- 入力があるまで待つ
//
//	NOTES
//	入力があるまで指定時間待つ。
//
//	ARGUMENTS
//	int iMilliseconds_
//		待つ時間(ミリ秒)。-1を与えると永遠に待つ。
//
//	RETURN
//	入力があった場合はtrue、それ以外の場合はfalse。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
Connection::wait(int iMilliseconds_)
{
	return m_pSerialIO->wait(iMilliseconds_);
}

//
//	FUNCTION public
//	Communication::Connection::setSerialIO -- シリアルIOを設定する
//
//	NOTES
//	シリアルIOを設定する
//
//	ARGUMENTS
//	Communication::SerialIO* pSerialIO_
//		シリアルIO
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Connection::setSerialIO(SerialIO* pSerialIO_)
{
	m_pSerialIO = pSerialIO_;
}

//
//	FUNCTION public
//	Communication::Connection::getMasterID -- マスターIDを得る
//
//	NOTES
//	マスターIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		マスターID
//
//	EXCEPTIONS
//	なし
//
int
Connection::getMasterID() const
{
	return m_iMasterID & CryptMode::MaskMasterID;//暗号化対応
}

//
//	FUNCTION public
//	Communication::Connection::getFullMasterID -- FullマスターIDを得る(暗号化対応)
//
//	NOTES
//	プロトコルバージョン以外も含めたマスターIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		マスターID
//
//	EXCEPTIONS
//	なし
//
int
Connection::getFullMasterID() const
{
	return m_iMasterID;
}

//
//	FUNCTION public
//	Communication::Connection::getSlaveID -- スレーブIDを得る
//
//	NOTES
//	スレーブIDを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スレーブID
//
//	EXCEPTIONS
//	なし
//
int
Connection::getSlaveID() const
{
	return m_iSlaveID;
}

//
//	FUNCTION public
//	Communication::Connection::setFullMasterID -- マスターIDを設定する
//
//	NOTES
//	マスターIDを設定する
//
//	ARGUMENTS
//	int iMasterID_
//		マスターID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Connection::setFullMasterID(int iMasterID_)
{
	m_iMasterID = iMasterID_;
}

//
//	FUNCTION public
//	Communication::Connection::setSlaveID -- スレーブIDを設定する
//
//	NOTES
//	スレーブIDを設定する
//
//	ARGUMENTS
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Connection::setSlaveID(int iSlaveID_)
{
	m_iSlaveID = iSlaveID_;
}

//
//	FUNCTION public
//	Communication::Connection::cancel -- キャンセルフラグを設定する
//
//	NOTES
//	Server::Connectionのみがこのメソッドを実行できる。
//	クライアントはServer::Connectionに対して、キャンセルのリクエストを送る。
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
Connection::cancel()
{
	m_bCancel = true;
}

//
//	FUNCTION public
//	Communication::Connection::isCanceled -- キャンセル要求がきているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool: true --- キャンセルメッセージが投げられた
//	      false -- キャンセルメッセージは投げられていない
//
//	EXCEPTIONS
//	なし
//
bool
Connection::isCanceled() const
{
	return m_bCancel;
}

//
//	FUNCTION public
//	Communication::Connection::clearCancel -- キャンセルフラグをリセットする
//
//	NOTES
//	Server::Connectionがコネクションを再利用するときに使用する。
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
void
Connection::clearCancel()
{
	m_bCancel = false;
}


//
//	FUNCTION public
//	Communication::Connection::getAlgorithm -- 使用されている暗号アルゴリズムを返す
//
//	NOTES
//	使用されている暗号アルゴリズムを返す(暗号化対応)
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		暗号アルゴリズム
//
//	EXCEPTIONS
//	なし
//
int
Connection::getAlgorithm() const
{
	return m_iMasterID & CryptMode::MaskAlgorithm;
}

//
//	FUNCTION public
//	Communication::Connection::setKey -- 共通鍵設定
//
//	NOTES
//	共通鍵設定(暗号化対応)
//
//	ARGUMETNS
//	CryptKey::Pointer pKey_
//		共通鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Connection::setKey(const CryptKey::Pointer& pKey_)
{
	m_pSerialIO->setKey(pKey_);
}

//
//	FUNCTION public
//	Communication::Connection::getKey -- 共通鍵取得
//
//	NOTES
//	共通鍵取得(暗号化対応)
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	CryptKey::Pointer
//		共通鍵
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const CryptKey::Pointer&
Connection::getKey()
{
	return m_pSerialIO->getKey();
}

// FUNCTION public
//	Communication::Connection::getAuthorization -- 認証方式を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Connection::
getAuthorization() const
{
	return m_iMasterID & AuthorizeMode::MaskMode;
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
