// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinUnorderedLeafNode.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "FullText2/WithinUnorderedLeafNode.h"

#include "FullText2/SearchInformation.h"
#include "FullText2/WithinUnorderedLeafLocationListIterator.h"

#include "Os/Limits.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	// 文書頻度順にソートするクラス
	class _Less
	{
	public:
		ModBoolean operator() (const ModPair<ModSize, LeafNode*>& a,
							   const ModPair<ModSize, LeafNode*>& b)
			{
				return (a.first < b.first) ? ModTrue : ModFalse;
			}
	};
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::WithinUnorderedLeafNode
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLower_
//		下限
//	ModSize uiUpper_
//		上限
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WithinUnorderedLeafNode::WithinUnorderedLeafNode(ModSize uiLower_,
												 ModSize uiUpper_)
	: ArrayLeafNode(),
	  m_uiCurrentID(0),
	  m_uiLower(uiLower_), m_uiUpper(uiUpper_), m_uiTermFrequency(0),
	  m_bInitialized(false)
{
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::~WithinUnorderedLeafNode -- デストラクタ
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
WithinUnorderedLeafNode::~WithinUnorderedLeafNode()
{
	m_pLocation = 0;
	// フリーリストを解放する
	clearFree();
	
	NodeVector::Iterator i = m_cVector.begin();
	for (; i < m_cVector.end(); ++i)
		delete (*i).second;
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::WithinUnorderedLeafNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::WithinUnorderedLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WithinUnorderedLeafNode::
WithinUnorderedLeafNode(const WithinUnorderedLeafNode& src_)
	: ArrayLeafNode(src_), m_uiCurrentID(0),
	  m_uiLower(src_.m_uiLower), m_uiUpper(src_.m_uiUpper),
	  m_uiTermFrequency(0), m_bInitialized(false)
{
	m_cVector.reserve(src_.m_cVector.getSize());
	NodeVector::ConstIterator i = src_.m_cVector.begin();
	for (; i < src_.m_cVector.end(); ++i)
		pushBack((*i).second->copy());
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::pushBack -- トークンを追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafNode* pNode_
//		ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
WithinUnorderedLeafNode::pushBack(LeafNode* pNode_)
{
	// 与えられたノードは、本クラスデストラクト時に破棄される
	
	m_cVector.pushBack(ModPair<ModSize, LeafNode*>(0, pNode_));
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::getEstimateCount
//		-- おおよその文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	ModSize
//		おおよその文書頻度
//
//	EXCEPTIONS
//
ModSize
WithinUnorderedLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	initialize(cSearchInfo_);
	return m_uiEstimateCount;
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTINONS
//
void
WithinUnorderedLeafNode::reset()
{
	m_uiCurrentID = 0;
	NodeVector::Iterator i = m_cVector.begin();
	for (; i < m_cVector.end(); ++i)
		(*i).second->reset();
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::getTermFrequency -- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
WithinUnorderedLeafNode::getTermFrequency()
{
	if (m_uiTermFrequency == 0)
	{
		if (m_pLocation.get() == 0)
			m_pLocation = getLocationListIteratorImpl();

		m_uiTermFrequency = m_pLocation->getTermFrequency();
	}
	return m_uiTermFrequency;
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator::AutoPointer
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
WithinUnorderedLeafNode::getLocationListIterator()
{
	LocationListIterator::AutoPointer p;
	if (m_pLocation.get())
	{
		m_pLocation->reset();
		p = m_pLocation;
		m_pLocation = 0;
	}
	else
	{
		p = getLocationListIteratorImpl();
	}
	return p;
}

//
//	FUNCTION public
//	FullText2::WithinUnorderedLeafNode::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LeafNode*
//		コピー
//
//	EXCEPTIONS
//
LeafNode*
WithinUnorderedLeafNode::copy() const
{
	return new WithinUnorderedLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafNode::initialize -- 初期化する
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
WithinUnorderedLeafNode::initialize(SearchInformation& cSearchInfo_)
{
	if (m_bInitialized == false)
	{
		// 全要素の件数を見積もる
		m_uiEstimateCount = Os::Limits<ModSize>::getMax();
		NodeVector::Iterator i = m_cVector.begin();
		for (; i < m_cVector.end(); ++i)
		{
			ModSize n = (*i).second->getEstimateCount(cSearchInfo_);
			(*i).first = n;
			
			if (n < m_uiEstimateCount)
				m_uiEstimateCount = n;
		}
		
		// 文書頻度でソートする
		ModSort(m_cVector.begin(), m_cVector.end(), _Less());
		
		m_bInitialized = true;
	}
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafNode::lowerBoundImpl -- 下限検索を行う
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		文書ID
//	bool isRough_
//		ラフ検索かどうか
//
//	RETURN
//	FullText2::DocumentID
//	   	次の文書ID。存在しない場合は UndefinedDocumentID を返す
//
//	EXCEPTIONS
//
DocumentID
WithinUnorderedLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
										DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値ならそのまま返す
		return m_uiCurrentID;

	// 初期化
	initialize(cSearchInfo_);
	m_uiCurrentID = id_;
	m_uiTermFrequency = 0; // 初期化
	m_pLocation = 0;

	for (;;)
	{
		// まずはすべてのノードに含まれている文書を探す
		NodeVector::Iterator i = m_cVector.begin();
		while (i < m_cVector.end())
		{
			DocumentID id  = (*i).second->lowerBound(cSearchInfo_,
													 m_uiCurrentID, isRough_);
			if (id != m_uiCurrentID)
			{
				m_uiCurrentID = id;
				if (m_uiCurrentID == UndefinedDocumentID)
					// 見つからなかった
					return UndefinedDocumentID;
			
				if (i != m_cVector.begin())
				{
					// 文書IDが変わったので初めから
					
					i = m_cVector.begin();
					continue;
				}
			}
			++i;
		}
		
		if (isRough_ == false)
		{
			// ラフではないので、位置を確認する
			WithinUnorderedLeafLocationListIterator* location
				= getLocationListIteratorImpl();
			m_pLocation = location;
			int dummy = 0;
			if (location->nextImpl(dummy) == UndefinedLocation)
			{
				// 位置を確認したら、該当しなかった
				++m_uiCurrentID;
				m_pLocation = 0;
				continue;
			}
		}

		// 一致した
		break;
	}
	
	return m_uiCurrentID;
}

//
//	FUNCTION private
//	FullText2::WithinUnorderedLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::WithinUnorderedLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
WithinUnorderedLeafLocationListIterator*
WithinUnorderedLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<WithinUnorderedLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(WithinUnorderedLeafLocationListIterator*,
							   getFree());
	if (location == 0)
	{
		location
			= new WithinUnorderedLeafLocationListIterator(*this,
														  m_uiLower,
														  m_uiUpper,
														  m_cVector.getSize());
	}

	// 子ノードの位置情報へのイテレータを設定する
	NodeVector::Iterator i = m_cVector.begin();
	for (; i != m_cVector.end(); ++i)
	{
		location->pushBack((*i).second->getLocationListIterator());
	}

	// 初期化のためリセットする
	location->resetImpl();

	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
