// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeIndex.cpp --
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
#include "KdTree/KdTreeIndex.h"

#include "KdTree/Archiver.h"
#include "KdTree/CalcVariance.h"
#include "KdTree/DataFile.h"
#include "KdTree/IndexFile.h"
#include "KdTree/LoadEntry.h"
#include "KdTree/MakeTreeRecursive.h"
#include "KdTree/SortEntry.h"

#include "Common/Message.h"
#include "Common/VectorMap.h"

#include "Os/Memory.h"

#include "Exception/Cancel.h"

#include "ModAutoPointer.h"
#include "ModTypes.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// ダミー
	class _DirectionForDummy : public KdTreeIndex::Direction
	{
	public:
		bool isAbort() const { return false; }
	};

}

//
//	FUNCTION public
//	KdTree::KdTreeIndex::KdTreeIndex -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iDimension_
//		次元数
//	const Trans::TimeStamp& cTimeStamp_
//		タイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
KdTreeIndex::KdTreeIndex(int iDimension_,
						 const Trans::TimeStamp& cTimeStamp_)
	: m_pNode(0), m_cAllocator(iDimension_), m_cTimeStamp(cTimeStamp_),
	  m_pPrev(0), m_pNext(0), m_eStatus(Status::Copy)
{
}

//
//	FUNCTION public
//	KdTree::KdTreeIndex::~KdTreeIndex -- デストラクタ
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
KdTreeIndex::~KdTreeIndex()
{
}

//
//	FUNCTION public
//	KdTree::KdTreeIndex::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* pEntry_
//		挿入するエントリ。コピーして挿入される
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::insert(const Entry* pEntry_)
{
	// コピーする
	Entry* p = m_cAllocator.allocateEntry();
	Os::Memory::copy(p, pEntry_, pEntry_->getSize());

	// 配列に格納する
	m_vecEntry.insert(p->getID(), p);

	// 木に挿入する
	if (m_pNode == 0)
	{
		// 初めての挿入なので、ノードを確保する
		m_pNode = m_cAllocator.allocateNode();
		m_pNode->m_pValue = p;
	}
	else
	{
		// ルートノードに挿入する
		m_pNode->insert(p, m_cAllocator);
	}
}

//
//	FUNCTION public
//	KdTree::KdTreeIndex::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	bool
//		削除対象が存在した場合はtrue、存在しなかった場合はfalse
//
//	EXCEPTIONS
//
bool
KdTreeIndex::expunge(ModUInt32 uiRowID_)
{
	bool result = false;
	
	// 配列をROWIDで検索する
	
	Common::VectorMap<ModUInt32, Entry*, ModLess<ModUInt32> >::Iterator i
		= m_vecEntry.find(uiRowID_);

	if (i != m_vecEntry.end())
	{
		// ヒットしたので、削除フラグを立てる
		
		(*i).second->expunge();

		// 配列からは削除する

		m_vecEntry.erase(i);

		result = true;
	}

	return result;
}

//
//	FUNCTION public
//	KdTree::KdTreeIndex::create -- 索引を作る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::DataFile& cDataFile_
//		データファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::create(DataFile& cDataFile_)
{
	//【注意】	このメソッドは差分ファイル用の索引を作るときにのみ呼び出される
	
	create(cDataFile_, _DirectionForDummy(), true);
}
	
//
//	FUNCTION public
//	KdTree::KdTreeIndex::create -- 索引を作る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::DataFile& cDataFile_
//		データファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::create(DataFile& cDataFile_,
					const Direction& cDirection_,
					bool isSmallIndex_)
{
	// 空にする
	
	m_pNode = 0;
	m_cAllocator.clearEntry();
	m_cAllocator.clearNode();

	// 新しく読み込む
	
	if (cDataFile_.getCount())
	{
		// 索引づくりは2段階で行う
		// データ数が多いうちは、ステップごとに OpenMP を利用する
		// データ数が少なくなってきたら、ノードごとに OpenMP を利用する

		// 全データをロードする

		LoadEntry cLoadEntry(cDataFile_, m_cAllocator);
		cLoadEntry.run();

		if (cDirection_.isAbort())
			_SYDNEY_THROW0(Exception::Cancel);

		if (isSmallIndex_)
		{
			// 差分ファイル用の索引なので、
			// 登録エントリを配列に格納する

			Common::LargeVector<Entry*>& vecEntry = cLoadEntry.getEntry();
			m_vecEntry.reserve(vecEntry.getSize());
			Common::LargeVector<Entry*>::Iterator i	= vecEntry.begin();
			for (; i != vecEntry.end(); ++i)
			{
				m_vecEntry.insert((*i)->getID(), *i);
			}
		}

		// 並列実行数を得る(実際の数を二倍にする)
		int iParallelCount = cLoadEntry.getParallelCount() << 1;

		// データ数が少なくなってきた場合に OpenMP で並列的に木を作るクラス

		MakeTreeRecursive cMakeTree(m_cAllocator);

		// データ数が多いうちは再帰的に処理していく

		makeTree(m_pNode,
				 cLoadEntry.getEntry().begin(),
				 cLoadEntry.getEntry().end(),
				 cMakeTree,
				 iParallelCount,
				 cDirection_);

		// データが少なくなってきたので、並列処理する
	
		cMakeTree.run();
	}

	// ステータスを変更する
	m_eStatus = Status::Fix;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndex::load -- 索引ダンプファイルから索引の中身をロードする
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::IndexFile& cIndexFile_
//		索引ダンプファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::load(IndexFile& cIndexFile_)
{
	m_pNode = m_cAllocator.allocateNode();
	ModAutoPointer<Archiver> archiver = cIndexFile_.getArchiver(false);
	if (m_pNode->load(*archiver, m_cAllocator) == false)
	{
		// ノードが存在していない

		m_pNode = 0;
	}
	
	// ステータスを変更する
	m_eStatus = Status::Fix;
}

//
//	FUNCTION private
//	KdTree::KdTreeIndex::dump -- 索引の中身をダンプする
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::IndexFile& cIndexFile_
//		索引ダンプファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::dump(IndexFile& cIndexFile_)
{
	ModAutoPointer<Archiver> archiver = cIndexFile_.getArchiver(true);
	
	if (m_pNode)
	{
		m_pNode->dump(*archiver);
	}
	else
	{
		// ノードがないので -1 を記録する
		//
		// Node::load で最初に読み込むのは m_iDimension である。
		// m_iDimenstion が -1 の場合、load は false を返し、
		// それは、ノードがないことを示している。
		//
		//【注意】Node::dump および Node::load の実装に依存している

		int d = -1;
		archiver->write(d);
	}
}

//
//	FUNCTION private
//	KdTree::KdTreeIndex::makeTree -- 再帰的にKD-Treeを作成する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Node*& pNode_
//		作成したノード
//	Common::LargeVector<KdTree::Entry*>::Iterator b_
//		先頭
//	Common::LargeVector<KdTree::Entry*>::Iterator e_
//		終端
//	KdTree::MakeTreeRecursive& cMakeTree_
//		KD-Tree作成クラス
//	int iParallelCount_
//		並列実行数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
KdTreeIndex::makeTree(Node*& pNode_,
					  Common::LargeVector<Entry*>::Iterator b_,
					  Common::LargeVector<Entry*>::Iterator e_,
					  MakeTreeRecursive& cMakeTree_,
					  int iParallelCount_,
					  const Direction& cDirection_)
{
	int count = e_ - b_;
	if (count == 0)
	{
		pNode_ = 0;
		return;
	}

	// ノードを得る
	pNode_ = m_cAllocator.allocateNode();

	if (count == 1)
	{
		// 1つだったらノードの値に設定して終わり
		pNode_->m_pValue = *b_;
		return;
	}

	// 一番大きな次元を求める
	CalcVariance cVariance(b_, e_);
	cVariance.run();
	pNode_->m_iDimension = cVariance.getMaxVarianceDimension();

	if (cDirection_.isAbort())
		_SYDNEY_THROW0(Exception::Cancel);

	// その次元でソートする
	SortEntry cSort(b_, e_, pNode_->m_iDimension);
	cSort.run();

	if (cDirection_.isAbort())
		_SYDNEY_THROW0(Exception::Cancel);

	// 中間値で分ける
	Common::LargeVector<Entry*>::Iterator m = b_ + (count / 2);
	pNode_->m_pValue = *m;

	iParallelCount_ >>= 1;
	if (iParallelCount_)
	{
		// 再帰呼び出しする
		makeTree(pNode_->m_pRight, b_, m, cMakeTree_, iParallelCount_,
				 cDirection_);
		makeTree(pNode_->m_pLeft, m + 1, e_, cMakeTree_, iParallelCount_,
				 cDirection_);
	}
	else
	{
		// ノードごとに処理する
		cMakeTree_.setEntry(&(pNode_->m_pRight), b_, m);
		cMakeTree_.setEntry(&(pNode_->m_pLeft), m + 1, e_);
	}
}

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
