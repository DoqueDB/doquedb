// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalLeafNode.cpp --
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
#include "FullText2/NormalLeafNode.h"

#include "FullText2/NormalLeafLocationListIterator.h"
#include "FullText2/ListManager.h"
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
//	FullText2::NormalLeafNode::NormalLeafNode -- コンストラクタ
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
NormalLeafNode::NormalLeafNode()
	: LeafNode(), m_uiCurrentID(0), m_uiTermFrequency(0),
	  m_bInitialized(false), m_bNolocation(false)
{
}

//
//	FUNCTION public
//	FullText2::NormalLeafNode::~NormalLeafNode -- デストラクタ
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
NormalLeafNode::~NormalLeafNode()
{
	m_pLocation = 0;
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
//	FullText2::NormalLeafNode::NormalLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::NormalLeafNode& src_
//	   	コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalLeafNode::NormalLeafNode(const NormalLeafNode& src_)
	: LeafNode(src_),
	  m_uiCurrentID(0), m_uiTermFrequency(0),
	  m_bInitialized(src_.m_bInitialized), m_bNolocation(src_.m_bNolocation)
{
	m_cVector.reserve(src_.m_cVector.getSize());
	NodeVector::ConstIterator i = src_.m_cVector.begin();
	NodeVector::ConstIterator e = src_.m_cVector.end();
	for (; i < e; ++i)
		pushBack((*i).first, (*i).second->copy());
}

//
//	FUNCTION public
//	FullText2::NormalLeafNode::pushBack -- トークンを追加する
//
//	NOTES
//	文書頻度の小さい順に追加しないと非効率
//
//	ARGUMENTS
//	ModSize pos_
//		トークンの位置
//	FullText2::ListIterator* pToken_
//		トークン(索引単位)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalLeafNode::pushBack(ModSize pos_, ListIterator* pToken_)
{
	// 与えられた索引単位は、本クラスデストラクト時に破棄される
	m_cVector.pushBack(NodePair(pos_, pToken_));
}

//
//	FUNCTION public
//	FullText2::NormalLeafNode::getEstimateCount -- おおよその文書頻度を得る
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
NormalLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	ModSize m = 1;	// 連続していた場合に割る数
	ModSize count = Os::Limits<ModSize>::getMax();
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	int j = 0;
	for (; i < e; ++i, ++j)
	{
		ModSize n = (*i).second->getEstimateCount();
		
		if (j > 0 && m < 10000)
		{
			// uni-gram の場合は、隣合う索引単位があるたびに 1/10 にする
			// それ以外の場合は、4つ目以上の場合に 1/2 にする
			
			if ((*i).second->getLength() <= 1)
				m *= 10;
			else if (j > 2)
				m *= 2;
		}
		
		if (n < count)
			// 一番少ないカウントを採用する
			count = n;
	}
	count /= m;
	
	return (count == 0) ? 1 : count;
}

//
//	FUNCTION public
//	FullText2::NormalLeafNode::getTermFrequency -- 文書内頻度を得る
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
NormalLeafNode::getTermFrequency()
{
	if (m_uiTermFrequency == 0)
	{
		if (m_bNolocation == true)
		{
			NodeVector::Iterator i = m_cVector.begin();
			NodeVector::Iterator e = m_cVector.end();
			for (; i < e; ++i)
			{
				// 一番小さいTFを採用
				
				if (m_uiTermFrequency == 0 ||
					m_uiTermFrequency > (*i).second->getTermFrequency())
				{
					m_uiTermFrequency = (*i).second->getTermFrequency();
				}
			}
		}
		else
		{
			if (m_pLocation.get() == 0)
				m_pLocation = getLocationListIteratorImpl();
			
			m_uiTermFrequency = m_pLocation->getTermFrequency();
		}
	}
	return m_uiTermFrequency;
}

//
//	FUNCTION public
//	FullText2::NormalLeafNode::getLocationListIterator
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
NormalLeafNode::getLocationListIterator()
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
//	FullText2::NormalLeafNode::copy -- コピーを得る
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
NormalLeafNode::copy() const
{
	return new NormalLeafNode(*this);
}

//
//	FUNCTION private
//	FullText2::NormalLeafNode::lowerBoundImpl -- 下限検索を行う
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
NormalLeafNode::lowerBoundImpl(SearchInformation& cSearchInfo_,
							   DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 現在値より小さい場合はそのまま返す
		return m_uiCurrentID;

	if (m_bInitialized == false)
	{
		// 高速化のためキャッシュする
		m_bNolocation = cSearchInfo_.isNolocation();
		m_bInitialized = true;
	}

	m_uiCurrentID = id_;
	m_uiTermFrequency = 0;					// 初期化
	m_pLocation = 0;

	NodeVector::Iterator b = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	
	for (;;)
	{
		// すべての索引単位に含まれている文書IDを探す
		NodeVector::Iterator i = b;
		while (i < e)
		{
			DocumentID id = (*i).second->lowerBound(cSearchInfo_,
													m_uiCurrentID);
			if (id != m_uiCurrentID)
			{
				m_uiCurrentID = id;
				if (m_uiCurrentID == UndefinedDocumentID)
					// これ以上存在していない
					return UndefinedDocumentID;
			
				if (i != b)
				{
					// 文書IDが変わったので初めから
					//
					//【注意】
					//	m_cVector内は文書頻度の少ない順にソートされているので
					//	続きから確認するより、最初に戻った方がたぶん効率的である

					i = b;
					continue;
				}
			}
			++i;
		}

		if (isRough_ == false && m_bNolocation == false)
		{
			// 次に位置情報を突きあわせる
			NormalLeafLocationListIterator* location
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
//	FullText2::NormalLeafNode::getLocationListIteratorImpl
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::NormalLeafLocationListIterator*
//		位置情報へのイテレータ
//
//	EXCEPTIONS
//
NormalLeafLocationListIterator*
NormalLeafNode::getLocationListIteratorImpl()
{
	// インスタンスを確保する
	ModAutoPointer<NormalLeafLocationListIterator> location
		= _SYDNEY_DYNAMIC_CAST(NormalLeafLocationListIterator*, getFree());
	if (location.get() == 0)
		location = new NormalLeafLocationListIterator(*this,
													  m_cVector.getSize());

	// 子ノードの位置情報へのイテレータを設定する
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		location->pushBack((*i).first, (*i).second->getLocationListIterator());
	}

	return location.release();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
