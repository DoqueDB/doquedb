// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CryptCodec.h -- ModCodecを親にもつCryptクラス
// 
// Copyright (c) 2006, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CRYPTCODEC_H
#define __TRMEISTER_COMMUNICATION_CRYPTCODEC_H

#include "Communication/Module.h"
#include "Communication/CryptMode.h"
#include "Communication/Crypt.h"
#include "ModCodec.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::CryptCodec -- ModCodecを親にもつCryptクラス
//	
//	NOTES
//	ModCodecを引き継ぎ暗号化／復号を行うクラス
//
class SYD_COMMUNICATION_FUNCTION CryptCodec : public ModCodec
{
public:
	// コンストラクター
	CryptCodec(ModSize bufferSize_);
	// デストラクター
	virtual ~CryptCodec();

	// ModCode の virtual
	void	encodeFlush(ModSerialIO* io_);
	void	decodeFlush(ModSerialIO* io_, ModSize min_ = 1);
	bool    hasBufferedData() const;
	
	// 共通鍵設定
	void	setKey(const CryptKey::Pointer& pKey_);
	// 共通鍵取得
	const CryptKey::Pointer& getKey();

	static Crypt*	getCrypt();
	static void		releaseCrypt(Crypt* pCrypt_);
	
private:
	// 暗号用バッファー
	unsigned char*	m_pCryptBuffer;
	// 暗号用バッファーサイズ
	int				m_iCryptBufferSize;
	// 暗号用ブロックサイズ
	int				m_iCryptBlockSize;
	// 暗号クラス
	Crypt*			m_pCrypt;

	// setKey時にすでにバッファにデータが存在している場合、そのサイズ
	int 			m_iExistDecodeBufferSize;
};

}
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CRYPTCODEC_H

//
//	Copyright (c) 2006, 2008, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
