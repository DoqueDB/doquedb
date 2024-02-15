// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AndLeafNode.cpp --
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
#include "FullText2/AndLeafNode.h"

#include "FullText2/SearchInformation.h"
#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::AndLeafNode -- コンストラクタ
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
AndLeafNode::AndLeafNode()
	: m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::~AndLeafNode -- デストラクタ
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
AndLeafNode::~AndLeafNode()
{
	// フリーリストを解放する
	clearFree();
	
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
		delete (*i);
	m_cVector.clear();
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::AndLeafNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::AndLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
AndLeafNode::AndLeafNode(const AndLeafNode& src_)
	: ArrayLeafNode(src_), m_uiCurrentID(0)
{
	m_cVector.reserve(src_.m_cVector.getSize());
	NodeVector::ConstIterator i = src_.m_cVector.begin();
	NodeVector::ConstIterator e = src_.m_cVector.end();
	for (; i < e; ++i)
		pushBack((*i)->copy());
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::pushBack -- トークンを追加する
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
AndLeafNode::pushBack(LeafNode* pNode_)
{
	// 与えられたノードは、本クラスデストラクト時に破棄される
	
	m_cVector.pushBack(pNode_);
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::getEstimateCount
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
AndLeafNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	double docCount = static_cast<double>(cSearchInfo_.getDocumentCount());
	double ratio = 1.0;
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i)->getEstimateCount(cSearchInfo_);
		double r = static_cast<double>(n)/docCount;
		ratio *= r;
	}
	ModSize count = static_cast<ModSize>(docCount * ratio);
	return (count == 0) ? 1 : count;
}

//
//	FUNCTION public
//	FullText2::AndLeafNode::reset -- リセットする
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
AndLeafNode::reset()
{
	m_uiCurrentID = 0;
	NodeVector::Iterator i = m_cVector.begin();
	NodeVector::Iterator e = m_cVector.end();
	for (; i < e; ++i)
		(*i)->reset();
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
