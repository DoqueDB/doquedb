// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleLeafNode.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/SimpleLeafNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::SimpleLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* pToken_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleLeafNode::SimpleLeafNode(ListIterator* pToken_)
	: LeafNode(), m_pListIterator(pToken_)
{
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::~SimpleLeafNode -- デストラクタ
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
SimpleLeafNode::~SimpleLeafNode()
{
	// フリーリストを解放する
	clearFree();
	
	delete m_pListIterator;
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::SimpleLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SimpleLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SimpleLeafNode::SimpleLeafNode(const SimpleLeafNode& src_)
	: LeafNode(src_), m_pListIterator(0)
{
	m_pListIterator = src_.m_pListIterator->copy();
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::lowerBound -- 下限検索を行う
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
SimpleLeafNode::lowerBound(SearchInformation& cSearchInfo_,
						   DocumentID id_, bool isRough_)
{
	return m_pListIterator->lowerBound(cSearchInfo_, id_);
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::getTermFrequency -- 文書内頻度を得る
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
SimpleLeafNode::getTermFrequency()
{
	return m_pListIterator->getTermFrequency();
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::getLocationListIterator
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
SimpleLeafNode::getLocationListIterator()
{
	return m_pListIterator->getLocationListIterator();
}

//
//	FUNCTION public
//	FullText2::SimpleLeafNode::copy -- コピーを得る
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
SimpleLeafNode::copy() const
{
	return new SimpleLeafNode(*this);
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
