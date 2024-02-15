// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Crypt.h -- Cryptクラス
// 
// Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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
#ifndef __TRMEISTER_COMMUNICATION_CRYPT_H
#define __TRMEISTER_COMMUNICATION_CRYPT_H

#include "Communication/Module.h"
#include "Communication/CryptMode.h"

#include "Common/ObjectPointer.h"
#include "Common/SafeExecutableObject.h"

#include "Os/CriticalSection.h"

_TRMEISTER_BEGIN

namespace Communication
{

// 鍵class
class CryptKey : public Common::SafeExecutableObject
{
public:
	// CryptKeyを扱うためのポインタ
	typedef Common::ObjectPointer<CryptKey> Pointer;
	
	// コンストラクタ
	CryptKey();
	// デストラクタ
	virtual ~CryptKey();
	// 鍵設定
	void setKey(CryptMode::Value mode_, int size_, const unsigned char* key_);
	// 設定された鍵のアルゴリズム取得
	CryptMode::Value getMode() const;
	// 設定された鍵のサイズ取得
	int getSize() const;
	// 設定された鍵取得
	const unsigned char* getKey() const;
	
private:
	CryptMode::Value	m_eMode;	// 暗号アルゴリズム
	int					m_iSize;	// 鍵サイズ
	unsigned char*		m_pKey;		// 鍵
};

///////////////////////////////////////////////////////////////////////////////
//
//	CLASS
//	Communication::Crypt -- Cryptクラス
//	
//	NOTES
//	暗号化／復号を行うクラス
//
class Crypt
{
public:
	// コンストラクター
	Crypt() {}
	// デストラクター
	virtual	~Crypt() {}
	// 公開・秘密の鍵生成
	virtual	void	generatePairKey(
							CryptMode::Value 	mode_,			// アルゴリズム
							CryptKey&			publicKey_,		// 公開鍵
							CryptKey&			privateKey_)=0;	// 秘密鍵
	// 共通鍵の生成
	virtual	void	generateCommonKey(
							CryptMode::Value 	mode_,			// アルゴリズム
							CryptKey&			comKey_)=0;		// 共通鍵
	// 共通鍵を公開鍵で暗号化する為に必要なサイズ
	virtual	int		getEncryptCommonKeySize(
							const CryptKey&		key_)=0;		// 公開鍵
	// 共通鍵を秘密鍵で復号する為に必要なサイズ
	virtual	int		getDecryptCommonKeySize(
							const CryptKey&		key_)=0;		// 秘密鍵
	// ブロックサイズ(暗号化・復号時に増える可能性のあるサイズ)
	virtual	int		getCipherCTXBlockSize(
							const CryptKey*		key_)=0;		// アルゴリズム

	// 初期化(encrypt/decryptを使用する前に実行する事)
	virtual	void	initialize(
							const CryptKey::Pointer&	key_)=0;		// 鍵
	// 終了処理(encrypt/decryptを使用後に実行する事)
	virtual	void	terminate()=0;
	// 暗号化
	virtual	void	encrypt(int						inSize_,	// データサイズ
							const unsigned char*	inData_,	// データ
							int&					outSize_,	// 結果サイズ
							unsigned char*			outData_)=0;// 結果データ
	// 復号
	virtual	void	decrypt(int						inSize_,	// データサイズ
							const unsigned char*	inData_,	// データ
							int&					outSize_,	// 結果サイズ
							unsigned char*			outData_)=0;// 結果データ

	// 共通鍵取得
	const CryptKey::Pointer& getKey()
	{
		return m_pKey;
	}
	// レジストリよりシステムの暗号モードを返す
	static CryptMode::Value	getMode();
	// 公開鍵取得
	static const CryptKey::Pointer& getPublicKey();
	// 秘密鍵取得
	static const CryptKey::Pointer& getPrivateKey();
	
protected:
	// 共通鍵設定
	void	setKey(const CryptKey::Pointer& pKey_)
	{
		m_pKey = pKey_;
	}
	
private:
	// 鍵
	CryptKey::Pointer	m_pKey;

	// SSL初期化
	static void initializeSSL();
};

}
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CRYPT_H

//
//	Copyright (c) 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
