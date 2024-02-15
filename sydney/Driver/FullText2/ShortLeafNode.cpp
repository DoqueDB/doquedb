// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortLeafNode.cpp --
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
#include "FullText2/ShortLeafNode.h"

#include "FullText2/ListIterator.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/ShortLeafLocationListIterator.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::ShortLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iLength_
//		検索文字列長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortLeafNode::ShortLeafNode(int iLength_)
	: m_iLength(iLength_),
	  m_uiCurrentID(0), m_uiTermFrequency(0)
{
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::~ShortLeafNode -- デストラクタ
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
ShortLeafNode::~ShortLeafNode()
{
	// フリーリストを解放する
	clearFree();
	
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
		delete (*i).second;
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::ShortLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ShortLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortLeafNode::ShortLeafNode(const ShortLeafNode& src_)
	: LeafNode(src_), m_iLength(src_.m_iLength),
	  m_uiCurrentID(0), m_uiTermFrequency(0)
{
	m_cVector.reserve(src_.m_cVector.getSize());
	NodeVector::ConstIterator i = src_.m_cVector.begin();
	NodeVector::ConstIterator e = src_.m_cVector.end();
	for (; i < e; ++i)
		pushBack((*i).second->copy());
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::pushBack -- トークンを追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* pToken_
//		トークン(索引単位)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortLeafNode::pushBack(ListIterator* pToken_)
{
	// 与えられた索引単位は、本クラスデストラクト時に破棄される
	
	m_cVector.pushBack(NodePair(0, pToken_));
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::getEstimateCount -- おおよその文書頻度を得る
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
ShortLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	ModSize docCount = cSearchInfo_.getDocumentCount();
	ModSize count = 0;
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i).second->getEstimateCount();
		double r = static_cast<double>(n)/static_cast<double>(docCount);
		count += static_cast<ModSize>(
			static_cast<double>(docCount - count) * r);
	}

	return count;
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::reset -- リセットする
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
ShortLeafNode::reset()
{
	m_uiCurrentID = 0;
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
		(*i).first = 0;
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::getTermFrequency -- 文書内頻度を得る
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
ShortLeafNode::getTermFrequency()
{
	if (m_uiTermFrequency == 0)
	{
		NodeVector::Iterator i = m_cVector.begin();
		NodeVector::Iterator e = m_cVector.end();
		for (; i < e; ++i)
			if ((*i).first == m_uiCurrentID)
				m_uiTermFrequency += (*i).second->getTermFrequency();
	}
	return m_uiTermFrequency;
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::getLocationListIterator
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
ShortLeafNode::getLocationListIterator()
{
	return LocationListIterator::AutoPointer(getLocationListIteratorImpl());
}

//
//	FUNCTION public
//	FullText2::ShortLeafNode::copy -- コピーを得る
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
ShortLeafNode::copy() const
{
	return new ShortLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::ShortLeafNode::lowerBoundImpl -- 下限検索を行う
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		検索条件の文書ID
//	bool isRough_
//		ラフ検索かどうか
//
//	RETURN
//	FullText2::DocumentID
//	   	検索した文書ID。存在しない場合は UndefinedDocumentID を返す
//
//	EXCEPTIONS
//
DocumentID
ShortLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値ならそのまま返す
		return m_uiCurrentID;

	//【注意】	メンバー変数の m_uiCurrentID を直接使わず、
	//			ローカル変数 d_id を利用しているのは高速化のため
	
	DocumentID d_id = UndefinedDocumentID;	// 最大値
	m_uiTermFrequency = 0; // 初期化

	// 最小の文書IDを探す
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		DocumentID tmp = (*i).first;
		
		if (tmp < id_)
		{
			tmp = (*i).second->lowerBound(cSearchInfo_, id_);
			(*i).first = tmp;
		}
		
		if (tmp < d_id)
			d_id = tmp;
	}

	m_uiCurrentID = d_id;
	return m_uiCurrentID;
}

//
//	FUNCTION private
//	FullText2::ShortLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ShortLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
ShortLeafLocationListIterator*
ShortLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<ShortLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(ShortLeafLocationListIterator*, getFree());
	if (location.get() == 0)
		location = new ShortLeafLocationListIterator(*this,
													 m_iLength,
													 m_cVector.getSize());

	// 子ノードの位置情報へのイテレータを設定する
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		if ((*i).first == m_uiCurrentID)
			location->pushBack((*i).second->getLocationListIterator());
	}

	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
