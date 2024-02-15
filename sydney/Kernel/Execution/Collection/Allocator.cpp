// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Allocator.cpp --
// 
// Copyright (c) 2013, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Collection";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Execution/Collection/Allocator.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Limits.h"
#include "Os/Memory.h"

#include "Exception/MemoryExhaust.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace
{
	//
	// メモリ確保の単位
	//
	Common::Configuration::ParameterInteger _cUnitSize(
		"Execution_CollectionAllocateUnitSize",
#ifdef SYD_ARCH64
		64 << 10 << 10,		// 64MB
#else
		 1 << 10 << 10,		//  1MB
#endif
		false);

	//
	//	メモリの最大サイズ
	//
	Common::Configuration::ParameterInteger64 _cMaxSize(
		"Execution_CollectionMaxSize", Os::Limits<ModUInt64>::getMax(), false);
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::Allocator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt64 ulMaxSize_
//		最大サイズ (0を指定するとパラメータ値を採用する。defaultは0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Allocator::Allocator(ModUInt64 ulMaxSize)
	: m_ulMaxSize(ulMaxSize), m_pMemory(0)
{
	if (m_ulMaxSize == 0)
		// パラメータ値を採用する
		m_ulMaxSize = _cMaxSize.get();

	// 残メモリ量
	m_ulRestSize = m_ulMaxSize;
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::~Allocator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Allocator::~Allocator()
{
	clear();
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::clear -- すべての領域を解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Allocator::clear()
{
	Common::LargeVector<Memory*>::Iterator i = m_vecpMemory1.begin();
	for (; i != m_vecpMemory1.end(); ++i)
	{
		delete (*i);
	}
	i = m_vecpMemory2.begin();
	for (; i != m_vecpMemory2.end(); ++i)
	{
		delete (*i);
	}

	m_ulRestSize = m_ulMaxSize;
	m_vecpMemory1.clear();
	m_vecpMemory2.clear();
	m_pMemory = 0;
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::get -- メモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		確保するサイズ
//
//	RETURN
//	char*
//		確保したメモリ領域の先頭へのポインタ
//
//	EXCEPTIONS
//
char*
Allocator::get(ModSize uiSize_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	Memory* pMemory = m_pMemory;

	if (pMemory == 0 ||
		(pMemory->m_uiAllocSize - pMemory->m_uiUsedSize) < uiSize_)
	{
		// 確保するサイズ
		ModSize uiSize = _cUnitSize.get();
		if (uiSize < uiSize_)
			// ユニットサイズ以上だったら、指定サイズにする
			uiSize = uiSize_;
		
		if (m_ulRestSize < uiSize)
		{
			// メモリが不足している
			SydErrorMessage
				<< "Collection memory size has reached the upper limits."
				<< ModEndl;
			_SYDNEY_THROW0(Exception::MemoryExhaust);
		}

		ModAutoPointer<Memory> p = new Memory();
		p->allocate(uiSize);
		pMemory = p.get();
		
		m_ulRestSize -= uiSize;
		
		if (uiSize == uiSize_)
		{
			// ユニットサイズと同じか、より大きい場合、
			// m_pMemory は置き換えない
			
			m_vecpMemory2.pushBack(p.release());
		}
		else
		{
			// ユニットサイズよりも小さい場合、
			// m_pMemory を置き換える
			
			m_pMemory = pMemory;
			m_vecpMemory1.pushBack(p.release());
		}
	}

	char* p = pMemory->m_pPointer;
	p += pMemory->m_uiUsedSize;
	pMemory->m_uiUsedSize += uiSize_;

	return p;
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::Memory::allocate
//		-- メモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		確保するメモリのサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Allocator::Memory::allocate(ModSize uiSize_)
{
	m_pPointer = static_cast<char*>(Os::Memory::map(uiSize_, false));
	m_uiAllocSize = uiSize_;
	m_uiUsedSize = 0;
}

//
//	FUNCTION public
//	Execution::Collection::Allocator::Memory::deallocate
//		-- メモリを解放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Allocator::Memory::deallocate()
{
	void* p = m_pPointer;
	Os::Memory::unmap(p, m_uiAllocSize);
	
	m_pPointer = 0;
	m_uiAllocSize = 0;
	m_uiUsedSize = 0;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
//	Copyright (c) 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
