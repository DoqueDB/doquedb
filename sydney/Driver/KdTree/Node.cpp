// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Node.cpp --
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
#include "KdTree/Node.h"

#include "KdTree/Allocator.h"
#include "KdTree/Archiver.h"
#include "KdTree/Entry.h"

#include "Os/Limits.h"

#include "Exception/BadArgument.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// 距離比較クラス
	class _Less
	{
	public:
		ModBoolean operator()(const ModPair<double, const Entry*>& s1_,
							  const ModPair<double, const Entry*>& s2_)
		{
			return (s1_.first < s2_.first) ? ModTrue : ModFalse;
		}
	};
}

//
//	FUNCTION public
//	KdTree::Node::Status::isExpunge -- 削除されているか否か
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* pEntry_
//		検査対象のエントリ
//
//	RETURN
//	bool
//		削除されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Node::Status::isExpunge(const Entry* pEntry_)
{
	if (pEntry_->isExpunge() ||
		(m_pExpunge && m_pExpunge->test(pEntry_->getID()) == true))
		
		// 削除されている
		
		return true;
	
	return false;
}

//
//	FUNCTION public
//	KdTree::Node::Status::pushBack
//		-- 距離を求めたエントリを追加する
//		   内部では、dsquare_の小さいものからm_iLimit個保持する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* pEntry_
//		距離を求めたエントリ
//	double dsquare_
//		求めた距離
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Node::Status::pushBack(const Entry* pEntry_, double dsquare_)
{
	if (m_uiLimit == 0) return;

	// 挿入位置を求める
	ModPair<double, const Entry*> v(dsquare_, pEntry_);
	ModVector<ModPair<double, const Entry*> >::Iterator i
		= ModUpperBound(m_vecEntry.begin(), m_vecEntry.end(),
						v, _Less());

	// 挿入する
	m_vecEntry.insert(i, v);

	// 上限以上だったら、最後のエントリを削除する
	if (m_vecEntry.getSize() > m_uiLimit)
	{
		m_vecEntry.popBack();
	}
}

//
//	FUNCTION public
//	KdTree::Node::Node -- コンストラクタ
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
Node::Node()
	: m_iDimension(-1), m_pValue(0), m_pRight(0), m_pLeft(0)
{
}

//
//	FUNCTION public
//	KdTree::Node::~Node -- デストラクタ
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
Node::~Node()
{
}

//
//	FUNCTION public static
//	KdTree::Node::makeTree -- KdTreeを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<Entry*>::Iterator b_
//		処理する要素の先頭
//	Common::LargeVector<Entry*>::Iterator e_
//		処理する要素の終端
//	ALlocator& cAllocator_
//		アロケータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Node::makeTree(Node*& pNode_,
			   Common::LargeVector<Entry*>::Iterator b_,
			   Common::LargeVector<Entry*>::Iterator e_,
			   Allocator& cAllocator_)
{
	int count = e_ - b_;
	if (count == 0)
	{
		pNode_ = 0;
		return;
	}

	// ノードを得る
	pNode_ = cAllocator_.allocateNode();

	if (count == 1)
	{
		// 1つだったらノードの値に設定して終わり
		pNode_->m_pValue = *b_;
		return;
	}

	//
	// 二分木の分岐条件は、このノード以下の集合で
	// 一番分散の大きな次元のデータの中央値とする
	// 中央値を持つエントリをこのノードのエントリとし、
	// 中央値より小さい値のエントリは右ノード、
	// 中央値より大きな値のエントリは左ノードにする
	//

	// 一番分散の大きな次元を求める
	pNode_->m_iDimension = getMaxVarianceDimension(b_, e_);

	// その次元でソートする
	ModSort(b_, e_, Entry::Less(pNode_->m_iDimension));

	// 中央値で分ける
	Common::LargeVector<Entry*>::Iterator m = b_ + (count / 2);
	pNode_->m_pValue = *m;
	makeTree(pNode_->m_pRight, b_, m, cAllocator_);
	makeTree(pNode_->m_pLeft, m + 1, e_, cAllocator_);
}

//
//	FUNCTION public
//	KdTree::Node::nnsearch -- 最近傍検索
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* condition_
//		検索条件
//
//	RETURN
//	KdTree::Entry*
//		本ノードより下位で一番近いエントリ
//
//	EXCEPTIONS
//
const Entry*
Node::nnsearch(const Entry* condition_,
			   double& dsquare_,
			   Status& cStatus_) const
{
	// 最近傍
	const Entry* entry = 0;

	switch (cStatus_.getTraceType())
	{
	case TraceType::Normal:
		{
			// 通常のKD-Treeの探索を行う
			// 検索回数上限に達してしまうと、最近傍は得られない

			entry = normalSearch(condition_, dsquare_, cStatus_);
		}
		break;
		
	case TraceType::RicohVisualSearch:
		{
			// 最近傍の距離
			dsquare_ = Os::Limits<double>::getMax();
			// 探索対象のノードを格納する配列
			Common::LargeVector<const Node*> queue;
			queue.reserve(cStatus_.m_iMaxCalcDistanceCount / 2 * 3);

			// 最近傍検索を行う
			queue.pushBack(this);
			ModSize i = 0;
			while (cStatus_.isContinue() && i < queue.getSize())
			{
				// キューの先頭のノードを得る
				const Node* pNode = queue[i];

				// 深さ優先探索を行う
				const Entry* e = pNode->dfsearch(condition_,
												 dsquare_,
												 cStatus_,
												 queue);

				if (e != 0)
				{
					// より近いものが見つかった
					entry = e;
				}

				++i;
			}
		}
		break;
		
	case TraceType::Serial:
		{
			// バカサーチするので、必ず最近傍が得られる

			entry = serialSearch(condition_, dsquare_, cStatus_);
		}
		break;

	default:
		// 想定外
		
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return entry;
}

//
//	FUNCTION public
//	KdTree::Node::dump -- ファイルに書き出す
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Archiver& archiver_
//		書き出すアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Node::dump(Archiver& archiver_)
{
	archiver_.write(m_iDimension);
	archiver_.write(m_pValue->operator const char*(), m_pValue->getSize());
	
	if (m_pRight)
	{
		archiver_.write(1);
		m_pRight->dump(archiver_);
	}
	else
	{
		archiver_.write(0);
	}
	
	if (m_pLeft)
	{
		archiver_.write(1);
		m_pLeft->dump(archiver_);
	}
	else
	{
		archiver_.write(0);
	}
}

//
//	FUNCTION public
//	KdTree::Node::load -- ファイルから読み込む
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Archiver& archiver_
//		読みだすアーカイバー
//	KdTree::Allocator& allocator_
//		アロケータ
//
//	RETURN
//	bool
//		ノードが存在している場合はtrue、それ以外の場合はfalseを返す
//
//	EXCEPTIONS
//
bool
Node::load(Archiver& archiver_, Allocator& allocator_)
{
	//【注意】	次元が -1 の場合、ノードが存在していないと判断する
	
	archiver_.read(m_iDimension);
	if (m_iDimension == -1)
		return false;
	
	m_pValue = allocator_.allocateEntry();
	archiver_.read(m_pValue->operator char*(), m_pValue->getSize());

	int n = 0;
	
	archiver_.read(n);
	if (n == 1)
	{
		m_pRight = allocator_.allocateNode();
		m_pRight->load(archiver_, allocator_);
	}

	archiver_.read(n);
	if (n == 1)
	{
		m_pLeft = allocator_.allocateNode();
		m_pLeft->load(archiver_, allocator_);
	}

	return true;
}

//
//	FUNCTION public
//	KdTree::Node::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Entry* pEntry_
//		挿入するエントリ
//	KdTree::Allocator& cAllocator_
//		アロケータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Node::insert(Entry* pEntry_, Allocator& cAllocator_)
{
	if (m_pRight == 0 && m_pLeft == 0)
	{
		// リーフなので、分岐で利用する次元を求めていない
		// すべての次元のデータを確認して、一番差の大きな次元とする

		m_iDimension = m_pValue->getMaxDifferenceDimension(pEntry_);
	}

	if (pEntry_->getValue(m_iDimension) < m_pValue->getValue(m_iDimension))
	{
		// 右ノード
		
		if (m_pRight)
		{
			m_pRight->insert(pEntry_, cAllocator_);
		}
		else
		{
			m_pRight = cAllocator_.allocateNode();
			m_pRight->m_pValue = pEntry_;
		}
	}
	else
	{
		// 左ノード
		
		if (m_pLeft)
		{
			m_pLeft->insert(pEntry_, cAllocator_);
		}
		else
		{
			m_pLeft = cAllocator_.allocateNode();
			m_pLeft->m_pValue = pEntry_;
		}
	}
}

//
//	FUNCTION private
//	KdTree::Node::normalSearch -- 通常のKD-Treeの最近傍検索
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* condition_
//		検索条件
//
//	RETURN
//	KdTree::Entry*
//		本ノードより下位で一番近いエントリ
//
//	EXCEPTIONS
//
const Entry*
Node::normalSearch(const Entry* condition_,
				   double& dsquare_,
				   Status& cStatus_) const
{
	if (m_pRight == 0 && m_pLeft == 0)
	{
		if (cStatus_.isExpunge(m_pValue) == false &&
			cStatus_.addCalcDistanceCount())
		{
			// 削除されていなく、計算数の上限にも達していない
			
			dsquare_ = condition_->calcDistance(m_pValue);
			cStatus_.pushBack(m_pValue, dsquare_);
		}
		else
		{
			dsquare_ = Os::Limits<double>::getMax();
		}
		return m_pValue;
	}
	
	const Entry* leaf = 0;

	//
	// 分岐に使用した次元の値を得る
	// ノードのエントリとは、分岐に使用した次元の中央値をもつエントリである
	//
	
    double cv = condition_->getValue(m_iDimension);
	double nv = m_pValue->getValue(m_iDimension);

	//
	// 条件とこのノードのエントリとの分岐した次元の値を比較し、
	// 小さければ右ノード、大きければ左ノードをたどる
	//
	
	if ((cv < nv && m_pRight) || m_pLeft == 0)
	{
		// 右ノードを検索する
		
		leaf = m_pRight->normalSearch(condition_, dsquare_, cStatus_);
	}
	else
	{
		// 左ノードを検索する

		leaf = m_pLeft->normalSearch(condition_, dsquare_, cStatus_);
	}

	//
	// 条件を中心とし得られた最近傍のものとの距離を半径とする円に、
	// このノードのエントリが入っているかを確認し、
	// 入っている場合には、反対側のノードも検索する
	//
		
	if (cStatus_.isContinue() && m_pRight && m_pLeft &&
		dsquare_ > (cv - nv) * (cv - nv))
	{
		double dsquare1;
		const Entry* leaf1 = (cv < nv) ?
			m_pLeft->normalSearch(condition_, dsquare1, cStatus_) :
			m_pRight->normalSearch(condition_, dsquare1, cStatus_);

		if (dsquare1 < dsquare_)
		{
			// 反対側のノードの方に近いものがあった
			
			dsquare_ = dsquare1;
			leaf = leaf1;
		}
	}

	// 得られたエントリと、このノードのエントリと近い方を返す
	
	if (cStatus_.isExpunge(m_pValue) == false)
	{
		// ノードなので、上限数に達しているかの判定は行わない
		cStatus_.addCalcDistanceCount();
		
		double dsquare2 = condition_->calcDistance(m_pValue);
		cStatus_.pushBack(m_pValue, dsquare2);
		if (dsquare2 < dsquare_)
		{
			dsquare_ = dsquare2;
			leaf = m_pValue;
		}
	}
	
	return leaf;
}

//
//	FUNCTION private
//	KdTree::Node::dfsearch -- 深さ優先探索で最近傍検索を行う
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
const Entry*
Node::dfsearch(const Entry* condition_,
			   double& dsquare_,
			   Status& cStatus_,
			   Common::LargeVector<const Node*>& queue_) const
{
	// RVSのripeと同じアルゴリズム

	// リーフまで再帰的にたどり、
	// maxDistance 以内に収まる可能性があるノードが反対側にあったら、
	// 探索するためのノードとしてキューに格納する

	if (m_pRight == 0 && m_pLeft == 0)
	{
		// 削除されていたり、計算数の上限に達していたりしたら、
		// 距離は計算しない

		double d = Os::Limits<double>::getMax();

		if (cStatus_.isExpunge(m_pValue) == false &&
			cStatus_.addCalcDistanceCount() == true)
		{
			d = condition_->calcDistance(m_pValue);
			cStatus_.pushBack(m_pValue, d);
		}

		if (d < dsquare_)
		{
			// これまでより近い
			dsquare_ = d;
			return m_pValue;
		}
		else
		{
			// これまでより遠い
			return 0;
		}
	}
	
	const Entry* leaf = 0;

	//
	// 分岐に使用した次元の値を得る
	// ノードのエントリとは、分岐に使用した次元の中央値をもつエントリである
	//
	
    double cv = condition_->getValue(m_iDimension);
	double nv = m_pValue->getValue(m_iDimension);

	//
	// 条件とこのノードのエントリとの分岐した次元の値を比較し、
	// 小さければ右ノード、大きければ左ノードをたどる
	//
	
	if ((cv < nv && m_pRight) || m_pLeft == 0)
	{
		// 左ノードが探索範囲内なら、キューに追加する
		
		if (m_pLeft && (cv - nv) * (cv - nv) < dsquare_)
		{
			queue_.pushBack(m_pLeft);
		}
		
		// 右ノードを検索する
		
		leaf = m_pRight->dfsearch(condition_, dsquare_, cStatus_, queue_);
	}
	else
	{
		// 右ノードが探索範囲内なら、キューに追加する
		
		if (m_pRight && (cv - nv) * (cv - nv) < dsquare_)
		{
			queue_.pushBack(m_pRight);
		}
		
		// 左ノードを検索する

		leaf = m_pLeft->dfsearch(condition_, dsquare_, cStatus_, queue_);
	}

	// 得られたエントリと、このノードのエントリと近い方を返す
	
	if (cStatus_.isExpunge(m_pValue) == false)
	{
		// 距離計算回数をインクリメントするが、
		// ノードなので、超えていても距離計算する
		
		cStatus_.addCalcDistanceCount();
		
		double dsquare2 = condition_->calcDistance(m_pValue);
		cStatus_.pushBack(m_pValue, dsquare2);
		if (dsquare2 < dsquare_)
		{
			dsquare_ = dsquare2;
			leaf = m_pValue;
		}
	}
	
	return leaf;
}	

//
//	FUNCTION private
//	KdTree::Node::serialSearch -- 逐次検索で最近傍検索を行う
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry* condition_
//		検索条件
//
//	RETURN
//	KdTree::Entry*
//		本ノードより下位で一番近いエントリ
//
//	EXCEPTIONS
//
const Entry*
Node::serialSearch(const Entry* condition_,
				   double& dsquare_,
				   Status& cStatus_) const
{
	//【注意】	主にデバッグや精度測定のための正解データ作成時に利用される
	//			逐次検索は時間がかかるが、複数の条件が与えられた場合、
	//			マルチスレッドで並列処理されるので、
	//			1つの条件を処理する平均時間は、1,900万件で約250ms程度である
	
	if (m_pRight == 0 && m_pLeft == 0)
	{
		if (cStatus_.isExpunge(m_pValue) == false)
		{
			dsquare_ = condition_->calcDistance(m_pValue);
			cStatus_.pushBack(m_pValue, dsquare_);
		}
		else
		{
			dsquare_ = Os::Limits<double>::getMax();
		}
		return m_pValue;
	}
	
	const Entry* leaf = 0;

	if (m_pRight)
	{
		// 右ノードを検索する

		leaf = m_pRight->serialSearch(condition_, dsquare_, cStatus_);
	}

	if (m_pLeft)
	{
		// 左ノードを検索する

		double dsquare1;
		const Entry* leaf1
			= m_pLeft->serialSearch(condition_, dsquare1, cStatus_);

		if (m_pRight == 0 || dsquare1 < dsquare_)
		{
			dsquare_ = dsquare1;
			leaf = leaf1;
		}
	}
	
	// 得られたエントリと、このノードのエントリと近い方を返す
	
	if (cStatus_.isExpunge(m_pValue) == false)
	{
		double dsquare2 = condition_->calcDistance(m_pValue);
		cStatus_.pushBack(m_pValue, dsquare2);
		if (dsquare2 < dsquare_)
		{
			dsquare_ = dsquare2;
			leaf = m_pValue;
		}
	}
	
	return leaf;
}

//
//	FUNCTION private static
//	KdTree::Node::getMaxVarianceDimension -- 一番分散の大きな次元を求める
//
//	NOTES
//
//	ARGUMENTS
//	Common::LargeVector<Entry*>::Iterator& b_
//		始点
//	Common::LargeVector<Entry*>::Iterator& e_
//		終端
//
//	RETURN
//	int
//		一番分散の大きな次元
//
//	EXCEPTIONS
//
int
Node::getMaxVarianceDimension(Common::LargeVector<Entry*>::Iterator& b_,
							  Common::LargeVector<Entry*>::Iterator& e_)
{
	// 次元数
	int dimSize = (*b_)->getDimensionSize();

	// 次元分のデータ領域を確保する
	ModVector<ModPair<double, double> > variance;
	variance.assign(dimSize, ModPair<double, double>(0.0, 0.0));

	// すべての次元の二乗の和と和を求める
	Common::LargeVector<Entry*>::Iterator i = b_;
	for (; i != e_; ++i)
	{
		int d = 0;
		ModVector<ModPair<double, double> >::Iterator j = variance.begin();
		for (; j != variance.end(); ++j, ++d)
		{
			float v = (*i)->getValue(d);

			// 二乗の和
			(*j).first += (v * v);
			// 和
			(*j).second += v;
		}
	}

	// 分散を求める
	double m = 0.0;
	int maxDim = 0;
	int count = e_ - b_;
	
	ModVector<ModPair<double, double> >::Iterator j = variance.begin();
	for (int d = 0; j != variance.end(); ++j, ++d)
	{
		// 分散は、(平均値 - 値) ^ 2 の平均であるので、
		// 二乗の平均値 - 平均値の二乗となる

		double a1 = (*j).first / count;
		double a2 = (*j).second / count;

		double s = a1 - (a2 * a2);

		if (s > m)
		{
			maxDim = d;
			m = s;
		}
	}

	return maxDim;
}

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
