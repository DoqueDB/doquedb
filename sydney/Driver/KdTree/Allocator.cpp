// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Allocator.cpp --
// 
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/Allocator.h"

#include "KdTree/Entry.h"
#include "KdTree/Node.h"

#include "Common/Configuration.h"

#include "Exception/Object.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// バッファが一度に確保するメモリの大きさ (default 8M)
	Common::Configuration::ParameterInteger
	_cAllocateUnitSize("KdTree_AllocateUnitSize", 8 << 10 << 10, false);
}

//
//	FUNCTION public
//	KdTree::Allocator::EntryBuffer::EntryBuffer -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iDimension_
//		エントリの次元数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Allocator::EntryBuffer::EntryBuffer(int iDimension_)
	: m_iEntrySize(Entry::calcSize(iDimension_)),
	  m_pEnd(0), m_pCurrent(0), m_pPrev(0), m_pNext(0)
{
}

//
//	FUNCTION public
//	KdTree::Allocator::EntryBuffer::~EntryBuffer -- デストラクタ
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
Allocator::EntryBuffer::~EntryBuffer()
{
	// メモリ確保の単位サイズを得る
	Os::Memory::Size s = _cAllocateUnitSize.get();

	// すべてのメモリを解放する
	Common::LargeVector<char*>::Iterator i = m_vecpBuffer.begin();
	for (; i != m_vecpBuffer.end(); ++i)
	{
		void* p = *i;
		Os::Memory::unmap(p, s);
	}
	m_vecpBuffer.clear();
}

//
//	FUNCTION public
//	KdTree::Allocator::EntryBuffer::allocate -- エントリのメモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Entry*
//		確保したエントリ
//
//	EXCEPTIONS
//
Entry*
Allocator::EntryBuffer::allocate()
{
	if (m_pCurrent == m_pEnd)
	{
		// これ以上確保できないので、新しく確保する
		m_pCurrent = 0;
	}

	if (m_pCurrent == 0)
	{
		// 新しく確保する

		Os::Memory::Size s = _cAllocateUnitSize.get();
		
		m_pCurrent = syd_reinterpret_cast<char*>(Os::Memory::map(s, true));
		m_pEnd = m_pCurrent + (s / m_iEntrySize) * m_iEntrySize;
		m_vecpBuffer.pushBack(m_pCurrent);
	}

	Entry* p = syd_reinterpret_cast<Entry*>(m_pCurrent);
	m_pCurrent += m_iEntrySize;
	return p;
}

//
//	FUNCTION public
//	KdTree::Allocator::EntryBuffer::getSize
//		-- これまでに確保したメモリサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		メモリサイズ
//
//	EXCEPTIONS
//
ModUInt64
Allocator::EntryBuffer::getSize() const
{
	ModUInt64 s = _cAllocateUnitSize.get();
	return s * m_vecpBuffer.getSize();
}

//
//	FUNCTION public
//	KdTree::Allocator::NodeBuffer::NodeBuffer -- コンストラクタ
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
Allocator::NodeBuffer::NodeBuffer()
	: m_pEnd(0), m_pCurrent(0), m_pPrev(0), m_pNext(0)
{
}

//
//	FUNCTION public
//	KdTree::Allocator::NodeBuffer::~NodeBuffer -- デストラクタ
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
Allocator::NodeBuffer::~NodeBuffer()
{
	// メモリ確保の単位サイズを得る
	Os::Memory::Size s = _cAllocateUnitSize.get();

	// すべてのメモリを解放する
	Common::LargeVector<Node*>::Iterator i = m_vecpBuffer.begin();
	for (; i != m_vecpBuffer.end(); ++i)
	{
		void* p = *i;
		Os::Memory::unmap(p, s);
	}
	m_vecpBuffer.clear();
}

//
//	FUNCTION public
//	KdTree::Allocator::NodeBuffer::allocate -- ノードのメモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Node*
//		確保したノード
//
//	EXCEPTIONS
//
Node*
Allocator::NodeBuffer::allocate()
{
	if (m_pCurrent == m_pEnd)
	{
		// これ以上確保できないので、新しく確保する
		m_pCurrent = 0;
	}

	if (m_pCurrent == 0)
	{
		// 新しく確保する

		Os::Memory::Size s = _cAllocateUnitSize.get();
		
		m_pCurrent = syd_reinterpret_cast<Node*>(Os::Memory::map(s, true));
		m_pEnd = m_pCurrent + (s / sizeof(Node));
		m_vecpBuffer.pushBack(m_pCurrent);
	}

	return m_pCurrent++;
}

//
//	FUNCTION public
//	KdTree::Allocator::NodeBuffer::getSize
//		-- これまでに確保したメモリサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		メモリサイズ
//
//	EXCEPTIONS
//
ModUInt64
Allocator::NodeBuffer::getSize() const
{
	ModUInt64 s = _cAllocateUnitSize.get();
	return s * m_vecpBuffer.getSize();
}

//
//	FUNCTION public
//	KdTree::Allocator::Allocator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iDimension_
//		エントリの次元数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Allocator::Allocator(int iDimension_)
	: m_cEntryBufferList(&EntryBuffer::m_pPrev, &EntryBuffer::m_pNext),
	  m_cNodeBufferList(&NodeBuffer::m_pPrev, &NodeBuffer::m_pNext),
	  m_iDimensionSize(iDimension_)
{
}

//
//	FUNCTION public
//	KdTree::Allocator::~Allocator -- デストラクタ
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
	clearEntry();
	clearNode();
}

//
//	FUNCTION public
//	KdTree::Allocator::allocateEntry -- エントリ用のメモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Entry*
//		確保したエントリ
//
//	EXCEPTIONS
//
Entry*
Allocator::allocateEntry()
{
	Entry* r = 0;

	// バッファを得る
	EntryBuffer* buf = popEntryBuffer();
	
	try
	{
		// メモリを確保する
		
		r = buf->allocate();
	}
	catch (...)
	{
		pushEntryBuffer(buf);
		_SYDNEY_RETHROW;
	}
	pushEntryBuffer(buf);

	r->m_iDimensionSize = m_iDimensionSize;

	return r;
}

//
//	FUNCTION public
//	KdTree::Allocator::allocateNode -- ノード用のメモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Node*
//		確保したノード
//
//	EXCEPTIONS
//
Node*
Allocator::allocateNode()
{
	Node* r = 0;

	// バッファを得る
	NodeBuffer* buf = popNodeBuffer();
	
	try
	{
		// メモリを確保する
		
		r = buf->allocate();
	}
	catch (...)
	{
		pushNodeBuffer(buf);
		_SYDNEY_RETHROW;
	}
	pushNodeBuffer(buf);

	return r;
}

//
//	FUNCTION public
//	KdTree::Allocator::clearEntry -- エントリをすべて解放する
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
Allocator::clearEntry()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	while (m_cEntryBufferList.getSize())
	{
		EntryBuffer* buf = &(m_cEntryBufferList.getFront());
		m_cEntryBufferList.popFront();
		delete buf;
	}
}

//
//	FUNCTION public
//	KdTree::Allocator::clearNode -- すべてのノードを解放する
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
Allocator::clearNode()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	while (m_cNodeBufferList.getSize())
	{
		NodeBuffer* buf = &(m_cNodeBufferList.getFront());
		m_cNodeBufferList.popFront();
		delete buf;
	}
}

//
//	FUNCTION public
//	KdTree::Allocator::getSize -- メモリ容量を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		確保したメモリの容量
//
//	EXCEPTIONS
//
ModUInt64
Allocator::getSize()
{
	ModUInt64 s = 0;
	
	Os::AutoCriticalSection cAuto(m_cLatch);

	Common::DoubleLinkedList<EntryBuffer>::Iterator
		i = m_cEntryBufferList.begin();
	for (; i != m_cEntryBufferList.end(); ++i)
	{
		s += (*i).getSize();
	}

	Common::DoubleLinkedList<NodeBuffer>::Iterator
		j = m_cNodeBufferList.begin();
	for (; j != m_cNodeBufferList.end(); ++j)
	{
		s += (*j).getSize();
	}

	return s;
}

//
//	FUNCTION private
//	KdTree::Allocator::popEntryBuffer -- エントリ用のバッファを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Allocator::EntryBuffer*
//		エントリ用のバッファ
//
//	EXCEPTIONS
//
Allocator::EntryBuffer*
Allocator::popEntryBuffer()
{
	EntryBuffer* pBuffer = 0;
	
	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		if (m_cEntryBufferList.getSize())
		{
			pBuffer = &(m_cEntryBufferList.getFront());
			m_cEntryBufferList.popFront();
		}
	}

	if (pBuffer == 0)
	{
		pBuffer = new EntryBuffer(m_iDimensionSize);
	}

	return pBuffer;
}

//
//	FUNCTION public
//	KdTree::Allocator::pushEntryBuffer -- エントリ用のバッファを戻す
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Allocator::EntryBuffer* pEntryBuffer_
//		エントリ用のバッファ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Allocator::pushEntryBuffer(Allocator::EntryBuffer* pEntryBuffer_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	m_cEntryBufferList.pushBack(*pEntryBuffer_);
}

//
//	FUNCTION private
//	KdTree::Allocator::popNodeBuffer -- ノード用のバッファを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Allocator::NodeBuffer*
//		ノード用のバッファ
//
//	EXCEPTIONS
//
Allocator::NodeBuffer*
Allocator::popNodeBuffer()
{
	NodeBuffer* pBuffer = 0;
	
	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		if (m_cNodeBufferList.getSize())
		{
			pBuffer = &(m_cNodeBufferList.getFront());
			m_cNodeBufferList.popFront();
		}
	}

	if (pBuffer == 0)
	{
		pBuffer = new NodeBuffer();
	}

	return pBuffer;
}

//
//	FUNCTION public
//	KdTree::Allocator::pushNodeBuffer -- ノード用のバッファを戻す
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Allocator::NodeBuffer* pNodeBuffer_
//		ノード用のバッファ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Allocator::pushNodeBuffer(Allocator::NodeBuffer* pNodeBuffer_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	m_cNodeBufferList.pushBack(*pNodeBuffer_);
}

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
