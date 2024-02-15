// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AtomicOrLeafNode.cpp --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/AtomicOrLeafNode.h"
#include "FullText2/AtomicOrLeafLocationListIterator.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::AtomicOrLeafNode -- コンストラクタ
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
AtomicOrLeafNode::AtomicOrLeafNode()
	: OrLeafNode(), m_uiTermFrequency(0)
{
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::~AtomicOrLeafNode -- デストラクタ
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
AtomicOrLeafNode::~AtomicOrLeafNode()
{
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::AtomicOrLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::AtomicOrLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
AtomicOrLeafNode::AtomicOrLeafNode(const AtomicOrLeafNode& src_)
	: OrLeafNode(src_), m_uiTermFrequency(0)
{
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::getTermFrequency -- 文書内頻度を得る
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
AtomicOrLeafNode::getTermFrequency()
{
	if (m_uiTermFrequency == 0)
	{
		NodeVector::Iterator i = m_cVector.begin();
		for (; i < m_cVector.end(); ++i)
			if ((*i).first == m_uiCurrentID)
				m_uiTermFrequency += (*i).second->getTermFrequency();
	}
	return m_uiTermFrequency;
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::getLocationListIterator
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
AtomicOrLeafNode::getLocationListIterator()
{
	return LocationListIterator::AutoPointer(getLocationListIteratorImpl());
}

//
//	FUNCTION public
//	FullText2::AtomicOrLeafNode::copy -- コピーを得る
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
AtomicOrLeafNode::copy() const
{
	return new AtomicOrLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::AtomicOrLeafNode::lowerBoundImpl -- 下限検索を行う
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		検索条件の文書ID
//	bool isRough_
//		ラフ検索するか否か
//
//	RETURN
//	FullText2::DocumentID
//	   	ヒットした文書ID。存在しない場合は UndefinedDocumentID を返す
//
//	EXCEPTIONS
//
DocumentID
AtomicOrLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
								 DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい場合はそのまま返す
		return m_uiCurrentID;

	m_uiCurrentID = id_;
	m_uiTermFrequency = 0;	// 初期化

	// 最小の文書IDを探す
	DocumentID id = UndefinedDocumentID;	// 最大値
	NodeVector::Iterator i = m_cVector.begin();
	for (; i < m_cVector.end(); ++i)
	{
		if ((*i).first < m_uiCurrentID)
			(*i).first = (*i).second->lowerBound(cSearchInfo_,
												 m_uiCurrentID,
												 isRough_);
		
		if ((*i).first < id)
			id = (*i).first;
	}

	m_uiCurrentID = id;
	
	return m_uiCurrentID;
}

//
//	FUNCTION private
//	FullText2::AtomicOrLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::AtomicOrLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
AtomicOrLeafLocationListIterator*
AtomicOrLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<AtomicOrLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(AtomicOrLeafLocationListIterator*, getFree());
	if (location.get() == 0)
		location = new AtomicOrLeafLocationListIterator(*this,
														m_cVector.getSize());

	// 子ノードの位置情報へのイテレータを設定する
	NodeVector::Iterator i = m_cVector.begin();
	for (; i != m_cVector.end(); ++i)
	{
		if ((*i).first == m_uiCurrentID)
			location->pushBack((*i).second->getLocationListIterator());
	}

	return location.release();
}

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
