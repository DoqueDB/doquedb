// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SerialIO.cpp -- 通信を行うクラスの共通基底クラス
// 
// Copyright (c) 1999, 2001, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Communication/SerialIO.h"

#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"

_TRMEISTER_USING

using namespace Communication;

namespace {
	CryptKey::Pointer _NullCryptKey;
}

//
//	FUNCTION public
//	Communication::SerialIO::SerialIO -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Communication::SerialIO::Type
//		コネクションタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
SerialIO::SerialIO(SerialIO::Type eType_)
: m_eType(eType_), m_pInputArchive(0), m_pOutputArchive(0)
{
}

//
//	FUNCTION public
//	Communication::SerialIO::~SerialIO -- デストラクタ
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
SerialIO::~SerialIO()
{
}

//
//	FUNCTION public
//	Communication::SerialIO::readObject -- オブジェクトを読み込む
//
//	NOTES
//	オブジェクトを読み込む
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	読み込んだオブジェクト
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable*
SerialIO::readObject()
{
	return m_pInputArchive->readObject();
}

//	FUNCTION public
//	Communication::SerialIO::readObject -- オブジェクトを読み込む
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
SerialIO::readObject(Common::Externalizable*	pData_)
{
	return m_pInputArchive->readObject(pData_);
}

//
//	FUNCTION public
//	Communication::SerialIO::writeObject -- オブジェクトを書き出す
//
//	NOTES
//	オブジェクトを書き出す
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
SerialIO::writeObject(const Common::Externalizable* pObject_)
{
	m_pOutputArchive->writeObject(pObject_);
}

//
//	FUNCTION public
//	Communication::SerialIO::readInteger -- 32ビット整数を読み込む
//
//	NOTES
//	32ビット整数を読み込む
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	32ビット整数
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
SerialIO::readInteger()
{
	int iValue;
	m_pInputArchive->readArchive(iValue);
	return iValue;
}

//
//	FUNCTION public
//	Communication::SerialIO::writeInteger -- 32ビット整数を書き込む
//
//	NOTES
//	32ビット整数を書き込む
//
//	ARGUMENTS
//	int iValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SerialIO::writeInteger(int iValue_)
{
	m_pOutputArchive->writeArchive(iValue_);
}

//
//	FUNCTION public
//	Communication::SerialIO::flush -- 出力をフラッシュする
//
//	NOTES
//	出力ストリームをフラッシュする
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
SerialIO::flush()
{
	m_pOutputArchive->flush();
}

//
//	FUNCTION public
//	Communication::SerialIO::getType -- コネクションタイプを得る
//
//	NOTES
//	コネクションタイプを得る
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
SerialIO::getType() const
{
	return m_eType;
}

//
//	FUNCTION public
//	Communication::SerialIO::getKey -- 共通鍵を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Communication::CryptKey::Pointer&
//		共通鍵(本、基底クラスではつねのnullへのポインタ)
//
//	EXCEPTIONS
//
const CryptKey::Pointer&
SerialIO::getKey()
{
	return _NullCryptKey;
}

//
//	FUNCTION protected
//	Communication::SerialIO::allocateArchive -- アーカイブを作成する
//
//	NOTES
//	アーカイブを作成する
//
//	ARGUMENTS
//	ModSerialIO& cInput_
//		入力用のシリアル入出力
//	ModSericalIO& cOutput_
//		出力用のシリアル入出力
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SerialIO::allocateArchive(ModSerialIO& cInput_,
										 ModSerialIO& cOutput_)
{
	m_pInputArchive = new Common::InputArchive(cInput_);
	m_pOutputArchive = new Common::OutputArchive(cOutput_);
}

//
//	FUNCTION protected
//	Communication::SerialIO::deallocateArchive -- アーカイブを削除する
//
//	NOTES
//	ストリームを削除する
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
SerialIO::deallocateArchive()
{
	delete m_pInputArchive;
	delete m_pOutputArchive;
}

//
//	Copyright (c) 1999, 2001, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
