// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CryptCodec.cpp -- ModCodecを親にもつCryptクラス
// 
// Copyright (c) 2007, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Communication/CryptCodec.h"
#include "Communication/NoCrypt.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/CryptLibraryError.h"

#include "Os/Memory.h"
#include "ModOsException.h"

_TRMEISTER_USING

using namespace Communication;

namespace {

	CryptKey::Pointer _NullCryptKey;
}

//
//	FUNCTION public
//	Communication::CryptCodec::CryptCodec -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	ModSize bufferSize_
//		コーデック用バッファサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
CryptCodec::CryptCodec(ModSize bufferSize_)
	: ModCodec(bufferSize_, bufferMode),
	  m_pCryptBuffer(0), m_iCryptBufferSize(0), m_iCryptBlockSize(0),
	  m_pCrypt(0), m_iExistDecodeBufferSize(0)
{
}

//
//	FUNCTION public
//	Communication::CryptCodec::~CryptCodec -- デストラクタ
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
//	その他
//		下位の例外はそのまま再送
//
CryptCodec::~CryptCodec()
{
	// 暗号用バッファー後始末
	if (m_pCryptBuffer) {
		void* pBuf = m_pCryptBuffer;
		Os::Memory::free(pBuf);
		m_pCryptBuffer=0;
	}
	// 暗号クラス解放
	if (m_pCrypt) {
		// 終了処理
		m_pCrypt->terminate();
		// 開放
		releaseCrypt(m_pCrypt);
	}
}

//
//	FUNCTION public
//	Communication::CryptCodec::encodeFlush -- エンコードバッファのフラッシュ
//
//	NOTES
//	エンコードバッファをフラッシュする。
//
//	ARGUMENTS
//	ModSerialIO* io_
//		実際にIOを行うクラスへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::CryptLibraryError
//		コーデックによるデータ部分のwriteでエラーが起きた
//  その他
//		ModSerialIOのサブクラスのrawWriteの実装で発生する例外を送出
//		下位の例外はそのまま再送
//
void
CryptCodec::encodeFlush(ModSerialIO* io_)
{
	; _TRMEISTER_ASSERT(io_ != 0);
	if (this->getEncodedSize() == 0)
		// バッファ中にデータが存在しない
		return;

	// バッファ上に未読み出しのデータは存在しないはず
	; _TRMEISTER_ASSERT(this->getDecodedSize() == 0);

	// データを書き出す
	; _TRMEISTER_ASSERT(getKey().get() == 0); 

	ModCodec::encodeFlush(io_);

}

//
//	FUNCTION public
//	Communication::CryptCodec::decodeFlush -- デコードバッファのフラッシュ
//
//	NOTES
//	デコードバッファをフラッシュする。
//
//	ARGUMENTS
//	ModSerialIO* io_
//		実際にIOを行うクラスへのポインタ
//	ModSize	min_
//		最低でも読み出す必要のあるデータのサイズ(B 単位)
//		指定されないときは、1 が指定されたものとみなす
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::CryptLibraryError
//		コーデックによるデータ部分のwriteでエラーが起きた
//	ModOsErrorReadDataInCodec
//		コーデックによるデータのreadでエラーが起きた
//	ModOsErrorEndOfFile
//		EOF から読み出そうとしたため、
//		ぜんぜんデータ部分を読み出せなかった
//  その他
//		ModSerialIOのサブクラスのrawReadの実装で発生する例外を送出
//		下位の例外はそのまま再送
//
void
CryptCodec::decodeFlush(ModSerialIO* io_, ModSize min_)
{
	; _TRMEISTER_ASSERT(io_ != 0);
	if (this->getDecodedSize() > 0)
		// バッファ中にデータが存在する
		return;

	if (this->getEncodedSize() > 0)
		// バッファ上の未書き出しのデータは捨てる
		this->reset();

	ModSize	decodeSize;

	// データを読み出す
	; _TRMEISTER_ASSERT(getKey().get() == 0);

	ModCodec::decodeFlush(io_, min_);

}

//
//	FUNCTION public
//    Communication::CryptCodec::hasBufferedData -- バッファにデータがあるか
//
//    NOTES
//
//    ARGUMENTS
//  なし
//
//    RETURN
//    bool
//      バッファにデータがあれば true, なれけば false
//
//    EXCEPTIONS
//  なし
//
bool
CryptCodec::hasBufferedData() const
{
       //      decodedSize または m_iExistDecodeBufferSize が 0 より大きければ true を返す。
       //  m_iExistDecodeBufferSize は暗号化通信に関連して、共通鍵を設定する前に
       //  データが読み込まれている場合にだけ、そのバイト数が設定される。
       //  詳細は setkey() 参照。

       return (this->decodedSize > 0 || this->m_iExistDecodeBufferSize > 0);
}

//
//    FUNCTION public
//	Communication::CryptCodec::setKey -- 共通鍵設定
//
//	NOTES
//	共通鍵を設定及び初期化処理
//
//	ARGUMENTS
//	const CryptKey* pKey_
//		共通鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::CryptLibraryError
//		指定された鍵によるブロックサイズが得られなかった
//  その他
//		下位の例外はそのまま再送
//
void
CryptCodec::setKey(const CryptKey::Pointer& pKey_)
{
	m_iExistDecodeBufferSize = 0;
}

//
//	FUNCTION public
//	Communication::CryptCodec::getKey -- 共通鍵取得
//
//	NOTES
//	共通鍵を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	共通鍵
//
//	EXCEPTIONS
//  その他
//		下位の例外はそのまま再送
//
const CryptKey::Pointer&
CryptCodec::getKey()
{
	return _NullCryptKey;
}

//
//	FUNCTION public
//	Communication::CryptCodec::getCrypt -- 暗号クラス取得
//
//	NOTES
//	システムで使用する暗号クラスを取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	暗号クラス
//
//	EXCEPTIONS
//  その他
//		下位の例外はそのまま再送
//
Crypt*
CryptCodec::getCrypt()
{
	return new NoCrypt();
}

//
//	FUNCTION public
//	Communication::CryptCodec::releaseCrypt -- 暗号クラス解放
//
//	NOTES
//	暗号クラス解放
//
//	ARGUMENTS
//	Crypt* pCrypt_
//		暗号クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//  その他
//		下位の例外はそのまま再送
//
void
CryptCodec::releaseCrypt(Crypt* pCrypt_)
{
	delete pCrypt_;
}

//
//	Copyright (c) 2007, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
