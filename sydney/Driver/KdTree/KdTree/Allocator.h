// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Allocator.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_ALLOCATOR_H
#define __SYDNEY_KDTREE_ALLOCATOR_H

#include "KdTree/Module.h"

#include "Common/DoubleLinkedList.h"
#include "Common/LargeVector.h"

#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Entry;
class Node;

//
//	CLASS
//	KdTree::Allocator -- アロケータ
//
//	NOTES
//
class Allocator
{
public:
	// コンストラクタ
	Allocator(int iDimension_);
	// デストラクタ
	virtual ~Allocator();

	// エントリのメモリを確保する
	Entry* allocateEntry();
	// ノード用のメモリを確保する
	Node* allocateNode();

	// すべてのエントリを解放する
	void clearEntry();
	// すべてのノードを解放する
	void clearNode();

	// 次元数を得る
	int getDimension() const { return m_iDimensionSize; }

	// メモリ容量を得る
	ModUInt64 getSize();

private:
	//
	//	CLASS
	//	KdTree::Allocator::EntryBuffer -- バッファ
	//
	class EntryBuffer
	{
	public:
		EntryBuffer(int iDimension_);
		~EntryBuffer();

		// エントリのメモリを確保する
		Entry* allocate();

		// 確保したメモリサイズ
		ModUInt64 getSize() const;

		// メモリ管理用配列
		Common::LargeVector<char*> m_vecpBuffer;

		// エントリのサイズ
		int m_iEntrySize;

		// 現在の利用ポインタ
		char* m_pEnd;
		char* m_pCurrent;

		// リスト用の前後のリンク
		EntryBuffer* m_pPrev;
		EntryBuffer* m_pNext;
	};

	//
	//	CLASS
	//	KdTree::Allocator::NodeBuffer -- バッファ
	//
	class NodeBuffer
	{
	public:
		NodeBuffer();
		~NodeBuffer();

		// ノードのメモリを確保する
		Node* allocate();

		// 確保したメモリサイズ
		ModUInt64 getSize() const;

		// メモリ管理用配列
		Common::LargeVector<Node*> m_vecpBuffer;

		// 現在の利用ポインタ
		Node* m_pEnd;
		Node* m_pCurrent;

		// リスト用の前後のリンク
		NodeBuffer* m_pPrev;
		NodeBuffer* m_pNext;
	};

	// エントリ用のバッファを得る
	EntryBuffer* popEntryBuffer();
	// エントリ用のバッファを返す
	void pushEntryBuffer(EntryBuffer* pEntryBuffer_);

	// ノード用のバッファを得る
	NodeBuffer* popNodeBuffer();
	// ノード用のバッファを返す
	void pushNodeBuffer(NodeBuffer* pNodeBuffer_);

	// エントリ用のバッファ
	Common::DoubleLinkedList<EntryBuffer> m_cEntryBufferList;
	// ノード用のバッファ
	Common::DoubleLinkedList<NodeBuffer> m_cNodeBufferList;

	// 次元数
	int m_iDimensionSize;

	// 排他制御用
	Os::CriticalSection m_cLatch;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_ALLOCATOR_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
