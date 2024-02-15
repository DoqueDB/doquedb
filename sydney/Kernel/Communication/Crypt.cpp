// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Crypt.cpp -- Cryptクラス
// 
// Copyright (c) 2006, 2007, 2008, 2013, 2017, 2023 Ricoh Company, Ltd.
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
#include "Communication/Crypt.h"
#include "Communication/CryptCodec.h"
#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Exception/NotSupported.h"

#include "Os/Memory.h"
#include "Os/AutoCriticalSection.h"

#include "ModUnicodeString.h"

_TRMEISTER_USING
using namespace Communication;

namespace
{
	// 公開鍵(暗号化対応)
	CryptKey::Pointer	_pPublicKey;
	// 秘密鍵(暗号化対応)
	CryptKey::Pointer	_pPrivateKey;
	
	// 暗号アルゴリズム(暗号化対応)
	CryptMode::Value	_eCryptMode = CryptMode::Unknown;

	// 初期化しているかどうか
	bool				_bCryptModeInit = false;

	// 排他制御用のクリティカルセクション
	Os::CriticalSection _cCriticalSection;
}

//
//	FUNCTION public
//	Communication::CryptKey::CryptKey -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
CryptKey::CryptKey()
: m_eMode(CryptMode::Unknown), m_iSize(0), m_pKey(0)
{
}

//
//	FUNCTION public
//	Communication::CryptKey::~CryptKey -- デストラクタ
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
CryptKey::~CryptKey()
{
	m_eMode = CryptMode::Unknown;
	m_iSize = 0;
	if(m_pKey){
		void* pKey = m_pKey;
		Os::Memory::free(pKey);
		m_pKey = 0;
	}
}

//
//	FUNCTION public
//	Communication::CryptKey::setKey -- 共通鍵設定
//
//	NOTES
//	共通鍵を設定
//
//	ARGUMENTS
//	Communication::CryptMode::Value mode_
//		暗号アルゴリズム
//	int size_
//		鍵サイズ
//	const unsigned char* key_
//		鍵データ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
CryptKey::setKey(CryptMode::Value mode_, int size_, const unsigned char* key_)
{
	_TRMEISTER_ASSERT(m_pKey==0);
	m_eMode = mode_;
	m_iSize = size_;
	if (size_ > 0)
	{
		m_pKey = static_cast<unsigned char*>( Os::Memory::allocate(size_) );
		Os::Memory::copy(m_pKey, key_, size_);
	}
}

//
//	FUNCTION public
//	Communication::CryptKey::getMode -- 設定された鍵のアルゴリズム取得
//
//	NOTES
//	設定された鍵のアルゴリズム取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::CryptMode::Value
//		暗号アルゴリズム
//
//	EXCEPTIONS
//	なし
//
CryptMode::Value
CryptKey::getMode() const
{
	return m_eMode;
}

//
//	FUNCTION public
//	Communication::CryptKey::getSize -- 設定された鍵のサイズ取得
//
//	NOTES
//	設定された鍵のサイズ取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		鍵サイズ
//
//	EXCEPTIONS
//	なし
//
int
CryptKey::getSize() const
{
	return m_iSize;
}

//
//	FUNCTION public
//	Communication::CryptKey::getKey -- 設定された鍵データ取得
//
//	NOTES
//	設定された鍵データ取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const unsigned char*
//		鍵データ
//
//	EXCEPTIONS
//	なし
//
const unsigned char*
CryptKey::getKey() const
{
	return m_pKey;
}

////////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	Communication::Crypt::initializeSSL -- SSL初期化
//
//	NOTES
//	SSL初期化
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
Crypt::initializeSSL()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::Crypt::getMode -- システムの暗号アルゴリズム取得
//
//	NOTES
//	システムの暗号アルゴリズム取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::CryptMode::Value
//		暗号アルゴリズム
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
CryptMode::Value
Crypt::getMode()
{
	// SSL初期化
	initializeSSL();
	return _eCryptMode;
}

//
//	FUNCTION public
//	Communication::Crypt::getPublicKey -- 公開鍵取得
//
//	NOTES
//	公開鍵取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const CryptKey&
//		公開鍵
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const CryptKey::Pointer&
Crypt::getPublicKey()
{
	// SSL初期化
	initializeSSL();
	return _pPublicKey;
}

//
//	FUNCTION public
//	Communication::Crypt::getPrivateKey -- 秘密鍵取得
//
//	NOTES
//	秘密鍵取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const CryptKey&
//		秘密鍵
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const CryptKey::Pointer&
Crypt::getPrivateKey()
{
	// SSL初期化
	initializeSSL();
	return _pPrivateKey;
}

//
//	Copyright (c) 2006, 2007, 2008, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
