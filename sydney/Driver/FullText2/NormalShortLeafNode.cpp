// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalShortLeafNode.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/NormalShortLeafNode.h"
#include "FullText2/NormalShortLeafLocationListIterator.h"
#include "FullText2/SearchInformation.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::NormalShortLeafNode
//		-- コンストラクタ
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
NormalShortLeafNode::NormalShortLeafNode()
	: LeafNode(), m_uiCurrentID(0), m_uiTermFrequency(0),
	  m_pNormal(0), m_pShort(0), m_uiPos(0)
{
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::~NormalShortLeafNode
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
NormalShortLeafNode::~NormalShortLeafNode()
{
	m_pLocation = 0;
	// フリーリストを解放する
	clearFree();
	
	delete m_pNormal;
	delete m_pShort;
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::NormalShortLeafNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::NormalShortLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalShortLeafNode::
NormalShortLeafNode(const NormalShortLeafNode& src_)
	: LeafNode(src_), m_uiCurrentID(0), m_uiTermFrequency(0),
	  m_uiPos(src_.m_uiPos)
{
	m_pNormal = src_.m_pNormal->copy();
	m_pShort = src_.m_pShort->copy();
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::getEstimateCount
//		-- おおよその文書頻度を求める
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//	   	検索情報クラス
//
//	RETURN
//	ModSize
//		おおよその文書頻度
//
//	EXCEPTIONS
//
ModSize
NormalShortLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	ModSize count = m_pNormal->getEstimateCount(cSearchInfo_) / 2;
	return (count == 0) ? 1 : count;
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::reset -- リセットする
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
NormalShortLeafNode::reset()
{
	m_uiCurrentID = 0;
	m_pNormal->reset();
	m_pShort->reset();
}

//
//	FUNCTION public
//	FullText2::NormalShortLeafNode::getTermFrequency -- 文書内頻度を得る
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
NormalShortLeafNode::getTermFrequency()
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
//	FullText2::NormalShortLeafNode::getLocationListIterator
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
NormalShortLeafNode::getLocationListIterator()
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
//	FullText2::NormalShortLeafNode::copy -- コピーを得る
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
NormalShortLeafNode::copy() const
{
	return new NormalShortLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::NormalShortLeafNode::lowerBoundImpl -- 下限検索を行う
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
NormalShortLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
									DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値ならそのまま返す
		return m_uiCurrentID;

	m_uiCurrentID = id_;
	m_uiTermFrequency = 0;	// 初期化
	m_pLocation = 0;

	for (;;)
	{
		// まずは、NormalLeafNodeで文書を探す
		m_uiCurrentID = m_pNormal->lowerBound(cSearchInfo_,
											  m_uiCurrentID, isRough_);
		if (m_uiCurrentID == UndefinedDocumentID)
			// 見つからなかった
			return UndefinedDocumentID;

		// 次に ShortLeafNode で文書を探す
		DocumentID id = m_pShort->lowerBound(cSearchInfo_,
											 m_uiCurrentID, isRough_);
		if (id != m_uiCurrentID)
		{
			m_uiCurrentID = id;
			if (m_uiCurrentID == UndefinedDocumentID)
				// 見つからなかった
				return UndefinedDocumentID;

			// 文書IDが変わったので、NormalLeafNodeから調べ直す
			continue;
		}

		if (isRough_ == false)
		{
			// ラフではないので、位置を確認する
			NormalShortLeafLocationListIterator* location
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
//	FullText2::NormalShortLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::NormalShortLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
NormalShortLeafLocationListIterator*
NormalShortLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<NormalShortLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(NormalShortLeafLocationListIterator*,
							   getFree());
	if (location.get() == 0)
	{
		location = new NormalShortLeafLocationListIterator(*this);
	}

	// 子ノードの位置情報へのイテレータを設定する
	location->setNormal(m_pNormal->getLocationListIterator());
	location->setShort(m_uiPos, m_pShort->getLocationListIterator());

	return location.release();
}

//
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
