// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordLeafNode.cpp --
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
#include "FullText2/WordLeafNode.h"

#include "FullText2/ListIterator.h"
#include "FullText2/ExactWordLeafLocationListIterator.h"
#include "FullText2/SimpleWordLeafLocationListIterator.h"

#include "Exception/BadArgument.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::WordLeafNode::WordLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafNode* pLeafNode_
//		検索語ノード
//	FullText2::ListIterator* pSeprator_
//		単語境界
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordLeafNode::WordLeafNode(LeafNode* pLeafNode_,
						   ListIterator* pSeparator_)
	: UnaryLeafNode(pLeafNode_), m_pSeparator(pSeparator_),
	  m_uiTermFrequency(0), m_bExact(false)
{
	// 引数に指定されたノードへのポインタは、本クラスデストラクト時に破棄される
}

//
//	FUNCTION public
//	FullText2::WordLeafNode::~WordLeafNode -- デストラクタ
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
WordLeafNode::~WordLeafNode()
{
	m_pLocation = 0;
	// フリーリストを解放する
	clearFree();
	
	delete m_pSeparator;
}

//
//	FUNCTION public
//	FullText2::WordLeafNode::WordLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::WordLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
WordLeafNode::WordLeafNode(const WordLeafNode& src_)
	: UnaryLeafNode(src_), m_pSeparator(0),
	  m_uiTermFrequency(0), m_bExact(src_.m_bExact)
{
	m_pSeparator = src_.m_pSeparator->copy();
	m_cWordPosition = src_.m_cWordPosition;
}

//
//	FUNCTION public
//	FullText2::WordLeafNode::getEstimateCount
//		-- おおよその文書頻度を求める
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
WordLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	ModSize n = m_pLeafNode->getEstimateCount(cSearchInfo_);
	
	// 厳格一致だったら 1/4 にし、それ以外だったら 1/2 にする
	n /= (m_bExact ? 4 : 2);

	return n;
}

//
//	FUNCTION public
//	FullText2::WordLeafNode::getTermFrequency -- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
WordLeafNode::getTermFrequency()
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
//	FullText2::WordLeafNode::getLocationListIterator
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
WordLeafNode::getLocationListIterator()
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
//	FullText2::WordLeafNode::copy -- コピーを得る
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
WordLeafNode::copy() const
{
	return new WordLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::WordLeafNode::lowerBoundImpl -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		検索する文書ID
//	bool isRough_
//		ラフ検索かどうか
//
//	RETURN
//	FullText2::DocumentID
//		ヒットした文書ID
//
//	EXCEPTIONS
//
DocumentID
WordLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
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
		// まず LeafNode を検索する
		m_uiCurrentID = m_pLeafNode->lowerBound(cSearchInfo_,
												m_uiCurrentID, isRough_);
		if (m_uiCurrentID == UndefinedDocumentID)
			// これ以上存在していない
			return UndefinedDocumentID;

		if (isRough_ == false)
		{
			// 候補の文書が見つかったので、位置情報を確認して単語境界が
			// マッチしているか確認する

			// 単語境界の索引単位を検索する
			if (m_pSeparator->find(cSearchInfo_, m_uiCurrentID) == false)
			{
				// これはありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			m_pLocation = getLocationListIteratorImpl();
			int dummy = 0;
			if (m_pLocation->next(dummy) == UndefinedLocation)
			{
				// 位置と単語境界を確認したら、該当しなかった
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
//	FullText2::WordLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::WordLeafLocationListIterator*
//		単語単位検索用位置情報イテレータ
//
//	EXCEPTIONS
//
WordLeafLocationListIterator*
WordLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<WordLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(WordLeafLocationListIterator*, getFree());
	if (location.get() == 0)
	{
		if (m_bExact)
			location = new ExactWordLeafLocationListIterator(*this,
															 m_cWordPosition);
		else
			location = new SimpleWordLeafLocationListIterator(*this,
															  m_cWordPosition);
	}

	// 子ノードの位置情報へのイテレータを設定する
	location->setTerm(m_pLeafNode->getLocationListIterator());
	location->setSeparator(m_pSeparator->getLocationListIterator());

	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
