// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocalMemory -- ローカルメモリーIOクラス
// 
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_LOCALMEMORY_H
#define __TRMEISTER_COMMUNICATION_LOCALMEMORY_H

#include "Communication/Module.h"
#include "Communication/SerialIO.h"

_TRMEISTER_BEGIN

namespace Communication
{
class Memory;

//
//	CLASS
//	Communication::LocalMemory -- ローカルメモリーIOクラス
//
//	NOTES
//	ローカルメモリーのIOクラス
//
class LocalMemory : public Communication::SerialIO
{
public:
	//コンストラクタ
	LocalMemory(Memory* pInputMemory_, Memory* pOutputMemory_);
	//デストラクタ
	virtual ~LocalMemory();

	//オープンする
	void open();
	//クローズする
	void close();

	//ポインタを読み込む
	void* readPointer();
	//ポインタを書き込む
	void writePointer(void* pPointer_);

	//書き込みが来るまで待つ
	bool wait(int iMilliseconds_);

private:
	//メモリクラス
	Memory* const m_pInputMemory;
	Memory* const m_pOutputMemory;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_LOCALMEMORY_H

//
//	Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
