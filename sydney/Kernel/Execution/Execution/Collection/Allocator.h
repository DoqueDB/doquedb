// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Allocator.h -- Collectionで扱うデータのメモリを確保するクラス
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_COLLECTION_ALLOCATOR_H
#define __SYDNEY_EXECUTION_COLLECTION_ALLOCATOR_H

#include "Execution/Collection/Module.h"

#include "Common/LargeVector.h"

#include "Os/CriticalSection.h"

#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

//
//	CLASS
//	Execution::Collection::Allocator
//		-- コレクションで扱いデータのメモリを確保する
//
//	NOTES
//	mmap でブロックサイズごとにメモリを確保し、切り出して提供する
//	設定されている上限を超えたメモリを確保しようとしたら、例外を送出する
//	提供したメモリを戻す操作を行うメソッドは提供しない
//
class Allocator
{
public:
	// コンストラクタ
	Allocator(ModUInt64 ulMaxSize_ = 0);
	// デストラクタ
	virtual ~Allocator();

	// すべてのメモリを解放する
	void clear();

	// メモリを確保する
	char* get(ModSize uiSize_);

private:
	// メモリクラス
	class Memory
	{
		//【注意】無駄なので、virtual 関数は定義しないこと

	public:
		Memory()
			: m_pPointer(0), m_uiAllocSize(0), m_uiUsedSize(0) {}
		~Memory() { deallocate(); }

		void allocate(ModSize uiSize_);
		void deallocate();
		
		char*	m_pPointer;
		ModSize	m_uiAllocSize;
		ModSize m_uiUsedSize;
	};
	
	// 最大サイズ
	ModUInt64 m_ulMaxSize;
	// 残メモリ量
	ModUInt64 m_ulRestSize;

	// 確保したメモリの配列
	Common::LargeVector<Memory*> m_vecpMemory1;
	Common::LargeVector<Memory*> m_vecpMemory2;

	// 現在のメモリクラス
	Memory* m_pMemory;

	// 排他制御用
	Os::CriticalSection m_cLatch;
};

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif //__SYDNEY_EXECUTION_COLLECTION_ALLOCATOR_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
