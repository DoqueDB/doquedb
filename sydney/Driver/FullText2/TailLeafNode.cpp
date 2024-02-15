// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TailLeafNode.cpp --
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
#include "FullText2/TailLeafNode.h"

#include "FullText2/SearchInformation.h"
#include "FullText2/TailLeafLocationListIterator.h"

#include "Exception/BadArgument.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::TailLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafNode* pLeafNode_
//		リーフノード
//	ModSize uiLocation_
//		後方からの位置(0から)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TailLeafNode::TailLeafNode(LeafNode* pLeafNode_,
						   ModSize uiLocation_)
	: UnaryLeafNode(pLeafNode_),
	  m_uiCurrentDocumentLength(0), m_uiLocation(uiLocation_)
{
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::~TailLeafNode -- デストラクタ
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
TailLeafNode::~TailLeafNode()
{
	m_pLocation = 0;
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::TailLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::TailLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TailLeafNode::TailLeafNode(const TailLeafNode& src_)
	: UnaryLeafNode(src_),
	  m_uiCurrentDocumentLength(src_.m_uiCurrentDocumentLength),
	  m_uiLocation(src_.m_uiLocation)
{
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::getEstimateCount
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
TailLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	// 1/30 にする
	
	ModSize n = m_pLeafNode->getEstimateCount(cSearchInfo_);
	n /= 30;
	return n;
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::lowerBound -- 下限検索を行う
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
TailLeafNode::lowerBound(SearchInformation& cSearchInfo_,
						 DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値ならそのまま返す
		return m_uiCurrentID;

	m_uiCurrentID = id_;
	m_pLocation = 0;

	for (;;)
	{
		// まず、LeafNode を検索する
		m_uiCurrentID = m_pLeafNode->lowerBound(cSearchInfo_,
												m_uiCurrentID, isRough_);
		if (m_uiCurrentID == UndefinedDocumentID)
			// これ以上存在していない
			return UndefinedDocumentID;
		
		if (isRough_ == false)
		{
			// 候補の文書が見つかったので、位置情報を確認して単語境界が
			// マッチしているか確認する

			// 文書長を設定する
			if (cSearchInfo_.getDocumentLength(m_uiCurrentID,
											   m_uiCurrentDocumentLength)
				== false)
			{
				// ありえない
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		
			TailLeafLocationListIterator* location
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
//	FUNCTION public
//	FullText2::TailLeafNode::getTermFrequency -- 文書内頻度を得る
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
TailLeafNode::getTermFrequency()
{
	return 1;
}

//
//	FUNCTION public
//	FullText2::TailLeafNode::getLocationListIterator
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
TailLeafNode::getLocationListIterator()
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
//	FullText2::TailLeafNode::copy -- コピーを得る
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
TailLeafNode::copy() const
{
	return new TailLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::TailLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	
//
//	RETURN
//	FullText2::TailLeafLocationListIterator*
//		単語単位検索用位置情報イテレータ
//
//	EXCEPTIONS
//
TailLeafLocationListIterator*
TailLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<TailLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(TailLeafLocationListIterator*, getFree());
	if (location.get() == 0)
		location = new TailLeafLocationListIterator(*this,
													m_uiLocation);
	
	// 子ノードの位置情報へのイテレータを設定する
	location->setTerm(m_pLeafNode->getLocationListIterator());

	// 文字列長を設定する
	location->setDocumentLength(m_uiCurrentDocumentLength);
	
	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
