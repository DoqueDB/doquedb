// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NoCrypt.h -- Cryptを親にもつCryptクラス
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

#ifndef __TRMEISTER_COMMUNICATION_NO_CRYPT_H
#define __TRMEISTER_COMMUNICATION_NO_CRYPT_H

#include "Communication/Crypt.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::NoCrypt -- Cryptを親にもつCryptクラス
//	
//	NOTES
//	Cryptを引き継ぎ暗号化／復号を行うクラス
//
class NoCrypt : public Crypt
{
public:
	// コンストラクター
	NoCrypt()
		: Crypt()
	{}
	// デストラクター
	virtual ~NoCrypt();

	// 公開・秘密の鍵生成
	void	generatePairKey(
							CryptMode::Value 	mode_,			// アルゴリズム
							CryptKey&			publicKey_,		// 公開鍵
							CryptKey&			privateKey_);	// 秘密鍵
	// 共通鍵の生成
	void	generateCommonKey(
							CryptMode::Value 	mode_,			// アルゴリズム
							CryptKey&			comKey_);		// 共通鍵
	// 共通鍵を公開鍵で暗号化する為に必要なサイズ
	int		getEncryptCommonKeySize(
							const CryptKey&	key_);				// 公開鍵
	// 共通鍵を秘密鍵で復号する為に必要なサイズ
	int		getDecryptCommonKeySize(
							const CryptKey&	key_);				// 秘密鍵
	// ブロックサイズ(暗号化・復号時に増える可能性のあるサイズ)
	int		getCipherCTXBlockSize(
							const CryptKey*	key_);				// アルゴリズム

	// 初期化(encrypt/decryptを使用する前に実行する事)
	void	initialize(
					const CryptKey::Pointer& key_);				// 鍵
	// 終了処理(encrypt/decryptを使用後に実行する事)
	void	terminate();
	// 暗号化
	void	encrypt(int						inSize_,			// データサイズ
					const unsigned char*	inData_,			// データ
					int&					outSize_,			// 結果サイズ
					unsigned char*			outData_);			// 結果データ
	// 復号
	void	decrypt(int						inSize_,			// データサイズ
					const unsigned char*	inData_,			// データ
					int&					outSize_,			// 結果サイズ
					unsigned char*			outData_);			// 結果データ
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_NO_CRYPT_H

//
//	Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
