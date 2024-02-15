// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.h -- メモリーIOクラス
// 
// Copyright (c) 1999, 2000, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_MEMORY_H
#define __TRMEISTER_COMMUNICATION_MEMORY_H

#include "Communication/Module.h"
#include "Common/Object.h"
#include "Os/CriticalSection.h"
#include "ModSerialIO.h"
#include "ModConditionVariable.h"

_TRMEISTER_BEGIN

namespace Communication
{

//
//	CLASS
//	Communication::Memory -- メモリーのIOクラス
//
//	NOTES
//	メモリーをかえして、IOをするためのクラス
//	ソケットではModのModClientSocketやModServerSocketに対応する。
//
class SYD_COMMUNICATION_FUNCTION Memory : public Common::Object, public ModSerialIO
{
public:
	//コンストラクタ(1)
	Memory(int iSize_ = 16*1024);
	//コンストラクタ(2)
	Memory(void* pAddress, int iSize);
	//デストラクタ
	virtual ~Memory();

	//シリアライズに必要な関数群
	int readSerial(void* pBuffer_, ModSize iByte_,
				   ModSerialIO::DataType eType_);
	int writeSerial(const void* pBuffer_, ModSize iBute_,
					ModSerialIO::DataType eType_);

	// 出力をフラッシュする
	void writeFlushSerial();

	//書き込みがあるまで待つ
	bool wait(int iMilliseconds_);

	//リリースする
	int release();
	
private:
	//現在位置のオフセット
	int m_iCurrentReadPosition;
	int m_iCurrentWritePosition;

	//メモリの先頭へのポインタ
	void* m_pHeadAddress;
	//メモリ全体のサイズ
	const int m_iTotalSize;

	//クリティカルセクション
	Os::CriticalSection m_cCriticalSection;
	//イベント
	ModConditionVariable m_cReadConditionVariable;
	ModConditionVariable m_cWriteConditionVariable;
	//書き込みまたは読み込み待ちをしているかどうか
	bool m_bReadWait;
	bool m_bWriteWait;
	//自分で確保したメモリーかどうか
	bool m_bOwner;
	//リリースされた回数
	int m_iRelease;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_MEMORY_H

//
//	Copyright (c) 1999, 2000, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
