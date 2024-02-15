// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OrderedDistanceLeafNode.cpp --
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"
#include "FullText2/OrderedDistanceLeafNode.h"
#include "FullText2/OrderedDistanceLeafLocationListIterator.h"
#include "FullText2/SearchInformation.h"

#include "Os/Limits.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafNode::OrderedDistanceLeafNode
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
OrderedDistanceLeafNode::OrderedDistanceLeafNode()
	: AndLeafNode(), m_uiTermFrequency(0),
	  m_bInitialized(false), m_uiEstimateCount(0)
{
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafNode::~OrderedDistanceLeafNode
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
OrderedDistanceLeafNode::~OrderedDistanceLeafNode()
{
	m_pLocation = 0;
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafNode::OrderedDistanceLeafNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OrderedDistanceLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OrderedDistanceLeafNode::
OrderedDistanceLeafNode(const OrderedDistanceLeafNode& src_)
	: AndLeafNode(src_), m_uiTermFrequency(0),
	  m_bInitialized(false), m_uiEstimateCount(0)
{
}

//
//	FUNCTION punblic
//	FullText2::OrderedDistanceLeafNode::getEstimateCount
//		-- おおよその文書数を得る
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
ModSize
OrderedDistanceLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	initialize(cSearchInfo_);
	return m_uiEstimateCount;
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafNode::reset -- リセットする
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
OrderedDistanceLeafNode::reset()
{
	AndLeafNode::reset();
	m_bInitialized = false;
}

//
//	FUNCTION public
//	FullText2::OrderedDistanceLeafNode::getTermFrequency -- 文書内頻度を得る
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
OrderedDistanceLeafNode::getTermFrequency()
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
//	FullText2::OrderedDistanceLeafNode::getLocationListIterator
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
OrderedDistanceLeafNode::getLocationListIterator()
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
//	FullText2::OrderedDistanceLeafNode::copy -- コピーを得る
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
OrderedDistanceLeafNode::copy() const
{
	return new OrderedDistanceLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafNode::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OrderedDistanceLeafNode::initialize(SearchInformation& cSearchInfo_)
{
	if (m_bInitialized == false)
	{
		// 一番件数の少ないノードを設定する
		// ついでに件数も見積もる
		
		m_uiEstimateCount = Os::Limits<ModSize>::getMax();
		NodeVector::Iterator i = m_cVector.begin();
		m_iterator = i;	// 初期値
		for (; i < m_cVector.end(); ++i)
		{
			ModSize c = (*i)->getEstimateCount(cSearchInfo_);
			if (c < m_uiEstimateCount)
			{
				m_iterator = i;
				m_uiEstimateCount = c;
			}
		}

		m_bInitialized = true;
	}
}

//
//	FUNCTION private
//	FullText2::OrderedDistanceLeafNode::lowerBoundImpl -- 下限検索を行う
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
OrderedDistanceLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
										DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値ならそのまま返す
		return m_uiCurrentID;

	// 初期化する
	initialize(cSearchInfo_);
	
	m_uiCurrentID = id_;
	m_uiTermFrequency = 0;	// 初期化
	m_pLocation = 0;

	NodeVector::Iterator b = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();

	for (;;)
	{
		// まずは、いちばん文書頻度が少ないノードで文書を探す
		m_uiCurrentID = (*m_iterator)->lowerBound(cSearchInfo_,
												  m_uiCurrentID, isRough_);
		if (m_uiCurrentID == UndefinedDocumentID)
			// 見つからなかった
			return UndefinedDocumentID;
		
		// すべてのノードに含まれている文書を探す
		NodeVector::Iterator i = b;
		for (; i < e; ++i)
		{
			if (i == m_iterator)
				continue;
			
			DocumentID id  = (*i)->lowerBound(cSearchInfo_,
											  m_uiCurrentID, isRough_);
			if (id != m_uiCurrentID)
			{
				m_uiCurrentID = id;
				if (m_uiCurrentID == UndefinedDocumentID)
					// 見つからなかった
					return UndefinedDocumentID;
			
				// 文書IDが変わったので初めから
				break;
			}
		}

		if (i != e)
			continue;
		
		if (isRough_ == false)
		{
			// ラフではないので、位置を確認する
			OrderedDistanceLeafLocationListIterator* location
				= getLocationListIteratorImpl();
			m_pLocation = location;
			int dummy;
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
//	FullText2::OrderedDistanceLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OrderedDistanceLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
OrderedDistanceLeafLocationListIterator*
OrderedDistanceLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<OrderedDistanceLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(OrderedDistanceLeafLocationListIterator*,
							   getFree());
	if (location.get() == 0)
	{
		location
			= new OrderedDistanceLeafLocationListIterator(*this,
														  m_cVector.getSize());
	}

	// 子ノードの位置情報へのイテレータを設定する
	NodeVector::Iterator i = m_cVector.begin();
	for (; i != m_cVector.end(); ++i)
	{
		location->pushBack((*i)->getLocationListIterator());
	}

	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
