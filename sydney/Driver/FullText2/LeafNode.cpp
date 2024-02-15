// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafNode.cpp -- 
// 
// Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/LeafNode.h"

#include "FullText2/DummyListIterator.h"
#include "FullText2/ListManager.h"
#include "FullText2/NormalLeafNode.h"
#include "FullText2/SimpleLeafNode.h"
#include "FullText2/ShortLeafNode.h"
#include "FullText2/ShortLeafNodeCompatible.h"

#include "Common/LargeVector.h"

#include "Os/Limits.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::LeafNode::LeafNode -- コンストラクタ
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
LeafNode::LeafNode()
{
}

//
//	FUNCTION public
//	FullText2::LeafNode::~LeafNode -- デストラクタ
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
LeafNode::~LeafNode()
{
}

//
//	FUNCTION public
//	FullText2::LeafNode::LeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::LeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LeafNode::LeafNode(const LeafNode& src_)
{
}

//
//	FUNCTION public static
//	FullText2::LeafNode::createNormalLeafNode
//		-- 通常のリーフノードを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const FullText2::LeafNode::LocationVector& cToken_
//		トークナイズした結果の配列
//
//	RETURN
//	FullText2::LeafNode*
//		NormalLeafNode か SimpleLeafNode へのポインタ
//
//	EXCEPTIONS
//
LeafNode*
LeafNode::createNormalLeafNode(ListManager& cManager_,
							   const LocationVector& cToken_)
{
	if (cToken_.getSize() == 0)
		_TRMEISTER_THROW0(Exception::NotSupported);

	if (cToken_.getSize() == 1)
	{
		// １つしか索引単位がないので、SimpleLeafNode

		ListIterator* ite = 0;

		// 転置リストを得る
		if (cManager_.reset((*cToken_.begin()).second,
							ListManager::AccessMode::Search) == true)
			ite = cManager_.getIterator();
		else
			ite = new DummyListIterator;	// 空のイテレータ

		return new SimpleLeafNode(ite);
	}

	LeafNode* ret = 0;
	
	if (cManager_.isNoLocation() == true)
	{
		// 位置情報がないので、すべての索引単位を採用
		// 重複があるかもしれないが、そんなにないと思うので、気にしない

		NormalLeafNode* node = new NormalLeafNode();
		ret = node;

		LocationVector::ConstIterator i = cToken_.begin();
		LocationVector::ConstIterator e = cToken_.end();
		for (; i < e; ++i)
		{
			ListIterator* ite = 0;
			
			if (cManager_.reset((*i).second,
								ListManager::AccessMode::Search) == true)
			{
				ite = cManager_.getIterator();
			}
			else
			{
				// 索引単位がなかったら、1件もヒットしないので、空でいい
				delete node;
				ret = new SimpleLeafNode(new DummyListIterator);
				break;
			}
				
			node->pushBack((*i).first, ite);
		}
	}
	else
	{
		// 複数の索引単位があるので、最適なパスを求める必要あり
		// 基本的な考え方は以下の通り
		//
		// 0. 同じ位置のものはないのが前提
		// 1. もっとも文書頻度が少ないものを求め、
		// 2. その前後でかつ重なっていないものと、３つは採用する
		// 3. その他は重ならないように採用

		// すべての文書頻度とリストイテレータを得る
		ModVector<ModPair<ModSize, ListIterator*> > vecIterator;
		vecIterator.reserve(cToken_.getSize());
		LocationVector::ConstIterator b = cToken_.begin();
		LocationVector::ConstIterator e = cToken_.end();
		LocationVector::ConstIterator i = b;
		int min = 0;
		ModSize minCount = Os::Limits<ModSize>::getMax();
		for (; i < e; ++i)
		{
			if (cManager_.reset((*i).second,
								ListManager::AccessMode::Search) == true)
			{
				ListIterator* ite = cManager_.getIterator();
				ModSize count  = ite->getEstimateCount();
				vecIterator.pushBack(
					ModPair<ModSize, ListIterator*>(count, ite));
				if (count < minCount)
				{
					// 一番文書頻度が少ないものを調べておく
					minCount = count;
					min = (i - b);
				}
			}
			else
			{
				// 索引単位がなかったら、1件もヒットしないので、空でいい
				ModVector<ModPair<ModSize, ListIterator*> >::
					Iterator j = vecIterator.begin();
				for (; j != vecIterator.end(); ++j)
					delete (*j).second;

				ret = new SimpleLeafNode(new DummyListIterator);
				break;
			}
		}

		if (ret == 0)
		{
			NormalLeafNode* node = new NormalLeafNode();
			ret = node;

			LocationVector::ConstIterator b = cToken_.begin();
			LocationVector::ConstIterator e = cToken_.end();
			LocationVector::ConstIterator i = b + min;
			ModVector<ModPair<ModSize, ListIterator*> >::
				Iterator j = vecIterator.begin() + min;
			
			// 一番少ないものを採用する
			ModSize len = (*j).second->getLength();
			ModSize minPosB = (*i).first;
			ModSize minPosE = minPosB + len;
			node->pushBack((*i).first, (*j).second);
			(*j).second = 0;

			// 前を確認し、重なっていないものを採用する
			while (i > b)
			{
				--i;
				--j;
				len = (*j).second->getLength();
				
				if (((*i).first + len) <= minPosB)
					break;
			}
			if (((*i).first + len) < minPosB)
			{
				// 離れちゃったので1つ戻す
				++i;
				++j;
			}
			if ((*j).second != 0)
			{
				minPosB = (*i).first;
				node->pushBack((*i).first, (*j).second);
				(*j).second = 0;
			}

			// 後ろを確認し、重なっていないものを採用する
			i = b + min;
			j = vecIterator.begin() + min;
			++i;
			++j;
			while (i < e)
			{
				if ((*i).first >= minPosE)
					break;
				++i;
				++j;
			}
			if (i == e || (*i).first > minPosE)
			{
				// 離れちゃったので1つ戻す
				--i;
				--j;
			}
			if ((*j).second != 0)
			{
				minPosE = (*i).first + (*j).second->getLength();
				node->pushBack((*i).first, (*j).second);
				(*j).second = 0;
			}

			// 残りは重ならないように先頭から追加する
			ModSize posE = 0;
			i = b;
			LocationVector::ConstIterator ii = i;
			++ii;	// i の次のエントリ
			j = vecIterator.begin();
			for (; ii != e; ++i, ++ii, ++j)
			{
				if (posE >= minPosB)
				{
					// 文書頻度の低いものの前後は採用済みなので、
					// その次から調べればよい
					
					posE = minPosE;
					minPosB = Os::Limits<ModSize>::getMax();
				}
					
				if ((*j).second == 0)
					// すでに採用されているので次へ
					continue;
				
				if ((*i).first < posE && (*ii).first <= posE)
					// これは重なっているが、
					// 次のが離れてしまわないかチェックする
					continue;

				posE = (*i).first + (*j).second->getLength();
				node->pushBack((*i).first, (*j).second);
				(*j).second = 0;
			}

			// 最後のものは重なっても採用する
			if ((*j).second != 0)
			{
				node->pushBack((*i).first, (*j).second);
				(*j).second = 0;
			}

			// 採用されなかったListIteratorを削除する
			j = vecIterator.begin();
			ModVector<ModPair<ModSize, ListIterator*> >::
				Iterator je = vecIterator.end();
			for (; j < je; ++j)
			{
				if ((*j).second != 0) delete (*j).second;
			}
		}
	}

	return ret;
}

//
//	FUNCTION public
//	FullText2::LeafNode::createShortLeafNode
//		-- ショートワードのリーフノードを作成する
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListManager& cManager_
//		リストマネージャー
//	const ModUnicodeString& cTerm_
//		検索語
//	int iLength_
//		検索語長
//	ModSize uiMinLength_
//		その文字種の最小長。単語単位索引の場合は 0 を指定すること。
//	bool isCompatible_
//		ShortLeafNode か ShortLeafNodeCompatible か
//
//	RETURN
//	FullText2::LeafNode*
//		ShortLeafNode か ShortLeafNodeCompatible へのポインタ
//
//	EXCEPTIONS
//
LeafNode*
LeafNode::createShortLeafNode(ListManager& cManager_,
							  const ModUnicodeString& cTerm_,
							  int iLength_,
							  ModSize uiMinLength_,
							  bool isCompatible_)
{
	if (cTerm_.getLength() == 0)
		_TRMEISTER_THROW0(Exception::NotSupported);
	
	// ショートワード用のLeafNode
	ShortLeafNode* node = 0;
	if (isCompatible_ == true)
		node = new ShortLeafNodeCompatible(iLength_);
	else
		node = new ShortLeafNode(iLength_);

	// ショートワードなので、下限検索して
	// 先頭文字列が cTerm_ と同じ場合はすべて加える

	bool enter = false;
	bool result = cManager_.reset(cTerm_,
								  ListManager::AccessMode::LowerBound);
	while (result)
	{
		const ModUnicodeString& key = cManager_.getKey();
		
		if (cTerm_.compare(key, cTerm_.getLength()) != 0)
			// 索引単位の先頭の文字列が cTerm_ とは違うので終了
			break;

		if (uiMinLength_ != 0 && key.getLength() > uiMinLength_)
		{
			// 最小の長さではないので、これは不要。次へ
			result = cManager_.next();
			continue;
		}
		
		// 追加する
		node->pushBack(cManager_.getIterator());
		enter = true;

		// 次へ
		result = cManager_.next();
	}

	if (enter == false)
	{
		// 索引単位が見つからなかった
		node->pushBack(new DummyListIterator);
	}

	return node;
}

//
//	FUNCTION public
//	FullText2::LeafNode::createEmptyLeafNode
//		-- 空のリーフノードを作成する
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LeafNode*
//		SimpleLeafNode へのポインタ
//
//	EXCEPTIONS
//
LeafNode*
LeafNode::createEmptyLeafNode()
{
	return new SimpleLeafNode(new DummyListIterator);
}

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
