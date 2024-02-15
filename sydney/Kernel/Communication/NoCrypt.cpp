// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NoCrypt.cpp -- Cryptを親にもつCryptクラス
// 
// Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
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
#include "Communication/NoCrypt.h"
#include "Exception/NotSupported.h"

_TRMEISTER_USING
using namespace Communication;

//
//	FUNCTION public
//	Communication::NoCrypt::~NoCrypt -- デストラクタ
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
NoCrypt::~NoCrypt()
{
}

//
//	FUNCTION public
//	Communication::NoCrypt::generatePairKey -- 公開・秘密鍵の生成
//
//	NOTES
//	公開・秘密鍵の生成(システム起動時に１度の実行)
//
//	ARGUMENTS
//	CryptMode 		mode_
//		アルゴリズム
//	CryptKey&		publicKey_
//		公開鍵
//	CryptKey&		privateKey_
//		秘密鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::generatePairKey(	CryptMode::Value 	mode_,		// アルゴリズム
							CryptKey&			publicKey_,	// 公開鍵
							CryptKey&			privateKey_)// 秘密鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::generateCommonKey -- 共通鍵の生成
//
//	NOTES
//	共通鍵の生成
//
//	ARGUMENTS
//	CryptMode 		mode_
//		アルゴリズム
//	CryptKey&		comKey_
//		共通鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::generateCommonKey(	CryptMode::Value 	mode_,		// アルゴリズム
							CryptKey&			comKey_)	// 共通鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::getEncryptCommonKeySize
//		-- 共通鍵を公開鍵で暗号化する為に必要なサイズ
//
//	NOTES
//	共通鍵を公開鍵で暗号化する為に必要なサイズ
//
//	ARGUMENTS
//	CryptKey&		key_
//		公開鍵
//
//	RETURN
//	int
//		サイズ
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
int
NoCrypt::getEncryptCommonKeySize(const CryptKey&	key_)	// 公開鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::getDecryptCommonKeySize
//		-- 共通鍵を秘密鍵で復号する為に必要なサイズ
//
//	NOTES
//	共通鍵を秘密鍵で復号する為に必要なサイズ
//
//	ARGUMENTS
//	CryptKey&		key_
//		秘密鍵
//
//	RETURN
//	int
//		サイズ
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
int
NoCrypt::getDecryptCommonKeySize(const CryptKey&	key_)	// 秘密鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::getCipherCTXBlockSize -- ブロックサイズ取得
//
//	NOTES
//	ブロックサイズ(暗号化・復号時に増える可能性のあるサイズ)
//
//	ARGUMENTS
//	CryptKey&		key_
//		鍵
//
//	RETURN
//	int
//		サイズ
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
int
NoCrypt::getCipherCTXBlockSize(const CryptKey*	key_)		// 鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::initialize -- 初期化
//
//	NOTES
//	初期化(encrypt/decryptを使用する前に実行する事)
//
//	ARGUMENTS
//	CryptKey&		key_
//		鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::initialize(const CryptKey::Pointer& key_)			// 鍵
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::terminate -- 終了処理
//
//	NOTES
//	終了処理(encrypt/decryptを使用後に実行する事)
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::terminate()
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::encrypt -- 暗号化
//
//	NOTES
//	暗号化
//
//	ARGUMENTS
//	int						inSize_
//		データサイズ
//	const unsigned char*	inData_
//		データ
//	int&					outSize_
//		結果サイズ
//	unsigned char*			outData_
//		結果データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::encrypt(	int						inSize_,		// データサイズ
					const unsigned char*	inData_,		// データ
					int&					outSize_,		// 結果サイズ
					unsigned char*			outData_)		// 結果データ
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Communication::NoCrypt::decrypt -- 復号
//
//	NOTES
//	復号
//
//	ARGUMENTS
//	int						inSize_
//		データサイズ
//	const unsigned char*	inData_
//		データ
//	int&					outSize_
//		結果サイズ
//	unsigned char*			outData_
//		結果データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::NotSupported
//		サポート外
//
void
NoCrypt::decrypt(	int						inSize_,		// データサイズ
					const unsigned char*	inData_,		// データ
					int&					outSize_,		// 結果サイズ
					unsigned char*			outData_)		// 結果データ
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
