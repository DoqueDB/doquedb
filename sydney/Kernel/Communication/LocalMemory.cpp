// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocalMemory.cpp -- ローカルメモリーIOクラス
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Communication/LocalMemory.h"
#include "Communication/Memory.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/Message.h"
#include "Common/ExceptionMessage.h"
#include "Common/ExceptionObject.h"
#include "Exception/ConnectionRanOut.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING

using namespace Communication;

//
//	FUNCTION public
//	Communication::LocalMemory::LocalMemory -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	入力用のメモリクラスのインスタンスはデストラクタでdeleteされる。
//
//	ARGUMENTS
//	Memory* pInputMemory_
//		入力用のメモリークラス
//	Memory* pOutputMemory_
//		出力用のメモリークラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
LocalMemory::LocalMemory(Memory* pInputMemory_, Memory* pOutputMemory_)
	: SerialIO(SerialIO::Local), m_pInputMemory(pInputMemory_), m_pOutputMemory(pOutputMemory_)
{
}

//
//	FUNCTION public
//	Communication::LocalMemory::~LocalMemory -- デストラクタ
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
LocalMemory::~LocalMemory()
{
}

//
//	FUNCTION public
//	Communication::LocalMemory::open -- オープンする
//
//	NOTES
//	入出力アーカイブを作成する。
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
LocalMemory::open()
{
	allocateArchive(*m_pInputMemory, *m_pOutputMemory);
}

//
//	FUNCTION public
//	Communication::LocalMemory::close -- クローズする
//
//	NOTES
//	入出力アーカイブを破棄する。
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
LocalMemory::close()
{
	deallocateArchive();
	if (m_pInputMemory->release() == 2)
		delete m_pInputMemory;
	if (m_pOutputMemory->release() == 2)
		delete m_pOutputMemory;
}

//
//	FUNCTION public
//	Communication::LocalMemory::readPointer -- ポインタを読み込む
//
//	NOTES
//	ポインタを読み込む
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		ポインタ
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void*
LocalMemory::readPointer()
{
	void* pPointer;
	m_pInputArchive->readArchive(static_cast<void*>(&pPointer), sizeof(void*));
	return pPointer;
}

//
//	FUNCTION public
//	Communication::LocalMemory::writePointer -- ポインタを書き込む
//
//	NOTES
//	ポインタを書き込む
//
//	ARGUMENTS
//	void* pPointer_
//		ポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
LocalMemory::writePointer(void* pPointer_)
{
	m_pOutputArchive->writeArchive(static_cast<void*>(&pPointer_),
								   sizeof(void*));
}

//
//	FUNCTION public
//	Communication::LocalMemory::wait -- 書き込みが来るまで待つ
//
//	NOTES
//	書き込みが来るまで待つ。
//
//	ARGUMENTS
//	int iMilliseconds_
//		待つ時間(ミリ秒)
//
//	RETURN
//	書き込みがあったらtrue、それ以外の場合はfalseを返す
//
//	EXCEPTIONS
//	なし
//
bool
LocalMemory::wait(int iMilliseconds_)
{
	return m_pInputMemory->wait(iMilliseconds_);
}

//
//	Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
