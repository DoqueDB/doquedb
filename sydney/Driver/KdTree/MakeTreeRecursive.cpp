// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MakeTreeRecursive.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/MakeTreeRecursive.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::MakeTreeRecursive::MakeTreeRecursive
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Allocator& cAllocator_
//		アロケータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
MakeTreeRecursive::MakeTreeRecursive(Allocator& cAllocator_)
	: m_cAllocator(cAllocator_), m_iNext(0)
{
}

//
//	FUNCTION public
//	KdTree::MakeTreeRecursive::~MakeTreeRecursive
//		-- デストラクタ
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
MakeTreeRecursive::~MakeTreeRecursive()
{
}

//
//	FUNCTION public
//	KdTree::MakeTreeRecursive::parallel -- 並列実行
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
MakeTreeRecursive::parallel()
{
	Common::LargeVector<Entry*>::Iterator b;
	Common::LargeVector<Entry*>::Iterator e;
	Node** v;

	while (getEntry(b, e, v) == true)
	{
		// 処理対象がある限り実行する

		Node::makeTree(*v, b, e, m_cAllocator);
	}
}

//
//	FUNCTION private
//	KdTree::MakeTreeRecursive::getEntry
//		-- このスレッドで処理するエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<Entry*>::Iterator& b_
//		処理するエントリ配列の先頭
//	Common::LargeVector<Entry*>::Iterator& e_
//		処理するエントリ配列の終端
//	Node**& n_
//		作成したノードへのポインタ
//
//	RETURN
//	bool
//		処理するエントリがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MakeTreeRecursive::getEntry(Common::LargeVector<Entry*>::Iterator& b_,
							Common::LargeVector<Entry*>::Iterator& e_,
							Node**& n_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_iNext >= getNodeSize())
		return false;

	b_ = m_beginIte[m_iNext];
	e_ = m_endIte[m_iNext];
	n_ = m_vecpNode[m_iNext];

	++m_iNext;

	return true;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
